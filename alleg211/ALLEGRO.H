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


#define VERSION_STR     "2.1"
#define DATE_STR        "1996"



/*******************************************/
/************ Some global stuff ************/
/*******************************************/

#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <dpmi.h>
#include <pc.h>

#ifndef TRUE 
#define TRUE         -1
#define FALSE        0
#endif

#ifndef MIN
#define MIN(x,y)     (((x) < (y)) ? (x) : (y))
#define MAX(x,y)     (((x) > (y)) ? (x) : (y))
#define MID(x,y,z)   MAX((x), MIN((y), (z)))
#endif

#ifndef ABS
#define ABS(x)       (((x) >= 0) ? (x) : (-(x)))
#endif

#ifndef SGN
#define SGN(x)       (((x) >= 0) ? 1 : -1)
#endif

#ifndef __INLINE__
#define __INLINE__ extern inline
#endif

typedef long fixed;

struct RGB;
struct BITMAP;
struct RLE_SPRITE;
struct FONT_8x8;
struct SAMPLE;
struct MIDI;

extern char allegro_id[];
extern char allegro_error[];

extern int windows_version, windows_sub_version;

int allegro_init();
void allegro_exit();

void lock_bitmap(struct BITMAP *bmp);
void lock_sample(struct SAMPLE *spl);
void lock_midi(struct MIDI *midi);

#define END_OF_FUNCTION(x)    void x##_end() { }

#define LOCK_VARIABLE(x)      _go32_dpmi_lock_data((void *)&x, sizeof(x))
#define LOCK_FUNCTION(x)      _go32_dpmi_lock_code(x, (long)x##_end - (long)x)



/****************************************/
/************ Mouse routines ************/
/****************************************/

int install_mouse();
void remove_mouse();

extern volatile int mouse_x;
extern volatile int mouse_y;
extern volatile int mouse_b;

void show_mouse(struct BITMAP *bmp);
void position_mouse(int x, int y);
void set_mouse_range(int x1, int y1, int x2, int y2);
void set_mouse_speed(int xspeed, int yspeed);
void set_mouse_sprite(struct BITMAP *sprite);
void set_mouse_sprite_focus(int x, int y);



/****************************************/
/************ Timer routines ************/
/****************************************/

#define TIMERS_PER_SECOND     1193181L
#define SECS_TO_TIMER(x)      ((long)(x) * TIMERS_PER_SECOND)
#define MSEC_TO_TIMER(x)      ((long)(x) * (TIMERS_PER_SECOND / 1000))
#define BPS_TO_TIMER(x)       (TIMERS_PER_SECOND / (long)(x))
#define BPM_TO_TIMER(x)       ((60 * TIMERS_PER_SECOND) / (long)(x))

int install_timer();
void remove_timer();

int install_int_ex(void (*proc)(), long speed);
int install_int(void (*proc)(), long speed);
void remove_int(void (*proc)());

extern volatile int retrace_count;
extern void (*retrace_proc)();

void timer_simulate_retrace(int enable);

void rest(long time);



/*******************************************/
/************ Keyboard routines ************/
/*******************************************/

int install_keyboard();
void remove_keyboard();

extern volatile char key[128];

extern int three_finger_flag;

int keypressed();
int readkey();
void simulate_keypress(int key);
void clear_keybuf();

extern char key_ascii_table[];
extern char key_shift_table[];
extern char key_control_table[];

#define SCANCODE_TO_KEY(c)       (((c)<<8) + key_ascii_table[c])
#define SCANCODE_TO_SHIFT(c)     (((c)<<8) + key_shift_table[c])
#define SCANCODE_TO_CONTROL(c)   (((c)<<8) + key_control_table[c])
#define SCANCODE_TO_ALT(c)       ((c)<<8)

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

extern int joy_type;

/* values for joy_type */
#define JOY_TYPE_STANDARD     0
#define JOY_TYPE_FSPRO        1

/* values for joy_hat */
#define JOY_HAT_CENTRE        0
#define JOY_HAT_LEFT          1
#define JOY_HAT_DOWN          2
#define JOY_HAT_RIGHT         3
#define JOY_HAT_UP            4

/* aliases for FSPro buttons */
#define joy_FSPRO_trigger     joy_b1
#define joy_FSPRO_butleft     joy_b2
#define joy_FSPRO_butright    joy_b3
#define joy_FSPRO_butmiddle   joy_b4

