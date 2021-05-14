/*
   GRABBER.C:
   The Allegro library grabber utility program.

   By Shawn Hargreaves, 1994.
*/


#include "stdlib.h"
#include "errno.h"
#include "string.h"
#include "bios.h"
#define lmalloc(x)         malloc((int)x)

#include "allegro.h"

#ifdef BORLAND
#include "alloc.h"
short _RTLENTRY _EXPFUNC sprintf(char _FAR *__buffer, const char _FAR *__format, ...);
#else
int sprintf(char *, const char*, ...);
#endif

#define CTRL(c)      (c-'A'+1)

#define CUR_ITEM     main_dlg[15].d1
#define CUR_TOP      main_dlg[15].d2

#define ONAME_SIZE      12

typedef struct DATANAME
{
   char type_str[5];
   char name[ONAME_SIZE+1];
} DATANAME;

short obj_count = 0;

DATAFILE *datafile = NULL;
short datafile_malloced = 0;

DATANAME *dataname = NULL;
short dataname_malloced = 0;

extern DIALOG main_dlg[], font_dlg[], data_dlg[], pallete_dlg[], grab_dlg[],
	      bmp_dlg[];

char ns_mem[] = "Out of memory!";

char dat_file[80] = "";

char imp_file[80] = "";

BITMAP *graphic = NULL;

RGB g_pallete[256];
short g_pallete_size = 0;

RGB selected_pallete[256];

DATAFILE *bmp = NULL;


void do_export(void);
void destroy_olist(void);
void destroy_obj(DATAFILE *o);
void load(void);
void save(short pack_flag);
void edit_object(DATAFILE *o, DATANAME *n, short first);
void edit_font(DATAFILE *o, DATANAME *n);
void edit_bitmap(DATAFILE *o, DATANAME *n);
void edit_data(DATAFILE *o, DATANAME *n);
void edit_pallete(DATAFILE *o, DATANAME *n);
void draw_wait(void);
void alph_char(char c, DIALOG *d);
short load_degas(void);
short load_degas_compressed(void);
short load_neo(void);
short load_pcx(void);
void save_degas(void);
void save_neo(void);
void save_pcx(void);
void load_st_pallete(FILE *f);
void _load_st_data(char *pos, long size, FILE *f);
void _load_pc_data(char *pos, long size, FILE *f);
void save_st_pallete(FILE *f);
void save_st_data(char *pos, FILE *f, long size);
void save_pc_data(char *pos, FILE *f, long size);
void greyscale(BITMAP *bmp);
short my_edit_proc(short msg, DIALOG *d, unsigned long ch);
short clear_proc(short msg, DIALOG *d, unsigned long ch);
short alphabet_proc(short msg, DIALOG *d, unsigned long ch);
short char_edit_proc(short msg, DIALOG *d, unsigned long ch);
short dataview_proc(short msg, DIALOG *d, unsigned long ch);
short dataexport_proc(short msg, DIALOG *d, unsigned long ch);
short counter_proc(short msg, DIALOG *d, unsigned long ch);
short c_counter_proc(short msg, DIALOG *d, unsigned long ch);
short pal_counter_proc(short msg, DIALOG *d, unsigned long ch);
short color_proc(short msg, DIALOG *d, unsigned long ch);
short sel_pal_proc(short msg, DIALOG *d, unsigned long ch);
short pal_colors_proc(short msg, DIALOG *d, unsigned long ch);
short list_proc(short msg, DIALOG *d, unsigned long ch);
short load_proc(short msg, DIALOG *d, unsigned long ch);
short save_proc(short msg, DIALOG *d, unsigned long ch);
short import_proc(short msg, DIALOG *d, unsigned long ch);
short view_proc(short msg, DIALOG *d, unsigned long ch);
short bmp_colors_proc(short msg, DIALOG *d, unsigned long ch);
short bmp_text_proc(short msg, DIALOG *d, unsigned long ch);
short bmp_exit_proc(short msg, DIALOG *d, unsigned long ch);
short bmp_grab_proc(short msg, DIALOG *d, unsigned long ch);
short bmp_view_proc(short msg, DIALOG *d, unsigned long ch);
short bmp_w_counter_proc(short msg, DIALOG *d, unsigned long ch);
short bmp_h_counter_proc(short msg, DIALOG *d, unsigned long ch);
short mask_check_proc(short msg, DIALOG *d, unsigned long ch);
short grab_bitmap_proc(short msg, DIALOG *d, unsigned long ch);
short grab_sprite_proc(short msg, DIALOG *d, unsigned long ch);
short grab_pallete_proc(short msg, DIALOG *d, unsigned long ch);
short grab_proc(short msg, DIALOG *d, unsigned long ch);
short font_proc(short msg, DIALOG *d, unsigned long ch);
short data_proc(short msg, DIALOG *d, unsigned long ch);
short edit_proc(short msg, DIALOG *d, unsigned long ch);
short delete_proc(short msg, DIALOG *d, unsigned long ch);
short quit_proc(short msg, DIALOG *d, unsigned long ch);



void greyscale(bmp)
BITMAP *bmp;
{
   short c;

   for (c=0; c<bmp->w; c+=2) {         /* grayscale top two lines */
      putpixel(bmp, c, 0, 15);
      putpixel(bmp, c+1, 0, 0);
      putpixel(bmp, c+1, 1, 15);
      putpixel(bmp, c, 1, 0);
   }
   for (c=2; c<bmp->h; c*=2)
      blit(bmp, bmp, 0, 0, 0, c, bmp->w, c);
}



void load_st_pallete(f)
FILE *f;
{
   short c;
   
   for (c=0; c<16; c++)
      g_pallete[c] = _atari_paltorgb(getw(f));

   for (c=16; c<256; c++)
      g_pallete[c] = g_pallete[c&0x0f];
   
   g_pallete_size = 16;
}



void save_st_pallete(f)
FILE *f;
{
   short c;
   
   for (c=0; c<16; c++)
      putw(_atari_rgbtopal(selected_pallete[c]), f);
}



void save_st_data(pos, f, size)
char *pos;
FILE *f;
long size;
{
   short c;
   unsigned short d1, d2, d3, d4;
   
   size /= 8;           /* number of 4 word planes to write */
   
   while (size) {
      d1 = d2 = d3 = d4 = 0;
      for (c=0; c<16; c++) {
	 d1 = (d1<<1) | (*pos&1);
	 d2 = (d2<<1) | ((*pos&2)>>1);
	 d3 = (d3<<1) | ((*pos&4)>>2);
	 d4 = (d4<<1) | ((*pos&8)>>3);
	 pos++;
      }
      putw(d1, f);
      putw(d2, f);
      putw(d3, f);
      putw(d4, f);
      size--;
   }
}



void save_pc_data(pos, f, size)
char *pos;
FILE *f;
long size;
{
   _fwrite((unsigned char *)pos, size, f);
}



short load_degas()
{
   FILE *f;

   f = fopen(imp_file, F_READ);
   if (!f) {
      alert("Error opening file", NULL, NULL, "OK", NULL, 13, 0);
      return FALSE;
   }
   
   graphic = create_bitmap(320,200);
   if (!graphic) {
      alert(ns_mem, NULL, NULL, "OK", NULL, 13, 0);
      fclose(f);
      return FALSE;
   }
   
   getw(f);             /* skip header bytes */

   load_st_pallete(f);

   _load_st_data(graphic->line[0], 32000L, f);

   fclose(f);
   if (errno) {
      alert("Error loading file", NULL, NULL, "OK", NULL, 13, 0);
      destroy_bitmap(graphic);
      graphic = NULL;
      return FALSE;
   }
   
   return TRUE;
}



short load_degas_compressed()
{
   FILE *f;
   short y, x;
   short c;
   char ch;
   char buf[160];    /* enough to hold an entire scan line */
   short *s;

   f = fopen(imp_file, F_READ);
   if (!f) {
      alert("Error opening file", NULL, NULL, "OK", NULL, 13, 0);
      return FALSE;
   }
   
   graphic = create_bitmap(320,200);
   if (!graphic) {
      alert(ns_mem, NULL, NULL, "OK", NULL, 13, 0);
      fclose(f);
      return FALSE;
   }
   
   getw(f);                         /* skip header bytes */

   load_st_pallete(f);

   for (y=0; y<200; y++) {          /* for each scanline... */
      x = 0;
      while (x<160) {               /* read and unpack the data */
	 c = getc(f);
	 if ((c >= 0) && (c <= 127)) {    /* c+1 literal bytes */
	    while ((c-- >= 0) && (x<160))
	       buf[x++] = getc(f);
	 }
	 else {                           /* repeated byte */
	    ch = getc(f);
	    while ((c++ <= 256) && (x<160))
	       buf[x++] = ch;
	 } 
      }

      s = (short *)buf;

      for (x=0; x<160; x+=2) {      /* fixup for intel byte ordering... */
	 ch=buf[x];
	 buf[x]=buf[x+1];
	 buf[x+1]=ch;
      }
      for (x=0; x<320; x+=16) {     /* for each 16 pixel block... */
	 for (c=0; c<16; c++) {     /* for each pixel in the block... */
	    graphic->line[y][x+c] =
	       (((s[0] >> (15-c)) & 1) |
		(((s[20] >> (15-c)) & 1) << 1) |
		(((s[40] >> (15-c)) & 1) << 2) |
		(((s[60] >> (15-c)) & 1) << 3));
	 }
	 s++;
      }
   }

   fclose(f);
   if (errno) {
      alert("Error loading file", NULL, NULL, "OK", NULL, 13, 0);
      destroy_bitmap(graphic);
      graphic = NULL;
      return FALSE;
   }
   
   return TRUE;
}



