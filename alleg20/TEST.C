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
 *      A test program for the Allegro library.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "allegro.h"


int xor();

#define TIME_SPEED   2

BITMAP *global_sprite = NULL;
RLE_SPRITE *global_rle_sprite = NULL;

char gfx_specs[80];

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
      do {
      } while (mouse_b);
      return TRUE;
   }

   return FALSE;
}



BITMAP *make_sprite()
{
   BITMAP *b;

   b = create_bitmap(32, 32);
   clear(b);
   rectfill(b, 6, 10, 26, 22, 6);
   line(b, 0, 0, 31, 31, 1);
   line(b, 0, 31, 31, 0, 1);
   circle(b, 16, 16, 12, 2);
   circle(b, 16, 16, 11, 3);
   text_mode(-1);
   textout(b, font, "32", 9, 13, 15);
   return b;
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
      hline(screen, xpos+48-c*4, ypos+c*3, xpos+48+c*4, c);
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
      line(screen, xpos+32, ypos+32, xpos+32+((c-8)<<2), ypos+32-(8<<2), c);
      line(screen, xpos+32, ypos+32, xpos+32+((c-8)<<2), ypos+32+(8<<2), c);
      line(screen, xpos+32, ypos+32, xpos+32-(8<<2), ypos+32+((c-8)<<2), c);
      line(screen, xpos+32, ypos+32, xpos+32+(8<<2), ypos+32+((c-8)<<2), c);
   }
}



void rectfill_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++) {
      rectfill(screen, xpos+((c&3)*17), ypos+((c>>2)*17),
		       xpos+15+((c&3)*17), ypos+15+((c>>2)*17), c);
   }
}



void triangle_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++) {
      triangle(screen, xpos+22+((c&3)<<4), ypos+15+((c>>2)<<4),
		       xpos+13+((c&3)<<3), ypos+7+((c>>2)<<4),
		       xpos+7+((c&3)<<4), ypos+27+((c>>2)<<3), c);
   }
}



void circle_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++)
      circle(screen, xpos+32, ypos+32, (c<<1), c);
}



void circlefill_test(int xpos, int ypos)
{
   int c;

   for (c=15; c>=0; c--)
      circlefill(screen, xpos+8+((c&3)<<4), ypos+8+((c>>2)<<4), c+1, c);
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



void blit_demo(int clip_flag)
{
   long ct;
   BITMAP *b, *b2, *s;
   int oldx, oldy;
   int x1, x2, y1, y2;

   set_clip(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);

   if (clip_flag) {
      do {
	 y1 = (random() & 127) + 40;
	 y2 = (random() & 127) + 40;
      } while (abs(y1-y2) < 20);
      if (y1 > y2) {
	 x1 = y1;
	 y1 = y2;
	 y2 = x1;
      }
      do {
	 x1 = (random() & 255) + 32;
	 x2 = (random() & 255) + 32;
      } while (abs(x1-x2) < 30);
   }
   else {
      x1 = y1 = 0;
      x2 = SCREEN_W-1;
      y2 = SCREEN_H-1;
   }

   b = create_bitmap(64, 32);
   if (!b) {
      clear(screen);
      textout(screen, font, "Out of memory!", 50, 50, 15);
      while (!next())
	 ;
      return;
   }

   b2 = create_bitmap(64, 32);
   if (!b2) {
      clear(screen);
      textout(screen, font, "Out of memory!", 50, 50, 15);
      destroy_bitmap(b);
      while (!next())
	 ;
      return;
   }

   s = create_bitmap(SCREEN_W, SCREEN_H);
   if (!s) {
      destroy_bitmap(b);
      destroy_bitmap(b2);
      clear(screen);
      textout(screen, font, "Out of memory!", 50, 50, 15);
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

   clear(s); 
   set_clip(s, x1, MAX(y1, 28), x2, y2);
   line(s, 0, 0, SCREEN_W-1, SCREEN_H-1, 15);
   line(s, 0, SCREEN_H-1, SCREEN_W-1, 0, 15);
   for (oldx=0; oldx<16; oldx++)
      circle(s, SCREEN_W/2, SCREEN_H/2, ((oldx+1)<<3), oldx);

   set_clip(s, 0, 0, SCREEN_W-1, SCREEN_H-1);
   if (clip_flag)
      textout_centre(s, font, "timing blit [clipped]", SCREEN_W/2, 6, 15);
   else
      textout_centre(s, font, "timing blit", SCREEN_W/2, 6, 15);

   textout_centre(s, font, "(3 64x32 & full screen move per refresh)", SCREEN_W/2, 16, 15);
   textout_centre(s, font, "Press a key or mouse button", SCREEN_W/2, SCREEN_H-10, 15);
   set_clip(s, xoff+x1, yoff+MAX(y1,28), xoff+x2, yoff+y2);

   oldx = -160;
   oldy = -100;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      set_clip(s, 0, 0, SCREEN_W-1, SCREEN_H-1);
      blit(b2, s, 0, 0, oldx, oldy, 64, 32);

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct, s, 28);
	    ct = -1;
	 }
	 else
	    ct++;
      }

      oldx = mouse_x - 32;
      oldy = mouse_y - 16;
      blit(s, b2, oldx, oldy, 0, 0, 64, 32);
      putpixel(s, oldx+32, oldy+16, 15);
      set_clip(s, x1, y1, x2, y2);
      blit(b, s, 0, 0, oldx, oldy, 64, 32);
      blit(s, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
   }

   destroy_bitmap(b);
   destroy_bitmap(b2);
   destroy_bitmap(s);
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