/* joystick status variables */
extern int joy_x, joy_y;
extern int joy_left, joy_right, joy_up, joy_down;
extern int joy_b1, joy_b2;

/* extended variables for the FSPro */
extern int joy_b3, joy_b4;
extern int joy_hat;
extern int joy_throttle;

int initialise_joystick();
int calibrate_joystick_tl();
int calibrate_joystick_br();
int calibrate_joystick_throttle_min();
int calibrate_joystick_throttle_max();

void poll_joystick();

int save_joystick_data(char *filename);
int load_joystick_data(char *filename);



/************************************************/
/************ Screen/bitmap routines ************/
/************************************************/

#define GFX_TEXT              -1
#define GFX_AUTODETECT        0
#define GFX_VGA               1
#define GFX_MODEX             2
#define GFX_VESA1             3
#define GFX_VESA2B            4
#define GFX_VESA2L            5
#define GFX_VBEAF             6
#define GFX_XTENDED           7
#define GFX_ATI               8
#define GFX_MACH64            9
#define GFX_CIRRUS64          10
#define GFX_CIRRUS54          11
#define GFX_S3                12
#define GFX_TRIDENT           13
#define GFX_ET3000            14
#define GFX_ET4000            15
#define GFX_VIDEO7            16


typedef struct GFX_DRIVER        /* creates and manages the screen bitmap */
{
   char *name;                   /* driver name */
   char *desc;                   /* description (VESA version, etc) */
   struct BITMAP *(*init)(int w, int h, int v_w, int v_h);
   void (*exit)(struct BITMAP *b);
   int (*scroll)(int x, int y);
   void (*vsync)();
   void (*set_pallete)(struct RGB *p, int from, int to, int vsync);
   int w, h;                     /* physical (not virtual!) screen size */
   int linear;                   /* true if video memory is linear */
   long bank_size;               /* bank size, in bytes */
   long bank_gran;               /* bank granularity, in bytes */
   long vid_mem;                 /* video memory size, in bytes */
} GFX_DRIVER;


extern GFX_DRIVER gfx_vga, gfx_modex, gfx_vesa_1, gfx_vesa_2b, gfx_vesa_2l, 
		  gfx_vbeaf, gfx_xtended, gfx_ati, gfx_mach64, gfx_cirrus64, 
		  gfx_cirrus54, gfx_realtek, gfx_s3, gfx_trident, gfx_et3000, 
		  gfx_et4000, gfx_video7;

extern GFX_DRIVER *gfx_driver;


#define BMP_TYPE_LINEAR    1     /* memory bitmaps, mode 13h, SVGA */
#define BMP_TYPE_PLANAR    2     /* mode-X bitmaps */


typedef struct GFX_VTABLE        /* functions for drawing onto bitmaps */
{
   int bitmap_type;

   int  (*getpixel)(struct BITMAP *bmp, int x, int y);
   void (*putpixel)(struct BITMAP *bmp, int x, int y, int color);
   void (*vline)(struct BITMAP *bmp, int x, int y1, int y2, int color);
   void (*hline)(struct BITMAP *bmp, int x1, int y, int x2, int color);
   void (*line)(struct BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
   void (*rectfill)(struct BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
   void (*draw_sprite)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
   void (*draw_sprite_v_flip)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
   void (*draw_sprite_h_flip)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
   void (*draw_sprite_vh_flip)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
   void (*rotate_sprite)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y, fixed angle);
   void (*draw_rle_sprite)(struct BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y);
   void (*draw_character)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y, int color);
   void (*textout_8x8)(struct BITMAP *bmp, struct FONT_8x8 *f, char *str, int x, int y, int color);
   void (*blit_from_memory)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
   void (*blit_to_memory)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
   void (*blit_to_self)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
   void (*blit_to_self_forward)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
   void (*blit_to_self_backward)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
   void (*stretch_blit)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height, int masked);
   void (*clear_to_color)(struct BITMAP *bitmap, int color);
} GFX_VTABLE;


typedef struct BITMAP            /* a bitmap structure */
{
   int w, h;                     /* width and height in pixels */
   int clip;                     /* flag if clipping is turned on */
   int cl, cr, ct, cb;           /* clip left, right, top and bottom values */
   GFX_VTABLE *vtable;           /* drawing functions */
   void (*write_bank)();         /* write bank selector, see bank.s */
   void (*read_bank)();          /* read bank selector, see bank.s */
   void *dat;                    /* the memory we allocated for the bitmap */
   int bitmap_id;                /* for identifying sub-bitmaps */
   int line_ofs;                 /* line offset (for screen sub-bitmaps) */
   int seg;                      /* bitmap segment */
   unsigned char *line[0];       /* pointers to the start of each line */
} BITMAP;


