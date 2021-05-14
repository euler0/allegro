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
 *      Graphics routines: mode set, pallete fading, stretch/rotate, 
 *      polygons, circles, fonts, etc.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <go32.h>
#include <limits.h>
#include <sys/farptr.h>

#include "allegro.h"
#include "internal.h"
#include "opcodes.h"


typedef struct GFX_DRIVER_INFO      /* info about a graphics driver */
{
   int driver_id;                   /* integer ID defined in allegro.h */
   GFX_DRIVER *driver;              /* the driver structure */
   int autodetect;                  /* set to allow autodetection */
} GFX_DRIVER_INFO;



/* Alter this table to add or remove specific graphics drivers. You may
 * wish to do this to reduce executable size, for instance if your program
 * only uses VGA mode 13h you could remove the mode-X and SVGA drivers.
 *
 * The first field is the integer ID defined in allegro.h, which is passed
 * as the first parameter to set_gfx_mode(). The second is a pointer to
 * the graphics driver structure, and the third is a flag indicating whether
 * it is safe to autodetect this driver. Autodetection takes place if you
 * pass GFX_AUTODETECT to set_gfx_mode(), and works down this list from
 * top to bottom until it finds a suitable driver.
 *
 * The current detection strategy puts the VESA 2.0 drivers above the 
 * register level ones, because these are more likely to work with recent 
 * cards, but prefers the register level drivers to VESA 1.x. Autodetection 
 * of the ATI 18800/28800, ATI mach64, Cirrus 64xx, Trident, Tseng ET3000,
 * and Video-7 drivers is disabled, because these have not yet been 
 * properly tested/debugged. Please mail me if you have access to one of
 * these cards and can help finish the driver!
 */

static GFX_DRIVER_INFO gfx_driver_list[] =
{
   /* driver ID         driver structure     autodetect flag */
   {  GFX_VBEAF,        &gfx_vbeaf,          TRUE },
   {  GFX_VGA,          &gfx_vga,            TRUE },
   {  GFX_MODEX,        &gfx_modex,          TRUE },
   {  GFX_VESA2L,       &gfx_vesa_2l,        TRUE },
   {  GFX_VESA2B,       &gfx_vesa_2b,        TRUE },
   {  GFX_XTENDED,      &gfx_xtended,        FALSE },
   {  GFX_ATI,          &gfx_ati,            FALSE },
   {  GFX_MACH64,       &gfx_mach64,         FALSE },
   {  GFX_CIRRUS64,     &gfx_cirrus64,       FALSE },
   {  GFX_CIRRUS54,     &gfx_cirrus54,       TRUE },
   {  GFX_S3,           &gfx_s3,             TRUE },
   {  GFX_TRIDENT,      &gfx_trident,        FALSE },
   {  GFX_ET3000,       &gfx_et3000,         FALSE },
   {  GFX_ET4000,       &gfx_et4000,         TRUE },
   {  GFX_VIDEO7,       &gfx_video7,         FALSE },
   {  GFX_VESA1,        &gfx_vesa_1,         TRUE },
   {  0,                NULL,                0 }
};



/* table of functions for drawing onto linear bitmaps */
GFX_VTABLE _linear_vtable =
{
   BMP_TYPE_LINEAR,

   _linear_getpixel,
   _linear_putpixel,
   _linear_vline,
   _linear_hline,
   _normal_line,
   _normal_rectfill,
   _linear_draw_sprite,
   _linear_draw_sprite_v_flip,
   _linear_draw_sprite_h_flip,
   _linear_draw_sprite_vh_flip,
   _normal_rotate_sprite,
   _linear_draw_rle_sprite,
   _linear_draw_character,
   _linear_textout_8x8,
   _linear_blit,
   _linear_blit,
   _linear_blit,
   _linear_blit,
   _linear_blit_backward,
   _normal_stretch_blit,
   _linear_clear_to_color
};



static int gfx_virgin = TRUE;       /* first time we've been called? */
static int a_rez = 3;               /* original screen mode */

int _sub_bitmap_id_count = 1;       /* hash value for sub-bitmaps */

int _modex_split_position = 0;      /* has the mode-X screen been split? */



/* lock_bitmap:
 *  Locks all the memory used by a bitmap structure.
 */
