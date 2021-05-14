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
 *      Main header file for the Allegro library.
 *      This should be included by everyone and everything.
 *
 *      See readme.txt for copyright information.
 */


#if DJGPP < 2
#error Allegro requires djgpp v2
#endif


#ifndef ALLEGRO_H
#define ALLEGRO_H

#ifdef __cplusplus
extern "C" {
#endif


#define VERSION_STR     "2.0"
#define DATE_STR        "1996"



/*******************************************/
/************ Some global stuff ************/
/*******************************************/

#include "stdlib.h"
#include "stddef.h"
#include "dpmi.h"

#ifndef TRUE 
#define TRUE         -1
#define FALSE        0
#endif

#ifndef MIN
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#define MID(x,y,z)   MAX(x, MIN(y, z))
#endif

#ifndef ABS
#define ABS(x)       (((x) >= 0) ? (x) : (-x))
#endif

#ifndef SGN
#define SGN(x)       (((x) >= 0) ? 1 : -1)
#endif

typedef long fixed;

struct BITMAP;
struct SAMPLE;
struct MIDI;

extern char allegro_id[];
extern char allegro_error[];

int allegro_init();
void allegro_exit();

void lock_bitmap(struct BITMAP *bmp);
void lock_sample(struct SAMPLE *spl);
void lock_midi(struct MIDI *midi);

#define END_OF_FUNCTION(x) void x##_end() { }

#define LOCK_VARIABLE(x)   _go32_dpmi_lock_data((void *)&x, sizeof(x))
#define LOCK_FUNCTION(x)   _go32_dpmi_lock_code(x, (long)x##_end - (long)x)



/****************************************/
/************ Mouse routines ************/
/****************************************/

int install_mouse();
void remove_mouse();

extern volatile int mouse_x;
extern volatile int mouse_y;
extern volatile int mouse_b;

void set_mouse_sprite(struct BITMAP *sprite);
void show_mouse(struct BITMAP *bmp);
void position_mouse(int x, int y);



/****************************************/
/************ Timer routines ************/
/****************************************/

#define TIMERS_PER_SECOND     1193181L
#define SECS_TO_TIMER(x)      ((long)x * TIMERS_PER_SECOND)
#define MSEC_TO_TIMER(x)      ((long)x * (TIMERS_PER_SECOND / 1000))
#define BPS_TO_TIMER(x)       (TIMERS_PER_SECOND / (long)x)
#define BPM_TO_TIMER(x)       ((60 * TIMERS_PER_SECOND) / (long)x)

int install_timer();
void remove_timer();

int install_int_ex(void (*proc)(), long speed);
int install_int(void (*proc)(), long speed);
void remove_int(void (*proc)());
void rest(long time);



/*******************************************/
/************ Keyboard routines ************/
/*******************************************/

int install_keyboard();
void remove_keyboard();

extern volatile char key[128];

extern int three_finger_flag;

int readkey();
int keypressed();
void clear_keybuf();

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



/*******************************************/
/************ Joystick routines ************/
/*******************************************/

extern int joy_x, joy_y;
extern int joy_left, joy_right, joy_up, joy_down;
extern int joy_b1, joy_b2;

int initialise_joystick();
int calibrate_joystick_tl();
int calibrate_joystick_br();

void poll_joystick();

int save_joystick_data(char *filename);
int load_joystick_data(char *filename);



/******************************************/
/************ Pallete routines ************/
/******************************************/

typedef struct RGB
{
   unsigned char r, g, b;
   unsigned char filler;
} RGB;

#define PAL_SIZE     256

typedef RGB PALLETE[PAL_SIZE];

extern RGB black_rgb;
extern PALLETE black_pallete, desktop_pallete;

void vsync();
void set_color(int index, RGB *p);
void set_pallete(RGB *p);
void get_color(int index, RGB *p);
void get_pallete(RGB *p);
void fade_in(RGB *p, int speed);
void fade_out(int speed);



/************************************************/
/************ Screen/bitmap routines ************/
/************************************************/

#define GFX_TEXT              -1
#define GFX_AUTODETECT        0
#define GFX_VGA               1
#define GFX_VESA1             2
#define GFX_VESA2B            3
#define GFX_VESA2L            4
#define GFX_CIRRUS64          5
#define GFX_CIRRUS54          6
#define GFX_S3                7
#define GFX_ET3000            8
#define GFX_ET4000            9


typedef struct GFX_DRIVER     /* creates and manages the screen bitmap */
{
   char *name;                /* driver name */
   char *desc;                /* description (VESA version, etc) */
   struct BITMAP *(*init)(int w, int h, int v_w, int v_h);
   void (*exit)(struct BITMAP *b);
   void (*scroll)(int x, int y);
   int w, h;                  /* physical (not virtual!) screen size */
   int linear;                /* true if video memory is linear */
   long bank_size;            /* bank size, in bytes */
   long bank_gran;            /* bank granularity, in bytes */
   long vid_mem;              /* video memory size, in bytes */
} GFX_DRIVER;


extern GFX_DRIVER gfx_vga, gfx_vesa_1, gfx_vesa_2b, gfx_vesa_2l, gfx_cirrus64, 
		  gfx_cirrus54, gfx_s3, gfx_et3000, gfx_et4000;

extern GFX_DRIVER *gfx_driver;


typedef struct BITMAP         /* a bitmap structure */
{
   int w, h;                  /* width and height in pixels */
   int clip;                  /* flag if clipping is turned on */
   int cl, cr, ct, cb;        /* clip left, right, top and bottom values */
   void *dat;                 /* the memory we allocated for the bitmap */
   int seg;                   /* bitmap segment */
   void (*write_bank)();      /* write bank selector, see bank.s */
   void (*read_bank)();       /* read bank selector, see bank.s */
   unsigned char *line[0];    /* pointers to the start of each line */
} BITMAP;


extern BITMAP *screen;

#define SCREEN_W     (gfx_driver ? gfx_driver->w : 0)
#define SCREEN_H     (gfx_driver ? gfx_driver->h : 0)

#define VIRTUAL_W    (screen ? screen->w : 0)
#define VIRTUAL_H    (screen ? screen->h : 0)

int set_gfx_mode(int card, int w, int h, int v_w, int v_h);
int scroll_screen(int x, int y);

BITMAP *create_bitmap(int width, int height);
void destroy_bitmap(BITMAP *bitmap);

void set_clip(BITMAP *bitmap, int x1, int y1, int x2, int y2);



/******************************************************/
/************ Graphics and sprite routines ************/
/******************************************************/

void xor_mode(int xor);
int  getpixel(BITMAP *bmp, int x, int y);
void putpixel(BITMAP *bmp, int x, int y, int color);
void vline(BITMAP *bmp, int x, int y1, int y2, int color);
void hline(BITMAP *bmp, int x1, int y, int x2, int color);
void do_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(BITMAP *, int, int, int));
void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void triangle(BITMAP *bmp, int x1, int y1, int x2, int y2, int x3, int y3, int color);
void polygon(BITMAP *bmp, int color, int vertices, ...);
void rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void do_circle(BITMAP *bmp, int x, int y, int radius, int d, void (*proc)(BITMAP *, int, int, int));
void circle(BITMAP *bmp, int x, int y, int radius, int color);
void circlefill(BITMAP *bmp, int x, int y, int radius, int color);
void draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y);
void draw_sprite_v_flip(BITMAP *bmp, BITMAP *sprite, int x, int y);
void draw_sprite_h_flip(BITMAP *bmp, BITMAP *sprite, int x, int y);
void draw_sprite_vh_flip(BITMAP *bmp, BITMAP *sprite, int x, int y);
void rotate_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, fixed angle);
void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void stretch_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height);
void clear_to_color(BITMAP *bitmap, int color);
void clear(BITMAP *bitmap);


