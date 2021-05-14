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


#ifndef ALLEGRO_H
#define ALLEGRO_H

#ifndef DJGPP
#ifndef LINUX
#error Allegro will only compile with djgpp or linux
#else
#error Linux version not finished. Want to help?
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ALLEGRO_VERSION          2
#define ALLEGRO_SUB_VERSION      2
#define ALLEGRO_VERSION_STR      "2.2"
#define ALLEGRO_DATE_STR         "1997"



/*******************************************/
/************ Some global stuff ************/
/*******************************************/

#include <stdlib.h>
#include <stddef.h>
#include <errno.h>

#ifdef DJGPP 
#include <dpmi.h>
#include <pc.h>
#endif 

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

#ifdef DJGPP 

/* for djgpp */
#define END_OF_FUNCTION(x)    void x##_end() { }
#define LOCK_VARIABLE(x)      _go32_dpmi_lock_data((void *)&x, sizeof(x))
#define LOCK_FUNCTION(x)      _go32_dpmi_lock_code(x, (long)x##_end - (long)x)

#else 

/* for linux */
#define END_OF_FUNCTION(x)
#define LOCK_VARIABLE(x)
#define LOCK_FUNCTION(x)

#endif 



/****************************************/
/************ Mouse routines ************/
/****************************************/

int install_mouse();
void remove_mouse();

extern volatile int mouse_x;
extern volatile int mouse_y;
extern volatile int mouse_b;

extern int freeze_mouse_flag;

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

extern int i_love_bill;

extern volatile int retrace_count;
extern void (*retrace_proc)();

void timer_simulate_retrace(int enable);

void rest(long time);



/*******************************************/
/************ Keyboard routines ************/
/*******************************************/

int install_keyboard();
void remove_keyboard();

void install_keyboard_hooks(int (*keypressed)(), int (*readkey)());

extern volatile char key[128];
extern volatile int key_shifts;

extern int three_finger_flag;
extern int key_led_flag;

int keypressed();
int readkey();
void simulate_keypress(int key);
void clear_keybuf();

extern char key_ascii_table[128];
extern char key_capslock_table[128];
extern char key_shift_table[128];
extern char key_control_table[128];
extern char key_numlock_table[128];
extern char key_extended_table[128];
extern char key_special_table[128];

#define SCANCODE_TO_KEY(c)       (((c)<<8) + key_ascii_table[c])
#define SCANCODE_TO_CAPS(c)      (((c)<<8) + key_capslock_table[c])
#define SCANCODE_TO_SHIFT(c)     (((c)<<8) + key_shift_table[c])
#define SCANCODE_TO_CONTROL(c)   (((c)<<8) + key_control_table[c])
#define SCANCODE_TO_ALT(c)       ((c)<<8)

#define KB_SHIFT_FLAG         1
#define KB_CTRL_FLAG          2
#define KB_ALT_FLAG           4
#define KB_SCROLOCK_FLAG      8
#define KB_NUMLOCK_FLAG       16
#define KB_CAPSLOCK_FLAG      32
#define KB_INALTSEQ_FLAG      64

#define KEY_ESC               1     /* keyboard scan codes  */
#define KEY_1                 2 
#define KEY_2                 3 
#define KEY_3                 4
#define KEY_4                 5
#define KEY_5                 6
#define KEY_6                 7
#define KEY_7                 8
#define KEY_8                 9
#define KEY_9                 10
#define KEY_0                 11
#define KEY_MINUS             12
#define KEY_EQUALS            13
#define KEY_BACKSPACE         14
#define KEY_TAB               15 
#define KEY_Q                 16
#define KEY_W                 17
#define KEY_E                 18
#define KEY_R                 19
#define KEY_T                 20
#define KEY_Y                 21
#define KEY_U                 22
#define KEY_I                 23
#define KEY_O                 24
#define KEY_P                 25
#define KEY_OPENBRACE         26
#define KEY_CLOSEBRACE        27
#define KEY_ENTER             28
#define KEY_CONTROL           29
#define KEY_A                 30
#define KEY_S                 31
#define KEY_D                 32
#define KEY_F                 33
#define KEY_G                 34
#define KEY_H                 35
#define KEY_J                 36
#define KEY_K                 37
#define KEY_L                 38
#define KEY_COLON             39
#define KEY_QUOTE             40
#define KEY_TILDE             41
#define KEY_LSHIFT            42
#define KEY_BACKSLASH         43
#define KEY_Z                 44
#define KEY_X                 45
#define KEY_C                 46
#define KEY_V                 47
#define KEY_B                 48
#define KEY_N                 49
#define KEY_M                 50
#define KEY_COMMA             51
#define KEY_STOP              52
#define KEY_SLASH             53
#define KEY_RSHIFT            54
#define KEY_ASTERISK          55
#define KEY_ALT               56
#define KEY_SPACE             57
#define KEY_CAPSLOCK          58
#define KEY_F1                59
#define KEY_F2                60
#define KEY_F3                61
#define KEY_F4                62
#define KEY_F5                63
#define KEY_F6                64
#define KEY_F7                65
#define KEY_F8                66
#define KEY_F9                67
#define KEY_F10               68
#define KEY_NUMLOCK           69
#define KEY_SCRLOCK           70
#define KEY_HOME              71
#define KEY_UP                72
#define KEY_PGUP              73
#define KEY_MINUS_PAD         74
#define KEY_LEFT              75
#define KEY_5_PAD             76
#define KEY_RIGHT             77
#define KEY_PLUS_PAD          78
#define KEY_END               79
#define KEY_DOWN              80
#define KEY_PGDN              81
#define KEY_INSERT            82
#define KEY_DEL               83
#define KEY_F11               87
#define KEY_F12               88
#define KEY_PAD               100