short load_neo()
{
   FILE *f;
   short c;

   f = fopen(imp_file, F_READ);
   if (!f) {
      alert("Error opening file", NULL, NULL, "OK", NULL, 13, 0);
      return FALSE;
   }

   graphic = create_bitmap(320,200);
   if (!graphic) {
      alert(ns_mem, NULL, NULL, "OK", NULL, 13, 0);
      fclose(f);
      return FALSE;
   }
   
   getl(f);                /* skip header bytes */

   load_st_pallete(f);

   for (c=0; c<92; c++)
      getc(f);             /* skip a bunch more data */

   _load_st_data(graphic->line[0], 32000L, f);

   fclose(f);
   if (errno) {
      alert("Error loading file", NULL, NULL, "OK", NULL, 13, 0);
      destroy_bitmap(graphic);
      graphic = NULL;
      return FALSE;
   }
   
   return TRUE;
}



short load_pcx()
{
   FILE *f;
   short c;
   short width, height;
   short bytes_per_line;
   short x, y;
   char ch;

   f = fopen(imp_file, F_READ);
   if (!f) {
      alert("Error opening file", NULL, NULL, "OK", NULL, 13, 0);
      return FALSE;
   }

   getc(f);                   /* skip manufacturer ID */
   getc(f);                   /* skip version flag */
   getc(f);                   /* skip encoding flag */
   
   if (getc(f) != 8) {        /* bits per pixel */
      alert("Not a 256 color PCX file!", NULL, NULL, "Sorry", NULL, 13, 0);
      fclose(f);
      return FALSE;
   }

   width = -(igetw(f));       /* xmin */
   height = -(igetw(f));      /* ymin */
   width += igetw(f) + 1;     /* xmax */
   height += igetw(f) + 1;    /* ymax */

   if (errno) {
      alert("Error loading file", NULL, NULL, "OK", NULL, 13, 0);
      fclose(f);
      return FALSE;
   }
   
   graphic = create_bitmap(width,height);
   if (!graphic) {
      alert(ns_mem, NULL, NULL, "OK", NULL, 13, 0);
      fclose(f);
      return FALSE;
   }
   clear(graphic);

   getl(f);                   /* skip DPI values */

   for (c=0; c<16; c++) {     /* read the 16 color pallete */
      g_pallete[c].r = getc(f);
      g_pallete[c].g = getc(f);
      g_pallete[c].b = getc(f);
   }

   for (c=16; c<256; c++)
      g_pallete[c] = g_pallete[c&0x0f];

   g_pallete_size = 16;

   getc(f);
   
   if (getc(f) != 1) {
      alert("Not a 256 color PCX file!", NULL, NULL, "Sorry", NULL, 13, 0);
      fclose(f);
      destroy_bitmap(graphic);
      graphic = NULL;
      return FALSE;
   }

   bytes_per_line = igetw(f);

   for (c=0; c<60; c++)                /* skip some more junk */
      getc(f);

   for (y=0; y<height; y++) {
      x = 0;
      while (x < bytes_per_line) {
	 ch = getc(f);
	 if ((ch & 0xC0) == 0xC0) {    /* this is an RLE token */
	    c = (ch & 0x3F);
	    ch = getc(f);
	 }
	 else
	    c = 1;                     /* single byte */
	 while (c--) {
	    graphic->line[y][x++] = ch;
	 }
      }
   }

   while (!feof(f)) {                  /* look for a 256 color pallete */
      if (getc(f)==12) {
	 for (c=0; c<256; c++) {
	    g_pallete[c].r = getc(f);
	    g_pallete[c].g = getc(f);
	    g_pallete[c].b = getc(f);
	 }
	 g_pallete_size = 256;
	 break;
      }
   }

   fclose(f);
   if (errno) {
      alert("Error loading file", NULL, NULL, "OK", NULL, 13, 0);
      destroy_bitmap(graphic);
      graphic = NULL;
      return FALSE;
   }

   return TRUE;
}



void save_degas()
{
   FILE *f;
   BITMAP *b;
   short ours = FALSE;

   f = fopen(imp_file, F_WRITE);
   if (!f) {
      alert("Error opening output file", NULL, NULL, "OK", NULL, 13, 0);
      return;
   }

   if ((((BITMAP *)bmp->dat)->w != SCREEN_W) ||
       (((BITMAP *)bmp->dat)->h != SCREEN_H)) {
      b = create_bitmap(SCREEN_W, SCREEN_H);
      if (!b) {
	 fclose(f);
	 alert(ns_mem, NULL, NULL, "OK", NULL, 13, 0);
	 return;
      }
      blit(bmp->dat, b, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
      ours = TRUE;
   }
   else
      b = (BITMAP *)bmp->dat;

   putw(0,f);
   save_st_pallete(f);
   save_st_data(b->line[0], f, 32000L);

   fclose(f);
   if (ours)
      destroy_bitmap(b);

   if (errno)
      alert("Error saving Degas file", NULL, NULL, "OK", NULL, 13, 0);
   else
      if ((bmp->type==DAT_SPRITE_256) || (bmp->type==DAT_BITMAP_256))
	 alert("Warning: 256 color", "image data has been", "converted to 16 colors", "OK", NULL, 13, 0);
}



void save_neo()
{
   FILE *f;
   BITMAP *b;
   short ours = FALSE;
   short c;

   f = fopen(imp_file, F_WRITE);
   if (!f) {
      alert("Error opening output file", NULL, NULL, "OK", NULL, 13, 0);
      return;
   }

   if ((((BITMAP *)bmp->dat)->w != SCREEN_W) ||
       (((BITMAP *)bmp->dat)->h != SCREEN_H)) {
      b = create_bitmap(SCREEN_W, SCREEN_H);
      if (!b) {
	 fclose(f);
	 alert(ns_mem, NULL, NULL, "OK", NULL, 13, 0);
	 return;
      }
      blit(bmp->dat, b, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
      ours = TRUE;
   }
   else
      b = (BITMAP *)bmp->dat;

   putw(0,f);              /* flag */
   putw(0,f);              /* low res */

   save_st_pallete(f);     /* pallete */

   for (c=0; c<8; c++)     /* filename */
      putc(' ', f);
   putc('.', f);
   for (c=0; c<3; c++)
      putc(' ', f);
      
   putl(0,f);              /* color animation */
   putw(0,f);              /* slideshow */
   putw(0,f);              /* x */
   putw(0,f);              /* y */
   putw(320,f);            /* width */
   putw(200,f);            /* height */
   for (c=0; c<33; c++)    /* unused */
      putw(0,f);

   save_st_data(b->line[0], f, 32000L);

   fclose(f);
   if (ours)
      destroy_bitmap(b);

   if (errno)
      alert("Error saving NEO file", NULL, NULL, "OK", NULL, 13, 0);
   else
      if ((bmp->type==DAT_SPRITE_256) || (bmp->type==DAT_BITMAP_256))
	 alert("Warning: 256 color", "image data has been", "converted to 16 colors", "OK", NULL, 13, 0);
}



void save_pcx()
{
   FILE *f;
   BITMAP *b = (BITMAP *)bmp->dat;
   short c;
   short x, y;
   short runcount;
   char runchar;
   char ch;

   f = fopen(imp_file, F_WRITE);
   if (!f) {
      alert("Error opening output file", NULL, NULL, "OK", NULL, 13, 0);
      return;
   }

   putc(10,f);             /* manufacturer */
   putc(5,f);              /* version */
   putc(1,f);              /* run length encoding  */
   putc(8,f);              /* 8 bits per pixel */
   iputw(0,f);             /* xmin */
   iputw(0,f);             /* ymin */
   iputw(b->w-1,f);        /* xmax */
   iputw(b->h-1,f);        /* ymax */
   iputw(320,f);           /* HDpi */
   iputw(200,f);           /* VDpi */
   for (c=0; c<16; c++) {
      putc(selected_pallete[c].r,f);
      putc(selected_pallete[c].g,f);
      putc(selected_pallete[c].b,f);
   }
   putc(0,f);              /* reserved */
   putc(1,f);              /* one color plane */
   iputw(b->w,f);          /* number of bytes per scanline */
   iputw(1,f);             /* color pallete */
   iputw(b->w,f);          /* hscreen size */
   iputw(b->h,f);          /* vscreen size */
   for (c=0; c<54; c++)    /* filler */
      putc(0, f);

   for (y=0; y<b->h; y++) {               /* for each scanline... */
      runcount = 0;
      runchar = 0;
      for (x=0; x<b->w; x++) {            /* for each pixel... */
	 ch = getpixel(b,x,y);
	 if (runcount==0) {
	    runcount = 1;
	    runchar = ch;
	 }
	 else {
	    if ((ch != runchar) || (runcount >= 0x3f)) {
	       if ((runcount > 1) || ((runchar & 0xC0) == 0xC0))
		  putc(0xC0 | runcount, f);
	       putc(runchar,f);
	       runcount = 1;
	       runchar = ch;
	    }
	    else
	       runcount++;
	 }
      }
      if ((runcount > 1) || ((runchar & 0xC0) == 0xC0))
	 putc(0xC0 | runcount, f);
      putc(runchar,f);
   }

   putc(12,f);             /* 256 color pallete flag */
   for (c=0; c<256; c++) {
      putc(selected_pallete[c].r,f);
      putc(selected_pallete[c].g,f);
      putc(selected_pallete[c].b,f);
   }
   
   fclose(f);

   if (errno)
      alert("Error saving PCX file", NULL, NULL, "OK", NULL, 13, 0);
}



short my_edit_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (msg==MSG_CHAR) {
      char c = (char)(ch & 0xff);
      
      if (((c < 'A') || (c > 'Z')) &&
	  ((c < 'a') || (c > 'z')) &&
	  ((c < '0') || (c > '9')) &&
	  (c != '_') && (c != 8))
	 return D_O_K;
   } 
   return d_edit_proc(msg, d, ch);
}



short clear_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (msg==MSG_DRAW)
      clear(screen);
   return D_O_K;
}



void draw_wait()
{
   show_mouse(NULL);
   rectfill(screen, 61, 81, 259, 119, 0);
   rect(screen, 60, 80, 260, 120, 15);
   textmode(0);
   textout(screen, font, "Please wait a moment", 80, 96, 15);
   show_mouse(screen);
}



