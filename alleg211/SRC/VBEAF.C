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
 *      Video driver using the VBE/AF hardware accelerator API.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/movedata.h>
#include <sys/farptr.h>
#include <sys/segments.h>

#include "allegro.h"
#include "internal.h"


static char vbeaf_desc[80] = "";

static BITMAP *vbeaf_init(int w, int h, int v_w, int v_h);
static void vbeaf_exit(BITMAP *b);
static void vbeaf_vsync();
static int vbeaf_scroll(int x, int y);
static void vbeaf_set_pallete_range(PALLETE p, int from, int to, int vsync);



GFX_DRIVER gfx_vbeaf = 
{
   "VBE/AF",
   vbeaf_desc,
   vbeaf_init,
   vbeaf_exit,
   vbeaf_scroll,
   vbeaf_vsync,
   vbeaf_set_pallete_range,
   0, 0, FALSE, 0, 0, 0
};



/* vbeaf_init:
 *  Tries to enter the specified graphics mode, and makes a screen bitmap
 *  for it.
 */
static BITMAP *vbeaf_init(int w, int h, int v_w, int v_h)
{
   strcpy(allegro_error, "VBE/AF not yet implemented");
   return NULL;
}



/* vbeaf_scroll:
 *  Hardware scrolling routine.
 */
static int vbeaf_scroll(int x, int y)
{
   return 0;
}



/* vbeaf_vsync:
 *  VBE/AF vsync routine, needed for cards that don't emulate the VGA 
 *  blanking registers. VBE/AF doesn't provide a vsync function, but we 
 *  can emulate it by altering the pallete with the vsync flag set.
 */
static void vbeaf_vsync()
{
   vbeaf_set_pallete_range(_current_pallete, 0, 0, TRUE);
}



/* vbeaf_set_pallete_range:
 *  Uses VBE/AF functions to set the pallete.
 */
static void vbeaf_set_pallete_range(PALLETE p, int from, int to, int vsync)
{
}



/* vbeaf_exit:
 *  Shuts down the VBE/AF driver.
 */
static void vbeaf_exit(BITMAP *b)
{
}