void lock_bitmap(BITMAP *bmp)
{
   _go32_dpmi_lock_data(bmp, sizeof(BITMAP) + sizeof(char *) * bmp->h);

   if (bmp->dat)
      _go32_dpmi_lock_data(bmp->dat, bmp->w * bmp->h);
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



/* check_virginity:
 *  Checks if this is the first call to any graphics related function, and
 *  if so does some once-only initialisation.
 */
static void check_virginity()
{
   extern void blit_end(), _linear_blit_end(), _linear_draw_sprite_end();

   __dpmi_regs r;
   int c;

   if (gfx_virgin) {
      LOCK_VARIABLE(_last_bank_1);
      LOCK_VARIABLE(_last_bank_2);
      LOCK_VARIABLE(_gfx_bank);
      LOCK_VARIABLE(_linear_vtable);
      LOCK_FUNCTION(_stub_bank_switch);
      LOCK_FUNCTION(_linear_draw_sprite);
      LOCK_FUNCTION(_linear_blit);
      LOCK_FUNCTION(blit);

      for (c=0; c<256; c++) {       /* store current color pallete */
	 outportb(0x3C7, c);
	 _current_pallete[c].r = inportb(0x3C9);
	 _current_pallete[c].g = inportb(0x3C9);
	 _current_pallete[c].b = inportb(0x3C9);
      }

      r.x.ax = 0x0F00;              /* store current video mode */
      __dpmi_int(0x10, &r);
      a_rez = r.x.ax & 0xFF; 

      _add_exit_func(shutdown_gfx);

      gfx_virgin = FALSE;
   }
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
   int retrace_enabled = _timer_use_retrace;

   check_virginity();

   timer_simulate_retrace(FALSE);
   _modex_split_position = 0;

   /* close down any existing graphics driver */
   if (gfx_driver) {
      bmp_read_line(screen, 0);
      bmp_write_line(screen, 0);

      if (gfx_driver->scroll)
	 gfx_driver->scroll(0, 0);

      if (gfx_driver->exit)
	 gfx_driver->exit(screen);

      destroy_bitmap(screen);
      gfx_driver = NULL;
      screen = NULL;
   }

   /* return to text mode? */
   if (card == GFX_TEXT) {
      r.x.ax = a_rez;
      __dpmi_int(0x10, &r);

      if (_gfx_bank) {
	 free(_gfx_bank);
	 _gfx_bank = NULL;
      }
      return 0;
   }

   /* search table for the requested driver */
   for (c=0; gfx_driver_list[c].driver; c++) {
      if (gfx_driver_list[c].driver_id == card) {
	 gfx_driver = gfx_driver_list[c].driver;
	 break;
      }
   }

   if (gfx_driver) {                               /* specific driver? */
      screen = gfx_driver->init(w, h, v_w, v_h);
   }
   else {                                          /* otherwise autodetect */
      for (c=0; gfx_driver_list[c].driver; c++) {
	 if (gfx_driver_list[c].autodetect) {
	    gfx_driver = gfx_driver_list[c].driver;
	    screen = gfx_driver->init(w, h, v_w, v_h);
	    if (screen)
	       break;
	 }
      }
   }

   if (!screen) {
      gfx_driver = NULL;
      screen = NULL;
      return -1;
   }

   clear(screen);
   _set_mouse_range();
   _go32_dpmi_lock_data(gfx_driver, sizeof(GFX_DRIVER));

   if (retrace_enabled)
      timer_simulate_retrace(TRUE);

   return 0;
} 



/* _gfx_mode_set_helper:
 *  Helper function for native SVGA drivers, does most of the work of setting
 *  up an SVGA mode, using the detect() and get_mode_number() functions in
 *  the driver structure.
 */
BITMAP *_gfx_mode_set_helper(int w, int h, int v_w, int v_h, GFX_DRIVER *driver, int (*detect)(), GFX_MODE_INFO *mode_list, void (*set_width)(int w))
{
   BITMAP *b;
   int mode;
   int height;
   int width;
   __dpmi_regs r;

   if (!(*detect)()) {
      sprintf(allegro_error, "%s not found", driver->name);
      return NULL;
   }

   mode = 0;
   while ((mode_list[mode].w != w) || (mode_list[mode].h != h)) {
      if (!mode_list[mode].bios) {
	 strcpy(allegro_error, "Resolution not supported");
	 return NULL;
      }
      mode++;
   }

   width = MAX(w, v_w);
   _sort_out_virtual_width(&width, driver);
   width = (width + 15) & 0xFFF0;
   height = driver->vid_mem / width;
   if ((width > 1024) || (h > height) || (v_w > width) || (v_h > height)) {
      strcpy(allegro_error, "Virtual screen size too large");
      return NULL;
   }

   b = _make_bitmap(width, height, 0xA0000, driver);
   if (!b)
      return NULL;

   if (mode_list[mode].set_command) { 
      r.x.ax = mode_list[mode].set_command;
      r.x.bx = mode_list[mode].bios;
   }
   else
      r.x.ax = mode_list[mode].bios;

   __dpmi_int(0x10, &r);            /* set gfx mode */

   if (set_width)
      set_width(width);
   else
      _set_vga_virtual_width(w, width);

   driver->w = b->cr = w;
   driver->h = b->cb = h;

   return b;
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

   /* hah! I love VBE 2.0... */
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

   size = sizeof(BITMAP) + sizeof(char *) * h;

   b = (BITMAP *)malloc(size);
   if (!b)
      return NULL;

   _go32_dpmi_lock_data(b, size);

   b->w = b->cr = w;
   b->h = b->cb = h;
   b->clip = TRUE;
   b->cl = b->ct = 0;
   b->vtable = &_linear_vtable;
   b->write_bank = b->read_bank = _stub_bank_switch;
   b->dat = NULL;
   b->bitmap_id = 0;
   b->line_ofs = 0;
   b->seg = _dos_ds;

   _last_bank_1 = _last_bank_2 = -1;

   _gfx_bank = realloc(_gfx_bank, h * sizeof(int));
   _go32_dpmi_lock_data(_gfx_bank, h * sizeof(int));

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
   bitmap->vtable = &_linear_vtable;
   bitmap->write_bank = bitmap->read_bank = _stub_bank_switch;
   bitmap->bitmap_id = 0;
   bitmap->line_ofs = 0;
   bitmap->seg = _my_ds();

   bitmap->line[0] = bitmap->dat;
   for (i=1; i<height; i++)
      bitmap->line[i] = bitmap->line[i-1] + width;

   return bitmap;
}



/* create_sub_bitmap:
 *  Creates a sub bitmap, ie. a bitmap sharing drawing memory with a
 *  pre-existing bitmap, but possibly with different clipping settings.
 *  Usually will be smaller, and positioned at some arbitrary point.
 *
 *  Mark Wodrich is the owner of the brain responsible this hugely useful 
 *  and beautiful function.
 */
BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height)
{
   BITMAP *bitmap;
   int i;

   if (!parent) 
      return NULL;

   if (x < 0) 
      x = 0; 

   if (y < 0) 
      y = 0;

   if (x+width > parent->w) 
      width = parent->w-x;

   if (y+height > parent->h) 
      height = parent->h-y;

   /* get memory for structure and line pointers */
   bitmap = malloc(sizeof(BITMAP) + (sizeof(char *) * height));
   if (!bitmap)
      return NULL;

   bitmap->w = bitmap->cr = width;
   bitmap->h = bitmap->cb = height;
   bitmap->clip = TRUE;
   bitmap->cl = bitmap->ct = 0;
   bitmap->vtable = parent->vtable;
   bitmap->write_bank = parent->write_bank;
   bitmap->read_bank = parent->read_bank;
   bitmap->dat = NULL;
   bitmap->line_ofs = y + parent->line_ofs;
   bitmap->seg = parent->seg;

   /* All bitmaps are created with zero ID's. When a sub-bitmap is created,
    * a unique ID is needed to identify the relationship when blitting from
    * one to the other. This is obtained from the global variable
    * _sub_bitmap_id_count, which provides a sequence of integers (yes I
    * know it will wrap eventually, but not for a long time :-) If the
    * parent already has an ID the sub-bitmap adopts it, otherwise a new
    * ID is given to both the parent and the child.
    */
   if (parent->bitmap_id)
      bitmap->bitmap_id = parent->bitmap_id;
   else
      bitmap->bitmap_id = parent->bitmap_id = _sub_bitmap_id_count++;

   if (is_planar_bitmap(bitmap))
      x /= 4;

   /* setup line pointers: each line points to a line in the parent bitmap */
   for (i=0; i<height; i++)
      bitmap->line[i] = parent->line[y+i] + x;

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
   int h;

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
   else {
      h = (_modex_split_position > 0) ? _modex_split_position : SCREEN_H;
      if (y > (VIRTUAL_H - h)) {
	 y = VIRTUAL_H - h;
	 ret = -1;
      }
   }

   /* scroll! */
   if (gfx_driver->scroll(x, y) != 0)
      ret = -1;

   return ret;
}



