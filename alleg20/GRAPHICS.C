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
 *      Graphics routines: pallete fading, circle drawing, fonts, etc.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <go32.h>
#include <sys/farptr.h>

#include "allegro.h"
#include "internal.h"



static int gfx_virgin = TRUE;       /* first time we've been called? */
static int a_rez = 3;               /* original screen mode */



/* lock_bitmap:
 *  Locks all the memory used by a bitmap structure.
 */
void lock_bitmap(BITMAP *bmp)
{
   _go32_dpmi_lock_data(bmp, sizeof(BITMAP) + sizeof(char *) * bmp->h);

   if (bmp->dat)
      _go32_dpmi_lock_data(bmp->dat, bmp->w * bmp->h);
}



/* setup_bitmap_tables:
 *  Some global tables (the array used for filling shapes, and the svga bank 
 *  switching tables) need to be as big as the largest bitmap in use. Rather
 *  than just allocating loads of memory and hoping it is enough, we check 
 *  to see if the tables need enlarging whenever bitmaps are created.
 */
static int setup_bitmap_tables(int size)
{
   if (size > _bitmap_table_size) {
      _fill_array = realloc(_fill_array, sizeof(FILL_STRUCT) * size);
      if (!_fill_array)
	 return -1;

      _gfx_bank = realloc(_gfx_bank, size * sizeof(int));
      if (!_gfx_bank)
	 return -1;

      _go32_dpmi_lock_data(_gfx_bank, size * sizeof(int));
      _bitmap_table_size = size;
   }

   return 0;
}



/* shutdown_gfx:
 *  Used by allegro_exit() to return the system to text mode.
 */
static void shutdown_gfx()
{
   __dpmi_regs r;

   r.x.ax = 0x0F00; 
   __dpmi_int(0x10, &r);
   if (((r.x.ax & 0xFF) != a_rez) || (gfx_driver != NULL))
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);

   _remove_exit_func(shutdown_gfx);
}



/* set_gfx_mode:
 *  Sets the graphics mode. The card should be one of the GFX_* constants
 *  from allegro.h, or GFX_AUTODETECT to accept any graphics driver. Pass
 *  GFX_TEXT to return to text mode (although allegro_exit() will usually 
 *  do this for you). The w and h parameters specify the screen resolution 
 *  you want, and v_w and v_h specify the minumum virtual screen size. 
 *  The graphics drivers may actually create a much larger virtual screen, 
 *  so you should check the values of VIRTUAL_W and VIRTUAL_H after you
 *  set the mode. If unable to select an appropriate mode, this function 
 *  returns -1.
 */
int set_gfx_mode(int card, int w, int h, int v_w, int v_h)
{
   __dpmi_regs r;
   int c;

   /* list of drivers for autodetection */
   static GFX_DRIVER *drivers[] = 
   {
      &gfx_vga, &gfx_vesa_2l, &gfx_vesa_2b, &gfx_cirrus54, &gfx_s3, 
      &gfx_et4000, &gfx_vesa_1, NULL
   };

   if (gfx_virgin) {
      r.x.ax = 0x0F00;              /* store current video mode */
      __dpmi_int(0x10, &r);
      a_rez = r.x.ax & 0xFF; 

      _add_exit_func(shutdown_gfx);
      gfx_virgin = FALSE;
   }

   /* close down any existing graphics driver */
   if (gfx_driver) {
      if (gfx_driver->scroll)
	 gfx_driver->scroll(0, 0);
      if (gfx_driver->exit)
	 gfx_driver->exit(screen);
      destroy_bitmap(screen);
      gfx_driver = NULL;
      screen = NULL;
   }

   if (card == GFX_TEXT) {
      r.x.ax = a_rez;
      __dpmi_int(0x10, &r);
      return 0;
   }

   switch (card) {
      case GFX_VGA:        gfx_driver = &gfx_vga;        break;
      case GFX_VESA1:      gfx_driver = &gfx_vesa_1;     break;
      case GFX_VESA2B:     gfx_driver = &gfx_vesa_2b;    break;
      case GFX_VESA2L:     gfx_driver = &gfx_vesa_2l;    break;
      case GFX_CIRRUS64:   gfx_driver = &gfx_cirrus64;   break;
      case GFX_CIRRUS54:   gfx_driver = &gfx_cirrus54;   break;
      case GFX_S3:         gfx_driver = &gfx_s3;         break;
      case GFX_ET3000:     gfx_driver = &gfx_et3000;     break;
      case GFX_ET4000:     gfx_driver = &gfx_et4000;     break;
      default:             gfx_driver = NULL;            break;
   }

   if (gfx_driver) {
      screen = gfx_driver->init(w, h, v_w, v_h);
   }
   else {
      for (c=0; drivers[c]; c++) {
	 gfx_driver = drivers[c];
	 screen = gfx_driver->init(w, h, v_w, v_h);
	 if (screen)
	    break;
      }
   }

   if (screen) {
      setup_bitmap_tables(screen->h);
      clear(screen);
      _set_mouse_range();
      _go32_dpmi_lock_data(gfx_driver, sizeof(GFX_DRIVER));
      return 0;
   }
   else {
      gfx_driver = NULL;
      screen = NULL;
      return -1;
   }
} 