void alph_char(c, d)
char c;
DIALOG *d;
{
   char s[2];
   textmode(d->bg);
   s[0] = c;
   s[1] = 0;
   c-=32;
   textout(screen, d->dp, s, d->x+(c&0x1f)*10, d->y+(c>>5)*12, d->fg);
}



short alphabet_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   short c;

   if (msg==MSG_DRAW) {
      for (c=32; c<127; c++)
	 alph_char(c, d);
   }
   else {
      if (msg==MSG_CLICK) {
	 do {
	    c = (((mouse_y - d->y) / 12) << 5) + ((mouse_x - d->x) / 10) + 32;
	    if (c < 32)
	       c = 32;
	    else
	       if (c > 126)
		  c = 126;
	    if (c != font_dlg[5].d1) {
	       font_dlg[5].d1 = c;
	       show_mouse(NULL);
	       (*font_dlg[5].proc)(MSG_DRAW,font_dlg+5,ch);
	       show_mouse(screen);
	    }
	 } while (mouse_b);
      }
   }
   return D_O_K;
}



short char_edit_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   char *f = ((FONT *)d->dp)->dat[d->d1-32];
   short c1, c2;
   char buf[40];
   short flag;
   
   if (msg==MSG_DRAW) {
      for (c1=0; c1<8; c1++) {
	 hline(screen, d->x, d->y+c1*16, d->x+128, d->fg);
	 vline(screen, d->x+c1*16, d->y, d->y+128, d->fg);
	 for (c2=0; c2<8; c2++)
	    rectfill(screen, d->x+c1*16+2, d->y+c2*16+2,
		     d->x+c1*16+14, d->y+c2*16+14,
		     (f[c2] & (0x80>>c1)) ? d->fg : d->bg);
      }
      hline(screen, d->x, d->y+128, d->x+128, d->fg);
      vline(screen, d->x+128, d->y, d->y+128, d->fg);
      textmode(d->bg);
      sprintf(buf, "%d (%c)  ", d->d1, d->d1);
      textout(screen, font, buf, d->x+200, d->y+32, d->fg);
      buf[0] = d->d1;
      buf[1] = 0;
      textout(screen, d->dp, buf, d->x+216, d->y+48, d->fg);
   }
   else {
      if (msg==MSG_CLICK) {
	 do {
	    c1 = (mouse_x - d->x) / 16;
	    c2 = (mouse_y - d->y) / 16;
	    if (c1 < 0)
	       c1 = 0;
	    if (c2 < 0)
	       c2 = 0;
	    if (c1 > 7)
	       c1 = 7;
	    if (c2 > 7)
	       c2 = 7;
	    flag = FALSE;
	    if (mouse_b & 1) {
	       if (!(f[c2] & (0x80>>c1))) {
		  flag = TRUE;
		  f[c2] |= (0x80>>c1);
	       }
	    }
	    else {
	       if (mouse_b & 2) {
		  if (f[c2] & (0x80>>c1)) {
		     flag = TRUE;
		     f[c2] &= ~(0x80>>c1);
		  }
	       }
	    }
	    if (flag) {
	       show_mouse(NULL);
	       alph_char(d->d1, font_dlg+4);
	       rectfill(screen, d->x+c1*16+2, d->y+c2*16+2,
			d->x+c1*16+14, d->y+c2*16+14,
			(f[c2] & (0x80>>c1)) ? d->fg : d->bg); 
	       buf[0] = d->d1;
	       buf[1] = 0;
	       textout(screen, d->dp, buf, d->x+216, d->y+48, d->fg);
	       show_mouse(screen);
	    }
	 } while (mouse_b);
      }
   }
   return D_O_K;
}



DIALOG font_dlg[] =
{
   { clear_proc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL },
   { d_text_proc, 0, 0, 1, 1, 15, 0, 0, 0, 0, 0, "Font:" },
   { my_edit_proc, 48, 0, (ONAME_SIZE+1)*8, 8, 15, 0, 0, D_GOTFOCUS, ONAME_SIZE, 0, NULL },
   { d_button_proc, 200, 170, 100, 16, 15, 0, 13, D_EXIT, 0, 0, "OK" },
   { alphabet_proc, 0, 24, 320, 36, 15, 0, 0, 0, 0, 0, NULL },
   { char_edit_proc, 32, 68, 128, 128, 15, 0, 0, 0, 32, 0, NULL },
   { NULL }
};



void edit_font(obj,name)
DATAFILE *obj;
DATANAME *name;
{
   font_dlg[2].dp = name->name;        /* edit proc */
   font_dlg[4].dp = obj->dat;          /* alphabet proc */
   font_dlg[5].dp = obj->dat;          /* char_edit proc */
   font_dlg[5].d1 = 32;
   do_dialog(font_dlg);
}



void edit_bitmap(obj,name)
DATAFILE *obj;
DATANAME *name;
{
   bmp_dlg[2].dp = name->name;
   bmp_dlg[7].d1 = ((BITMAP *)obj->dat)->w;
   bmp_dlg[8].d1 = ((BITMAP *)obj->dat)->h;
   if (obj->size & SPRITE_MASKED)
      bmp_dlg[11].flags = D_SELECTED;
   else
      bmp_dlg[11].flags = 0;
   bmp = obj;
   do_dialog(bmp_dlg);
}



short dataview_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   char buf[80];
   long l;
   short c, c2;

   if (msg==MSG_DRAW) {
      l = ((DATAFILE *)d->dp)->size;
      textmode(d->bg);
      sprintf(buf,"%ld bytes:", l);
      textout(screen, font, buf, d->x, d->y, d->fg);
      for (c=0; c<8; c++) {
	 for (c2=0; c2<32; c2++) {
	    if ((c*32+c2) >= l)
	       buf[c2] = ' ';
	    else
	       buf[c2] = ((char *)((DATAFILE *)d->dp)->dat)[c*32+c2];
	    if ((buf[c2] < 32) || (buf[c2] > 126))
	       buf[c2] = ' ';
	 }
	 buf[32] = 0;
	 textout(screen, font, buf, d->x+24, d->y+32+c*8, d->fg);
      }
      if (l > 32*8)
	 textout(screen, font, "...", d->x+24+32*8, d->y+108, d->fg);
   }
   return D_O_K;
}



short dataexport_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   FILE *f;
   DATAFILE *df = (DATAFILE *)data_dlg[4].dp;

   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      draw_wait();
      if (!imp_file[0])
	 strcpy(imp_file,dat_file);
      strcpy(get_filename(imp_file),data_dlg[2].dp);
      if (file_select("Export data to file", imp_file)) {
	 if (file_exists(imp_file, F_RDONLY | F_HIDDEN, NULL))
	    if (alert("File already exists.", "Overwrite?", NULL, "OK", "Cancel", 13, 27)==2)
	       return D_REDRAW;
	 draw_wait();
	 f = fopen(imp_file, F_WRITE);
	 if (!f) {
	    alert("Error opening output file", NULL, NULL, "OK", NULL, 13, 0);
	    return D_REDRAW;
	 }
	 _fwrite(df->dat, df->size, f);
	 fclose(f);
	 if (errno)
	    alert("Error exporting data", NULL, NULL, "OK", NULL, 13, 0);
      }
      return D_REDRAW;
   }
   return D_O_K;
}



DIALOG data_dlg[] =
{
   { clear_proc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL },
   { d_text_proc, 0, 0, 1, 1, 15, 0, 0, 0, 0, 0, "Data:" },
   { my_edit_proc, 48, 0, (ONAME_SIZE+1)*8, 8, 15, 0, 0, D_GOTFOCUS, ONAME_SIZE, 0, NULL },
   { d_button_proc, 200, 170, 100, 16, 15, 0, 13, D_EXIT, 0, 0, "OK" },
   { dataview_proc, 0, 24, 320, 36, 15, 0, CTRL('V'), 0, 0, 0, NULL },
   { dataexport_proc, 80, 170, 100, 16, 15, 0, CTRL('E'), D_EXIT, 0, 0, "Export" },
   { NULL }
};



void edit_data(obj,name)
DATAFILE *obj;
DATANAME *name;
{
   data_dlg[2].dp = name->name;        /* edit proc */
   data_dlg[4].dp = obj;
   do_dialog(data_dlg);
}



short counter_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   static char number[6];
   short x;
   short first = TRUE;
   short c;

   if (msg==MSG_CLICK) {
      while(mouse_b) {
	 x = d->d1;
	 if (mouse_b==2) {
	    if (key[KEY_LSHIFT] || key[KEY_RSHIFT])
	       d->d1+=10;
	    else
	       d->d1++;
	 }
	 else {
	    if (mouse_b&1) {
	       if (key[KEY_LSHIFT] || key[KEY_RSHIFT])
		  d->d1-=10;
	       else
		  d->d1--;
	    }
	 }
	 if (d->d1 >= d->d2)
	    d->d1 = d->d2;
	 else
	    if (d->d1 < 0)
	       d->d1 = 0;
	 if (x != d->d1) {
	    show_mouse(NULL);
	    (*d->proc)(MSG_DRAW, d, ch);
	    (*d->proc)(MSG_KEY, d, ch);
	    show_mouse(screen);
	    if (first) {
	       for (c=0; c<16; c++) {
		  rest(20);
		  if (!mouse_b)
		     break;
	       }
	       first = FALSE;
	    }
	 }
      }
      return D_O_K;
   }

   if (msg==MSG_DRAW) {
      d->dp = number;
      sprintf(number, "%d", d->d1);
   }
   
   return d_button_proc(msg, d, ch);
}



short c_counter_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (msg==MSG_KEY) {
      ((RGB *)pallete_dlg[3].dp)[pallete_dlg[3].d1].r = pallete_dlg[7].d1;
      ((RGB *)pallete_dlg[3].dp)[pallete_dlg[3].d1].g = pallete_dlg[8].d1;
      ((RGB *)pallete_dlg[3].dp)[pallete_dlg[3].d1].b = pallete_dlg[9].d1;
      counter_proc(MSG_DRAW, d, ch);
      color_proc(MSG_DRAW, pallete_dlg+3, 0);
      return D_O_K;
   }

   return counter_proc(msg, d, ch);
}



