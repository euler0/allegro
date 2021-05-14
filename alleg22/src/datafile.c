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
#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef DJGPP
#include <sys/segments.h>
#endif

#include "allegro.h"
#include "internal.h"


/* data file ID's for compatibility with the old datafile format */
#define V1_DAT_MAGIC             0x616C6C2EL

#define V1_DAT_DATA              0
#define V1_DAT_FONT              1
#define V1_DAT_BITMAP_16         2 
#define V1_DAT_BITMAP_256        3
#define V1_DAT_SPRITE_16         4
#define V1_DAT_SPRITE_256        5
#define V1_DAT_PALLETE_16        6
#define V1_DAT_PALLETE_256       7
#define V1_DAT_FONT_8x8          8
#define V1_DAT_FONT_PROP         9
#define V1_DAT_BITMAP            10
#define V1_DAT_PALLETE           11
#define V1_DAT_SAMPLE            12
#define V1_DAT_MIDI              13
#define V1_DAT_RLE_SPRITE        14
#define V1_DAT_FLI               15
#define V1_DAT_C_SPRITE          16
#define V1_DAT_XC_SPRITE         17

#define OLD_FONT_SIZE            95


/* object loading functions */
void *load_data_object(PACKFILE *f, long size);
void *load_file_object(PACKFILE *f, long size);
void *load_font_object(PACKFILE *f, long size);
void *load_sample_object(PACKFILE *f, long size);
void *load_midi_object(PACKFILE *f, long size);
void *load_bitmap_object(PACKFILE *f, long size);
void *load_rle_sprite_object(PACKFILE *f, long size);
void *load_compiled_sprite_object(PACKFILE *f, long size);
void *load_xcompiled_sprite_object(PACKFILE *f, long size);

/* object unloading functions */
void unload_sample(SAMPLE *s);
void unload_midi(MIDI *m);


/* information about a datafile object */
typedef struct DATAFILE_TYPE
{
   int type;
   void *(*load)(PACKFILE *f, long size);
   void (*destroy)();
} DATAFILE_TYPE;


#define MAX_DATAFILE_TYPES    32


/* list of objects, and methods for loading and destroying them */
static DATAFILE_TYPE datafile_type[MAX_DATAFILE_TYPES] =
{
   {  DAT_FILE,         load_file_object,             unload_datafile         },
   {  DAT_FONT,         load_font_object,             destroy_font            },
   {  DAT_SAMPLE,       load_sample_object,           unload_sample           },
   {  DAT_MIDI,         load_midi_object,             unload_midi             },
   {  DAT_BITMAP,       load_bitmap_object,           destroy_bitmap          },
   {  DAT_RLE_SPRITE,   load_rle_sprite_object,       destroy_rle_sprite      },
   {  DAT_C_SPRITE,     load_compiled_sprite_object,  destroy_compiled_sprite },
   {  DAT_XC_SPRITE,    load_xcompiled_sprite_object, destroy_compiled_sprite },
   {  DAT_END, NULL, NULL }, { DAT_END, NULL, NULL }, { DAT_END, NULL, NULL },
   {  DAT_END, NULL, NULL }, { DAT_END, NULL, NULL }, { DAT_END, NULL, NULL },
   {  DAT_END, NULL, NULL }, { DAT_END, NULL, NULL }, { DAT_END, NULL, NULL },
   {  DAT_END, NULL, NULL }, { DAT_END, NULL, NULL }, { DAT_END, NULL, NULL },
   {  DAT_END, NULL, NULL }, { DAT_END, NULL, NULL }, { DAT_END, NULL, NULL },
   {  DAT_END, NULL, NULL }, { DAT_END, NULL, NULL }, { DAT_END, NULL, NULL },
   {  DAT_END, NULL, NULL }, { DAT_END, NULL, NULL }, { DAT_END, NULL, NULL },
   {  DAT_END, NULL, NULL }, { DAT_END, NULL, NULL }, { DAT_END, NULL, NULL }
};