/*******************************************/
/************ Joystick routines ************/
/*******************************************/

extern int joy_type;

/* values for joy_type */
#define JOY_TYPE_STANDARD     0
#define JOY_TYPE_FSPRO        1
#define JOY_TYPE_4BUTTON      2

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

#ifdef DJGPP 

/* for djgpp */
#define GFX_VESA1             3
#define GFX_VESA2B            4
#define GFX_VESA2L            5
#define GFX_VBEAF             6
#define GFX_XTENDED           7
#define GFX_ATI               8
#define GFX_MACH64            9
#define GFX_CIRRUS64          10
#define GFX_CIRRUS54          11
#define GFX_PARADISE          12
#define GFX_S3                13
#define GFX_TRIDENT           14
#define GFX_ET3000            15
#define GFX_ET4000            16
#define GFX_VIDEO7            17

#else 

/* for linux */
#define GFX_SVGALIB           3

#endif 


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
   long vid_phys_base;           /* physical address of video memory */
} GFX_DRIVER;


extern GFX_DRIVER gfx_vga, gfx_modex;

#ifdef DJGPP 

/* for djgpp */
extern GFX_DRIVER gfx_vesa_1, gfx_vesa_2b, gfx_vesa_2l, gfx_vbeaf, 
		  gfx_xtended, gfx_ati, gfx_mach64, gfx_cirrus64, 
		  gfx_cirrus54, gfx_realtek, gfx_s3, gfx_trident, 
		  gfx_et3000, gfx_et4000, gfx_paradise, gfx_video7;

#else 

/* for linux */
extern GFX_DRIVER gfx_svgalib;

