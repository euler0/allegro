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
 *      Main test program for the Allegro library.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dir.h>
#include <errno.h>

#include "allegro.h"


int mode = DRAW_MODE_SOLID;
char mode_string[80];

#define TIME_SPEED   2

BITMAP *global_sprite = NULL;
RLE_SPRITE *global_rle_sprite = NULL;
COMPILED_SPRITE *global_compiled_sprite = NULL;

#define NUM_PATTERNS    8
BITMAP *pattern[NUM_PATTERNS];

char gfx_specs[80];
char mouse_specs[80];

char buf[160];

int xoff, yoff;

long tm = 0;        /* counter, incremented once a second */
int _tm = 0;


void tm_tick()
{
   if (++_tm >= 100) {
      _tm = 0;
      tm++;
   }
}

END_OF_FUNCTION(tm_tick);



void show_time(long t, BITMAP *bmp, int y)
{
   int cf, cl, ct, cr, cb;

   cf = bmp->clip;
   cl = bmp->cl;
   cr = bmp->cr;
   ct = bmp->ct;
   cb = bmp->cb;

   sprintf(buf, "%ld per second", t / TIME_SPEED);
   set_clip(bmp, 0, 0, SCREEN_W, SCREEN_H);
   textout_centre(bmp, font, buf, SCREEN_W/2, y, 15);
   bmp->clip = cf;
   bmp->cl = cl;
   bmp->cr = cr;
   bmp->ct = ct;
   bmp->cb = cb;
}



void message(char *s)
{
   textout_centre(screen, font, s, SCREEN_W/2, 6, 15);
   textout_centre(screen, font, "Press a key or mouse button", SCREEN_W/2, SCREEN_H-10, 15);
}



int next()
{
   if (keypressed()) {
      clear_keybuf();
      return TRUE;
   }

   if (mouse_b) {
      retrace_proc = NULL;
      do {
      } while (mouse_b);
      return TRUE;
   }

   return FALSE;
}



BITMAP *make_sprite()
{
   BITMAP *b;

   solid_mode();
   b = create_bitmap(32, 32);
   clear(b);
   circlefill(b, 16, 16, 8, 2);
   circle(b, 16, 16, 8, 1);
   line(b, 0, 0, 31, 31, 3);
   line(b, 31, 0, 0, 31, 3);
   text_mode(-1);
   textout(b, font, "Test", 1, 12, 15);
   return b;
}



void make_patterns()
{
   int c;

   pattern[0] = create_bitmap(2, 2);
   clear(pattern[0]);
   putpixel(pattern[0], 0, 0, 255);

   pattern[1] = create_bitmap(2, 2);
   clear(pattern[1]);
   putpixel(pattern[1], 0, 0, 255);
   putpixel(pattern[1], 1, 1, 255);

   pattern[2] = create_bitmap(4, 4);
   clear(pattern[2]);
   vline(pattern[2], 0, 0, 4, 255);
   hline(pattern[2], 0, 0, 4, 255);

   pattern[3] = create_bitmap(4, 4);
   clear(pattern[3]);
   line(pattern[3], 0, 3, 3, 0, 255);

   pattern[4] = create_bitmap(16, 16);
   clear(pattern[4]);
   for (c=0; c<32; c+=2)
      circle(pattern[4], 8, 8, c, c);

   pattern[5] = create_bitmap(16, 16);
   clear(pattern[5]);
   for (c=0; c<16; c+=2) {
      line(pattern[5], 8, 8, 0, c, c);
      line(pattern[5], 8, 8, c, 0, c);
      line(pattern[5], 8, 8, 15, c, c);
      line(pattern[5], 8, 8, c, 15, c);
   }

   pattern[6] = create_bitmap(16, 16);
   clear(pattern[6]);
   for (c=0; c<16; c++)
      hline(pattern[6], 0, c, 16, c);

   pattern[7] = create_bitmap(64, 8);
   clear(pattern[7]);
   text_mode(0);
   textout(pattern[7], font, "PATTERN!", 0, 0, 255);
}



void getpix_demo()
{
   int c;

   clear(screen); 
   message("getpixel test");

   for (c=0; c<16; c++)
      rectfill(screen, xoff+100+((c&3)<<5), yoff+50+((c>>2)<<5),
		       xoff+120+((c&3)<<5), yoff+70+((c>>2)<<5), c);

   while (!next()) {
      rest(20);
      show_mouse(NULL);
      c = getpixel(screen, mouse_x-2, mouse_y-2);
      sprintf(buf, " %d ", c);
      textout_centre(screen, font, buf, SCREEN_W/2, yoff+24, 15);
      show_mouse(screen);
   }

   show_mouse(NULL);
}



void putpix_test(int xpos, int ypos)
{
   int c = 0;
   int x, y;

   for (c=0; c<16; c++)
      for (x=0; x<16; x+=2)
	 for (y=0; y<16; y+=2)
	    putpixel(screen, xpos+((c&3)<<4)+x, ypos+((c>>2)<<4)+y, c);
}



void hline_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++)
      hline(screen, xpos+48-c*3, ypos+c*3, xpos+48+c*3, c);
}



void vline_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++)
      vline(screen, xpos+c*4, ypos+36-c*3, ypos+36+c*3, c); 
}



void line_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++) {
      line(screen, xpos+32, ypos+32, xpos+32+((c-8)<<2), ypos, c%15+1);
      line(screen, xpos+32, ypos+32, xpos+32-((c-8)<<2), ypos+64, c%15+1);
      line(screen, xpos+32, ypos+32, xpos, ypos+32-((c-8)<<2), c%15+1);
      line(screen, xpos+32, ypos+32, xpos+64, ypos+32+((c-8)<<2), c%15+1);
   }
}



void rectfill_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++)
      rectfill(screen, xpos+((c&3)*17), ypos+((c>>2)*17),
		       xpos+15+((c&3)*17), ypos+15+((c>>2)*17), c);
}



void triangle_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++)
      triangle(screen, xpos+22+((c&3)<<4), ypos+15+((c>>2)<<4),
		       xpos+13+((c&3)<<3), ypos+7+((c>>2)<<4),
		       xpos+7+((c&3)<<4), ypos+27+((c>>2)<<3), c);
}



void circle_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++)
      circle(screen, xpos+32, ypos+32, c*2, c%15+1);
}



void circlefill_test(int xpos, int ypos)
{
   int c;

   for (c=15; c>=0; c--)
      circlefill(screen, xpos+8+((c&3)<<4), ypos+8+((c>>2)<<4), c, c%15+1);
}



void textout_test(int xpos, int ypos)
{
   text_mode(0);
   textout(screen, font,"This is a", xpos-8, ypos, 1);
   textout(screen, font,"test of the", xpos+3, ypos+10, 1);
   textout(screen, font,"textout", xpos+14, ypos+20, 1);
   textout(screen, font,"function.", xpos+25, ypos+30, 1);

   text_mode(0);
   textout(screen, font,"text_mode(0)", xpos, ypos+48, 2);
   textout(screen, font,"text_mode(0)", xpos+4, ypos+52, 4);

   text_mode(-1);
   textout(screen, font,"text_mode(-1)", xpos, ypos+68, 2);
   textout(screen, font,"text_mode(-1)", xpos+4, ypos+72, 4);
   text_mode(0);
}