/* hack to let the grabber prevent compiled sprites from being compiled */
int _compile_sprites = TRUE;



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
static void *read_block(PACKFILE *f, int size, int alloc_size)
{
   void *p;

   p = malloc(MAX(size, alloc_size));
   if (!p) {
      errno = ENOMEM;
      return NULL;
   }

   pack_fread(p, size, f);

   if (pack_ferror(f)) {
      free(p);
      return NULL;
   }

   return p;
}



/* read_bitmap:
 *  Reads a bitmap from a file, allocating memory to store it.
 */
static BITMAP *read_bitmap(PACKFILE *f, int bits)
{
   int w, h;
   BITMAP *b;

   w = pack_mgetw(f);
   h = pack_mgetw(f);

   b = create_bitmap(w, h);
   if (!b) {
      errno = ENOMEM;
      return NULL;
   }

   if (bits == 4)
      load_st_data(b->dat, w*h/2, f);
   else
      pack_fread(b->dat, w*h, f);

   return b;
}



/* read_font_fixed:
 *  Reads a fixed pitch font from a file.
 */
static FONT *read_font_fixed(PACKFILE *f, int height, int maxchars)
{
   FONT *p;

   p = malloc(sizeof(FONT));
   if (!p) {
      errno = ENOMEM;
      return NULL;
   }

   if (height == 8) {
      p->dat.dat_8x8 = read_block(f, maxchars*8, sizeof(FONT_8x8));
      if (!p->dat.dat_8x8) {
	 free(p);
	 return NULL;
      }
   }
   else if (height == 16) {
      p->dat.dat_8x16 = read_block(f, maxchars*16, sizeof(FONT_8x16));
      if (!p->dat.dat_8x16) {
	 free(p);
	 return NULL;
      }
   }
   else {
      free(p);
      return NULL;
   }

   p->height = height;
   return p;
}



/* read_font_prop:
 *  Reads a proportional font from a file.
 */
static FONT *read_font_prop(PACKFILE *f, int maxchars)
{
   FONT *p;
   FONT_PROP *fp;
   int c;

   p = malloc(sizeof(FONT));
   if (!p) {
      errno = ENOMEM;
      return NULL;
   }

   p->height = -1;
   p->dat.dat_prop = fp = malloc(sizeof(FONT_PROP));
   if (!p->dat.dat_prop) {
      free(p);
      return NULL;
   }

   for (c=0; c<FONT_SIZE; c++)
      fp->dat[c] = NULL;

   for (c=0; c<maxchars; c++) {
      if (pack_feof(f))
	 break;
      fp->dat[c] = read_bitmap(f, 8);
      if (!fp->dat[c]) {
	 destroy_font(p);
	 return NULL;
      }
   }

   while (c<FONT_SIZE) {
      fp->dat[c] = create_bitmap(8, fp->dat[0]->h);
      clear(fp->dat[c]);
      c++;
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

   s->data = read_block(f, s->len, 0);
   if (!s->data) {
      free(s);
      return NULL;
   }

   _go32_dpmi_lock_data(s, sizeof(SAMPLE));
   _go32_dpmi_lock_data(s->data, s->len*s->bits/8);

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
	 m->track[c].data = read_block(f, m->track[c].len, 0);
	 if (!m->track[c].data) {
	    unload_midi(m);
	    return NULL;
	 }
      }
   }

   _go32_dpmi_lock_data(m, sizeof(MIDI));
   for (c=0; c<MIDI_TRACKS; c++)
      if (m->track[c].data)
	 _go32_dpmi_lock_data(m->track[c].data, m->track[c].len);

   return m;
}



/* read_rle_sprite:
 *  Reads an RLE compressed sprite from a file, allocating memory for it. 
 */