typedef struct RLE_SPRITE     /* a RLE compressed sprite */
{
   int w, h;                  /* width and height in pixels */
   int size;                  /* size of sprite data in bytes */
   signed char dat[0];        /* RLE bitmap data */
} RLE_SPRITE;


RLE_SPRITE *get_rle_sprite(BITMAP *bitmap);
void destroy_rle_sprite(RLE_SPRITE *sprite);
void draw_rle_sprite(BITMAP *bmp, RLE_SPRITE *sprite, int x, int y);


#define FONT_SIZE    95             /* number of characters in a font */


typedef struct FONT_8x8             /* a simple 8x8 font */
{
   unsigned char dat[FONT_SIZE][8];
} FONT_8x8;


typedef struct FONT_PROP            /* a proportional font */
{
   BITMAP *dat[FONT_SIZE]; 
} FONT_PROP;


typedef struct FONT                 /* can be either */
{
   int flag_8x8;
   union {
      FONT_8x8 *dat_8x8;
      FONT_PROP *dat_prop;
   } dat;
} FONT;


extern FONT *font;

void text_mode(int mode);
void textout(BITMAP *bmp, FONT *f, char *str, int x, int y, int color);
void textout_centre(BITMAP *bmp, FONT *f, char *str, int x, int y, int color);
int text_length(FONT *f, char *str);
void destroy_font(FONT *f);



