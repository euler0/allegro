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
 *      Mouse routines (using the int 33 driver).
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <dos.h>
#include <go32.h>
#include <dpmi.h>

#include "allegro.h"
#include "internal.h"


volatile int mouse_x = 0;                    /* mouse x pos */
volatile int mouse_y = 0;                    /* mouse y pos */
volatile int mouse_b = 0;                    /* mouse button state */

static int mouse_installed = FALSE;


static unsigned char mouse_pointer_data[256] =
{
   16,  16,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
   16,  255, 16,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
   16,  255, 255, 16,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
   16,  255, 255, 255, 16,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
   16,  255, 255, 255, 255, 16,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, 
   16,  255, 255, 255, 255, 255, 16,  0,   0,   0,   0,   0,   0,   0,   0,   0, 
   16,  255, 255, 255, 255, 255, 255, 16,  0,   0,   0,   0,   0,   0,   0,   0, 
   16,  255, 255, 255, 255, 255, 255, 255, 16,  0,   0,   0,   0,   0,   0,   0, 
   16,  255, 255, 255, 255, 255, 255, 255, 255, 16,  0,   0,   0,   0,   0,   0, 
   16,  255, 255, 255, 255, 255, 16,  16,  16,  0,   0,   0,   0,   0,   0,   0, 
   16,  255, 255, 16,  255, 255, 16,  0,   0,   0,   0,   0,   0,   0,   0,   0, 
   16,  255, 16,  0,   16,  255, 255, 16,  0,   0,   0,   0,   0,   0,   0,   0, 
   0,   16,  0,   0,   16,  255, 255, 16,  0,   0,   0,   0,   0,   0,   0,   0, 
   0,   0,   0,   0,   0,   16,  255, 255, 16,  0,   0,   0,   0,   0,   0,   0, 
   0,   0,   0,   0,   0,   16,  255, 255, 16,  0,   0,   0,   0,   0,   0,   0, 
   0,   0,   0,   0,   0,   0,   16,  16,  0,   0,   0,   0,   0,   0,   0,   0
};

static BITMAP *mouse_pointer = NULL;         /* default mouse pointer */

static BITMAP *mouse_sprite = NULL;          /* mouse pointer */

BITMAP *_mouse_screen = NULL;                /* where to draw the pointer */

static int mx, my;                           /* previous mouse position */
static BITMAP *ms = NULL;                    /* previous screen data */

static _go32_dpmi_seginfo mouse_seginfo;
static _go32_dpmi_registers mouse_regs;



/* draw_mouse:
 *  Mouse pointer drawing routine. If remove is set, deletes the old mouse
 *  pointer. If add is set, draws a new one.
 */
static void draw_mouse(int remove, int add)
{
   int cf = _mouse_screen->clip;
   int cl = _mouse_screen->cl;
   int cr = _mouse_screen->cr;
   int ct = _mouse_screen->ct;
   int cb = _mouse_screen->cb;

   _mouse_screen->clip = TRUE;
   _mouse_screen->cl = _mouse_screen->ct = 0;
   _mouse_screen->cr = _mouse_screen->w;
   _mouse_screen->cb = _mouse_screen->h;

   if (remove)
      blit(ms, _mouse_screen, 0, 0, mx, my, mouse_sprite->w, mouse_sprite->h);

   mx = mouse_x - 1;
   my = mouse_y - 1;

   if (add) {
      blit(_mouse_screen, ms, mx, my, 0, 0, mouse_sprite->w, mouse_sprite->h);
      draw_sprite(_mouse_screen, mouse_sprite, mx, my);
   }

   _mouse_screen->clip = cf;
   _mouse_screen->cl = cl;
   _mouse_screen->cr = cr;
   _mouse_screen->ct = ct;
   _mouse_screen->cb = cb;
}

static END_OF_FUNCTION(draw_mouse);



/* mouse_move:
 *  Do we need to redraw the mouse pointer?
 */
static void mouse_move() 
{
   if ((mx != mouse_x - 1) || (my != mouse_y - 1))
      draw_mouse(TRUE, TRUE);
}

static END_OF_FUNCTION(mouse_move);



/* mouseint:
 *  Mouse movement callback for the int 33 mouse driver.
 */
static void mouseint(_go32_dpmi_registers *r)
{
   mouse_b = r->x.bx;
   mouse_x = r->x.cx / 8;
   mouse_y = r->x.dx / 8;
}

static END_OF_FUNCTION(mouseint);



/* set_mouse_sprite:
 *  Sets the sprite to be used for the mouse pointer. If the sprite is
 *  NULL, restores the default arrow.
 */
void set_mouse_sprite(struct BITMAP *sprite)
{
   BITMAP *old_mouse_screen = _mouse_screen;

   if (_mouse_screen)
      show_mouse(NULL);

   if (sprite)
      mouse_sprite = sprite;
   else
      mouse_sprite = mouse_pointer;

   lock_bitmap(mouse_sprite);

   /* make sure the ms bitmap is big enough */
   if ((!ms) || (ms->w < mouse_sprite->w) || (ms->h < mouse_sprite->h)) {
      if (ms)
	 destroy_bitmap(ms);
      ms = create_bitmap(mouse_sprite->w, mouse_sprite->h);
      lock_bitmap(ms);
   }

   if (old_mouse_screen)
      show_mouse(old_mouse_screen);
}



/* show_mouse:
 *  Tells Allegro to display a mouse pointer. This only works when the timer 
 *  module is active. The mouse pointer will be drawn onto the bitmap bmp, 
 *  which should normally be the hardware screen. To turn off the mouse 
 *  pointer, which you must do before you draw anything onto the screen, call 
 *  show_mouse(NULL). If you forget to turn off the mouse pointer when 
 *  drawing something, the SVGA bank switching code will become confused and 
 *  will produce garbage all over the screen.
 */