static RLE_SPRITE *read_rle_sprite(PACKFILE *f, int bits)
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
static COMPILED_SPRITE *read_compiled_sprite(PACKFILE *f, int planar, int bits)
{
   BITMAP *b;
   COMPILED_SPRITE *s;

   b = read_bitmap(f, 8);
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



/* read_old_datafile:
 *  Helper for reading old-format datafiles (only needed for backward
 *  compatibility).
 */
static DATAFILE *read_old_datafile(PACKFILE *f)
{
   DATAFILE *dat;
   int size;
   int type;
   int c;

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
      dat[c].dat = NULL;
      dat[c].size = 0;
      dat[c].prop = NULL;
   }

   for (c=0; c<size; c++) {

      type = pack_mgetw(f);

      switch (type) {

	 case V1_DAT_FONT: 
	 case V1_DAT_FONT_8x8: 
	    dat[c].type = DAT_FONT;
	    dat[c].dat = read_font_fixed(f, 8, OLD_FONT_SIZE);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_FONT_PROP:
	    dat[c].type = DAT_FONT;
	    dat[c].dat = read_font_prop(f, OLD_FONT_SIZE);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_BITMAP:
	 case V1_DAT_BITMAP_256:
	    dat[c].type = DAT_BITMAP;
	    dat[c].dat = read_bitmap(f, 8);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_BITMAP_16:
	    dat[c].type = DAT_BITMAP;
	    dat[c].dat = read_bitmap(f, 4);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_SPRITE_256:
	    dat[c].type = DAT_BITMAP;
	    pack_mgetw(f);
	    dat[c].dat = read_bitmap(f, 8);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_SPRITE_16:
	    dat[c].type = DAT_BITMAP;
	    pack_mgetw(f);
	    dat[c].dat = read_bitmap(f, 4);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_PALLETE:
	 case V1_DAT_PALLETE_256:
	    dat[c].type = DAT_PALLETE;
	    dat[c].dat = read_pallete(f, PAL_SIZE);
	    dat[c].size = sizeof(PALLETE);
	    break;

	 case V1_DAT_PALLETE_16:
	    dat[c].type = DAT_PALLETE;
	    dat[c].dat = read_pallete(f, 16);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_SAMPLE:
	    dat[c].type = DAT_SAMPLE;
	    dat[c].dat = read_sample(f);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_MIDI:
	    dat[c].type = DAT_MIDI;
	    dat[c].dat = read_midi(f);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_RLE_SPRITE:
	    dat[c].type = DAT_RLE_SPRITE;
	    dat[c].dat = read_rle_sprite(f, 8);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_FLI:
	    dat[c].type = DAT_FLI;
	    dat[c].size = pack_mgetl(f);
	    dat[c].dat = read_block(f, dat[c].size, 0);
	    break;

	 case V1_DAT_C_SPRITE:
	    dat[c].type = DAT_C_SPRITE;
	    dat[c].dat = read_compiled_sprite(f, FALSE, 8);
	    dat[c].size = 0;
	    break;

	 case V1_DAT_XC_SPRITE:
	    dat[c].type = DAT_XC_SPRITE;
	    dat[c].dat = read_compiled_sprite(f, TRUE, 8);
	    dat[c].size = 0;
	    break;

	 default:
	    dat[c].type = DAT_DATA;
	    dat[c].size = pack_mgetl(f);
	    dat[c].dat = read_block(f, dat[c].size, 0);
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

   return dat;
}



/* load_data_object:
 *  Loads a binary data object from a datafile.
 */
void *load_data_object(PACKFILE *f, long size)
{
   return read_block(f, size, 0);
}



/* load_font_object:
 *  Loads a font object from a datafile.
 */
void *load_font_object(PACKFILE *f, long size)
{
   short height = pack_mgetw(f);

   if (height > 0)
      return read_font_fixed(f, height, FONT_SIZE);
   else
      return read_font_prop(f, FONT_SIZE);
}



/* load_sample_object:
 *  Loads a sample object from a datafile.
 */
void *load_sample_object(PACKFILE *f, long size)
{
   return read_sample(f);
}



/* load_midi_object:
 *  Loads a midifile object from a datafile.
 */
void *load_midi_object(PACKFILE *f, long size)
{
   return read_midi(f);
}



/* load_bitmap_object:
 *  Loads a bitmap object from a datafile.
 */
void *load_bitmap_object(PACKFILE *f, long size)
{
   int bits = pack_mgetw(f);

   return read_bitmap(f, bits);
}



/* load_rle_sprite_object:
 *  Loads an RLE sprite object from a datafile.
 */
void *load_rle_sprite_object(PACKFILE *f, long size)
{
   int bits = pack_mgetw(f);

   return read_rle_sprite(f, bits);
}



/* load_compiled_sprite_object:
 *  Loads a compiled sprite object from a datafile.
 */
void *load_compiled_sprite_object(PACKFILE *f, long size)
{
   int bits = pack_mgetw(f);

   return read_compiled_sprite(f, FALSE, bits);
}



/* load_xcompiled_sprite_object:
 *  Loads a mode-X compiled object from a datafile.
 */
void *load_xcompiled_sprite_object(PACKFILE *f, long size)
{
   int bits = pack_mgetw(f);

   return read_compiled_sprite(f, TRUE, bits);
}



/* unload_sample: 
 *  Destroys a sample object.
 */
void unload_sample(SAMPLE *s)
{
   if (s) {
      if (s->data) {
	 _unlock_dpmi_data(s->data, s->len*s->bits/8);
	 free(s->data);
      }

      _unlock_dpmi_data(s, sizeof(SAMPLE));
      free(s);
   }
}



/* unload_midi: 
 *  Destroys a MIDI object.
 */
void unload_midi(MIDI *m)
{
   int c;

   if (m) {
      for (c=0; c<MIDI_TRACKS; c++) {
	 if (m->track[c].data) {
	    _unlock_dpmi_data(m->track[c].data, m->track[c].len);
	    free(m->track[c].data);
	 }
      }
      _unlock_dpmi_data(m, sizeof(MIDI));
      free(m);
   }
}



/* load_object:
 *  Helper to load an object from a datafile.
 */
static void *load_object(PACKFILE *f, int type, long size)
{
   int i;

   /* look for a load function */
   for (i=0; i<MAX_DATAFILE_TYPES; i++)
      if (datafile_type[i].type == type)
	 return datafile_type[i].load(f, size);

   /* if not found, load binary data */
   return load_data_object(f, size);
}



/* load_file_object:
 *  Loads a datafile object.
 */
void *load_file_object(PACKFILE *f, long size)
{
   #define MAX_PROPERTIES  64

   DATAFILE_PROPERTY prop[MAX_PROPERTIES];
   DATAFILE *dat;
   int prop_count, count, type, c, d;

   count = pack_mgetl(f);

   dat = malloc(sizeof(DATAFILE)*(count+1));
   if (!dat) {
      errno = ENOMEM;
      return NULL;
   }

   for (c=0; c<=count; c++) {
      dat[c].type = DAT_END;
      dat[c].dat = NULL;
      dat[c].size = 0;
      dat[c].prop = NULL;
   }

   for (c=0; c<MAX_PROPERTIES; c++)
      prop[c].dat = NULL;

   c = 0;
   prop_count = 0;

   while (c < count) {
      type = pack_mgetl(f);

      if (type == DAT_PROPERTY) {
	 /* load an object property */
	 type = pack_mgetl(f);
	 d = pack_mgetl(f);
	 if (prop_count < MAX_PROPERTIES) {
	    prop[prop_count].type = type;
	    prop[prop_count].dat = malloc(d+1);
	    pack_fread(prop[prop_count].dat, d, f);
	    prop[prop_count].dat[d] = 0;
	    prop_count++;
	 }
	 else {
	    while (d-- > 0)
	       pack_getc(f);
	 }
      }
      else {
	 /* load actual data */
	 f = pack_fopen_chunk(f, FALSE);
	 d = f->todo;

	 dat[c].dat = load_object(f, type, d);

	 if (dat[c].dat) {
	    dat[c].type = type;
	    dat[c].size = d;

	    if (prop_count > 0) {
	       dat[c].prop = malloc(sizeof(DATAFILE_PROPERTY)*(prop_count+1));
	       for (d=0; d<prop_count; d++) {
		  dat[c].prop[d].dat = prop[d].dat;
		  dat[c].prop[d].type = prop[d].type;
		  prop[d].dat = NULL;
	       }
	       dat[c].prop[d].dat = NULL;
	       dat[c].prop[d].type = DAT_END;
	       prop_count = 0;
	    }
	    else
	       dat[c].prop = NULL;
	 }

	 f = pack_fclose_chunk(f);
	 c++;
      }

      if (errno) {
	 unload_datafile(dat);

	 for (c=0; c<MAX_PROPERTIES; c++)
	    if (prop[c].dat)
	       free(prop[c].dat);

	 return NULL;
      }
   }

   for (c=0; c<MAX_PROPERTIES; c++)
      if (prop[c].dat)
	 free(prop[c].dat);

   return dat;
}



/* load_datafile:
 *  Loads an entire data file into memory, and returns a pointer to it. 
 *  On error, sets errno and returns NULL.
 */
DATAFILE *load_datafile(char *filename)
{
   PACKFILE *f;
   DATAFILE *dat;
   int type;

   f = pack_fopen(filename, F_READ_PACKED);
   if (!f)
      return NULL;

   type = pack_mgetl(f);

   if (type == V1_DAT_MAGIC) {
      dat = read_old_datafile(f);
   }
   else {
      if (type == DAT_MAGIC)
	 dat = load_file_object(f, 0);
      else
	 dat = NULL;
   }

   pack_fclose(f);
   return dat; 
}



/* load_datafile_object:
 *  Loads a single object from a datafile.
 */
DATAFILE *load_datafile_object(char *filename, char *objectname)
{
   PACKFILE *f;
   DATAFILE *dat = NULL;
   void *object;
   int type, size;
   int use_next = FALSE;
   char buf[256];

   f = pack_fopen(filename, F_READ_PACKED);
   if (!f)
      return NULL;

   type = pack_mgetl(f);

   if (type != DAT_MAGIC)
      goto getout;

   pack_mgetl(f);

   while (!pack_feof(f)) {
      type = pack_mgetl(f);

      if (type == DAT_PROPERTY) {
	 type = pack_mgetl(f);
	 size = pack_mgetl(f);
	 if (type == DAT_ID('N','A','M','E')) {
	    /* examine name property */
	    pack_fread(buf, size, f);
	    buf[size] = 0;
	    if (stricmp(buf, objectname) == 0)
	       use_next = TRUE;
	 }
	 else {
	    /* skip property */
	    pack_fseek(f, size);
	 }
      }
      else {
	 if (use_next) {
	    /* load actual data */
	    f = pack_fopen_chunk(f, FALSE);
	    size = f->todo;
	    object = load_object(f, type, size);
	    f = pack_fclose_chunk(f);
	    if (object) {
	       dat = malloc(sizeof(DATAFILE));
	       dat->dat = object;
	       dat->type = type;
	       dat->size = size;
	       dat->prop = NULL;
	    }
	    goto getout;
	 }
	 else {
	    /* skip unwanted object */
	    size = pack_mgetl(f);
	    pack_fseek(f, size+4);
	 }
      }
   }

   getout:

   pack_fclose(f);

   return dat; 
}



/* _unload_datafile_object:
 *  Helper to destroy a datafile object.
 */
void _unload_datafile_object(DATAFILE *dat)
{
   int i;

   /* free the property list */
   if (dat->prop) {
      for (i=0; dat->prop[i].type != DAT_END; i++)
	 if (dat->prop[i].dat)
	    free(dat->prop[i].dat);

      free(dat->prop);
   }

   /* look for a destructor function */
   for (i=0; i<MAX_DATAFILE_TYPES; i++) {
      if (datafile_type[i].type == dat->type) {
	 if (dat->dat) {
	    if (datafile_type[i].destroy)
	       datafile_type[i].destroy(dat->dat);
	    else
	       free(dat->dat);
	 }
	 return;
      }
   }

   /* if not found, just free the data */
   if (dat->dat)
      free(dat->dat);
}



/* unload_datafile:
 *  Frees all the objects in a datafile.
 */
void unload_datafile(DATAFILE *dat)
{
   int i;

   if (dat) {
      for (i=0; dat[i].type != DAT_END; i++)
	 _unload_datafile_object(dat+i);

      free(dat);
   }
}



/* unload_datafile_object:
 *  Unloads a single datafile object, returned by load_datafile_object().
 */
void unload_datafile_object(DATAFILE *dat)
{
   if (dat) {
      _unload_datafile_object(dat);
      free(dat);
   }
}



/* get_datafile_property:
 *  Returns the specified property string for the datafile object, or
 *  an empty string if the property does not exist.
 */
char *get_datafile_property(DATAFILE *dat, int type)
{
   DATAFILE_PROPERTY *prop = dat->prop;

   if (prop) {
      while (prop->type != DAT_END) {
	 if (prop->type == type)
	    return prop->dat;

	 prop++;
      }
   }

   return "";
}



/* register_datafile_object: 
 *  Registers a custom datafile object, providing functions for loading
 *  and destroying the objects.
 */
void register_datafile_object(int id, void *(*load)(PACKFILE *f, long size), void (*destroy)(void *data))
{
   int i;

   /* replacing an existing type? */
   for (i=0; i<MAX_DATAFILE_TYPES; i++) {
      if (datafile_type[i].type == id) {
	 if (load)
	    datafile_type[i].load = load;
	 if (destroy)
	    datafile_type[i].destroy = destroy;
	 return;
      }
   }

   /* add a new type */
   for (i=0; i<MAX_DATAFILE_TYPES; i++) {
      if (datafile_type[i].type == DAT_END) {
	 datafile_type[i].type = id;
	 datafile_type[i].load = load;
	 datafile_type[i].destroy = destroy;
	 return;
      }
   }
}



/* _construct_datafile:
 *  Helper used by the output from dat2s.exe, for fixing up parts of
 *  the data that can't be statically initialised, such as locking
 *  samples and MIDI files, and setting the segment selectors in the 
 *  BITMAP structures.
 */
void _construct_datafile(DATAFILE *data)
{
   int c, c2;
   SAMPLE *s;
   MIDI *m;

   for (c=0; data[c].type != DAT_END; c++) {

      switch (data[c].type) {

	 case DAT_FILE:
	    _construct_datafile((DATAFILE *)data[c].dat);
	    break;

	 case DAT_BITMAP:
	    ((BITMAP *)data[c].dat)->seg = _my_ds();
	    break;

	 case DAT_FONT:
	    if (((FONT *)data[c].dat)->height < 0) {
	       for (c2=0; c2<FONT_SIZE; c2++)
		  ((FONT *)data[c].dat)->dat.dat_prop->dat[c2]->seg = _my_ds();
	    }
	    break;

	 case DAT_SAMPLE:
	    s = (SAMPLE *)data[c].dat;
	    _go32_dpmi_lock_data(s, sizeof(SAMPLE));
	    _go32_dpmi_lock_data(s->data, s->len*s->bits/8);
	    break;

	 case DAT_MIDI:
	    m = (MIDI *)data[c].dat;
	    _go32_dpmi_lock_data(m, sizeof(MIDI));
	    for (c2=0; c2<MIDI_TRACKS; c2++)
	       if (m->track[c2].data)
		  _go32_dpmi_lock_data(m->track[c2].data, m->track[c2].len);
	    break;
      }
   }
}