/* drawing_mode:
 *  Sets the drawing mode. This only affects routines like putpixel,
 *  lines, rectangles, triangles, etc, not the blitting or sprite
 *  drawing functions.
 */
void drawing_mode(int mode, BITMAP *pattern, int x_anchor, int y_anchor)
{
   _drawing_mode = mode;
   _drawing_pattern = pattern;
   _drawing_x_anchor = x_anchor;
   _drawing_y_anchor = y_anchor;

   if (pattern) {
      _drawing_x_mask = 1; 
      while (_drawing_x_mask < pattern->w)
	 _drawing_x_mask <<= 1;        /* find power of two greater than w */

      if (_drawing_x_mask > pattern->w)
	 _drawing_x_mask >>= 1;        /* round down if required */

      _drawing_x_mask--;               /* convert to AND mask */

      _drawing_y_mask = 1;
      while (_drawing_y_mask < pattern->h)
	 _drawing_y_mask <<= 1;        /* find power of two greater than h */

      if (_drawing_y_mask > pattern->h)
	 _drawing_y_mask >>= 1;        /* round down if required */

      _drawing_y_mask--;               /* convert to AND mask */
   }
   else
      _drawing_x_mask = _drawing_y_mask = 0;
}



/* xor_mode:
 *  Shortcut function for toggling XOR mode on and off.
 */