/* _sort_out_virtual_width:
 *  Decides how wide the virtual screen really needs to be. That is more 
 *  complicated than it sounds, because the Allegro graphics primitives 
 *  require that each scanline be contained within a single bank. That 
 *  causes problems on cards that don't have overlapping banks, unless the 
 *  bank size is a multiple of the virtual width. So we may need to adjust 
 *  the width just to keep things running smoothly...
 */
void _sort_out_virtual_width(int *width, GFX_DRIVER *driver)
{
   int w = *width;

   /* hah! I love these things... */
   if (driver->linear)
      return;

   /* if banks can overlap, we are ok... */ 
   if (driver->bank_size > driver->bank_gran)
      return;

   /* damn, have to increase the virtual width */
   while (((driver->bank_size / w) * w) != driver->bank_size) {
      w++;
      if (w > driver->bank_size)
	 break; /* oh shit */
   }

   *width = w;
}



/* _make_bitmap:
 *  Helper function for creating the screen bitmap. Sets up a bitmap 
 *  structure for addressing video memory at addr, and fills the bank 
 *  switching table using bank size/granularity information from the 
 *  specified graphics driver.
 */
BITMAP *_make_bitmap(int w, int h, unsigned long addr, GFX_DRIVER *driver)
{
   BITMAP *b;
   int i;
   int bank;
   int size;

   LOCK_VARIABLE(_last_bank_1);
   LOCK_VARIABLE(_last_bank_2);
   LOCK_VARIABLE(_gfx_bank);
   LOCK_FUNCTION(_stub_bank_switch);

   size = sizeof(BITMAP) + sizeof(char *) * h;

   b = (BITMAP *)malloc(size);
   if (!b)
      return NULL;

   _go32_dpmi_lock_data(b, size);

   b->w = b->cr = w;
   b->h = b->cb = h;
   b->clip = TRUE;
   b->cl = b->ct = 0;
   b->dat = NULL;
   b->seg = _dos_ds;
   b->write_bank = _stub_bank_switch;
   b->read_bank = _stub_bank_switch;

   _last_bank_1 = _last_bank_2 = 0;

   setup_bitmap_tables(h);

   b->line[0] = (char *)addr;
   _gfx_bank[0] = 0;

   if (driver->linear) {
      for (i=1; i<h; i++) {
	 b->line[i] = b->line[i-1] + w;
	 _gfx_bank[i] = 0;
      }
   }
   else {
      bank = 0;

      for (i=1; i<h; i++) {
	 b->line[i] = b->line[i-1] + w;
	 if (b->line[i]+w-1 >= (unsigned char *)addr + driver->bank_size) {
	    while (b->line[i] >= (unsigned char *)addr + driver->bank_gran) {
	       b->line[i] -= driver->bank_gran;
	       bank++;
	    }
	 }
	 _gfx_bank[i] = bank;
      }
   }

   return b;
}



/* create_bitmap:
 *  Creates a new memory bitmap.
 */
BITMAP *create_bitmap(int width, int height)
{
   BITMAP *bitmap;
   int i;

   setup_bitmap_tables(height);     /* do we need a bigger fill array? */

   bitmap = malloc(sizeof(BITMAP) + (sizeof(char *) * height));
   if (!bitmap)
      return NULL;

   bitmap->dat = malloc(width * height);
   if (!bitmap->dat) {
      free(bitmap);
      return NULL;
   }

   bitmap->w = bitmap->cr = width;
   bitmap->h = bitmap->cb = height;
   bitmap->clip = TRUE;
   bitmap->cl = bitmap->ct = 0;
   bitmap->seg = _my_ds();
   bitmap->write_bank = _stub_bank_switch;
   bitmap->read_bank = _stub_bank_switch;

   bitmap->line[0] = bitmap->dat;
   for (i=1; i<height; i++)
      bitmap->line[i] = bitmap->line[i-1] + width;

   return bitmap;
}



/* destroy_bitmap:
 *  Destroys a memory bitmap.
 */
void destroy_bitmap(BITMAP *bitmap)
{
   if (bitmap) {
      if (bitmap->dat)
	 free(bitmap->dat);
      free(bitmap);
   }
}



/* set_clip:
 *  Sets the two opposite corners of the clipping rectangle to be used when
 *  drawing to the bitmap. Nothing will be drawn to positions outside of this 
 *  rectangle. When a new bitmap is created the clipping rectangle will be 
 *  set to the full area of the bitmap. If x1, y1, x2 and y2 are all zero 
 *  clipping will be turned off, which will slightly speed up drawing 
 *  operations but will allow memory to be corrupted if you attempt to draw 
 *  off the edge of the bitmap.
 */
