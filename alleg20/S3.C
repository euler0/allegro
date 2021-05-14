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
 *      Video driver for S3 graphics cards.
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


static BITMAP *s3_init(int w, int h, int v_w, int v_h);
static void s3_scroll(int x, int y);



GFX_DRIVER gfx_s3 = 
{
   "S3",
   "",
   s3_init,
   NULL,
   s3_scroll,
   0, 0, FALSE, 
   0x10000,       /* 64k banks */
   0x10000,       /* 64k granularity */
   0
};



/* get_s3_mode:
 *  Returns an S3 video mode number for the specified width and height.
 */
static int get_s3_mode(int w, int h)
{
   if ((w == 640) && (h == 480))
      return 0x101;

   if ((w == 800) && (h == 600))
      return 0x103;

   if ((w == 1024) && (h == 768))
      return 0x105;

   return 0;
}



/* s3_detect:
 *  Detects the presence of a S3 card.
 */
static int s3_detect()
{
   _write_vga_register(_crtc, 0x38, 0);                  /* disable ext. */
   if (!_test_vga_register(_crtc, 0x35, 0xF)) {          /* test */
      _write_vga_register(_crtc, 0x38, 0x48);            /* enable ext. */
      if (_test_vga_register(_crtc, 0x35, 0xF))          /* test again */
	 return TRUE;                                    /* found it */
   }

   return FALSE;
}



/* s3_init:
 *  Tries to enter the specified graphics mode, and makes a screen bitmap
 *  for it.
 */
static BITMAP *s3_init(int w, int h, int v_w, int v_h)
{
   BITMAP *b;
   int mode;
   int height;
   int width;
   __dpmi_regs r;

   if (!s3_detect()) {
      strcpy(allegro_error, "S3 not found");
      return NULL;
   }

   LOCK_FUNCTION(_s3_bank);

   mode = get_s3_mode(w, h);
   if (mode == 0) {
      strcpy(allegro_error, "Resolution not supported");
      return NULL;
   }

   gfx_s3.vid_mem = _vesa_vidmem_check(1024*1024);

   width = MAX(w, v_w);
   _sort_out_virtual_width(&width, &gfx_s3);
   height = gfx_s3.vid_mem / width;
   if ((width > 1024) || (h > height) || (v_w > width) || (v_h > height)) {
      strcpy(allegro_error, "Virtual screen size too large");
      return NULL;
   }

   b = _make_bitmap(width, height, 0xA0000, &gfx_s3);
   if (!b)
      return NULL;

   r.x.ax = 0x4F02;
   r.x.bx = mode;
   __dpmi_int(0x10, &r);                        /* set gfx mode */
   if (r.h.ah) {
      strcpy(allegro_error, "S3 set gfx mode failed");
      destroy_bitmap(b);
      return NULL;
   }

   _set_vga_virtual_width(w, width);

   gfx_s3.w = b->cr = w;
   gfx_s3.h = b->cb = h;

   b->write_bank = b->read_bank = _s3_bank;

   return b;
}



/* s3_scroll:
 *  Hardware scrolling for S3 cards.
 */
static void s3_scroll(int x, int y)
{
   long a = x + (y * VIRTUAL_W);

   /* write high bits to S3-specific registers */
   _write_vga_register(_crtc, 0x38, 0x48);
   _alter_vga_register(_crtc, 0x31, 0x30, a>>14);
   _write_vga_register(_crtc, 0x38, 0);

   /* write to normal VGA address registers */
   _write_vga_register(_crtc, 0x0D, (a>>2) & 0xFF);
   _write_vga_register(_crtc, 0x0C, (a>>10) & 0xFF);
}