extern BITMAP *screen;

#define SCREEN_W     (gfx_driver ? gfx_driver->w : 0)
#define SCREEN_H     (gfx_driver ? gfx_driver->h : 0)

#define VIRTUAL_W    (screen ? screen->w : 0)
#define VIRTUAL_H    (screen ? screen->h : 0)

int set_gfx_mode(int card, int w, int h, int v_w, int v_h);
int scroll_screen(int x, int y);
void request_modex_scroll(int x, int y);
int poll_modex_scroll();
void split_modex_screen(int line);

BITMAP *create_bitmap(int width, int height);
BITMAP *create_sub_bitmap(BITMAP *parent, int x, int y, int width, int height);
void destroy_bitmap(BITMAP *bitmap);


__INLINE__ int is_same_bitmap(BITMAP *bmp1, BITMAP *bmp2)
{
   return ((bmp1 == bmp2) || 
	   ((bmp1->bitmap_id != 0) && (bmp1->bitmap_id == bmp2->bitmap_id)));
}


__INLINE__ int is_linear_bitmap(BITMAP *bmp)
{
   return (bmp->vtable->bitmap_type == BMP_TYPE_LINEAR);
}


__INLINE__ int is_planar_bitmap(BITMAP *bmp)
{
   return (bmp->vtable->bitmap_type == BMP_TYPE_PLANAR);
}


__INLINE__ int is_memory_bitmap(BITMAP *bmp)
{
   return (bmp->dat != NULL);
}


__INLINE__ int is_screen_bitmap(BITMAP *bmp)
{
   return is_same_bitmap(bmp, screen);
}


__INLINE__ int is_sub_bitmap(BITMAP *bmp)
{
   return ((bmp->dat == NULL) && (bmp != screen));
}



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
extern PALLETE black_pallete, desktop_pallete, _current_pallete;

void set_color(int index, RGB *p);
void set_pallete(PALLETE p);
void set_pallete_range(PALLETE p, int from, int to, int vsync);

void get_color(int index, RGB *p);
void get_pallete(PALLETE p);
void get_pallete_range(PALLETE p, int from, int to);

void fade_interpolate(PALLETE source, PALLETE dest, PALLETE output, int pos, int from, int to);
void fade_from_range(PALLETE source, PALLETE dest, int speed, int from, int to);
void fade_in_range(PALLETE p, int speed, int from, int to);
void fade_out_range(int speed, int from, int to);
void fade_from(PALLETE source, PALLETE dest, int speed);
void fade_in(PALLETE p, int speed);
void fade_out(int speed);


__INLINE__ void vsync()
{
   gfx_driver->vsync();
}


__INLINE__ void _set_color(int index, RGB *p)
{
   outportb(0x3C8, index);
   outportb(0x3C9, p->r);
   outportb(0x3C9, p->g);
   outportb(0x3C9, p->b);

   _current_pallete[index] = *p;
}


/* in case you want to spell 'pallete' as 'palette' */
#define PALETTE            PALLETE
#define black_palette      black_pallete
#define desktop_palette    desktop_pallete
#define set_palette        set_pallete
#define get_palette        get_pallete
#define set_palette_range  set_pallete_range
#define get_palette_range  get_pallete_range



/******************************************************/
/************ Graphics and sprite routines ************/
/******************************************************/

void set_clip(BITMAP *bitmap, int x1, int y1, int x2, int y2);

#define DRAW_MODE_SOLID             0        /* flags for drawing_mode() */
#define DRAW_MODE_XOR               1
#define DRAW_MODE_COPY_PATTERN      2
#define DRAW_MODE_SOLID_PATTERN     3
#define DRAW_MODE_MASKED_PATTERN    4

void drawing_mode(int mode, BITMAP *pattern, int x_anchor, int y_anchor);
void xor_mode(int xor);
void solid_mode();


__INLINE__ int getpixel(BITMAP *bmp, int x, int y) 
{ 
   return bmp->vtable->getpixel(bmp, x, y);
}


__INLINE__ void putpixel(BITMAP *bmp, int x, int y, int color) 
{ 
   bmp->vtable->putpixel(bmp, x, y, color);
}


