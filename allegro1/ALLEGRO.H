/*
		  //  /     /     ,----  ,----.  ,----.  ,----.
		/ /  /     /     /      /    /  /    /  /    /
	      /  /  /     /     /___   /       /____/  /    /
	    /---/  /     /     /      /  __   /\      /    /
	  /    /  /     /     /      /    /  /  \    /    /
	/     /  /____ /____ /____  /____/  /    \  /____/

	Low Level Game Routines (version 1.0)

	Main header file for the Allegro library.

	See allegro.txt for instructions and copyright conditions.

	By Shawn Hargreaves,
	1 Salisbury Road,
	Market Drayton,
	Shropshire,
	England TF9 1AJ
	email slh100@tower.york.ac.uk (until 1996)
*/

#ifndef ALLEGRO_H
#define ALLEGRO_H

/* Are we using Borland or djgpp? */

#ifdef __GNUC__
#define GCC
#else
#define BORLAND
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TRUE            /* define these if they don't already exist */
#define TRUE    -1
#define FALSE   0
#endif

#ifndef NULL            /* might need this if stdio is not being used */
#define NULL    ((char *)0L)
#endif

#ifndef MIN
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#define MID(x,y,z)   MAX(x,MIN(y,z))
#endif

#ifndef ABS
#define ABS(x)       (((x) >= 0) ? (x) : (-x))
#endif

#ifndef SGN
#define SGN(x)       (((x) >= 0) ? 1 : -1)
#endif

#define SCREEN_W     320
#define SCREEN_H     200

#define PAL_SIZE     256

#define TICKS_PER_SECOND   200

short allegro_init(void);
void allegro_exit(void);

void install_mouse(void);
void remove_mouse(void);

extern short allegro_use_timer;

struct BITMAP;
void show_mouse(struct BITMAP *bmp);

extern void (*port_callback)();

extern struct SPRITE *mouse_sprite;

extern volatile short mouse_x;
extern volatile short mouse_y;
extern volatile short mouse_b;

short install_int(void (*proc)(), short speed);
void remove_int(void (*proc)());
void rest(long time);

void install_keyboard(void);
void remove_keyboard(void);

extern volatile char key[128];

long readkey(void);
short keypressed(void);
void clear_keybuf(void);

#define KEY_ESC         1        /* keyboard scan codes */
#define KEY_1           2 
#define KEY_2           3 
#define KEY_3           4
#define KEY_4           5
#define KEY_5           6
#define KEY_6           7
#define KEY_7           8
#define KEY_8           9
#define KEY_9           10
#define KEY_0           11
#define KEY_MINUS       12
#define KEY_EQUALS      13
#define KEY_BACKSPACE   14
#define KEY_TAB         15 
#define KEY_Q           16
#define KEY_W           17
#define KEY_E           18
#define KEY_R           19
#define KEY_T           20
#define KEY_Y           21
#define KEY_U           22
#define KEY_I           23
#define KEY_O           24
#define KEY_P           25
#define KEY_OPENBRACE   26
#define KEY_CLOSEBRACE  27
#define KEY_ENTER       28
#define KEY_CONTROL     29
#define KEY_A           30
#define KEY_S           31
#define KEY_D           32
#define KEY_F           33
#define KEY_G           34
#define KEY_H           35
#define KEY_J           36
#define KEY_K           37
#define KEY_L           38
#define KEY_COLON       39
#define KEY_QUOTE       40
#define KEY_TILDE       41
#define KEY_LSHIFT      42
#define KEY_Z           44
#define KEY_X           45
#define KEY_C           46
#define KEY_V           47
#define KEY_B           48
#define KEY_N           49
#define KEY_M           50
#define KEY_COMMA       51
#define KEY_STOP        52
#define KEY_SLASH       53
#define KEY_RSHIFT      54
#define KEY_ASTERISK    55
#define KEY_ALT         56
#define KEY_SPACE       57
#define KEY_CAPSLOCK    58
#define KEY_F1          59
#define KEY_F2          60
#define KEY_F3          61
#define KEY_F4          62
#define KEY_F5          63
#define KEY_F6          64
#define KEY_F7          65
#define KEY_F8          66
#define KEY_F9          67
#define KEY_F10         68
#define KEY_NUMLOCK     69
#define KEY_SCRLOCK     70
#define KEY_HOME        71
#define KEY_UP          72
#define KEY_PGUP        73
#define KEY_MINUS_PAD   74
#define KEY_LEFT        75
#define KEY_5_PAD       76
#define KEY_RIGHT       77
#define KEY_PLUS_PAD    78
#define KEY_END         79
#define KEY_DOWN        80
#define KEY_PGDN        81
#define KEY_INSERT      82
#define KEY_DEL         83
#define KEY_F11         87
#define KEY_F12         88




