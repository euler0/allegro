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
#include <bios.h>
#include <errno.h>
#include <math.h>

#include "allegro.h"


int mode = DRAW_MODE_SOLID;
char mode_string[80];

#define CHECK_TRANS_BLENDER()                \
   if (bitmap_color_depth(screen) > 8)       \
      set_trans_blender(0, 0, 0, 0x40);

#define TIME_SPEED   2

BITMAP *global_sprite = NULL;
RLE_SPRITE *global_rle_sprite = NULL;
COMPILED_SPRITE *global_compiled_sprite = NULL;

BITMAP *realscreen = NULL;

PALLETE mypal;

#define NUM_PATTERNS    8
BITMAP *pattern[NUM_PATTERNS];

char gfx_specs[80];
char gfx_specs2[80];
char mouse_specs[80];

char buf[160];

int xoff, yoff;

long tm = 0;        /* counter, incremented once a second */
int _tm = 0;


COLOR_MAP *trans_map = NULL;
COLOR_MAP *light_map = NULL;


void tm_tick()
{
   if (++_tm >= 100) {
      _tm = 0;
      tm++;

      if (realscreen)
	 blit(screen, realscreen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
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
   textout_centre(bmp, font, buf, SCREEN_W/2, y, pallete_color[15]);
   bmp->clip = cf;
   bmp->cl = cl;
   bmp->cr = cr;
   bmp->ct = ct;
   bmp->cb = cb;
}



void message(char *s)
{
   textout_centre(screen, font, s, SCREEN_W/2, 6, pallete_color[15]);
   textout_centre(screen, font, "Press a key or mouse button", SCREEN_W/2, SCREEN_H-10, pallete_color[15]);
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
   clear_to_color(b, bitmap_mask_color(b));
   circlefill(b, 16, 16, 8, pallete_color[2]);
   circle(b, 16, 16, 8, pallete_color[1]);
   line(b, 0, 0, 31, 31, pallete_color[3]);
   line(b, 31, 0, 0, 31, pallete_color[3]);
   text_mode(-1);
   textout(b, font, "Test", 1, 12, pallete_color[15]);
   return b;
}



void table_callback(int pos)
{
   rect(screen, (SCREEN_W-256)/2-1, 128, (SCREEN_W+256)/2+1, 140, pallete_color[255]);
   rectfill(screen, (SCREEN_W-256)/2, 129, (SCREEN_W-256)/2+pos, 139, pallete_color[1]);
}



int check_tables()
{
   if (bitmap_color_depth(screen) > 8) {
      set_trans_blender(0, 0, 0, 128);
   }
   else if ((!rgb_map) || (!trans_map) || (!light_map)) {
      show_mouse(NULL);
      text_mode(pallete_color[0]);

      if (!rgb_map) {
	 clear_to_color(screen, pallete_color[0]);
	 textout_centre(screen, font, "Creating RGB table", SCREEN_W/2, 64, pallete_color[255]);

	 rgb_map = malloc(sizeof(RGB_MAP));
	 create_rgb_table(rgb_map, mypal, table_callback);
      }

      if (!trans_map) {
	 clear_to_color(screen, pallete_color[0]);
	 textout_centre(screen, font, "Creating translucency table", SCREEN_W/2, 64, pallete_color[255]);

	 trans_map = malloc(sizeof(COLOR_MAP));
	 create_trans_table(trans_map, mypal, 96, 96, 96, table_callback);
      }

      if (!light_map) {
	 clear_to_color(screen, pallete_color[0]);
	 textout_centre(screen, font, "Creating lighting table", SCREEN_W/2, 64, pallete_color[255]);

	 light_map = malloc(sizeof(COLOR_MAP));
	 create_light_table(light_map, mypal, 0, 0, 0, table_callback);
      }

      color_map = trans_map;
      return D_REDRAW;
   }

   return D_O_K;
}



void make_patterns()
{
   int c;

   pattern[0] = create_bitmap(2, 2);
   clear_to_color(pattern[0], bitmap_mask_color(pattern[0]));
   putpixel(pattern[0], 0, 0, pallete_color[255]);

   pattern[1] = create_bitmap(2, 2);
   clear_to_color(pattern[1], bitmap_mask_color(pattern[1]));
   putpixel(pattern[1], 0, 0, pallete_color[255]);
   putpixel(pattern[1], 1, 1, pallete_color[255]);

   pattern[2] = create_bitmap(4, 4);
   clear_to_color(pattern[2], bitmap_mask_color(pattern[2]));
   vline(pattern[2], 0, 0, 4, pallete_color[255]);
   hline(pattern[2], 0, 0, 4, pallete_color[255]);

   pattern[3] = create_bitmap(4, 4);
   clear_to_color(pattern[3], bitmap_mask_color(pattern[3]));
   line(pattern[3], 0, 3, 3, 0, pallete_color[255]);

   pattern[4] = create_bitmap(16, 16);
   clear_to_color(pattern[4], bitmap_mask_color(pattern[4]));
   for (c=0; c<32; c+=2)
      circle(pattern[4], 8, 8, c, pallete_color[c]);

   pattern[5] = create_bitmap(16, 16);
   clear_to_color(pattern[5], bitmap_mask_color(pattern[5]));
   for (c=0; c<16; c+=2) {
      line(pattern[5], 8, 8, 0, c, pallete_color[c]);
      line(pattern[5], 8, 8, c, 0, pallete_color[c]);
      line(pattern[5], 8, 8, 15, c, pallete_color[c]);
      line(pattern[5], 8, 8, c, 15, pallete_color[c]);
   }

   pattern[6] = create_bitmap(16, 16);
   clear_to_color(pattern[6], bitmap_mask_color(pattern[6]));
   for (c=0; c<16; c++)
      hline(pattern[6], 0, c, 16, pallete_color[c]);

   pattern[7] = create_bitmap(64, 8);
   clear_to_color(pattern[7], bitmap_mask_color(pattern[7]));
   text_mode(bitmap_mask_color(pattern[7]));
   textout(pattern[7], font, "PATTERN!", 0, 0, pallete_color[255]);
}



void getpix_demo()
{
   int c;

   clear_to_color(screen, pallete_color[0]); 
   message("getpixel test");

   for (c=0; c<16; c++)
      rectfill(screen, xoff+100+((c&3)<<5), yoff+50+((c>>2)<<5),
		       xoff+120+((c&3)<<5), yoff+70+((c>>2)<<5), pallete_color[c]);

   while (!next()) {
      rest(20);
      show_mouse(NULL);
      c = getpixel(screen, mouse_x-2, mouse_y-2);
      sprintf(buf, "     %X     ", c);
      textout_centre(screen, font, buf, SCREEN_W/2, yoff+24, pallete_color[15]);
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
	    putpixel(screen, xpos+((c&3)<<4)+x, ypos+((c>>2)<<4)+y, pallete_color[c]);
}



void hline_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++)
      hline(screen, xpos+48-c*3, ypos+c*3, xpos+48+c*3, pallete_color[c]);
}



void vline_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++)
      vline(screen, xpos+c*4, ypos+36-c*3, ypos+36+c*3, pallete_color[c]); 
}



void line_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++) {
      line(screen, xpos+32, ypos+32, xpos+32+((c-8)<<2), ypos, pallete_color[c%15+1]);
      line(screen, xpos+32, ypos+32, xpos+32-((c-8)<<2), ypos+64, pallete_color[c%15+1]);
      line(screen, xpos+32, ypos+32, xpos, ypos+32-((c-8)<<2), pallete_color[c%15+1]);
      line(screen, xpos+32, ypos+32, xpos+64, ypos+32+((c-8)<<2), pallete_color[c%15+1]);
   }
}



void rectfill_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++)
      rectfill(screen, xpos+((c&3)*17), ypos+((c>>2)*17),
		       xpos+15+((c&3)*17), ypos+15+((c>>2)*17), pallete_color[c]);
}



void triangle_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++)
      triangle(screen, xpos+22+((c&3)<<4), ypos+15+((c>>2)<<4),
		       xpos+13+((c&3)<<3), ypos+7+((c>>2)<<4),
		       xpos+7+((c&3)<<4), ypos+27+((c>>2)<<3), pallete_color[c]);
}



void circle_test(int xpos, int ypos)
{
   int c;

   for (c=0; c<16; c++)
      circle(screen, xpos+32, ypos+32, c*2, pallete_color[c%15+1]);
}



void circlefill_test(int xpos, int ypos)
{
   int c;

   for (c=15; c>=0; c--)
      circlefill(screen, xpos+8+((c&3)<<4), ypos+8+((c>>2)<<4), c, pallete_color[c%15+1]);
}