__INLINE__ void vline(BITMAP *bmp, int x, int y1, int y2, int color) 
{ 
   bmp->vtable->vline(bmp, x, y1, y2, color); 
}


__INLINE__ void hline(BITMAP *bmp, int x1, int y, int x2, int color) 
{ 
   bmp->vtable->hline(bmp, x1, y, x2, color); 
}


__INLINE__ void line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   bmp->vtable->line(bmp, x1, y1, x2, y2, color);
}


__INLINE__ void rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   bmp->vtable->rectfill(bmp, x1, y1, x2, y2, color);
}


__INLINE__ void draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y) 
{ 
   bmp->vtable->draw_sprite(bmp, sprite, x, y); 
}


__INLINE__ void draw_sprite_v_flip(BITMAP *bmp, BITMAP *sprite, int x, int y) 
{ 
   bmp->vtable->draw_sprite_v_flip(bmp, sprite, x, y); 
}


__INLINE__ void draw_sprite_h_flip(BITMAP *bmp, BITMAP *sprite, int x, int y) 
{ 
   bmp->vtable->draw_sprite_h_flip(bmp, sprite, x, y); 
}


__INLINE__ void draw_sprite_vh_flip(BITMAP *bmp, BITMAP *sprite, int x, int y) 
{ 
   bmp->vtable->draw_sprite_vh_flip(bmp, sprite, x, y); 
}


__INLINE__ void rotate_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, fixed angle) 
{ 
   bmp->vtable->rotate_sprite(bmp, sprite, x, y, angle); 
}


__INLINE__ void draw_rle_sprite(BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y)
{
   bmp->vtable->draw_rle_sprite(bmp, sprite, x, y);
}


__INLINE__ void stretch_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, int w, int h) 
{ 
   bmp->vtable->stretch_blit(sprite, bmp, 0, 0, sprite->w, sprite->h, x, y, w, h, TRUE); 
}


__INLINE__ void stretch_blit(BITMAP *s, BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h) 
{ 
   d->vtable->stretch_blit(s, d, s_x, s_y, s_w, s_h, d_x, d_y, d_w, d_h, FALSE); 
}


__INLINE__ void clear_to_color(BITMAP *bitmap, int color) 
{ 
   bitmap->vtable->clear_to_color(bitmap, color); 
}


void do_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int d, void (*proc)(BITMAP *, int, int, int));
void triangle(BITMAP *bmp, int x1, int y1, int x2, int y2, int x3, int y3, int color);
void polygon(BITMAP *bmp, int vertices, int *points, int color);
void rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void do_circle(BITMAP *bmp, int x, int y, int radius, int d, void (*proc)(BITMAP *, int, int, int));
void circle(BITMAP *bmp, int x, int y, int radius, int color);
void circlefill(BITMAP *bmp, int x, int y, int radius, int color);
void floodfill(BITMAP *bmp, int x, int y, int color);
void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void clear(BITMAP *bitmap);


typedef struct RLE_SPRITE           /* a RLE compressed sprite */
{
   int w, h;                        /* width and height in pixels */
   int size;                        /* size of sprite data in bytes */
   signed char dat[0];              /* RLE bitmap data */
} RLE_SPRITE;


RLE_SPRITE *get_rle_sprite(BITMAP *bitmap);
void destroy_rle_sprite(RLE_SPRITE *sprite);


typedef struct COMPILED_SPRITE      /* a compiled sprite */
{
   int planar;                      /* set if it's a planar (mode-X) sprite */
   void *draw[4];                   /* routines to draw the image */
} COMPILED_SPRITE;


COMPILED_SPRITE *get_compiled_sprite(BITMAP *bitmap, int planar);
void destroy_compiled_sprite(COMPILED_SPRITE *sprite);
void draw_compiled_sprite(BITMAP *bmp, COMPILED_SPRITE *sprite, int x, int y);


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
int text_height(FONT *f);
void destroy_font(FONT *f);



/*********************************************/
/************ Video memory access ************/
/*********************************************/

__INLINE__ unsigned long bmp_write_line(BITMAP *bmp, int line)
{
   unsigned long result;

   asm volatile (
      "  call *%3 "

   : "=a" (result)                     /* result in eax */

   : "d" (bmp),                        /* bitmap in edx */
     "0" (line),                       /* line number in eax */
     "r" (bmp->write_bank)             /* the bank switch routine */
   );

   return result;
}