/*****************************************************/
/************ Bitmap and pallete routines ************/
/*****************************************************/

typedef struct RGB {
   unsigned char r, g, b;
} RGB;

typedef short _ATARI_RGB;

typedef struct _PC_RGB {
   unsigned char r, g, b;
} _PC_RGB;

RGB _atari_paltorgb(_ATARI_RGB);
_ATARI_RGB _atari_rgbtopal(RGB);
RGB _pc_paltorgb(_PC_RGB);
_PC_RGB _pc_rgbtopal(RGB);

#define _RGB    _PC_RGB
#define paltorgb(c)     _pc_paltorgb(c)
#define rgbtopal(c)     _pc_rgbtopal(c)

typedef _RGB PALLETE[PAL_SIZE];

extern PALLETE black_pallete, desktop_pallete;

void vsync(void);
void set_pallete(PALLETE p);
void get_pallete(PALLETE p);
void fade_in(PALLETE p, short speed);
void fade_out(short speed);

typedef struct BITMAP      /* a bitmap structure */
{
   short w, h;               /* width and height in pixels */
   short clip;               /* flag if clipping is turned on */
   short cl, cr, ct, cb;     /* clip left, right, top and bottom values */
   long size;                /* amount of memory required (in bytes) */
   char *dat;                /* Don't use this! For free() only */
#ifndef GCC
   char *line[];             /* pointers to the start of each scan line */
#else
   short seg;                /* bitmap segment, 0 = _go32_my_ds() */
   char *line[0];
#endif;
} BITMAP;

extern BITMAP *screen;

BITMAP *create_bitmap(short width, short height);
void destroy_bitmap(BITMAP *bitmap);
void set_clip(BITMAP *bitmap, short x1, short y1, short x2, short y2);
void show(BITMAP *bitmap);
void clear(BITMAP *bitmap);



/******************************************************/
/************ Graphics and sprite routines ************/
/******************************************************/

void blit(BITMAP *source, BITMAP *dest, short source_x, short source_y,
	  short dest_x, short dest_y, short width, short height);
void putpixel(BITMAP *bmp, short x, short y, short color);
short getpixel(BITMAP *bmp, short x, short y);
void line(BITMAP *bmp, short x1, short y1, short x2, short y2, short color);
void vline(BITMAP *bmp, short x, short y1, short y2, short color);
void hline(BITMAP *bmp, short x1, short y, short x2, short color);
void polyline(BITMAP *bmp, short n, short *points, short color);
void triangle(BITMAP *bmp, short x1, short y1, short x2, short y2,
	      short x3, short y3, short color);
void rect(BITMAP *bmp, short x1, short y1, short x2, short y2, short color);
void rectfill(BITMAP *bmp, short x1, short y1, short x2, short y2, short color);
void circle(BITMAP *bmp, short x, short y, short radius, short color);
void circlefill(BITMAP *bmp, short x, short y, short radius, short color);

typedef struct FONT
{
   char dat[95][8];
} FONT;

extern FONT *font;

void textmode(short mode);
void textout(BITMAP *bmp, FONT *font, char *str, short x, short y, short color);

#define SPRITE_OPAQUE      0
#define SPRITE_MASKED      1

typedef struct SPRITE
{
   short flags;              /* masked? fastword? */
   short w;                  /* must be 16 or 32 */
   short h;                  /* anything */
#ifndef GCC
   char dat[];
#else
   short filler;             /* for alignment */
   char dat[0];
#endif;
} SPRITE;

SPRITE *create_sprite(short flags, short width, short height);
void destroy_sprite(SPRITE *sprite);
void drawsprite(BITMAP *bmp, SPRITE *sprite, short x, short y);
void get_sprite(SPRITE *sprite, BITMAP *bmp, short x, short y);



/***********************************************************/
/************ File I/O and compression routines ************/
/***********************************************************/