#endif 

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
   void (*draw_trans_sprite)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
   void (*draw_lit_sprite)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y, int color);
   void (*draw_rle_sprite)(struct BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y);
   void (*draw_trans_rle_sprite)(struct BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y);
   void (*draw_lit_rle_sprite)(struct BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y, int color);
   void (*draw_character)(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y, int color);
   void (*textout_fixed)(struct BITMAP *bmp, void *f, int h, unsigned char *str, int x, int y, int color);
   void (*blit_from_memory)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
   void (*blit_to_memory)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
   void (*blit_to_self)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
   void (*blit_to_self_forward)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
   void (*blit_to_self_backward)(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
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

void set_color_depth(int depth);
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



/************************************************/
/************ Color/Pallete routines ************/
/************************************************/

typedef struct RGB
{
   unsigned char r, g, b;
   unsigned char filler;
} RGB;

#define PAL_SIZE     256

typedef RGB PALLETE[PAL_SIZE];

extern RGB black_rgb;
extern PALLETE black_pallete, desktop_pallete, _current_pallete;

typedef struct {
   unsigned char data[32][32][32];
} RGB_MAP;

typedef struct {
   unsigned char data[PAL_SIZE][PAL_SIZE];
} COLOR_MAP;

extern RGB_MAP *rgb_map;
extern COLOR_MAP *color_map;

extern int _color_depth;

extern unsigned char _rgb_r_shift_15, _rgb_g_shift_15, _rgb_b_shift_15,
		     _rgb_r_shift_16, _rgb_g_shift_16, _rgb_b_shift_16,
		     _rgb_r_shift_32, _rgb_g_shift_32, _rgb_b_shift_32;

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

void create_rgb_table(RGB_MAP *table, PALLETE pal, void (*callback)(int pos));
void create_light_table(COLOR_MAP *table, PALLETE pal, int r, int g, int b, void (*callback)(int pos));
void create_trans_table(COLOR_MAP *table, PALLETE pal, int r, int g, int b, void (*callback)(int pos));
void create_color_table(COLOR_MAP *table, PALLETE pal, RGB (*blend)(PALLETE pal, int x, int y), void (*callback)(int pos));

void hsv_to_rgb(float h, float s, float v, RGB *rgb);
void rgb_to_hsv(RGB *rgb, float *h, float *s, float *v);

int makecol(int r, int g, int b);
int makecol8(int r, int g, int b);

int getr(int c);
int getg(int c);
int getb(int c);


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


__INLINE__ int makecol15(int r, int g, int b)
{
   return (((r >> 3) << _rgb_r_shift_15) |
	   ((g >> 3) << _rgb_g_shift_15) |
	   ((b >> 3) << _rgb_b_shift_15));
}


__INLINE__ int makecol16(int r, int g, int b)
{
   return (((r >> 3) << _rgb_r_shift_16) |
	   ((g >> 2) << _rgb_g_shift_16) |
	   ((b >> 3) << _rgb_b_shift_16));
}


__INLINE__ int makecol32(int r, int g, int b)
{
   return ((r << _rgb_r_shift_32) |
	   (g << _rgb_g_shift_32) |
	   (b << _rgb_b_shift_32));
}


__INLINE__ int getr8(int c)
{
   return ((int)_current_pallete[c].r * 255 / 63);
}


__INLINE__ int getg8(int c)
{
   return ((int)_current_pallete[c].g * 255 / 63);
}


__INLINE__ int getb8(int c)
{
   return ((int)_current_pallete[c].b * 255 / 63);
}


__INLINE__ int getr15(int c)
{
   return (((c >> _rgb_r_shift_15) & 0x1F) * 255 / 31);
}


__INLINE__ int getg15(int c)
{
   return (((c >> _rgb_g_shift_15) & 0x1F) * 255 / 31);
}


__INLINE__ int getb15(int c)
{
   return (((c >> _rgb_b_shift_15) & 0x1F) * 255 / 31);
}


__INLINE__ int getr16(int c)
{
   return (((c >> _rgb_r_shift_16) & 0x1F) * 255 / 31);
}


__INLINE__ int getg16(int c)
{
   return (((c >> _rgb_g_shift_16) & 0x3F) * 255 / 63);
}


__INLINE__ int getb16(int c)
{
   return (((c >> _rgb_b_shift_16) & 0x1F) * 255 / 31);
}


__INLINE__ int getr32(int c)
{
   return ((c >> _rgb_r_shift_32) & 0xFF);
}


__INLINE__ int getg32(int c)
{
   return ((c >> _rgb_g_shift_32) & 0xFF);
}


__INLINE__ int getb32(int c)
{
   return ((c >> _rgb_b_shift_32) & 0xFF);
}


/* in case you want to spell 'pallete' as 'palette' */
#define PALETTE            PALLETE
#define black_palette      black_pallete
#define desktop_palette    desktop_pallete
#define set_palette        set_pallete
#define get_palette        get_pallete
#define set_palette_range  set_pallete_range
#define get_palette_range  get_pallete_range
#define fli_palette        fli_pallete
#define DAT_PALETTE        DAT_PALLETE



/******************************************************/
/************ Graphics and sprite routines ************/
/******************************************************/

void set_clip(BITMAP *bitmap, int x1, int y1, int x2, int y2);

#define DRAW_MODE_SOLID             0        /* flags for drawing_mode() */
#define DRAW_MODE_XOR               1
#define DRAW_MODE_COPY_PATTERN      2
#define DRAW_MODE_SOLID_PATTERN     3
#define DRAW_MODE_MASKED_PATTERN    4
#define DRAW_MODE_TRANS             5

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


__INLINE__ void draw_trans_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y) 
{ 
   bmp->vtable->draw_trans_sprite(bmp, sprite, x, y); 
}


__INLINE__ void draw_lit_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, int color) 
{ 
   bmp->vtable->draw_lit_sprite(bmp, sprite, x, y, color); 
}


__INLINE__ void draw_rle_sprite(BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y)
{
   bmp->vtable->draw_rle_sprite(bmp, sprite, x, y);
}


__INLINE__ void draw_trans_rle_sprite(BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y)
{
   bmp->vtable->draw_trans_rle_sprite(bmp, sprite, x, y);
}


__INLINE__ void draw_lit_rle_sprite(BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y, int color)
{
   bmp->vtable->draw_lit_rle_sprite(bmp, sprite, x, y, color);
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
void calc_spline(int points[8], int npts, int *x, int *y);
void spline(BITMAP *bmp, int points[8], int color);
void floodfill(BITMAP *bmp, int x, int y, int color);
void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void stretch_blit(BITMAP *s, BITMAP *d, int s_x, int s_y, int s_w, int s_h, int d_x, int d_y, int d_w, int d_h);
void stretch_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, int w, int h);
void rotate_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, fixed angle);
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
   struct {
      void *draw;                   /* routines to draw the image */
      int len;                      /* length of the drawing functions */
   } proc[4];
} COMPILED_SPRITE;


