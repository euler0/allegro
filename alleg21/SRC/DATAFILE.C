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
 *      Datafile reading routines.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <errno.h>

#include "allegro.h"


/* hack to let the grabber prevent compiled sprites from being compiled */
int _compile_sprites = TRUE;


/* data file ID's from version 1 */
#define V1_DAT_FONT              1
#define V1_DAT_BITMAP_16         2 
#define V1_DAT_BITMAP_256        3
#define V1_DAT_SPRITE_16         4
#define V1_DAT_SPRITE_256        5
#define V1_DAT_PALLETE_16        6
#define V1_DAT_PALLETE_256       7



/* load_st_data:
 *  I'm not using this format any more, but files created with the old
 *  version of Allegro might have some bitmaps stored like this. It is 
 *  the 4bpp planar system used by the Atari ST low resolution mode.
 */
static void load_st_data(unsigned char *pos, long size, PACKFILE *f)
{
   int c;
   unsigned int d1, d2, d3, d4;

   size /= 8;           /* number of 4 word planes to read */

   while (size) {
      d1 = pack_mgetw(f);
      d2 = pack_mgetw(f);
      d3 = pack_mgetw(f);
      d4 = pack_mgetw(f);
      for (c=0; c<16; c++) {
	 *(pos++) = ((d1 & 0x8000) >> 15) + ((d2 & 0x8000) >> 14) +
		    ((d3 & 0x8000) >> 13) + ((d4 & 0x8000) >> 12);
	 d1 <<= 1;
	 d2 <<= 1;
	 d3 <<= 1;
	 d4 <<= 1; 
      }
      size--;
   }
}



/* read_block:
 *  Reads a block of size bytes from a file, allocating memory to store it.
 */
static void *read_block(PACKFILE *f, long size)
{
   void *p;

   p = malloc(size);
   if (!p) {
      errno = ENOMEM;
      return NULL;
   }

   if (pack_fread(p, size, f) != size) {
      free(p);
      return NULL;
   }

   return p;
}



/* read_bitmap:
 *  Reads a bitmap from a file, allocating memory to store it. If old_format
 *  is set, skips over the 16 bit flags field which used to be part of the 
 *  structure.
 */
static BITMAP *read_bitmap(PACKFILE *f, int old_format, int bits)
{
   int w, h;
   BITMAP *b;

   /* sprite flags no longer exist */
   if (old_format)
      pack_mgetw(f);

   w = pack_mgetw(f);
   h = pack_mgetw(f);

   b = create_bitmap(w, h);
   if (!b) {
      errno = ENOMEM;
      return NULL;
   }

   if (bits == 16)
      load_st_data(b->dat, w*h/2, f);
   else
      pack_fread(b->dat, w*h, f);

   return b;
}



/* read_font_8x8:
 *  Reads an 8x8 fixed pitch font from a file.
 */
static FONT *read_font_8x8(PACKFILE *f)
{
   FONT *p;

   p = malloc(sizeof(FONT));
   if (!p) {
      errno = ENOMEM;
      return NULL;
   }

   p->dat.dat_8x8 = read_block(f, sizeof(FONT_8x8));
   if (!p->dat.dat_8x8) {
      free(p);
      return NULL;
   }

   p->flag_8x8 = TRUE;
   return p;
}



/* read_font_prop:
 *  Reads a proportional font from a file.
 */
static FONT *read_font_prop(PACKFILE *f)
{
   FONT *p;
   FONT_PROP *fp;
   int c;

   p = malloc(sizeof(FONT));
   if (!p) {
      errno = ENOMEM;
      return NULL;
   }

   p->flag_8x8 = FALSE;
   p->dat.dat_prop = fp = malloc(sizeof(FONT_PROP));
   if (!p->dat.dat_prop) {
      free(p);
      return NULL;
   }

   for (c=0; c<FONT_SIZE; c++)
      fp->dat[c] = NULL;

   for (c=0; c<FONT_SIZE; c++) {
      fp->dat[c] = read_bitmap(f, FALSE, 256);
      if (!fp->dat[c]) {
	 destroy_font(p);
	 return NULL;
      }
   }

   return p;
}



/* read_pallete:
 *  Reads a pallete from a file.
 */
static RGB *read_pallete(PACKFILE *f, int size)
{
   RGB *p;
   int c, x;

   p = malloc(sizeof(PALLETE));
   if (!p) {
      errno = ENOMEM;
      return NULL;
   }

   for (c=0; c<size; c++) {
      p[c].r = pack_getc(f) >> 2;
      p[c].g = pack_getc(f) >> 2;
      p[c].b = pack_getc(f) >> 2;
   }

   x = 0;
   while (c < PAL_SIZE) {
      p[c] = p[x];
      c++;
      x++;
      if (x >= size)
	 x = 0;
   }

   return p;
}