__INLINE__ unsigned long bmp_read_line(BITMAP *bmp, int line)
{
   unsigned long result;

   asm volatile (
      "  call *%3 "

   : "=a" (result)                     /* result in eax */

   : "d" (bmp),                        /* bitmap in edx */
     "0" (line),                       /* line number in eax */
     "r" (bmp->read_bank)              /* the bank switch routine */
   );

   return result;
}


__INLINE__ void _putpixel(BITMAP *bmp, int x, int y, unsigned char color)
{
   asm (
      "  movw %w0, %%fs ; "
      "  .byte 0x64 ; "
      "  movb %b3, (%1, %2) "
   :                                   /* no outputs */

   : "rm" (bmp->seg),                  /* segment selector in reg or mem */
     "r" (bmp_write_line(bmp, y)),     /* line pointer in reg */
     "r" (x),                          /* line offset in reg */
     "qi" (color)                      /* the pixel in reg or immediate */
   );
}


__INLINE__ unsigned char _getpixel(BITMAP *bmp, int x, int y)
{
   unsigned char result;

   asm (
      "  movw %w1, %%fs ; "
      "  .byte 0x64 ; "
      "  movb (%2, %3), %b0"

   : "=&q" (result)                    /* result in al, bl, cl, or dl */

   : "rm" (bmp->seg),                  /* segment selector in reg or mem */
     "r" (bmp_read_line(bmp, y)),      /* line pointer in reg */
     "r" (x)                           /* line offset in reg */
   );

   return result;
}



/******************************************/
/************ FLI/FLC routines ************/
/******************************************/

#define FLI_OK          0              /* FLI player return values */
#define FLI_EOF         -1
#define FLI_ERROR       -2
#define FLI_NOT_OPEN    -3

int play_fli(char *filename, BITMAP *bmp, int loop, int (*callback)());
int play_memory_fli(void *fli_data, BITMAP *bmp, int loop, int (*callback)());

int open_fli(char *filename);
int open_memory_fli(void *fli_data);
void close_fli();
int next_fli_frame(int loop);
void reset_fli_variables();

extern BITMAP *fli_bitmap;             /* current frame of the FLI */
extern PALLETE fli_pallete;            /* current FLI pallete */

extern int fli_bmp_dirty_from;         /* what part of fli_bitmap is dirty */
extern int fli_bmp_dirty_to;
extern int fli_pal_dirty_from;         /* what part of fli_pallete is dirty */
extern int fli_pal_dirty_to;

extern int fli_frame;                  /* current frame number */

extern volatile int fli_timer;         /* for timing FLI playback */



/****************************************/
/************ Sound routines ************/
/****************************************/

#define DIGI_VOICES           16       /* Theoretical maximums */
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
   int (*mixer_volume)(int volume);
} DIGI_DRIVER;