void mouse_test()
{
   clear(screen);
   textout_centre(screen, font, "Mouse test", SCREEN_W/2, 6, 15);
   textout_centre(screen, font, "Press a key", SCREEN_W/2, SCREEN_H-10, 15);

   do {
      sprintf(buf, "X=%-4d Y=%-4d", mouse_x, mouse_y);
      if (mouse_b & 1)
	 strcat(buf," left");
      else
	 strcat(buf,"     ");
      if (mouse_b & 2)
	 strcat(buf," right"); 
      else
	 strcat(buf,"      ");
      textout_centre(screen, font, buf, SCREEN_W/2, SCREEN_H/2, 15);
   } while (!keypressed());

   clear_keybuf();
}



void keyboard_test()
{
   int k;

   clear(screen);
   textout_centre(screen, font, "Keyboard test", SCREEN_W/2, 6, 15);
   textout_centre(screen, font, "Press a mouse button", SCREEN_W/2, SCREEN_H-10, 15);

   do {
      if (keypressed()) {
	 blit(screen, screen, xoff+0, yoff+48, xoff+0, yoff+40, 320, 112);
	 k = readkey();
	 sprintf(buf,"0x%04X - '%c'", k, (k&0xFF));
	 textout_centre(screen, font, buf, SCREEN_W/2, yoff+152, 15);
      }
   } while (!mouse_b);

   do {
   } while (mouse_b);
}



void joystick_test()
{
   if (initialise_joystick() != 0) {
      clear(screen);
      textout_centre(screen, font, "Joystick not found", SCREEN_W/2, SCREEN_H/2, 15);

      do {
      } while (!next());

      return;
   }

   clear(screen);
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

   clear(screen);
   message("Joystick test");

   while (!next()) {
      poll_joystick();

      sprintf(buf, "joy_x: %-8d", joy_x);
      textout(screen, font, buf, xoff+80, SCREEN_H/2-56, 15);

      sprintf(buf, "joy_y: %-8d", joy_y);
      textout(screen, font, buf, xoff+80, SCREEN_H/2-40, 15);

      sprintf(buf, "joy_left: %s", joy_left ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2-24, 15);

      sprintf(buf, "joy_right: %s", joy_right ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2-8, 15);

      sprintf(buf, "joy_up: %s", joy_up ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2+8, 15);

      sprintf(buf, "joy_down: %s", joy_down ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2+24, 15);

      sprintf(buf, "joy_b1: %s", joy_b1 ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2+40, 15);

      sprintf(buf, "joy_b2: %s", joy_b2 ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2+56, 15);
   }
}



int int_c1 = 0;
int int_c2 = 0;
int int_c3 = 0;



void int1()
{
   if (++int_c1 >= 16)
      int_c1 = 0;

   rectfill(screen, xoff+110, yoff+90, xoff+130, yoff+110, int_c1);
}

END_OF_FUNCTION(int1);



void int2()
{
   if (++int_c2 >= 16)
      int_c2 = 0;

   rectfill(screen, xoff+150, yoff+90, xoff+170, yoff+110, int_c2);
}

END_OF_FUNCTION(int2);



void int3()
{
   if (++int_c3 >= 16)
      int_c3 = 0;

   rectfill(screen, xoff+190, yoff+90, xoff+210, yoff+110, int_c3);
}

END_OF_FUNCTION(int3);