void ellipse_test(int xpos, int ypos)
{
   int c;

   for (c=15; c>=0; c--)
      ellipse(screen, xpos+8+((c&3)<<4), ypos+8+((c>>2)<<4), (16-c)*2, (c+1)*2, pallete_color[c%15+1]);
}



void ellipsefill_test(int xpos, int ypos)
{
   int c;

   for (c=15; c>=0; c--)
      ellipsefill(screen, xpos+8+((c&3)<<4), ypos+8+((c>>2)<<4), (16-c)*2, (c+1)*2, pallete_color[c%15+1]);
}



void textout_test(int xpos, int ypos)
{
   text_mode(pallete_color[0]);
   textout(screen, font,"This is a", xpos-8, ypos, pallete_color[1]);
   textout(screen, font,"test of the", xpos+3, ypos+10, pallete_color[1]);
   textout(screen, font,"textout", xpos+14, ypos+20, pallete_color[1]);
   textout(screen, font,"function.", xpos+25, ypos+30, pallete_color[1]);

   text_mode(pallete_color[0]);
   textout(screen, font,"text_mode(0)", xpos, ypos+48, pallete_color[2]);
   textout(screen, font,"text_mode(0)", xpos+4, ypos+52, pallete_color[4]);

   text_mode(-1);
   textout(screen, font,"text_mode(-1)", xpos, ypos+68, pallete_color[2]);
   textout(screen, font,"text_mode(-1)", xpos+4, ypos+72, pallete_color[4]);
   text_mode(pallete_color[0]);
}



void sprite_test(int xpos, int ypos)
{
   int x,y;

   for (y=0;y<82;y++)
      for (x=0;x<82;x+=2)
	 putpixel(screen, xpos+x+(y&1), ypos+y, pallete_color[8]);

   for (x=6; x<64; x+=global_sprite->w+6)
      for (y=6; y<64; y+=global_sprite->w+6)
	 draw_sprite(screen, global_sprite, xpos+x, ypos+y);
}



void xlu_sprite_test(int xpos, int ypos)
{
   int x,y;

   for (y=0;y<82;y++)
      for (x=0;x<82;x+=2)
	 putpixel(screen, xpos+x+(y&1), ypos+y, pallete_color[8]);

   for (x=6; x<64; x+=global_sprite->w+6) {
      for (y=6; y<64; y+=global_sprite->w+6) {
	 set_trans_blender(0, 0, 0, x+y*3);
	 draw_trans_sprite(screen, global_sprite, xpos+x, ypos+y);
      }
   }
}



void lit_sprite_test(int xpos, int ypos)
{
   int x,y;

   for (y=0;y<82;y++)
      for (x=0;x<82;x+=2)
	 putpixel(screen, xpos+x+(y&1), ypos+y, pallete_color[8]);

   for (x=6; x<64; x+=global_sprite->w+6) {
      for (y=6; y<64; y+=global_sprite->w+6) {
	 set_trans_blender(x*4, (x+y)*2, y*4, 0);
	 draw_lit_sprite(screen, global_sprite, xpos+x, ypos+y, ((x*2+y)*5)&0xFF);
      }
   }
}



void rle_xlu_sprite_test(int xpos, int ypos)
{
   int x,y;

   for (y=0;y<82;y++)
      for (x=0;x<82;x+=2)
	 putpixel(screen, xpos+x+(y&1), ypos+y, pallete_color[8]);

   for (x=6; x<64; x+=global_sprite->w+6) {
      for (y=6; y<64; y+=global_sprite->w+6) {
	 set_trans_blender(0, 0, 0, x+y*3);
	 draw_trans_rle_sprite(screen, global_rle_sprite, xpos+x, ypos+y);
      }
   }
}



void rle_lit_sprite_test(int xpos, int ypos)
{
   int x,y;

   for (y=0;y<82;y++)
      for (x=0;x<82;x+=2)
	 putpixel(screen, xpos+x+(y&1), ypos+y, pallete_color[8]);

   for (x=6; x<64; x+=global_sprite->w+6) {
      for (y=6; y<64; y+=global_sprite->w+6) {
	 set_trans_blender(x*4, (x+y)*2, y*4, 0);
	 draw_lit_rle_sprite(screen, global_rle_sprite, xpos+x, ypos+y, ((x*2+y)*5)&0xFF);
      }
   }
}



void rle_sprite_test(int xpos, int ypos)
{
   int x,y;

   for (y=0;y<82;y++)
      for (x=0;x<82;x+=2)
	 putpixel(screen, xpos+x+(y&1), ypos+y, pallete_color[8]);

   for (x=6; x<64; x+=global_sprite->w+6)
      for (y=6; y<64; y+=global_sprite->w+6)
	 draw_rle_sprite(screen, global_rle_sprite, xpos+x, ypos+y);
}



void compiled_sprite_test(int xpos, int ypos)
{
   int x,y;

   for (y=0;y<82;y++)
      for (x=0;x<82;x+=2)
	 putpixel(screen, xpos+x+(y&1), ypos+y, pallete_color[8]);

   for (x=6; x<64; x+=global_sprite->w+6)
      for (y=6; y<64; y+=global_sprite->w+6)
	 draw_compiled_sprite(screen, global_compiled_sprite, xpos+x, ypos+y);
}



void flipped_sprite_test(int xpos, int ypos)
{
   int x, y;

   for (y=0;y<88;y++)
      for (x=0;x<88;x+=2)
	 putpixel(screen, xpos+x+(y&1), ypos+y, pallete_color[8]);

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
      putpixel(screen, xoff+x, yoff+y, pallete_color[c]);

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
      hline(screen, xoff+x1, yoff+y, xoff+x2, pallete_color[c]);

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
      vline(screen, xoff+x, yoff+y1, yoff+y2, pallete_color[c]);

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
      line(screen, xoff+x1, yoff+y1, xoff+x2, yoff+y2, pallete_color[c]);

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
      rectfill(screen, xoff+x1, yoff+y1, xoff+x2, yoff+y2, pallete_color[c]);

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
      triangle(screen, xoff+x1, yoff+y1, xoff+x2, yoff+y2, xoff+x3, yoff+y3, pallete_color[c]);

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
      circle(screen, xoff+x, yoff+y, r, pallete_color[c]);

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
      circlefill(screen, xoff+x, yoff+y, r, pallete_color[c]);

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



void ellipse_demo()
{
   int c = 0;
   int x, y, rx, ry;
   long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & 127) + 92;
      y = (random() & 63) + 76;
      rx = (random() & 31) + 16;
      ry = (random() & 31) + 16;
      ellipse(screen, xoff+x, yoff+y, rx, ry, pallete_color[c]);

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



void ellipsefill_demo()
{
   int c = 0;
   int x, y, rx, ry;
   long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & 127) + 92;
      y = (random() & 63) + 76;
      rx = (random() & 31) + 16;
      ry = (random() & 31) + 16;
      ellipsefill(screen, xoff+x, yoff+y, rx, ry, pallete_color[c]);

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

   text_mode(pallete_color[0]);

   while (!next()) {

      x = (random() & 127) + 40;
      y = (random() & 127) + 40;
      textout(screen, font, "textout test", xoff+x, yoff+y, pallete_color[c]);

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
   int xand = (SCREEN_W >= 320) ? 255 : 127;
   int xadd = (SCREEN_W >= 320) ? 16 : 80;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & xand) + xadd;
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



void xlu_sprite_demo()
{
   long ct;
   int x, y;
   int xand = (SCREEN_W >= 320) ? 255 : 127;
   int xadd = (SCREEN_W >= 320) ? 16 : 80;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & xand) + xadd;
      y = (random() & 127) + 30;

      if (bitmap_color_depth(screen) > 8)
	 set_trans_blender(0, 0, 0, random()&0x7F);

      draw_trans_sprite(screen, global_sprite, xoff+x, yoff+y);

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



void lit_sprite_demo()
{
   int c = 1;
   long ct;
   int x, y;
   int xand = (SCREEN_W >= 320) ? 255 : 127;
   int xadd = (SCREEN_W >= 320) ? 16 : 80;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & xand) + xadd;
      y = (random() & 127) + 30;

      if (bitmap_color_depth(screen) > 8)
	 set_trans_blender(random()&0xFF, random()&0xFF, random()&0xFF, 0);

      draw_lit_sprite(screen, global_sprite, xoff+x, yoff+y, c);

      c = (c+13) & 0xFF;

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



void rle_xlu_sprite_demo()
{
   long ct;
   int x, y;
   int xand = (SCREEN_W >= 320) ? 255 : 127;
   int xadd = (SCREEN_W >= 320) ? 16 : 80;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & xand) + xadd;
      y = (random() & 127) + 30;

      if (bitmap_color_depth(screen) > 8)
	 set_trans_blender(0, 0, 0, random()&0x7F);

      draw_trans_rle_sprite(screen, global_rle_sprite, xoff+x, yoff+y);

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



void rle_lit_sprite_demo()
{
   int c = 1;
   long ct;
   int x, y;
   int xand = (SCREEN_W >= 320) ? 255 : 127;
   int xadd = (SCREEN_W >= 320) ? 16 : 80;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & xand) + xadd;
      y = (random() & 127) + 30;

      if (bitmap_color_depth(screen) > 8)
	 set_trans_blender(random()&0xFF, random()&0xFF, random()&0xFF, 0);

      draw_lit_rle_sprite(screen, global_rle_sprite, xoff+x, yoff+y, c);

      c = (c+13) & 0xFF;

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
   int xand = (SCREEN_W >= 320) ? 255 : 127;
   int xadd = (SCREEN_W >= 320) ? 16 : 80;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & xand) + xadd;
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
   int xand = (SCREEN_W >= 320) ? 255 : 127;
   int xadd = (SCREEN_W >= 320) ? 16 : 80;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (random() & xand) + xadd;
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
int blit_mask = FALSE;


