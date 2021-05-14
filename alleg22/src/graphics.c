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
 *      Graphics mode set and bitmap creation routines.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <limits.h>

#ifdef DJGPP
#include <go32.h>
#include <conio.h>
#include <sys/farptr.h>
#endif

#include "allegro.h"
#include "internal.h"


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
 * of the ATI 18800/28800, ATI mach64, Cirrus 64xx and Tseng ET3000 drivers 
 * is disabled, because these have not yet been properly tested/debugged. 
 * Please mail me if you have access to one of these cards and can help 
 * finish the driver!
 */

static GFX_DRIVER_INFO gfx_driver_list[] =
{
   /* driver ID         driver structure     autodetect flag */
   {  GFX_VGA,          &gfx_vga,            TRUE  },
   {  GFX_MODEX,        &gfx_modex,          TRUE  },

   #ifdef DJGPP
      /* djgpp drivers */
      {  GFX_VBEAF,        &gfx_vbeaf,          FALSE  },
      {  GFX_VESA2L,       &gfx_vesa_2l,        TRUE   },
      {  GFX_VESA2B,       &gfx_vesa_2b,        TRUE   },
      {  GFX_XTENDED,      &gfx_xtended,        FALSE  },
      {  GFX_ATI,          &gfx_ati,            FALSE  },
      {  GFX_MACH64,       &gfx_mach64,         FALSE  },
      {  GFX_CIRRUS64,     &gfx_cirrus64,       FALSE  },
      {  GFX_CIRRUS54,     &gfx_cirrus54,       TRUE   },
      {  GFX_PARADISE,     &gfx_paradise,       TRUE   },
      {  GFX_S3,           &gfx_s3,             TRUE   },
      {  GFX_TRIDENT,      &gfx_trident,        TRUE   },
      {  GFX_ET3000,       &gfx_et3000,         FALSE  },
      {  GFX_ET4000,       &gfx_et4000,         TRUE   },
      {  GFX_VIDEO7,       &gfx_video7,         TRUE   },
      {  GFX_VESA1,        &gfx_vesa_1,         TRUE   },

   #else
      /* linux drivers */
      {  GFX_SVGALIB,      &gfx_svgalib,        TRUE   },
   #endif

   {  0,                NULL,                0  }
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
   _linear_draw_trans_sprite,
   _linear_draw_lit_sprite,
   _linear_draw_rle_sprite,
   _linear_draw_trans_rle_sprite,
   _linear_draw_lit_rle_sprite,
   _linear_draw_character,
   _linear_textout_fixed,
   _linear_blit,
   _linear_blit,
   _linear_blit,
   _linear_blit,
   _linear_blit_backward,
   _linear_clear_to_color
};



static int gfx_virgin = TRUE;          /* first time we've been called? */
static int a_rez = 3;                  /* original screen mode */
static int a_lines = -1;               /* original screen height */

int _sub_bitmap_id_count = 1;          /* hash value for sub-bitmaps */

int _modex_split_position = 0;         /* has the mode-X screen been split? */

RGB_MAP *rgb_map = NULL;               /* RGB -> pallete entry conversion */

COLOR_MAP *color_map = NULL;           /* translucency/lighting table */

int _color_depth = 8;

unsigned char _rgb_r_shift_15 = 10;    /* truecolor pixel format */
unsigned char _rgb_g_shift_15 = 5;
unsigned char _rgb_b_shift_15 = 0;
unsigned char _rgb_r_shift_16 = 11;
unsigned char _rgb_g_shift_16 = 5;
unsigned char _rgb_b_shift_16 = 0;
unsigned char _rgb_r_shift_32 = 16;
unsigned char _rgb_g_shift_32 = 8;
unsigned char _rgb_b_shift_32 = 0;



/* lock_bitmap:
 *  Locks all the memory used by a bitmap structure.
 */
void lock_bitmap(BITMAP *bmp)
{
   #ifdef DJGPP
      _go32_dpmi_lock_data(bmp, sizeof(BITMAP) + sizeof(char *) * bmp->h);

      if (bmp->dat)
	 _go32_dpmi_lock_data(bmp->dat, bmp->w * bmp->h);
   #endif
}



/* shutdown_gfx:
 *  Used by allegro_exit() to return the system to text mode.
 */
static void shutdown_gfx()
{
   #ifdef DJGPP
      /* djgpp shutdown */
      __dpmi_regs r;

      r.x.ax = 0x0F00; 
      __dpmi_int(0x10, &r);

      if (((r.x.ax & 0xFF) != a_rez) || (gfx_driver != NULL))
	 set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);

   #else
      /* linux shutdown */

   #endif

   _remove_exit_func(shutdown_gfx);
}



/* _check_gfx_virginity:
 *  Checks if this is the first call to any graphics related function, and
 *  if so does some once-only initialisation.
 */
void _check_gfx_virginity()
{
   extern void blit_end(), _linear_blit_end(), _linear_draw_sprite_end();

   int c;

   if (gfx_virgin) {
      #ifdef DJGPP
	 /* djgpp initialisation */
	 __dpmi_regs r;
	 struct text_info textinfo;

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

	 if (a_rez == 3) {             /* store current screen height */
	    gettextinfo(&textinfo); 
	    a_lines = textinfo.screenheight;
	 }
	 else
	    a_lines = -1;

      #else
	 /* linux initialisation */

      #endif

      _add_exit_func(shutdown_gfx);

      gfx_virgin = FALSE;
   }
}



/* set_color_depth:
 *  Sets the pixel size (in bits) which will be used by subsequent calls to 
 *  set_gfx_mode() and create_bitmap(). Valid depths are 8, 15, 16, and 32.
 */
void set_color_depth(int depth)
{
   if ((depth == 8) || (depth == 15) || (depth == 16) || (depth == 32))
      _color_depth = depth;
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
   int c;
   int retrace_enabled = _timer_use_retrace;

   _check_gfx_virginity();

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
      #ifdef DJGPP
	 /* djgpp set text mode */
	 __dpmi_regs r;

	 r.x.ax = a_rez;
	 __dpmi_int(0x10, &r);

	 if (a_lines > 0)
	    _set_screen_lines(a_lines);

      #else
	 /* linux set text mode */

      #endif

      if (_gfx_bank) {
	 free(_gfx_bank);
	 _gfx_bank = NULL;
      }
      return 0;
   }

   /* restore default truecolor pixel format */
   _rgb_r_shift_15 = 10; 
   _rgb_g_shift_15 = 5;
   _rgb_b_shift_15 = 0;
   _rgb_r_shift_16 = 11;
   _rgb_g_shift_16 = 5;
   _rgb_b_shift_16 = 0;
   _rgb_r_shift_32 = 16;
   _rgb_g_shift_32 = 8;
   _rgb_b_shift_32 = 0;

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
   #ifdef DJGPP
      /* this function is currently only implemented under djgpp */
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

      if ((mode_list[mode].set_command == 0x4F02) && (r.h.ah)) {
	 strcpy(allegro_error, "VBE mode not available");
	 destroy_bitmap(b);
	 return NULL;
      }

      if (set_width)
	 set_width(width);
      else
	 _set_vga_virtual_width(w, width);

      driver->w = b->cr = w;
      driver->h = b->cb = h;

      return b;

   #else
      /* linux version not implemented. It will be required if anyone ever
       * wants to get the low level SVGA hardware drivers running under
       * linux.
       */
      return NULL;
   #endif
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

   driver->vid_phys_base = addr;

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