void interrupt_test()
{
   clear(screen);
   message("Interrupt test");

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

   while (!next())
      ;

   remove_int(int1);
   remove_int(int2);
   remove_int(int3);
}



void rotate_test()
{
   int c = 0;
   long ct;
   BITMAP *b, *b2;

   set_clip(screen, 0, 0, VIRTUAL_W-1, VIRTUAL_H-1);
   clear(screen);
   message("Bitmap rotation test");

   b = create_bitmap(32, 32);
   b2 = create_bitmap(32, 32);

   clear(b);
   circle(b, 16, 16, 8, 1);
   line(b, 0, 0, 31, 31, 2);
   line(b, 31, 0, 0, 31, 3);
   textout(b, font, "Test", 0, 12, 15);

   draw_sprite(screen, b, SCREEN_W/2-16-32, SCREEN_H/2-16-32);
   draw_sprite(screen, b, SCREEN_W/2-16-64, SCREEN_H/2-16-64);

   draw_sprite_v_flip(screen, b, SCREEN_W/2-16-32, SCREEN_H/2-16+32);
   draw_sprite_v_flip(screen, b, SCREEN_W/2-16-64, SCREEN_H/2-16+64);

   draw_sprite_h_flip(screen, b, SCREEN_W/2-16+32, SCREEN_H/2-16-32);
   draw_sprite_h_flip(screen, b, SCREEN_W/2-16+64, SCREEN_H/2-16-64);

   draw_sprite_vh_flip(screen, b, SCREEN_W/2-16+32, SCREEN_H/2-16+32);
   draw_sprite_vh_flip(screen, b, SCREEN_W/2-16+64, SCREEN_H/2-16+64);

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {
      clear(b2);
      rotate_sprite(b2, b, 0, 0, itofix(c));
      blit(b2, screen, 0, 0, SCREEN_W/2-16, SCREEN_H/2-16, 32, 32);
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
   destroy_bitmap(b2);
}



void stretch_test()
{
   int c = 0;
   long ct;
   BITMAP *b;

   set_clip(screen, 0, 0, VIRTUAL_W-1, VIRTUAL_H-1);

   b = create_bitmap(32, 32);

   clear(b);
   circle(b, 16, 16, 8, 1);
   line(b, 0, 0, 31, 31, 2);
   line(b, 31, 0, 0, 31, 3);
   textout(b, font, "Test", 0, 12, 15);

   clear(screen);
   message("Bitmap scaling test");

   tm = 0; _tm = 0;
   ct = 0;
   c = 1;

   rect(screen, SCREEN_W/2-128, SCREEN_H/2-64, SCREEN_W/2+128, SCREEN_H/2+64, 15);
   set_clip(screen, SCREEN_W/2-127, SCREEN_H/2-63, SCREEN_W/2+127, SCREEN_H/2+63);

   while (!next()) {
      stretch_blit(b, screen, 0, 0, 32, 32, SCREEN_W/2-c, SCREEN_H/2-(256-c), c*2, (256-c)*2);
      if (c >= 255) {
	 c = 1;
	 clear(screen);
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

   destroy_bitmap(b);
}



void hscroll_test()
{
   int x, y;
   int done = FALSE;
   int ox = mouse_x;
   int oy = mouse_y;

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
	    case KEY_LEFT:  x--; break;
	    case KEY_RIGHT: x++; break;
	    case KEY_UP:    y--; break;
	    case KEY_DOWN:  y++; break;
	    default: done = TRUE; break;
	 }
      }

      if (x < 0)
	 x = 0;
      else if (x > (VIRTUAL_W - SCREEN_W))
	 x = VIRTUAL_W - SCREEN_W;

      if (y < 0)
	 y = 0;
      else if (y > (VIRTUAL_H - SCREEN_H))
	 y = VIRTUAL_H - SCREEN_H;

      vsync();
      scroll_screen(x, y);
   }

   scroll_screen(0, 0);
   position_mouse(ox, oy);
   clear_keybuf();
   do {
   } while (mouse_b);
}



void test_it(char *msg, void (*func)(int, int))
{ 
   set_clip(screen, 0, 0, 0, 0);
   clear(screen); 
   message(msg);

   xor_mode(xor());

   textout(screen, font, "unclipped:", xoff+48, yoff+50, 15);
   (*func)(xoff+60, yoff+70);

   textout(screen, font, "clipped:", xoff+180, yoff+62, 15);
   rect(screen, xoff+191, yoff+83, xoff+240, yoff+104, 15);
   set_clip(screen, xoff+192, yoff+84, xoff+239, yoff+103);
   (*func)(xoff+180, yoff+70);

   xor_mode(FALSE);

   while (!next())
      ;
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

   xor_mode(xor());

   (*func)();

   xor_mode(FALSE);
}