/*********************************************/
/************ Video memory access ************/
/*********************************************/

extern __inline__ unsigned long bmp_write_line(BITMAP *bmp, int line)
{
   unsigned long result;

   asm (
      "  call *%3 "

   : "=a" (result)                        /* result in eax */

   : "d" (bmp),                           /* bitmap in edx */
     "0" (line),                          /* line number in eax */
     "r" (bmp->write_bank)                /* the bank switch routine */
   );

   return result;
}


extern __inline__ unsigned long bmp_read_line(BITMAP *bmp, int line)
{
   unsigned long result;

   asm (
      "  call *%3 "

   : "=a" (result)                        /* result in eax */

   : "d" (bmp),                           /* bitmap in edx */
     "0" (line),                          /* line number in eax */
     "r" (bmp->read_bank)                 /* the bank switch routine */
   );

   return result;
}


extern __inline__ void _putpixel(BITMAP *bmp, int x, int y, int color)
{
   asm (
      "  movw %w0, %%fs ; "
      "  .byte 0x64 ; "
      "  movb %b3, (%1, %2) "
   :                                      /* no outputs */

   : "rm" (bmp->seg),                     /* segment selector in reg or mem */
     "r" (bmp_write_line(bmp, y)),        /* line pointer in reg */
     "r" (x),                             /* line offset in reg */
     "qi" (color)                         /* the pixel in reg or immediate */
   );
}



/****************************************/
/************ Sound routines ************/
/****************************************/

#define DIGI_VOICES           16       /* Theoretical maximums. */
#define MIDI_TRACKS           32       /* Actual drivers may not be */ 
#define MIDI_VOICES           32       /* able to handle this many */


typedef struct SAMPLE                  /* a sample */
{
   int bits;                           /* currently always 8 */
   int freq;                           /* sample frequency */
   unsigned long len;                  /* sample length in bytes */
   unsigned char *data;                /* sample data */
} SAMPLE;


typedef struct MIDI                    /* a midi file */
{
   int divisions;                      /* number of ticks per quarter note */
   struct {
      unsigned char *data;             /* MIDI message stream */
      int len;                         /* length of the track data */
   } track[MIDI_TRACKS]; 
} MIDI;


#define DIGI_AUTODETECT       -1       /* for passing to install_sound() */
#define DIGI_NONE             0 
#define DIGI_SB               1
#define DIGI_SB10             2 
#define DIGI_SB15             3 
#define DIGI_SB20             4 
#define DIGI_SBPRO            5 
#define DIGI_SB16             6 
#define DIGI_GUS              7

#define MIDI_AUTODETECT       -1 
#define MIDI_NONE             0 
#define MIDI_ADLIB            1 
#define MIDI_OPL2             2 
#define MIDI_2XOPL2           3 
#define MIDI_OPL3             4
#define MIDI_SB_OUT           5
#define MIDI_MPU              6 
#define MIDI_GUS              7


typedef struct DIGI_DRIVER             /* driver for playing digital sfx */
{
   char *name;                         /* driver name */
   char *desc;                         /* description (ports, irq, etc) */
   int voices;                         /* max simultaneous samples */
   int (*detect)(); 
   int (*init)(); 
   void (*exit)(); 
   void (*play)(int voice, SAMPLE *spl, int vol, int pan, int freq, int loop);
   void (*adjust)(int voice, int vol, int pan, int freq, int loop);
   void (*stop)(int voice);
   unsigned long (*voice_status)(int voice);
} DIGI_DRIVER;


