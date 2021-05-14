/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates how to get mouse input.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


void main()
{
   BITMAP *custom_cursor;
   char msg[80];
   int c;

   allegro_init();
   install_keyboard(); 
   install_timer();
   install_mouse();
   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);
   set_pallete(desktop_pallete);

   /* the mouse is accessed via the variables mouse_x, mouse_y, and mouse_b */
   do {
      sprintf(msg, "mouse_x=%-4d", mouse_x);
      textout(screen, font, msg, 16, 16, 255);

      sprintf(msg, "mouse_y=%-4d", mouse_y);
      textout(screen, font, msg, 16, 32, 255);

      if (mouse_b & 1)
	 textout(screen, font, "left button is pressed ", 16, 48, 255);
      else
	 textout(screen, font, "left button not pressed", 16, 48, 255);

      if (mouse_b & 2)
	 textout(screen, font, "right button is pressed ", 16, 64, 255);
      else
	 textout(screen, font, "right button not pressed", 16, 64, 255);

      if (mouse_b & 4)
	 textout(screen, font, "middle button is pressed ", 16, 80, 255);
      else
	 textout(screen, font, "middle button not pressed", 16, 80, 255);

   } while (!keypressed());

   clear_keybuf();

   /*  To display a mouse pointer, call show_mouse(). There are several 
    *  things you should be aware of before you do this, though. For one,
    *  it won't work unless you call install_timer() first. For another,
    *  you must never draw anything onto the screen while the mouse
    *  pointer is visible. So before you draw anything, be sure to turn 
    *  the mouse off with show_mouse(NULL), and turn it back on again when
    *  you are done.
    */
   clear(screen);
   textout_centre(screen, font, "Press a key to change cursor", SCREEN_W/2, SCREEN_H/2, 255);
   show_mouse(screen);
   readkey();
   show_mouse(NULL);

   /* create a custom mouse cursor bitmap... */
   custom_cursor = create_bitmap(32, 32);
   clear(custom_cursor); 
   for (c=0; c<8; c++)
      circle(custom_cursor, 16, 16, c*2, c);

   /* select the custom cursor and set the focus point to the middle of it */
   set_mouse_sprite(custom_cursor);
   set_mouse_sprite_focus(16, 16);

   clear(screen);
   textout_centre(screen, font, "Press a key to quit", SCREEN_W/2, SCREEN_H/2, 255);
   show_mouse(screen);
   readkey();
   show_mouse(NULL);

   destroy_bitmap(custom_cursor);

   exit(0);
}