void sprite_test(int xpos, int ypos)
{
   int x,y;

   for (y=0;y<82;y++)
      for (x=0;x<82;x+=2)
	 putpixel(screen, xpos+x+(y&1), ypos+y, 8);

   for (x=6; x<64; x+=global_sprite->w+6)
      for (y=6; y<64; y+=global_sprite->w+6)
	 draw_sprite(screen, global_sprite, xpos+x, ypos+y);
}



void rle_sprite_test(int xpos, int ypos)
{
   int x,y;

   for (y=0;y<82;y++)
      for (x=0;x<82;x+=2)
	 putpixel(screen, xpos+x+(y&1), ypos+y, 8);

   for (x=6; x<64; x+=global_sprite->w+6)
      for (y=6; y<64; y+=global_sprite->w+6)
	 draw_rle_sprite(screen, global_rle_sprite, xpos+x, ypos+y);
}



void compiled_sprite_test(int xpos, int ypos)
{
   int x,y;

   for (y=0;y<82;y++)
      for (x=0;x<82;x+=2)
	 putpixel(screen, xpos+x+(y&1), ypos+y, 8);

   for (x=6; x<64; x+=global_sprite->w+6)
      for (y=6; y<64; y+=global_sprite->w+6)
	 draw_compiled_sprite(screen, global_compiled_sprite, xpos+x, ypos+y);
}



void flipped_sprite_test(int xpos, int ypos)
{
   int x, y;

   for (y=0;y<88;y++)
      for (x=0;x<88;x+=2)
	 putpixel(screen, xpos+x+(y&1), ypos+y, 8);

   draw_sprite(screen, global_sprite, xpos+8, ypos+8);
   draw_sprite_h_flip(screen, global_sprite, xpos+48, ypos+8);
   draw_sprite_v_flip(screen, global_sprite, xpos+8, ypos+48);
   draw_sprite_vh_flip(screen, global_sprite, xpos+48, ypos+48);
}



