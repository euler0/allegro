/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *      By Shawn Hargreaves,
 *      1 Salisbury Road,
 *      Market Drayton,
 *      Shropshire,
 *      England, TF9 1AJ.
 *
 *      Various bits and pieces for setting up and cleaning up the system.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <crt0.h>
#include <sys/segments.h>
#include <signal.h>
#include <unistd.h>
#include <io.h>


/* this definition will force the compilation of the inlined functions
 * in allegro.h, so there will be a copy in the library for use when
 * compiling without optimisation, and in case anything needs to take 
 * the address of the functions.
 */
#define __inline__
#include "allegro.h"
#undef __inline__

#include "internal.h"


/* I think this is the default in any case, but it can't hurt to be sure.
 * Using the unix sbrk algorithm with Allegro would be a Bad Thing.
 */
int _crt0_startup_flags = _CRT0_FLAG_NONMOVE_SBRK;


/* in case you want to report version numbers */
char allegro_id[] = "Allegro " VERSION_STR ", by Shawn Hargreaves, " DATE_STR;


/* error message for sound and gfx init routines */
char allegro_error[80] = "";


/* the graphics driver currently in use */
GFX_DRIVER *gfx_driver = NULL;


/* a bitmap structure for accessing the physical screen */
BITMAP *screen = NULL;


int _xor = FALSE;


PALLETE black_pallete;

RGB black_rgb = { 0, 0, 0 };

PALLETE desktop_pallete = {
   { 63, 63, 63 },   { 63, 0,  0 },
   { 0,  63, 0 },    { 63, 63, 0 },
   { 0,  0,  63 },   { 63, 0,  63 },
   { 0,  63, 63 },   { 16, 16, 16 },
   { 31, 31, 31 },   { 63, 31, 31 },
   { 31, 63, 31 },   { 63, 63, 31 },
   { 31, 31, 63 },   { 63, 31, 63 },
   { 31, 63, 63 },   { 0,  0,  0 }
};


FILL_STRUCT *_fill_array = NULL;
int _bitmap_table_size = 0;


int _last_bank_1 = 0;
int _last_bank_2 = 0;
int *_gfx_bank = NULL;

int _crtc = 0x3D4;     /* port address of CRTC registers */


#define MAX_EXIT_FUNCS     8

/* we have to use a dynamic list rather than calling all the cleanup
 * routines directly, so the linker won't end up including more code than 
 * is really required.
 */
typedef void (*funcptr)();
static funcptr exit_funcs[MAX_EXIT_FUNCS];



/* _add_exit_func:
 *  Adds a function to the list that need to be called by allegro_exit().
 */
void _add_exit_func(void (*func)())
{
   int c;

   for (c=0; c<MAX_EXIT_FUNCS; c++) {
      if (!exit_funcs[c]) {
	 exit_funcs[c] = func;
	 break;
      }
   }
}



/* _remove_exit_func:
 *  Removes a function from the list that need to be called by allegro_exit().
 */
void _remove_exit_func(void (*func)())
{
   int c;

   for (c=0; c<MAX_EXIT_FUNCS; c++) {
      if (exit_funcs[c] == func) {
	 exit_funcs[c] = NULL;
	 break;
      }
   }
}



/* signal_handler:
 *  Used to trap various signals, to make sure things get shut down cleanly.
 */
static void signal_handler(int num)
{
   static char msg[] = "Shutting down Allegro\r\n";

   allegro_exit();

   _write(STDERR_FILENO, msg, sizeof(msg)-1);

   signal(num, SIG_DFL);
   raise(num);
}



/* allegro_init:
 *  Initialises the Allegro library. Doesn't actually do very much except
 *  setting up some variables, locking some memory, and installing 
 *  allegro_exit() as an atexit routine.
 */
