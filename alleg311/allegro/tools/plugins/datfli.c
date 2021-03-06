/*         ______   ___    ___ 
 *        /\  _  \ /\_ \  /\_ \ 
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Grabber plugin for managing FLIC animation objects.
 *
 *      By Shawn Hargreaves.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "allegro.h"
#include "../datedit.h"



/* creates a new FLIC object */
static void *makenew_fli(long *size)
{
   char *v = malloc(1);

   *v = 0;
   *size = 1;

   return v;
}



/* displays a FLIC object in the grabber object view window */
static void plot_fli(DATAFILE *dat, int x, int y)
{
   textout(screen, font, "Double-click in the item list to play it", x, y+32, gui_fg_color);
}



/* callback to quit out of the FLI player */
static int fli_stopper()
{
   if ((keypressed()) || (mouse_b))
      return 1;
   else
      return 0;
}



/* handles double-clicking on a FLIC object in the grabber */
static int view_fli(DATAFILE *dat)
{
   show_mouse(NULL);
   clear_to_color(screen, gui_mg_color);
   play_memory_fli(dat->dat, screen, TRUE, fli_stopper);
   do {
   } while (mouse_b);
   clear_keybuf();
   set_pallete(datedit_current_palette);
   show_mouse(screen);
   return D_REDRAW;
}



/* plugin interface header */
DATEDIT_OBJECT_INFO datfli_info =
{ 
   DAT_FLI, 
   "FLI/FLC animation", 
   NULL,
   makenew_fli,
   NULL,
   plot_fli,
   view_fli,
   NULL
};



DATEDIT_GRABBER_INFO datfli_grabber =
{ 
   DAT_FLI, 
   "fli;flc",
   "fli;flc",
   NULL,
   NULL
};