void blit_demo()
{
   long ct;
   int x, y;
   int sx, sy;
   BITMAP *b;

   solid_mode();

   b = create_bitmap(64, 32);
   if (!b) {
      clear_to_color(screen, pallete_color[0]);
      textout(screen, font, "Out of memory!", 50, 50, pallete_color[15]);
      destroy_bitmap(b);
      while (!next())
	 ;
      return;
   }

   clear_to_color(b, (blit_mask ? bitmap_mask_color(b) : pallete_color[0]));
   circlefill(b, 32, 16, 16, pallete_color[4]);
   circlefill(b, 32, 16, 10, pallete_color[2]);
   circlefill(b, 32, 16, 6, pallete_color[1]);
   line(b, 0, 0, 63, 31, pallete_color[3]);
   line(b, 0, 31, 63, 0, pallete_color[3]);
   rect(b, 8, 4, 56, 28, pallete_color[3]);
   rect(b, 0, 0, 63, 31, pallete_color[15]);

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
      else if (blit_mask)
	 masked_blit(b, screen, 0, 0, xoff+x, yoff+y, 64, 32);
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

   clear_to_color(screen, pallete_color[0]);
   textout(screen,font,"Timing some other routines...", xoff+44, 6, pallete_color[15]);

   p = create_bitmap(320, 200);
   if (!p)
      textout(screen,font,"Out of memory!", 16, 50, pallete_color[15]);
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
      textout(screen, font, buf, xoff+16, yoff+50, pallete_color[15]);
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
   textout(screen, font, buf, xoff+16, yoff+60, pallete_color[15]);

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
   textout(screen, font, buf, xoff+16, yoff+70, pallete_color[15]);

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
   textout(screen, font, buf, xoff+16, yoff+80, pallete_color[15]);

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
   textout(screen, font, buf, xoff+16, yoff+90, pallete_color[15]);

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
   textout(screen, font, buf, xoff+16, yoff+100, pallete_color[15]);

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
   textout(screen, font, buf, xoff+16, yoff+110, pallete_color[15]);

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
   textout(screen, font, buf, xoff+16, yoff+120, pallete_color[15]);

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
   textout(screen, font, buf, xoff+16, yoff+130, pallete_color[15]);

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
   textout(screen, font, buf, xoff+16, yoff+140, pallete_color[15]);

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
   textout(screen, font, buf, xoff+16, yoff+150, pallete_color[15]);

   textout(screen, font, "Press a key or mouse button", xoff+52, SCREEN_H-10, pallete_color[15]);

   while (!next())
      ;
}



void rainbow()
{
   char buf[80];
   int x, y;
   int r, g, b;
   float h, s, v, c;

   clear_to_color(screen, pallete_color[0]);
   sprintf(buf, "%d bit color...", bitmap_color_depth(screen));
   textout_centre(screen, font, buf, SCREEN_W/2, 6, pallete_color[15]);

   for (h=0; h<360; h+=0.25) {
      for (c=0; c<1; c+=0.005) {
	 s = 1.0-ABS(1.0-c*2);
	 v = MIN(c*2, 1.0);

	 x = cos(h*M_PI/180.0)*c*128.0;
	 y = sin(h*M_PI/180.0)*c*128.0;

	 hsv_to_rgb(h, s, v, &r, &g, &b);
	 putpixel(screen, SCREEN_W/2+x, SCREEN_H/2+y, makecol(r, g, b));
      }
   }

   textout(screen, font, "Press a key or mouse button", xoff+52, SCREEN_H-10, pallete_color[15]);

   while (!next())
      ;
}



int mouse_proc()
{
   show_mouse(NULL);
   text_mode(pallete_color[0]);
   clear_to_color(screen, pallete_color[0]);
   textout_centre(screen, font, "Mouse test", SCREEN_W/2, 6, pallete_color[15]);
   textout_centre(screen, font, "Press a key", SCREEN_W/2, SCREEN_H-10, pallete_color[15]);

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
      textout_centre(screen, font, buf, SCREEN_W/2, SCREEN_H/2, pallete_color[15]);
   } while (!keypressed());

   clear_keybuf();
   show_mouse(screen);
   return D_REDRAW;
}



int keyboard_proc()
{
   int k, c;

   show_mouse(NULL);
   text_mode(pallete_color[0]);
   clear_to_color(screen, pallete_color[0]);
   textout_centre(screen, font, "Keyboard test", SCREEN_W/2, 6, pallete_color[15]);
   textout_centre(screen, font, "Press a mouse button", SCREEN_W/2, SCREEN_H-10, pallete_color[15]);

   do {
      if (keypressed()) {
	 blit(screen, screen, xoff+0, yoff+48, xoff+0, yoff+40, 320, 112);
	 k = readkey();
	 c = k & 0xFF;
	 if ((c < ' ') || (c >= ' '+FONT_SIZE))
	    c = ' ';
	 sprintf(buf,"0x%04X - '%c'", k, c);
	 textout_centre(screen, font, buf, SCREEN_W/2, yoff+152, pallete_color[15]);
      }
   } while (!mouse_b);

   do {
   } while (mouse_b);

   show_mouse(screen);
   return D_REDRAW;
}



char *joy_list_getter(int index, int *list_size)
{
   static char *s[] =
   {
      "Standard",
      "Flightstick Pro",
      "4-Button",
      "6-Button",
      "Dual Joysticks",
      "Wingman Extreme"
   };

   if (index < 0) {
      *list_size = 6;
      return NULL;
   }

   return s[index];
}



DIALOG joy_type_dlg[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)  (d2)  (dp) */
   { d_clear_proc,      0,    0,    0,    0,    0,    0,    0,    0,          0,    0,    NULL },
   { d_shadow_box_proc, 0,    0,    280,  95,   255,  0,    0,    0,          0,    0,    NULL },
   { d_ctext_proc,      140,  8,    1,    1,    255,  0,    0,    0,          0,    0,    "Joystick Type" },
   { d_list_proc,       16,   28,   152,  51,   255,  0,    0,    D_EXIT,     0,    0,    joy_list_getter },
   { d_button_proc,     184,  28,   80,   16,   255,  0,    13,   D_EXIT,     0,    0,    "OK" },
   { NULL }
}; 