/* read_sample:
 *  Reads a sample from a file.
 */
static SAMPLE *read_sample(PACKFILE *f)
{
   SAMPLE *s;

   s = malloc(sizeof(SAMPLE));
   if (!s) {
      errno = ENOMEM;
      return NULL;
   }

   s->bits = pack_mgetw(f);
   s->freq = pack_mgetw(f);
   s->len = pack_mgetl(f);

   s->data = read_block(f, s->len);
   if (!s->data) {
      free(s);
      return NULL;
   }

   lock_sample(s);
   return s;
}



/* read_midi:
 *  Reads MIDI data from a datafile (this is not the same thing as the 
 *  standard midi file format).
 */
static MIDI *read_midi(PACKFILE *f)
{
   MIDI *m;
   int c;

   m = malloc(sizeof(MIDI));
   if (!m) {
      errno = ENOMEM;
      return NULL;
   }

   for (c=0; c<MIDI_TRACKS; c++) {
      m->track[c].len = 0;
      m->track[c].data = NULL;
   }

   m->divisions = pack_mgetw(f);

   for (c=0; c<MIDI_TRACKS; c++) {
      m->track[c].len = pack_mgetl(f);
      if (m->track[c].len > 0) {
	 m->track[c].data = read_block(f, m->track[c].len);
	 if (!m->track[c].data) {
	    destroy_midi(m);
	    return NULL;
	 }
      }
   }

   lock_midi(m);
   return m;
}



/* read_rle_sprite:
 *  Reads an RLE compressed sprite from a file, allocating memory for it. 
 */
static RLE_SPRITE *read_rle_sprite(PACKFILE *f)
{
   int w, h;
   int size;
   RLE_SPRITE *s;

   w = pack_mgetw(f);
   h = pack_mgetw(f);
   size = pack_mgetl(f);

   s = malloc(sizeof(RLE_SPRITE) + size);
   if (!s) {
      errno = ENOMEM;
      return NULL;
   }

   s->w = w;
   s->h = h;
   s->size = size;

   if (pack_fread(s->dat, size, f) != size) {
      free(s);
      return NULL;
   }

   return s;
}



/* read_compiled_sprite:
 *  Reads a compiled sprite from a file, allocating memory for it.
 */
static COMPILED_SPRITE *read_compiled_sprite(PACKFILE *f, int planar)
{
   BITMAP *b;
   COMPILED_SPRITE *s;

   b = read_bitmap(f, FALSE, 256);
   if (!b)
      return NULL;

   if (!_compile_sprites)
      return (COMPILED_SPRITE *)b;

   s = get_compiled_sprite(b, planar);
   if (!s)
      errno = ENOMEM;

   destroy_bitmap(b);

   return s;
}



/* load_datafile:
 *  Loads an entire data file into memory, and returns a pointer to it. 
 *  On error, sets errno and returns NULL.
 */
