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
 *      Sprite rotation, RLE, and compressed sprite routines.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

#ifdef DJGPP
#include <sys/farptr.h>
#endif

#include "allegro.h"
#include "internal.h"
#include "opcodes.h"



/* rotate_sprite:
 *  Draws a sprite image onto a bitmap at the specified position, rotating 
 *  it by the specified angle. The angle is a fixed point 16.16 number in 
 *  the same format used by the fixed point trig routines, with 256 equal 
 *  to a full circle, 64 a right angle, etc. This function can draw onto
 *  both linear and mode-X bitmaps.
 */
void rotate_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, fixed angle)
{
   fixed f1x, f1y, f1xd, f1yd;
   fixed f2x, f2y, f2xd, f2yd;
   fixed f3x, f3y;
   fixed w, h;
   fixed dir, dir2, dist;
   int x1, y1, x2, y2;
   int dx, dy;
   int sx, sy;
   unsigned long addr;
   int plane;
   int wgap = sprite->w;
   int hgap = sprite->h;

   /* rotate the top left pixel of the sprite */
   w = itofix(wgap/2);
   h = itofix(hgap/2);
   dir = fatan2(h, w);
   dir2 = fatan2(h, -w);
   dist = fsqrt((wgap*wgap + hgap*hgap) << 6) << 4;
   f1x = w - fmul(dist, fcos(dir - angle));
   f1y = h - fmul(dist, fsin(dir - angle));

   /* map the destination y axis onto the sprite */
   f1xd = fcos(itofix(64) - angle);
   f1yd = fsin(itofix(64) - angle);

   /* map the destination x axis onto the sprite */
   f2xd = fcos(-angle);
   f2yd = fsin(-angle);

   /* adjust the size of the region to be scanned */
   x1 = fixtoi(fmul(dist, fcos(dir + angle)));
   y1 = fixtoi(fmul(dist, fsin(dir + angle)));

   x2 = fixtoi(fmul(dist, fcos(dir2 + angle)));
   y2 = fixtoi(fmul(dist, fsin(dir2 + angle)));

   x1 = MAX(ABS(x1), ABS(x2)) - wgap/2;
   y1 = MAX(ABS(y1), ABS(y2)) - hgap/2;

   x -= x1;
   wgap += x1 * 2;
   f1x -= f2xd * x1;
   f1y -= f2yd * x1;

   y -= y1;
   hgap += y1 * 2;
   f1x -= f1xd * y1;
   f1y -= f1yd * y1;

   /* clip the output rectangle */
   if (bmp->clip) {
      while (x < bmp->cl) {
	 x++;
	 wgap--;
	 f1x += f2xd;
	 f1y += f2yd;
      }

      while (y < bmp->ct) {
	 y++;
	 hgap--;
	 f1x += f1xd;
	 f1y += f1yd;
      }

      while (x+wgap >= bmp->cr)
	 wgap--;

      while (y+hgap >= bmp->cb)
	 hgap--;

      if ((wgap <= 0) || (hgap <= 0))
	 return;
   }

   /* select the destination segment */
   _farsetsel(bmp->seg);

   /* and trace a bunch of lines through the bitmaps */
   for (dy=0; dy<hgap; dy++) {
      f2x = f1x;
      f2y = f1y;

      if (is_linear_bitmap(bmp)) {           /* draw onto a linear bitmap */
	 addr = bmp_write_line(bmp, y+dy) + x;

	 for (dx=0; dx<wgap; dx++) {
	    sx = fixtoi(f2x);
	    sy = fixtoi(f2y);

	    if ((sx >= 0) && (sx < sprite->w) && (sy >= 0) && (sy < sprite->h))
	       if (sprite->line[sy][sx])
		  _farnspokeb(addr, sprite->line[sy][sx]);

	    addr++;
	    f2x += f2xd;
	    f2y += f2yd;
	 }
      }
      else {                                 /* draw onto a mode-X bitmap */
	 for (plane=0; plane<4; plane++) {
	    f3x = f2x;
	    f3y = f2y;
	    addr = (unsigned long)bmp->line[y+dy] + ((x+plane)>>2);
	    outportw(0x3C4, (0x100<<((x+plane)&3))|2);

	    for (dx=plane; dx<wgap; dx+=4) {
	       sx = fixtoi(f3x);
	       sy = fixtoi(f3y);

	       if ((sx >= 0) && (sx < sprite->w) && (sy >= 0) && (sy < sprite->h))
		  if (sprite->line[sy][sx])
		     _farnspokeb(addr, sprite->line[sy][sx]);

	       addr++;
	       f3x += (f2xd<<2);
	       f3y += (f2yd<<2);
	    }

	    f2x += f2xd;
	    f2y += f2yd;
	 }
      }

      f1x += f1xd;
      f1y += f1yd;
   }
}



/* get_rle_sprite:
 *  Creates a run length encoded sprite based on the specified bitmap.
 *  The returned sprite is likely to be a lot smaller than the original
 *  bitmap, and can be drawn to the screen with draw_rle_sprite().
 *
 *  The compression is done individually for each line of the image.
 *  Format is a series of command bytes, 1-127 marks a run of that many
 *  solid pixels, negative numbers mark a gap of -n pixels, and 0 marks
 *  the end of a line (since zero can't occur anywhere else in the data,
 *  this can be used to find the start of a specified line when clipping).
 */