void set_clip(BITMAP *bitmap, int x1, int y1, int x2, int y2)
{
   int t;

   if ((x1==0) && (y1==0) && (x2==0) && (y2==0)) {
      bitmap->clip = FALSE;
      bitmap->cl = bitmap->ct = 0;
      bitmap->cr = SCREEN_W;
      bitmap->cb = SCREEN_H;
      return;
   }

   if (x2 < x1) {
      t = x1;
      x1 = x2;
      x2 = t;
   }

   if (y2 < y1) {
      t = y1;
      y1 = y2;
      y2 = t;
   }

   x2++;
   y2++;

   bitmap->clip = TRUE;
   bitmap->cl = MID(0, x1, bitmap->w-1);
   bitmap->ct = MID(0, y1, bitmap->h-1);
   bitmap->cr = MID(0, x2, bitmap->w);
   bitmap->cb = MID(0, y2, bitmap->h);
}



/* scroll_screen:
 *  Attempts to scroll the hardware screen, returning 0 on success. 
 *  Check the VIRTUAL_W and VIRTUAL_H values to see how far the screen
 *  can be scrolled. Note that a lot of VESA drivers can only handle
 *  horizontal scrolling in four pixel increments.
 */
int scroll_screen(int x, int y)
{
   int ret = 0;

   /* can driver handle hardware scrolling? */
   if (!gfx_driver->scroll)
      return -1;

   /* clip x */
   if (x < 0) {
      x = 0;
      ret = -1;
   }
   else if (x > (VIRTUAL_W - SCREEN_W)) {
      x = VIRTUAL_W - SCREEN_W;
      ret = -1;
   }

   /* clip y */
   if (y < 0) {
      y = 0;
      ret = -1;
   }
   else if (y > (VIRTUAL_H - SCREEN_H)) {
      y = VIRTUAL_H - SCREEN_H;
      ret = -1;
   }

   gfx_driver->scroll(x, y);
   return ret;
}



/* xor_mode:
 *  Enables or disables exclusive-or drawing mode (defaults to off). In xor 
 *  mode, colors are written to the bitmap with an exclusive-or operation 
 *  rather than a simple copy, so drawing the same shape twice will erase it. 
 *  This only affects drawing routines like putpixel, lines, rectangles, 
 *  triangles, etc, not the blitting or sprite drawing functions.
 */
void xor_mode(int xor)
{
   _xor = xor;
}



/* clear:
 *  Clears the bitmap to color 0.
 */
void clear(BITMAP *bitmap)
{
   clear_to_color(bitmap, 0);
}



/* get_pallete:
 *  Retrieves the color pallete. You should provide an array of 256 RGB 
 *  structures to store it in.
 */
void get_pallete(RGB *p)
{
   int c;

   for (c=0; c<PAL_SIZE; c++)
      get_color(c, p+c);
}



/* fade_in:
 *  Fades the pallete gradually from a black screen to the specified pallete.
 *  Speed is from 1 (the slowest) up to 64 (instant).
 */
void fade_in(RGB *p, int speed)
{
   PALLETE temp;
   int i, c;

   set_pallete(black_pallete);

   for (c=0; c<64; c+=speed) {             /* do the fade */
      for (i=0; i < PAL_SIZE; i++) { 
	 temp[i].r = ((int)p[i].r * (int)c) / 64;
	 temp[i].g = ((int)p[i].g * (int)c) / 64;
	 temp[i].b = ((int)p[i].b * (int)c) / 64;
      }
      set_pallete(temp);                  /* twice to get the speed right */
      set_pallete(temp);
   }

   set_pallete(p);                        /* just in case... */
}



/* fade_out:
 *  Fades the pallete gradually from the current pallete to a black screen.
 *  Speed is from 1 (the slowest) up to 64 (instant).
 */
void fade_out(int speed)
{
   PALLETE temp, p;
   int i, c;

   get_pallete(p);

   for (c=64; c>0; c-=speed) {            /* do the fade */
      for (i=0; i < PAL_SIZE; i++) { 
	 temp[i].r = ((int)p[i].r * (int)c) / 64;
	 temp[i].g = ((int)p[i].g * (int)c) / 64;
	 temp[i].b = ((int)p[i].b * (int)c) / 64;
      }
      set_pallete(temp);                  /* twice to get the speed right */
      set_pallete(temp);
   }

   set_pallete(black_pallete);            /* just in case... */
}



/* stretch_blit:
 *  Like blit(), except it can scale images so the source and destination 
 *  rectangles don't need to be the same size. This routine doesn't do as 
 *  much safety checking as the regular blit: in particular you must take 
 *  care not to copy from areas outside the source bitmap, and you cannot 
 *  blit between overlapping regions, ie. you must use different bitmaps for 
 *  the source and the destination.
 *
 *  This routine does some very dodgy stuff. It dynamically generates code
 *  to scale a line of the bitmap, and then calls this. I just _had_ to
 *  use self modifying code _somewhere_ in Allegro :-) Doing it like this
 *  is marginally faster than using a 'normal' method, and much more fun...
 */
