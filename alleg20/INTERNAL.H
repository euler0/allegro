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
 *      Some definitions for internal use by the library code.
 *      This should not be included by user programs.
 *
 *      See readme.txt for copyright information.
 */


#ifndef INTERNAL_H
#define INTERNAL_H


/* macros to enable and disable interrupts */
#define DISABLE()   asm volatile ("cli")
#define ENABLE()    asm volatile ("sti")

/* interrupt hander stuff */
int _install_irq(int num, int (*handler)());
void _remove_irq(int num);

typedef struct _IRQ_HANDLER
{
   int (*handler)();             /* our C handler */
   int number;                   /* irq number */
   __dpmi_paddr old_vector;      /* original protected mode vector */
} _IRQ_HANDLER;

/* list of functions to call at program cleanup */
void _add_exit_func(void (*func)());
void _remove_exit_func(void (*func)());

/* various bits of mouse stuff */
void _set_mouse_range();
extern BITMAP *_mouse_screen;

/* caches and tables for svga bank switching */
extern int _last_bank_1, _last_bank_2; 
extern int *_gfx_bank; 

/* bank switching routines */
void _stub_bank_switch(); 
void _stub_bank_switch_end();

void _vesa_window_1();
void _vesa_window_1_end();
void _vesa_window_2();
void _vesa_window_2_end();

void _vesa_pm_window_1();
void _vesa_pm_window_1_end();
void _vesa_pm_window_2();
void _vesa_pm_window_2_end();

void _vesa_pm_es_window_1();
void _vesa_pm_es_window_1_end();
void _vesa_pm_es_window_2();
void _vesa_pm_es_window_2_end();

void _cirrus64_write_bank();
void _cirrus64_write_bank_end();
void _cirrus64_read_bank();
void _cirrus64_read_bank_end();

void _cirrus54_bank();
void _cirrus54_bank_end();

void _s3_bank();
void _s3_bank_end();

void _et3000_write_bank();
void _et3000_write_bank_end();
void _et3000_read_bank();
void _et3000_read_bank_end();

void _et4000_write_bank();
void _et4000_write_bank_end();
void _et4000_read_bank();
void _et4000_read_bank_end();

/* helpers for setting up the screen bitmap */
BITMAP *_make_bitmap(int w, int h, unsigned long addr, GFX_DRIVER *driver);
void _sort_out_virtual_width(int *width, GFX_DRIVER *driver);

long _vesa_vidmem_check(long mem);

/* VGA register access routines */
extern int _crtc;

int _read_vga_register(int port, int index);
void _write_vga_register(int port, int index, int v);
void _alter_vga_register(int port, int index, int mask, int v);
int _test_vga_register(int port, int index, int mask);
int _test_register(int port, int mask);
void _set_vga_virtual_width(int old_width, int new_width);

/* xor? */
extern int _xor;

/* for drawing filled polygons and circles */
typedef struct {
   int lpos, rpos;
} FILL_STRUCT;

extern FILL_STRUCT *_fill_array;

extern int _bitmap_table_size;

void _fill_init(BITMAP *bmp);
void _fill_putpix(BITMAP *bmp, int x, int y, int c);
void _fill_finish(BITMAP *bmp, int color);
void _fill_line(int x1, int y1, int x2, int y2, int min, int max);

/* text output routines */
void _textout_8x8(BITMAP *bmp, FONT_8x8 *f, char *str, int x, int y, int color);
void _draw_character(BITMAP *bmp, BITMAP *sprite, int x, int y, int color);

/* asm helper for stretch_blit() */
void _do_stretch(BITMAP *source, BITMAP *dest, void *drawer, int sx, fixed sy, fixed syd, int dx, int dy, int dh);

/* sound lib stuff */
extern int _flip_pan; 
extern int _fm_port;
extern int _mpu_port;
extern int _sb_freq;
extern int _sb_port; 
extern int _sb_dma; 
extern int _sb_irq; 

extern int _fm_drum_mode;

int sb_read_dsp_version();

int _midi_init();
void _midi_exit();
void _midi_map_program(int program, int bank1, int bank2, int prog, int pitch);

extern volatile long _midi_tick;

#define MIXER_MAX_SFX               8
#define MIXER_VOLUME_LEVELS         32
#define MIXER_FIX_SHIFT             12

int _mixer_init(int bufsize, int freq, int stereo, int is16bit);
void _mixer_exit();
unsigned long _mixer_voice_status(int voice);
void _mixer_play(int voice, SAMPLE *spl, int vol, int pan, int freq, int loop);
void _mixer_adjust(int voice, int vol, int pan, int freq, int loop);
void _mixer_stop(int voice);
void _mix_some_samples(unsigned long buf);

int _dma_allocate_mem(int bytes, int *sel, unsigned long *phys);
void _dma_start(int channel, unsigned long addr, int size, int auto_init);
void _dma_stop(int channel);
unsigned long _dma_todo(int channel);
void _dma_lock_mem();

/* dummy functions for the nosound drivers */
int _dummy_detect();
int _dummy_init();
void _dummy_exit();
void _dummy_play(int voice, SAMPLE *spl, int vol, int pan, int freq, int loop);
void _dummy_adjust(int voice, int vol, int pan, int freq, int loop);
void _dummy_stop(int voice);
unsigned long _dummy_voice_status(int voice);
int _dummy_load_patches(char *patches, char *drums);
void _dummy_key_on(int voice, int inst, int note, int bend, int vol, int pan);
void _dummy_key_off(int voice);
void _dummy_set_volume(int voice, int vol);
void _dummy_set_pitch(int voice, int note, int bend);


#endif          /* ifndef INTERNAL_H */
