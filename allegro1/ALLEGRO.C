/*
		  //  /     /     ,----  ,----.  ,----.  ,----.
		/ /  /     /     /      /    /  /    /  /    /
	      /  /  /     /     /___   /       /____/  /    /
	    /---/  /     /     /      /  __   /\      /    /
	  /    /  /     /     /      /    /  /  \    /    /
	/     /  /____ /____ /____  /____/  /    \  /____/

	Low Level Game Routines (version 1.0)

	Initialise / terminate, interrupt handlers and things like that.

	See allegro.txt for instructions and copyright conditions.

	By Shawn Hargreaves,
	1 Salisbury Road,
	Market Drayton,
	Shropshire,
	England TF9 1AJ
	email slh100@tower.york.ac.uk (until 1996)
*/


#include "stdlib.h"
#include "bios.h"
#include "dos.h"

#include "allegro.h"

#ifdef GCC
#include "go32.h"
#include "string.h"
#include "dpmi.h"
#include "errno.h"
#endif   /* ifdef GCC */

short allegro_use_timer = TRUE;

static short _a_rez = 3;            /* original rez */
static short _a_inited = FALSE;     /* has the system been initialised? */
static PALLETE _a_pallete;          /* original pallete */

short _setup_fill_array(short size);
void _install_int(void);
void _remove_int(void);

short _my_int_q_size = 0;       /* my own int queue */

struct {
   void ((*p)());
   short speed;
   short counter;
} _my_int_queue[8];



short allegro_init()
{
   register short i;
   extern _RGB _black_rgb;
#ifdef BORLAND 
   union REGS r;
#else
   __dpmi_regs r;
#endif

   if (!_a_inited) {

      get_pallete(_a_pallete);

      for (i=0; i<256; i++)
	 black_pallete[i] = _black_rgb;

      for (i=16; i<256; i++)
	 desktop_pallete[i] = desktop_pallete[i & 0x0f];

      r.x.ax = 0x0f00;
#ifdef BORLAND
      int86(0x10, &r, &r);
#else
      __dpmi_int(0x10, &r);
#endif
      _a_rez = r.x.ax & 0xf;     /* store current video mode */

      r.x.ax = 0x13;
#ifdef BORLAND
      int86(0x10, &r, &r);       /* get graphics mode 13h */
#else
      __dpmi_int(0x10, &r);
#endif

      screen = (BITMAP *)malloc(sizeof(BITMAP)+(sizeof(char *)*SCREEN_H));
      if (!screen)
	return -1;
      screen->w = screen->cr = SCREEN_W;
      screen->h = screen->cb = SCREEN_H;
      screen->clip = TRUE;
      screen->cl = screen->ct = 0;
      screen->dat = NULL;
      screen->size = 64000L;
#ifdef BORLAND
      screen->line[0] = (char *)0xa0000000L;
#else /* GCC */
      screen->seg = _go32_conventional_mem_selector();
      screen->line[0] = (char *)0xa0000L;
#endif
      for (i=1; i<SCREEN_H; i++)
	 screen->line[i] = screen->line[i-1] + SCREEN_W;
      if (!(_setup_fill_array(SCREEN_H))) {
	 free(screen);
	 screen = NULL;
	 return -1;
      }

#ifdef GCC       /* mouse init fails after we take over int8 */
      install_mouse();
#endif
      if (allegro_use_timer)
	 _install_int();
      install_divzero();
      _a_inited = TRUE;
   }
   return 0;
}



void allegro_exit()
{
#ifdef BORLAND
   union REGS r;
#else
   __dpmi_regs r;
#endif

   if (_a_inited) {
      clear(screen);
      set_pallete(_a_pallete);

      if (allegro_use_timer)
	 _remove_int();
      remove_keyboard();
      remove_mouse();
      remove_divzero();

      free(screen);
      screen = NULL;

      r.x.ax = _a_rez;
#ifdef BORLAND
      int86(0x10, &r, &r);     /* restore original video mode */
#else
      __dpmi_int(0x10, &r);
#endif

      _a_inited = FALSE;
   }
}




short _keyboard_mode = FALSE;    /* do we have a keyboard handler? */

volatile char key[128];          /* the key flags */