void putpix_demo()
{
   int c = 0;
   int x, y;
   long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & 255) + 32;
      y = (random() & 127) + 40;
      putpixel(screen, xoff+x, yoff+y, c);

      if (mode >= DRAW_MODE_COPY_PATTERN)
	 drawing_mode(mode, pattern[random()%NUM_PATTERNS], 0, 0);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void hline_demo()
{
   int c = 0;
   int x1, x2, y;
   long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x1 = (random() & 255) + 32;
      x2 = (random() & 255) + 32;
      y = (random() & 127) + 40;
      hline(screen, xoff+x1, yoff+y, xoff+x2, c);

      if (mode >= DRAW_MODE_COPY_PATTERN)
	 drawing_mode(mode, pattern[random()%NUM_PATTERNS], 0, 0);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void vline_demo()
{
   int c = 0;
   int x, y1, y2;
   long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & 255) + 32;
      y1 = (random() & 127) + 40;
      y2 = (random() & 127) + 40;
      vline(screen, xoff+x, yoff+y1, yoff+y2, c);

      if (mode >= DRAW_MODE_COPY_PATTERN)
	 drawing_mode(mode, pattern[random()%NUM_PATTERNS], 0, 0);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void line_demo()
{
   int c = 0;
   int x1, y1, x2, y2;
   long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x1 = (random() & 255) + 32;
      x2 = (random() & 255) + 32;
      y1 = (random() & 127) + 40;
      y2 = (random() & 127) + 40;
      line(screen, xoff+x1, yoff+y1, xoff+x2, yoff+y2, c);

      if (mode >= DRAW_MODE_COPY_PATTERN)
	 drawing_mode(mode, pattern[random()%NUM_PATTERNS], 0, 0);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void rectfill_demo()
{
   int c = 0;
   int x1, y1, x2, y2;
   long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x1 = (random() & 255) + 32;
      y1 = (random() & 127) + 40;
      x2 = (random() & 255) + 32;
      y2 = (random() & 127) + 40;
      rectfill(screen, xoff+x1, yoff+y1, xoff+x2, yoff+y2, c);

      if (mode >= DRAW_MODE_COPY_PATTERN)
	 drawing_mode(mode, pattern[random()%NUM_PATTERNS], 0, 0);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void triangle_demo()
{
   int c = 0;
   int x1, y1, x2, y2, x3, y3;
   long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x1 = (random() & 255) + 32;
      x2 = (random() & 255) + 32;
      x3 = (random() & 255) + 32;
      y1 = (random() & 127) + 40;
      y2 = (random() & 127) + 40;
      y3 = (random() & 127) + 40;
      triangle(screen, xoff+x1, yoff+y1, xoff+x2, yoff+y2, xoff+x3, yoff+y3, c);

      if (mode >= DRAW_MODE_COPY_PATTERN)
	 drawing_mode(mode, pattern[random()%NUM_PATTERNS], 0, 0);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void circle_demo()
{
   int c = 0;
   int x, y, r;
   long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & 127) + 92;
      y = (random() & 63) + 76;
      r = (random() & 31) + 16;
      circle(screen, xoff+x, yoff+y, r, c);

      if (mode >= DRAW_MODE_COPY_PATTERN)
	 drawing_mode(mode, pattern[random()%NUM_PATTERNS], 0, 0);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void circlefill_demo()
{
   int c = 0;
   int x, y, r;
   long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & 127) + 92;
      y = (random() & 63) + 76;
      r = (random() & 31) + 16;
      circlefill(screen, xoff+x, yoff+y, r, c);

      if (mode >= DRAW_MODE_COPY_PATTERN)
	 drawing_mode(mode, pattern[random()%NUM_PATTERNS], 0, 0);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void textout_demo()
{
   int c = 0;
   int x, y;
   long ct;

   tm = 0; _tm = 0;
   ct = 0;

   text_mode(0);

   while (!next()) {

      x = (random() & 127) + 40;
      y = (random() & 127) + 40;
      textout(screen, font, "textout test", xoff+x, yoff+y, c);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void sprite_demo()
{
   long ct;
   int x, y;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & 255) + 16;
      y = (random() & 127) + 30;
      draw_sprite(screen, global_sprite, xoff+x, yoff+y);

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void rle_sprite_demo()
{
   long ct;
   int x, y;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & 255) + 16;
      y = (random() & 127) + 30;
      draw_rle_sprite(screen, global_rle_sprite, xoff+x, yoff+y);

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void compiled_sprite_demo()
{
   long ct;
   int x, y;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & 255) + 16;
      y = (random() & 127) + 30;
      draw_compiled_sprite(screen, global_compiled_sprite, xoff+x, yoff+y);

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



int blit_from_screen = FALSE;
int blit_align = FALSE;


void blit_demo()
{
   long ct;
   int x, y;
   int sx, sy;
   BITMAP *b;

   b = create_bitmap(64, 32);
   if (!b) {
      clear(screen);
      textout(screen, font, "Out of memory!", 50, 50, 15);
      destroy_bitmap(b);
      while (!next())
	 ;
      return;
   }

   clear(b);
   circlefill(b, 32, 16, 16, 4);
   circlefill(b, 32, 16, 10, 2);
   circlefill(b, 32, 16, 6, 1);
   line(b, 0, 0, 63, 31, 5);
   line(b, 0, 31, 63, 0, 5);
   rect(b, 8, 4, 56, 28, 3);
   rect(b, 0, 0, 63, 31, 15);

   tm = 0; _tm = 0;
   ct = 0;

   sx = ((SCREEN_W-64) / 2) & 0xFFFC;
   sy = yoff + 32;

   if (blit_from_screen) 
      blit(b, screen, 0, 0, sx, sy, 64, 32);

   while (!next()) {

      x = (random() & 127) + 60;
      y = (random() & 63) + 50;

      if (blit_align)
	 x &= 0xFFFC;

      if (blit_from_screen)
	 blit(screen, screen, sx, sy, xoff+x, yoff+y+24, 64, 32);
      else
	 blit(b, screen, 0, 0, xoff+x, yoff+y, 64, 32);

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }

   destroy_bitmap(b);
}



void misc()
{
   BITMAP *p;
   long ct;
   fixed x, y, z;

   clear(screen);
   textout(screen,font,"Timing some other routines...", xoff+44, 6, 15);

   p = create_bitmap(320, 200);
   if (!p)
      textout(screen,font,"Out of memory!", 16, 50, 15);
   else {
      tm = 0; _tm = 0;
      ct = 0;
      do {
	 clear(p);
	 ct++;
	 if (next())
	    return;
      } while (tm < TIME_SPEED);
      destroy_bitmap(p);
      sprintf(buf,"clear(320x200): %ld per second", ct/TIME_SPEED);
      textout(screen, font, buf, xoff+16, yoff+50, 15);
   }

   x = y = 0;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      z = fmul(x,y);
      x += 1317;
      y += 7143;
      ct++;
      if (next())
	 return;
   } while (tm < TIME_SPEED);

   sprintf(buf, "fmul(): %ld per second", ct/TIME_SPEED);
   textout(screen, font, buf, xoff+16, yoff+60, 15);

   x = y = 0;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      z = fdiv(x,y);
      x += 1317;
      y += 7143;
      if (y==0)
	 y++;
      ct++;
      if (next())
	 return;
   } while (tm < TIME_SPEED);

   sprintf(buf, "fdiv(): %ld per second", ct/TIME_SPEED);
   textout(screen, font, buf, xoff+16, yoff+70, 15);

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = fsqrt(x);
      x += 7361;
      ct++;
      if (next())
	 return;
   } while (tm < TIME_SPEED);

   sprintf(buf, "fsqrt(): %ld per second", ct/TIME_SPEED);
   textout(screen, font, buf, xoff+16, yoff+80, 15);

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = fsin(x);
      x += 4283;
      ct++;
      if (next())
	 return;
   } while (tm < TIME_SPEED);

   sprintf(buf, "fsin(): %ld per second", ct / TIME_SPEED);
   textout(screen, font, buf, xoff+16, yoff+90, 15);

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = fcos(x);
      x += 4283;
      ct++;
      if (next())
	 return;
   } while (tm < TIME_SPEED);

   sprintf(buf, "fcos(): %ld per second", ct / TIME_SPEED);
   textout(screen, font, buf, xoff+16, yoff+100, 15);

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = ftan(x);
      x += 8372;
      ct++;
      if (next())
	 return;
   } while (tm < TIME_SPEED);

   sprintf(buf, "ftan(): %ld per second", ct / TIME_SPEED);
   textout(screen, font, buf, xoff+16, yoff+110, 15);

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = fasin(x);
      x += 5621;
      x &= 0xffff;
      ct++;
      if (next())
	 return;
   } while (tm < TIME_SPEED);

   sprintf(buf, "fasin(): %ld per second", ct / TIME_SPEED);
   textout(screen, font, buf, xoff+16, yoff+120, 15);

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = facos(x);
      x += 5621;
      x &= 0xffff;
      ct++;
      if (next())
	 return;
   } while (tm < TIME_SPEED);

   sprintf(buf,"facos(): %ld per second", ct / TIME_SPEED);
   textout(screen, font, buf, xoff+16, yoff+130, 15);

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = fatan(x);
      x += 7358;
      ct++;
      if (next())
	 return;
   } while (tm < TIME_SPEED);

   sprintf(buf, "fatan(): %ld per second", ct / TIME_SPEED);
   textout(screen, font, buf, xoff+16, yoff+140, 15);

   x = 1, y = 2;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      z = fatan2(x, y);
      x += 5621;
      y += 7335;
      ct++;
      if (next())
	 return;
   } while (tm < TIME_SPEED);

   sprintf(buf, "fatan2(): %ld per second", ct / TIME_SPEED);
   textout(screen, font, buf, xoff+16, yoff+150, 15);

   textout(screen, font, "Press a key or mouse button", xoff+52, SCREEN_H-10, 15);

   while (!next())
      ;
}



int mouse_proc()
{
   show_mouse(NULL);
   text_mode(0);
   clear(screen);
   textout_centre(screen, font, "Mouse test", SCREEN_W/2, 6, 15);
   textout_centre(screen, font, "Press a key", SCREEN_W/2, SCREEN_H-10, 15);

   do {
      sprintf(buf, "X=%-4d Y=%-4d", mouse_x, mouse_y);
      if (mouse_b & 1)
	 strcat(buf," left");
      else
	 strcat(buf,"     ");
      if (mouse_b & 4)
	 strcat(buf," middle");
      else
	 strcat(buf,"       ");
      if (mouse_b & 2)
	 strcat(buf," right"); 
      else
	 strcat(buf,"      ");
      textout_centre(screen, font, buf, SCREEN_W/2, SCREEN_H/2, 15);
   } while (!keypressed());

   clear_keybuf();
   show_mouse(screen);
   return D_REDRAW;
}



int keyboard_proc()
{
   int k, c;

   show_mouse(NULL);
   text_mode(0);
   clear(screen);
   textout_centre(screen, font, "Keyboard test", SCREEN_W/2, 6, 15);
   textout_centre(screen, font, "Press a mouse button", SCREEN_W/2, SCREEN_H-10, 15);

   do {
      if (keypressed()) {
	 blit(screen, screen, xoff+0, yoff+48, xoff+0, yoff+40, 320, 112);
	 k = readkey();
	 c = k & 0xFF;
	 if ((c < ' ') || (c >= ' '+FONT_SIZE))
	    c = ' ';
	 sprintf(buf,"0x%04X - '%c'", k, c);
	 textout_centre(screen, font, buf, SCREEN_W/2, yoff+152, 15);
      }
   } while (!mouse_b);

   do {
   } while (mouse_b);

   show_mouse(screen);
   return D_REDRAW;
}