int putpixel_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      test_it("putpixel test", putpix_test);
      do_it("timing putpixel", FALSE, putpix_demo);
      do_it("timing putpixel [clipped]", TRUE, putpix_demo);
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int getpixel_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      getpix_demo();
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int hline_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      test_it("hline test", hline_test);
      do_it("timing hline", FALSE, hline_demo);
      do_it("timing hline [clipped]", TRUE, hline_demo);
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int vline_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      test_it("vline test", vline_test);
      do_it("timing vline", FALSE, vline_demo);
      do_it("timing vline [clipped]", TRUE, vline_demo);
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int line_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      test_it("line test", line_test);
      do_it("timing line", FALSE, line_demo);
      do_it("timing line [clipped]", TRUE, line_demo);
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int rectfill_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      test_it("rectfill test", rectfill_test);
      do_it("timing rectfill", FALSE, rectfill_demo);
      do_it("timing rectfill [clipped]", TRUE, rectfill_demo);
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int triangle_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      test_it("triangle test", triangle_test);
      do_it("timing triangle", FALSE, triangle_demo);
      do_it("timing triangle [clipped]", TRUE, triangle_demo);
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int circle_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      test_it("circle test", circle_test);
      do_it("timing circle", FALSE, circle_demo);
      do_it("timing circle [clipped]", TRUE, circle_demo);
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int circlefill_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      test_it("circlefill test", circlefill_test);
      do_it("timing circlefill", FALSE, circlefill_demo);
      do_it("timing circlefill [clipped]", TRUE, circlefill_demo);
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int textout_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      test_it("textout test", textout_test);
      do_it("timing textout", FALSE, textout_demo);
      do_it("timing textout [clipped]", TRUE, textout_demo);
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int blit_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      blit_demo(FALSE);
      blit_demo(TRUE);
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int sprite_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      global_sprite = make_sprite();
      global_rle_sprite = get_rle_sprite(global_sprite);
      test_it("sprite test", sprite_test);
      do_it("timing draw_sprite", FALSE, sprite_demo);
      do_it("timing draw_sprite [clipped]", TRUE, sprite_demo);
      test_it("RLE sprite test", rle_sprite_test);
      do_it("timing draw_rle_sprite", FALSE, rle_sprite_demo);
      do_it("timing draw_rle_sprite [clipped]", TRUE, rle_sprite_demo);
      destroy_bitmap(global_sprite);
      global_sprite = NULL;
      destroy_rle_sprite(global_rle_sprite);
      global_rle_sprite = NULL;
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int rotate_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      rotate_test();
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int stretch_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      stretch_test();
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int hscroll_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      hscroll_test();
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int misc_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      misc();
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int io_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      mouse_test();
      keyboard_test();
      joystick_test();
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int interrupts_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(0);
      interrupt_test();
      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



#define O_W          90
#define O_H          16