short pal_counter_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (msg == MSG_KEY) {
      pallete_dlg[3].d1 = d->d1;
      pallete_dlg[7].d1 = ((RGB *)pallete_dlg[3].dp)[d->d1].r;
      pallete_dlg[8].d1 = ((RGB *)pallete_dlg[3].dp)[d->d1].g;
      pallete_dlg[9].d1 = ((RGB *)pallete_dlg[3].dp)[d->d1].b;
      color_proc(MSG_DRAW, pallete_dlg+3, 0);
      c_counter_proc(MSG_DRAW, pallete_dlg+7, 0);
      c_counter_proc(MSG_DRAW, pallete_dlg+8, 0);
      c_counter_proc(MSG_DRAW, pallete_dlg+9, 0);
      return D_O_K;
   }

   return counter_proc(msg, d, ch);
}



short color_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (msg==MSG_DRAW) {
      rect(screen, d->x, d->y, d->x+d->w, d->y+d->h, d->fg);
      rectfill(screen, d->x+1, d->y+1, d->x+d->w-1, d->y+d->h-1, d->bg);
      desktop_pallete[(int)d->bg] = rgbtopal(((RGB *)d->dp)[d->d1]);
      set_pallete(desktop_pallete);
   }
   return D_O_K;
}



short sel_pal_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (d_button_proc(msg, d, ch)==D_CLOSE) {
      short c;
      for (c=0; c<256; c++)
	 selected_pallete[c] = ((RGB *)pallete_dlg[3].dp)[c];
      alert("This pallete will be used", "when viewing and exporting", "bitmap and sprite data", "OK", NULL, 13, 0);
      return D_REDRAW;
   }
   return D_O_K;
}



short pal_colors_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   short ret;
   short c;
   short nest_flag = FALSE;
   static DATAFILE *obj = NULL;

   if (!obj) {
      obj = (DATAFILE *)d->dp;
      d->dp = (obj->type==DAT_PALLETE_16) ? "16 colors" : "256 colors";
      nest_flag = TRUE;
   }

   ret = d_button_proc(msg, d, ch);

   if (ret ==D_CLOSE) {
      ret = D_REDRAW;
      if (obj->type==DAT_PALLETE_16) {
	 obj->type=DAT_PALLETE_256;
	 pallete_dlg[4].d2 = 255;
	 for (c=16; c<256; c++)
	    ((RGB *)obj->dat)[c] = ((RGB *)obj->dat)[c&0x0f];
      }
      else {
	 if (alert("Really convert 256 color", "pallete to 16 colors?", NULL, "Yes", "Cancel", 13, 27)==1)
	 {
	    obj->type=DAT_PALLETE_16;
	    pallete_dlg[3].d1 = 0;
	    pallete_dlg[4].d1 = 0;
	    pallete_dlg[4].d2 = 15;
	    pallete_dlg[7].d1 = ((RGB *)obj->dat)->r;
	    pallete_dlg[8].d1 = ((RGB *)obj->dat)->g;
	    pallete_dlg[9].d1 = ((RGB *)obj->dat)->b;
	    for (c=16; c<256; c++)
	       ((RGB *)obj->dat)[c] = ((RGB *)obj->dat)[c&0x0f];
	 }
      }
   }

   if (nest_flag) {
      d->dp = obj;
      obj = NULL;
   }
   return ret;
}



DIALOG pallete_dlg[] =
{
   { clear_proc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL },
   { d_text_proc, 0, 0, 1, 1, 15, 0, 0, 0, 0, 0, "Pallete:" },
   { my_edit_proc, 72, 0, (ONAME_SIZE+1)*8, 8, 15, 0, 0, D_GOTFOCUS, ONAME_SIZE, 0, NULL },
   { color_proc, 160, 48, 100, 100, 15, 1, 0, 0, 0, 0, NULL },
   { pal_counter_proc, 64, 48, 80, 16, 15, 0, 0, 0, 0, 0, NULL },
   { d_button_proc, 200, 170, 100, 16, 15, 0, 13, D_EXIT, 0, 0, "OK" },
   { sel_pal_proc, 80, 170, 100, 16, 15, 0, CTRL('S'), D_EXIT, 0, 0, "Select" },
   { c_counter_proc, 64, 92, 80, 16, 15, 0, 0, 0, 0, 255, NULL },
   { c_counter_proc, 64, 112, 80, 16, 15, 0, 0, 0, 0, 255, NULL },
   { c_counter_proc, 64, 132, 80, 16, 15, 0, 0, 0, 0, 255, NULL },
   { pal_colors_proc, 200, 1, 100, 16, 15, 0, CTRL('C'), D_EXIT, 0, 0, NULL },
   { d_text_proc, 8, 48, 1, 1, 15, 0, 0, 0, 0, 0, "Color:" },
   { d_text_proc, 40, 92, 1, 1, 15, 0, 0, 0, 0, 0, "R:" },
   { d_text_proc, 40, 112, 1, 1, 15, 0, 0, 0, 0, 0, "G:" },
   { d_text_proc, 40, 132, 1, 1, 15, 0, 0, 0, 0, 0, "B:" },
   { NULL }
};



void edit_pallete(obj,name)
DATAFILE *obj;
DATANAME *name;
{
   pallete_dlg[2].dp = name->name;        /* edit proc */
   pallete_dlg[3].d1 = 0;                 /* current color */
   pallete_dlg[3].dp = obj->dat;          /* pallete pointer */
   pallete_dlg[4].d1 = 0;                 /* current color */
   pallete_dlg[4].d2 = (obj->type==DAT_PALLETE_16) ? 15 : 255;
   pallete_dlg[7].d1 = ((RGB *)obj->dat)->r;
   pallete_dlg[8].d1 = ((RGB *)obj->dat)->g;
   pallete_dlg[9].d1 = ((RGB *)obj->dat)->b;
   pallete_dlg[10].dp = obj;
   do_dialog(pallete_dlg);
}



void edit_object(obj,name,first)
DATAFILE *obj;
DATANAME *name;
short first;
{
   DATAFILE *oldf;
   DATANAME *oldn;

   short c, c2;

   if (datafile_malloced <= obj_count) {
      oldf = datafile;
      if (datafile)
	 datafile = realloc(datafile,sizeof(DATAFILE)*(obj_count+4));
      else
	 datafile = malloc(sizeof(DATAFILE)*(obj_count+4));
      if (!datafile) {
	 alert(ns_mem, NULL, NULL, "OK", NULL, 13, 0);
	 destroy_obj(obj);
	 datafile = oldf;
	 return;
      } 
      datafile_malloced=obj_count+4;
   }

   if (dataname_malloced <= obj_count) {
      oldn = dataname;
      if (dataname)
	 dataname = realloc(dataname,sizeof(DATANAME)*(obj_count+4));
      else
	 dataname = malloc(sizeof(DATANAME)*(obj_count+4));
      if (!dataname) {
	 alert(ns_mem, NULL, NULL, "OK", NULL, 13, 0);
	 destroy_obj(obj);
	 dataname = oldn;
	 return;
      } 
      dataname_malloced=obj_count+4;
   }

   switch(obj->type) {

   case DAT_DATA:
      if (!first)
	 edit_data(obj, name);
      break;

   case DAT_FONT:
      edit_font(obj, name);
      break;

   case DAT_BITMAP_16: 
   case DAT_BITMAP_256: 
   case DAT_SPRITE_16: 
   case DAT_SPRITE_256: 
      if (!first)
	 edit_bitmap(obj, name);
      break;

   case DAT_PALLETE_16:
   case DAT_PALLETE_256:
      edit_pallete(obj, name);
      break;
   }

   for (c=0; c<obj_count; c++)
      if (strcmp(dataname[c].name, name->name) > 0)
	 break;

   for (c2=obj_count-1; c2>=c; c2--) {
      datafile[c2+1] = datafile[c2];
      dataname[c2+1] = dataname[c2];
   }

   datafile[c] = *obj;
   dataname[c] = *name;
   obj_count++;
   CUR_ITEM = c;
   if ((CUR_TOP > c) || (CUR_TOP < c-18)) {
      CUR_TOP = c - 9;
      if (CUR_TOP > obj_count-18)
	 CUR_TOP = obj_count-18;
      if (CUR_TOP < 0)
	 CUR_TOP = 0;
   }
}



void destroy_olist()
{
   register i;
   
   for (i=0; i<obj_count; i++)
      destroy_obj(datafile+i);
   
   if (datafile)
      free(datafile);
   if (dataname)
      free(dataname);
   datafile = NULL;
   dataname = NULL;
   obj_count = datafile_malloced = dataname_malloced = 0;
}



void destroy_obj(o)
DATAFILE *o;
{
   switch (o->type) {
   
   case DAT_DATA:
   case DAT_FONT:
   case DAT_PALLETE_16:
   case DAT_PALLETE_256:
      free(o->dat);
      break;
   
   case DAT_BITMAP_16:
   case DAT_BITMAP_256:
   case DAT_SPRITE_16:
   case DAT_SPRITE_256:
      destroy_bitmap(o->dat);
      break;
   }
}