int joystick_proc()
{
   static char *hat_strings[] = 
      { "centre ", "left   ", "down   ", "right  ", "up     " };

   show_mouse(NULL);
   clear(screen);

   if (alert("Select joystick type", NULL, NULL, "Standard", "FSPro", 0, 0) == 1)
      joy_type = JOY_TYPE_STANDARD;
   else
      joy_type = JOY_TYPE_FSPRO;

   text_mode(0);

   if (initialise_joystick() != 0) {
      textout_centre(screen, font, "Joystick not found", SCREEN_W/2, SCREEN_H/2, 15);
      do {
      } while (!next());
      return D_REDRAW;
   }

   textout_centre(screen, font, "Centre the joystick", SCREEN_W/2, SCREEN_H/2-8, 15);
   textout_centre(screen, font, "and press a button", SCREEN_W/2, SCREEN_H/2+8, 15);

   rest(10);
   do {
      poll_joystick();
   } while ((!joy_b1) && (!joy_b2));

   initialise_joystick();

   rest(10);
   do {
      poll_joystick();
   } while ((joy_b1) || (joy_b2));

   clear(screen);
   textout_centre(screen, font, "Move the joystick to the top", SCREEN_W/2, SCREEN_H/2-8, 15);
   textout_centre(screen, font, "left corner and press a button", SCREEN_W/2, SCREEN_H/2+8, 15);

   rest(10);
   do {
      poll_joystick();
   } while ((!joy_b1) && (!joy_b2));

   calibrate_joystick_tl();

   rest(10);
   do {
      poll_joystick();
   } while ((joy_b1) || (joy_b2));

   clear(screen);
   textout_centre(screen, font, "Move the joystick to the bottom", SCREEN_W/2, SCREEN_H/2-8, 15);
   textout_centre(screen, font, "right corner and press a button", SCREEN_W/2, SCREEN_H/2+8, 15);

   rest(10);
   do {
      poll_joystick();
   } while ((!joy_b1) && (!joy_b2));

   calibrate_joystick_br();

   rest(10);
   do {
      poll_joystick();
   } while ((joy_b1) || (joy_b2));

   if (joy_type == JOY_TYPE_FSPRO) {
      clear(screen);
      textout_centre(screen, font, "Move the throttle to the minimum", SCREEN_W/2, SCREEN_H/2-8, 15);
      textout_centre(screen, font, "setting and press a button", SCREEN_W/2, SCREEN_H/2+8, 15);

      rest(10);
      do {
	 poll_joystick();
      } while ((!joy_b1) && (!joy_b2));

      calibrate_joystick_throttle_min();

      rest(10);
      do {
	 poll_joystick();
      } while ((joy_b1) || (joy_b2));

      clear(screen);
      textout_centre(screen, font, "Move the throttle to the maximum", SCREEN_W/2, SCREEN_H/2-8, 15);
      textout_centre(screen, font, "setting and press a button", SCREEN_W/2, SCREEN_H/2+8, 15);

      rest(10);
      do {
	 poll_joystick();
      } while ((!joy_b1) && (!joy_b2));

      calibrate_joystick_throttle_max();

      rest(10);
      do {
	 poll_joystick();
      } while ((joy_b1) || (joy_b2));
   }

   clear(screen);
   message("Joystick test");

   while (!next()) {
      poll_joystick();

      sprintf(buf, "joy_x: %-8d", joy_x);
      textout(screen, font, buf, xoff+80, SCREEN_H/2-66, 15);

      sprintf(buf, "joy_y: %-8d", joy_y);
      textout(screen, font, buf, xoff+80, SCREEN_H/2-54, 15);

      sprintf(buf, "joy_left: %s", joy_left ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2-42, 15);

      sprintf(buf, "joy_right: %s", joy_right ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2-30, 15);

      sprintf(buf, "joy_up: %s", joy_up ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2-18, 15);

      sprintf(buf, "joy_down: %s", joy_down ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2-6, 15);

      if (joy_type == JOY_TYPE_FSPRO) {
	 sprintf(buf, "joy_FSPRO_trigger: %s", joy_FSPRO_trigger ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+6, 15);

	 sprintf(buf, "joy_FSPRO_butleft: %s", joy_FSPRO_butleft ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+18, 15);

	 sprintf(buf, "joy_FSPRO_butright: %s", joy_FSPRO_butright ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+30, 15);

	 sprintf(buf, "joy_FSPRO_butmiddle: %s", joy_FSPRO_butmiddle ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+42, 15);

	 sprintf(buf, "joy_hat: %s", hat_strings[joy_hat]);
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+54, 15);

	 sprintf(buf, "joy_throttle: %-8d", joy_throttle);
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+66, 15);
      }
      else {
	 sprintf(buf, "joy_b1: %s", joy_b1 ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+6, 15);

	 sprintf(buf, "joy_b2: %s", joy_b2 ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+18, 15);
      } 
   }

   show_mouse(screen);
   return D_REDRAW;
}



int int_c1 = 0;
int int_c2 = 0;
int int_c3 = 0;



void int1()
{
   if (++int_c1 >= 16)
      int_c1 = 0;
}

END_OF_FUNCTION(int1);



void int2()
{
   if (++int_c2 >= 16)
      int_c2 = 0;
}

END_OF_FUNCTION(int2);



void int3()
{
   if (++int_c3 >= 16)
      int_c3 = 0;
}

END_OF_FUNCTION(int3);



void interrupt_test()
{
   clear(screen);
   message("Timer interrupt test");

   textout(screen,font,"1/4", xoff+108, yoff+78, 15);
   textout(screen,font,"1", xoff+156, yoff+78, 15);
   textout(screen,font,"5", xoff+196, yoff+78, 15);

   LOCK_VARIABLE(int_c1);
   LOCK_VARIABLE(int_c2);
   LOCK_VARIABLE(int_c3);
   LOCK_FUNCTION(int1);
   LOCK_FUNCTION(int2);
   LOCK_FUNCTION(int3);

   install_int(int1, 250);
   install_int(int2, 1000);
   install_int(int3, 5000);

   while (!next()) {
      rectfill(screen, xoff+110, yoff+90, xoff+130, yoff+110, int_c1);
      rectfill(screen, xoff+150, yoff+90, xoff+170, yoff+110, int_c2);
      rectfill(screen, xoff+190, yoff+90, xoff+210, yoff+110, int_c3);
   }

   remove_int(int1);
   remove_int(int2);
   remove_int(int3);
}



int fade_color = 63;

void fade()
{
   int c = (fade_color < 64) ? fade_color : 127 - fade_color;
   RGB rgb = { c, c, c };

   _set_color(0, &rgb);

   fade_color++;
   if (fade_color >= 128)
      fade_color = 0;
}

