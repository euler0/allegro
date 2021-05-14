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
 *      Color manipulation routines (blending, format conversion, lighting
 *      table construction, etc).
 *
 *      Dave Thomson contributed the RGB <-> HSV conversion routines.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <math.h>
#include <limits.h>

#include "allegro.h"



/* makecol:
 *  Converts R, G, and B values (ranging 0-255) to whatever pixel format
 *  is required by the current video mode.
 */
int makecol(int r, int g, int b)
{
   switch (_color_depth) {

      case 8:
	 return makecol8(r, g, b);

      case 15:
	 return makecol15(r, g, b);

      case 16:
	 return makecol16(r, g, b);

      case 32:
	 return makecol32(r, g, b);
   }

   return 0;
}



/* getr:
 *  Extracts the red component (ranging 0-255) from a pixel in the format
 *  being used by the current video mode.
 */
int getr(int c)
{
   switch (_color_depth) {

      case 8:
	 return getr8(c);

      case 15:
	 return getr15(c);

      case 16:
	 return getr16(c);

      case 32:
	 return getr32(c);
   }

   return 0;
}



/* getg:
 *  Extracts the green component (ranging 0-255) from a pixel in the format
 *  being used by the current video mode.
 */
int getg(int c)
{
   switch (_color_depth) {

      case 8:
	 return getg8(c);

      case 15:
	 return getg15(c);

      case 16:
	 return getr16(c);

      case 32:
	 return getr32(c);
   }

   return 0;
}



/* getb:
 *  Extracts the blue component (ranging 0-255) from a pixel in the format
 *  being used by the current video mode.
 */
int getb(int c)
{
   switch (_color_depth) {

      case 8:
	 return getb8(c);

      case 15:
	 return getb15(c);

      case 16:
	 return getr16(c);

      case 32:
	 return getr32(c);
   }

   return 0;
}



/* 1.5k lookup table for color matching */
static unsigned col_diff[3*128]; 



/* bestfit_init:
 *  Color matching is done with weighted squares, which are much faster
 *  if we pregenerate a little lookup table...
 */
static void bestfit_init()
{
   int i;

   for (i=1; i<64; i++) {
      int k = i * i;
      col_diff[0  +i] = col_diff[0  +128-i] = k * (59 * 59);
      col_diff[128+i] = col_diff[128+128-i] = k * (30 * 30);
      col_diff[256+i] = col_diff[256+128-i] = k * (11 * 11);
   }
}



/* bestfit_color:
 *  Searches a palette for the color closest to the requested R, G, B value.
 */
static int bestfit_color(PALLETE pal, int r, int g, int b)
{
   int i, coldiff, lowest, bestfit;

   if (col_diff[1] == 0)
      bestfit_init();

   bestfit = 0;
   lowest = INT_MAX;

   for (i=1; i<PAL_SIZE; i++) {
      RGB *rgb = &pal[i];
      coldiff = (col_diff + 0) [ (rgb->g - g) & 0x7F ];
      if (coldiff < lowest) {
	 coldiff += (col_diff + 128) [ (rgb->r - r) & 0x7F ];
	 if (coldiff < lowest) {
	    coldiff += (col_diff + 256) [ (rgb->b - b) & 0x7F ];
	    if (coldiff < lowest) {
	       bestfit = rgb - pal;    /* faster than `bestfit = i;' */
	       if (coldiff == 0)
		  return bestfit;
	       lowest = coldiff;
	    }
	 }
      }
   }

   return bestfit;
}



/* makecol8: 
 *  Converts R, G, and B values (ranging 0-255) to an 8 bit paletted color.
 *  If the global rgb_map table is initialised, it uses that, otherwise
 *  it searches through the current palette to find the best match.
 */
int makecol8(int r, int g, int b)
{
   if (rgb_map)
      return rgb_map->data[r>>3][g>>3][b>>3];
   else
      return bestfit_color(_current_pallete, r>>2, g>>2, b>>2);
}



/* hsv_to_rgb:
 *  Converts from HSV colorspace to RGB values.
 */
void hsv_to_rgb (float h, float s, float v, RGB *rgb)
{
   int i, a, b, c, f;

   if (s == 0.0) {   /* greyscale */
      rgb->r = rgb->g = rgb->b = (int)(v * 64.0);
   }
   else {
      if (h == 1.0)
	 h = 0.0;

      i = (int)(h);
      f = h - i;
      a = (int)(v * (1 - s) * 64.0);
      b = (int)(v * (1 - (s * f)) * 64.0);
      c = (int)(v * (1 - (s * (1 - f))) * 64.0);

      switch (i) {
	 case 0:  rgb->r = v;    rgb->g = c;    rgb->b = a;    break;
	 case 1:  rgb->r = b;    rgb->g = v;    rgb->b = a;    break;
	 case 2:  rgb->r = a;    rgb->g = v;    rgb->b = c;    break;
	 case 3:  rgb->r = a;    rgb->g = b;    rgb->b = v;    break;
	 case 4:  rgb->r = c;    rgb->g = a;    rgb->b = v;    break;
	 case 5:  rgb->r = c;    rgb->g = a;    rgb->b = b;    break;
      }
   }
}



/* rgb_to_hsv:
 *  Converts an RGB value into the HSV colorspace.
 */