void load()
{
   FILE *f = NULL;
   char buf[80];
   char buf2[20];
   short c, c2, i;
   short ch;
   char *s;
   short w, h;

   draw_wait();
   destroy_olist();
   main_dlg[3].dp = get_filename(dat_file);

   f = fopen(dat_file, F_READ_PACKED);
   if (!f) {
      alert("Error opening file", NULL, NULL, "Oh dear", NULL, 13, 0);
      return;
   }

   if (getl(f) != DAT_MAGIC) {
      fclose(f);
      alert("Bad data file format", NULL, NULL, "Oh dear", NULL, 13, 0);
      return;
   }
   
   obj_count = getw(f);
   if (errno) {
      obj_count = 0;
      goto err;
   }

   datafile = malloc(sizeof(DATAFILE)*obj_count);
   dataname = malloc(sizeof(DATANAME)*obj_count);
   if ((!datafile) || (!dataname)) {
      fclose(f);
      alert(ns_mem, NULL, NULL, "Oh dear", NULL, 13, 0);
      if (datafile)
	 free(datafile);
      if (dataname)
	 free(dataname);
      obj_count = 0;
      return;
   }
   datafile_malloced = dataname_malloced = obj_count;

   for (c=0; c<obj_count; c++) {
      datafile[c].type = DAT_END;
      datafile[c].size = 0;
      datafile[c].dat = NULL;
      sprintf(dataname[c].type_str,"<?>  <err %d>", c+1);
   }

   for (c=0; c<obj_count; c++) {

      datafile[c].type = getw(f);

      switch(datafile[c].type) {
      
      case DAT_DATA:
	 datafile[c].size = getl(f);
	 datafile[c].dat = lmalloc(datafile[c].size);
	 if (!datafile[c].dat) {
	    alert(ns_mem, NULL, NULL, "Oh dear", NULL, 13, 0);
	    fclose(f);
	    return;
	 }
	 _fread(datafile[c].dat, datafile[c].size, f);
	 if (errno) {
	    free(datafile[c].dat);
	    datafile[c].dat = NULL;
	 }
	 else
	    sprintf(dataname[c].type_str,"dat: <%d>", c+1);
	 break;

      case DAT_FONT: 
	 datafile[c].dat = malloc(sizeof(FONT));
	 if (!datafile[c].dat) {
	    alert(ns_mem, NULL, NULL, "Oh dear", NULL, 13, 0);
	    fclose(f);
	    return;
	 }
	 _fread(datafile[c].dat, sizeof(FONT), f);
	 if (errno) {
	    free(datafile[c].dat);
	    datafile[c].dat = NULL;
	 }
	 else
	    sprintf(dataname[c].type_str,"fnt: <%d>", c+1);
	 break;

      case DAT_BITMAP_16: 
	 w = getw(f);
	 h = getw(f);
	 datafile[c].dat = create_bitmap(w,h);
	 if (!datafile[c].dat) {
	    alert(ns_mem, NULL, NULL, "Oh dear", NULL, 13, 0);
	    fclose(f);
	    return;
	 }
	 _load_st_data(((BITMAP *)datafile[c].dat)->line[0],
		       (long)w / 2 * (long)h, f);
	 if (errno) {
	    destroy_bitmap(datafile[c].dat);
	    datafile[c].dat = NULL;
	 }
	 else
	    sprintf(dataname[c].type_str,"bmp: <%d>", c+1);
	 break;

      case DAT_BITMAP_256:
	 w = getw(f);
	 h = getw(f);
	 datafile[c].dat = create_bitmap(w,h);
	 if (!datafile[c].dat) {
	    alert(ns_mem, NULL, NULL, "Oh dear", NULL, 13, 0);
	    fclose(f);
	    return;
	 }
	 _load_pc_data(((BITMAP *)datafile[c].dat)->line[0],
		       (long)w * (long)h, f);
	 if (errno) {
	    destroy_bitmap(datafile[c].dat);
	    datafile[c].dat = NULL;
	 }
	 else
	    sprintf(dataname[c].type_str,"bmp: <%d>", c+1);
	 break;

      case DAT_SPRITE_16:
	 datafile[c].size = getw(f);
	 w = getw(f);
	 h = getw(f);
	 datafile[c].dat = create_bitmap(w,h);
	 if (!datafile[c].dat) {
	    alert(ns_mem, NULL, NULL, "Oh dear", NULL, 13, 0);
	    fclose(f);
	    return;
	 }
	 _load_st_data(((BITMAP *)datafile[c].dat)->line[0],
		       (long)w / 2 * (long)h, f);
	 if (errno) {
	    destroy_bitmap(datafile[c].dat);
	    datafile[c].dat = NULL;
	 }
	 else
	    sprintf(dataname[c].type_str,"spr: <%d>", c+1);
	 break;

      case DAT_SPRITE_256: 
	 datafile[c].size = getw(f);
	 w = getw(f);
	 h = getw(f);
	 datafile[c].dat = create_bitmap(w,h);
	 if (!datafile[c].dat) {
	    alert(ns_mem, NULL, NULL, "Oh dear", NULL, 13, 0);
	    fclose(f);
	    return;
	 }
	 _load_pc_data(((BITMAP *)datafile[c].dat)->line[0],
		       (long)w * (long)h, f);
	 if (errno) {
	    destroy_bitmap(datafile[c].dat);
	    datafile[c].dat = NULL;
	 }
	 else
	    sprintf(dataname[c].type_str,"spr: <%d>", c+1);
	 break;

      case DAT_PALLETE_16: 
      case DAT_PALLETE_256:
	 datafile[c].dat = malloc(sizeof(RGB)*256);
	 if (!datafile[c].dat) {
	    alert(ns_mem, NULL, NULL, "Oh dear", NULL, 13, 0);
	    fclose(f);
	    return;
	 }
	 if (datafile[c].type==DAT_PALLETE_16) {
	    RGB *rgb = (RGB *)datafile[c].dat;
	    _fread(datafile[c].dat, sizeof(RGB)*16, f);
	    for (c2=16; c2<256; c2++)
	       rgb[c2] = rgb[c2&0x0f];
	 }
	 else
	    _fread(datafile[c].dat, sizeof(RGB)*256, f);
	 if (errno) {
	    free(datafile[c].dat);
	    datafile[c].dat = NULL;
	 }
	 else
	    sprintf(dataname[c].type_str,"pal: <%d>", c+1);
	 break;
      }

      if (errno) {
	 datafile[c].type = DAT_END;
	 goto err;
      }
   }

   fclose(f);
   f = NULL;

   if (errno)
      goto err;

   strcpy(buf, dat_file);
   s = get_extension(buf);
   if ((s > buf) & (*(s-1)=='.'))
      strcpy(s,"H");
   else
      strcpy(s,".H");
   f = fopen(buf, F_READ);
   if (!f) {
      alert("Unable to open header file", get_filename(buf), NULL, "Oh well...", NULL, 13, 0);
      return;
   }

   c = 0;
   ch = getc(f);

   while (ch != EOF) { 
      if ((c>=80) || (ch=='\r') || (ch=='\n')) {
	 buf[c] = 0;
	 if (c > 8) {
	    if ((buf[0]=='#') && (buf[1]=='d') && (buf[2]=='e') &&
		(buf[3]=='f') && (buf[4]=='i') && (buf[5]=='n') &&
		(buf[6]=='e') && (buf[7]==' ')) {
	       c2 = 0;
	       c = 8;
	       while ((buf[c]) && (buf[c]!=' ') && (c2 < ONAME_SIZE))
		  buf2[c2++] = buf[c++];
	       buf2[c2] = 0;
	       while (buf[c]==' ')
		  c++;
	       i = 0;
	       while ((buf[c] >= '0') && (buf[c] <= '9')) {
		  i *= 10;
		  i += buf[c] - '0';
		  c++;
	       }
	       if (i < obj_count)
		  strcpy(dataname[i].name, buf2);
	    }
	 }
	 c = 0; 
      }
      else
	 buf[c++] = ch;

      ch = getc(f);
   }

   fclose(f);
   f = NULL;
   if (errno)
      alert("Error reading header file", NULL, NULL, "Oh dear", NULL, 13, 0);
   return;

   err:
   alert("Error loading file", NULL, NULL, "Oh dear", NULL, 13, 0);
   if (f)
      fclose(f);
}