RLE_SPRITE *get_rle_sprite(BITMAP *bitmap)
{
   RLE_SPRITE *s;
   signed char *p;
   int x, y;
   int run;
   int c;

   #define WRITE_TO_SPRITE(x) {                                              \
      _grow_scratch_mem(c+1);                                                \
      p = (signed char *)_scratch_mem;                                       \
      p[c] = x;                                                              \
      c++;                                                                   \
   }

   c = 0;
   p = (signed char *)_scratch_mem;

   for (y=0; y<bitmap->h; y++) { 
      run = -1;
      for (x=0; x<bitmap->w; x++) { 
	 if (getpixel(bitmap, x, y)) {
	    if ((run >= 0) && (p[run] > 0) && (p[run] < 127))
	       p[run]++;
	    else {
	       run = c;
	       WRITE_TO_SPRITE(1);
	    }
	    WRITE_TO_SPRITE(getpixel(bitmap, x, y));
	 }
	 else {
	    if ((run >= 0) && (p[run] < 0) && (p[run] > -128))
	       p[run]--;
	    else {
	       run = c;
	       WRITE_TO_SPRITE(-1);
	    }
	 }
      }
      WRITE_TO_SPRITE(0);
   }

   s = malloc(sizeof(RLE_SPRITE) + c);

   if (s) {
      s->w = bitmap->w;
      s->h = bitmap->h;
      s->size = c;
      memcpy(s->dat, _scratch_mem, c);
   }

   return s;
}



/* destroy_rle_sprite:
 *  Destroys an RLE sprite structure returned by get_rle_sprite().
 */
void destroy_rle_sprite(RLE_SPRITE *sprite)
{
   if (sprite)
      free(sprite);
}



/* compile_sprite:
 *  Helper function for making compiled sprites.
 */
static void *compile_sprite(BITMAP *b, int l, int planar, int *len)
{
   int x, y;
   int offset;
   int run;
   int run_pos;
   int compiler_pos = 0;
   int xc = planar ? 4 : 1;
   void *p;

   for (y=0; y<b->h; y++) {

      /* for linear bitmaps, time for some bank switching... */
      if (!planar) {
	 COMPILER_MOV_EDI_EAX();
	 COMPILER_CALL_ESI();
	 COMPILER_ADD_ECX_EAX();
      }

      offset = 0;
      x = l;

      /* compile a line of the sprite */
      while (x<b->w) {
	 if (b->line[y][x]) {
	    run = 0;
	    run_pos = x;

	    while ((run_pos<b->w) && (b->line[y][run_pos])) {
	       run++;
	       run_pos += xc;
	    }

	    while (run>=4) {
	       COMPILER_MOVL_IMMED(offset, ((int)b->line[y][x]) |
					   ((int)b->line[y][x+xc] << 8) |
					   ((int)b->line[y][x+xc*2] << 16) |
					   ((int)b->line[y][x+xc*3] << 24));
	       x += xc*4;
	       offset += 4;
	       run -= 4;
	    }

	    if (run>=2) {
	       COMPILER_MOVW_IMMED(offset, ((int)b->line[y][x]) |
					   ((int)b->line[y][x+xc] << 8));
	       x += xc*2;
	       offset += 2;
	       run -= 2;
	    }

	    if (run>0) {
	       COMPILER_MOVB_IMMED(offset, b->line[y][x]);
	       x += xc;
	       offset++;
	    }
	 }
	 else {
	    x += xc;
	    offset++;
	 }
      }

      /* move on to the next line */
      if (y+1 < b->h) {
	 if (planar) {
	    COMPILER_ADD_ECX_EAX();
	 }
	 else {
	    COMPILER_INC_EDI();
	 }
      }
   }

   COMPILER_RET();

   p = malloc(compiler_pos);
   if (p) {
      memcpy(p, _scratch_mem, compiler_pos);
      *len = compiler_pos;
   }

   return p;
}



/* get_compiled_sprite:
 *  Creates a compiled sprite based on the specified bitmap.
 */
COMPILED_SPRITE *get_compiled_sprite(BITMAP *bitmap, int planar)
{
   COMPILED_SPRITE *s;
   int plane;

   s = malloc(sizeof(COMPILED_SPRITE));
   if (!s)
      return NULL;

   s->planar = planar;

   for (plane=0; plane<4; plane++) {
      s->proc[plane].draw = NULL;
      s->proc[plane].len = 0;
   }

   for (plane=0; plane < (planar ? 4 : 1); plane++) {
      s->proc[plane].draw = compile_sprite(bitmap, plane, planar, &s->proc[plane].len);

      if (!s->proc[plane].draw) {
	 destroy_compiled_sprite(s);
	 return NULL;
      }
   }

   return s;
}



/* destroy_compiled_sprite:
 *  Destroys a compiled sprite structure returned by get_compiled_sprite().
 */
void destroy_compiled_sprite(COMPILED_SPRITE *sprite)
{
   int plane;

   if (sprite) {
      for (plane=0; plane<4; plane++)
	 if (sprite->proc[plane].draw)
	    free(sprite->proc[plane].draw);

      free(sprite);
   }
}