int joystick_proc()
{
   static char *hat_strings[] = { "centre ", "left   ", "down   ", "right  ", "up     " };
   static char *hat_strings2[] = { "", "left", "down", "right", "up" };
   int i;

   show_mouse(NULL);
   clear_to_color(screen, pallete_color[0]);

   centre_dialog(joy_type_dlg);
   set_dialog_color(joy_type_dlg, gui_fg_color, gui_bg_color);
   do_dialog(joy_type_dlg, 3);
   joy_type = joy_type_dlg[3].d1;

   text_mode(pallete_color[0]);
   clear_to_color(screen, pallete_color[0]);

   if (initialise_joystick() != 0) {
      textout_centre(screen, font, "Joystick not found", SCREEN_W/2, SCREEN_H/2, pallete_color[15]);
      do {
      } while (!next());
      return D_REDRAW;
   }

   textout_centre(screen, font, "Centre the joystick", SCREEN_W/2, SCREEN_H/2-8, pallete_color[15]);
   textout_centre(screen, font, "and press a button", SCREEN_W/2, SCREEN_H/2+8, pallete_color[15]);

   rest(10);
   do {
      poll_joystick();
   } while ((!joy_b1) && (!joy_b2));

   initialise_joystick();

   rest(10);
   do {
      poll_joystick();
   } while ((joy_b1) || (joy_b2));

   clear_to_color(screen, pallete_color[0]);
   textout_centre(screen, font, "Move the joystick to the top", SCREEN_W/2, SCREEN_H/2-8, pallete_color[15]);
   textout_centre(screen, font, "left corner and press a button", SCREEN_W/2, SCREEN_H/2+8, pallete_color[15]);

   rest(10);
   do {
      poll_joystick();
   } while ((!joy_b1) && (!joy_b2));

   calibrate_joystick_tl();

   rest(10);
   do {
      poll_joystick();
   } while ((joy_b1) || (joy_b2));

   clear_to_color(screen, pallete_color[0]);
   textout_centre(screen, font, "Move the joystick to the bottom", SCREEN_W/2, SCREEN_H/2-8, pallete_color[15]);
   textout_centre(screen, font, "right corner and press a button", SCREEN_W/2, SCREEN_H/2+8, pallete_color[15]);

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
      clear_to_color(screen, pallete_color[0]);
      textout_centre(screen, font, "Move the throttle to the minimum", SCREEN_W/2, SCREEN_H/2-8, pallete_color[15]);
      textout_centre(screen, font, "setting and press a button", SCREEN_W/2, SCREEN_H/2+8, pallete_color[15]);

      rest(10);
      do {
	 poll_joystick();
      } while ((!joy_b1) && (!joy_b2));

      calibrate_joystick_throttle_min();

      rest(10);
      do {
	 poll_joystick();
      } while ((joy_b1) || (joy_b2));

      clear_to_color(screen, pallete_color[0]);
      textout_centre(screen, font, "Move the throttle to the maximum", SCREEN_W/2, SCREEN_H/2-8, pallete_color[15]);
      textout_centre(screen, font, "setting and press a button", SCREEN_W/2, SCREEN_H/2+8, pallete_color[15]);

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

   if (joy_type == JOY_TYPE_WINGEX) {
      calibrate_joystick_hat(0);

      for (i=JOY_HAT_LEFT; i<=JOY_HAT_UP; i++) {
	 clear_to_color(screen, pallete_color[0]);
	 textprintf_centre(screen, font, SCREEN_W/2, SCREEN_H/2-8, pallete_color[15], "Move the hat %s", hat_strings2[i]);
	 textout_centre(screen, font, "and press a button", SCREEN_W/2, SCREEN_H/2+8, pallete_color[15]);

	 rest(10);
	 do {
	    poll_joystick();
	 } while ((!joy_b1) && (!joy_b2));

	 calibrate_joystick_hat(i);

	 rest(10);
	 do {
	    poll_joystick();
	 } while ((joy_b1) || (joy_b2));
      }
   }

   clear_to_color(screen, pallete_color[0]);
   message("Joystick test");

   while (!next()) {
      poll_joystick();

      sprintf(buf, "joy_x: %-8d", joy_x);
      textout(screen, font, buf, xoff+80, SCREEN_H/2-66, pallete_color[15]);

      sprintf(buf, "joy_y: %-8d", joy_y);
      textout(screen, font, buf, xoff+80, SCREEN_H/2-54, pallete_color[15]);

      sprintf(buf, "joy_left: %s", joy_left ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2-42, pallete_color[15]);

      sprintf(buf, "joy_right: %s", joy_right ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2-30, pallete_color[15]);

      sprintf(buf, "joy_up: %s", joy_up ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2-18, pallete_color[15]);

      sprintf(buf, "joy_down: %s", joy_down ? "yes" : "no ");
      textout(screen, font, buf, xoff+80, SCREEN_H/2-6, pallete_color[15]);

      if (joy_type == JOY_TYPE_FSPRO) {
	 sprintf(buf, "joy_FSPRO_trigger: %s", joy_FSPRO_trigger ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+6, pallete_color[15]);

	 sprintf(buf, "joy_FSPRO_butleft: %s", joy_FSPRO_butleft ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+18, pallete_color[15]);

	 sprintf(buf, "joy_FSPRO_butright: %s", joy_FSPRO_butright ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+30, pallete_color[15]);

	 sprintf(buf, "joy_FSPRO_butmiddle: %s", joy_FSPRO_butmiddle ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+42, pallete_color[15]);

	 sprintf(buf, "joy_hat: %s", hat_strings[joy_hat]);
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+54, pallete_color[15]);

	 sprintf(buf, "joy_throttle: %-8d", joy_throttle);
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+66, pallete_color[15]);
      }
      else if (joy_type == JOY_TYPE_WINGEX) {
	 sprintf(buf, "joy_WINGEX_trigger: %s", joy_WINGEX_trigger ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+6, pallete_color[15]);

	 sprintf(buf, "joy_WINGEX_butthumb: %s", joy_WINGEX_butthumb ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+18, pallete_color[15]);

	 sprintf(buf, "joy_WINGEX_buttop: %s", joy_WINGEX_buttop ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+30, pallete_color[15]);

	 sprintf(buf, "joy_WINGEX_butmiddle: %s", joy_WINGEX_butmiddle ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+42, pallete_color[15]);

	 sprintf(buf, "joy_hat: %s", hat_strings[joy_hat]);
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+54, pallete_color[15]);
      } 
      else {
	 sprintf(buf, "joy_b1: %s", joy_b1 ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+6, pallete_color[15]);

	 sprintf(buf, "joy_b2: %s", joy_b2 ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+18, pallete_color[15]);

	 if (joy_type == JOY_TYPE_4BUTTON || joy_type == JOY_TYPE_6BUTTON) {
	    sprintf(buf, "joy_b3: %s", joy_b3 ? "yes" : "no ");
	    textout(screen, font, buf, xoff+80, SCREEN_H/2+30, pallete_color[15]);

	    sprintf(buf, "joy_b4: %s", joy_b4 ? "yes" : "no ");
	    textout(screen, font, buf, xoff+80, SCREEN_H/2+42, pallete_color[15]);
	 }

	 if (joy_type == JOY_TYPE_6BUTTON) {
	    sprintf(buf, "joy_b5: %s", joy_b5 ? "yes" : "no ");
	    textout(screen, font, buf, xoff+80, SCREEN_H/2+54, pallete_color[15]);

	    sprintf(buf, "joy_b6: %s", joy_b6 ? "yes" : "no ");
	    textout(screen, font, buf, xoff+80, SCREEN_H/2+66, pallete_color[15]);
	 }
      } 

      if (joy_type == JOY_TYPE_2PADS) {
	 sprintf(buf, "joy2_x: %-8d", joy2_x);
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+42, pallete_color[15]);

	 sprintf(buf, "joy2_y: %-8d", joy2_y);
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+54, pallete_color[15]);

	 sprintf(buf, "joy2_left: %s", joy2_left ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+66, pallete_color[15]);

	 sprintf(buf, "joy2_right: %s", joy2_right ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+78, pallete_color[15]);

	 sprintf(buf, "joy2_up: %s", joy2_up ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+90, pallete_color[15]);

	 sprintf(buf, "joy2_down: %s", joy2_down ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+102, pallete_color[15]);

	 sprintf(buf, "joy2_b1: %s", joy2_b1 ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+114, pallete_color[15]);

	 sprintf(buf, "joy2_b2: %s", joy2_b2 ? "yes" : "no ");
	 textout(screen, font, buf, xoff+80, SCREEN_H/2+126, pallete_color[15]);
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
   clear_to_color(screen, pallete_color[0]);
   message("Timer interrupt test");

   textout(screen,font,"1/4", xoff+108, yoff+78, pallete_color[15]);
   textout(screen,font,"1", xoff+156, yoff+78, pallete_color[15]);
   textout(screen,font,"5", xoff+196, yoff+78, pallete_color[15]);

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
      rectfill(screen, xoff+110, yoff+90, xoff+130, yoff+110, pallete_color[int_c1]);
      rectfill(screen, xoff+150, yoff+90, xoff+170, yoff+110, pallete_color[int_c2]);
      rectfill(screen, xoff+190, yoff+90, xoff+210, yoff+110, pallete_color[int_c3]);
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

   clear_to_color(screen, pallete_color[0]);

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

   text_mode(pallete_color[0]);
   message("Vertical retrace interrupt test");
   textout_centre(screen, font, "Without retrace synchronisation", SCREEN_W/2, SCREEN_H/2-32, pallete_color[15]);

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
	 textout_centre(screen, font, buf, SCREEN_W/2, SCREEN_H/2, pallete_color[15]);
      }
   }

   textout_centre(screen, font, "  With retrace synchronisation  ", SCREEN_W/2, SCREEN_H/2-32, pallete_color[15]);
   timer_simulate_retrace(TRUE);
   retrace_proc = fade;

   while (!next()) {
      if (int_c1 > 0) {
	 int_c1 = 0;
	 x2 = retrace_count - x;
	 x = retrace_count;
	 sprintf(buf, "%d retraces per second", x2);
	 textout_centre(screen, font, buf, SCREEN_W/2, SCREEN_H/2, pallete_color[15]);
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
   fixed c = 0;
   long ct;
   BITMAP *b;

   set_clip(screen, 0, 0, VIRTUAL_W-1, VIRTUAL_H-1);
   clear_to_color(screen, pallete_color[0]);
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
      clear_to_color(b, pallete_color[0]);
      rotate_sprite(b, global_sprite, 0, 0, c);
      blit(b, screen, 0, 0, SCREEN_W/2-16, SCREEN_H/2-16, b->w, b->h);

      c += itofix(1) / 16;

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
   BITMAP *b;

   set_clip(screen, 0, 0, VIRTUAL_W-1, VIRTUAL_H-1);

   clear_to_color(screen, pallete_color[0]);
   message("Bitmap scaling test");

   tm = 0; _tm = 0;
   ct = 0;
   c = 1;

   rect(screen, SCREEN_W/2-128, SCREEN_H/2-64, SCREEN_W/2+128, SCREEN_H/2+64, pallete_color[15]);
   set_clip(screen, SCREEN_W/2-127, SCREEN_H/2-63, SCREEN_W/2+127, SCREEN_H/2+63);

   solid_mode();
   b = create_bitmap(32, 32);
   clear_to_color(b, pallete_color[0]);
   circlefill(b, 16, 16, 8, pallete_color[2]);
   circle(b, 16, 16, 8, pallete_color[1]);
   line(b, 0, 0, 31, 31, pallete_color[3]);
   line(b, 31, 0, 0, 31, pallete_color[3]);
   text_mode(-1);
   textout(b, font, "Test", 1, 12, pallete_color[15]);

   while (!next()) {
      stretch_blit(b, screen, 0, 0, 32, 32, SCREEN_W/2-c, SCREEN_H/2-(256-c), c*2, (256-c)*2);

      if (c >= 255) {
	 c = 1;
	 rectfill(screen, SCREEN_W/2-127, SCREEN_H/2-63, SCREEN_W/2+127, SCREEN_H/2+63, pallete_color[0]);
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
   int split = (SCREEN_H*3)/4;

   set_clip(screen, 0, 0, VIRTUAL_W-1, VIRTUAL_H-1);
   clear_to_color(screen, pallete_color[0]);
   rect(screen, 0, 0, VIRTUAL_W-1, VIRTUAL_H-1, pallete_color[15]);
   text_mode(-1);

   for (x=1; x<16; x++) {
      vline(screen, VIRTUAL_W*x/16, 1, VIRTUAL_H-2, pallete_color[x]);
      hline(screen, 1, VIRTUAL_H*x/16, VIRTUAL_W-2, pallete_color[x]);
      sprintf(buf, "%x", x);
      textout(screen, font, buf, 2, VIRTUAL_H*x/16-4, pallete_color[15]);
      textout(screen, font, buf, VIRTUAL_W-9, VIRTUAL_H*x/16-4, pallete_color[15]);
      textout(screen, font, buf, VIRTUAL_W*x/16-4, 2, pallete_color[15]);
      textout(screen, font, buf, VIRTUAL_W*x/16-4, VIRTUAL_H-9, pallete_color[15]);
   }

   sprintf(buf, "Graphics driver: %s", gfx_driver->name);
   textout(screen, font, buf, 32, 32, pallete_color[15]);

   sprintf(buf, "Description: %s", gfx_driver->desc);
   textout(screen, font, buf, 32, 48, pallete_color[15]);

   sprintf(buf, "Specs: %s", gfx_specs);
   textout(screen, font, buf, 32, 64, pallete_color[15]);

   sprintf(buf, "Color depth: %s", gfx_specs2);
   textout(screen, font, buf, 32, 80, pallete_color[15]);

   if (gfx_driver->scroll == NULL)
      textout(screen, font, "Hardware scrolling not supported", 32, 96, pallete_color[15]);
   else if (gfx_driver == &gfx_modex)
      textout(screen, font, "PGUP/PGDN to adjust the split screen", 32, 96, pallete_color[15]);

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
      clear_to_color(screen, pallete_color[0]); 
      message(msg);

      textout_centre(screen, font, "(arrow keys to slide)", SCREEN_W/2, 28, pallete_color[15]);
      textout(screen, font, "unclipped:", xoff+48, yoff+50, pallete_color[15]);
      textout(screen, font, "clipped:", xoff+180, yoff+62, pallete_color[15]);
      rect(screen, xoff+191, yoff+83, xoff+240, yoff+114, pallete_color[15]);

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
   clear_to_color(screen, pallete_color[0]);
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
   circlefill(b, x, y, 4, pallete_color[c]);
}



int floodfill_proc()
{
   int c = 0;
   int ox, oy, nx, ny;

   show_mouse(NULL);
   text_mode(pallete_color[0]);

   clear_to_color(screen, pallete_color[0]);

   textout_centre(screen, font, "floodfill test", SCREEN_W/2, 6, pallete_color[15]);
   textout_centre(screen, font, "Press a mouse button to draw,", SCREEN_W/2, 64, pallete_color[15]);
   textout_centre(screen, font, "a key 0-9 to floodfill,", SCREEN_W/2, 80, pallete_color[15]);
   textout_centre(screen, font, "and ESC to finish", SCREEN_W/2, 96, pallete_color[15]);

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
	       line(screen, ox, oy, nx, ny, pallete_color[255]);
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
	    floodfill(screen, mouse_x, mouse_y, pallete_color[c-'0']);
	    solid_mode();
	    show_mouse(screen);
	 }
      } 

   } while (c != 27);

   return D_REDRAW;
}