void xor_mode(int xor)
{
   drawing_mode(xor ? DRAW_MODE_XOR : DRAW_MODE_SOLID, NULL, 0, 0);
}



/* solid_mode:
 *  Shortcut function for selecting solid drawing mode.
 */
void solid_mode()
{
   drawing_mode(DRAW_MODE_SOLID, NULL, 0, 0);
}



/* clear:
 *  Clears the bitmap to color 0.
 */
void clear(BITMAP *bitmap)
{
   clear_to_color(bitmap, 0);
}



/* _vga_vsync:
 *  Waits for the start of a vertical retrace.
 */
void _vga_vsync()
{
   _vsync_out();
   _vsync_in();
}



/* set_color:
 *  Sets a single pallete entry.
 */
void set_color(int index, RGB *p)
{
   set_pallete_range(p-index, index, index, FALSE);
}



/* set_pallete:
 *  Sets the entire color pallete.
 */
void set_pallete(PALLETE p)
{
   set_pallete_range(p, 0, PAL_SIZE-1, TRUE);
}



/* set_pallete_range:
 *  Sets a part of the color pallete.
 */
void set_pallete_range(PALLETE p, int from, int to, int vsync)
{
   int c;

   check_virginity();

   if (gfx_driver)
      gfx_driver->set_pallete(p, from, to, vsync);
   else
      _vga_set_pallete_range(p, from, to, vsync);

   for (c=from; c<=to; c++)
      _current_pallete[c] = p[c];
}



/* get_color:
 *  Retrieves a single color from the pallete.
 */
void get_color(int index, RGB *p)
{
   get_pallete_range(p-index, index, index);
}



/* get_pallete:
 *  Retrieves the entire color pallete.
 */
void get_pallete(PALLETE p)
{
   get_pallete_range(p, 0, PAL_SIZE-1);
}



/* get_pallete_range:
 *  Retrieves a part of the color pallete.
 */
void get_pallete_range(PALLETE p, int from, int to)
{
   int c;

   check_virginity();

   for (c=from; c<=to; c++)
      p[c] = _current_pallete[c];
}



/* fade_interpolate: 
 *  Calculates a pallete part way between source and dest, returning it
 *  in output. The pos indicates how far between the two extremes it should
 *  be: 0 = return source, 64 = return dest, 32 = return exactly half way.
 *  Only affects colors between from and to (inclusive).
 */
void fade_interpolate(PALLETE source, PALLETE dest, PALLETE output, int pos, int from, int to)
{
   int c;

   for (c=from; c<=to; c++) { 
      output[c].r = ((int)source[c].r * (63-pos) + (int)dest[c].r * pos) / 64;
      output[c].g = ((int)source[c].g * (63-pos) + (int)dest[c].g * pos) / 64;
      output[c].b = ((int)source[c].b * (63-pos) + (int)dest[c].b * pos) / 64;
   }
}



/* fade_from_range:
 *  Fades from source to dest, at the specified speed (1 is the slowest, 64
 *  is instantaneous). Only affects colors between from and to (inclusive,
 *  pass 0 and 255 to fade the entire pallete).
 */
void fade_from_range(PALLETE source, PALLETE dest, int speed, int from, int to)
{
   PALLETE temp;
   int c;

   for (c=0; c<PAL_SIZE; c++)
      temp[c] = source[c];

   for (c=0; c<64; c+=speed) {
      fade_interpolate(source, dest, temp, c, from, to);
      set_pallete(temp);
      set_pallete(temp);
   }

   set_pallete(dest);
}