void stretch_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height)
{
   static unsigned char *stretcher = NULL;
   static int stretcher_size = 0;

   /* adds a byte to the dynamically generated stretch routine */
   #define STRETCHER_BYTE(val) {                                             \
      if (stretcher_pos >= stretcher_size) {                                 \
	 stretcher_size = stretcher_pos+64;                                  \
	 stretcher = realloc(stretcher, stretcher_size);                     \
      }                                                                      \
      stretcher[stretcher_pos++] = val;                                      \
   }

   /* adds a long to the dynamically generated stretch routine */
   #define STRETCHER_LONG(val) {                                             \
      if (stretcher_pos+sizeof(long) > stretcher_size) {                     \
	 stretcher_size = stretcher_pos+64;                                  \
	 stretcher = realloc(stretcher, stretcher_size);                     \
      }                                                                      \
      *((long *)(stretcher+stretcher_pos)) = val;                            \
      stretcher_pos += sizeof(long);                                         \
   }

   /* opcodes to write into the stretch routine: */

   #define STRETCHER_INC_ESI() {                                             \
      STRETCHER_BYTE(0x46);      /* incl %esi */                             \
   }

   #define STRETCHER_INC_EDI() {                                             \
      STRETCHER_BYTE(0x47);      /* incl %edi */                             \
   }

   #define STRETCHER_ADD_ESI(val) {                                          \
      STRETCHER_BYTE(0x81);      /* addl $val, %esi */                       \
      STRETCHER_BYTE(0xC6);                                                  \
      STRETCHER_LONG(val);                                                   \
   }

   #define STRETCHER_MOV_ECX(val) {                                          \
      STRETCHER_BYTE(0xB9);      /* movl $val, %ecx */                       \
      STRETCHER_LONG(val);                                                   \
   }

   #define STRETCHER_REP_MOVSB() {                                           \
      STRETCHER_BYTE(0xF2);      /* rep */                                   \
      STRETCHER_BYTE(0xA4);      /* movsb */                                 \
   }

   #define STRETCHER_LODSB() {                                               \
      STRETCHER_BYTE(0x8A);      /* movb (%esi), %al */                      \
      STRETCHER_BYTE(0x06);                                                  \
      STRETCHER_INC_ESI();       /* incl %esi */                             \
   }

   #define STRETCHER_STOSB() {                                               \
      STRETCHER_BYTE(0x26);      /* movb %al, %es:(%edi) */                  \
      STRETCHER_BYTE(0x88);                                                  \
      STRETCHER_BYTE(0x07);                                                  \
      STRETCHER_INC_EDI();       /* incl %edi */                             \
   }

   #define STRETCHER_MOVSB() {                                               \
      STRETCHER_LODSB();                                                     \
      STRETCHER_STOSB();                                                     \
   }

   #define STRETCHER_RET() {                                                 \
      STRETCHER_BYTE(0xC3);      /* ret */                                   \
   }

   /* and now for some real code */

   int stretcher_pos = 0;

   fixed sx, sy, sxd, syd;
   int x, x2;

   if ((source_width <= 0) || (source_height <= 0) || 
       (dest_width <= 0) || (dest_height <= 0))
      return;

   sx = itofix(source_x);
   sy = itofix(source_y);

   sxd = itofix(source_width) / dest_width;
   syd = itofix(source_height) / dest_height;

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

   source_x = sx >> 16;

   if (sxd == itofix(1)) {                /* easy case for 1 -> 1 scaling */
      STRETCHER_MOV_ECX(dest_width);
      STRETCHER_REP_MOVSB();
   }
   else if (sxd > itofix(1)) {            /* big -> little scaling */
      for (x=0; x<dest_width; x++) {
	 STRETCHER_MOVSB();
	 x2 = (sx >> 16) + 1;
	 sx += sxd;
	 x2 = (sx >> 16) - x2;
	 if (x2 > 1) {
	    STRETCHER_ADD_ESI(x2);
	 }
	 else if (x2 == 1)
	    STRETCHER_INC_ESI();
      }
   }
   else  {                                /* little -> big scaling */
      x2 = sx >> 16;
      STRETCHER_LODSB();
      for (x=0; x<dest_width; x++) {
	 STRETCHER_STOSB();
	 sx += sxd;
	 if ((sx >> 16) > x2) {
	    STRETCHER_LODSB();
	    x2++;
	 }
      }
   }

   STRETCHER_RET();

   _do_stretch(source, dest, stretcher, source_x, sy, syd, 
	       dest_x, dest_y, dest_height);
}



/* rotate_sprite:
 *  Draws the sprite image onto the bitmap at the specified position, 
 *  rotating it by the specified angle. The angle is a fixed point 16.16 
 *  number in the same format used by the fixed point trig routines, with
 *  256 equal to a full circle, 64 a right angle, etc.
 */