void draw_spline(int points[8])
{
   int i, c1, c2;

   if (bitmap_color_depth(screen) == 8) {
      c1 = 255;
      c2 = 1;
   }
   else {
      c1 = makecol(255, 255, 255);
      c2 = makecol(0, 255, 255);
   }

   spline(screen, points, c1);

   for (i=0; i<4; i++)
      rect(screen, points[i*2]-6, points[i*2+1]-6, 
		   points[i*2]+5, points[i*2+1]+5, c2);
}



int spline_proc()
{
   int points[8];
   int nx, ny, ox, oy;
   int sel, os;
   int c;

   show_mouse(NULL);
   text_mode(pallete_color[0]);

   clear_to_color(screen, pallete_color[0]);

   textout_centre(screen, font, "spline test", SCREEN_W/2, 6, pallete_color[15]);
   textout_centre(screen, font, "Drag boxes to change guide points,", SCREEN_W/2, 64, pallete_color[15]);
   textout_centre(screen, font, "and press ESC to finish", SCREEN_W/2, 80, pallete_color[15]);

   for (c=0; c<4; c++) {
      points[c*2] = SCREEN_W/2 + c*64 - 96;
      points[c*2+1] = SCREEN_H/2 + ((c&1) ? 32 : -32);
   }

   xor_mode(TRUE);

   nx = ox = mouse_x;
   ny = oy = mouse_x;
   sel = -1;

   draw_spline(points); 
   show_mouse(screen);

   for (;;) {
      nx = mouse_x;
      ny = mouse_y;
      os = sel;

      if (mouse_b) {
	 if (sel < 0) {
	    for (sel=3; sel>=0; sel--) {
	       if ((nx >= points[sel*2]-6) &&
		   (nx < points[sel*2]+6) &&
		   (ny >= points[sel*2+1]-6) &&
		   (ny < points[sel*2+1]+6))
		  break;
	    }
	 }
	 if ((sel >= 0) && ((ox != nx) || (oy != ny) || (os != sel))) {
	    show_mouse(NULL);
	    draw_spline(points); 

	    points[sel*2] = nx;
	    points[sel*2+1] = ny;

	    draw_spline(points); 
	    show_mouse(screen);
	 }
      }
      else
	 sel = -1;

      if (keypressed()) {
	 c = readkey();
	 if ((c&0xFF) == 27)
	    break;
      }

      ox = nx;
      oy = ny;
   }

   xor_mode(FALSE);

   return D_REDRAW;
}



