/*----------------------------------------------------------------------------*/
/*                         Allegro Library example file.                      */
/*                                                                            */
/* This example demonstrates how to use pcx files, palletes and stretch blits.*/
/* It loads a pcx file, sets its pallete and does some random stretch_blits.  */
/*----------------------------------------------------------------------------*/
/* by Grzegorz "LUD0" Ludorowski (pajonk@ajax.umcs.lublin.pl)                 */
/*                               (http://ajax.umcs.lublin.pl/~pajonk)         */
/*----------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <dir.h>
#include <pc.h>
#include "allegro.h"

/* Declaration of a user pallete where pallete of loaded pcx will be stored.  */
PALLETE my_pallete;
/* Pointer to a buffer, where pcx file will be stored.                        */
BITMAP *scr_buffer;
/* Name of pcx file.                                                          */
/* w-screen width, h-screen height, card-card type (defined in allegro.h)     */
int card, w, h;
/* Place to store set_gfx_mode () output. -1 means error */
int err_vmode;

/* Begin of main routine                                 */
int main (int argc, char *argv[])
{
   char pcx_name[80];
   strcpy(pcx_name, argv[0]);
   strcpy(get_filename(pcx_name), "mysha.pcx");
   printf ("This example demonstrates how to use pcx files, palletes and stretch blits.\n");
   printf ("It loads a pcx file, sets its pallete and does some random stretch blits.\n");
   printf ("Don't worry - it's VERY slowed down using vsync()\n");
   printf ("Hit any key to begin.\n");
   /* Wait for a key (Allegro not installed yet, so I had to use standard   */
   /* procedure kbhit () ).                                                 */
   while ( !kbhit() ) {}
   /* Check if pcx file exists in current directory.                        */
   if ( (file_exists (pcx_name, 32, NULL) ) == 0)
   {
   printf ("File %s not found. Exiting",pcx_name);
   return 0;
   }

   allegro_init();              /* Initialise all Allegro stuff       */
   install_keyboard();          /* Install keyboard interrupt handler */
   /* Removes everything from keyboard's buffer                       */
   clear_keybuf ();
   card = GFX_VGA;
   w = 320;
   h = 200;

   /* Sets 320x200x256 mode */
   err_vmode = set_gfx_mode(card, w, h, 0, 0);
   /* Checks if any error occured during screen mode initialization   */
   if ( err_vmode != 0 )
   {
    printf("Error setting graphics mode\n%s\n\n", allegro_error);
   /* Shut down Allegro          */
    allegro_exit ();
    return 1;
   }
   /* Loads pcx file into buffer */
   scr_buffer = load_pcx (pcx_name,my_pallete);
   /* Sets pcx's pallete         */
   set_pallete (my_pallete);
   blit (scr_buffer, screen, 0,0,0,0,320,200);

   while (!keypressed () )
   {
   /* Blit until key pressed */
   stretch_blit (scr_buffer, screen, 0,0,random() & 320 ,random () & 200,random () & 320, random () & 200, random () & 320, random () &200);
   /* Waits for vertical retrace */
   vsync ();
   }
   /* Destroys bitmap buffer */
   destroy_bitmap(scr_buffer);
   /* Shuts down Allegro     */
   allegro_exit();
   /* Exit program           */
   return 0;
}