char *get_filename(char *path);
char *get_extension(char *filename);
void put_backslash(char *filename);
short file_exists(char *filename, short attrib, short *aret);
long file_size(char *filename);
short delete_file(char *filename);
short for_each_file(char *name, short attrib, void (*call_back)(), short param);

#define F_RDONLY   0x01 
#define F_HIDDEN   0x02
#define F_SYSTEM   0x04
#define F_VOLUME   0x08
#define F_SUBDIR   0x10
#define F_ARCH     0x20 

#ifndef STDIO_H   /* only use our own file system if we don't have stdio... */
#ifndef EOF       /* stdio.h might not define STDIO_H so check this too */

#define EOF    -1

#define F_READ          "r"      /* constants for use with fopen() */
#define F_WRITE         "w"
#define F_READ_PACKED   "rp"
#define F_WRITE_PACKED  "wp"
#define F_WRITE_NOPACK  "w!"

#define F_BUF_SIZE   2048           /* 2K buffer for caching data */
#define F_PACK_MAGIC    0x736C6821L /* magic number for packed files */
#define F_NOPACK_MAGIC  0x736C682EL /* magic number for autodetect */

typedef struct FILE                 /* our very own FILE structure... */
{
   int hndl;                        /* OS file handle */
   char write;                      /* is file being read or written? */
   char pack;                       /* is file being compressed? */
   char eof;                        /* end of file flag */
   char error;                      /* error flag */
   unsigned char *buf_pos;          /* position in buffer */
   short buf_size;                  /* number of bytes in the buffer */
   long todo;                       /* number of bytes waiting on the disk */
   void *pack_data;                 /* for the LZSS compression routines */
   unsigned char buf[F_BUF_SIZE];   /* the actual data buffer */
} FILE;

   /* We do these as externs with funny names, and use macros to implement
      the stdio syntax, so that people can use the stdio versions if they
      want without the linker getting confused. */

FILE *_my_fopen(char *filename, char *mode);
short _my_fclose(FILE *f);
short _sort_out_getc(FILE *f);
short _my_getw(FILE *f);
short _my_igetw(FILE *f);
long _my_getl(FILE *f);
long _my_igetl(FILE *f);
short _sort_out_putc(short c, FILE *f);
short _my_putw(short w, FILE *f);
short _my_iputw(short w, FILE *f);
long _my_putl(long l, FILE *f);
long _my_iputl(long l, FILE *f);
long _my_fread(void *p, long n, FILE *f);
long _my_fwrite(void *p, long n, FILE *f);

#define getc(f)            ((--((f)->buf_size) > 0) ? \
			    *((f)->buf_pos++) : _sort_out_getc(f))

#define putc(c,f)          ((++((f)->buf_size) >= F_BUF_SIZE) ? \
			      _sort_out_putc(c,f) : (*((f)->buf_pos++)=(c)))

#define fopen(name,mode)   _my_fopen(name,mode)
#define fclose(f)          _my_fclose(f)
#define feof(f)            (f->eof)
#define ferror(f)          (f->error)
#define fgetc(f)           getc(f)
#define getw(f)            _my_getw(f)
#define igetw(f)           _my_igetw(f)
#define getl(f)            _my_getl(f)
#define igetl(f)           _my_igetl(f)
#define fputc(c,f)         putc(c,f)
#define putw(w,f)          _my_putw((short)w,f)
#define iputw(w,f)         _my_iputw((short)w,f)
#define putl(l,f)          _my_putl((long)l,f)
#define iputl(l,f)         _my_iputl((long)l,f)
#define _fread(p,n,f)      _my_fread(p,(long)n,f)
#define fread(p,size,n,f)  (_my_fread(p,(long)(size)*(long)(n),f) / size)
#define _fwrite(p,n,f)     _my_fwrite(p,(long)n,f)
#define fwrite(p,size,n,f) (_my_fwrite(p,(long)(size)*(long)(n),f) / size)

#endif      /* ifndef EOF */
#endif      /* ifndef STDIO_H */

#define DAT_MAGIC          0x616C6C2EL    /* magic number for data files */

#define DAT_END            -1
#define DAT_DATA           0
#define DAT_FONT           1
#define DAT_BITMAP_16      2
#define DAT_BITMAP_256     3
#define DAT_SPRITE_16      4
#define DAT_SPRITE_256     5
#define DAT_PALLETE_16     6
#define DAT_PALLETE_256    7

typedef struct DATAFILE
{
   short type;
   long size;
   void *dat;
} DATAFILE;