COMPILED_SPRITE *get_compiled_sprite(BITMAP *bitmap, int planar);
void destroy_compiled_sprite(COMPILED_SPRITE *sprite);
void draw_compiled_sprite(BITMAP *bmp, COMPILED_SPRITE *sprite, int x, int y);


#define FONT_SIZE    224            /* number of characters in a font */


typedef struct FONT_8x8             /* a simple 8x8 font */
{
   unsigned char dat[FONT_SIZE][8];
} FONT_8x8;


typedef struct FONT_8x16            /* a simple 8x16 font */
{
   unsigned char dat[FONT_SIZE][16];
} FONT_8x16;


typedef struct FONT_PROP            /* a proportional font */
{
   BITMAP *dat[FONT_SIZE]; 
} FONT_PROP;


typedef struct FONT                 /* can be either */
{
   int height;
   union {
      FONT_8x8 *dat_8x8;
      FONT_8x16 *dat_8x16;
      FONT_PROP *dat_prop;
   } dat;
} FONT;


extern FONT *font;

void text_mode(int mode);
void textout(BITMAP *bmp, FONT *f, unsigned char *str, int x, int y, int color);
void textout_centre(BITMAP *bmp, FONT *f, unsigned char *str, int x, int y, int color);
int text_length(FONT *f, unsigned char *str);
int text_height(FONT *f);
void destroy_font(FONT *f);


#ifdef __cplusplus

}  /* end of extern "C" */


__INLINE__ void textout(BITMAP *bmp, FONT *f, char *str, int x, int y, int color)
{
   textout(bmp, f, (unsigned char *)str, x, y, color);
}


__INLINE__ void textout_centre(BITMAP *bmp, FONT *f, char *str, int x, int y, int color)
{
   textout_centre(bmp, f, (unsigned char *)str, x, y, color);
}


__INLINE__ int text_length(FONT *f, char *str)
{
   return text_length(f, (unsigned char *)str);
}