END_OF_FUNCTION(fade);



void retrace_test()
{
   int x, x2;

   clear(screen);

   if ((gfx_driver != &gfx_vga) && (gfx_driver != &gfx_modex)) {
      alert("Vertical retrace interrupts can only",
	    "reliably used in standard VGA graphics",
	    "modes (mode 13h and mode-X)",
	    "Sorry", NULL, 13, 0);
      return;
   }

   if (windows_version != 0) {
      sprintf(buf, "Windows %d.%d detected. Vertical", windows_version, windows_sub_version);
      alert(buf, 
	    "retrace interrupts are unlikely",
	    "to work. You have been warned!",
	    "OK", NULL, 13, 0);
   }

   text_mode(0);
   message("Vertical retrace interrupt test");
   textout_centre(screen, font, "Without retrace synchronisation", SCREEN_W/2, SCREEN_H/2-32, 15);

   LOCK_VARIABLE(int_c1);
   LOCK_VARIABLE(fade_color);
   LOCK_FUNCTION(int1);
   LOCK_FUNCTION(fade);

   install_int(int1, 1000);
   retrace_proc = fade;
   int_c1 = 0;
   x = retrace_count;

   while (!next()) {
      if (int_c1 > 0) {
	 int_c1 = 0;
	 x2 = retrace_count - x;
	 x = retrace_count;
	 sprintf(buf, "%d retraces per second", MID(0, x2, 99));
	 textout_centre(screen, font, buf, SCREEN_W/2, SCREEN_H/2, 15);
      }
   }

   textout_centre(screen, font, "  With retrace synchronisation  ", SCREEN_W/2, SCREEN_H/2-32, 15);
   timer_simulate_retrace(TRUE);
   retrace_proc = fade;

   while (!next()) {
      if (int_c1 > 0) {
	 int_c1 = 0;
	 x2 = retrace_count - x;
	 x = retrace_count;
	 sprintf(buf, "%d retraces per second", x2);
	 textout_centre(screen, font, buf, SCREEN_W/2, SCREEN_H/2, 15);
      }
   }

   timer_simulate_retrace(FALSE);
   remove_int(int1);
   retrace_proc = NULL;

   outportb(0x3C8, 0);
   outportb(0x3C9, 63);
   outportb(0x3C9, 63);
   outportb(0x3C9, 63);
}



void rotate_test()
{
   int c = 0;
   long ct;
   BITMAP *b;

   set_clip(screen, 0, 0, VIRTUAL_W-1, VIRTUAL_H-1);
   clear(screen);
   message("Bitmap rotation test");

   b = create_bitmap(32, 32);

   draw_sprite(screen, global_sprite, SCREEN_W/2-16-32, SCREEN_H/2-16-32);
   draw_sprite(screen, global_sprite, SCREEN_W/2-16-64, SCREEN_H/2-16-64);

   draw_sprite_v_flip(screen, global_sprite, SCREEN_W/2-16-32, SCREEN_H/2-16+32);
   draw_sprite_v_flip(screen, global_sprite, SCREEN_W/2-16-64, SCREEN_H/2-16+64);

   draw_sprite_h_flip(screen, global_sprite, SCREEN_W/2-16+32, SCREEN_H/2-16-32);
   draw_sprite_h_flip(screen, global_sprite, SCREEN_W/2-16+64, SCREEN_H/2-16-64);

   draw_sprite_vh_flip(screen, global_sprite, SCREEN_W/2-16+32, SCREEN_H/2-16+32);
   draw_sprite_vh_flip(screen, global_sprite, SCREEN_W/2-16+64, SCREEN_H/2-16+64);

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {
      clear(b);
      rotate_sprite(b, global_sprite, 0, 0, itofix(c));
      blit(b, screen, 0, 0, SCREEN_W/2-16, SCREEN_H/2-16, 32, 32);

      c++;
      if (c >= 256)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }

   destroy_bitmap(b);
}



