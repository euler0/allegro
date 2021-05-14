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
 *      Video driver for Tseng ET3000 and ET4000 graphics cards.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pc.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/movedata.h>
#include <sys/farptr.h>

#include "allegro.h"
#include "internal.h"


static BITMAP *et3000_init(int w, int h, int v_w, int v_h);
static BITMAP *et4000_init(int w, int h, int v_w, int v_h);
static void tseng_scroll(int x, int y);


#define ET_NONE         0
#define ET_3000         1
#define ET_4000         2

static int tseng_type = ET_NONE;



GFX_DRIVER gfx_et3000 = 
{
   "Tseng ET3000",
   "",
   et3000_init,
   NULL,
   tseng_scroll,
   0, 0, FALSE, 
   0x10000,       /* 64k banks */
   0x10000,       /* 64k granularity */
   0
};



GFX_DRIVER gfx_et4000 = 
{
   "Tseng ET4000",
   "",
   et4000_init,
   NULL,
   tseng_scroll,
   0, 0, FALSE, 
   0x10000,       /* 64k banks */
   0x10000,       /* 64k granularity */
   0
};



/* get_tseng_mode:
 *  Returns a Tseng BIOS video mode number for the specified width and height.
 */
static int get_tseng_mode(int w, int h)
{
   if ((w == 320) && (h == 200))
      return 0x13;

   if ((w == 640) && (h == 350))
      return 0x2D;

   if ((w == 640) && (h == 480))
      return 0x2E;

   if ((w == 800) && (h == 600))
      return 0x30;

   if (tseng_type == ET_3000)
      return 0;

   if ((w == 640) && (h == 400))
      return 0x2F;

   if ((w == 1024) && (h == 768))
      return 0x38;

   return 0;
}



/* tseng_detect:
 *  Detects the presence of a Tseng card. Returns one of the ET_* constants.
 */
static int tseng_detect()
{
   outportb(0x3BF, 3);
   outportb(_crtc+4, 0xA0);                           /* enable extensions */

   if (!_test_register(0x3CD, 0x3F))
      return ET_NONE;

   if (!_test_vga_register(_crtc, 0x33, 0x0F))        /* ET3000 */
      return ET_3000;
   else
      return ET_4000;
}



/* tseng_setup:
 *  Helper function to create a screen bitmap for one of the Tseng drivers.
 */
static BITMAP *tseng_setup(int w, int h, int v_w, int v_h, GFX_DRIVER *driver)
{
   BITMAP *b;
   int mode;
   int height;
   int width;
   __dpmi_regs r;

   mode = get_tseng_mode(w, h);
   if (mode == 0) {
      strcpy(allegro_error, "Resolution not supported");
      return NULL;
   }

   driver->vid_mem = _vesa_vidmem_check(driver->vid_mem);

   width = MAX(w, v_w);
   _sort_out_virtual_width(&width, driver);
   height = driver->vid_mem / width;
   if ((width > 1024) || (h > height) || (v_w > width) || (v_h > height)) {
      strcpy(allegro_error, "Virtual screen size too large");
      return NULL;
   }

   b = _make_bitmap(width, height, 0xA0000, driver);
   if (!b)
      return NULL;

   r.x.ax = mode;                               /* set gfx mode */
   __dpmi_int(0x10, &r);

   _set_vga_virtual_width(w, width);

   driver->w = b->cr = w;
   driver->h = b->cb = h;

   return b;
}



/* et3000_init:
 *  Tries to enter the specified graphics mode, and makes a screen bitmap
 *  for it.
 */
static BITMAP *et3000_init(int w, int h, int v_w, int v_h)
{
   BITMAP *b;

   tseng_type = tseng_detect();
   if (tseng_type != ET_3000) {
      strcpy(allegro_error, "Tseng ET3000 not found");
      return NULL;
   }

   LOCK_FUNCTION(_et3000_write_bank);
   LOCK_FUNCTION(_et3000_read_bank);

   gfx_et3000.vid_mem = 512*1024;

   b = tseng_setup(w, h, v_w, v_h, &gfx_et3000);

   if (b) {
      b->write_bank = _et3000_write_bank;
      b->read_bank = _et3000_read_bank;
   }

   return b;
}



/* et4000_init:
 *  Tries to enter the specified graphics mode, and makes a screen bitmap
 *  for it.
 */
static BITMAP *et4000_init(int w, int h, int v_w, int v_h)
{
   BITMAP *b;

   tseng_type = tseng_detect();
   if (tseng_type != ET_4000) {
      strcpy(allegro_error, "Tseng ET4000 not found");
      return NULL;
   }

   LOCK_FUNCTION(_et4000_write_bank);
   LOCK_FUNCTION(_et4000_read_bank);

   gfx_et4000.vid_mem = 1024*1024;

   b = tseng_setup(w, h, v_w, v_h, &gfx_et4000);

   if (b) {
      b->write_bank = _et4000_write_bank;
      b->read_bank = _et4000_read_bank;
   }

   return b;
}



/* tseng_scroll:
 *  Hardware scrolling for Tseng cards.
 */
static void tseng_scroll(int x, int y)
{
   long a = x + (y * VIRTUAL_W);

   /* write high bit(s) to Tseng-specific registers */
   if (tseng_type == ET_3000)
      _alter_vga_register(_crtc, 0x23, 2, a>>17);
   else
      _alter_vga_register(_crtc, 0x33, 3, a>>18);

   /* write to normal VGA address registers */
   _write_vga_register(_crtc, 0x0D, (a>>2) & 0xFF);
   _write_vga_register(_crtc, 0x0C, (a>>10) & 0xFF);

   /* write low 2 bits to VGA horizontal pan register */
   vsync();
   _alter_vga_register(0x3C0, 0x33, 0x0F,
		       (gfx_driver->w==320) ? ((a&3)<<1) : (a&3));
}


