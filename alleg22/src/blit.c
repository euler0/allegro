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
 *      Blitting and bitmap stretching functions.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>

#include "allegro.h"
#include "internal.h"
#include "opcodes.h"



/* get_bitmap_addr:
 *  Helper function for deciding which way round to do a blit. Returns
 *  an absolute address corresponding to a pixel in a bitmap, converting
 *  banked modes into a theoretical linear-style address.
 */
static inline unsigned long get_bitmap_addr(BITMAP *bmp, int x, int y)
{
   unsigned long ret;

   ret = (unsigned long)bmp->line[y];

   if (is_planar_bitmap(bmp))
      ret *= 4;

   ret += x;

   if (bmp->write_bank != _stub_bank_switch)
      ret += _gfx_bank[y+bmp->line_ofs] * gfx_driver->bank_size;

   return ret;
}



/* void blit(BITMAP *src, BITMAP *dest, int s_x, s_y, int d_x, d_y, int w, h);
 *
 *  Copies an area of the source bitmap to the destination bitmap. s_x and 
 *  s_y give the top left corner of the area of the source bitmap to copy, 
 *  and d_x and d_y give the position in the destination bitmap. w and h 
 *  give the size of the area to blit. This routine respects the clipping 
 *  rectangle of the destination bitmap, and will work correctly even when 
 *  the two memory areas overlap (ie. src and dest are the same). 
 */
void blit(BITMAP *src, BITMAP *dest, int s_x, int s_y, int d_x, int d_y, int w, int h)
{
   BITMAP *tmp;
   unsigned long s_low, s_high, d_low, d_high;

   /* check for ridiculous cases */
   if ((s_x >= src->w) || (s_y >= src->h) || 
       (d_x >= dest->cr) || (d_y >= dest->cb))
      return;

   /* clip src left */
   if (s_x < 0) { 
      w += s_x;
      d_x -= s_x;
      s_x = 0;
   }

   /* clip src top */
   if (s_y < 0) { 
      h += s_y;
      d_y -= s_y;
      s_y = 0;
   }

   /* clip src right */
   if (s_x+w > src->w) 
      w = src->w - s_x;

   /* clip src bottom */
   if (s_y+h > src->h) 
      h = src->h - s_y;

   /* clip dest left */
   if (d_x < dest->cl) { 
      d_x -= dest->cl;
      w += d_x;
      s_x -= d_x;
      d_x = dest->cl;
   }

   /* clip dest top */
   if (d_y < dest->ct) { 
      d_y -= dest->ct;
      h += d_y;
      s_y -= d_y;
      d_y = dest->ct;
   }

   /* clip dest right */
   if (d_x+w > dest->cr) 
      w = dest->cr - d_x;

   /* clip dest bottom */
   if (d_y+h > dest->cb) 
      h = dest->cb - d_y;

   /* bottle out if zero size */
   if ((w <= 0) || (h <= 0)) 
      return;

   /* if the bitmaps are the same... */
   if (is_same_bitmap(src, dest)) {
      /* with single-banked cards we have to use a temporary bitmap */
      if ((dest->write_bank == dest->read_bank) && 
	  (dest->write_bank != _stub_bank_switch)) {
	 tmp = create_bitmap(w, h);
	 if (tmp) {
	    src->vtable->blit_to_memory(src, tmp, s_x, s_y, 0, 0, w, h);
	    dest->vtable->blit_from_memory(tmp, dest, 0, 0, d_x, d_y, w, h);
	    destroy_bitmap(tmp);
	 }
      }
      else {
	 /* check which way round to do the blit */
	 s_low = get_bitmap_addr(src, s_x, s_y);
	 s_high = get_bitmap_addr(src, s_x+w, s_y+h-1);
	 d_low = get_bitmap_addr(dest, d_x, d_y);
	 d_high = get_bitmap_addr(dest, d_x+w, d_y+h-1);

	 if ((s_low > d_high) || (d_low > s_high))
	    dest->vtable->blit_to_self(src, dest, s_x, s_y, d_x, d_y, w, h);
	 else if (s_low >= d_low)
	    dest->vtable->blit_to_self_forward(src, dest, s_x, s_y, d_x, d_y, w, h);
	 else
	    dest->vtable->blit_to_self_backward(src, dest, s_x, s_y, d_x, d_y, w, h);
      } 
   }
   else {
      /* if the bitmaps are different, just check which vtable to use... */
      if (src->vtable == &_linear_vtable)
	 dest->vtable->blit_from_memory(src, dest, s_x, s_y, d_x, d_y, w, h);
      else
	 src->vtable->blit_to_memory(src, dest, s_x, s_y, d_x, d_y, w, h);
   }
}

END_OF_FUNCTION(blit);



/* make_stretcher:
 *  Helper function for stretch_blit(). Builds a machine code stretching
 *  routine in the scratch memory area.
 */