void show_mouse(BITMAP *bmp)
{
   if (!mouse_installed)
      return;

   remove_int(mouse_move);

   if (_mouse_screen)
      draw_mouse(TRUE, FALSE);

   _mouse_screen = bmp;

   if (bmp) {
      draw_mouse(FALSE, TRUE);
      install_int(mouse_move, 20);
   }
}



/* position_mouse:
 *  Moves the mouse to screen position x, y. This is safe to call even
 *  when a mouse pointer is being displayed.
 */
void position_mouse(int x, int y)
{
   __dpmi_regs r;
   BITMAP *old_mouse_screen = _mouse_screen;

   if (_mouse_screen)
      show_mouse(NULL);

   r.x.ax = 4;
   r.x.cx = x * 8;
   r.x.dx = y * 8;
   __dpmi_int(0x33, &r); 

   mouse_x = x;
   mouse_y = y;

   if (old_mouse_screen)
      show_mouse(old_mouse_screen);
}



/* _set_mouse_range:
 *  The int33 driver tends to report mouse movements in chunks of 4 or 8
 *  pixels when in an svga mode. So, we increase the mouse range and
 *  sensitivity, and then divide all the values it returns by 8.
 */
void _set_mouse_range()
{
   __dpmi_regs r;

   r.x.ax = 7;
   r.x.cx = 0;
   r.x.dx = (SCREEN_W - 1) * 8;
   __dpmi_int(0x33, &r);         /* set horizontal range */

   r.x.ax = 8;
   r.x.cx = 0;
   r.x.dx = (SCREEN_H - 1) * 8;
   __dpmi_int(0x33, &r);         /* set vertical range */

   r.x.ax = 15;
   r.x.cx = 2;
   r.x.dx = 2;
   __dpmi_int(0x33, &r);         /* set sensitivity */

   position_mouse(SCREEN_W/2, SCREEN_H/2);
}



/* install_mouse:
 *  Installs the Allegro mouse handler. You must do this before using any
 *  other mouse functions. Returns zero for success, and -1 if it can't find 
 *  a mouse driver.
 */
int install_mouse()
{
   __dpmi_regs r;
   int x;
   extern void draw_sprite_end(), blit_end();

   if (mouse_installed)
      return -1;

   r.x.ax = 0;
   __dpmi_int(0x33, &r);         /* initialise mouse driver */

   if (r.x.ax == 0)
      return -1;

   LOCK_VARIABLE(mouse_x);
   LOCK_VARIABLE(mouse_y);
   LOCK_VARIABLE(mouse_b);
   LOCK_VARIABLE(mouse_sprite);
   LOCK_VARIABLE(mouse_pointer_data);
   LOCK_VARIABLE(mouse_pointer);
   LOCK_VARIABLE(_mouse_screen);
   LOCK_VARIABLE(mx);
   LOCK_VARIABLE(my);
   LOCK_VARIABLE(ms);
   LOCK_VARIABLE(mouse_seginfo);
   LOCK_VARIABLE(mouse_regs);
   LOCK_FUNCTION(draw_mouse);
   LOCK_FUNCTION(mouse_move);
   LOCK_FUNCTION(mouseint);
   LOCK_FUNCTION(draw_sprite);
   LOCK_FUNCTION(blit);

   /* create the mouse pointer bitmap */
   mouse_pointer = malloc(sizeof(BITMAP) + sizeof(char *)*16);
   if (!mouse_pointer)
      return NULL;

   mouse_pointer->w = mouse_pointer->cr = 16;
   mouse_pointer->h = mouse_pointer->cb = 16;
   mouse_pointer->cl = mouse_pointer->ct = 0;
   mouse_pointer->clip = TRUE;
   mouse_pointer->dat = NULL;
   mouse_pointer->seg = _my_ds();
   mouse_pointer->read_bank = mouse_pointer->write_bank = _stub_bank_switch;
   for (x=0; x<16; x++)
      mouse_pointer->line[x] = mouse_pointer_data + x*16;

   set_mouse_sprite(mouse_pointer);

   /* create real mode callback for the int33 driver */
   mouse_seginfo.pm_offset = (int)mouseint;
   mouse_seginfo.pm_selector = _my_cs();
   _go32_dpmi_allocate_real_mode_callback_retf(&mouse_seginfo, &mouse_regs);
   r.x.ax = 0x0C;
   r.x.cx = 0x1f;
   r.x.dx = mouse_seginfo.rm_offset;
   r.x.es = mouse_seginfo.rm_segment;
   __dpmi_int(0x33, &r);         /* install callback */

   _set_mouse_range();

   _add_exit_func(remove_mouse);
   mouse_installed = TRUE;
   return 0;
}



/* remove_mouse:
 *  Removes the mouse handler. You don't normally need to call this, because
 *  allegro_exit() will do it for you.
 */
void remove_mouse()
{
   __dpmi_regs r;

   if (!mouse_installed)
      return;

   r.x.ax = 0x0C;
   r.x.cx = 0;
   r.x.dx = 0;
   r.x.es = 0;
   __dpmi_int(0x33, &r);         /* install NULL callback */

   mouse_x = mouse_y = mouse_b = 0;

   _go32_dpmi_free_real_mode_callback(&mouse_seginfo);

   if (mouse_pointer) {
      destroy_bitmap(mouse_pointer);
      mouse_pointer = NULL;
   }

   if (ms) {
      destroy_bitmap(ms);
      ms = NULL;
   }

   _remove_exit_func(remove_mouse);
   mouse_installed = FALSE;
}