void save(pack_flag)
short pack_flag;
{
   FILE *df = NULL;
   FILE *hf = NULL;
   char buf[80], buf2[80];
   short c;
   char *s;
   long size, outsize;
   BITMAP *b;
   
   main_dlg[3].dp = get_filename(dat_file);

   df = fopen(dat_file, (pack_flag) ? F_WRITE_PACKED : F_WRITE_NOPACK);
   if (!df)
      goto err;
   
   strcpy(buf, dat_file);
   s = get_extension(buf);
   if ((s > buf) & (*(s-1)=='.'))
      strcpy(s,"H");
   else
      strcpy(s,".H");
   hf = fopen(buf, F_WRITE);
   if (!hf)
      goto err;

   putl(DAT_MAGIC, df);
   putw(obj_count, df);
   size = 6;

   _fwrite((unsigned char *)"/* Allegro data file object indexes */\r\n/* Do not hand edit! */\r\n\r\n", 67, hf);
   if (errno)
      goto err;

   for (c=0; c<obj_count; c++) {

      show_mouse(NULL);
      rectfill(screen, 61, 73, 259, 127, 0);
      rect(screen, 60, 72, 260, 128, 15);
      textmode(0);
      textout(screen, font, "Please wait a moment", 80, 84, 15);
      textout(screen, font, dataname[c].type_str, 160-strlen(dataname[c].type_str)*4, 108, 15);
      show_mouse(screen);

      putw(datafile[c].type, df);
      size += 2;

      switch(datafile[c].type) {
      
      case DAT_DATA:
	 putl(datafile[c].size, df);
	 _fwrite(datafile[c].dat, datafile[c].size, df);
	 size += datafile[c].size + 4;
	 break;

      case DAT_FONT: 
	 _fwrite(datafile[c].dat, sizeof(FONT), df);
	 size += sizeof(FONT);
	 break;

      case DAT_SPRITE_16:
	 b = (BITMAP *)datafile[c].dat;
	 putw((short)datafile[c].size, df);
	 putw(b->w, df);
	 putw(b->h, df);
	 save_st_data(b->line[0], df, (long)b->w / 2 * (long)b->h);
	 size += (long)b->w / 2 * (long)b->h + 6;
	 break;

      case DAT_SPRITE_256:
	 b = (BITMAP *)datafile[c].dat;
	 putw((short)datafile[c].size, df);
	 putw(b->w, df);
	 putw(b->h, df);
	 save_pc_data(b->line[0], df, (long)b->w * (long)b->h);
	 size += (long)b->w * (long)b->h + 6;
	 break;

      case DAT_BITMAP_16:
	 b = (BITMAP *)datafile[c].dat;
	 putw(b->w, df);
	 putw(b->h, df);
	 save_st_data(b->line[0], df, (long)b->w / 2 * (long)b->h);
	 size += (long)b->w / 2 * (long)b->h + 4;
	 break;

      case DAT_BITMAP_256:
	 b = (BITMAP *)datafile[c].dat;
	 putw(b->w, df);
	 putw(b->h, df);
	 save_pc_data(b->line[0], df, (long)b->w * (long)b->h);
	 size += (long)b->w * (long)b->h + 4;
	 break;

      case DAT_PALLETE_16: 
	 _fwrite(datafile[c].dat, sizeof(RGB)*16, df);
	 size += sizeof(RGB)*16;
	 break;

      case DAT_PALLETE_256: 
	 _fwrite(datafile[c].dat, sizeof(RGB)*256, df);
	 size += sizeof(RGB)*256;
	 break;
      }

      if (errno)
	 goto err;

      sprintf(buf, "#define %-14s%-8d/* %c%c%c */\r\n",
	      dataname[c].name, c, dataname[c].type_str[0],
	      dataname[c].type_str[1], dataname[c].type_str[2]);

      _fwrite((unsigned char *)buf, strlen(buf), hf);
      if (errno)
	 goto err; 
   }
   
   fclose(df);
   fclose(hf);
   df = hf = NULL;
   if (!errno) {
      outsize = file_size(dat_file);
      if (pack_flag) {
	 sprintf(buf, "%ld bytes packed into", size);
	 sprintf(buf2, "%ld bytes - %ld%%", outsize, (outsize*100L+size/2)/size);
      }
      else {
	 sprintf(buf, "%ld bytes - unpacked", outsize);
	 buf2[0]=0;
      }
      alert("File saved:", buf, buf2, "OK", NULL, 13, 0);
      return;
   }

   err:
   alert("Error saving file", NULL, NULL, "Oh dear", NULL, 13, 0);
   if (df)
      fclose(df);
   if (hf)
      fclose(hf);
}



char *list_getter(index, list_size)
short index;
short *list_size;
{
   if (index < 0) {
      if (list_size)
	 *list_size = obj_count;
      return NULL;
   }
   return dataname[index].type_str;
}



short list_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (d_list_proc(msg,d,ch)==D_CLOSE) {
      edit_proc(MSG_KEY, main_dlg+12, 0);
      return D_REDRAW;
   }
   return D_O_K;
}



short load_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      if (obj_count)
	 if (alert("Abandon current data file?", NULL, NULL, "Yes", "Cancel", 13, 27)==2)
	    return D_REDRAW;
      draw_wait();
      if (file_select("Load data file", dat_file))
	 load();
      return D_REDRAW;
   }
   return D_O_K;
}



short save_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (d_button_proc(msg,d,ch)==D_CLOSE) {

      char buf[80], buf2[80];
      short pack = main_dlg[4].flags & D_SELECTED;
      short e1, e2;
      char *s;

      if (pack)
	 strcpy(buf,"Save data file (packed)");
      else
	 strcpy(buf,"Save data file (unpacked)");

      strcpy(buf2,dat_file);
      draw_wait();

      if (file_select(buf, dat_file)) {
	 if (strcmp(dat_file,buf2)) {
	    strcpy(buf2,dat_file);
	    s = get_extension(buf2);
	    if ((s > buf2) & (*(s-1)=='.'))
	       strcpy(s,"H");
	    else
	       strcpy(s,".H");
	    e1 = file_exists(dat_file, F_RDONLY | F_HIDDEN, NULL);
	    e2 = file_exists(buf2, F_RDONLY | F_HIDDEN, NULL);
	    if (e1 || e2) {
	       if (e1) {
		  strcpy(buf,get_filename(dat_file));
		  if (e2)
		     strcat(buf," and ");
	       }
	       else
		  buf[0]=0;
	       if (e2)
		  strcat(buf,get_filename(buf2));
	       strcat(buf,"?");
	       strcpy(buf2,"Overwrite existing file");
	       if (e1 && e2)
		  strcat(buf2,"s");
	       if (alert(buf2,buf,NULL,"Yes","Cancel",13,27)==2)
		  return D_REDRAW;
	    } 
	 }
	 save(pack);
      }
      return D_REDRAW;
   }
   return D_O_K;
}



short import_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   short c;

   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      draw_wait();
      *get_filename(imp_file)=0;
      if (file_select("Import Graphics (Degas, NEO, PCX)", imp_file)) {
	 char *s;
	 draw_wait();
	 if (graphic)
	    destroy_bitmap(graphic);
	 graphic = NULL;
	 s = get_extension(imp_file);
	 if (strcmp(s,"PI1")==0) {
	    if (load_degas())
	       view_proc(MSG_KEY,main_dlg+8,0);
	 }
	 else 
	    if (strcmp(s,"PC1")==0) {
	       if (load_degas_compressed())
		  view_proc(MSG_KEY,main_dlg+8,0);
	    }
	    else
	       if (strcmp(s,"NEO")==0) {
		  if (load_neo())
		      view_proc(MSG_KEY,main_dlg+8,0);
	       }
	       else
		  if (strcmp(s,"PCX")==0) {
		     if (load_pcx())
			view_proc(MSG_KEY,main_dlg+8,0);
		  }
		  else
		     alert("You must specify a", "PI1, PC1, NEO, or PCX file!", NULL, "OK", NULL, 13, 0);

	 for (c=0; c<256; c++)
	    selected_pallete[c] = g_pallete[c];
      }
      return D_REDRAW;
   }
   return D_O_K;
}



short view_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      if (graphic) {
	 PALLETE p;
	 short c;
	 for (c=0; c<PAL_SIZE; c++)
	    p[c] = rgbtopal(g_pallete[c]);
	 show_mouse(NULL);
	 clear(screen);
	 set_pallete(p);
	 blit(graphic,screen,0,0,0,0,SCREEN_W,SCREEN_H);
	 clear_keybuf();
	 do {
	 } while (mouse_b);
	 do {
	 } while ((!mouse_b) && (!keypressed()));
	 clear_keybuf();
	 do {
	 } while (mouse_b);
	 clear(screen); 
	 set_pallete(desktop_pallete);
	 show_mouse(screen);
      }
      else
	 alert("Nothing to view!", "You must import a", "graphics file first", "OK", NULL, 13, 0);
      return D_REDRAW;
   }
   return D_O_K;
}



void do_export()
{
   char *s;

   draw_wait();
   *get_filename(imp_file)=0;

   if (file_select("Export graphics (PI1, NEO, PCX)", imp_file)) {
      if (file_exists(imp_file, F_RDONLY | F_HIDDEN, NULL))
	 if (alert("File already exists.", "Overwrite?", NULL, "OK", "Cancel", 13, 27)==2)
	    return;

      draw_wait();
      s = get_extension(imp_file);
      if (strcmp(s,"PI1")==0)
	 save_degas();
      else
	 if (strcmp(s,"NEO")==0)
	    save_neo();
	 else
	    if (strcmp(s,"PCX")==0)
	       save_pcx();
	    else
	       alert("You must specify a", "PI1, NEO, or PCX file!", NULL, "OK", NULL, 13, 0);
   }
}



short bmp_colors_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   d->dp = ((bmp->type==DAT_BITMAP_16) ||
	    (bmp->type==DAT_SPRITE_16)) ? "16 colors" : "256 colors";

   if (d_button_proc(msg,d,ch) == D_CLOSE) {
      if (bmp->type==DAT_BITMAP_16)
	 bmp->type=DAT_BITMAP_256;
      else {
	 if (bmp->type==DAT_SPRITE_16)
	    bmp->type=DAT_SPRITE_256;
	 else {
	    short x, y;
	    char *p;

	    if (bmp->dat) {
	       if (alert("Really convert 256 color", "data to 16 colors?", NULL, "Yes", "Cancel", 13, 27)==2)
		  return D_REDRAW;

	       for (y=0; y<((BITMAP *)bmp->dat)->h; y++) {
		  p = ((BITMAP *)bmp->dat)->line[y];
		  for (x=0; x<((BITMAP *)bmp->dat)->w; x++)
		     p[x] &= 0x0f;
	       }
	    }
	    if (bmp->type==DAT_BITMAP_256)
	       bmp->type=DAT_BITMAP_16;
	    else
	       bmp->type=DAT_SPRITE_16;
	 }
      }
      return D_REDRAW;
   }
   return D_O_K;
}



short bmp_text_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (msg==MSG_START) {
      if ((bmp->type==DAT_SPRITE_16) || (bmp->type==DAT_SPRITE_256))
	 d->dp = "Sprite: ";
      else
	 d->dp = "Bitmap: ";
   }
   
   return d_text_proc(msg,d,ch);
}



short bmp_exit_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (msg==MSG_START) {
      if (bmp->dat) {
	 d->dp = "OK";
	 d->key = 13;
      }
      else {
	 d->dp = "Cancel";
	 d->key = 27;
      } 
   }
   
   return d_button_proc(msg,d,ch);
}



DIALOG grab_ok_dlg[] =
{
   { d_button_proc, 200, 170, 100, 16, 15, 0, 13, D_EXIT, 0, 0, "OK" },
   { d_button_proc, 80, 170, 100, 16, 15, 0, 27, D_EXIT, 0, 0, "Cancel" },
   { NULL }
};