static int make_stretcher(int compiler_pos, fixed sx, fixed sxd, int dest_width, int masked)
{
   int x, x2;
   int c;

   if (dest_width > 0) {
      if (sxd == itofix(1)) {             /* easy case for 1 -> 1 scaling */
	 if (masked) {
	    for (c=0; c<dest_width; c++) {
	       COMPILER_LODSB(); 
	       COMPILER_MASKED_STOSB(); 
	    }
	 }
	 else {
	    COMPILER_MOV_ECX(dest_width);
	    COMPILER_REP_MOVSB();
	 }
      }
      else if (sxd > itofix(1)) {         /* big -> little scaling */
	 for (x=0; x<dest_width; x++) {
	    COMPILER_LODSB(); 
	    if (masked) {
	       COMPILER_MASKED_STOSB(); 
	    }
	    else {
	       COMPILER_STOSB(); 
	    }
	    x2 = (sx >> 16) + 1;
	    sx += sxd;
	    x2 = (sx >> 16) - x2;
	    if (x2 > 1) {
	       COMPILER_ADD_ESI(x2);
	    }
	    else if (x2 == 1) {
	       COMPILER_INC_ESI();
	    }
	 }
      }
      else  {                             /* little -> big scaling */
	 x2 = sx >> 16;
	 COMPILER_LODSB();
	 for (x=0; x<dest_width; x++) {
	    if (masked) {
	       COMPILER_MASKED_STOSB();
	    }
	    else {
	       COMPILER_STOSB();
	    }
	    sx += sxd;
	    if ((sx >> 16) > x2) {
	       COMPILER_LODSB();
	       x2++;
	    }
	 }
      }
   }

   return compiler_pos;
}



/* do_stretch_blit:
 *  Like blit(), except it can scale images so the source and destination 
 *  rectangles don't need to be the same size. This routine doesn't do as 
 *  much safety checking as the regular blit: in particular you must take 
 *  care not to copy from areas outside the source bitmap, and you cannot 
 *  blit between overlapping regions, ie. you must use different bitmaps for 
 *  the source and the destination. This function can draw onto both linear
 *  and mode-X bitmaps.
 *
 *  This routine does some very dodgy stuff. It dynamically generates a
 *  chunk of machine code to scale a line of the bitmap, and then calls this. 
 *  I just _had_ to use self modifying code _somewhere_ in Allegro :-) 
 */
static void do_stretch_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height, int masked)
{
   fixed sx, sy, sxd, syd;
   int compiler_pos = 0;
   int plane;
   int d;

   /* trivial reject for zero sizes */
   if ((source_width <= 0) || (source_height <= 0) || 
       (dest_width <= 0) || (dest_height <= 0))
      return;

   /* convert to fixed point */
   sx = itofix(source_x);
   sy = itofix(source_y);

   /* calculate delta values */
   sxd = itofix(source_width) / dest_width;
   syd = itofix(source_height) / dest_height;

   /* clip? */
   if (dest->clip) {
      while (dest_x < dest->cl) {
	 dest_x++;
	 dest_width--;
	 sx += sxd;
      }

      while (dest_y < dest->ct) {
	 dest_y++;
	 dest_height--;
	 sy += syd;
      }

      if (dest_x+dest_width > dest->cr)
	 dest_width = dest->cr - dest_x;

      if (dest_y+dest_height > dest->cb)
	 dest_height = dest->cb - dest_y;

      if ((dest_width <= 0) || (dest_height <= 0))
	 return;
   }

   if (is_linear_bitmap(dest)) { 
      /* build a simple linear stretcher */
      compiler_pos = make_stretcher(0, sx, sxd, dest_width, masked);
   }
   else { 
      /* build four stretchers, one for each mode-X plane */
      for (plane=0; plane<4; plane++) {
	 COMPILER_PUSH_ESI();
	 COMPILER_PUSH_EDI();

	 COMPILER_MOV_EAX((0x100<<((dest_x+plane)&3))|2);
	 COMPILER_MOV_EDX(0x3C4);
	 COMPILER_OUTW();

	 compiler_pos = make_stretcher(compiler_pos, sx+sxd*plane, sxd<<2,
				       (dest_width-plane+3)>>2, masked);

	 COMPILER_POP_EDI();
	 COMPILER_POP_ESI();

	 if (((dest_x+plane) & 3) == 3) {
	    COMPILER_INC_EDI();
	 }

	 d = ((sx+sxd*(plane+1))>>16) - ((sx+sxd*plane)>>16);
	 if (d > 0) {
	    COMPILER_ADD_ESI(d);
	 }
      }

      dest_x >>= 2;
   }

   COMPILER_RET();

   /* call the stretcher function for each line */
   _do_stretch(source, dest, _scratch_mem, sx>>16, sy, syd, 
	       dest_x, dest_y, dest_height);
}



/* stretch_blit:
 *  Opaque bitmap scaling function.
 */
void stretch_blit(BITMAP *s, BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h)
{
   do_stretch_blit(s, d, s_x, s_y, s_w, s_h, d_x, d_y, d_w, d_h, FALSE);
}



/* stretch_sprite:
 *  Masked version of stretch_blit().
 */
void stretch_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, int w, int h)
{
   do_stretch_blit(sprite, bmp, 0, 0, sprite->w, sprite->h, x, y, w, h, TRUE); 
}