typedef struct MIDI_DRIVER             /* driver for playing midi music */
{
   char *name;                         /* driver name */
   char *desc;                         /* description (ports, irq, etc) */
   int voices;                         /* max simultaneous samples */
   int (*detect)();
   int (*init)();
   void (*exit)();
   int (*load_patches)(char *patches, char *drums);
   void (*key_on)(int voice, int inst, int note, int bend, int vol, int pan);
   void (*key_off)(int voice);
   void (*set_volume)(int voice, int vol);
   void (*set_pitch)(int voice, int note, int bend);
   void (*raw_midi)(unsigned char data);
} MIDI_DRIVER;


extern DIGI_DRIVER digi_none, digi_sb, digi_gus;
extern MIDI_DRIVER midi_none, midi_adlib, midi_sb_out, midi_mpu401, midi_gus;

extern DIGI_DRIVER *digi_driver;       /* the drivers currently in use */
extern MIDI_DRIVER *midi_driver;

extern int digi_card, midi_card;

extern volatile long midi_pos;         /* current position in the midi file */

int install_sound(int digi, int midi, char *cfg_path); 
void remove_sound();

SAMPLE *load_sample(char *filename);
void destroy_sample(SAMPLE *spl);
void play_sample(SAMPLE *spl, int vol, int pan, int freq, int loop);
void adjust_sample(SAMPLE *spl, int vol, int pan, int freq, int loop);
void stop_sample(SAMPLE *spl);

MIDI *load_midi(char *filename);
void destroy_midi(MIDI *midi);
int play_midi(MIDI *midi, int loop);
void stop_midi();



/***********************************************************/
/************ File I/O and compression routines ************/
/***********************************************************/

char *get_filename(char *path);
char *get_extension(char *filename);
void put_backslash(char *filename);
int file_exists(char *filename, int attrib, int *aret);
long file_size(char *filename);
int delete_file(char *filename);
int for_each_file(char *name, int attrib, void (*callback)(), int param);

#ifndef EOF 
#define EOF    -1
#endif

#define F_READ          "r"            /* for use with pack_fopen() */
#define F_WRITE         "w"
#define F_READ_PACKED   "rp"
#define F_WRITE_PACKED  "wp"
#define F_WRITE_NOPACK  "w!"

#define F_BUF_SIZE      4096           /* 4K buffer for caching data */
#define F_PACK_MAGIC    0x736C6821L    /* magic number for packed files */
#define F_NOPACK_MAGIC  0x736C682EL    /* magic number for autodetect */


typedef struct PACKFILE                /* our very own FILE structure... */
{
   int hndl;                           /* DOS file handle */
   char write;                         /* is file being read or written? */
   char pack;                          /* is file being compressed? */
   char eof;                           /* end of file flag */
   char error;                         /* error flag */
   unsigned char *buf_pos;             /* position in buffer */
   int buf_size;                       /* number of bytes in the buffer */
   long todo;                          /* number of bytes still on the disk */
   void *pack_data;                    /* for the LZSS compression routines */
   unsigned char buf[F_BUF_SIZE];      /* the actual data buffer */
} PACKFILE;


PACKFILE *pack_fopen(char *filename, char *mode);
int pack_fclose(PACKFILE *f);
int pack_igetw(PACKFILE *f);
long pack_igetl(PACKFILE *f);
int pack_iputw(int w, PACKFILE *f);
long pack_iputl(long l, PACKFILE *f);
int pack_mgetw(PACKFILE *f);
long pack_mgetl(PACKFILE *f);
int pack_mputw(int w, PACKFILE *f);
long pack_mputl(long l, PACKFILE *f);
long pack_fread(void *p, long n, PACKFILE *f);
long pack_fwrite(void *p, long n, PACKFILE *f);
char *pack_fgets(char *p, int max, PACKFILE *f);
int pack_fputs(char *p, PACKFILE *f);

int _sort_out_getc(PACKFILE *f);
int _sort_out_putc(int c, PACKFILE *f);

#define pack_getc(f)       ((--((f)->buf_size) > 0) ? \
			    *((f)->buf_pos++) : _sort_out_getc(f))