extern "C" {

#endif   /* ifdef __cplusplus */


typedef struct V3D                  /* a 3d point (fixed point version) */
{
   fixed x, y, z;                   /* position */
   fixed u, v;                      /* texture map coordinates */
   int c;                           /* color */
} V3D;


typedef struct V3D_f                /* a 3d point (floating point version) */
{
   float x, y, z;                   /* position */
   float u, v;                      /* texture map coordinates */
   int c;                           /* color */
} V3D_f;


#define POLYTYPE_FLAT               0
#define POLYTYPE_GCOL               1
#define POLYTYPE_GRGB               2
#define POLYTYPE_ATEX               3
#define POLYTYPE_PTEX               4
#define POLYTYPE_ATEX_MASK          5
#define POLYTYPE_PTEX_MASK          6
#define POLYTYPE_ATEX_LIT           7
#define POLYTYPE_PTEX_LIT           8


void polygon3d(BITMAP *bmp, int type, BITMAP *texture, int vc, V3D *vtx[]);
void polygon3d_f(BITMAP *bmp, int type, BITMAP *texture, int vc, V3D_f *vtx[]);
void triangle3d(BITMAP *bmp, int type, BITMAP *texture, V3D *v1, V3D *v2, V3D *v3);
void triangle3d_f(BITMAP *bmp, int type, BITMAP *texture, V3D_f *v1, V3D_f *v2, V3D_f *v3);
void quad3d(BITMAP *bmp, int type, BITMAP *texture, V3D *v1, V3D *v2, V3D *v3, V3D *v4);
void quad3d_f(BITMAP *bmp, int type, BITMAP *texture, V3D_f *v1, V3D_f *v2, V3D_f *v3, V3D_f *v4);



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

#define DIGI_VOICES           16       /* Theoretical maximums: */
#define MIDI_TRACKS           32       /* actual drivers may not be */ 
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

#ifdef DJGPP 

/* for djgpp */
#define DIGI_SB               1
#define DIGI_SB10             2 
#define DIGI_SB15             3 
#define DIGI_SB20             4 
#define DIGI_SBPRO            5 
#define DIGI_SB16             6 
#define DIGI_GUS              7

#else 

/* for linux */
#define DIGI_LINUX_SOUND_NOT_IMPLEMENTED_YET

#endif 

#define MIDI_AUTODETECT       -1 
#define MIDI_NONE             0 

#ifdef DJGPP 

/* for djgpp */
#define MIDI_ADLIB            1 
#define MIDI_OPL2             2 
#define MIDI_2XOPL2           3 
#define MIDI_OPL3             4
#define MIDI_SB_OUT           5
#define MIDI_MPU              6 
#define MIDI_GUS              7
#define MIDI_DIGMID           8

#else 

/* for linux */
#define MIDI_LINUX_SOUND_NOT_IMPLEMENTED_YET

#endif 


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


extern DIGI_DRIVER digi_none;

#ifdef DJGPP
/* for djgpp */
extern DIGI_DRIVER digi_sb, digi_gus;
#else
/* for linux */
extern DIGI_DRIVER digi_linux_sound_not_implemented_yet;
#endif

extern MIDI_DRIVER midi_none;

#ifdef DJGPP
/* for djgpp */
extern MIDI_DRIVER midi_adlib, midi_sb_out, midi_mpu401, midi_gus, midi_digmid;
#else
/* for linux */
extern MIDI_DRIVER midi_linux_sound_not_implemented_yet;
#endif

extern DIGI_DRIVER *digi_driver;       /* the drivers currently in use */
extern MIDI_DRIVER *midi_driver;

extern int digi_card, midi_card;

extern volatile long midi_pos;         /* current position in the midi file */

int install_sound(int digi, int midi, char *cfg_path); 
void remove_sound();
void set_volume(int digi_volume, int midi_volume);

SAMPLE *load_sample(char *filename);
SAMPLE *load_wav(char *filename);
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
int exists(char *filename);
long file_size(char *filename);
long file_time(char *filename);
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

#define PACKFILE_FLAG_WRITE   1        /* the file is being written */
#define PACKFILE_FLAG_PACK    2        /* data is compressed */
#define PACKFILE_FLAG_CHUNK   4        /* file is a sub-chunk */
#define PACKFILE_FLAG_EOF     8        /* reached the end-of-file */
#define PACKFILE_FLAG_ERROR   16       /* an error has occurred */


typedef struct PACKFILE                /* our very own FILE structure... */
{
   int hndl;                           /* DOS file handle */
   int flags;                          /* PACKFILE_FLAG_* constants */
   unsigned char *buf_pos;             /* position in buffer */
   int buf_size;                       /* number of bytes in the buffer */
   long todo;                          /* number of bytes still on the disk */
   struct PACKFILE *parent;            /* nested, parent file */
   void *pack_data;                    /* for LZSS compression */
   char *filename;                     /* name of the file */
   unsigned char buf[F_BUF_SIZE];      /* the actual data buffer */
} PACKFILE;


PACKFILE *pack_fopen(char *filename, char *mode);
int pack_fclose(PACKFILE *f);
int pack_fseek(PACKFILE *f, int offset);
PACKFILE *pack_fopen_chunk(PACKFILE *f, int pack);
PACKFILE *pack_fclose_chunk(PACKFILE *f);
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

#define pack_feof(f)       ((f)->flags & PACKFILE_FLAG_EOF)
#define pack_ferror(f)     ((f)->flags & PACKFILE_FLAG_ERROR)


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



/*******************************************/
/************ Datafile routines ************/
/*******************************************/

#define DAT_ID(a,b,c,d)    ((a<<24) | (b<<16) | (c<<8) | d)

#define DAT_MAGIC          DAT_ID('A','L','L','.')
#define DAT_FILE           DAT_ID('F','I','L','E')
#define DAT_DATA           DAT_ID('D','A','T','A')
#define DAT_FONT           DAT_ID('F','O','N','T')
#define DAT_SAMPLE         DAT_ID('S','A','M','P')
#define DAT_MIDI           DAT_ID('M','I','D','I')
#define DAT_FLI            DAT_ID('F','L','I','C')
#define DAT_BITMAP         DAT_ID('B','M','P',' ')
#define DAT_RLE_SPRITE     DAT_ID('R','L','E',' ')
#define DAT_C_SPRITE       DAT_ID('C','M','P',' ')
#define DAT_XC_SPRITE      DAT_ID('X','C','M','P')
#define DAT_PALLETE        DAT_ID('P','A','L',' ')
#define DAT_PROPERTY       DAT_ID('p','r','o','p')
#define DAT_END            -1


typedef struct DATAFILE_PROPERTY
{
   char *dat;                          /* pointer to the data */
   int type;                           /* property type */
} DATAFILE_PROPERTY;


typedef struct DATAFILE
{
   void *dat;                          /* pointer to the data */
   int type;                           /* object type */
   long size;                          /* size of the object */
   DATAFILE_PROPERTY *prop;            /* object properties */
} DATAFILE;


DATAFILE *load_datafile(char *filename);
void unload_datafile(DATAFILE *dat);

DATAFILE *load_datafile_object(char *filename, char *objectname);
void unload_datafile_object(DATAFILE *dat);

char *get_datafile_property(DATAFILE *dat, int type);
void register_datafile_object(int id, void *(*load)(PACKFILE *f, long size), void (*destroy)(void *data));

BITMAP *load_bitmap(char *filename, RGB *pal);
BITMAP *load_bmp(char *filename, RGB *pal);
BITMAP *load_lbm(char *filename, RGB *pal);
BITMAP *load_pcx(char *filename, RGB *pal);
BITMAP *load_tga(char *filename, RGB *pal);

int save_bitmap(char *filename, BITMAP *bmp, RGB *pal);
int save_pcx(char *filename, BITMAP *bmp, RGB *pal);
int save_tga(char *filename, BITMAP *bmp, RGB *pal);



/***************************************/
/************ Math routines ************/
/***************************************/

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
   if (x > 32767.0) {
      errno = ERANGE;
      return 0x7FFFFFFF;
   }

   if (x < -32768.0) {
      errno = ERANGE;
      return -0x80000000; 
   }

   return (long)(x * 65536.0 + (x < 0 ? -0.5 : 0.5)); 
}