void rgb_to_hsv(RGB *rgb, float *h, float *s, float *v)
{
   float min, max, delta, rc, gc, bc;

   rc = (float)(rgb->r) / 64.0;
   gc = (float)(rgb->g) / 64.0;
   bc = (float)(rgb->b) / 64.0;
   max = MAX(rc, MAX(gc, bc));
   min = MIN(rc, MIN(gc, bc));
   delta = max - min;

   *v = max;
   if (max != 0.0)
      *s = delta / max;
   else
      *s = 0.0;

   if (*s == 0.0)
      *h = -1.0;        /* colour has no hue */
   else {
      if (rc == max)
	 *h = (gc - bc) / delta;
      else if (gc == max)
	 *h = 2 + (bc - rc) / delta;
      else if (bc == max)
	 *h = 4 + (rc - gc) / delta;
      *h *= 60.0;       /* turn it into a 360 degree angle */
      if (*h < 0.0)
	 *h /= 360.0;
    }
}



/* create_rgb_table:
 *  Fills an RGB_MAP lookup table with conversion data for the specified
 *  palette. If the callback function is not NULL, it will be called 256
 *  times during the calculation, passed a position value which will 
 *  increment from 0 to 255, allowing you to display a progress indicator.
 */
void create_rgb_table(RGB_MAP *table, PALLETE pal, void (*callback)(int pos))
{
   int r, g, b;

   for (r=0; r<32; r++) {
      for (g=0; g<32; g++) {
	 for (b=0; b<32; b++) {
	    table->data[r][g][b] = bestfit_color(pal, r*2, g*2, b*2);
	 }
	 if ((!(g&3)) && (callback))
	    (*callback)((r*32+g)>>2);
      }
   }
}



/* create_light_table:
 *  Constructs a lighting color table for the specified palette. At light
 *  intensity 255 the table will produce the palette colors directly, and
 *  at level 0 it will produce the specified R, G, B value for all colors
 *  (this is specified in 0-63 VGA format). If the callback function is 
 *  not NULL, it will be called 256 times during the calculation, allowing
 *  you to display a progress indicator.
 */
void create_light_table(COLOR_MAP *table, PALLETE pal, int r, int g, int b, void (*callback)(int pos))
{
   int x, y;
   RGB c;

   for (x=0; x<PAL_SIZE; x++) {
      for (y=0; y<PAL_SIZE; y++) {
	 c.r = (r * (255 - x) / 255) + ((int)pal[y].r * x / 255);
	 c.g = (g * (255 - x) / 255) + ((int)pal[y].g * x / 255);
	 c.b = (b * (255 - x) / 255) + ((int)pal[y].b * x / 255);

	 if (rgb_map)
	    table->data[x][y] = rgb_map->data[c.r>>1][c.g>>1][c.b>>1];
	 else
	    table->data[x][y] = bestfit_color(pal, c.r, c.g, c.b);
      }

      if (callback)
	 (*callback)(x);
   }
}



/* create_trans_table:
 *  Constructs a translucency color table for the specified palette. The
 *  r, g, and b parameters specifiy the solidity of each color component,
 *  ranging from 0 (totally transparent) to 255 (totally solid). Source
 *  color #0 is a special case, and is set to leave the destination 
 *  unchanged, so that masked sprites will draw correctly. If the callback 
 *  function is not NULL, it will be called 256 times during the calculation, 
 *  allowing you to display a progress indicator.
 */
void create_trans_table(COLOR_MAP *table, PALLETE pal, int r, int g, int b, void (*callback)(int pos))
{
   int x, y;
   RGB c;

   for (y=0; y<PAL_SIZE; y++)
      table->data[0][y] = y;

   if (callback)
      (*callback)(0);

   for (x=1; x<PAL_SIZE; x++) {
      for (y=0; y<PAL_SIZE; y++) {
	 c.r = ((int)pal[x].r * r / 255) + ((int)pal[y].r * (255 - r) / 255);
	 c.g = ((int)pal[x].g * g / 255) + ((int)pal[y].g * (255 - g) / 255);
	 c.b = ((int)pal[x].b * b / 255) + ((int)pal[y].b * (255 - b) / 255);

	 if (rgb_map)
	    table->data[x][y] = rgb_map->data[c.r>>1][c.g>>1][c.b>>1];
	 else
	    table->data[x][y] = bestfit_color(pal, c.r, c.g, c.b);
      }

      if (callback)
	 (*callback)(x);
   }
}



/* create_color_table:
 *  Creates a color mapping table, using a user-supplied callback to blend
 *  each pair of colors. Your blend routine will be passed a pointer to the
 *  palette and the two colors to be blended (x is the source color, y is
 *  the destination), and should return the desired output RGB for this
 *  combination. If the callback function is not NULL, it will be called 
 *  256 times during the calculation, allowing you to display a progress 
 *  indicator.
 */
void create_color_table(COLOR_MAP *table, PALLETE pal, RGB (*blend)(PALLETE pal, int x, int y), void (*callback)(int pos))
{
   int x, y;
   RGB c;

   for (x=0; x<PAL_SIZE; x++) {
      for (y=0; y<PAL_SIZE; y++) {
	 c = (*blend)(pal, x, y);

	 if (rgb_map)
	    table->data[x][y] = rgb_map->data[c.r>>1][c.g>>1][c.b>>1];
	 else
	    table->data[x][y] = bestfit_color(pal, c.r, c.g, c.b);
      }

      if (callback)
	 (*callback)(x);
   }
}