DATAFILE *load_datafile(char *filename)
{
   PACKFILE *f;
   DATAFILE *dat;
   int size;
   int type;
   int c;

   f = pack_fopen(filename, F_READ_PACKED);
   if (!f)
      return NULL;

   if (pack_mgetl(f) != DAT_MAGIC) {
      pack_fclose(f);
      errno = EDOM;
      return NULL;
   }

   size = pack_mgetw(f);
   if (errno) {
      pack_fclose(f);
      return NULL;
   }

   dat = malloc(sizeof(DATAFILE)*(size+1));
   if (!dat) {
      pack_fclose(f);
      errno = ENOMEM;
      return NULL;
   }

   for (c=0; c<=size; c++) {
      dat[c].type = DAT_END;
      dat[c].size = 0;
      dat[c].dat = NULL;
   }

   for (c=0; c<size; c++) {

      type = pack_mgetw(f);

      switch (type) {

	 case DAT_DATA:
	    dat[c].type = DAT_DATA;
	    dat[c].size = pack_mgetl(f);
	    dat[c].dat = read_block(f, dat[c].size);
	    break;

	 case DAT_FONT_8x8: 
	 case V1_DAT_FONT: 
	    dat[c].type = DAT_FONT_8x8;
	    dat[c].dat = read_font_8x8(f);
	    dat[c].size = 0;
	    break;

	 case DAT_FONT_PROP:
	    dat[c].type = DAT_FONT_PROP;
	    dat[c].dat = read_font_prop(f);
	    dat[c].size = 0;
	    break;

	 case DAT_BITMAP:
	 case V1_DAT_BITMAP_256:
	    dat[c].type = DAT_BITMAP;
	    dat[c].dat = read_bitmap(f, FALSE, 256);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_BITMAP_16:
	    dat[c].type = DAT_BITMAP;
	    dat[c].dat = read_bitmap(f, FALSE, 16);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_SPRITE_256:
	    dat[c].type = DAT_BITMAP;
	    dat[c].dat = read_bitmap(f, TRUE, 256);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_SPRITE_16:
	    dat[c].type = DAT_BITMAP;
	    dat[c].dat = read_bitmap(f, TRUE, 16);
	    dat[c].size = 0;
	    break;

	 case DAT_PALLETE:
	 case V1_DAT_PALLETE_256:
	    dat[c].type = DAT_PALLETE;
	    dat[c].dat = read_pallete(f, PAL_SIZE);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_PALLETE_16:
	    dat[c].type = DAT_PALLETE;
	    dat[c].dat = read_pallete(f, 16);
	    dat[c].size = 0;
	    break;

	 case DAT_SAMPLE:
	    dat[c].type = DAT_SAMPLE;
	    dat[c].dat = read_sample(f);
	    dat[c].size = 0;
	    break;

	 case DAT_MIDI:
	    dat[c].type = DAT_MIDI;
	    dat[c].dat = read_midi(f);
	    dat[c].size = 0;
	    break;

	 case DAT_RLE_SPRITE:
	    dat[c].type = DAT_RLE_SPRITE;
	    dat[c].dat = read_rle_sprite(f);
	    dat[c].size = 0;
	    break;

	 case DAT_FLI:
	    dat[c].type = DAT_FLI;
	    dat[c].size = pack_mgetl(f);
	    dat[c].dat = read_block(f, dat[c].size);
	    break;

	 case DAT_C_SPRITE:
	    dat[c].type = DAT_C_SPRITE;
	    dat[c].dat = read_compiled_sprite(f, FALSE);
	    dat[c].size = 0;
	    break;

	 case DAT_XC_SPRITE:
	    dat[c].type = DAT_XC_SPRITE;
	    dat[c].dat = read_compiled_sprite(f, TRUE);
	    dat[c].size = 0;
	    break;
      }

      if (errno) {
	 if (!dat[c].dat)
	    dat[c].type = DAT_END;
	 unload_datafile(dat);
	 pack_fclose(f);
	 return NULL;
      }
   }

   pack_fclose(f);
   return dat;
}



/* unload_datafile:
 *  Frees all the objects in a datafile.
 */
void unload_datafile(DATAFILE *dat)
{
   DATAFILE *p = dat;

   if (!dat)
      return;

   while (p->type != DAT_END) {

      switch (p->type) {

	 case DAT_DATA:
	 case DAT_PALLETE:
	    free(p->dat);
	    break;

	 case DAT_BITMAP:
	    destroy_bitmap(p->dat);
	    break;

	 case DAT_FONT_8x8:
	 case DAT_FONT_PROP:
	    destroy_font(p->dat);
	    break;

	 case DAT_SAMPLE:
	    destroy_sample(p->dat);
	    break;

	 case DAT_MIDI:
	    destroy_midi(p->dat);
	    break;

	 case DAT_RLE_SPRITE:
	    destroy_rle_sprite(p->dat);
	    break;

	 case DAT_FLI:
	    free(p->dat);
	    break;

	 case DAT_C_SPRITE:
	 case DAT_XC_SPRITE:
	    destroy_compiled_sprite(p->dat);
	    break;
      }

      p++;
   }

   free(dat);
}



/* load_pcx:
 *  Loads a 256 color PCX file, returning a bitmap structure and storing
 *  the pallete data in the specified pallete (this should be an array of
 *  at least 256 RGB structures).
 */