/* fade_in_range:
 *  Fades from a solid black pallete to p, at the specified speed (1 is
 *  the slowest, 64 is instantaneous). Only affects colors between from and 
 *  to (inclusive, pass 0 and 255 to fade the entire pallete).
 */
void fade_in_range(PALLETE p, int speed, int from, int to)
{
   fade_from_range(black_pallete, p, speed, from, to);
}



/* fade_out_range:
 *  Fades from the current pallete to a solid black pallete, at the 
 *  specified speed (1 is the slowest, 64 is instantaneous). Only affects 
 *  colors between from and to (inclusive, pass 0 and 255 to fade the 
 *  entire pallete).
 */
void fade_out_range(int speed, int from, int to)
{
   PALLETE temp;

   get_pallete(temp);
   fade_from_range(temp, black_pallete, speed, from, to);
}



/* fade_from:
 *  Fades from source to dest, at the specified speed (1 is the slowest, 64
 *  is instantaneous).
 */
void fade_from(PALLETE source, PALLETE dest, int speed)
{
   fade_from_range(source, dest, speed, 0, PAL_SIZE-1);
}



/* fade_in:
 *  Fades from a solid black pallete to p, at the specified speed (1 is
 *  the slowest, 64 is instantaneous).
 */
void fade_in(PALLETE p, int speed)
{
   fade_in_range(p, speed, 0, PAL_SIZE-1);
}



/* fade_out:
 *  Fades from the current pallete to a solid black pallete, at the 
 *  specified speed (1 is the slowest, 64 is instantaneous).
 */
void fade_out(int speed)
{
   fade_out_range(speed, 0, PAL_SIZE-1);
}



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



/* _normal_stretch_blit:
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
void _normal_stretch_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height, int masked)
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



/* _normal_rotate_sprite:
 *  Draws a sprite image onto a bitmap at the specified position, rotating 
 *  it by the specified angle. The angle is a fixed point 16.16 number in 
 *  the same format used by the fixed point trig routines, with 256 equal 
 *  to a full circle, 64 a right angle, etc. This function can draw onto
 *  both linear and mode-X bitmaps.
 */