#define pack_putc(c,f)     ((++((f)->buf_size) >= F_BUF_SIZE) ? \
			      _sort_out_putc(c,f) : (*((f)->buf_pos++)=(c)))

#define pack_feof(f)       (f->eof)
#define pack_ferror(f)     (f->error)



/********************************************/
/************ Data file routines ************/
/********************************************/

#define DAT_MAGIC          0x616C6C2EL    /* magic number for data files */

#define DAT_END            -1
#define DAT_DATA           0
#define DAT_FONT_8x8       8
#define DAT_FONT_PROP      9
#define DAT_BITMAP         10
#define DAT_PALLETE        11
#define DAT_SAMPLE         12
#define DAT_MIDI           13
#define DAT_RLE_SPRITE     14


typedef struct DATAFILE
{
   void *dat;        /* pointer to the data */
   int type;         /* one of the DAT_* values defined above */
   long size;        /* size of binary data objects */
} DATAFILE;


DATAFILE *load_datafile(char *filename);
void unload_datafile(DATAFILE *dat);

BITMAP *load_pcx(char *filename, RGB *pal);
int save_pcx(char *filename, BITMAP *bmp, RGB *pal);



/***********************************************************/
/************ Fixed point (16.16) math routines ************/
/***********************************************************/

#define itofix(x)    (((long)(x)) << 16)
#define fixtoi(x)    (((x) >> 16) + (((x) & 0x8000) >> 15))
#define ftofix(x)    ((long)((x) * 65536.0 + (x < 0 ? -0.5 : 0.5)))
#define fixtof(x)    (((float)(x)) / 65536.0)

fixed fsqrt(fixed x);
fixed facos(fixed x);
fixed fatan(fixed x);
fixed fatan2(fixed y, fixed x);

extern fixed _cos_tbl[];
extern fixed _tan_tbl[];

#define fcos(x)   _cos_tbl[(((x) & 0x4000) ? ((int)((x) >> 15) + 1) : (int)((x) >> 15)) & 0x1ff]
#define fsin(x)   _cos_tbl[((((x) & 0x4000) ? ((int)((x) >> 15) + 1) : (int)((x) >> 15)) -128) & 0x1ff]
#define ftan(x)   _tan_tbl[(((x) & 0x4000) ? ((int)((x) >> 15) + 1) : (int)((x) >> 15)) & 0xff]
#define fasin(x)  (0x00400000L - facos(x))


extern __inline__ fixed fmul(fixed x, fixed y)
{
   fixed result;

   asm (
      "  movl %1, %0 ; "
      "  imull %2 ; "                     /* do the multiply */
      "  shrdl $16, %%edx, %0 ; "

      "  shrl $16, %%edx ; "              /* check for overflow */
      "  jz 0f ; "
      "  cmpw $0xFFFF, %%dx ; "
      "  je 0f ; "

      "  movl $2, _errno ; "              /* on overflow, set errno */
      "  movl $0x7fffffff, %0 ; "         /* and return MAXINT */
      "  cmpl $0, %1 ; " 
      "  jge 1f ; "
      "  negl %0 ; "
      " 1: "
      "  cmpl $0, %2 ; " 
      "  jge 0f ; "
      "  negl %0 ; "

      " 0: "                              /* finished */

   : "=&a" (result)                       /* the result has to go in eax */

   : "mr" (x),                            /* x and y can be regs or mem */
     "mr" (y) 

   : "edx", "cc"                          /* clobbers edx and flags */
   );

   return result;
}


