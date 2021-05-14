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
 *      Driver for Video 7 graphics cards.
 *
 *      Contributed by Peter Monks.
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


static BITMAP *video7_init(int w, int h, int v_w, int v_h);
static int video7_scroll(int x, int y);



GFX_DRIVER gfx_video7 =
{
   "Video-7",
   "",
   video7_init,
   NULL,
   video7_scroll,
   _vga_vsync,
   _vga_set_pallete_range,
   0, 0, FALSE,
   0x10000,       /* 64k banks */
   0x10000,       /* 64k granularity */
   0
};



static GFX_MODE_INFO video7_mode_list[] =
{
   {  640,  400,  0x66,  0x6F05  },
   {  640,  480,  0x67,  0x6F05  },
   {  720,  540,  0x68,  0x6F05  },
   {  800,  600,  0x69,  0x6F05  },
   {  1024, 768,  0x6A,  0x6F05  },
   {  0,    0,    0,     0       }
};



/* video7_detect:
 *  Detects the presence of a video7 card.
 */
static int video7_detect()
{
   __dpmi_regs r;
   int v;
   int old;

   old = _read_vga_register(0x3C4, 6);
   _write_vga_register(0x3C4, 6, 0xEA);        /* enable extensions */

   r.x.ax = 0x6F00;                            /* are extensions enabled? */
   __dpmi_int(0x10, &r);

   if ((r.x.bx != 0x5637) && (r.x.bx != 0x4850)) {
      _write_vga_register(0x3C4, 6, old);
      return FALSE;
   }

   v = (_read_vga_register(0x3C4, 0x8F)<<8) + _read_vga_register(0x3C4, 0x8E);

   switch (v) {
      case 0x7140 ... 
	   0x714F:   gfx_video7.desc = "V7 208A";     break;
      case 0x7151:   gfx_video7.desc = "V7 208B";     break;
      case 0x7152:   gfx_video7.desc = "V7 208CD";    break;
      case 0x7760:   gfx_video7.desc = "V7 216BC";    break;
      case 0x7763:   gfx_video7.desc = "V7 216D";     break;
      case 0x7764:   gfx_video7.desc = "V7 216E";     break;
      case 0x7765:   gfx_video7.desc = "V7 216F";     break;

      default:
	 return FALSE;
   }

   r.x.ax = 0x6F07; 
   __dpmi_int(0x10, &r);
   gfx_video7.vid_mem = (r.h.ah & 0x7F) * 256 * 1024;

   return TRUE;
}



/* video7_init:
 *  Tries to enter the specified graphics mode, and makes a screen bitmap
 *  for it.
 */
static BITMAP *video7_init(int w, int h, int v_w, int v_h)
{
   BITMAP *b;

   b = _gfx_mode_set_helper(w, h, v_w, v_h, &gfx_video7, video7_detect, video7_mode_list, NULL);
   if (!b)
      return NULL;

   LOCK_FUNCTION(_video7_bank);

   _write_vga_register(0x3C4, 6, 0xEA);            /* enable extensions */
   _alter_vga_register(0x3C4, 0xE0, 0x80, 0);      /* disable r/w banks */

   b->write_bank = b->read_bank = _video7_bank;

   return b;
}



/* video7_scroll:
 *  Hardware scrolling for Video7 HT216 cards.
 */
static int video7_scroll(int x, int y)
{
   long a = x + (y * VIRTUAL_W);

   DISABLE();

   _vsync_out();

   /* write high bits to Video7 registers */
   _alter_vga_register(0x3C4, 0xF6, 0x70, a>>14);

   /* write to normal VGA address registers */
   _write_vga_register(_crtc, 0x0D, (a>>2) & 0xFF);
   _write_vga_register(_crtc, 0x0C, (a>>10) & 0xFF);

   ENABLE();

   /* write low 2 bits to VGA horizontal pan register */
   _write_hpp(a&3);

   return 0;
}