void _normal_rotate_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, fixed angle)
{
   fixed f1x, f1y, f1xd, f1yd;
   fixed f2x, f2y, f2xd, f2yd;
   fixed f3x, f3y;
   fixed w, h, dist;
   int dx, dy;
   int sx, sy;
   unsigned long addr;
   int plane;
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



typedef struct FLOODED_LINE      /* store segments which have been flooded */
{
   short flags;                  /* status of the segment */
   short lpos, rpos;             /* left and right ends of segment */
   short y;                      /* y coordinate of the segment */
   short next;                   /* linked list if several per line */
} FLOODED_LINE;

static int flood_count;          /* number of flooded segments */

#define FLOOD_IN_USE             1
#define FLOOD_TODO_ABOVE         2
#define FLOOD_TODO_BELOW         4

#define FLOOD_LINE(c)            (((FLOODED_LINE *)_scratch_mem) + c)



/* flooder:
 *  Fills a horizontal line around the specified position, and adds it
 *  to the list of drawn segments. Returns the first x coordinate after 
 *  the part of the line which it has dealt with.
 */
static int flooder(BITMAP *bmp, int x, int y, int src_color, int dest_color)
{
   int c;
   FLOODED_LINE *p;
   int left, right;
   unsigned long addr;

   if (is_linear_bitmap(bmp)) {     /* use direct access for linear bitmaps */
      addr = bmp_read_line(bmp, y);
      _farsetsel(bmp->seg);

      /* check start pixel */
      if (_farnspeekb(addr+x) != src_color)
	 return x+1;

      /* work left from starting point */ 
      for (left=x-1; left>=bmp->cl; left--)
	 if (_farnspeekb(addr+left) != src_color)
	    break;

      /* work right from starting point */ 
      for (right=x+1; right<bmp->cr; right++)
	 if (_farnspeekb(addr+right) != src_color)
	    break;
   }
   else {                           /* have to use getpixel() for mode-X */
      /* check start pixel */
      if (getpixel(bmp, x, y) != src_color)
	 return x+1;

      /* work left from starting point */ 
      for (left=x-1; left>=bmp->cl; left--)
	 if (getpixel(bmp, left, y) != src_color)
	    break;

      /* work right from starting point */ 
      for (right=x+1; right<bmp->cr; right++)
	 if (getpixel(bmp, right, y) != src_color)
	    break;
   } 

   left++;
   right--;

   /* draw the line */
   hline(bmp, left, y, right, dest_color);

   /* store it in the list of flooded segments */
   c = y;
   p = FLOOD_LINE(c);

   if (p->flags) {
      while (p->next) {
	 c = p->next;
	 p = FLOOD_LINE(c);
      }

      p->next = c = flood_count++;
      _grow_scratch_mem(sizeof(FLOODED_LINE) * flood_count);
      p = FLOOD_LINE(c);
   }

   p->flags = FLOOD_IN_USE;
   p->lpos = left;
   p->rpos = right;
   p->y = y;
   p->next = 0;

   if (y > bmp->ct)
      p->flags |= FLOOD_TODO_ABOVE;

   if (y+1 < bmp->cb)
      p->flags |= FLOOD_TODO_BELOW;

   return right+2;
}



/* check_flood_line:
 *  Checks a line segment, using the scratch buffer is to store a list of 
 *  segments which have already been drawn in order to minimise the required 
 *  number of tests.
 */
static int check_flood_line(BITMAP *bmp, int y, int left, int right, int src_color, int dest_color)
{
   int c;
   FLOODED_LINE *p;
   int ret = FALSE;

   while (left <= right) {
      c = y;

      do {
	 p = FLOOD_LINE(c);
	 if ((left >= p->lpos) && (left <= p->rpos)) {
	    left = p->rpos+2;
	    goto no_flood;
	 }

	 c = p->next;

      } while (c);

      left = flooder(bmp, left, y, src_color, dest_color);
      ret = TRUE;

      no_flood:
   }

   return ret;
}



/* floodfill:
 *  Fills an enclosed area (starting at point x, y) with the specified color.
 */
void floodfill(BITMAP *bmp, int x, int y, int color)
{
   int src_color;
   int c, done;
   FLOODED_LINE *p;

   /* make sure we have a valid starting point */ 
   if ((x < bmp->cl) || (x >= bmp->cr) || (y < bmp->ct) || (y >= bmp->cb))
      return;

   /* what color to replace? */
   src_color = getpixel(bmp, x, y);
   if (src_color == color)
      return;

   /* set up the list of flooded segments */
   _grow_scratch_mem(sizeof(FLOODED_LINE) * bmp->cb);
   flood_count = bmp->cb;
   p = _scratch_mem;
   for (c=0; c<flood_count; c++) {
      p[c].flags = 0;
      p[c].lpos = SHRT_MAX;
      p[c].rpos = SHRT_MIN;
      p[c].y = y;
      p[c].next = 0;
   }

   /* start up the flood algorithm */
   flooder(bmp, x, y, src_color, color);

   /* continue as long as there are some segments still to test */
   do {
      done = TRUE;

      /* for each line on the screen */
      for (c=0; c<flood_count; c++) {

	 p = FLOOD_LINE(c);

	 /* check below the segment? */
	 if (p->flags & FLOOD_TODO_BELOW) {
	    p->flags &= ~FLOOD_TODO_BELOW;
	    if (check_flood_line(bmp, p->y+1, p->lpos, p->rpos, src_color, color)) {
	       done = FALSE;
	       p = FLOOD_LINE(c);
	    }
	 }

	 /* check above the segment? */
	 if (p->flags & FLOOD_TODO_ABOVE) {
	    p->flags &= ~FLOOD_TODO_ABOVE;
	    if (check_flood_line(bmp, p->y-1, p->lpos, p->rpos, src_color, color)) {
	       done = FALSE;
	       /* special case shortcut for going backwards */
	       if ((c < bmp->cb) && (c > 0))
		  c -= 2;
	    }
	 }
      }

   } while (!done);
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



/* _normal_rectfill:
 *  Draws a solid filled rectangle, using hline() to do the work.
 */
void _normal_rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
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



/* _normal_line:
 *  Draws a line from x1, y1 to x2, y2, using putpixel() to do the work.
 */
void _normal_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   if (x1 == x2)
      vline(bmp, x1, y1, y2, color);
   else if (y1 == y2)
      hline(bmp, x1, y1, x2, color);
   else
      do_line(bmp, x1, y1, x2, y2, color, bmp->vtable->putpixel);
}



/* triangle:
 *  Draws a filled triangle between the three points. Note: this depends 
 *  on a dodgy assumption about parameter passing conventions. I assume that 
 *  the point coordinates are all on the stack in consecutive locations, so 
 *  I can pass that block of stack as the array for polygon() without 
 *  bothering to copy the data to a temporary location.
 */
void triangle(BITMAP *bmp, int x1, int y1, int x2, int y2, int x3, int y3, int color)
{
   polygon(bmp, 3, &x1, color);
}



typedef struct EDGE        /* an edge for the polygon drawer */
{
   int y;                  /* current (starting at the top) y position */
   int bottom;             /* bottom y position of this edge */
   long x;                 /* fixed point x position */
   long dx;                /* fixed point x gradient */
   long w;                 /* width of line segment */
   struct EDGE *prev;      /* doubly linked list */
   struct EDGE *next;
} EDGE;



#define POLYGON_FIX_SHIFT     20



/* fill_edge_structure:
 *  Polygon helper function: initialises an edge structure.
 */
static void fill_edge_structure(EDGE *edge, int *i1, int *i2)
{
   if (i2[1] < i1[1]) {
      int *t = i1;
      i1 = i2;
      i2 = t;
   }

   edge->y = i1[1];
   edge->bottom = i2[1] - 1;
   edge->dx = ((i2[0] - i1[0]) << POLYGON_FIX_SHIFT) / (i2[1] - i1[1]);
   edge->x = (i1[0] << POLYGON_FIX_SHIFT) + (1<<(POLYGON_FIX_SHIFT-1)) - 1;
   edge->prev = NULL;
   edge->next = NULL;

   if (edge->dx < 0)
      edge->x += MIN(edge->dx+(1<<POLYGON_FIX_SHIFT), 0);

   edge->w = MAX(ABS(edge->dx)-(1<<POLYGON_FIX_SHIFT), 0);
}



/* add_edge:
 *  Adds an edge structure to a linked list, returning the new head pointer.
 */
static EDGE *add_edge(EDGE *list, EDGE *edge, int sort_by_x)
{
   EDGE *pos = list;
   EDGE *prev = NULL;

   if (sort_by_x) {
      while ((pos) && (pos->x+pos->w/2 < edge->x+edge->w/2)) {
	 prev = pos;
	 pos = pos->next;
      }
   }
   else {
      while ((pos) && (pos->y < edge->y)) {
	 prev = pos;
	 pos = pos->next;
      }
   }

   edge->next = pos;
   edge->prev = prev;

   if (pos)
      pos->prev = edge;

   if (prev) {
      prev->next = edge;
      return list;
   }
   else
      return edge;
}



/* remove_edge:
 *  Removes an edge structure from a list, returning the new head pointer.
 */
static EDGE *remove_edge(EDGE *list, EDGE *edge)
{
   if (edge->next) 
      edge->next->prev = edge->prev;

   if (edge->prev) {
      edge->prev->next = edge->next;
      return list;
   }
   else
      return edge->next;
}



/* polygon:
 *  Draws a filled polygon with an arbitrary number of corners. Pass the 
 *  number of vertices, then an array containing a series of x, y points 
 *  (a total of vertices*2 values).
 */
void polygon(BITMAP *bmp, int vertices, int *points, int color)
{
   int c;
   int top = INT_MAX;
   int bottom = INT_MIN;
   int *i1, *i2;
   EDGE *edge, *next_edge;
   EDGE *active_edges = NULL;
   EDGE *inactive_edges = NULL;

   /* allocate some space and fill the edge table */
   _grow_scratch_mem(sizeof(EDGE) * vertices);

   edge = (EDGE *)_scratch_mem;
   i1 = points;
   i2 = points + (vertices-1) * 2;

   for (c=0; c<vertices; c++) {
      if (i1[1] != i2[1]) {
	 fill_edge_structure(edge, i1, i2);

	 if (edge->bottom >= edge->y) {

	    if (edge->y < top)
	       top = edge->y;

	    if (edge->bottom > bottom)
	       bottom = edge->bottom;

	    inactive_edges = add_edge(inactive_edges, edge, FALSE);
	    edge++;
	 }
      }
      i2 = i1;
      i1 += 2;
   }

   /* for each scanline in the polygon... */
   for (c=top; c<=bottom; c++) {

      /* check for newly active edges */
      edge = inactive_edges;
      while ((edge) && (edge->y == c)) {
	 next_edge = edge->next;
	 inactive_edges = remove_edge(inactive_edges, edge);
	 active_edges = add_edge(active_edges, edge, TRUE);
	 edge = next_edge;
      }

      /* draw horizontal line segments */
      edge = active_edges;
      while ((edge) && (edge->next)) {
	 hline(bmp, edge->x>>POLYGON_FIX_SHIFT, c, 
		    (edge->next->x+edge->next->w)>>POLYGON_FIX_SHIFT, color);
	 edge = edge->next->next;
      }

      /* update edges, sorting and removing dead ones */
      edge = active_edges;
      while (edge) {
	 next_edge = edge->next;
	 if (c >= edge->bottom) {
	    active_edges = remove_edge(active_edges, edge);
	 }
	 else {
	    edge->x += edge->dx;
	    while ((edge->prev) && 
		   (edge->x+edge->w/2 < edge->prev->x+edge->prev->w/2)) {
	       if (edge->next)
		  edge->next->prev = edge->prev;
	       edge->prev->next = edge->next;
	       edge->next = edge->prev;
	       edge->prev = edge->prev->prev;
	       edge->next->prev = edge;
	       if (edge->prev)
		  edge->prev->next = edge;
	       else
		  active_edges = edge;
	    }
	 }
	 edge = next_edge;
      }
   }
}



/* do_circle:
 *  Helper function for the circle drawing routines. Calculates the points
 *  in a circle of radius r around point x, y, and calls the specified 
 *  routine for each one. The output proc will be passed first a copy of
 *  the bmp parameter, then the x, y point, then a copy of the d parameter
 *  (so putpixel() can be used as the callback).
 */
void do_circle(BITMAP *bmp, int x, int y, int radius, int d, void (*proc)())
{
   int cx = 0;
   int cy = radius;
   int df = 1 - radius; 
   int d_e = 3;
   int d_se = -2 * radius + 5;

   do {
      proc(bmp, x+cx, y+cy, d); 

      if (cx) 
	 proc(bmp, x-cx, y+cy, d); 

      if (cy) 
	 proc(bmp, x+cx, y-cy, d);

      if ((cx) && (cy)) 
	 proc(bmp, x-cx, y-cy, d); 

      if (cx != cy) {
	 proc(bmp, x+cy, y+cx, d); 

	 if (cx) 
	    proc(bmp, x+cy, y-cx, d);

	 if (cy) 
	    proc(bmp, x-cy, y+cx, d); 

	 if (cx && cy) 
	    proc(bmp, x-cy, y-cx, d); 
      }

      if (df < 0)  {
	 df += d_e;
	 d_e += 2;
	 d_se += 2;
      }
      else { 
	 df += d_se;
	 d_e += 2;
	 d_se += 4;
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
   do_circle(bmp, x, y, radius, color, bmp->vtable->putpixel);
}



/* circlefill:
 *  Draws a filled circle.
 */
void circlefill(BITMAP *bmp, int x, int y, int radius, int color)
{
   int cx = 0;
   int cy = radius;
   int df = 1 - radius; 
   int d_e = 3;
   int d_se = -2 * radius + 5;

   do {
      hline(bmp, x-cy, y-cx, x+cy, color);

      if (cx)
	 hline(bmp, x-cy, y+cx, x+cy, color);

      if (df < 0)  {
	 df += d_e;
	 d_e += 2;
	 d_se += 2;
      }
      else { 
	 if (cx != cy) {
	    hline(bmp, x-cx, y-cy, x+cx, color);

	    if (cy)
	       hline(bmp, x-cx, y+cy, x+cx, color);
	 }

	 df += d_se;
	 d_e += 2;
	 d_se += 4;
	 cy--;
      } 

      cx++; 

   } while (cx <= cy);
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
      bmp->vtable->textout_8x8(bmp, f->dat.dat_8x8, str, x, y, color);
      return;
   }

   fp = f->dat.dat_prop;

   if (color < 0) {
      if (_textmode < 0)
	 putter = bmp->vtable->draw_sprite;  /* masked multicolor output */
      else
	 putter = blit_character;            /* opaque multicolor output */
   }
   else
      putter = bmp->vtable->draw_character;  /* single color output */

   while (*str) {
      c = (int)*str - ' ';
      if ((c < 0) || (c >= FONT_SIZE))
	 c = 0;
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



/* text_height:
 *  Returns the height of a character in the specified font.
 */
int text_height(FONT *f)
{
   return (f->flag_8x8) ? 8 : f->dat.dat_prop->dat[0]->h;
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



/* compile_sprite:
 *  Helper function for making compiled sprites.
 */
static void *compile_sprite(BITMAP *b, int l, int planar)
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
   if (p)
      memcpy(p, _scratch_mem, compiler_pos);

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

   for (plane=0; plane<4; plane++)
      s->draw[plane] = NULL;

   for (plane=0; plane < (planar ? 4 : 1); plane++) {
      s->draw[plane] = compile_sprite(bitmap, plane, planar);

      if (!s->draw[plane]) {
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
	 if (sprite->draw[plane])
	    free(sprite->draw[plane]);

      free(sprite);
   }
}