void rotate_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, fixed angle)
{
   fixed f1x, f1y, f1xd, f1yd;
   fixed f2x, f2y, f2xd, f2yd;
   fixed w, h, dist;
   int dx, dy;
   int sx, sy;
   unsigned long addr;
   int wgap = sprite->w;
   int hgap = sprite->h;

   /* rotate the top left pixel of the sprite */
   w = itofix(wgap/2);
   h = itofix(hgap/2);
   dist = fsqrt(fmul(w, w) + fmul(h, h));
   f1x = w - fmul(dist, fcos(itofix(32) - angle));
   f1y = h - fmul(dist, fsin(itofix(32) - angle));

   /* map the destination y axis onto the sprite */
   f1xd = fcos(itofix(64) - angle);
   f1yd = fsin(itofix(64) - angle);

   /* map the destination x axis onto the sprite */
   f2xd = fcos(-angle);
   f2yd = fsin(-angle);

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

      f1x += f1xd;
      f1y += f1yd;
   }
}



/* rect:
 *  Draws an outline rectangle.
 */
void rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   hline(bmp, x1, y1, x2, color);
   hline(bmp, x1, y2, x2, color);
   vline(bmp, x1, y1+1, y2-1, color);
   vline(bmp, x2, y1+1, y2-1, color);
}



/* rectfill:
 *  Draws a solid filled rectangle.
 */
void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   int t;

   if (y1 > y2) {
      t = y1;
      y1 = y2;
      y2 = t;
   }

   while (y1 <= y2) {
      hline(bmp, x1, y1, x2, color);
      y1++;
   };
}



/* line:
 *  Draws a line from x1, y1 to x2, y2.
 */
void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   if (x1 == x2)
      vline(bmp, x1, y1, y2, color);
   else if (y1 == y2)
      hline(bmp, x1, y1, x2, color);
   else
      do_line(bmp, x1, y1, x2, y2, color, putpixel);
}



/* triangle:
 *  Draws a filled triangle between the three points.
 */
void triangle(BITMAP *bmp, int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
   _fill_init(bmp);
   _fill_line(x1, y1, x2, y2, bmp->ct, bmp->cb);
   _fill_line(x2, y2, x3, y3, bmp->ct, bmp->cb);
   _fill_line(x3, y3, x1, y1, bmp->ct, bmp->cb);
   _fill_finish(bmp, color);
}



/* polygon:
 *  Draws a filled convex polygon with an arbitrary number of corners.
 *  Pass the number of vertices, then a series of x, y points (a total
 *  of vertices*2 values). Note that this routine can only draw convex
 *  shapes, not concave ones.
 */
void polygon(BITMAP *bmp, int color, int vertices, ...)
{
   va_list ap;
   int c;
   int ox, oy, x1, y1, x2, y2;

   _fill_init(bmp);

   va_start(ap, vertices);
   ox = x1 = va_arg(ap, int);
   oy = y1 = va_arg(ap, int);

   for (c=1; c<vertices; c++) {
      x2 = va_arg(ap, int);
      y2 = va_arg(ap, int);
      _fill_line(x1, y1, x2, y2, bmp->ct, bmp->cb);
      x1 = x2;
      y1 = y2;
   }

   _fill_line(x1, y1, ox, oy, bmp->ct, bmp->cb);

   va_end(ap);

   _fill_finish(bmp, color);
}



/* do_circle:
 *  Helper function for the circle drawing routines. Calculates the points
 *  in a circle of radius r around point x, y, and calls the specified 
 *  routine for each one. The output proc will be passed first a copy of
 *  the bmp parameter, then the x, y point, then a copy of the d parameter
 *  (so do_circle() can be used with putpixel()).
 */
void do_circle(BITMAP *bmp, int x, int y, int radius, int d, void (*proc)())
{
   int cx, cy, df;

   cx = 0; 
   cy = radius; 
   df = 3 - (radius << 1); 

   do { 
      proc(bmp, x + cx, y + cy, d); 
      proc(bmp, x - cx, y + cy, d); 
      proc(bmp, x + cy, y + cx, d); 
      proc(bmp, x - cy, y + cx, d); 
      proc(bmp, x + cx, y - cy, d); 
      proc(bmp, x - cx, y - cy, d); 
      proc(bmp, x + cy, y - cx, d); 
      proc(bmp, x - cy, y - cx, d); 

      if (df < 0) 
	 df += (cx << 2) + 6; 
      else { 
	 df += ((cx - cy) << 2) + 10; 
	 cy--; 
      } 

      cx++; 

   } while (cx <= cy);
}



/* circle:
 *  Draws a circle.
 */
void circle(BITMAP *bmp, int x, int y, int radius, int color)
{
   do_circle(bmp, x, y, radius, color, putpixel);
}



/* circlefill:
 *  Draws a filled circle.
 */
void circlefill(BITMAP *bmp, int x, int y, int radius, int color)
{
   _fill_init(bmp);
   do_circle(bmp, x, y, radius, color, _fill_putpix);
   _fill_finish(bmp, color);
}