extern __inline__ fixed fdiv(fixed x, fixed y)
{
   fixed result;

   asm (
      "  movl %2, %%ecx ; "
      "  xorl %%ebx, %%ebx ; "

      "  orl %0, %0 ; "                   /* test sign of x */
      "  jns 0f ; "

      "  negl %0 ; "
      "  incl %%ebx ; "

      " 0: "
      "  orl %%ecx, %%ecx ; "             /* test sign of y */
      "  jns 1f ; "

      "  negl %%ecx ; "
      "  incb %%ebx ; "

      " 1: "
      "  movl %0, %%edx ; "               /* check the range is ok */
      "  shrl $16, %%edx ; "
      "  shll $16, %0 ; "
      "  cmpl %%ecx, %%edx ; "
      "  jae 2f ; "

      "  divl %%ecx ; "                   /* do the divide */
      "  orl %0, %0 ; "
      "  jns 3f ; "

      " 2: "
      "  movl $2, _errno ; "              /* on overflow, set errno */
      "  movl $0x7fffffff, %0 ; "         /* and return MAXINT */

      " 3: "
      "  testl $1, %%ebx ; "              /* fix up the sign of the result */
      "  jz 4f ; "

      "  negl %0 ; "

      " 4: "                              /* finished */

   : "=a" (result)                        /* the result has to go in eax */

   : "0" (x),                             /* x in eax */
     "g" (y)                              /* y can be anywhere */

   : "ebx", "ecx", "edx", "cc"            /* clobbers ebx, ecx, edx + flags */
   );

   return result;
}


#ifdef __cplusplus

}  /* end of extern "C" */


class fix      /* C++ wrapper for the fixed point routines */
{
private:
   fixed v;

public:
   fix()                               { }
   fix(const fix &x)                   { v = x.v; }
   fix(const int x)                    { v = itofix(x); }
   fix(const long x)                   { v = itofix(x); }
   fix(const float x)                  { v = ftofix(x); }
   fix(const double x)                 { v = ftofix(x); }

   operator int()                      { return fixtoi(v); }
   operator long()                     { return fixtoi(v); }
   operator float()                    { return fixtof(v); }
   operator double()                   { return fixtof(v); }

   fix& operator = (const fix &x)      { v = x.v;           return *this; }
   fix& operator = (const int x)       { v = itofix(x);     return *this; }
   fix& operator = (const long x)      { v = itofix(x);     return *this; }
   fix& operator = (const float x)     { v = ftofix(x);     return *this; }
   fix& operator = (const double x)    { v = ftofix(x);     return *this; }

   fix& operator +=  (const fix x)     { v += x.v;          return *this; }
   fix& operator -=  (const fix x)     { v -= x.v;          return *this; }
   fix& operator *=  (const fix x)     { v = fmul(v, x.v);  return *this; }
   fix& operator *=  (const int x)     { v *= x;            return *this; }
   fix& operator /=  (const fix x)     { v = fdiv(v, x.v);  return *this; }
   fix& operator /=  (const int x)     { v /= x;            return *this; }
   fix& operator <<= (const int x)     { v <<= x;           return *this; }
   fix& operator >>= (const int x)     { v >>= x;           return *this; }

   inline friend fix operator +  (const fix x, const fix y)  { fix t;  t.v = x.v + y.v;       return t; }
   inline friend fix operator -  (const fix x, const fix y)  { fix t;  t.v = x.v - y.v;       return t; }
   inline friend fix operator *  (const fix x, const fix y)  { fix t;  t.v = fmul(x.v, y.v);  return t; }
   inline friend fix operator *  (const fix x, const int y)  { fix t;  t.v = x.v * y;         return t; }
   inline friend fix operator *  (const int x, const fix y)  { fix t;  t.v = y.v * x;         return t; }
   inline friend fix operator /  (const fix x, const fix y)  { fix t;  t.v = fdiv(x.v, y.v);  return t; }
   inline friend fix operator /  (const fix x, const int y)  { fix t;  t.v = x.v / y;         return t; }
   inline friend fix operator << (const fix x, const int y)  { fix t;  t.v = x.v << y;        return t; }
   inline friend fix operator >> (const fix x, const int y)  { fix t;  t.v = x.v >> y;        return t; }

   inline friend int operator == (const fix x, const fix y)  { return (x.v == y.v); }
   inline friend int operator != (const fix x, const fix y)  { return (x.v != y.v); }
   inline friend int operator <  (const fix x, const fix y)  { return (x.v < y.v);  }
   inline friend int operator >  (const fix x, const fix y)  { return (x.v > y.v);  }
   inline friend int operator <= (const fix x, const fix y)  { return (x.v <= y.v); }
   inline friend int operator >= (const fix x, const fix y)  { return (x.v >= y.v); }

