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
 *      FLI player test program for the Allegro library.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "allegro.h"


void usage() __attribute__ ((noreturn));


void usage()
{
   printf("\nFLI player test program for Allegro " VERSION_STR);
   printf("\nBy Shawn Hargreaves, " DATE_STR "\n\n");
   printf("Usage: playfli [options] filename.(fli|flc)\n\n");
   printf("Options:\n");
   printf("\t'-loop' cycles the animation until a key is pressed\n");
   printf("\t'-mode screen_w screen_h' sets the screen mode\n");

   exit(1);
}



int key_checker()
{
   if (keypressed())
      return 1;
   else
      return 0;
}



void main(int argc, char *argv[])
{
   int w = 320;
   int h = 200;
   int loop = FALSE;
   int c, ret;

   if (argc < 2)
      usage();

   for (c=1; c<argc-1; c++) {
      if (stricmp(argv[c], "-loop") == 0)
	 loop = TRUE;
      else if ((stricmp(argv[c], "-mode") == 0) && (c<argc-3)) {
	 w = atoi(argv[c+1]);
	 h = atoi(argv[c+2]);
	 c += 2;
      }
      else
	 usage();
   }

   allegro_init();
   install_keyboard();
   install_timer();

   if (set_gfx_mode(GFX_AUTODETECT, w, h, 0, 0) != 0) {
      allegro_exit();
      printf("\nError setting graphics mode\n%s\n", allegro_error);
      exit(1);
   }

   ret = play_fli(argv[c], screen, loop, key_checker);

   allegro_exit();

   if (ret < 0)
      printf("Error playing FLI file\n");

   exit(0);
}