/* _fill_init:
 *  Sets up a fill operation, clearing the fill table.
 */
void _fill_init(BITMAP *bmp)
{
   int y;

   for (y=bmp->ct; y<=bmp->cb; y++) {
      _fill_array[y].lpos = 0x7fff;
      _fill_array[y].rpos = -0x7fff;
   }
}



/* _fill_init:
 *  Ends a fill operation, drawing the lines stored in the fill table.
 */
void _fill_finish(BITMAP *bmp, int color)
{
   int y;

   for (y=bmp->ct; y<=bmp->cb; y++)
      if (_fill_array[y].lpos < _fill_array[y].rpos)
	 hline(bmp, _fill_array[y].lpos, y, _fill_array[y].rpos, color);
}



/* write_to_sprite:
 *  Helper function for RLE compression: writes a byte to the specified
 *  index in the sprite, growing the sprite if needed.
 */
static RLE_SPRITE *write_to_sprite(RLE_SPRITE *s, int pos, int val)
{
   if (pos >= s->size) {
      s->size = pos+64;
      s = realloc(s, sizeof(RLE_SPRITE) + s->size);
   }

   s->dat[pos] = val;
   return s;
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
   int x, y;
   int c;
   int run;

   s = malloc(sizeof(RLE_SPRITE));

   if (s) {
      s->w = bitmap->w;
      s->h = bitmap->h;
      s->size = c = 0;

      for (y=0; y<bitmap->h; y++) { 
	 run = -1;
	 for (x=0; x<bitmap->w; x++) { 
	    if (getpixel(bitmap, x, y)) {
	       if ((run >= 0) && (s->dat[run] > 0) && (s->dat[run] < 127))
		  s->dat[run]++;
	       else {
		  run = c;
		  s = write_to_sprite(s, c++, 1);
	       }
	       s = write_to_sprite(s, c++, getpixel(bitmap, x, y));
	    }
	    else {
	       if ((run >= 0) && (s->dat[run] < 0) && (s->dat[run] > -128))
		  s->dat[run]--;
	       else {
		  run = c;
		  s = write_to_sprite(s, c++, -1);
	       }
	    }
	 }
	 s = write_to_sprite(s, c++, 0);
      }

      s->size = c;
      s = realloc(s, sizeof(RLE_SPRITE) + s->size);
   }

   return s;
}



/* destroy_rle_sprite:
 *  Destroys an RLE sprite structure returned by get_rle_sprite().
 */
void destroy_rle_sprite(RLE_SPRITE *sprite)
{
   free(sprite);
}