   inline friend fix sqrt(fix x)          { fix t;  t.v = fsqrt(x.v);  return t; }
   inline friend fix cos(fix x)           { fix t;  t.v = fcos(x.v);   return t; }
   inline friend fix sin(fix x)           { fix t;  t.v = fsin(x.v);   return t; }
   inline friend fix tan(fix x)           { fix t;  t.v = ftan(x.v);   return t; }
   inline friend fix acos(fix x)          { fix t;  t.v = facos(x.v);  return t; }
   inline friend fix asin(fix x)          { fix t;  t.v = fasin(x.v);  return t; }
   inline friend fix atan(fix x)          { fix t;  t.v = fatan(x.v);  return t; }
   inline friend fix atan2(fix x, fix y)  { fix t;  t.v = fatan2(x.v, y.v);  return t; }
};


extern "C" {

#endif   /* ifdef __cplusplus */



/***************************************/
/************ GUI routines  ************/
/***************************************/

/* a GUI object */
typedef struct DIALOG 
{
   int (*proc)(int, struct DIALOG *, int );  /* dialog procedure */
   int x, y, w, h;               /* position and size of the object */
   int fg, bg;                   /* foreground and background colors */
   int key;                      /* keyboard shortcut (ASCII code) */
   int flags;                    /* flags about the object state */
   int d1, d2;                   /* any data the object might require */
   void *dp;                     /* pointer to more object data */
} DIALOG;


#define SEND_MESSAGE(d, msg, c)  (d)->proc(msg, d, c)

/* bits for the flags field */
#define D_EXIT          1        /* object makes the dialog exit */
#define D_SELECTED      2        /* object is selected */
#define D_GOTFOCUS      4        /* object has the input focus */
#define D_GOTMOUSE      8        /* mouse is on top of object */
#define D_HIDDEN        16       /* object is not visible */

/* return values for the dialog procedures */
#define D_O_K           0        /* normal exit status */
#define D_CLOSE         1        /* request to close the dialog */
#define D_REDRAW        2        /* request to redraw the dialog */
#define D_WANTFOCUS     4        /* this object wants the input focus */
#define D_USED_CHAR     8        /* object has used the keypress */

/* messages for the dialog procedures */
#define MSG_START       1        /* start the dialog, initialise */
#define MSG_END         2        /* dialog is finished - cleanup */
#define MSG_DRAW        3        /* draw the object */
#define MSG_CLICK       4        /* mouse click on the object */
#define MSG_DCLICK      5        /* double click on the object */
#define MSG_KEY         6        /* keyboard shortcut */
#define MSG_CHAR        7        /* other keyboard input */
#define MSG_WANTFOCUS   8        /* does object want the input focus? */
#define MSG_GOTFOCUS    9        /* got the input focus */
#define MSG_LOSTFOCUS   10       /* lost the input focus */
#define MSG_GOTMOUSE    11       /* mouse on top of object */
#define MSG_LOSTMOUSE   12       /* mouse moved away from object */
#define MSG_IDLE        13       /* update any background stuff */

/* some dialog procedures */
int d_clear_proc(int msg, DIALOG *d, int c);
int d_box_proc(int msg, DIALOG *d, int c);
int d_shadow_box_proc(int msg, DIALOG *d, int c);
int d_bitmap_proc(int msg, DIALOG *d, int c);
int d_text_proc(int msg, DIALOG *d, int c);
int d_ctext_proc(int msg, DIALOG *d, int c);
int d_button_proc(int msg, DIALOG *d, int c);
int d_check_proc(int msg, DIALOG *d, int c);
int d_edit_proc(int msg, DIALOG *d, int c);
int d_list_proc(int msg, DIALOG *d, int c);

extern int gui_mouse_focus;
extern int gui_fg_color, gui_bg_color;

void centre_dialog(DIALOG *dialog);
void set_dialog_color(DIALOG *dialog, int fg, int bg);
int dialog_message(DIALOG *dialog, int msg, int c, int *obj);
int do_dialog(DIALOG *dialog, int focus_obj);
int popup_dialog(DIALOG *dialog, int focus_obj);
int alert(char *s1, char *s2, char *s3, char *b1, char *b2, int c1, int c2);
int file_select(char *message, char *path, char *ext);
int gfx_mode_select(int *card, int *w, int *h);


#ifdef __cplusplus
}
#endif

#endif          /* ifndef ALLEGRO_H */

