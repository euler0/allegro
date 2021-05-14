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
 *      Video driver for VGA mode 13h (320x200x256).
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/segments.h>

#include "allegro.h"
#include "internal.h"


static BITMAP *vga_init(int w, int h, int v_w, int v_h);



GFX_DRIVER gfx_vga = 
{
   "VGA", 
   "Mode 13h", 
   vga_init,
   NULL,
   NULL,          /* can't hardware scroll in 13h */
   320, 200,
   TRUE,          /* no need for bank switches */
   0, 0,
   0x10000
};



/* vga_init:
 *  Selects mode 13h and creates a screen bitmap for it.
 */
static BITMAP *vga_init(int w, int h, int v_w, int v_h)
{
   BITMAP *b;
   __dpmi_regs r;

   if ((w != 320) || (h != 200) || (v_w > 320) || (v_h > 200)) {
      strcpy(allegro_error, "Not a VGA mode");
      return NULL;
   }

   b = _make_bitmap(320, 200, 0xA0000, &gfx_vga);
   if (!b)
      return NULL;

   r.x.ax = 0x13;
   __dpmi_int(0x10, &r);

   return b;
}


