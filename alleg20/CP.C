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
 *      File compression test program for the Allegro library.
 *
 *      See readme.txt for copyright information.
 */


#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


void usage()
{
   printf("\nFile compression test program for Allegro " VERSION_STR);
   printf("\nBy Shawn Hargreaves, " DATE_STR "\n\n");
   printf("Usage: 'cp <in> <out>' copies file <in> to <out>\n");
   printf("       'cp p <in> <out>' packs file <in> into <out>\n");
   printf("       'cp u <in> <out>' unpacks file <in> into <out>\n");
}


void err(char *s1, char *s2)
{
   printf("\nError %d", errno);

   if (s1)
      printf(": %s", s1);

   if (s2)
      printf(s2);

   printf("\n");

   if (errno==EDOM)
      printf("Not a packed file\n");

   exit(1);
}


void main(int argc, char *argv[])
{
   char *f1, *f2;
   char *m1 = F_READ;
   char *m2 = F_WRITE;
   long s1, s2;
   char *t;
   PACKFILE *in, *out;
   int c;

   if (argc==3) {
      f1 = argv[1];
      f2 = argv[2];
      t = "Copy";
   }
   else {
      if (argc==4) {
	 if (argv[1][1]==0) {
	    f1 = argv[2];
	    f2 = argv[3];
	    if ((argv[1][0]=='p') || (argv[1][0]=='P')) {
	       m2 = F_WRITE_PACKED;
	       t = "Pack";
	    }
	    else
	       if ((argv[1][0]=='u') || (argv[1][0]=='U')) {
		  m1 = F_READ_PACKED;
		  t = "Unpack";
	       }
	       else {
		  usage();
		  exit(-1);
	       }
	 }
	 else {
	    usage();
	    exit(-1);
	 }
      }
      else {
	 usage();
	 exit(-1);
      }
   }

   s1 = file_size(f1);

   in = pack_fopen(f1,m1);
   if (!in)
      err("can't open ", f1);

   out = pack_fopen(f2,m2);
   if (!out) {
      delete_file(f2);
      pack_fclose(in);
      err("can't create ", f2);
   }

   printf("\n%sing %s to %s:\n", t, f1, f2);

   c = pack_getc(in);
   while (c != EOF) {
      if (pack_putc(c, out) != c)
	 break;
      c = pack_getc(in);
   }

   pack_fclose(in);
   pack_fclose(out);

   if (errno) {
      delete_file(f2);
      err(NULL, NULL);
   }

   s2 = file_size(f2);
   printf("\nInput size: %ld\nOutput size: %ld\n%ld%%\n",
	  s1, s2, (s2*100+(s1>>1))/s1);

   exit(0);
}
