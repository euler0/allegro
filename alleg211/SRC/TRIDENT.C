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
 *      Video driver for Trident graphics cards.
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


static BITMAP *trident_init(int w, int h, int v_w, int v_h);
static int trident_scroll(int x, int y);



GFX_DRIVER gfx_trident = 
{
   "Trident",
   "",
   trident_init,
   NULL,
   trident_scroll,
   _vga_vsync,
   _vga_set_pallete_range,
   0, 0, FALSE, 
   0x10000,       /* 64k banks */
   0x10000,       /* 64k granularity */
   0
};



static GFX_MODE_INFO trident_mode_list[] =
{
   {  640,  400,  0x5C,  0  },
   {  640,  480,  0x5D,  0  },
   {  800,  600,  0x5E,  0  },
   {  1024, 768,  0x62,  0  },
   {  0,    0,    0,     0  }
};



/* trident_detect:
 *  Detects the presence of a Trident card.
 */
static int trident_detect()
{
   int old1, old2;
   int chip;

   old1 = _read_vga_register(0x3C4, 0x0B);
   _write_vga_register(0x3C4, 0x0B, 0);      /* select old mode */
   chip = inportb(0x3C5);                    /* switch to new mode */
   old2 = _read_vga_register(0x3C4, 0xE);    /* read old value */
   outportb(0x3C5, 0);                       /* write zero bank */
   if ((inportb(0x3C5) & 0xF) == 2) {        /* should have changed to 2 */
      outportb(0x3C5, old2^2);
      if (chip <= 2)
	 return FALSE;
      gfx_trident.vid_mem = _vesa_vidmem_check(1024*1024);
      return TRUE;
    }

   outportb(0x3C5, old2);
   _write_vga_register(0x3C4, 0x0B, old1);
   return FALSE;
}



/* trident_init:
 *  Tries to enter the specified graphics mode, and makes a screen bitmap
 *  for it.
 */
static BITMAP *trident_init(int w, int h, int v_w, int v_h)
{
   BITMAP *b;
   int x;

   b = _gfx_mode_set_helper(w, h, v_w, v_h, &gfx_trident, trident_detect, trident_mode_list, NULL);
   if (!b)
      return NULL;

   LOCK_FUNCTION(_trident_bank);

   _alter_vga_register(_crtc, 0x1E, 0x80, 0x80);   /* 17 bit display start */
   _write_vga_register(0x3C4, 0x0B, 0);            /* select old mode */
   _alter_vga_register(0x3C4, 0x0D, 0x10, 0x10);   /* paging mode */
   _read_vga_register(0x3C4, 0x0B);                /* switch to new mode */
   x = _read_vga_register(0x3C4, 0x0E);            /* save 3C4:0E */
   _write_vga_register(0x3C4, 0x0E, 0x80);         /* enable write to 3C4:0C */
   _alter_vga_register(0x3C4, 0x0C, 0x20, 0x20);   /* enable upper 512k */
   _write_vga_register(0x3C4, 0x0E, x);            /* restore 3C4:0E */

   b->read_bank = b->write_bank = _trident_bank;

   return b;
}



/* trident_scroll:
 *  Hardware scrolling for Trident cards.
 */
static int trident_scroll(int x, int y)
{
   long a = x + (y * VIRTUAL_W);

   DISABLE();

   _vsync_out();

   /* write high bit to Trident-specific register */
   _alter_vga_register(_crtc, 0x1E, 0x20, a>>10);

   /* write to normal VGA address registers */
   _write_vga_register(_crtc, 0x0D, (a>>3) & 0xFF);
   _write_vga_register(_crtc, 0x0C, (a>>11) & 0xFF);

   ENABLE();

   _vsync_in();

   return 0;
}