void stretch_test()
{
   int c = 0;
   long ct;

   set_clip(screen, 0, 0, VIRTUAL_W-1, VIRTUAL_H-1);

   clear(screen);
   message("Bitmap scaling test");

   tm = 0; _tm = 0;
   ct = 0;
   c = 1;

   rect(screen, SCREEN_W/2-128, SCREEN_H/2-64, SCREEN_W/2+128, SCREEN_H/2+64, 15);
   set_clip(screen, SCREEN_W/2-127, SCREEN_H/2-63, SCREEN_W/2+127, SCREEN_H/2+63);

   while (!next()) {
      stretch_blit(global_sprite, screen, 0, 0, 32, 32, SCREEN_W/2-c, SCREEN_H/2-(256-c), c*2, (256-c)*2);

      if (c >= 255) {
	 c = 1;
	 rectfill(screen, SCREEN_W/2-127, SCREEN_H/2-63, SCREEN_W/2+127, SCREEN_H/2+63, 0);
      }
      else
	 c++;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, screen, 16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void hscroll_test()
{
   int x, y;
   int done = FALSE;
   int ox = mouse_x;
   int oy = mouse_y;
   int split = (SCREEN_H*3)/4;

   set_clip(screen, 0, 0, VIRTUAL_W-1, VIRTUAL_H-1);
   clear(screen);
   rect(screen, 0, 0, VIRTUAL_W-1, VIRTUAL_H-1, 15);
   text_mode(-1);

   for (x=1; x<16; x++) {
      vline(screen, VIRTUAL_W*x/16, 1, VIRTUAL_H-2, x);
      hline(screen, 1, VIRTUAL_H*x/16, VIRTUAL_W-2, x);
      sprintf(buf, "%x", x);
      textout(screen, font, buf, 2, VIRTUAL_H*x/16-4, 15);
      textout(screen, font, buf, VIRTUAL_W-9, VIRTUAL_H*x/16-4, 15);
      textout(screen, font, buf, VIRTUAL_W*x/16-4, 2, 15);
      textout(screen, font, buf, VIRTUAL_W*x/16-4, VIRTUAL_H-9, 15);
   }

   sprintf(buf, "Graphics driver: %s", gfx_driver->name);
   textout(screen, font, buf, 32, 32, 15);

   sprintf(buf, "Description: %s", gfx_driver->desc);
   textout(screen, font, buf, 32, 48, 15);

   sprintf(buf, "Specs: %s", gfx_specs);
   textout(screen, font, buf, 32, 64, 15);

   if (gfx_driver->scroll == NULL)
      textout(screen, font, "Hardware scrolling not supported", 32, 80, 15);

   if (gfx_driver == &gfx_modex)
      textout(screen, font, "PGUP/PGDN to adjust the split screen", 32, 80, 15);

   x = y = 0;
   position_mouse(64, 64);

   while ((!done) && (!mouse_b)) {
      if ((mouse_x != 64) || (mouse_y != 64)) {
	 x += mouse_x - 64;
	 y += mouse_y - 64;
	 position_mouse(64, 64);
      }

      if (keypressed()) {
	 switch (readkey() >> 8) {
	    case KEY_LEFT:  x--;          break;
	    case KEY_RIGHT: x++;          break;
	    case KEY_UP:    y--;          break;
	    case KEY_DOWN:  y++;          break;
	    case KEY_PGUP:  split--;      break;
	    case KEY_PGDN:  split++;      break;
	    default:       done = TRUE;   break;
	 }
      }

      if (x < 0)
	 x = 0;
      else if (x > (VIRTUAL_W - SCREEN_W))
	 x = VIRTUAL_W - SCREEN_W;

      if (y < 0)
	 y = 0;
      else if (y > (VIRTUAL_H - split))
	 y = VIRTUAL_H - split;

      if (split < 1)
	 split = 1;
      else if (split > SCREEN_H)
	 split = SCREEN_H;

      scroll_screen(x, y);

      if (gfx_driver == &gfx_modex)
	 split_modex_screen(split);
   }

   do {
   } while (mouse_b);

   position_mouse(ox, oy);
   clear_keybuf();

   scroll_screen(0, 0);

   if (gfx_driver == &gfx_modex)
      split_modex_screen(0);
}



void test_it(char *msg, void (*func)(int, int))
{ 
   int x = 0;
   int y = 0;
   int c = 0;
   int pat = random()%NUM_PATTERNS;

   do { 
      set_clip(screen, 0, 0, 0, 0);
      clear(screen); 
      message(msg);

      textout_centre(screen, font, "(arrow keys to slide)", SCREEN_W/2, 28, 15);
      textout(screen, font, "unclipped:", xoff+48, yoff+50, 15);
      textout(screen, font, "clipped:", xoff+180, yoff+62, 15);
      rect(screen, xoff+191, yoff+83, xoff+240, yoff+114, 15);

      drawing_mode(mode, pattern[pat], 0, 0);
      (*func)(xoff+x+60, yoff+y+70);
      set_clip(screen, xoff+192, yoff+84, xoff+239, yoff+113);
      (*func)(xoff+x+180, yoff+y+70);
      solid_mode();

      do {
	 if (mouse_b) {
	    do {
	    } while (mouse_b);
	    c = KEY_ESC<<8;
	    break;
	 }

	 if (keypressed())
	    c = readkey();

      } while (!c);

      if ((c>>8) == KEY_LEFT) {
	 if (x > -16)
	    x--;
	 c = 0;
      }
      else if ((c>>8) == KEY_RIGHT) {
	 if (x < 16)
	    x++;
	 c = 0;
      }
      else if ((c>>8) == KEY_UP) {
	 if (y > -16)
	    y--;
	 c = 0;
      }
      else if ((c>>8) == KEY_DOWN) {
	 if (y < 16)
	    y++;
	 c = 0;
      }

   } while (!c);
}



void do_it(char *msg, int clip_flag, void (*func)())
{ 
   int x1, y1, x2, y2;

   set_clip(screen, 0, 0, 0, 0);
   clear(screen);
   message(msg);

   if (clip_flag) {
      do {
	 x1 = (random() & 255) + 32;
	 x2 = (random() & 255) + 32;
      } while (abs(x1-x2) < 30);
      do {
	 y1 = (random() & 127) + 40;
	 y2 = (random() & 127) + 40;
      } while (abs(y1-y2) < 20);
      set_clip(screen, xoff+x1, yoff+y1, xoff+x2, yoff+y2);
   }

   drawing_mode(mode, pattern[random()%NUM_PATTERNS], 0, 0);

   (*func)();

   solid_mode();
}



void circler(BITMAP *b, int x, int y, int c)
{
   circlefill(b, x, y, 4, c);
}



int floodfill_proc()
{
   int c = 0;
   int ox, oy, nx, ny;

   show_mouse(NULL);
   text_mode(0);

   clear(screen);

   textout_centre(screen, font, "floodfill test", SCREEN_W/2, 6, 15);
   textout_centre(screen, font, "Press a mouse button to draw,", SCREEN_W/2, 64, 15);
   textout_centre(screen, font, "a key 0-9 to floodfill,", SCREEN_W/2, 80, 15);
   textout_centre(screen, font, "and ESC to finish", SCREEN_W/2, 96, 15);

   show_mouse(screen);

   ox = -1;
   oy = -1;

   do {
      c = mouse_b;

      if (c) {
	 nx = mouse_x;
	 ny = mouse_y;
	 if ((ox >= 0) && (oy >= 0)) {
	    show_mouse(NULL);
	    if (c&1)
	       line(screen, ox, oy, nx, ny, 255);
	    else
	       do_line(screen, ox, oy, nx, ny, 255, circler);
	    show_mouse(screen);
	 }
	 ox = nx;
	 oy = ny;
      } 
      else 
	 ox = oy = -1;

      if (keypressed()) {
	 c = readkey() & 0xff;
	 if ((c >= '0') && (c <= '9')) {
	    show_mouse(NULL);
	    drawing_mode(mode, pattern[random()%NUM_PATTERNS], 0, 0);
	    floodfill(screen, mouse_x, mouse_y, c-'0');
	    solid_mode();
	    show_mouse(screen);
	 }
      } 

   } while (c != 27);

   return D_REDRAW;
}



int polygon_proc()
{
   #define MAX_POINTS      256

   int k = 0;
   int num_points = 0;
   int points[MAX_POINTS*2];

   show_mouse(NULL);
   text_mode(0);

   clear(screen);

   textout_centre(screen, font, "polygon test", SCREEN_W/2, 6, 15);
   textout_centre(screen, font, "Press left mouse button to add a", SCREEN_W/2, 64, 15);
   textout_centre(screen, font, "point, right mouse button to draw,", SCREEN_W/2, 80, 15);
   textout_centre(screen, font, "and ESC to finish", SCREEN_W/2, 96, 15);

   show_mouse(screen);

   do {
   } while (mouse_b);

   do {
      if ((mouse_b & 1) && (num_points < MAX_POINTS)) {
	 points[num_points*2] = mouse_x;
	 points[num_points*2+1] = mouse_y;

	 show_mouse(NULL);

	 if (num_points > 0)
	    line(screen, points[(num_points-1)*2], points[(num_points-1)*2+1], 
			 points[num_points*2], points[num_points*2+1], 255);

	 circlefill(screen, points[num_points*2], points[num_points*2+1], 2, 255);

	 num_points++;

	 show_mouse(screen);
	 do {
	 } while (mouse_b);
      }

      if ((mouse_b & 2) && (num_points > 2)) {
	 show_mouse(NULL);

	 line(screen, points[(num_points-1)*2], points[(num_points-1)*2+1], 
						   points[0], points[1], 255);
	 drawing_mode(mode, pattern[random()%NUM_PATTERNS], 0, 0);
	 polygon(screen, num_points, points, 1);
	 solid_mode();

	 num_points = 0;

	 show_mouse(screen);
	 do {
	 } while (mouse_b);
      }

      if (keypressed())
	 k = readkey() & 0xff;

   } while (k != 27);

   return D_REDRAW;
}



int putpixel_proc()
{
   show_mouse(NULL);
   text_mode(0);
   test_it("putpixel test", putpix_test);
   do_it("timing putpixel", FALSE, putpix_demo);
   do_it("timing putpixel [clipped]", TRUE, putpix_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int getpixel_proc()
{
   show_mouse(NULL);
   text_mode(0);
   getpix_demo();
   show_mouse(screen);
   return D_REDRAW;
}



int hline_proc()
{
   show_mouse(NULL);
   text_mode(0);
   test_it("hline test", hline_test);
   do_it("timing hline", FALSE, hline_demo);
   do_it("timing hline [clipped]", TRUE, hline_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int vline_proc()
{
   show_mouse(NULL);
   text_mode(0);
   test_it("vline test", vline_test);
   do_it("timing vline", FALSE, vline_demo);
   do_it("timing vline [clipped]", TRUE, vline_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int line_proc()
{
   show_mouse(NULL);
   text_mode(0);
   test_it("line test", line_test);
   do_it("timing line", FALSE, line_demo);
   do_it("timing line [clipped]", TRUE, line_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int rectfill_proc()
{
   show_mouse(NULL);
   text_mode(0);
   test_it("rectfill test", rectfill_test);
   do_it("timing rectfill", FALSE, rectfill_demo);
   do_it("timing rectfill [clipped]", TRUE, rectfill_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int triangle_proc()
{
   show_mouse(NULL);
   text_mode(0);
   test_it("triangle test", triangle_test);
   do_it("timing triangle", FALSE, triangle_demo);
   do_it("timing triangle [clipped]", TRUE, triangle_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int circle_proc()
{
   show_mouse(NULL);
   text_mode(0);
   test_it("circle test", circle_test);
   do_it("timing circle", FALSE, circle_demo);
   do_it("timing circle [clipped]", TRUE, circle_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int circlefill_proc()
{
   show_mouse(NULL);
   text_mode(0);
   test_it("circlefill test", circlefill_test);
   do_it("timing circlefill", FALSE, circlefill_demo);
   do_it("timing circlefill [clipped]", TRUE, circlefill_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int textout_proc()
{
   show_mouse(NULL);
   text_mode(0);
   test_it("textout test", textout_test);
   do_it("timing textout", FALSE, textout_demo);
   do_it("timing textout [clipped]", TRUE, textout_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int blit_proc()
{
   int c;

   show_mouse(NULL);
   text_mode(0);
   set_clip(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);
   clear(screen);
   textout_centre(screen, font, "Testing overlapping blits", SCREEN_W/2, 6, 15);

   for (c=0; c<30; c++)
      circle(screen, xoff+160, yoff+100, c, c);

   for (c=0; c<16; c++) {
      blit(screen, screen, xoff+112, yoff+52, xoff+113, yoff+52, 96, 96);
      rest(5);
   }

   for (c=0; c<32; c++) {
      blit(screen, screen, xoff+113, yoff+52, xoff+112, yoff+52, 96, 96);
      rest(5);
   }

   for (c=0; c<16; c++) {
      blit(screen, screen, xoff+112, yoff+52, xoff+113, yoff+52, 96, 96);
      rest(5);
   }

   for (c=0; c<16; c++) {
      blit(screen, screen, xoff+112, yoff+52, xoff+112, yoff+53, 96, 96);
      rest(5);
   }

   for (c=0; c<32; c++) {
      blit(screen, screen, xoff+112, yoff+53, xoff+112, yoff+52, 96, 96);
      rest(5);
   }

   for (c=0; c<16; c++) {
      blit(screen, screen, xoff+112, yoff+52, xoff+112, yoff+53, 96, 96);
      rest(5);
   }

   blit_from_screen = TRUE;
   do_it("timing blit screen->screen", FALSE, blit_demo);
   blit_align = TRUE;
   do_it("timing blit screen->screen (aligned)", FALSE, blit_demo);
   blit_align = FALSE;
   blit_from_screen = FALSE;
   do_it("timing blit memory->screen", FALSE, blit_demo);
   blit_align = TRUE;
   do_it("timing blit memory->screen (aligned)", FALSE, blit_demo);
   blit_align = FALSE;
   do_it("timing blit [clipped]", TRUE, blit_demo);

   show_mouse(screen);
   return D_REDRAW;
}



int sprite_proc()
{
   show_mouse(NULL);
   text_mode(0);

   test_it("sprite test", sprite_test);
   do_it("timing draw_sprite", FALSE, sprite_demo);
   do_it("timing draw_sprite [clipped]", TRUE, sprite_demo);

   test_it("RLE sprite test", rle_sprite_test);
   do_it("timing draw_rle_sprite", FALSE, rle_sprite_demo);
   do_it("timing draw_rle_sprite [clipped]", TRUE, rle_sprite_demo);

   global_compiled_sprite = get_compiled_sprite(global_sprite, is_planar_bitmap(screen));

   test_it("compiled sprite test", compiled_sprite_test);
   do_it("timing draw_compiled_sprite", FALSE, compiled_sprite_demo);

   destroy_compiled_sprite(global_compiled_sprite);

   show_mouse(screen);
   return D_REDRAW;
}



int rotate_proc()
{
   show_mouse(NULL);
   text_mode(0);
   test_it("Flipped sprite test", flipped_sprite_test);
   rotate_test();
   show_mouse(screen);
   return D_REDRAW;
}



int stretch_proc()
{
   show_mouse(NULL);
   text_mode(0);
   stretch_test();
   show_mouse(screen);
   return D_REDRAW;
}



int hscroll_proc()
{
   show_mouse(NULL);
   text_mode(0);
   hscroll_test();
   show_mouse(screen);
   return D_REDRAW;
}



int misc_proc()
{
   show_mouse(NULL);
   text_mode(0);
   misc();
   show_mouse(screen);
   return D_REDRAW;
}



int interrupts_proc()
{
   show_mouse(NULL);
   text_mode(0);
   interrupt_test();
   show_mouse(screen);
   return D_REDRAW;
}



int vsync_proc()
{
   show_mouse(NULL);
   text_mode(0);
   retrace_test();
   show_mouse(screen);
   return D_REDRAW;
}



int quit_proc()
{
   return D_CLOSE;
}



void set_mode_str()
{
   static char *mode_name[] =
   {
      "solid",
      "xor",
      "copy pattern",
      "solid pattern",
      "masked pattern",
   };

   sprintf(mode_string, "&Drawing mode (%s)", mode_name[mode]);
}



int solid_proc()
{
   mode = DRAW_MODE_SOLID;
   set_mode_str();
   return D_O_K;
}



int xor_proc()
{
   mode = DRAW_MODE_XOR;
   set_mode_str();
   return D_O_K;
}



int copy_pat_proc()
{
   mode = DRAW_MODE_COPY_PATTERN;
   set_mode_str();
   return D_O_K;
}



int solid_pat_proc()
{
   mode = DRAW_MODE_SOLID_PATTERN;
   set_mode_str();
   return D_O_K;
}



int masked_pat_proc()
{
   mode = DRAW_MODE_MASKED_PATTERN;
   set_mode_str();
   return D_O_K;
}



int gfx_mode();


int gfx_mode_proc()
{
   show_mouse(NULL);
   clear(screen);
   gfx_mode();
   return D_REDRAW;
}



MENU mode_menu[] =
{
   { "&Solid",                   solid_proc,       NULL },
   { "&XOR",                     xor_proc,         NULL },
   { "&Copy pattern",            copy_pat_proc,    NULL },
   { "Solid &pattern",           solid_pat_proc,   NULL },
   { "&Masked pattern",          masked_pat_proc,  NULL },
   { NULL,                       NULL,             NULL }
};



MENU test_menu[] =
{
   { "&Graphics mode",           gfx_mode_proc,    NULL },
   { mode_string,                NULL,             mode_menu },
   { "",                         NULL,             NULL },
   { "&Quit",                    quit_proc,        NULL },
   { NULL,                       NULL,             NULL }
};



MENU primitives_menu[] =
{
   { "&putpixel()",              putpixel_proc,    NULL },
   { "&hline()",                 hline_proc,       NULL },
   { "&vline()",                 vline_proc,       NULL },
   { "&line()",                  line_proc,        NULL },
   { "&rectfill()",              rectfill_proc,    NULL },
   { "&circle()",                circle_proc,      NULL },
   { "c&irclefill()",            circlefill_proc,  NULL },
   { "&triangle()",              triangle_proc,    NULL },
   { NULL,                       NULL,             NULL }
};



MENU blitter_menu[] =
{
   { "&textout()",               textout_proc,     NULL },
   { "&blit()",                  blit_proc,        NULL },
   { "&stretch_blit()",          stretch_proc,     NULL },
   { "&draw_sprite()",           sprite_proc,      NULL },
   { "&rotate_sprite()",         rotate_proc,      NULL },
   { NULL,                       NULL,             NULL }
};



MENU interactive_menu[] =
{
   { "&getpixel()",              getpixel_proc,    NULL },
   { "&polygon()",               polygon_proc,     NULL },
   { "&floodfill()",             floodfill_proc,   NULL },
   { NULL,                       NULL,             NULL }
};



MENU gfx_menu[] =
{
   { "&Primitives",              NULL,             primitives_menu },
   { "&Blitting functions",      NULL,             blitter_menu },
   { "&Interactive tests",       NULL,             interactive_menu },
   { NULL,                       NULL,             NULL }
};



MENU io_menu[] =
{
   { "&Mouse",                   mouse_proc,       NULL },
   { "&Keyboard",                keyboard_proc,    NULL },
   { "&Joystick",                joystick_proc,    NULL },
   { "&Timers",                  interrupts_proc,  NULL },
   { "&Retrace",                 vsync_proc,       NULL },
   { NULL,                       NULL,             NULL }
};



MENU misc_menu[] =
{
   { "&Scrolling",               hscroll_proc,     NULL },
   { "&Time some stuff",         misc_proc,        NULL },
   { NULL,                       NULL,             NULL }
};



MENU menu[] =
{
   { "&Test",                    NULL,             test_menu },
   { "&Graphics",                NULL,             gfx_menu },
   { "&I/O",                     NULL,             io_menu },
   { "&Misc",                    NULL,             misc_menu },
   { NULL,                       NULL,             NULL }
};



DIALOG title_screen[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp) */
   { d_clear_proc,      0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL },
   { d_menu_proc,       0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    menu },
   { d_ctext_proc,      0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    "Allegro " VERSION_STR },
   { d_ctext_proc,      0,    16,   0,    0,    255,  0,    0,    0,       0,    0,    "By Shawn Hargreaves, " DATE_STR },
   { d_ctext_proc,      0,    64,   0,    0,    255,  0,    0,    0,       0,    0,    "" },
   { d_ctext_proc,      0,    80,   0,    0,    255,  0,    0,    0,       0,    0,    "" },
   { d_ctext_proc,      0,    96,   0,    0,    255,  0,    0,    0,       0,    0,    gfx_specs },
   { d_ctext_proc,      0,    128,  0,    0,    255,  0,    0,    0,       0,    0,    mouse_specs },
   { NULL,              0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL }
};

#define DIALOG_NAME     4
#define DIALOG_DESC     5



int gfx_mode()
{
   int card, w, h;

   if (!gfx_mode_select(&card, &w, &h))
      return -1;

   show_mouse(NULL);

   /* try to set a wide virtual screen... */
   if (set_gfx_mode(card, w, h, (w >= 512) ? 1024 : 512, 0) != 0) {
      if (set_gfx_mode(card, w, h, 0, 0) != 0) {
	 set_gfx_mode(GFX_VGA, 320, 200, 0, 0);
	 set_pallete(desktop_pallete);
	 alert("Error setting mode:", allegro_error, NULL, "Sorry", NULL, 13, 0);
      }
   }

   set_pallete(desktop_pallete);

   xoff = (SCREEN_W - 320) / 2;
   yoff = (SCREEN_H - 200) / 2;

   sprintf(gfx_specs, "%dx%d (%dx%d), %ldk vram", 
	   SCREEN_W, SCREEN_H, VIRTUAL_W, VIRTUAL_H, gfx_driver->vid_mem/1024);
   if (!gfx_driver->linear)
      sprintf(gfx_specs+strlen(gfx_specs), ", %ldk banks, %ldk granularity",
	      gfx_driver->bank_size/1024, gfx_driver->bank_gran/1024);

   title_screen[DIALOG_NAME].dp = gfx_driver->name;
   title_screen[DIALOG_DESC].dp = gfx_driver->desc;
   centre_dialog(title_screen+2);

   return 0;
}



void main()
{
   int buttons;
   int c;

   LOCK_FUNCTION(tm_tick);
   LOCK_VARIABLE(tm);
   LOCK_VARIABLE(_tm);

   allegro_init();

   buttons = install_mouse();
   sprintf(mouse_specs, "Mouse has %d buttons", buttons);

   install_keyboard();
   install_timer();
   initialise_joystick();
   install_int(tm_tick, 10);
   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);
   set_pallete(desktop_pallete);

   if (gfx_mode() != 0) {
      allegro_exit();
      exit(0);
   }

   set_mode_str();

   make_patterns();

   global_sprite = make_sprite();
   global_rle_sprite = get_rle_sprite(global_sprite);

   do_dialog(title_screen, -1);

   destroy_bitmap(global_sprite);
   destroy_rle_sprite(global_rle_sprite);

   for (c=0; c<NUM_PATTERNS; c++)
      destroy_bitmap(pattern[c]);

   allegro_exit();
   exit(0);
}