BITMAP *load_pcx(char *filename, RGB *pal)
{
   PACKFILE *f;
   BITMAP *b;
   int c;
   int width, height;
   int bytes_per_line;
   int x, y;
   char ch;

   f = pack_fopen(filename, F_READ);
   if (!f)
      return NULL;

   pack_getc(f);                    /* skip manufacturer ID */
   pack_getc(f);                    /* skip version flag */
   pack_getc(f);                    /* skip encoding flag */

   if (pack_getc(f) != 8) {         /* we can only handle 8 bits per pixel */
      pack_fclose(f);
      return NULL;
   }

   width = -(pack_igetw(f));        /* xmin */
   height = -(pack_igetw(f));       /* ymin */
   width += pack_igetw(f) + 1;      /* xmax */
   height += pack_igetw(f) + 1;     /* ymax */

   b = create_bitmap(width, height);
   if (!b) {
      pack_fclose(f);
      return FALSE;
   }

   pack_igetl(f);                   /* skip DPI values */

   for (c=0; c<16; c++) {           /* read the 16 color pallete */
      pal[c].r = pack_getc(f) / 4;
      pal[c].g = pack_getc(f) / 4;
      pal[c].b = pack_getc(f) / 4;
   }

   pack_getc(f);

   if (pack_getc(f) != 1) {         /* must be a 256 color file */
      pack_fclose(f);
      destroy_bitmap(b);
      return NULL;
   }

   bytes_per_line = pack_igetw(f);

   for (c=0; c<60; c++)             /* skip some more junk */
      pack_getc(f);

   for (y=0; y<height; y++) {       /* read RLE encoded PCX data */
      x = 0;

      while (x < bytes_per_line) {
	 ch = pack_getc(f);
	 if ((ch & 0xC0) == 0xC0) { 
	    c = (ch & 0x3F);
	    ch = pack_getc(f);
	 }
	 else
	    c = 1; 

	 while (c--) {
	    b->line[y][x++] = ch;
	 }
      }
   }

   while (!pack_feof(f)) {          /* look for a 256 color pallete */
      if (pack_getc(f)==12) {
	 for (c=0; c<256; c++) {
	    pal[c].r = pack_getc(f) / 4;
	    pal[c].g = pack_getc(f) / 4;
	    pal[c].b = pack_getc(f) / 4;
	 }
	 break;
      }
   }

   pack_fclose(f);

   if (errno) {
      destroy_bitmap(b);
      return FALSE;
   }

   return b;
}



/* save_pcx:
 *  Writes a bitmap into a PCX file, using the specified pallete (this 
 *  should be an array of at least 256 RGB structures).
 */
int save_pcx(char *filename, BITMAP *bmp, RGB *pal)
{
   PACKFILE *f;
   int c;
   int x, y;
   int runcount;
   char runchar;
   char ch;

   f = pack_fopen(filename, F_WRITE);
   if (!f)
      return NULL;

   pack_putc(10, f);                /* manufacturer */
   pack_putc(5, f);                 /* version */
   pack_putc(1, f);                 /* run length encoding  */
   pack_putc(8, f);                 /* 8 bits per pixel */
   pack_iputw(0, f);                /* xmin */
   pack_iputw(0, f);                /* ymin */
   pack_iputw(bmp->w-1, f);         /* xmax */
   pack_iputw(bmp->h-1, f);         /* ymax */
   pack_iputw(320, f);              /* HDpi */
   pack_iputw(200, f);              /* VDpi */
   for (c=0; c<16; c++) {
      pack_putc(pal[c].r*4, f);
      pack_putc(pal[c].g*4, f);
      pack_putc(pal[c].b*4, f);
   }
   pack_putc(0, f);                 /* reserved */
   pack_putc(1, f);                 /* one color plane */
   pack_iputw(bmp->w, f);           /* number of bytes per scanline */
   pack_iputw(1, f);                /* color pallete */
   pack_iputw(bmp->w, f);           /* hscreen size */
   pack_iputw(bmp->h, f);           /* vscreen size */
   for (c=0; c<54; c++)             /* filler */
      pack_putc(0, f);

   for (y=0; y<bmp->h; y++) {       /* for each scanline... */
      runcount = 0;
      runchar = 0;
      for (x=0; x<bmp->w; x++) {    /* for each pixel... */
	 ch = getpixel(bmp, x, y);
	 if (runcount==0) {
	    runcount = 1;
	    runchar = ch;
	 }
	 else {
	    if ((ch != runchar) || (runcount >= 0x3f)) {
	       if ((runcount > 1) || ((runchar & 0xC0) == 0xC0))
		  pack_putc(0xC0 | runcount, f);
	       pack_putc(runchar,f);
	       runcount = 1;
	       runchar = ch;
	    }
	    else
	       runcount++;
	 }
      }
      if ((runcount > 1) || ((runchar & 0xC0) == 0xC0))
	 pack_putc(0xC0 | runcount, f);
      pack_putc(runchar,f);
   }

   pack_putc(12,f);                 /* 256 color pallete flag */
   for (c=0; c<256; c++) {
      pack_putc(pal[c].r*4, f);
      pack_putc(pal[c].g*4, f);
      pack_putc(pal[c].b*4, f);
   }

   pack_fclose(f);
   return errno;
}