__INLINE__ double fixtof(fixed x) 
{ 
   return (double)x / 65536.0; 
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


__INLINE__ fixed fadd(fixed x, fixed y)
{
   fixed result;

   asm (
      "  addl %2, %0 ; "                  /* do the addition */
      "  jno 0f ; "                       /* check for overflow */

      "  movl $2, _errno ; "              /* on overflow, set errno */
      "  movl $0x7fffffff, %0 ; "         /* and return MAXINT */
      "  cmpl $0, %2 ; " 
      "  jg 0f ; "
      "  negl %0 ; "

      " 0: "                              /* finished */

   : "=r" (result)                        /* result in a register */

   : "0" (x),                             /* x in the output register */
     "g" (y)                              /* y can go anywhere */

   : "%cc"                                /* clobbers flags */
   );

   return result;
}


__INLINE__ fixed fsub(fixed x, fixed y)
{
   fixed result;

   asm (
      "  subl %2, %0 ; "                  /* do the subtraction */
      "  jno 0f ; "                       /* check for overflow */

      "  movl $2, _errno ; "              /* on overflow, set errno */
      "  movl $0x7fffffff, %0 ; "         /* and return MAXINT */
      "  cmpl $0, %2 ; " 
      "  jl 0f ; "
      "  negl %0 ; "

      " 0: "                              /* finished */

   : "=r" (result)                        /* result in a register */

   : "0" (x),                             /* x in the output register */
     "g" (y)                              /* y can go anywhere */

   : "%cc"                                /* clobbers flags */
   );

   return result;
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
public:
   fixed v;

   fix()                                     { }
   fix(const fix &x)                         { v = x.v; }
   fix(const int x)                          { v = itofix(x); }
   fix(const long x)                         { v = itofix(x); }
   fix(const unsigned int x)                 { v = itofix(x); }
   fix(const unsigned long x)                { v = itofix(x); }
   fix(const float x)                        { v = ftofix(x); }
   fix(const double x)                       { v = ftofix(x); }

   operator int()                            { return fixtoi(v); }
   operator long()                           { return fixtoi(v); }
   operator unsigned int()                   { return fixtoi(v); }
   operator unsigned long()                  { return fixtoi(v); }
   operator float()                          { return fixtof(v); }
   operator double()                         { return fixtof(v); }

   fix& operator = (const fix &x)            { v = x.v;           return *this; }
   fix& operator = (const int x)             { v = itofix(x);     return *this; }
   fix& operator = (const long x)            { v = itofix(x);     return *this; }
   fix& operator = (const unsigned int x)    { v = itofix(x);     return *this; }
   fix& operator = (const unsigned long x)   { v = itofix(x);     return *this; }
   fix& operator = (const float x)           { v = ftofix(x);     return *this; }
   fix& operator = (const double x)          { v = ftofix(x);     return *this; }

   fix& operator +=  (const fix x)           { v += x.v;          return *this; }
   fix& operator -=  (const fix x)           { v -= x.v;          return *this; }
   fix& operator *=  (const fix x)           { v = fmul(v, x.v);  return *this; }
   fix& operator *=  (const int x)           { v *= x;            return *this; }
   fix& operator /=  (const fix x)           { v = fdiv(v, x.v);  return *this; }
   fix& operator /=  (const int x)           { v /= x;            return *this; }
   fix& operator <<= (const int x)           { v <<= x;           return *this; }
   fix& operator >>= (const int x)           { v >>= x;           return *this; }

   fix& operator ++ ()                       { v += itofix(1);    return *this; }
   fix& operator -- ()                       { v -= itofix(1);    return *this; }

   fix  operator -  ()                       { fix t;  t.v = -v;  return t; }

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


typedef struct MATRIX            /* transformation matrix (fixed point) */
{
   fixed v[3][3];                /* scaling and rotation */
   fixed t[3];                   /* translation */
} MATRIX;


typedef struct MATRIX_f          /* transformation matrix (floating point) */
{
   float v[3][3];                /* scaling and rotation */
   float t[3];                   /* translation */
} MATRIX_f;


extern MATRIX identity_matrix;
extern MATRIX_f identity_matrix_f;


void get_translation_matrix(MATRIX *m, fixed x, fixed y, fixed z);
void get_translation_matrix_f(MATRIX_f *m, float x, float y, float z);

void get_scaling_matrix(MATRIX *m, fixed x, fixed y, fixed z);
void get_scaling_matrix_f(MATRIX_f *m, float x, float y, float z);

void get_x_rotate_matrix(MATRIX *m, fixed r);
void get_x_rotate_matrix_f(MATRIX_f *m, float r);

void get_y_rotate_matrix(MATRIX *m, fixed r);
void get_y_rotate_matrix_f(MATRIX_f *m, float r);

void get_z_rotate_matrix(MATRIX *m, fixed r);
void get_z_rotate_matrix_f(MATRIX_f *m, float r);

void get_rotation_matrix(MATRIX *m, fixed x, fixed y, fixed z);
void get_rotation_matrix_f(MATRIX_f *m, float x, float y, float z);

void get_vector_rotation_matrix(MATRIX *m, fixed x, fixed y, fixed z, fixed a);
void get_vector_rotation_matrix_f(MATRIX_f *m, float x, float y, float z, float a);

void get_transformation_matrix(MATRIX *m, fixed scale, fixed xrot, fixed yrot, fixed zrot, fixed x, fixed y, fixed z);
void get_transformation_matrix_f(MATRIX_f *m, float scale, float xrot, float yrot, float zrot, float x, float y, float z);

void get_camera_matrix(MATRIX *m, fixed x, fixed y, fixed z, fixed xfront, fixed yfront, fixed zfront, fixed xup, fixed yup, fixed zup, fixed fov, fixed aspect);
void get_camera_matrix_f(MATRIX_f *m, float x, float y, float z, float xfront, float yfront, float zfront, float xup, float yup, float zup, float fov, float aspect);

void matrix_mul(MATRIX *m1, MATRIX *m2, MATRIX *out);
void matrix_mul_f(MATRIX_f *m1, MATRIX_f *m2, MATRIX_f *out);

fixed vector_length(fixed x, fixed y, fixed z);
float vector_length_f(float x, float y, float z);

void normalize_vector(fixed *x, fixed *y, fixed *z);
void normalize_vector_f(float *x, float *y, float *z);

void cross_product(fixed x1, fixed y1, fixed z1, fixed x2, fixed y2, fixed z2, fixed *xout, fixed *yout, fixed *zout);
void cross_product_f(float x1, float y1, float z1, float x2, float y2, float z2, float *xout, float *yout, float *zout);


__INLINE__ fixed dot_product(fixed x1, fixed y1, fixed z1, fixed x2, fixed y2, fixed z2)
{
   return fmul(x1, x2) + fmul(y1, y2) + fmul(z1, z2);
}


__INLINE__ float dot_product_f(float x1, float y1, float z1, float x2, float y2, float z2)
{
   return (x1 * x2) + (y1 * y2) + (z1 * z2);
}


__INLINE__ void apply_matrix(MATRIX *m, fixed x, fixed y, fixed z, fixed *xout, fixed *yout, fixed *zout)
{
   #define CALC_ROW(n)     (fmul(x, m->v[n][0]) +        \
			    fmul(y, m->v[n][1]) +        \
			    fmul(z, m->v[n][2]) +        \
			    m->t[n])

   *xout = CALC_ROW(0);
   *yout = CALC_ROW(1);
   *zout = CALC_ROW(2);

   #undef CALC_ROW
}


void apply_matrix_f(MATRIX_f *m, float x, float y, float z, float *xout, float *yout, float *zout);

extern fixed _persp_xscale, _persp_yscale, _persp_xoffset, _persp_yoffset;
extern float _persp_xscale_f, _persp_yscale_f, _persp_xoffset_f, _persp_yoffset_f;

void set_projection_viewport(int x, int y, int w, int h);


__INLINE__ void persp_project(fixed x, fixed y, fixed z, fixed *xout, fixed *yout)
{
   *xout = fmul(fdiv(x, z), _persp_xscale) + _persp_xoffset;
   *yout = fmul(fdiv(y, z), _persp_yscale) + _persp_yoffset;
}


__INLINE__ void persp_project_f(float x, float y, float z, float *xout, float *yout)
{
   float z1 = 1.0 / z;
   *xout = ((x * z1) * _persp_xscale_f) + _persp_xoffset_f;
   *yout = ((y * z1) * _persp_yscale_f) + _persp_yoffset_f;
}


#ifdef __cplusplus

}  /* end of extern "C" */