typedef struct MIDI_DRIVER             /* driver for playing midi music */
{
   char *name;                         /* driver name */
   char *desc;                         /* description (ports, irq, etc) */
   int voices;                         /* max simultaneous voices */
   int (*detect)();
   int (*init)();
   void (*exit)();
   int (*load_patches)(char *patches, char *drums);
   void (*key_on)(int voice, int inst, int note, int bend, int vol, int pan);
   void (*key_off)(int voice);
   void (*set_volume)(int voice, int vol);
   void (*set_pitch)(int voice, int note, int bend);
   int (*mixer_volume)(int volume);
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
void set_volume(int digi_volume, int midi_volume);

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

#define pack_feof(f)       f->eof
#define pack_ferror(f)     f->error


__INLINE__ int pack_getc(PACKFILE *f)
{
   f->buf_size--;
   if (f->buf_size > 0)
      return *(f->buf_pos++);
   else
      return _sort_out_getc(f);
}


__INLINE__ int pack_putc(int c, PACKFILE *f)
{
   f->buf_size++;
   if (f->buf_size >= F_BUF_SIZE)
      return _sort_out_putc(c, f);
   else
      return (*(f->buf_pos++) = c);
}



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
#define DAT_FLI            15
#define DAT_C_SPRITE       16
#define DAT_XC_SPRITE      17


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

__INLINE__ fixed itofix(int x) 
{ 
   return x << 16;
}


__INLINE__ int fixtoi(fixed x) 
{ 
   return (x >> 16) + ((x & 0x8000) >> 15);
}


__INLINE__ fixed ftofix(double x) 
{ 
   if (x > 32767) {
      errno = ERANGE;
      return 0x7FFFFFFF;
   }

   if (x < -32768) {
      errno = ERANGE;
      return -0x80000000; 
   }

   return (long)(x * 65536 + (x < 0 ? -0.5 : 0.5)); 
}


__INLINE__ double fixtof(fixed x) 
{ 
   return (double)x / 65536; 
}


fixed fsqrt(fixed x);
fixed fatan(fixed x);
fixed fatan2(fixed y, fixed x);

extern fixed _cos_tbl[];
extern fixed _tan_tbl[];
extern fixed _acos_tbl[];


__INLINE__ fixed fcos(fixed x)
{
   return _cos_tbl[((x & 0x4000) ? (x >> 15) + 1 : (x >> 15)) & 0x1ff];
}


__INLINE__ fixed fsin(fixed x) 
{ 
   return _cos_tbl[(((x & 0x4000) ? (x >> 15) + 1 : (x >> 15)) -128) & 0x1ff];
}


__INLINE__ fixed ftan(fixed x) 
{ 
   return _tan_tbl[((x & 0x4000) ? (x >> 15) + 1 : (x >> 15)) & 0xff];
}


__INLINE__ fixed facos(fixed x)
{
   if ((x < -65536L) || (x > 65536L)) {
      errno = EDOM;
      return 0L;
   }

   return _acos_tbl[(x+65536L)>>8];
}


__INLINE__ fixed fasin(fixed x) 
{ 
   if ((x < -65536L) || (x > 65536L)) {
      errno = EDOM;
      return 0L;
   }

   return 0x00400000L - _acos_tbl[(x+65536L)>>8];
}


__INLINE__ fixed fmul(fixed x, fixed y)
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

   : "%edx", "%cc"                        /* clobbers edx and flags */
   );

   return result;
}


__INLINE__ fixed fdiv(fixed x, fixed y)
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

   : "%ebx", "%ecx", "%edx", "%cc"        /* clobbers ebx, ecx, edx + flags */
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


/* a popup menu */
typedef struct MENU
{
   char *text;                   /* menu item text */
   int (*proc)();                /* callback function */
   struct MENU *child;           /* to allow nested menus */
} MENU;


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
#define MSG_XCHAR       8        /* broadcast character to all objects */
#define MSG_WANTFOCUS   9        /* does object want the input focus? */
#define MSG_GOTFOCUS    10       /* got the input focus */
#define MSG_LOSTFOCUS   11       /* lost the input focus */
#define MSG_GOTMOUSE    12       /* mouse on top of object */
#define MSG_LOSTMOUSE   13       /* mouse moved away from object */
#define MSG_IDLE        14       /* update any background stuff */

/* some dialog procedures */
int d_clear_proc(int msg, DIALOG *d, int c);
int d_box_proc(int msg, DIALOG *d, int c);
int d_shadow_box_proc(int msg, DIALOG *d, int c);
int d_bitmap_proc(int msg, DIALOG *d, int c);
int d_text_proc(int msg, DIALOG *d, int c);
int d_ctext_proc(int msg, DIALOG *d, int c);
int d_button_proc(int msg, DIALOG *d, int c);
int d_check_proc(int msg, DIALOG *d, int c);
int d_keyboard_proc(int msg, DIALOG *d, int c);
int d_edit_proc(int msg, DIALOG *d, int c);
int d_list_proc(int msg, DIALOG *d, int c);
int d_menu_proc(int msg, DIALOG *d, int c);

extern int gui_mouse_focus;
extern int gui_fg_color, gui_bg_color;

void centre_dialog(DIALOG *dialog);
void set_dialog_color(DIALOG *dialog, int fg, int bg);
int find_dialog_focus(DIALOG *dialog);
int dialog_message(DIALOG *dialog, int msg, int c, int *obj);
int do_dialog(DIALOG *dialog, int focus_obj);
int popup_dialog(DIALOG *dialog, int focus_obj);
int do_menu(MENU *menu, int x, int y);
int alert(char *s1, char *s2, char *s3, char *b1, char *b2, int c1, int c2);
int alert3(char *s1, char *s2, char *s3, char *b1, char *b2, char *b3, int c1, int c2, int c3);
int file_select(char *message, char *path, char *ext);
int gfx_mode_select(int *card, int *w, int *h);


#ifdef __cplusplus
}
#endif

#endif          /* ifndef ALLEGRO_H */