static FONT_8x8 _font_8x8 =                     /* the default 8x8 font */
{
   {
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   /* ' ' */
      { 0x18, 0x3c, 0x3c, 0x18, 0x18, 0x00, 0x18, 0x00 },   /* '!' */
      { 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00 },   /* '"' */
      { 0x6c, 0x6c, 0xfe, 0x6c, 0xfe, 0x6c, 0x6c, 0x00 },   /* '#' */
      { 0x18, 0x7e, 0xc0, 0x7c, 0x06, 0xfc, 0x18, 0x00 },   /* '$' */
      { 0x00, 0xc6, 0xcc, 0x18, 0x30, 0x66, 0xc6, 0x00 },   /* '%' */
      { 0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x76, 0x00 },   /* '&' */
      { 0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00 },   /* ''' */
      { 0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00 },   /* '(' */
      { 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00 },   /* ')' */
      { 0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00 },   /* '*' */
      { 0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00 },   /* '+' */
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30 },   /* ',' */
      { 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00 },   /* '-' */
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00 },   /* '.' */
      { 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x00 },   /* '/' */
      { 0x7c, 0xce, 0xde, 0xf6, 0xe6, 0xc6, 0x7c, 0x00 },   /* '0' */
      { 0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x00 },   /* '1' */
      { 0x78, 0xcc, 0x0c, 0x38, 0x60, 0xcc, 0xfc, 0x00 },   /* '2' */
      { 0x78, 0xcc, 0x0c, 0x38, 0x0c, 0xcc, 0x78, 0x00 },   /* '3' */
      { 0x1c, 0x3c, 0x6c, 0xcc, 0xfe, 0x0c, 0x1e, 0x00 },   /* '4' */
      { 0xfc, 0xc0, 0xf8, 0x0c, 0x0c, 0xcc, 0x78, 0x00 },   /* '5' */
      { 0x38, 0x60, 0xc0, 0xf8, 0xcc, 0xcc, 0x78, 0x00 },   /* '6' */
      { 0xfc, 0xcc, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00 },   /* '7' */
      { 0x78, 0xcc, 0xcc, 0x78, 0xcc, 0xcc, 0x78, 0x00 },   /* '8' */
      { 0x78, 0xcc, 0xcc, 0x7c, 0x0c, 0x18, 0x70, 0x00 },   /* '9' */
      { 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00 },   /* ':' */
      { 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30 },   /* ';' */
      { 0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x00 },   /* '<' */
      { 0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00 },   /* '=' */
      { 0x60, 0x30, 0x18, 0x0c, 0x18, 0x30, 0x60, 0x00 },   /* '>' */
      { 0x3c, 0x66, 0x0c, 0x18, 0x18, 0x00, 0x18, 0x00 },   /* '?' */
      { 0x7c, 0xc6, 0xde, 0xde, 0xdc, 0xc0, 0x7c, 0x00 },   /* '@' */
      { 0x30, 0x78, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0x00 },   /* 'A' */
      { 0xfc, 0x66, 0x66, 0x7c, 0x66, 0x66, 0xfc, 0x00 },   /* 'B' */
      { 0x3c, 0x66, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x00 },   /* 'C' */
      { 0xf8, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0xf8, 0x00 },   /* 'D' */
      { 0xfe, 0x62, 0x68, 0x78, 0x68, 0x62, 0xfe, 0x00 },   /* 'E' */
      { 0xfe, 0x62, 0x68, 0x78, 0x68, 0x60, 0xf0, 0x00 },   /* 'F' */
      { 0x3c, 0x66, 0xc0, 0xc0, 0xce, 0x66, 0x3a, 0x00 },   /* 'G' */
      { 0xcc, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0xcc, 0x00 },   /* 'H' */
      { 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 },   /* 'I' */
      { 0x1e, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78, 0x00 },   /* 'J' */
      { 0xe6, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0xe6, 0x00 },   /* 'K' */
      { 0xf0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xfe, 0x00 },   /* 'L' */
      { 0xc6, 0xee, 0xfe, 0xfe, 0xd6, 0xc6, 0xc6, 0x00 },   /* 'M' */
      { 0xc6, 0xe6, 0xf6, 0xde, 0xce, 0xc6, 0xc6, 0x00 },   /* 'N' */
      { 0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00 },   /* 'O' */
      { 0xfc, 0x66, 0x66, 0x7c, 0x60, 0x60, 0xf0, 0x00 },   /* 'P' */
      { 0x7c, 0xc6, 0xc6, 0xc6, 0xd6, 0x7c, 0x0e, 0x00 },   /* 'Q' */
      { 0xfc, 0x66, 0x66, 0x7c, 0x6c, 0x66, 0xe6, 0x00 },   /* 'R' */
      { 0x7c, 0xc6, 0xe0, 0x78, 0x0e, 0xc6, 0x7c, 0x00 },   /* 'S' */
      { 0xfc, 0xb4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 },   /* 'T' */
      { 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xfc, 0x00 },   /* 'U' */
      { 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00 },   /* 'V' */
      { 0xc6, 0xc6, 0xc6, 0xc6, 0xd6, 0xfe, 0x6c, 0x00 },   /* 'W' */
      { 0xc6, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0xc6, 0x00 },   /* 'X' */
      { 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x30, 0x78, 0x00 },   /* 'Y' */
      { 0xfe, 0xc6, 0x8c, 0x18, 0x32, 0x66, 0xfe, 0x00 },   /* 'Z' */
      { 0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00 },   /* '[' */
      { 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x02, 0x00 },   /* '\' */
      { 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00 },   /* ']' */
      { 0x10, 0x38, 0x6c, 0xc6, 0x00, 0x00, 0x00, 0x00 },   /* '^' */
      { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff },   /* '_' */
      { 0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00 },   /* '`' */
      { 0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00 },   /* 'a' */
      { 0xe0, 0x60, 0x60, 0x7c, 0x66, 0x66, 0xdc, 0x00 },   /* 'b' */
      { 0x00, 0x00, 0x78, 0xcc, 0xc0, 0xcc, 0x78, 0x00 },   /* 'c' */
      { 0x1c, 0x0c, 0x0c, 0x7c, 0xcc, 0xcc, 0x76, 0x00 },   /* 'd' */
      { 0x00, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00 },   /* 'e' */
      { 0x38, 0x6c, 0x64, 0xf0, 0x60, 0x60, 0xf0, 0x00 },   /* 'f' */
      { 0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8 },   /* 'g' */
      { 0xe0, 0x60, 0x6c, 0x76, 0x66, 0x66, 0xe6, 0x00 },   /* 'h' */
      { 0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00 },   /* 'i' */
      { 0x0c, 0x00, 0x1c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78 },   /* 'j' */
      { 0xe0, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0xe6, 0x00 },   /* 'k' */
      { 0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 },   /* 'l' */
      { 0x00, 0x00, 0xcc, 0xfe, 0xfe, 0xd6, 0xd6, 0x00 },   /* 'm' */
      { 0x00, 0x00, 0xb8, 0xcc, 0xcc, 0xcc, 0xcc, 0x00 },   /* 'n' */
      { 0x00, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0x78, 0x00 },   /* 'o' */
      { 0x00, 0x00, 0xdc, 0x66, 0x66, 0x7c, 0x60, 0xf0 },   /* 'p' */
      { 0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0x1e },   /* 'q' */
      { 0x00, 0x00, 0xdc, 0x76, 0x62, 0x60, 0xf0, 0x00 },   /* 'r' */
      { 0x00, 0x00, 0x7c, 0xc0, 0x70, 0x1c, 0xf8, 0x00 },   /* 's' */
      { 0x10, 0x30, 0xfc, 0x30, 0x30, 0x34, 0x18, 0x00 },   /* 't' */
      { 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x76, 0x00 },   /* 'u' */
      { 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00 },   /* 'v' */
      { 0x00, 0x00, 0xc6, 0xc6, 0xd6, 0xfe, 0x6c, 0x00 },   /* 'w' */
      { 0x00, 0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00 },   /* 'x' */
      { 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8 },   /* 'y' */
      { 0x00, 0x00, 0xfc, 0x98, 0x30, 0x64, 0xfc, 0x00 },   /* 'z' */
      { 0x1c, 0x30, 0x30, 0xe0, 0x30, 0x30, 0x1c, 0x00 },   /* '{' */
      { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00 },   /* '|' */
      { 0xe0, 0x30, 0x30, 0x1c, 0x30, 0x30, 0xe0, 0x00 },   /* '}' */
      { 0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }    /* '~' */
   }
};