short bmp_grab_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (msg==MSG_START) {
      if (bmp->dat) {
	 d->dp = "Export";
	 d->key = CTRL('E');
      }
      else {
	 d->dp = "Grab";
	 d->key = 13;
      }
   }
   
   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      if (bmp->dat) {                           /* export */
	 do_export();
	 return D_REDRAW;
      }

      else {                                    /* grab */
	 if ((bmp_dlg[7].d1==0) || (bmp_dlg[8].d1==0)) {
	    alert("Can't grab an", "image of zero size!", NULL, "OK", NULL, 13, 0);
	    return D_REDRAW;
	 }

	 bmp->dat = create_bitmap(bmp_dlg[7].d1, bmp_dlg[8].d1);
	 if (!bmp->dat)
	    alert(ns_mem, NULL, NULL, "Oh dear", NULL, 13, 0);

	 else {
	    PALLETE p;
	    short c;
	    short x, y, ox, oy;

	    if (((bmp->type==DAT_BITMAP_16) || (bmp->type==DAT_SPRITE_16)) &&
		(g_pallete_size==256))
	       alert("Warning: 256 color", "image data will be", "grabbed using 16 colors", "OK", NULL, 13, 0);

	    for (c=0; c<PAL_SIZE; c++)
	       p[c] = rgbtopal(selected_pallete[c]);

	    show_mouse(NULL);
	    if ((graphic->w < SCREEN_W) || (graphic->h < SCREEN_H))
	       greyscale(screen);
	    set_pallete(p);
	    set_clip(screen, 0, 0, graphic->w-1, graphic->h-1);
	    do {
	    } while(mouse_b);
	    ox = oy = -1;

	    while (!mouse_b) {
	       x = mouse_x & 0xfff0;
	       if (x+((BITMAP *)bmp->dat)->w > graphic->w)
		  x = (graphic->w-((BITMAP *)bmp->dat)->w) & 0xfff0;
	       if (x<0)
		  x=0;
	       y = mouse_y & 0xfff0;
	       if (y+((BITMAP *)bmp->dat)->h > graphic->h)
		  y = (graphic->h - ((BITMAP *)bmp->dat)->h) & 0xfff0;
	       if (y<0)
		  y=0;
	       if ((x!=ox) || (y!=oy)) {
		  blit(graphic,screen,0,0,0,0,SCREEN_W,SCREEN_H);
		  rect(screen, x-1, y-1, x+((BITMAP *)bmp->dat)->w,
		       y+((BITMAP *)bmp->dat)->h, 15);
		  ox = x;
		  oy = y;
	       }
	    }

	    set_clip(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);
	    clear(bmp->dat);
	    blit(graphic, bmp->dat, ox, oy, 0, 0,
		 ((BITMAP *)bmp->dat)->w, ((BITMAP *)bmp->dat)->h);

	    if ((bmp->type==DAT_BITMAP_16) || (bmp->type==DAT_SPRITE_16))
	       for (x=0; x<((BITMAP *)bmp->dat)->w; x++)
		  for (y=0; y<((BITMAP *)bmp->dat)->h; y++)
		     ((BITMAP *)bmp->dat)->line[y][x] &= 0x0f;

	    if ((((BITMAP *)bmp->dat)->w < SCREEN_W) ||
		(((BITMAP *)bmp->dat)->h < SCREEN_H))
	       greyscale(screen);
	    if ((bmp->type==DAT_BITMAP_16) || (bmp->type==DAT_BITMAP_256))
	       blit(bmp->dat,screen,0,0,0,0,SCREEN_W,SCREEN_H);
	    else {
	       SPRITE *s = create_sprite((short)bmp->size,
					 ((BITMAP *)bmp->dat)->w,
					 ((BITMAP *)bmp->dat)->h);
	       if (!s) {
		  textmode(0);
		  textout(screen, font, ns_mem, 104, 96, 15);
	       }
	       else {
		  get_sprite(s, bmp->dat, 0, 0);
		  drawsprite(screen, s, 32, 32);
		  destroy_sprite(s);
	       }
	    }

	    show_mouse(screen);
	    c = 15;
	    while ((selected_pallete[c].r == selected_pallete[0].r) &&
		   (selected_pallete[c].g == selected_pallete[0].g) &&
		   (selected_pallete[c].b == selected_pallete[0].b)) {
	       c++;
	       if (c >= 256)
		  c = 1;
	       if (c==15)
		  break;
	    }
	    grab_ok_dlg[0].fg = grab_ok_dlg[1].fg = c;
	    if (do_dialog(grab_ok_dlg)==1) {
	       destroy_bitmap(bmp->dat);
	       bmp->dat = NULL;
	    }

	    clear(screen);
	    set_pallete(desktop_pallete);
	 }

	 return D_CLOSE;
      }
   }

   return D_O_K;
}



short bmp_view_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (!bmp->dat)
      return D_O_K;
   
   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      PALLETE p;
      short c;
      for (c=0; c<PAL_SIZE; c++)
	 p[c] = rgbtopal(selected_pallete[c]);
      show_mouse(NULL);
      if ((((BITMAP *)bmp->dat)->w < SCREEN_W) ||
	  (((BITMAP *)bmp->dat)->h < SCREEN_H))
	 greyscale(screen);
      set_pallete(p);
      if ((bmp->type==DAT_BITMAP_16) || (bmp->type==DAT_BITMAP_256))
	 blit(bmp->dat,screen,0,0,0,0,SCREEN_W,SCREEN_H);
      else {
	 SPRITE *s = create_sprite((short)bmp->size, ((BITMAP *)bmp->dat)->w,
				   ((BITMAP *)bmp->dat)->h);
	 if (!s) {
	    textmode(0);
	    textout(screen, font, ns_mem, 104, 96, 15);
	 }
	 else {
	    get_sprite(s, bmp->dat, 0, 0);
	    drawsprite(screen, s, 32, 32);
	    destroy_sprite(s);
	 } 
      }
      clear_keybuf();
      do {
      } while (mouse_b);
      do {
      } while ((!mouse_b) && (!keypressed()));
      clear_keybuf();
      do {
      } while (mouse_b);
      clear(screen); 
      set_pallete(desktop_pallete);
      show_mouse(screen);
      return D_REDRAW;
   }

   return D_O_K;
}



short bmp_w_counter_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   static char number[6];
   short x;
   short first = TRUE;
   short c;

   if ((((msg==MSG_CLICK) || (msg==MSG_DCLICK)) &&
	(bmp->dat)) ||
       (msg==MSG_KEY))
      return D_O_K;

   if (msg==MSG_CLICK) {
      while(mouse_b) {
	 x = d->d1;
	 if (mouse_b==2)
	    d->d1+=16;
	 else
	    if (mouse_b&1)
	       d->d1-=16;
	 if (d->d1 >= d->d2)
	    d->d1 = d->d2 & 0xfff0;
	 else
	    if (d->d1 < 16)
	       d->d1 = 16;
	 if (x != d->d1) {
	    show_mouse(NULL);
	    (*d->proc)(MSG_DRAW,d,0);
	    show_mouse(screen);
	    if (first) {
	       for (c=0; c<16; c++) {
		  rest(20);
		  if (!mouse_b)
		     break;
	       }
	       first = FALSE;
	    }
	    else
	       rest(40);
	 }
      }
      return D_O_K;
   }

   if (msg==MSG_DRAW) {
      d->dp = number;
      sprintf(number, "%d", d->d1);
   }

   return d_button_proc(msg,d,ch);
}



short bmp_h_counter_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (((msg==MSG_CLICK) || (msg==MSG_DCLICK)) &&
       (bmp->dat))
      return D_O_K;

   if (msg==MSG_KEY) {
      rest(20);
      return D_O_K;
   }

   return counter_proc(msg,d,ch);
}



short mask_check_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   short ret;

   if ((bmp->type==DAT_BITMAP_16) || (bmp->type==DAT_BITMAP_256))
      return D_O_K;

   ret = d_check_proc(msg,d,ch);

   if (d->flags & D_SELECTED)
      bmp->size |= SPRITE_MASKED;
   else
      bmp->size &= ~SPRITE_MASKED;
   
   return ret;
}



DIALOG bmp_dlg[] = 
{
   { clear_proc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL },
   { bmp_text_proc, 0, 0, 1, 1, 15, 0, 0, 0, 0, 0, NULL },
   { my_edit_proc, 64, 0, (ONAME_SIZE+1)*8, 8, 15, 0, 0, D_GOTFOCUS, ONAME_SIZE, 0, NULL },
   { bmp_exit_proc, 200, 170, 100, 16, 15, 0, 13, D_EXIT, 0, 0, NULL },
   { bmp_grab_proc, 80, 170, 100, 16, 15, 0, 13, D_EXIT, 0, 0, NULL },
   { bmp_colors_proc, 200, 1, 100, 16, 15, 0, CTRL('C'), D_EXIT, 0, 0, NULL },
   { bmp_view_proc, 200, 146, 100, 16, 15, 0, CTRL('V'), D_EXIT, 0, 0, "View" },
   { bmp_h_counter_proc, 60, 64, 80, 16, 15, 0, 0, 0, 32, 0, NULL },
   { bmp_h_counter_proc, 230, 64, 80, 16, 15, 0, 0, 0, 32, 0, NULL },
   { d_text_proc, 4, 68, 1, 1, 15, 0, 0, 0, 0, 0, "Width:" },
   { d_text_proc, 166, 68, 1, 1, 15, 0, 0, 0, 0, 0, "Height:" },
   { mask_check_proc, 48, 100, 64, 16, 15, 0, 0, 0, 0, 0, "Mask:" },
   { NULL }
};



short grab_bitmap_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   static short bc = 1;

   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      DATAFILE o;
      DATANAME d;
      sprintf(d.type_str, "bmp: BITMAP%d", bc++);
      o.type = (g_pallete_size==256) ? DAT_BITMAP_256 : DAT_BITMAP_16;
      o.dat = NULL;
      bmp_dlg[2].dp = d.name;
      bmp_dlg[7].d2 = graphic->w;
      if (bmp_dlg[7].d1 > bmp_dlg[7].d2)
	 bmp_dlg[7].d1 = bmp_dlg[7].d2;
      bmp_dlg[8].d2 = graphic->h;
      if (bmp_dlg[8].d1 > bmp_dlg[8].d2)
	 bmp_dlg[8].d1 = bmp_dlg[8].d2;
      bmp = &o;
      do_dialog(bmp_dlg); 
      bmp = NULL;
      if (o.dat)
	 edit_object(&o,&d,TRUE);
      return D_CLOSE;
   }
   return D_O_K;
}