/* overloaded functions for use with the fix class */


__INLINE__ void get_translation_matrix(MATRIX *m, fix x, fix y, fix z)
{ 
   get_translation_matrix(m, x.v, y.v, z.v);
}


__INLINE__ void get_scaling_matrix(MATRIX *m, fix x, fix y, fix z)
{ 
   get_scaling_matrix(m, x.v, y.v, z.v); 
}


__INLINE__ void get_x_rotate_matrix(MATRIX *m, fix r)
{ 
   get_x_rotate_matrix(m, r.v);
}


__INLINE__ void get_y_rotate_matrix(MATRIX *m, fix r)
{ 
   get_y_rotate_matrix(m, r.v);
}


__INLINE__ void get_z_rotate_matrix(MATRIX *m, fix r)
{ 
   get_z_rotate_matrix(m, r.v);
}


__INLINE__ void get_rotation_matrix(MATRIX *m, fix x, fix y, fix z)
{ 
   get_rotation_matrix(m, x.v, y.v, z.v);
}


__INLINE__ void get_vector_rotation_matrix(MATRIX *m, fix x, fix y, fix z, fix a)
{ 
   get_vector_rotation_matrix(m, x.v, y.v, z.v, a.v);
}


__INLINE__ void get_transformation_matrix(MATRIX *m, fix scale, fix xrot, fix yrot, fix zrot, fix x, fix y, fix z)
{ 
   get_transformation_matrix(m, scale.v, xrot.v, yrot.v, zrot.v, x.v, y.v, z.v);
}


