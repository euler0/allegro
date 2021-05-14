/*
 *      Example program for the Allegro library, by Grzegorz Adam Hankiewicz
 *
 *      gregorio@jet.es         -       http://web.jet.es/gregorio
 *
 *      This program uses the Allegro library to detect and read the value
 *      of a joystick. The output of the program is a small target sight
 *      on the screen which you can move.
 *      At the same time the program will tell you what you are doing with
 *      the joystick (moving or firing).
 */

#include <stdio.h>
#include <allegro.h>

int main()
{
   BITMAP *bmp;            /* We create a pointer to a virtual screen */
   int x=160, y=100;       /* These will be used to show the target sight */
   int analogmode;

   allegro_init();         /* You NEED this man! ;-) */
   install_keyboard();     /* Ahh... read the docs. I will explain only
			    * Joystick specific routines
			    */

   printf("\nWelcome to the Joystick test.\n");
   printf("Press a key to continue. Press ESC to exit now.\n");

   clear_keybuf();
   do {
   } while (!keypressed());
   if ((readkey()&0xFF) == 27)
      exit(0);

   printf("\nPlease center the Joystick and press a key.\n");

   clear_keybuf();
   do {
   } while (!keypressed());
   if ((readkey()&0xFF) == 27)
      exit(0);

   /* We have pleased the user to center the joystick. Now we can initialise
    * all the Joystick routines. You ALWAYS need to call this function first
    */

   joy_type = JOY_TYPE_4BUTTON;  /* This is cheating, hehehe. We declare
				  * the user has a 4 button joy to read
				  * the 3rd and 4th button. Maybe the
				  * user doesn't have one, but hey, then
				  * he won't notice we are cheating ;)
				  */

   initialise_joystick();

   printf("\nNow press 'D' to use a digital joystick or 'A' if it's analog. Esc to exit\n");

   clear_keybuf();
   do {
      if (key[KEY_ESC]) exit(0);
   } while (!key[KEY_D] && !key[KEY_A]);

   analogmode = key[KEY_A];

   if (analogmode) {
      /* Analog Joysticks return ALWAYS a value which ranges from -128 to 128.
       * Every joy can have this values a little bit 'wrong' and that's what
       * calibration is for, to calibrate those values. 
       */

      printf("\nAhhhh! Analog Joysticks need more calibration.\n");
      printf("Pull your Joystick to the top left and press a key.\n");

      clear_keybuf();
      do {
      } while (!keypressed());
      if ((readkey()&0xFF) == 27)
	 exit(0);

      calibrate_joystick_tl(); /* We calibrate the top left corner */

      printf("\nNow take it to the bottom right and press a key again.\n");
      clear_keybuf();
      do {
      } while (!keypressed());
      if ((readkey()&0xFF) == 27)
	 exit(0);

      calibrate_joystick_br(); /* We calibrate the bottom right corner */
   }

   set_gfx_mode(GFX_AUTODETECT, 320, 200, 0, 0);
   drawing_mode(DRAW_MODE_XOR, 0, 0, 0);
   clear_keybuf();

   bmp = create_bitmap(320, 200);
   clear(bmp);

   do {
      poll_joystick(); /* We HAVE to do this to read the joy */

      clear(bmp);

      if (analogmode)
	 textout_centre(bmp, font, "Analog Joystick selected", 160, 160, 15);
      else
	 textout_centre(bmp, font, "Digital Joystick selected", 160, 160, 15);

      textout_centre(bmp, font, "Move the Joystick all around", 160, 170, 15);
      textout_centre(bmp, font, "Press any key to exit", 160, 180, 15);
      textout_centre(bmp, font, "Made by Grzegorz Adam Hankiewicz,1997", 160, 190, 15);

      /* If we detect the buttons, we print a message on the screen */
      if (joy_b1) textout_centre(bmp, font, "Button 1 pressed", 160, 0,  15);
      if (joy_b2) textout_centre(bmp, font, "Button 2 pressed", 160, 10, 15);
      if (joy_b3) textout_centre(bmp, font, "Button 3 pressed", 160, 20, 15);
      if (joy_b4) textout_centre(bmp, font, "Button 4 pressed", 160, 30, 15);

      if (!analogmode) {
	 /* Now we have to check individually every possible movement
	  * and actualize the coordinates of the target sight.
	  * Of course we will be polite and tell the user what he is
	  * doing.
	  */
	 if (joy_left && x>0) {
	    x--;
	    textout_centre(bmp, font, "Left", 120, 100, 15);
	 }
	 if (joy_right && x<319) {
	    x++;
	    textout_centre(bmp, font, "Right", 200, 100, 15);
	 }
	 if (joy_up && y>0) {
	    y--;
	    textout_centre(bmp, font, "Up", 160, 70, 15);
	 }
	 if (joy_down && y<199) {
	    y++;
	    textout_centre(bmp, font, "Down", 160, 130, 15);
	 }
      }
      else {
	 /* Yeah! Remember the 'ifs' of the digital part? This looks
	  * much better, only 2 lines. Now look:
	  * joy_x and joy_y return a value between -128 and 128. If we
	  * add the value to the coords, it could be enough. But then
	  * we would move the target sight 128 pixels every time if the
	  * user pulled the joy to the extremes !!!
	  * We have to divide the number, and then we will get a slower
	  * movement. Notice : The higher the number you divide, the
	  * slower the movement will be.
	  * If you want to make it quicker, just add a pixel or two to
	  * the end values. Something like...
	  * x += (joy_x/30) + 2;
	  */

	 x += joy_x/30;
	 y += joy_y/30;

	 /* By checking if the values were positive or negative, we
	  * can know in which the direction the user pulled the joy.
	  * We will also print it
	  */

	 if (joy_x/30 < 0) textout_centre(bmp, font, "Left", 120, 100, 15);
	 if (joy_x/30 > 0) textout_centre(bmp, font, "Right", 200, 100, 15);
	 if (joy_y/30 < 0) textout_centre(bmp, font, "Up", 160, 70, 15);
	 if (joy_y/30 > 0) textout_centre(bmp, font, "Down", 160, 130, 15);

	 /* WARNING! An analog joystick can move more than 1 pixel at
	  * a time and the checks we did with the digital part don't
	  * work any longer because the steps of the target sight could
	  * 'jump' over the limits.
	  * To avoid this, we just check if the target sight has gone
	  * out of the screen. If yes, we put it back at the border
	  */

	 if (x>319) x=319;
	 if (x<0)   x=0;
	 if (y<0)   y=0;
	 if (y>199) y=199;
      }

      /* This draws the target sight. */
      circle(bmp, x, y, 5, 15);
      putpixel(bmp, x, y, 15);
      putpixel(bmp, x+1, y, 15);
      putpixel(bmp, x, y+1, 15);
      putpixel(bmp, x-1, y, 15);
      putpixel(bmp, x, y-1, 15);
      putpixel(bmp, x+5, y, 15);
      putpixel(bmp, x, y+5, 15);
      putpixel(bmp, x-5, y, 15);
      putpixel(bmp, x, y-5, 15);

      blit(bmp, screen, 0, 0, 0, 0, 320, 200);

   } while (!keypressed());

   /* If you press a key, you will be again in the peaceful DOS. */
   return 0;
}
