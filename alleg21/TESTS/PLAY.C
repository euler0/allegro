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
 *      Sound code test program for the Allegro library.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <ctype.h>
#include <go32.h>
#include <sys/farptr.h>

#include "allegro.h"


void usage() __attribute__ ((noreturn));


void usage()
{
   printf("\nSound code test program for Allegro " VERSION_STR);
   printf("\nBy Shawn Hargreaves, " DATE_STR "\n\n");
   printf("Usage: play [digital driver [midi driver]] files.(wav|mid)\n");

   printf("\nDigital drivers:\n");
   printf("\t0: none         1: SB (autodetect breed)\n");
   printf("\t2: SB 1.0       3: SB 1.5\n");
   printf("\t4: SB 2.0       5: SB Pro\n");
   printf("\t6: SB16\n");

   printf("\nMidi drivers:\n");
   printf("\t0: none         1: Adlib (autodetect OPL version)\n");
   printf("\t2: OPL2         3: Dual OPL2 (SB Pro-1)\n");
   printf("\t4: OPL3         5: SB MIDI interface\n");
   printf("\t6: MPU-401\n");

   printf("\nIf you don't specify the card, Allegro will auto-detect\n");

   exit(1);
}



void quiet_put(char *buf, int line)
{
   /* some SB cards produce loads of static if we sit there in a tight 
    * loop calling the BIOS text output routines, so this method is required.
    */

   int c;
   long addr;

   _farsetsel(_dos_ds);
   addr = 0xb8000 + MID(0, line, 24) * 160;

   for (c=0; buf[c]; c++) {
      _farnspokeb(addr, buf[c]);
      addr += 2;
   }

   while (c++ < 80) {
      _farnspokeb(addr, 0);
      addr += 2;
   }
}



void main(int argc, char *argv[])
{
   int digicard = -1;
   int midicard = -1;
   int k = 0;
   int vol = 255;
   int pan = 128;
   int freq = 1000;
   char buf[80];
   void *item[9];
   int is_midi[9];
   int item_count = 0;
   int i;
   int line;
   long old_midi_pos;

   if (argc < 2)
      usage();

   if ((argc > 2) && (isdigit(argv[1][0]))) {
      digicard = atoi(argv[1]);
      if ((argc > 3) && (isdigit(argv[2][0]))) {
	 midicard = atoi(argv[2]);
	 i = 3;
      }
      else
	 i = 2;
   }
   else
      i = 1;

   allegro_init();
   install_timer();
   install_keyboard();

   if (install_sound(digicard, midicard, argv[0]) != 0) {
      printf("\nError initialising sound system\n%s\n", allegro_error);
      allegro_exit();
      exit(1);
   }

   printf("\nDigital sound driver: %s\n", digi_driver->name);
   printf("Description: %s\n\n", digi_driver->desc);
   printf("Midi music driver: %s\n", midi_driver->name);
   printf("Description: %s\n\n", midi_driver->desc);

   while ((i < argc) && (item_count < 9)) {
      if (stricmp(get_extension(argv[i]), "wav") == 0) {
	 item[item_count] = load_sample(argv[i]);
	 is_midi[item_count] = 0;
      }
      else if (stricmp(get_extension(argv[i]), "mid") == 0) {
	 item[item_count] = load_midi(argv[i]);
	 is_midi[item_count] = 1;
      }
      else {
	 printf("Unknown file type '%s'\n", argv[i]);
	 goto get_out;
      }

      if (!item[item_count]) {
	 printf("Error reading %s\n", argv[i]);
	 goto get_out;
      }

      printf("%d: %s\n", ++item_count, argv[i]);
      i++;
   }

   printf("\nPress a number 1-9 to trigger a sample or midi file\n");
   printf("v/V changes sfx volume, p/P changes sfx pan, and f/F changes sfx frequency\n\n\n");

   line = wherey() - 2;

   k = '1';      /* start sound automatically */

   do {
      switch (k) {

	 case 'v':
	    vol -= 8;
	    if (vol < 0)
	       vol = 0;
	    break;

	 case 'V':
	    vol += 8;
	    if (vol > 255)
	       vol = 255;
	    break;

	 case 'p':
	    pan -= 8;
	    if (pan < 0)
	       pan = 0;
	    break;

	 case 'P':
	    pan += 8;
	    if (pan > 255)
	       pan = 255;
	    break;

	 case 'f':
	    freq -= 8;
	    if (freq < 1)
	       freq = 1;
	    break;

	 case 'F':
	    freq += 8;
	    break;

	 case '0':
	    play_midi(NULL, FALSE);
	    break;

	 default:
	    if ((k >= '1') && (k < '1'+item_count)) {
	       k -= '1';
	       if (is_midi[k])
		  play_midi((MIDI *)item[k], TRUE);
	       else
		  play_sample((SAMPLE *)item[k], vol, pan, freq, FALSE);
	    }
	    break;
      }

      old_midi_pos = midi_pos;

      sprintf(buf, "midi pos: %ld   vol: %d   pan: %d   freq: %d",
						midi_pos, vol, pan, freq);
      quiet_put(buf, line);

      do {
      } while ((!keypressed()) && (midi_pos == old_midi_pos));

      if (keypressed())
	 k = readkey() & 0xFF;
      else
	 k = 0;

   } while (k != 27);

   printf("\n");

   get_out:

   for (i=0; i<item_count; i++)
      if (is_midi[i])
	 destroy_midi((MIDI *)item[i]);
      else
	 destroy_sample((SAMPLE *)item[i]);

   allegro_exit();
   exit(0);
}