__INLINE__ void get_camera_matrix(MATRIX *m, fix x, fix y, fix z, fix xfront, fix yfront, fix zfront, fix xup, fix yup, fix zup, fix fov, fix aspect)
{ 
   get_camera_matrix(m, x.v, y.v, z.v, xfront.v, yfront.v, zfront.v, xup.v, yup.v, zup.v, fov.v, aspect.v);
}


__INLINE__ fix vector_length(fix x, fix y, fix z)
{ 
   fix t;
   t.v = vector_length(x.v, y.v, z.v);
   return t;
}


__INLINE__ void normalize_vector(fix *x, fix *y, fix *z)
{ 
   normalize_vector(&x->v, &y->v, &z->v);
}


__INLINE__ void cross_product(fix x1, fix y1, fix z1, fix x2, fix y2, fix z2, fix *xout, fix *yout, fix *zout)
{ 
   cross_product(x1.v, y1.v, z1.v, x2.v, y2.v, z2.v, &xout->v, &yout->v, &zout->v);
}


__INLINE__ fix dot_product(fix x1, fix y1, fix z1, fix x2, fix y2, fix z2)
{ 
   fix t;
   t.v = dot_product(x1.v, y1.v, z1.v, x2.v, y2.v, z2.v);
   return t;
}


__INLINE__ void apply_matrix(MATRIX *m, fix x, fix y, fix z, fix *xout, fix *yout, fix *zout)
{ 
   apply_matrix(m, x.v, y.v, z.v, &xout->v, &yout->v, &zout->v);
}


__INLINE__ void persp_project(fix x, fix y, fix z, fix *xout, fix *yout)
{ 
   persp_project(x.v, y.v, z.v, &xout->v, &yout->v);
}


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
   int (*proc)(void);            /* callback function */
   struct MENU *child;           /* to allow nested menus */
} MENU;


#define SEND_MESSAGE(d, msg, c)  (d)->proc(msg, d, c)

/* bits for the flags field */
#define D_EXIT          1        /* object makes the dialog exit */
#define D_SELECTED      2        /* object is selected */
#define D_GOTFOCUS      4        /* object has the input focus */
#define D_GOTMOUSE      8        /* mouse is on top of object */
#define D_HIDDEN        16       /* object is not visible */
#define D_DISABLED      32       /* object is visible but inactive */

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
#define MSG_RADIO       15       /* clear radio buttons */

/* some dialog procedures */
int d_clear_proc(int msg, DIALOG *d, int c);
int d_box_proc(int msg, DIALOG *d, int c);
int d_shadow_box_proc(int msg, DIALOG *d, int c);
int d_bitmap_proc(int msg, DIALOG *d, int c);
int d_text_proc(int msg, DIALOG *d, int c);
int d_ctext_proc(int msg, DIALOG *d, int c);
int d_button_proc(int msg, DIALOG *d, int c);
int d_check_proc(int msg, DIALOG *d, int c);
int d_radio_proc(int msg, DIALOG *d, int c);
int d_keyboard_proc(int msg, DIALOG *d, int c);
int d_edit_proc(int msg, DIALOG *d, int c);
int d_list_proc(int msg, DIALOG *d, int c);
int d_menu_proc(int msg, DIALOG *d, int c);

extern int gui_mouse_focus;
extern int gui_fg_color, gui_mg_color, gui_bg_color;
extern int gui_font_baseline;

void centre_dialog(DIALOG *dialog);
void set_dialog_color(DIALOG *dialog, int fg, int bg);
int find_dialog_focus(DIALOG *dialog);
int dialog_message(DIALOG *dialog, int msg, int c, int *obj);
int broadcast_dialog_message(int msg, int c);
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