DIALOG title_screen[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp) */
   { d_clear_proc,      0,    0,    320,  200,  0,    0,    0,    0,       0,    0,    NULL },
   { d_ctext_proc,      160,  4,    1,    1,    15,   0,    0,    0,       0,    0,    "Allegro " VERSION_STR },
   { d_ctext_proc,      160,  14,   1,    1,    15,   0,    0,    0,       0,    0,    "" },
   { d_ctext_proc,      160,  24,   1,    1,    15,   0,    0,    0,       0,    0,    "" },
   { d_ctext_proc,      160,  34,   1,    1,    15,   0,    0,    0,       0,    0,    "" },
   { putpixel_proc,     10,   55,   O_W,  O_H,  15,   1,    'p',  D_EXIT,  0,    0,    "putpixel" },
   { getpixel_proc,     115,  55,   O_W,  O_H,  15,   2,    'g',  D_EXIT,  0,    0,    "getpixel" },
   { hline_proc,        220,  55,   O_W,  O_H,  15,   3,    'h',  D_EXIT,  0,    0,    "hline" },
   { vline_proc,        10,   75,   O_W,  O_H,  15,   4,    'v',  D_EXIT,  0,    0,    "vline" },
   { line_proc,         115,  75,   O_W,  O_H,  15,   5,    'l',  D_EXIT,  0,    0,    "line", },
   { rectfill_proc,     220,  75,   O_W,  O_H,  15,   6,    'r',  D_EXIT,  0,    0,    "rectfill" },
   { triangle_proc,     10,   95,   O_W,  O_H,  15,   10,   't',  D_EXIT,  0,    0,    "triangle" },
   { circle_proc,       115,  95,   O_W,  O_H,  15,   11,   0,    D_EXIT,  0,    0,    "circle" },
   { circlefill_proc,   220,  95,   O_W,  O_H,  15,   12,   'c',  D_EXIT,  0,    0,    "circlefill" },
   { textout_proc,      10,   115,  O_W,  O_H,  15,   13,   0,    D_EXIT,  0,    0,    "textout" },
   { blit_proc,         115,  115,  O_W,  O_H,  15,   14,   'b',  D_EXIT,  0,    0,    "blit" },
   { sprite_proc,       220,  115,  O_W,  O_H,  15,   1,    's',  D_EXIT,  0,    0,    "Sprites" },
   { rotate_proc,       10,   135,  O_W,  O_H,  15,   2,    0,    D_EXIT,  0,    0,    "Rotation" },
   { stretch_proc,      115,  135,  O_W,  O_H,  15,   3,    0,    D_EXIT,  0,    0,    "Stretch" },
   { hscroll_proc,      220,  135,  O_W,  O_H,  15,   4,    0,    D_EXIT,  0,    0,    "Scrolling" },
   { misc_proc,         10,   155,  O_W,  O_H,  15,   5,    'm',  D_EXIT,  0,    0,    "Misc" },
   { io_proc,           115,  155,  O_W,  O_H,  15,   6,    0,    D_EXIT,  0,    0,    "I/O" },
   { interrupts_proc,   220,  155,  O_W,  O_H,  15,   10,   'i',  D_EXIT,  0,    0,    "Timers" },
   { d_button_proc,     115,  180,  O_W,  O_H,  15,   0,    27,   D_EXIT,  0,    0,    "Quit" },
   { d_check_proc,      32,   187,  48,  12,   15,   0,    'x',   0,       0,    0,    "XOR:" },
   { NULL }
};

#define DIALOG_TITLE    1
#define DIALOG_NAME     2
#define DIALOG_DESC     3
#define DIALOG_SPECS    4
#define DIALOG_QUIT     23
#define DIALOG_XOR      24



int xor()
{
   return (title_screen[DIALOG_XOR].flags & D_SELECTED);
}



void main()
{
   int card, w, h;

   LOCK_FUNCTION(tm_tick);
   LOCK_VARIABLE(tm);
   LOCK_VARIABLE(_tm);

   allegro_init();
   install_keyboard();
   install_mouse();
   install_timer();
   install_int(tm_tick, 10);
   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);
   set_pallete(desktop_pallete);

   if (!gfx_mode_select(&card, &w, &h)) {
      allegro_exit();
      exit(1);
   }

   /* try to set a wide virtual screen... */
   if (set_gfx_mode(card, w, h, 1024, 0) != 0) {
      if (set_gfx_mode(card, w, h, 0, 0) != 0) {
	 allegro_exit();
	 printf("Error setting graphics mode\n%s\n\n", allegro_error);
	 exit(1);
      }
   }
   set_pallete(desktop_pallete);

   xoff = (SCREEN_W - 320) / 2;
   yoff = (SCREEN_H - 200) / 2;
   centre_dialog(title_screen);

   sprintf(gfx_specs, "%dx%d (%dx%d), %ldk vram", 
	   SCREEN_W, SCREEN_H, VIRTUAL_W, VIRTUAL_H, gfx_driver->vid_mem/1024);
   if (!gfx_driver->linear)
      sprintf(gfx_specs+strlen(gfx_specs), ", %ldk banks, %ldk granularity",
	      gfx_driver->bank_size/1024, gfx_driver->bank_gran/1024);

   title_screen[DIALOG_NAME].dp = gfx_driver->name;
   title_screen[DIALOG_DESC].dp = gfx_driver->desc;
   title_screen[DIALOG_SPECS].dp = gfx_specs;
   title_screen[DIALOG_TITLE].y = 4;
   title_screen[DIALOG_NAME].y = 14;
   title_screen[DIALOG_DESC].y = 24;
   title_screen[DIALOG_SPECS].y = 34;
   title_screen[DIALOG_QUIT].y = SCREEN_H - 20;

   if (SCREEN_H > 200)
      title_screen[DIALOG_XOR].y += 32;

   do_dialog(title_screen, -1);

   allegro_exit();
   exit(0);
}