int polygon_proc()
{
   #define MAX_POINTS      256

   int k = 0;
   int num_points = 0;
   int points[MAX_POINTS*2];

   CHECK_TRANS_BLENDER();

   show_mouse(NULL);
   text_mode(pallete_color[0]);

   clear_to_color(screen, pallete_color[0]);

   textout_centre(screen, font, "polygon test", SCREEN_W/2, 6, pallete_color[15]);
   textout_centre(screen, font, "Press left mouse button to add a", SCREEN_W/2, 64, pallete_color[15]);
   textout_centre(screen, font, "point, right mouse button to draw,", SCREEN_W/2, 80, pallete_color[15]);
   textout_centre(screen, font, "and ESC to finish", SCREEN_W/2, 96, pallete_color[15]);

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
			 points[num_points*2], points[num_points*2+1], pallete_color[255]);

	 circlefill(screen, points[num_points*2], points[num_points*2+1], 2, pallete_color[255]);

	 num_points++;

	 show_mouse(screen);
	 do {
	 } while (mouse_b);
      }

      if ((mouse_b & 2) && (num_points > 2)) {
	 show_mouse(NULL);

	 line(screen, points[(num_points-1)*2], points[(num_points-1)*2+1], 
						   points[0], points[1], pallete_color[255]);
	 drawing_mode(mode, pattern[random()%NUM_PATTERNS], 0, 0);
	 polygon(screen, num_points, points, pallete_color[1]);
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
   CHECK_TRANS_BLENDER();

   show_mouse(NULL);
   text_mode(pallete_color[0]);
   test_it("putpixel test", putpix_test);
   do_it("timing putpixel", FALSE, putpix_demo);
   do_it("timing putpixel [clipped]", TRUE, putpix_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int getpixel_proc()
{
   show_mouse(NULL);
   text_mode(pallete_color[0]);
   getpix_demo();
   show_mouse(screen);
   return D_REDRAW;
}



int hline_proc()
{
   CHECK_TRANS_BLENDER();

   show_mouse(NULL);
   text_mode(pallete_color[0]);
   test_it("hline test", hline_test);
   do_it("timing hline", FALSE, hline_demo);
   do_it("timing hline [clipped]", TRUE, hline_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int vline_proc()
{
   CHECK_TRANS_BLENDER();

   show_mouse(NULL);
   text_mode(pallete_color[0]);
   test_it("vline test", vline_test);
   do_it("timing vline", FALSE, vline_demo);
   do_it("timing vline [clipped]", TRUE, vline_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int line_proc()
{
   CHECK_TRANS_BLENDER();

   show_mouse(NULL);
   text_mode(pallete_color[0]);
   test_it("line test", line_test);
   do_it("timing line", FALSE, line_demo);
   do_it("timing line [clipped]", TRUE, line_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int rectfill_proc()
{
   CHECK_TRANS_BLENDER();

   show_mouse(NULL);
   text_mode(pallete_color[0]);
   test_it("rectfill test", rectfill_test);
   do_it("timing rectfill", FALSE, rectfill_demo);
   do_it("timing rectfill [clipped]", TRUE, rectfill_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int triangle_proc()
{
   CHECK_TRANS_BLENDER();

   show_mouse(NULL);
   text_mode(pallete_color[0]);
   test_it("triangle test", triangle_test);
   do_it("timing triangle", FALSE, triangle_demo);
   do_it("timing triangle [clipped]", TRUE, triangle_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int circle_proc()
{
   CHECK_TRANS_BLENDER();

   show_mouse(NULL);
   text_mode(pallete_color[0]);
   test_it("circle test", circle_test);
   do_it("timing circle", FALSE, circle_demo);
   do_it("timing circle [clipped]", TRUE, circle_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int circlefill_proc()
{
   CHECK_TRANS_BLENDER();

   show_mouse(NULL);
   text_mode(pallete_color[0]);
   test_it("circlefill test", circlefill_test);
   do_it("timing circlefill", FALSE, circlefill_demo);
   do_it("timing circlefill [clipped]", TRUE, circlefill_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int ellipse_proc()
{
   CHECK_TRANS_BLENDER();

   show_mouse(NULL);
   text_mode(pallete_color[0]);
   test_it("ellipse test", ellipse_test);
   do_it("timing ellipse", FALSE, ellipse_demo);
   do_it("timing ellipse [clipped]", TRUE, ellipse_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int ellipsefill_proc()
{
   CHECK_TRANS_BLENDER();

   show_mouse(NULL);
   text_mode(pallete_color[0]);
   test_it("ellipsefill test", ellipsefill_test);
   do_it("timing ellipsefill", FALSE, ellipsefill_demo);
   do_it("timing ellipsefill [clipped]", TRUE, ellipsefill_demo);
   show_mouse(screen);
   return D_REDRAW;
}



int textout_proc()
{
   show_mouse(NULL);
   text_mode(pallete_color[0]);
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
   text_mode(pallete_color[0]);
   set_clip(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);
   clear_to_color(screen, pallete_color[0]);
   textout_centre(screen, font, "Testing overlapping blits", SCREEN_W/2, 6, 15);

   for (c=0; c<30; c++)
      circle(screen, xoff+160, yoff+100, c, pallete_color[c]);

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
   blit_mask = TRUE;
   do_it("timing masked blit", FALSE, blit_demo);
   blit_mask = FALSE;
   do_it("timing blit [clipped]", TRUE, blit_demo);

   show_mouse(screen);
   return D_REDRAW;
}



int sprite_proc()
{
   show_mouse(NULL);
   text_mode(pallete_color[0]);

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



int xlu_sprite_proc()
{
   check_tables();

   show_mouse(NULL);
   text_mode(pallete_color[0]);

   test_it("translucent sprite test", xlu_sprite_test);
   do_it("timing draw_trans_sprite", FALSE, xlu_sprite_demo);
   do_it("timing draw_trans_sprite [clipped]", TRUE, xlu_sprite_demo);

   test_it("translucent RLE sprite test", rle_xlu_sprite_test);
   do_it("timing draw_trans_rle_sprite", FALSE, rle_xlu_sprite_demo);
   do_it("timing draw_trans_rle_sprite [clipped]", TRUE, rle_xlu_sprite_demo);

   show_mouse(screen);
   return D_REDRAW;
}



int lit_sprite_proc()
{
   check_tables();
   color_map = light_map;

   show_mouse(NULL);
   text_mode(pallete_color[0]);

   test_it("tinted sprite test", lit_sprite_test);
   do_it("timing draw_lit_sprite", FALSE, lit_sprite_demo);
   do_it("timing draw_lit_sprite [clipped]", TRUE, lit_sprite_demo);

   test_it("tinted RLE sprite test", rle_lit_sprite_test);
   do_it("timing draw_lit_rle_sprite", FALSE, rle_lit_sprite_demo);
   do_it("timing draw_lit_rle_sprite [clipped]", TRUE, rle_lit_sprite_demo);

   color_map = trans_map;
   show_mouse(screen);
   return D_REDRAW;
}



int rotate_proc()
{
   show_mouse(NULL);
   text_mode(pallete_color[0]);
   test_it("Flipped sprite test", flipped_sprite_test);
   rotate_test();
   show_mouse(screen);
   return D_REDRAW;
}



int polygon3d_proc()
{
   #define NUM_POINTS   8+6
   #define NUM_FACES    6
   #define BUFFER_SIZE  128
   #define NUM_MODES    9

   /* a 3d x,y,z position */
   typedef struct POINT
   {
      float x, y, z;
   } POINT;

   /* four vertices plus a normal make up a quad */
   typedef struct QUAD 
   {
      int v1, v2, v3, v4;
      int normal;
      int visible;
   } QUAD;

   /* vertices of the cube */
   static POINT point[NUM_POINTS] =
   {
      /* regular vertices */
      { -32, -32, -32 },
      { -32,  32, -32 },
      {  32,  32, -32 },
      {  32, -32, -32 },
      { -32, -32,  32 },
      { -32,  32,  32 },
      {  32,  32,  32 },
      {  32, -32,  32 },

      /* normals */
      { -32, -32, -33 },
      { -32, -32,  33 },
      { -33,  32, -32 },
      {  33,  32, -32 },
      {  32, -33,  32 },
      {  32,  33,  32 }
   };

   /* output vertex list */
   static V3D_f vtx[NUM_POINTS] =
   {
     /* x  y  z  u   v   c    */
      { 0, 0, 0, 0,  0,  0x30 },
      { 0, 0, 0, 0,  31, 0x99 },
      { 0, 0, 0, 31, 31, 0x55 },
      { 0, 0, 0, 31, 0,  0xDD },
      { 0, 0, 0, 31, 0,  0x40 },
      { 0, 0, 0, 31, 31, 0xBB },
      { 0, 0, 0, 0,  31, 0x77 },
      { 0, 0, 0, 0,  0,  0xF0 }
   };

   /* six faces makes up a cube */
   QUAD face[NUM_FACES] = 
   {
     /* v1 v2 v3 v4 norm */
      { 0, 3, 2, 1, 8  },
      { 4, 5, 6, 7, 9  },
      { 1, 5, 4, 0, 10 },
      { 2, 3, 7, 6, 11 },
      { 7, 3, 0, 4, 12 },
      { 6, 5, 1, 2, 13 }
   };

   /* descriptions of the render modes */
   static char *mode_desc[NUM_MODES] = 
   {
      "POLYTYPE_FLAT",
      "POLYTYPE_GCOL",
      "POLYTYPE_GRGB",
      "POLYTYPE_ATEX",
      "POLYTYPE_PTEX",
      "POLYTYPE_ATEX_MASK",
      "POLYTYPE_PTEX_MASK",
      "POLYTYPE_ATEX_LIT",
      "POLYTYPE_PTEX_LIT"
   };

   int c;
   int key;
   int mode = POLYTYPE_FLAT;
   int tile = 1;

   float xr = -16;
   float yr = 24;
   float zr = 0;
   float dist = 128;
   float vx, vy, vz;
   float nx, ny, nz;
   int redraw_mode = TRUE;
   MATRIX_f transform, camera;
   V3D_f *vertex, *normal;
   BITMAP *buffer = create_bitmap(BUFFER_SIZE, BUFFER_SIZE);
   BITMAP *texture = create_bitmap(32, 32);

   check_tables();
   color_map = light_map;

   blit(global_sprite, texture, 0, 0, 0, 0, 32, 32);
   rect(texture, 0, 0, 31, 31, pallete_color[1]);

   show_mouse(NULL);
   clear_to_color(screen, pallete_color[0]);
   text_mode(pallete_color[0]);

   textout_centre(screen, font, "3d polygon test", SCREEN_W/2, 6, pallete_color[15]);
   textout_centre(screen, font, "Use the arrow keys to rotate the", SCREEN_W/2, 64, pallete_color[15]);
   textout_centre(screen, font, "cube, + and - to zoom, space to", SCREEN_W/2, 80, pallete_color[15]);
   textout_centre(screen, font, "change drawing mode, enter to tile", SCREEN_W/2, 96, pallete_color[15]);
   textout_centre(screen, font, "the texture, and ESC to finish", SCREEN_W/2, 112, pallete_color[15]);

   /* set projection parameters */
   set_projection_viewport(0, 0, BUFFER_SIZE, BUFFER_SIZE);

   get_camera_matrix_f(&camera, 
		     0, 0, 0,             /* eye position */
		     0, 0, 1,             /* front vector */
		     0, -1, 0,            /* up vector */
		     32,                  /* field of view */
		     1);                  /* aspect ratio */

   for (;;) {
      if (redraw_mode) {
	 rectfill(screen, 0, 24, SCREEN_W, 32, pallete_color[0]);
	 textout_centre(screen, font, mode_desc[mode], SCREEN_W/2, 24, pallete_color[255]);
	 redraw_mode = FALSE;
      }

      clear_to_color(buffer, 8);

      /* build a transformation matrix */
      get_transformation_matrix_f(&transform, 1, xr, yr, zr, 0, 0, dist);

      /* transform vertices into view space */
      for (c=0; c<NUM_POINTS; c++)
	 apply_matrix_f(&transform, point[c].x, point[c].y, point[c].z, &vtx[c].x, &vtx[c].y, &vtx[c].z);

      /* check for hidden faces */
      for (c=0; c<NUM_FACES; c++) {
	 vertex = &vtx[face[c].v1];
	 normal = &vtx[face[c].normal];
	 vx = vertex->x;
	 vy = vertex->y;
	 vz = vertex->z;
	 nx = normal->x - vx;
	 ny = normal->y - vy;
	 nz = normal->z - vz;
	 if (dot_product_f(vx, vy, vz, nx, ny, nz) < 0)
	    face[c].visible = TRUE;
	 else
	    face[c].visible = FALSE;
      }

      /* project vertices into screen space */
      for (c=0; c<NUM_POINTS; c++) {
	 apply_matrix_f(&camera, vtx[c].x, vtx[c].y, vtx[c].z, &vtx[c].x, &vtx[c].y, &vtx[c].z);
	 persp_project_f(vtx[c].x, vtx[c].y, vtx[c].z, &vtx[c].x, &vtx[c].y);
      }

      /* if mask mode, draw backfaces that may be seen through holes */
      if ((mode == POLYTYPE_ATEX_MASK) || (mode == POLYTYPE_PTEX_MASK)) {
	 for (c=0; c<NUM_FACES; c++) {
	    if (!face[c].visible) {
	       quad3d_f(buffer, mode, texture, 
			&vtx[face[c].v1], &vtx[face[c].v2], 
			&vtx[face[c].v3], &vtx[face[c].v4]);
	    }
	 }
      }

      /* draw the cube */
      for (c=0; c<NUM_FACES; c++) {
	 if (face[c].visible) {
	    quad3d_f(buffer, mode, texture, 
		     &vtx[face[c].v1], &vtx[face[c].v2], 
		     &vtx[face[c].v3], &vtx[face[c].v4]);
	 }
      }

      /* blit to the screen */
      vsync();
      blit(buffer, screen, 0, 0, 
	   (SCREEN_W-BUFFER_SIZE)/2, 
	   (SCREEN_H-BUFFER_SIZE)/2, BUFFER_SIZE, BUFFER_SIZE);

      /* handle user input */
      if (keypressed()) {
	 key = readkey();

	 switch (key >> 8) {

	    case KEY_DOWN:
	       xr -= 4;
	       break;

	    case KEY_UP:
	       xr += 4;
	       break;

	    case KEY_LEFT:
	       yr -= 4;
	       break;

	    case KEY_RIGHT:
	       yr += 4;
	       break;

	    case KEY_ESC:
	       goto getout;
	       break; 

	    case KEY_SPACE:
	       mode++;
	       if (mode >= NUM_MODES)
		  mode = 0;
	       redraw_mode = TRUE;
	       break;

	    case KEY_ENTER:
	       tile = (tile == 1) ? 2 : 1;
	       for (c=0; c<8; c++) {
		  if (vtx[c].u)
		     vtx[c].u = 32 * tile - 1;
		  if (vtx[c].v)
		     vtx[c].v = 32 * tile - 1;
	       }
	       break;

	    default:
	       if ((key & 0xFF) == '+') {
		  if (dist > 64)
		     dist -= 16;
	       }
	       else if ((key & 0xFF) == '-') {
		  if (dist < 1024)
		     dist += 16;
	       }
	       break;
	 }
      }
   }

   getout:
   color_map = trans_map;
   destroy_bitmap(buffer);
   destroy_bitmap(texture);
   show_mouse(screen);
   return D_REDRAW;
}



int stretch_proc()
{
   show_mouse(NULL);
   text_mode(pallete_color[0]);
   stretch_test();
   show_mouse(screen);
   return D_REDRAW;
}



int hscroll_proc()
{
   show_mouse(NULL);
   text_mode(pallete_color[0]);
   hscroll_test();
   show_mouse(screen);
   return D_REDRAW;
}



int misc_proc()
{
   show_mouse(NULL);
   text_mode(pallete_color[0]);
   misc();
   show_mouse(screen);
   return D_REDRAW;
}



int rainbow_proc()
{
   show_mouse(NULL);
   text_mode(pallete_color[0]);
   rainbow();
   show_mouse(screen);
   return D_REDRAW;
}



int interrupts_proc()
{
   show_mouse(NULL);
   text_mode(pallete_color[0]);
   interrupt_test();
   show_mouse(screen);
   return D_REDRAW;
}



int vsync_proc()
{
   show_mouse(NULL);
   text_mode(pallete_color[0]);
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
   extern MENU mode_menu[];

   static char *mode_name[] =
   {
      "solid",
      "xor",
      "copy pattern",
      "solid pattern",
      "masked pattern",
      "translucent"
   };

   int i;

   sprintf(mode_string, "&Drawing mode (%s)", mode_name[mode]);

   for (i=0; mode_menu[i].proc; i++)
      mode_menu[i].flags = 0;

   mode_menu[mode].flags = D_SELECTED;
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



int trans_proc()
{
   mode = DRAW_MODE_TRANS;
   set_mode_str();

   return check_tables();
}



int gfx_mode();


int gfx_mode_proc()
{
   show_mouse(NULL);
   clear_to_color(screen, pallete_color[0]);
   gfx_mode();
   return D_REDRAW;
}



MENU mode_menu[] =
{
   { "&Solid",                   solid_proc,       NULL,    D_SELECTED  },
   { "&XOR",                     xor_proc,         NULL,    0           },
   { "&Copy pattern",            copy_pat_proc,    NULL,    0           },
   { "Solid &pattern",           solid_pat_proc,   NULL,    0           },
   { "&Masked pattern",          masked_pat_proc,  NULL,    0           },
   { "&Translucent",             trans_proc,       NULL,    0           },
   { NULL,                       NULL,             NULL,    0           }
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
   { "&ellipse()",               ellipse_proc,     NULL },
   { "ellip&sefill()",           ellipsefill_proc, NULL },
   { "&triangle()",              triangle_proc,    NULL },
   { NULL,                       NULL,             NULL }
};



MENU blitter_menu[] =
{
   { "&textout()",               textout_proc,     NULL },
   { "&blit()",                  blit_proc,        NULL },
   { "&stretch_blit()",          stretch_proc,     NULL },
   { "&draw_sprite()",           sprite_proc,      NULL },
   { "draw_tr&ans_sprite()",     xlu_sprite_proc,  NULL },
   { "draw_&lit_sprite()",       lit_sprite_proc,  NULL },
   { "&rotate_sprite()",         rotate_proc,      NULL },
   { NULL,                       NULL,             NULL }
};



MENU interactive_menu[] =
{
   { "&getpixel()",              getpixel_proc,    NULL },
   { "&polygon()",               polygon_proc,     NULL },
   { "polygon&3d()",             polygon3d_proc,   NULL },
   { "&floodfill()",             floodfill_proc,   NULL },
   { "&spline()",                spline_proc,      NULL },
   { NULL,                       NULL,             NULL }
};



MENU gfx_menu[] =
{
   { "&Primitives",              NULL,             primitives_menu, },
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
   { "&Color rainbows",          rainbow_proc,     NULL },
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
   { d_ctext_proc,      0,    0,    0,    0,    255,  0,    0,    0,       0,    0,    "Allegro " ALLEGRO_VERSION_STR },
   { d_ctext_proc,      0,    16,   0,    0,    255,  0,    0,    0,       0,    0,    "By Shawn Hargreaves, " ALLEGRO_DATE_STR },
   { d_ctext_proc,      0,    64,   0,    0,    255,  0,    0,    0,       0,    0,    "" },
   { d_ctext_proc,      0,    80,   0,    0,    255,  0,    0,    0,       0,    0,    "" },
   { d_ctext_proc,      0,    96,   0,    0,    255,  0,    0,    0,       0,    0,    gfx_specs },
   { d_ctext_proc,      0,    112,  0,    0,    255,  0,    0,    0,       0,    0,    gfx_specs2 },
   { d_ctext_proc,      0,    144,  0,    0,    255,  0,    0,    0,       0,    0,    mouse_specs },
   { NULL,              0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL }
};

#define DIALOG_NAME     4
#define DIALOG_DESC     5



int gfx_mode()
{
   static int card = GFX_AUTODETECT;
   static int w = 640;
   static int h = 480;
   static int color_depth = 8;
   int c;

   if (!gfx_mode_select_ex(&card, &w, &h, &color_depth))
      return -1;

   show_mouse(NULL);

   if (realscreen) {
      BITMAP *b = realscreen;
      realscreen = NULL;
      destroy_bitmap(screen);
      screen = b;
   }

   /* try to set a wide virtual screen... */
   set_color_depth(color_depth);
   if (set_gfx_mode(card, w, h, (w >= 512) ? 1024 : 512, (w >= 512) ? 1024 : 512) != 0) {
      if (set_gfx_mode(card, w, h, (w >= 512) ? 1024 : 512, 0) != 0) {
	 if (set_gfx_mode(card, w, h, 0, 0) != 0) {
	    if ((card == GFX_AUTODETECT) && (color_depth > 8) && (w == 640) && (h == 480)) {
	       set_color_depth(8);
	       set_gfx_mode(GFX_VGA, 320, 200, 0, 0);
	       set_pallete(mypal);
	       gui_fg_color = 255;
	       gui_mg_color = 8;
	       gui_bg_color = 0;
	       if (alert("Error setting mode. Do you want to", "emulate it? (very unstable and slow,", "intended only for Shawn's debugging)", "Go for it...", "Cancel", 13, 27) == 1) {
		  BITMAP *b;
		  set_color_depth(32);
		  set_gfx_mode(GFX_AUTODETECT, w, h, 0, 0);
		  set_pallete(mypal);
		  set_color_depth(color_depth);
		  b = screen;
		  screen = create_bitmap(w, h);
		  realscreen = b;
	       }
	    }
	    else {
	       set_color_depth(8);
	       set_gfx_mode(GFX_VGA, 320, 200, 0, 0);
	       set_pallete(mypal);
	       gui_fg_color = 255;
	       gui_mg_color = 8;
	       gui_bg_color = 0;
	       alert("Error setting mode:", allegro_error, NULL, "Sorry", NULL, 13, 0);
	    }
	 }
      }
   }

   set_pallete(mypal);

   if (mode == DRAW_MODE_TRANS)
      check_tables();

   xoff = (SCREEN_W - 320) / 2;
   yoff = (SCREEN_H - 200) / 2;

   sprintf(gfx_specs, "%dx%d (%dx%d), %ldk vram", 
	   SCREEN_W, SCREEN_H, VIRTUAL_W, VIRTUAL_H, gfx_driver->vid_mem/1024);
   if (!gfx_driver->linear)
      sprintf(gfx_specs+strlen(gfx_specs), ", %ldk banks, %ldk granularity",
	      gfx_driver->bank_size/1024, gfx_driver->bank_gran/1024);

   switch (bitmap_color_depth(screen)) {

      case 8:
	 strcpy(gfx_specs2, "8 bit (256 color)");
	 break;

      case 15:
	 strcpy(gfx_specs2, "15 bit (32K HiColor)");
	 break;

      case 16:
	 strcpy(gfx_specs2, "16 bit (64K HiColor)");
	 break;

      case 24:
	 strcpy(gfx_specs2, "24 bit (16M TrueColor)");
	 break;

      case 32:
	 strcpy(gfx_specs2, "32 bit (16M TrueColor)");
	 break;

      default:
	 strcpy(gfx_specs2, "Unknown color depth!");
	 break;
   }

   if (global_sprite) {
      destroy_bitmap(global_sprite);
      destroy_rle_sprite(global_rle_sprite);

      for (c=0; c<NUM_PATTERNS; c++)
	 destroy_bitmap(pattern[c]);
   }

   make_patterns();

   global_sprite = make_sprite();
   global_rle_sprite = get_rle_sprite(global_sprite);

   gui_fg_color = pallete_color[255];
   gui_mg_color = pallete_color[8];
   gui_bg_color = pallete_color[0];

   title_screen[DIALOG_NAME].dp = gfx_driver->name;
   title_screen[DIALOG_DESC].dp = gfx_driver->desc;
   centre_dialog(title_screen+2);
   set_dialog_color(title_screen, gui_fg_color, gui_bg_color);

   return 0;
}



int main()
{
   int buttons;
   int c;

   LOCK_FUNCTION(tm_tick);
   LOCK_VARIABLE(tm);
   LOCK_VARIABLE(_tm);

   allegro_init();

   for (c=0; c<32; c++)
      mypal[c] = desktop_pallete[c];

   for (c=0; c<32; c++) {
      mypal[c+32].r = c*2;
      mypal[c+32].g = mypal[c+32].b = 0;
   }

   for (c=0; c<32; c++) {
      mypal[c+64].g = c*2;
      mypal[c+64].r = mypal[c+64].b = 0;
   }

   for (c=0; c<32; c++) {
      mypal[c+96].b = c*2;
      mypal[c+96].r = mypal[c+96].g = 0;
   }

   for (c=0; c<32; c++) {
      mypal[c+128].r = mypal[c+128].g = c*2;
      mypal[c+128].b = 0;
   }

   for (c=0; c<32; c++) {
      mypal[c+160].r = mypal[c+160].b = c*2;
      mypal[c+160].g = 0;
   }

   for (c=0; c<32; c++) {
      mypal[c+192].g = mypal[c+192].b = c*2;
      mypal[c+192].r = 0;
   }

   for (c=0; c<31; c++)
      mypal[c+224].r = mypal[c+224].g = mypal[c+224].b = c*2;

   mypal[255].r = mypal[255].g = mypal[255].b = 0;

   buttons = install_mouse();
   sprintf(mouse_specs, "Mouse has %d buttons", buttons);

   install_keyboard();
   install_timer();
   initialise_joystick();
   install_int(tm_tick, 10);

   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);
   set_pallete(mypal);

   if (gfx_mode() != 0) {
      allegro_exit();
      return 0;
   }

   set_mode_str();

   do_dialog(title_screen, -1);

   destroy_bitmap(global_sprite);
   destroy_rle_sprite(global_rle_sprite);

   for (c=0; c<NUM_PATTERNS; c++)
      destroy_bitmap(pattern[c]);

   if (rgb_map)
      free(rgb_map);

   if (trans_map)
      free(trans_map);

   if (light_map)
      free(light_map);

   if (realscreen) {
      BITMAP *b = realscreen;
      realscreen = NULL;
      destroy_bitmap(screen);
      screen = b;
   }

   return 0;
}

