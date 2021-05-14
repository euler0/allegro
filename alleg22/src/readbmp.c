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
 *      Bitmap reading routines.
 *
 *      BMP reader by Seymour Shlien.
 *      LBM reader by Adrian Oboroc.
 *      TGA reader by Tim Gunn.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "allegro.h"



/* load_bitmap:
 *  Loads a bitmap from disk.
 */
BITMAP *load_bitmap(char *filename, RGB *pal)
{
   if (stricmp(get_extension(filename), "bmp") == 0)
      return load_bmp(filename, pal);
   if (stricmp(get_extension(filename), "lbm") == 0)
      return load_lbm(filename, pal);
   else if (stricmp(get_extension(filename), "pcx") == 0)
      return load_pcx(filename, pal);
   else if (stricmp(get_extension(filename), "tga") == 0)
      return load_tga(filename, pal);
   else
      return NULL;
}



/* save_bitmap:
 *  Writes a bitmap to disk.
 */
int save_bitmap(char *filename, BITMAP *bmp, RGB *pal)
{
   if (stricmp(get_extension(filename), "tga") == 0)
      return save_tga(filename, bmp, pal);
   else
      return save_pcx(filename, bmp, pal);
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



/* load_tga:
 *  Loads a 256 color or 24 bit uncompressed TGA file, returning a bitmap 
 *  structure and storing the pallete data in the specified pallete (this 
 *  should be an array of at least 256 RGB structures).
 */
BITMAP *load_tga(char *filename, RGB *pal)
{
   unsigned char image_id[256], image_palette[256][3], rgb[3];
   unsigned char id_length, palette_type, image_type, palette_entry_size;
   unsigned char pixel_bits, descriptor_bits;
   short unsigned int first_color, palette_colors;
   short unsigned int left, top, image_width, image_height;
   unsigned int i, x, y, yc;
   PACKFILE *f;
   BITMAP *bmp;

   f = pack_fopen(filename, F_READ);
   if (!f)
      return NULL;

   id_length = pack_getc(f);
   palette_type = pack_getc(f);
   image_type = pack_getc(f);
   first_color = pack_igetw(f);
   palette_colors  = pack_igetw(f);
   palette_entry_size = pack_getc(f);
   left = pack_igetw(f);
   top = pack_igetw(f);
   image_width = pack_igetw(f);
   image_height = pack_igetw(f);
   pixel_bits = pack_getc(f);
   descriptor_bits = pack_getc(f);

   pack_fread(image_id, id_length, f);
   pack_fread(image_palette,palette_colors*3 , f);

   /* Image type:
    *    0 = no image data
    *    1 = uncompressed color mapped
    *    2 = uncompressed true color
    *    3 = grayscale
    */
   if ((image_type < 1) || (image_type > 3))
      return NULL;

   switch (image_type) {

      case 1:
	 if ((palette_type != 1) || (pixel_bits != 8))
	    return NULL;

	 for(i=0; i<palette_colors; i++) {
	     pal[i].r = image_palette[i][2] >> 2;
	     pal[i].g = image_palette[i][1] >> 2;
	     pal[i].b = image_palette[i][0] >> 2;
	 }
	 break;

      case 2:
	 if ((palette_type != 0) || (pixel_bits != 24))
	    return NULL;

	 get_pallete(pal);
	 break;

      case 3:
	 if ((palette_type != 0) || (pixel_bits != 8))
	    return NULL;

	 for (i=0; i<256; i++) {
	     pal[i].r = i>>2;
	     pal[i].g = i>>2;
	     pal[i].b = i>>2;
	 }
	 break;
   }

   bmp = create_bitmap(image_width, image_height);
   if (!bmp)
      return NULL;

   for (y=image_height; y; y--) {
      yc = (descriptor_bits & 0x20) ? image_height-y : y-1;

      switch (image_type) {

	 case 1:
	 case 3:
	    pack_fread(bmp->line[yc], image_width, f);
	    break;

	 case 2:
	    for(x=0; x<image_width; x++) {
	       pack_fread(rgb, 3, f);
	       _putpixel(bmp, x, yc, makecol8(rgb[2], rgb[1], rgb[0]));
	    }
	    break;
      }
   }

   pack_fclose(f);

   if (errno) {
      destroy_bitmap(bmp);
      return NULL;
   }

   return bmp;
}



/* load_lbm:
 *  Loads IFF ILBM/PBM files with up to 8 bits per pixel, returning
 *  a bitmap structure and storing the palette data in the specified
 *  palette (this should be an array of at least 256 RGB structures).
 */
BITMAP *load_lbm(char *filename, RGB *pal)
{
   #define IFF_FORM     0x4D524F46     /* 'FORM' - IFF FORM structure  */
   #define IFF_ILBM     0x4D424C49     /* 'ILBM' - interleaved bitmap  */
   #define IFF_PBM      0x204D4250     /* 'PBM ' - new DP2e format     */
   #define IFF_BMHD     0x44484D42     /* 'BMHD' - bitmap header       */
   #define IFF_CMAP     0x50414D43     /* 'CMAP' - color map (palette) */
   #define IFF_BODY     0x59444F42     /* 'BODY' - bitmap data         */

   /* BSWAPL, BSWAPW
    *  Byte swapping macros for convertion between
    *  Intel and Motorolla byte ordering.
    */

   #define BSWAPL(x)   ((((x) & 0x000000ff) << 24) + \
			(((x) & 0x0000ff00) <<  8) + \
			(((x) & 0x00ff0000) >>  8) + \
			(((x) & 0xff000000) >> 24))

   #define BSWAPW(x)    (((x) & 0x00ff) << 8) + (((x) & 0xff00) >> 8)

   PACKFILE *f;
   BITMAP *b = NULL;
   int w, h, i, x, y, bpl, ppl, pbm_mode;
   char ch, cmp_type, bit_plane, color_depth;
   unsigned char uc, check_flags, bit_mask, *line_buf;
   long id, len, l;

   f = pack_fopen(filename, F_READ);
   if (!f)
      return NULL;

   id = pack_igetl(f);              /* read file header    */
   if (id != IFF_FORM) {            /* check for 'FORM' id */
      pack_fclose(f);
      return NULL;
   }

   pack_igetl(f);                   /* skip FORM length    */

   id = pack_igetl(f);              /* read id             */

   /* check image type ('ILBM' or 'PBM ') */
   if ((id != IFF_ILBM) && (id != IFF_PBM)) {
      pack_fclose(f);
      return NULL;
   }

   pbm_mode = id == IFF_PBM;

   id = pack_igetl(f);              /* read id               */
   if (id != IFF_BMHD) {            /* check for header      */
      pack_fclose(f);
      return NULL;
   }

   len = pack_igetl(f);             /* read header length    */
   if (len != BSWAPL(20)) {         /* check, if it is right */
      pack_fclose(f);
      return NULL;
   }

   i = pack_igetw(f);               /* read screen width  */
   w = BSWAPW(i);

   i = pack_igetw(f);               /* read screen height */
   h = BSWAPW(i);

   pack_igetw(f);                   /* skip initial x position  */
   pack_igetw(f);                   /* skip initial y position  */

   color_depth = pack_getc(f);      /* get image depth   */
   if (color_depth > 8) {
      pack_fclose(f);
      return NULL;
   }

   pack_getc(f);                    /* skip masking type */

   cmp_type = pack_getc(f);         /* get compression type */
   if ((cmp_type != 0) && (cmp_type != 1)) {
      pack_fclose(f);
      return NULL;
   }

   pack_getc(f);                    /* skip unused field        */
   pack_igetw(f);                   /* skip transparent color   */
   pack_getc(f);                    /* skip x aspect ratio      */
   pack_getc(f);                    /* skip y aspect ratio      */
   pack_igetw(f);                   /* skip default page width  */
   pack_igetw(f);                   /* skip default page height */

   check_flags = 0;

   do {  /* We'll use cycle to skip possible junk      */
	 /*  chunks: ANNO, CAMG, GRAB, DEST, TEXT etc. */
      id = pack_igetl(f);

      switch(id) {

	 case IFF_CMAP:
	    memset(pal, 0, 256 * 3);
	    l = pack_igetl(f);
	    len = BSWAPL(l) / 3;
	    for (i=0; i<len; i++) {
	       pal[i].r = pack_getc(f) >> 2;
	       pal[i].g = pack_getc(f) >> 2;
	       pal[i].b = pack_getc(f) >> 2;
	    }
	    check_flags |= 1;       /* flag "palette read" */
	    break;

	 case IFF_BODY:
	    pack_igetl(f);          /* skip BODY size */
	    b = create_bitmap(w, h);
	    if (!b) {
	       pack_fclose(f);
	       return NULL;
	    }

	    memset(b->dat, 0, w * h);

	    if (pbm_mode)
	       bpl = w;
	    else {
	       bpl = w >> 3;        /* calc bytes per line  */
	       if (w & 7)           /* for finish bits      */
		  bpl++;
	    }
	    if (bpl & 1)            /* alignment            */
	       bpl++;
	    line_buf = malloc(bpl);
	    if (!line_buf) {
	       pack_fclose(f);
	       return NULL;
	    }

	    if (pbm_mode) {
	       for (y = 0; y < h; y++) {
		  if (cmp_type) {
		     i = 0;
		     while (i < bpl) {
			uc = pack_getc(f);
			if (uc < 128) {
			   uc++;
			   pack_fread(&line_buf[i], uc, f);
			   i += uc;
			}
			else if (uc > 128) {
			   uc = 257 - uc;
			   ch = pack_getc(f);
			   memset(&line_buf[i], ch, uc);
			   i += uc;
			}
			/* 128 (0x80) means NOP - no operation  */
		     }
		  } 
		  else  /* pure theoretical situation */
		     pack_fread(line_buf, bpl, f);

		  memcpy(b->line[y], line_buf, bpl);
	       }
	    } 
	    else {
	       for (y = 0; y < h; y++) {
		  for (bit_plane = 0; bit_plane < color_depth; bit_plane++) {
		     if (cmp_type) {
			i = 0;
			while (i < bpl) {
			   uc = pack_getc(f);
			   if (uc < 128) {
			      uc++;
			      pack_fread(&line_buf[i], uc, f);
			      i += uc;
			   } 
			   else if (uc > 128) {
			      uc = 257 - uc;
			      ch = pack_getc(f);
			      memset(&line_buf[i], ch, uc);
			      i += uc;
			   }
			   /* 128 (0x80) means NOP - no operation  */
			}
		     }
		     else
			pack_fread(line_buf, bpl, f);

		     bit_mask = 1 << bit_plane;
		     ppl = bpl;     /* for all pixel blocks */
		     if (w & 7)     /*  may be, except the  */
			ppl--;      /*  the last            */

		     for (x = 0; x < ppl; x++) {
			if (line_buf[x] & 128)
			   b->line[y][x * 8]     |= bit_mask;
			if (line_buf[x] & 64)
			   b->line[y][x * 8 + 1] |= bit_mask;
			if (line_buf[x] & 32)
			   b->line[y][x * 8 + 2] |= bit_mask;
			if (line_buf[x] & 16)
			   b->line[y][x * 8 + 3] |= bit_mask;
			if (line_buf[x] & 8)
			   b->line[y][x * 8 + 4] |= bit_mask;
			if (line_buf[x] & 4)
			   b->line[y][x * 8 + 5] |= bit_mask;
			if (line_buf[x] & 2)
			   b->line[y][x * 8 + 6] |= bit_mask;
			if (line_buf[x] & 1)
			   b->line[y][x * 8 + 7] |= bit_mask;
		     }

		     /* last pixel block */
		     if (w & 7) {
			x = bpl - 1;

			/* no necessary to check if (w & 7) > 0 in */
			/* first condition, because (w & 7) != 0   */
			if (line_buf[x] & 128)
			   b->line[y][x * 8]     |= bit_mask;
			if ((line_buf[x] & 64) && ((w & 7) > 1))
			   b->line[y][x * 8 + 1] |= bit_mask;
			if ((line_buf[x] & 32) && ((w & 7) > 2))
			   b->line[y][x * 8 + 2] |= bit_mask;
			if ((line_buf[x] & 16) && ((w & 7) > 3))
			   b->line[y][x * 8 + 3] |= bit_mask;
			if ((line_buf[x] & 8)  && ((w & 7) > 4))
			   b->line[y][x * 8 + 4] |= bit_mask;
			if ((line_buf[x] & 4)  && ((w & 7) > 5))
			   b->line[y][x * 8 + 5] |= bit_mask;
			if ((line_buf[x] & 2)  && ((w & 7) > 6))
			   b->line[y][x * 8 + 6] |= bit_mask;
			if ((line_buf[x] & 1)  && ((w & 7) > 7))
			   b->line[y][x * 8 + 7] |= bit_mask;
		     }
		  }
	       }
	    }
	    free(line_buf);
	    check_flags |= 2;       /* flag "bitmap read" */
	    break;

	 default:                   /* skip useless chunks  */
	    l = pack_igetl(f);
	    len = BSWAPL(l);
	    if (len & 1)
	       len++;
	    for (l=0; l < (len >> 1); l++)
	       pack_igetw(f);
      }

      /* Exit from loop if we are at the end of file, */
      /* or if we loaded both bitmap and palette      */

   } while ((check_flags != 3) && (!pack_feof(f)));

   pack_fclose(f);

   if (check_flags != 3) {
      if (check_flags & 2)
	 destroy_bitmap(b);
      return FALSE;
   }

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
      return errno;

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



/* save_tga:
 *  Writes a bitmap into a TGA file, using the specified pallete (this 
 *  should be an array of at least 256 RGB structures).
 */
int save_tga(char *filename, BITMAP *bmp, RGB *pal)
{
   unsigned char image_palette[256][3];
   unsigned int y;
   PACKFILE *f;

   f = pack_fopen(filename, F_WRITE);
   if (!f)
      return errno;

   pack_putc(0, f);           /* id length (no id saved) */
   pack_putc(1, f);           /* palette type (1 = colour map) */
   pack_putc(1, f);           /* image Type (1 color mapped image) */
   pack_iputw(0, f);          /* first colours */
   pack_iputw(256, f);        /* number of colours */
   pack_putc(24, f);          /* 24 bit palette entries */
   pack_iputw(0, f);          /* left */
   pack_iputw(0, f);          /* top */
   pack_iputw(bmp->w, f);     /* width */
   pack_iputw(bmp->h, f);     /* height */
   pack_putc(8, f);           /* bits per pixel */
   pack_putc(0, f);           /* descriptor (bottom to top) */

   for(y=0; y<256; y++) {
      image_palette[y][2] = pal[y].r<<2;
      image_palette[y][1] = pal[y].g<<2;
      image_palette[y][0] = pal[y].b<<2;
   }

   pack_fwrite(image_palette, 768, f);

   for(y=bmp->h; y; y--)
      pack_fwrite(bmp->line[y-1], bmp->w, f);

   pack_fclose(f);
   return errno;
}



/* Windows BMP file reader, by  Seymour Shlien.
 */

#define BI_RGB          0
#define BI_RLE8         1
#define BI_RLE4         2
#define BI_BITFIELDS    3


typedef struct BITMAPFILEHEADER 
{
   unsigned long  bfType;
   unsigned long  bfSize;
   unsigned short bfReserved1;
   unsigned short bfReserved2;
   unsigned long  bfOffBits;
} BITMAPFILEHEADER;


typedef struct BITMAPINFOHEADER
{
   unsigned long  biSize;
   unsigned long  biWidth;
   unsigned long  biHeight;
   unsigned short biPlanes;
   unsigned short biBitCount;
   unsigned long  biCompression;
   unsigned long  biSizeImage;
   unsigned long  biXPelsPerMeter;
   unsigned long  biYPelsPerMeter;
   unsigned long  biClrUsed;
   unsigned long  biClrImportant;
} BITMAPINFOHEADER;



/* read_bmfileheader:
 *  Reads a BMP file header and check that it has the BMP magic number.
 */
static int read_bmfileheader(PACKFILE *f, BITMAPFILEHEADER *fileheader)
{
   fileheader->bfType = pack_igetw(f);
   fileheader->bfSize= pack_igetl(f);
   fileheader->bfReserved1= pack_igetw(f);
   fileheader->bfReserved2= pack_igetw(f);
   fileheader->bfOffBits= pack_igetl(f);

   if (fileheader->bfType != 19778) 
      return -1;

   return 0;
}



/* read_bminfoheader:
 *  Reads information from a BMP file header.
 */
static int read_bminfoheader(PACKFILE *f, BITMAPINFOHEADER *infoheader)
{
   infoheader->biSize = pack_igetl(f);
   infoheader->biWidth = pack_igetl(f);
   infoheader->biHeight = pack_igetl(f);
   infoheader->biPlanes = pack_igetw(f);
   infoheader->biBitCount = pack_igetw(f);
   infoheader->biCompression = pack_igetl(f);
   infoheader->biSizeImage = pack_igetl(f);
   infoheader->biXPelsPerMeter = pack_igetl(f);
   infoheader->biYPelsPerMeter = pack_igetl(f);
   infoheader->biClrUsed = pack_igetl(f);
   infoheader->biClrImportant = pack_igetl(f);

   if (infoheader->biSize != 40)
      return -1;

   return 0;
}



/* read_bmicolors:
 *  Loads the color pallete for 1,4,8 bit formats.
 */
static void read_bmicolors(int ncols, RGB *pal, PACKFILE *f)
{
   int i;

   for (i=0; i<ncols; i++) {
      pal[i].b = pack_getc(f) / 4;
      pal[i].g = pack_getc(f) / 4;
      pal[i].r = pack_getc(f) / 4;
      pack_getc(f);
  }
}



/* read_1bit_line:
 *  Support function for reading the 1 bit bitmap file format.
 */
static void read_1bit_line(int length, PACKFILE *f, BITMAP *bmp, int line)
{
   unsigned char b[32];
   unsigned long n;
   int i, j, k;
   int pix;

   for (i=0; i<length; i++) {
      j = i % 32;
      if (j == 0) {
	 n = pack_mgetl(f); 
	 for (k=0; k<32; k++) {
	    b[k] = n & 1;
	    n = n >> 1;
	 } 
      }
      pix = b[j];
      bmp->line[line][i] = pix;
   }
}



/* read_4bit_line:
 *  Support function for reading the 4 bit bitmap file format.
 */
static void read_4bit_line(int length, PACKFILE *f, BITMAP *bmp, int line)
{
   unsigned char b[8];
   unsigned long n;
   int i, j, k;
   int temp;
   int pix;

   for (i=0; i<length; i++) {
      j = i % 8;
      if (j == 0) {
	 n = pack_igetl(f);
	 for (k=0; k<4; k++) {
	    temp = n & 255;
	    b[k*2+1] = temp & 15;
	    temp = temp >> 4;
	    b[k*2] = temp & 15;
	    n = n >> 8;
	 }
      }
      pix = b[j];
      bmp->line[line][i] = pix;
   }
}



/* read_8bit_line:
 *  Support function for reading the 8 bit bitmap file format.
 */
static void read_8bit_line(int length, PACKFILE *f, BITMAP *bmp, int line)
{
   unsigned char b[4];
   unsigned long n;
   int i, j, k;
   int pix;

   for (i=0; i<length; i++) {
      j = i % 4;
      if (j == 0) {
	 n = pack_igetl(f);
	 for (k=0; k<4; k++) {
	    b[k] = n & 255;
	    n = n >> 8;
	 }
      }
      pix = b[j];
      bmp->line[line][i] = pix;
   }
}



/* read_24bit_line:
 *  Support function for reading the 24 bit bitmap file format, doing
 *  our best to convert it down to a 256 color pallete.
 */
static void read_24bit_line(int length, PACKFILE *f, BITMAP *bmp, int line)
{
   int i, pix;
   int nbytes;
   RGB c;

   nbytes=0;

   for (i=0; i<length; i++) {
      c.b = pack_getc(f);
      c.g = pack_getc(f);
      c.r = pack_getc(f);
      nbytes += 3;
      pix = makecol(c.r, c.g, c.b);
      bmp->line[line][i] = pix;
   }

   nbytes = nbytes % 4;
   if (nbytes != 0) 
      for (i=nbytes; i<4; i++) 
	 pack_getc(f);
} 



/* read_image:
 *  For reading the noncompressed BMP image format.
 */ 
static void read_image(PACKFILE *f, BITMAP *bmp, BITMAPINFOHEADER *infoheader)
{
   int i, line;

   for (i=0; i<infoheader->biHeight; i++) {
      line = i;

      switch (infoheader->biBitCount) {

	 case 1:
	    read_1bit_line(infoheader->biWidth, f, bmp, infoheader->biHeight-i-1);
	    break;

	 case 4:
	    read_4bit_line(infoheader->biWidth, f, bmp, infoheader->biHeight-i-1);
	    break;

	 case 8:
	    read_8bit_line(infoheader->biWidth, f, bmp, infoheader->biHeight-i-1);
	    break;

	 case 24:
	    read_24bit_line(infoheader->biWidth, f, bmp, infoheader->biHeight-i-1);
	    break;
      }
   }
}



/* read_RLE8_compressed_image:
 *  For reading the 8 bit RLE compressed BMP image format.
 */ 
static void read_RLE8_compressed_image(PACKFILE *f, BITMAP *bmp, BITMAPINFOHEADER *infoheader)
{
   unsigned char count, val, val0;
   int j, pos, line;
   int eolflag, eopicflag;

   eopicflag = 0;
   line = infoheader->biHeight - 1; 

   while (eopicflag ==0) {
      pos = 0;                               /* x position in bitmap */
      eolflag = 0;                           /* end of line flag */

      while (eolflag == 0) {
	 count = pack_getc(f);
	 val = pack_getc(f);

	 if (count > 0) {                    /* repeat pixel count times */
	    for (j=0;j<count;j++) {
	       bmp->line[line][pos] = val;
	       pos++;
	    }
	 }
	 else {
	    switch (val) {

	       case 0:                       /* end of line flag */
		  eolflag=1;
		  break;

	       case 1:                       /* end of picture flag */
		  eopicflag=1;
		  break;

	       case 2:                       /* displace picture */
		  count = pack_getc(f);
		  val = pack_getc(f);
		  pos += count;
		  line += val;
		  break;

	       default:                      /* read in absolute mode */
		  for (j=0; j<val; j++) {
		     val0 = pack_getc(f);
		     bmp->line[line][pos] = val0;
		     pos++;
		  } 

		  if (j%2 == 1) 
		     val0 = pack_getc(f);    /* align on word boundary */
		  break;

	    } 
	 } 

	 if (pos > infoheader->biWidth) 
	    eolflag=1;
      } 

      line--;
      if (line < 1) 
	 eopicflag = 1;
   } 
}



/* read_RLE4_compressed_image:
 *  For reading the 4 bit RLE compressed BMP image format.
 */ 
static void read_RLE4_compressed_image(PACKFILE *f, BITMAP *bmp, BITMAPINFOHEADER *infoheader)
{
   unsigned char b[8];
   unsigned char count;
   unsigned short val0, val;
   int j, k, pos, line;
   int eolflag, eopicflag;

   eopicflag = 0;                            /* end of picture flag */
   line = infoheader->biHeight - 1; 

   while (eopicflag ==0) {
      pos =0;
      eolflag = 0;                           /* end of line flag */

      while (eolflag == 0) {
	 count = pack_getc(f);
	 val = pack_getc(f);

	 if (count > 0) {                    /* repeat pixels count times */
	    b[1] = val & 15;
	    b[0] = (val >> 4) & 15;
	    for (j=0; j<count; j++) {
	       bmp->line[line][pos] = b[j%2];
	       pos++;
	    }
	 }
	 else {
	    switch (val) {

	       case 0:                       /* end of line */
		  eolflag=1;
		  break;

	       case 1:                       /* end of picture */
		  eopicflag=1;
		  break;

	       case 2:                       /* displace image */
		  count = pack_getc(f);
		  val = pack_getc(f);
		  pos += count;
		  line += val;
		  break;

	       default:                      /* read in absolute mode */
		  for (j=0; j<val; j++) {
		     if ((j%4) == 0) {
			val0 = pack_igetw(f);
			for (k=0; k<2; k++) {
			   b[2*k+1] = val0 & 15;
			   val0 = val0 >> 4;
			   b[2*k] = val0 & 15;
			   val0 = val0 >> 4;
			}
		     }
		     bmp->line[line][pos] = b[j%4];
		     pos++;
		  } 
		  break;
	    } 
	 } 

	 if (pos > infoheader->biWidth) 
	    eolflag=1;
      } 

      line --;
      if (line < 1) 
	 eopicflag = 1;
   } 
}



/* load_bmp:
 *  Loads a Windows BMP file, returning a bitmap structure and storing
 *  the pallete data in the specified pallete (this should be an array of
 *  at least 256 RGB structures).
 *
 *  Thanks to Seymour Shlien for contributing this function.
 */
BITMAP *load_bmp(char *filename, RGB *pal)
{
   BITMAPFILEHEADER fileheader;
   BITMAPINFOHEADER infoheader;
   PACKFILE *f;
   BITMAP *bmp;
   int ncol;

   f = pack_fopen(filename, F_READ);
   if (!f)
      return NULL;

   if (read_bmfileheader(f, &fileheader) != 0) {
      pack_fclose(f);
      return NULL;
   }

   if (read_bminfoheader(f, &infoheader) != 0) {
      pack_fclose(f);
      return NULL;
   }

   /* compute number of colors recorded */
   ncol = (fileheader.bfOffBits - 54) / 4;
   read_bmicolors(ncol, pal, f);

   /* if 24 bit format then we use whatever pallete is currently
    * active and try our best to represent the image with this
    * color pallete. 
    */
   if (infoheader.biBitCount == 24) 
      get_pallete(pal); 

   bmp = create_bitmap(infoheader.biWidth, infoheader.biHeight);
   if (!bmp) {
      pack_fclose(f);
      return NULL;
   }

   clear(bmp);

   switch (infoheader.biCompression) {

      case BI_RGB:
	 read_image(f, bmp, &infoheader);
	 break;

      case BI_RLE8:
	 read_RLE8_compressed_image(f, bmp, &infoheader);
	 break;

      case BI_RLE4:
	 read_RLE4_compressed_image(f, bmp, &infoheader);
	 break;

      default:
	 destroy_bitmap(bmp);
	 bmp = NULL;
   }

   pack_fclose(f);
   return bmp;
}