static char _key_asc_table[128] =
{
   0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
   '-', '=', 8, 9, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
   '[', ']', 13, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
   ';', '\'', '`', 0, 0, 'z', 'x', 'c', 'v', 'b', 'n', 'm',
   ',', '.', '/', 0, '*', 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, '-', 0, '5', 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static char _key_shift_table[128] =
{
   0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
   '_', '+', 8, 9, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
   '{', '}', 13, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
   ':', '"', '~', 0, 0, 'Z', 'X', 'C', 'V', 'B', 'N', 'M',
   '<', '>', '?', 0, '*', 0, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, '-', 0, '5', 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define KEY_BUFFER_SIZE 32

volatile short _k_temp;          /* stop djgpp messing up with stack reads */
volatile long _key_buffer[KEY_BUFFER_SIZE];     /* buffer for keypresses */
volatile short _key_buffer_start = 0;
volatile short _key_buffer_end = 0;

void _clear_keybuf(void);


void _clear_keybuf()
{
   short c;

   for (c=0; c<128; c++)
      key[c] = FALSE;

   _key_buffer_start = 0;
   _key_buffer_end = 0;
}



void clear_keybuf()
{
   _key_buffer_start = 0;
   _key_buffer_end = 0;
}



short keypressed()
{
   if (_key_buffer_start == _key_buffer_end)
      return FALSE;
   else
      return TRUE;
}



long readkey()
{
   long r;

   if (!_keyboard_mode)
      return 0;

   do {
   } while (_key_buffer_start == _key_buffer_end);  /* wait for a press */

   disable();
   r = _key_buffer[_key_buffer_start];
   _key_buffer_start++;
   if (_key_buffer_start >= KEY_BUFFER_SIZE)
      _key_buffer_start = 0;
   enable();

   return r;
}



#ifdef BORLAND
void far interrupt _my_keyint()
#else
void _my_keyint(_go32_dpmi_registers *regs)
#endif
{
   _k_temp = inportb(0x60);   /* read keyboard byte */

   if (_k_temp & 0x80)        /* key was released */
      key[_k_temp&0x7f] = FALSE; 

   else {                     /* key was pressed */
      _k_temp &= 0x7f;
      key[_k_temp] = TRUE;

      if ((_k_temp != KEY_CONTROL) && (_k_temp != KEY_LSHIFT) &&
	  (_k_temp != KEY_RSHIFT) && (_k_temp != KEY_ALT) &&
	  (_k_temp != KEY_CAPSLOCK) && (_k_temp != KEY_NUMLOCK) &&
	  (_k_temp != KEY_SCRLOCK)) {

	 if ((key[KEY_LSHIFT]) || (key[KEY_RSHIFT]))
	    _key_buffer[_key_buffer_end] =
	       ((long)_k_temp<<8) + _key_shift_table[_k_temp];
	 else
	    _key_buffer[_key_buffer_end] =
	       ((long)_k_temp<<8) + _key_asc_table[_k_temp];
	 _key_buffer_end++;
	 if (_key_buffer_end >= KEY_BUFFER_SIZE)
	    _key_buffer_end = 0;
	 if (_key_buffer_end == _key_buffer_start) {  /* buffer full */
	    _key_buffer_start++;
	    if (_key_buffer_start >= KEY_BUFFER_SIZE)
	       _key_buffer_start = 0;
	 }
      }
   }

   outportb(0x20,0x20);       /* ack. */
}



volatile short mouse_x = 0;        /* mouse x pos */
volatile short mouse_y = 0;        /* mouse y pos */
volatile short mouse_b = 0;        /* mouse button state */

void (*mouse_callback)() = NULL;   /* mouse change callback function */

short _mouse_mode = FALSE;

typedef struct MOUSE_SPRITE   /* a specific structure for 16x8 sprites */
{
   short flags;
   short w;
   short h;
#ifdef GCC
   short filler;
#endif
   char dat[256];
} MOUSE_SPRITE;

MOUSE_SPRITE _mouse_sprite =
{
   SPRITE_MASKED, 16, 16,
#ifdef GCC
   0,
#endif
   {
      16, 16, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
      16, 15, 16, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
      16, 15, 15, 16, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
      16, 15, 15, 15, 16, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
      16, 15, 15, 15, 15, 16, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
      16, 15, 15, 15, 15, 15, 16, 0,  0,  0,  0,  0,  0,  0,  0,  0, 
      16, 15, 15, 15, 15, 15, 15, 16, 0,  0,  0,  0,  0,  0,  0,  0,
      16, 15, 15, 15, 15, 15, 15, 15, 16, 0,  0,  0,  0,  0,  0,  0,
      16, 15, 15, 15, 15, 15, 15, 15, 15, 16, 0,  0,  0,  0,  0,  0, 
      16, 15, 15, 15, 15, 15, 16, 16, 16, 0,  0,  0,  0,  0,  0,  0, 
      16, 15, 15, 16, 15, 15, 16, 0,  0,  0,  0,  0,  0,  0,  0,  0, 
      16, 15, 16, 0,  16, 15, 15, 16, 0,  0,  0,  0,  0,  0,  0,  0, 
      0,  16, 0,  0,  16, 15, 15, 16, 0,  0,  0,  0,  0,  0,  0,  0, 
      0,  0,  0,  0,  0,  16, 15, 15, 16, 0,  0,  0,  0,  0,  0,  0, 
      0,  0,  0,  0,  0,  16, 15, 15, 16, 0,  0,  0,  0,  0,  0,  0, 
      0,  0,  0,  0,  0,  0,  16, 16, 0,  0,  0,  0,  0,  0,  0,  0, 
   }
};


SPRITE *mouse_sprite = (SPRITE *)&_mouse_sprite;   /* mouse pointer */

BITMAP *_mouse_bmp = NULL;       /* where to draw the pointer */

volatile short _mx, _my;        /* previous mouse position */

volatile MOUSE_SPRITE _ms =   /* previous screen data */
{
   SPRITE_OPAQUE, 16, 16
};


void _mouse_move()         /* routine that draws the mouse pointer */
{
   short cf = _mouse_bmp->clip;
   short cl = _mouse_bmp->cl;
   short cr = _mouse_bmp->cr;
   short ct = _mouse_bmp->ct;
   short cb = _mouse_bmp->cb;

   _mouse_bmp->clip = TRUE;
   _mouse_bmp->cl = _mouse_bmp->ct = 0;
   _mouse_bmp->cr = SCREEN_W;
   _mouse_bmp->cb = SCREEN_H;

   drawsprite(_mouse_bmp, (SPRITE *)&_ms, _mx, _my);

   _mx = mouse_x - 1;
   _my = mouse_y - 1;

   get_sprite((SPRITE *)&_ms, _mouse_bmp, _mx, _my);
   drawsprite(_mouse_bmp, mouse_sprite, _mx, _my);

   _mouse_bmp->clip = cf;
   _mouse_bmp->cl = cl;
   _mouse_bmp->cr = cr;
   _mouse_bmp->ct = ct;
   _mouse_bmp->cb = cb;
}



void show_mouse(bmp)
BITMAP *bmp;
{
   short cf, cl, cr, ct, cb;

   if (!_mouse_mode)
      install_mouse();

   if (_mouse_bmp) {
      mouse_callback = NULL;

      cf = _mouse_bmp->clip;
      cl = _mouse_bmp->cl;
      cr = _mouse_bmp->cr;
      ct = _mouse_bmp->ct;
      cb = _mouse_bmp->cb; 

      _mouse_bmp->clip = TRUE;
      _mouse_bmp->cl = _mouse_bmp->ct = 0;
      _mouse_bmp->cr = SCREEN_W;
      _mouse_bmp->cb = SCREEN_H;

      drawsprite(_mouse_bmp, (SPRITE *)&_ms, _mx, _my);

      _mouse_bmp->clip = cf;
      _mouse_bmp->cl = cl;
      _mouse_bmp->cr = cr;
      _mouse_bmp->ct = ct;
      _mouse_bmp->cb = cb;
   }
   if (bmp) {
      cf = bmp->clip;
      cl = bmp->cl;
      cr = bmp->cr;
      ct = bmp->ct;
      cb = bmp->cb;

      bmp->clip = TRUE;
      bmp->cl = bmp->ct = 0;
      bmp->cr = SCREEN_W;
      bmp->cb = SCREEN_H;

      _mx = mouse_x - 1;
      _my = mouse_y - 1;

      get_sprite((SPRITE *)&_ms, bmp, _mx, _my);
      drawsprite(bmp, mouse_sprite, _mx, _my);
      _mouse_bmp = bmp;
      mouse_callback = _mouse_move;

      bmp->clip = cf;
      bmp->cl = cl;
      bmp->cr = cr;
      bmp->ct = ct;
      bmp->cb = cb;
   }
   else {
      _mouse_bmp = NULL;
   }
}



#ifdef GCC


volatile int _callback_flag = FALSE;

_go32_dpmi_seginfo mouse_seginfo;
_go32_dpmi_registers mouse_regs;



void _mouseint(_go32_dpmi_registers *r)
{
   mouse_b = r->x.bx;
   mouse_x = r->x.cx;
   mouse_y = r->x.dx;
   _callback_flag = TRUE;
}



void install_mouse()
{
   __dpmi_regs r;

   if (_mouse_mode)
      return;

   _mouse_mode = TRUE;

   r.x.ax = 0;
   __dpmi_int(0x33, &r);          /* initialise mouse driver */
   if (r.x.ax == 0) {
      _mouse_mode = FALSE;
      return;
   }

   mouse_seginfo.pm_offset = (int)_mouseint;
   mouse_seginfo.pm_selector = _my_cs();
   _go32_dpmi_allocate_real_mode_callback_retf(&mouse_seginfo, &mouse_regs);
   r.x.ax = 0x0C;
   r.x.cx = 0x1f;
   r.x.dx = mouse_seginfo.rm_offset;
   r.x.es = mouse_seginfo.rm_segment;
   __dpmi_int(0x33, &r);         /* install callback */
   r.x.es = 0;

   r.x.ax = 7;
   r.x.cx = 0;
   r.x.dx = SCREEN_W - 1;
   __dpmi_int(0x33, &r);         /* set horizontal range */

   r.x.ax = 8;
   r.x.dx = SCREEN_H - 1;
   __dpmi_int(0x33, &r);         /* set vertical range */

   r.x.ax = 4;
   r.x.cx = SCREEN_W / 2;
   r.x.dx = SCREEN_H / 2;
   __dpmi_int(0x33, &r);         /* position the mouse */

   mouse_x = SCREEN_W / 2;
   mouse_y = SCREEN_H / 2;
   return;
}



void remove_mouse()
{
   __dpmi_regs r;

   if (!_mouse_mode)
      return;

   r.x.ax = 0x0C;
   r.x.cx = 0;
   r.x.dx = 0;
   r.x.es = 0;
   __dpmi_int(0x33, &r);         /* install NULL callback */

   mouse_x = mouse_y = mouse_b = 0;
   _mouse_mode = FALSE;

   _go32_dpmi_free_real_mode_callback(&mouse_seginfo);
}



#endif      /* ifdef GCC */


#ifdef BORLAND



void _mouseint()
{
   asm {
      push ds

      mov ax, SEG &mouse_b
      mov ds, ax
      mov mouse_b, bx               // store mouse state
      mov mouse_x, cx
      mov mouse_y, dx

      mov ax, word ptr mouse_callback    // do we have a callback function?
      or ax, word ptr mouse_callback + 2
      je no_callback

      call mouse_callback

   no_callback:
      pop ds
   }
}



void install_mouse()
{
   union REGS r;
   struct SREGS s;

   if (_mouse_mode)
      return;

   _mouse_mode = TRUE;

   r.x.ax = 0;
   int86(0x33, &r, &r);          /* initialise mouse driver */
   if (r.x.ax == 0) {
      _mouse_mode = FALSE;
      return;
   }

   r.x.ax = 0x0C;
   r.x.cx = 0x1f;
   r.x.dx = FP_OFF(_mouseint);
   s.es = FP_SEG(_mouseint);
   int86x(0x33, &r, &r, &s);     /* install new mouse interrupt */

   r.x.ax = 7;
   r.x.cx = 0;
   r.x.dx = SCREEN_W - 1;
   int86(0x33, &r, &r);          /* set horizontal range */

   r.x.ax = 8;
   r.x.dx = SCREEN_H - 1;
   int86(0x33, &r, &r);          /* set vertical range */

   r.x.ax = 4;
   r.x.cx = SCREEN_W / 2;
   r.x.dx = SCREEN_H / 2;
   int86(0x33, &r, &r);          /* position the mouse */

   mouse_x = SCREEN_W / 2;
   mouse_y = SCREEN_H / 2;
   return;
}



void remove_mouse()
{
   union REGS r;
   struct SREGS s;

   if (!_mouse_mode)
      return;

   r.x.dx = 0;
   r.x.ax = 0x0C;
   r.x.cx = 0;
   s.es = 0;
   int86x(0x33, &r, &r, &s);        /* install NULL callback */

   mouse_x = mouse_y = mouse_b = 0;
   _mouse_mode = FALSE;
}



void interrupt (*_oldint1C)(void) = NULL;
void interrupt (*_oldint8)(void) = NULL;

short _countdown = 0;



void far interrupt _my_int1C()
{
   /* int 1C handler, calls our int routines as required */

   asm {
      mov si, _my_int_q_size
      shl si, 3                                 // position in table

   int_loop:
      sub si, 8                                 // 8 byte entries
      js int_done                               // are we finished?

      dec word ptr _my_int_queue+6[si]          // decrement counter
      jg int_loop

      mov ax, word ptr _my_int_queue+4[si]
      mov word ptr _my_int_queue+6[si], ax      // counter = speed
      push si
      call dword ptr _my_int_queue[si]          // call the routine
      pop si
      jmp int_loop

   int_done:
   }
}



void far interrupt _my_int8()
{
   /* int 8 handler, calls int 1C and old int as required */

   asm {
      dec word ptr _countdown
      jg our_tick

      mov word ptr _countdown, 11      // call the bios handler
      pushf 
      call dword ptr _oldint8
      jmp int_done

   our_tick:               // this is one of our ticks
			   // call 0x1c and take care of 8259 PIC
      int 0x1C
      mov dx, 32
      mov al, 32
      out dx, al

   int_done:
   }
}



void _install_int()
{
   /* install our timer int and speed the system timer up by a factor
      of eleven, giving 200 ticks per second (18.2 * 11 = 200.2) */

   _countdown = 11;
   disable();
   outportb(0x43, 0x36);
   outportb(0x40, 0x46);   // 0x10000 / 11 = 0x1746
   outportb(0x40, 0x17);
   _oldint8 = getvect(0x08);
   setvect(0x08, _my_int8);
   enable();
   _oldint1C = getvect(0x1C);
   setvect(0x1C, _my_int1C);
}



void _remove_int()
{
   /* remove our timer int and restore timer to old speed */

   disable();
   outportb(0x43, 0x36);
   outportb(0x40, 0);
   outportb(0x40, 0);
   if (_oldint8)
      setvect(0x08, _oldint8);
   enable() ;
   if (_oldint1C)
      setvect(0x1C, _oldint1C);
}



void interrupt (*_oldkeyint)(void) = NULL;


void install_keyboard()
{
   if (_keyboard_mode)
      return;

   _clear_keybuf();
   _keyboard_mode = TRUE;
   _oldkeyint = getvect(9);
   setvect(9, _my_keyint);
}



void remove_keyboard()
{
   if (!_keyboard_mode)
      return;
   if (_oldkeyint)
      setvect(9, _oldkeyint);
   _keyboard_mode = FALSE;
   _clear_keybuf();
}



#endif      /* ifdef BORLAND */



#ifdef GCC     /* djgpp (32 bit pmode) versions of the above */



_go32_dpmi_seginfo _timer_rm_oldint;   /* original real mode timer IRQ */
_go32_dpmi_seginfo _timer_rm_int;      /* real mode interrupt segment info */
_go32_dpmi_registers _timer_rm_regs;
_go32_dpmi_seginfo _timer_pm_oldint;   /* original prot-mode timer IRQ */
_go32_dpmi_seginfo _timer_pm_int;      /* prot-mode interrupt segment info */

volatile int _countdown = 0;
int _bios_counter;

#define BIOS_CLOCK   0x46c

#define TIMER_INT    0x8

void _my_timerint(_go32_dpmi_registers *regs);
   /* _my_timerint() in misc.s */



void _install_int()
{
   /* install our timer int and speed the system timer up by a factor
      of eleven, giving 200 ticks per second (18.2 * 11 = 200.2) */

   _countdown = 11;
   dosmemget(BIOS_CLOCK, 4, &_bios_counter);

   disable();              /* speed up timer */
   outportb(0x43, 0x36);
   outportb(0x40, 0x46);   /* 0x10000 / 11 = 0x1746 */
   outportb(0x40, 0x17);
   enable();

   _timer_pm_int.pm_offset = (int)_my_timerint;       /* pm handler */
   _go32_dpmi_allocate_iret_wrapper(&_timer_pm_int);
   _timer_pm_int.pm_selector = _go32_my_cs();
   _go32_dpmi_get_protected_mode_interrupt_vector(TIMER_INT, &_timer_pm_oldint);
   _go32_dpmi_set_protected_mode_interrupt_vector(TIMER_INT, &_timer_pm_int);

   memset(&_timer_rm_regs,0,sizeof(_go32_dpmi_registers));    /* rm handler */
   _timer_rm_int.pm_offset = (int)_my_timerint;
   _go32_dpmi_allocate_real_mode_callback_iret(&_timer_rm_int, &_timer_rm_regs);
   _go32_dpmi_get_real_mode_interrupt_vector(TIMER_INT, &_timer_rm_oldint);
   _go32_dpmi_set_real_mode_interrupt_vector(TIMER_INT, &_timer_rm_int);
}



void _remove_int()
{
   /* remove our timer int and restore timer to old speed */

   disable();
   outportb(0x43, 0x36);
   outportb(0x40, 0);
   outportb(0x40, 0);
   enable();

   _go32_dpmi_set_protected_mode_interrupt_vector(TIMER_INT, &_timer_pm_oldint);
   _go32_dpmi_free_iret_wrapper(&_timer_pm_int);
   _go32_dpmi_set_real_mode_interrupt_vector(TIMER_INT, &_timer_rm_oldint);
   _go32_dpmi_free_real_mode_callback(&_timer_rm_int);
   dosmemput(&_bios_counter, 4, BIOS_CLOCK);
}



_go32_dpmi_seginfo _key_rm_oldint;   /* original real mode key IRQ */
_go32_dpmi_seginfo _key_rm_int;      /* real mode interrupt segment info */
_go32_dpmi_registers _key_rm_regs;
_go32_dpmi_seginfo _key_pm_oldint;   /* original prot-mode key IRQ */
_go32_dpmi_seginfo _key_pm_int;      /* prot-mode interrupt segment info */



void install_keyboard()
{
   if (_keyboard_mode)
      return;

   _keyboard_mode = TRUE;
   _clear_keybuf();

   _key_pm_int.pm_offset = (int)_my_keyint;       /* pm handler */
   _go32_dpmi_allocate_iret_wrapper(&_key_pm_int);
   _key_pm_int.pm_selector = _go32_my_cs();
   _go32_dpmi_get_protected_mode_interrupt_vector(9, &_key_pm_oldint);
   _go32_dpmi_set_protected_mode_interrupt_vector(9, &_key_pm_int);

   memset(&_key_rm_regs,0,sizeof(_go32_dpmi_registers));    /* rm handler */
   _key_rm_int.pm_offset = (int)_my_keyint;
   _go32_dpmi_allocate_real_mode_callback_iret(&_key_rm_int, &_key_rm_regs);
   _go32_dpmi_get_real_mode_interrupt_vector(9, &_key_rm_oldint);
   _go32_dpmi_set_real_mode_interrupt_vector(9, &_key_rm_int);
}



void remove_keyboard()
{
   if (!_keyboard_mode)
      return;

   _go32_dpmi_set_protected_mode_interrupt_vector(9, &_key_pm_oldint);
   _go32_dpmi_free_iret_wrapper(&_key_pm_int);
   _go32_dpmi_set_real_mode_interrupt_vector(9, &_key_rm_oldint);
   _go32_dpmi_free_real_mode_callback(&_key_rm_int);
   _clear_keybuf();
   _keyboard_mode = FALSE;
}



#endif      /* ifdef GCC */



short install_int(proc, speed)
void (*proc)();
short speed;
{
   /* add a new entry to our list of interrupt routines */

   if (_my_int_q_size >= 8)      /* no room in our queue */
      return -1;

   _my_int_queue[_my_int_q_size].speed = 
   _my_int_queue[_my_int_q_size].counter =
      (short)((long)speed * (long)TICKS_PER_SECOND / 1000L);

   _my_int_queue[_my_int_q_size].p = proc;
   _my_int_q_size++;
   return _my_int_q_size;
}



void remove_int(proc)
void (*proc)();
{
   /* remove an entry from our list of interrupt routines */

   short x;
   short i, i2;

   for (i=0; i<_my_int_q_size; i++) {     /* look for the proc. */
      if (_my_int_queue[i].p == proc) {   /* found it! */
	 x = _my_int_q_size - 1;
	 _my_int_q_size = 0;
	 for (i2=i; i2<x; i2++)
	    _my_int_queue[i2] = _my_int_queue[i2+1];
	 _my_int_q_size = x;
	 break;
      }
   }
}



#define PAUSE_RES    (1000 / TICKS_PER_SECOND)

volatile long _rest_count;

void _rest_int()
{
   _rest_count--;
}



void rest(time)
long time;
{
   _rest_count = time / PAUSE_RES;
   install_int(_rest_int, PAUSE_RES);
   do {
   } while (_rest_count > 0);
   remove_int(_rest_int);
}