int allegro_init()
{
   int i;

   void _read_vga_register_end(), _write_vga_register_end(), 
	_alter_vga_register_end(), _test_vga_register_end(), 
	_test_register_end();

   for (i=0; i<256; i++)
      black_pallete[i] = black_rgb;

   for (i=16; i<256; i++)
      desktop_pallete[i] = desktop_pallete[i & 0x0F];

   for (i=0; i<MAX_EXIT_FUNCS; i++)
      exit_funcs[i] = NULL;

   if (inportb(0x3CC) & 1)       /* detect CRTC register address */
      _crtc = 0x3D4;
   else 
      _crtc = 0x3B4;

   LOCK_VARIABLE(screen);
   LOCK_VARIABLE(gfx_driver);
   LOCK_VARIABLE(_crtc);
   LOCK_VARIABLE(_xor);
   LOCK_FUNCTION(_read_vga_register);
   LOCK_FUNCTION(_write_vga_register);
   LOCK_FUNCTION(_alter_vga_register);
   LOCK_FUNCTION(_test_vga_register);
   LOCK_FUNCTION(_test_register);

   atexit(allegro_exit);

   signal(SIGABRT, signal_handler);
   signal(SIGFPE,  signal_handler);
   signal(SIGILL,  signal_handler);
   signal(SIGSEGV, signal_handler);
   signal(SIGTERM, signal_handler);
   signal(SIGINT,  signal_handler);
   signal(SIGKILL, signal_handler);
   signal(SIGQUIT, signal_handler);

   return 0;
}



/* allegro_exit:
 *  Closes down the Allegro system. This includes restoring the initial
 *  pallete and video mode, and removing whatever mouse, keyboard, and
 *  timer routines have been installed.
 */
void allegro_exit()
{
   int c;

   for (c=0; c<MAX_EXIT_FUNCS; c++)
      if (exit_funcs[c])
	 (*exit_funcs[c])();
}



/* _read_vga_register:
 *  Reads the contents of a VGA register.
 */
int _read_vga_register(int port, int index)
{
   if (port==0x3C0)
      inportb(_crtc+6); 

   outportb(port, index);
   return inportb(port+1);
}

END_OF_FUNCTION(_read_vga_register);



/* _write_vga_register:
 *  Writes a byte to a VGA register.
 */
void _write_vga_register(int port, int index, int v) 
{
   if (port==0x3C0) {
      inportb(_crtc+6);
      outportb(port, index);
      outportb(port, v);
   }
   else {
      outportb(port, index);
      outportb(port+1, v);
   }
}

END_OF_FUNCTION(_write_vga_register);



/* _alter_vga_register:
 *  Alters specific bits of a VGA register.
 */
void _alter_vga_register(int port, int index, int mask, int v)
{
   int temp;
   temp = _read_vga_register(port, index);
   temp &= (~mask);
   temp |= (v & mask);
   _write_vga_register(port, index, temp);
}

END_OF_FUNCTION(_alter_vga_register);



/* _test_vga_register:
 *  Tests whether specific bits of a VGA register can be changed.
 */
int _test_vga_register(int port, int index, int mask)
{
   int old, nw1, nw2;

   old = _read_vga_register(port, index);
   _write_vga_register(port, index, old & (~mask));
   nw1 = _read_vga_register(port, index) & mask;
   _write_vga_register(port, index, old | mask);
   nw2 = _read_vga_register(port, index) & mask;
   _write_vga_register(port, index, old);

   return ((nw1==0) && (nw2==mask));
}

END_OF_FUNCTION(_test_vga_register);



/* _test_register:
 *  Tests whether specific bits of a register can be changed.
 */
int _test_register(int port, int mask)
{
   int old, nw1, nw2;

   old = inportb(port);
   outportb(port, old & (~mask));
   nw1 = inportb(port) & mask;
   outportb(port, old | mask);
   nw2 = inportb(port) & mask;
   outportb(port, old);

   return ((nw1==0) && (nw2==mask));
}

END_OF_FUNCTION(_test_register);



/* _set_vga_virtual_width:
 *  Used by various graphics drivers to adjust the virtual width of
 *  the screen, using VGA register 0x3D4 index 0x13.
 */
void _set_vga_virtual_width(int old_width, int new_width)
{
   int width = _read_vga_register(0x3D4, 0x13);
   _write_vga_register(_crtc, 0x13, (width * new_width) / old_width);
}