short grab_sprite_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   static short sc = 1;

   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      DATAFILE o;
      DATANAME d;
      sprintf(d.type_str, "spr: SPRITE%d", sc++);
      o.type = (g_pallete_size==256) ? DAT_SPRITE_256 : DAT_SPRITE_16;
      o.size = 0;
      o.dat = NULL;
      bmp_dlg[2].dp = d.name;
      bmp_dlg[7].d2 = graphic->w;
      if (bmp_dlg[7].d1 > bmp_dlg[7].d2)
	 bmp_dlg[7].d1 = bmp_dlg[7].d2;
      bmp_dlg[8].d2 = graphic->h;
      if (bmp_dlg[8].d1 > bmp_dlg[8].d2)
	 bmp_dlg[8].d1 = bmp_dlg[8].d2;
      bmp = &o;
      do_dialog(bmp_dlg);
      bmp = NULL;
      if (o.dat)
	 edit_object(&o,&d,TRUE);
      return D_CLOSE;
   }
   return D_O_K;
}



short grab_pallete_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   static short pc = 1;

   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      DATAFILE o;
      DATANAME d;
      short c;
      sprintf(d.type_str, "pal: PALLETE%d", pc++);
      o.type = (g_pallete_size==256) ? DAT_PALLETE_256 : DAT_PALLETE_16;
      o.size = sizeof(RGB) * 256;
      o.dat = malloc(sizeof(RGB) * 256);
      if (!o.dat)
	 alert("Out of memory", NULL ,NULL, "OK", NULL, 13, 0);
      else {
	 for (c=0; c<256; c++)
	    ((RGB *)o.dat)[c] = g_pallete[c];
	 edit_object(&o,&d,TRUE);
      }
      return D_CLOSE;
   }
   return D_O_K;
}



DIALOG grab_dlg[] =
{
   { d_shadow_box_proc, 80, 40, 160, 120, 15, 0, 0, 0, 0, 0, NULL },
   { grab_sprite_proc, 100, 56, 120, 16, 15, 0, 's', D_EXIT, 0, 0, "Grab Sprite" },
   { grab_bitmap_proc, 100, 80, 120, 16, 15, 0, 'b', D_EXIT, 0, 0, "Grab Bitmap" },
   { grab_pallete_proc, 100, 104, 120, 16, 15, 0, 'p', D_EXIT, 0, 0, "Grab Pallete" },
   { d_button_proc, 100, 128, 120, 16, 15, 0, 27, D_EXIT, 0, 0, "Cancel" },
   { NULL }
};



short grab_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      if (graphic) {
	 do_dialog(grab_dlg);
      }
      else
	  alert("Nothing to grab!", "You must import a", "graphics file first", "OK", NULL, 13, 0);
      return D_REDRAW;
   }
   return D_O_K;
}



short font_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   static short fc = 1;

   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      DATAFILE o;
      DATANAME d;
      sprintf(d.type_str, "fnt: FONT%d", fc++);
      o.type = DAT_FONT;
      o.size = sizeof(FONT);
      o.dat = malloc(sizeof(FONT));
      if (!o.dat)
	 alert("Out of memory", NULL ,NULL, "OK", NULL, 13, 0);
      else {
	 *((FONT *)o.dat) = *font;
	 edit_object(&o,&d,TRUE);
      }
      return D_REDRAW;
   }
   return D_O_K;
}



short data_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      draw_wait();
      *get_filename(imp_file) = 0;
      if (file_select("Import Data", imp_file)) {
	 DATAFILE o;
	 DATANAME d;
	 if (file_exists(imp_file, F_RDONLY | F_HIDDEN | F_ARCH, NULL)) {
	    d.type_str[0] = 'd';
	    d.type_str[1] = 'a';
	    d.type_str[2] = 't';
	    d.type_str[3] = ':';
	    d.type_str[4] = ' ';
	    strcpy(d.name,get_filename(imp_file));
	    o.type = DAT_DATA;
	    o.size = file_size(imp_file);
	    o.dat = lmalloc(o.size);
	    if (!o.dat)
	       alert("Out of memory", NULL ,NULL, "OK", NULL, 13, 0);
	    else {
	       FILE *f = fopen(imp_file, F_READ);
	       if (f) {
		  _fread(o.dat, o.size, f);
		  fclose(f);
		  if (errno) {
		     alert("Error reading file", NULL, NULL, "OK", NULL, 13, 0);
		     free(o.dat);
		  }
		  else
		     edit_object(&o,&d,TRUE);
	       }
	       else {
		  alert("Error opening file", NULL, NULL, "OK", NULL, 13, 0);
		  free(o.dat);
	       }
	    }
	 }
	 else
	    alert("File not found", NULL, NULL, "OK", NULL, 13, 0);
      }
      return D_REDRAW;
   }
   return D_O_K;
}



short edit_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   DATAFILE o;
   DATANAME n;
   short c;

   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      if (obj_count) {
	 o = datafile[CUR_ITEM];
	 n = dataname[CUR_ITEM];
	 for (c=CUR_ITEM; c<obj_count-1; c++) {
	    datafile[c] = datafile[c+1];
	    dataname[c] = dataname[c+1];
	 }
	 obj_count--;
	 edit_object(&o,&n,FALSE);
      }
      else
	 alert("Nothing to edit!", NULL, NULL, "OK", NULL, 13, 0);
      return D_REDRAW;
   }
   return D_O_K;
}



short delete_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   char buf[40];
   short c;

   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      if (obj_count) {
	 strcpy(buf,dataname[CUR_ITEM].type_str);
	 strcat(buf,"?");
	 if (alert("Really delete", buf, NULL, "OK", "Cancel", 13, 27)==1) {
	    destroy_obj(datafile+CUR_ITEM);
	    for (c=CUR_ITEM; c<obj_count-1; c++) {
	       datafile[c] = datafile[c+1];
	       dataname[c] = dataname[c+1];
	    }
	    obj_count--;
	    if (CUR_ITEM >= obj_count) {
	       if (obj_count)
		  CUR_ITEM = obj_count-1;
	       else
		  CUR_ITEM = 0;
	    }
	    if (CUR_TOP > CUR_ITEM)
	       CUR_TOP = CUR_ITEM;
	 }
      }
      else
	 alert("Nothing to delete!", NULL, NULL, "OK", NULL, 13, 0);
      return D_REDRAW;
   }
   return D_O_K;
}



short quit_proc(msg,d,ch)
short msg;
DIALOG *d;
unsigned long ch;
{
   if (d_button_proc(msg,d,ch)==D_CLOSE) {
      if (alert("Really want to quit?", NULL, NULL, "Quit", "Cancel", 13, 27)==1)
	 return D_CLOSE;
      else
	 return D_REDRAW;
   }
   return D_O_K;
}



DIALOG main_dlg[] =
{
   { clear_proc, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NULL },
#ifdef BORLAND
   { d_text_proc, 28, 0, 1, 1, 15, 0, 0, 0, 0, 0, "Allegro Data File Editor (Borland)" },
#else
   { d_text_proc, 36, 0, 1, 1, 15, 0, 0, 0, 0, 0, "Allegro Data File Editor (djgpp)" },
#endif
   { d_text_proc, 0, 16, 1, 1, 15, 0, 0, 0, 0, 0, "Editing:" },
   { d_text_proc, 72, 16, 1, 1, 15, 0, 0, 0, 0, 0, dat_file },
   { d_check_proc, 196, 23, 100, 12, 15, 0, 'p', D_SELECTED, 0, 0, "Pack file:" },
   { load_proc, 0, 28, 76, 16, 15, 0, 'l', D_EXIT, 0, 0, "Load" },
   { save_proc, 92, 28, 76, 16, 15, 0, 's', D_EXIT, 0, 0, "Save" },
   { import_proc, 186, 43, 132, 16, 15, 0, 'i', D_EXIT, 0, 0, "Import Graphics" },
   { view_proc, 186, 63, 132, 16, 15, 0, 'v', D_EXIT, 0, 0, "View Graphics" },
   { grab_proc, 186, 83, 132, 16, 15, 0, 'g', D_EXIT, 0, 0, "Grab Graphics" },
   { font_proc, 186, 103, 132, 16, 15, 0, 'f', D_EXIT, 0, 0, "Create Font" },
   { data_proc, 186, 123, 132, 16, 15, 0, 'd', D_EXIT, 0, 0, "Import Data" },
   { edit_proc, 186, 143, 132, 16, 15, 0, 0, D_EXIT, 0, 0, "Edit Item" },
   { delete_proc, 186, 163, 132, 16, 15, 0, 8, D_EXIT, 0, 0, "Delete Item" },
   { quit_proc, 186, 183, 132, 16, 15, 0, 27, D_EXIT, 0, 0, "Quit" },
   { list_proc, 0, 52, 168, 147, 15, 0, 13, D_EXIT | D_GOTFOCUS, 0, 0, list_getter },
   { NULL }
};



void main()
{
   short c;

   allegro_init();
   install_mouse();
   install_keyboard();
   set_pallete(desktop_pallete);
   for (c=0; c<PAL_SIZE; c++)
      selected_pallete[c] = g_pallete[c] = paltorgb(desktop_pallete[c]);
   for (c=16; c<256; c++)
      selected_pallete[c] = g_pallete[c] = selected_pallete[c&0x0f];

   do_dialog(main_dlg);

   destroy_olist();
   if (graphic)
      destroy_bitmap(graphic);

   allegro_exit();
   exit(0);
}