DATAFILE *load_datafile(char *filename);
void unload_datafile(DATAFILE *dat);



/***********************************************************/
/************ Fixed point (16.16) math routines ************/
/***********************************************************/

typedef long fixed;

#define itofix(x)    (((long)(x)) << 16)
#define fixtoi(x)    (((x) & 0x8000) ? (((x) >> 16) + 1) : ((x) >> 16))
#define ftofix(x)    ((long)((x) * 65536.0 + (x < 0 ? -0.5 : 0.5)))
#define fixtof(x)    (((float)(x)) / 65536.0)

fixed fmul(fixed x, fixed y);
fixed fdiv(fixed x, fixed y);

void install_divzero(void);
void remove_divzero(void);

extern fixed _cos_tbl[];
extern fixed _tan_tbl[];

fixed facos(fixed x);
fixed fatan(fixed x);
fixed fatan2(fixed y, fixed x);
fixed fsqrt(fixed x);

#define fcos(x)      _cos_tbl[(((x) & 0x4000) ? ((short)((x) >> 15) + 1) : (short)((x) >> 15)) & 0x1ff]
#define fsin(x)      _cos_tbl[((((x) & 0x4000) ? ((short)((x) >> 15) + 1) : (short)((x) >> 15)) -128) & 0x1ff]
#define ftan(x)      _tan_tbl[(((x) & 0x4000) ? ((short)((x) >> 15) + 1) : (short)((x) >> 15)) & 0xff]
#define fasin(x)     (0x00400000L - facos(x))



/***************************************/
/************ GUI routines  ************/
/***************************************/

typedef struct DIALOG            /* a GUI object */
{
   short (*proc)(short, struct DIALOG *, unsigned long);
			/* pointer to dialog procedure (NULL = end of list) */
   short x, y, w, h;             /* the position and size of the object */
   char fg, bg;                  /* foreground and background colors */
   char key;                     /* keyboard shortcut (ASCII code) */
   char flags;                   /* flags about the object state */
   short d1, d2;                 /* any data the object might require */
   void *dp;                     /* pointer to more object data */
} DIALOG;

				 /* bits for the flags field */
#define D_SELECTED      1        /* object is selected */
#define D_GOTFOCUS      2        /* object has the input focus */
#define D_EXIT          4        /* this object causes the dialog to exit */

				 /* return values for the dialog procedures */
#define D_O_K           0        /* normal exit status */
#define D_CLOSE         1        /* request to close the dialog */
#define D_REDRAW        2        /* request to redraw the dialog */

				 /* some prewritten dialog procedures */
short d_box_proc(short msg, DIALOG *d, unsigned long c);
						      /* no data required */
short d_shadow_box_proc(short msg, DIALOG *d, unsigned long c);
						      /* no data required */
short d_text_proc(short msg, DIALOG *d, unsigned long c);
						      /* dp = string */
short d_button_proc(short msg, DIALOG *d, unsigned long c);
						      /* dp = string */
short d_check_proc(short msg, DIALOG *d, unsigned long c);
						      /* dp = string */
short d_edit_proc(short msg, DIALOG *d, unsigned long c);
					     /* dp = string, d1 = length */
short d_list_proc(short msg, DIALOG *d, unsigned long c);
					     /* dp = callback function,
						d1 = selected item,
						d2 = item at top of box */

				 /* messages to pass to dialog procedures */
#define MSG_START       0        /* start the dialog, initialise */
#define MSG_DRAW        1        /* draw the object */
#define MSG_CLICK       2        /* mouse click on the object */
#define MSG_DCLICK      3        /* double click on the object */
#define MSG_KEY         4        /* keyboard shortcut */
#define MSG_WANTFOCUS   5        /* does object want the input focus? */
#define MSG_GOTFOCUS    6        /* got the input focus */
#define MSG_LOSTFOCUS   7        /* lost the input focus */
#define MSG_CHAR        8        /* key pressed (object must have focus) */
#define MSG_END         9        /* dialog is finished - cleanup */

short do_dialog(DIALOG *dialog);
short popup_dialog(DIALOG *dialog);
short alert(char *s1, char *s2, char *s3, char *b1, char *b2, char c1, char c2);
short file_select(char *message, char *path);

#ifdef __cplusplus
}
#endif

#endif          /* ifndef ALLEGRO_H */