static FONT _font =
{
   TRUE,
   {
      &_font_8x8
   }
};


FONT *font = &_font;

int _textmode = 0;



/* text_mode:
 *  Sets the mode in which text will be drawn. If mode is positive, text
 *  output will be opaque and the background will be set to mode. If mode
 *  is negative, text will be drawn transparently (ie. the background will
 *  not be altered). The default is a mode of zero.
 */
void text_mode(int mode)
{
   if (mode < 0)
      _textmode = -1;
   else
      _textmode = mode;
}



/* blit_character:
 *  Helper routine for opaque multicolor output of proportional fonts.
 */
static void blit_character(BITMAP *bmp, BITMAP *sprite, int x, int y)
{
   blit(sprite, bmp, 0, 0, x, y, sprite->w, sprite->h);
}



/* textout:
 *  Writes the null terminated string str onto bmp at position x, y, using 
 *  the current text mode and the specified font and foreground color.
 *  If color is -1 and a proportional font is in use, it will be drawn
 *  using the colors from the original font bitmap (the one imported into
 *  the grabber program), which allows multicolored text output.
 */
void textout(BITMAP *bmp, FONT *f, char *str, int x, int y, int color)
{
   FONT_PROP *fp;
   BITMAP *b;
   int c;
   void (*putter)();

   if (f->flag_8x8) {
      _textout_8x8(bmp, f->dat.dat_8x8, str, x, y, color);
      return;
   }

   fp = f->dat.dat_prop;

   if (color < 0) {
      if (_textmode < 0)
	 putter = draw_sprite;         /* masked multicolor output */
      else
	 putter = blit_character;      /* opaque multicolor output */
   }
   else
      putter = _draw_character;        /* single color output */

   while (*str) {
      c = (int)*str - ' ';
      if (c < 0)
	 c = 0;
      else {
	 if (c >= FONT_SIZE)
	    c = FONT_SIZE-1;
      }
      b = fp->dat[c];
      putter(bmp, b, x, y, color);
      x += b->w;
      if (x >= bmp->cr)
	 return;
      str++;
   }
}



/* textout_centre:
 *  Like textout(), but uses the x coordinate as the centre rather than 
 *  the left of the string.
 */
void textout_centre(BITMAP *bmp, FONT *f, char *str, int x, int y, int color)
{
   int len;

   len = text_length(f, str);
   textout(bmp, f, str, x - len/2, y, color);
}



/* text_length:
 *  Calculates the length of a string in a particular font.
 */
int text_length(FONT *f, char *str)
{
   FONT_PROP *fp;
   int c;
   int len;

   if (f->flag_8x8)
      return strlen(str) * 8;

   fp = f->dat.dat_prop;
   len = 0;

   while (*str) {
      c = (int)*str - ' ';
      if (c < 0)
	 c = 0;
      else {
	 if (c >= FONT_SIZE)
	    c = FONT_SIZE-1;
      }
      len += fp->dat[c]->w;
      str++;
   }

   return len;
}



/* destroy_font:
 *  Frees the memory being used by a font structure.
 */
void destroy_font(FONT *f)
{
   FONT_PROP *fp;
   int c;

   if (f) {
      if (f->flag_8x8) {
	 /* free 8x8 font */
	 if (f->dat.dat_8x8)
	    free(f->dat.dat_8x8);
      }
      else {
	 /* free proportional font */
	 fp = f->dat.dat_prop;
	 if (fp) {
	    for (c=0; c<FONT_SIZE; c++) {
	       if (fp->dat[c])
		  destroy_bitmap(fp->dat[c]);
	    }
	   free(fp); 
	 }
      }
      free(f);
   }
}


