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

#include "allegro.h"


/* macros to enable and disable interrupts */
#define DISABLE()   asm volatile ("cli")
#define ENABLE()    asm volatile ("sti")


__INLINE__ void enter_critical() 
{
   if (windows_version >= 3) {
      __dpmi_regs r;
      r.x.ax = 0x1681; 
      __dpmi_int(0x2F, &r);
   }

   DISABLE();
}


__INLINE__ void exit_critical() 
{
   if (windows_version >= 3) {
      __dpmi_regs r;
      r.x.ax = 0x1682; 
      __dpmi_int(0x2F, &r);
   }

   ENABLE();
}


/* some Allegro functions need a block of scratch memory */
extern void *_scratch_mem;
extern int _scratch_mem_size;

__INLINE__ void _grow_scratch_mem(int size)
{
   if (size > _scratch_mem_size) {
      size = (size+1023) & 0xFFFFFC00;
      _scratch_mem = realloc(_scratch_mem, size);
      _scratch_mem_size = size;
   }
}


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


/* DPMI memory mapping routines */
int _create_physical_mapping(unsigned long *linear, int *segment, unsigned long physaddr, int size);
void _remove_physical_mapping(unsigned long *linear, int *segment);


/* various bits of mouse stuff */
void _set_mouse_range();
extern BITMAP *_mouse_screen;


/* various bits of timer stuff */
extern int _timer_use_retrace;
extern volatile int _retrace_hpp_value;


/* asm joystick polling routine */
int _poll_joystick(int *x, int *y, int *x2, int *y2, int poll_mask);


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

void _vbeaf_bank();
void _vbeaf_bank_end();

void _ati_bank();
void _ati_bank_end();

void _mach64_write_bank();
void _mach64_write_bank_end();
void _mach64_read_bank();
void _mach64_read_bank_end();

void _cirrus64_write_bank();
void _cirrus64_write_bank_end();
void _cirrus64_read_bank();
void _cirrus64_read_bank_end();

void _cirrus54_bank();
void _cirrus54_bank_end();

void _s3_bank();
void _s3_bank_end();

void _trident_bank();
void _trident_bank_end();

void _et3000_write_bank();
void _et3000_write_bank_end();
void _et3000_read_bank();
void _et3000_read_bank_end();

void _et4000_write_bank();
void _et4000_write_bank_end();
void _et4000_read_bank();
void _et4000_read_bank_end();

void _video7_bank();
void _video7_bank_end();


/* stuff for setting up bitmaps */
typedef struct GFX_MODE_INFO
{
   int w, h;
   int bios;
   int set_command;
} GFX_MODE_INFO;

BITMAP *_gfx_mode_set_helper(int w, int h, int v_w, int v_h, GFX_DRIVER *driver, int (*detect)(), GFX_MODE_INFO *mode_list, void (*set_width)(int w));
BITMAP *_make_bitmap(int w, int h, unsigned long addr, GFX_DRIVER *driver);
void _sort_out_virtual_width(int *width, GFX_DRIVER *driver);

long _vesa_vidmem_check(long mem);

extern GFX_VTABLE _linear_vtable, _modex_vtable;

extern int _sub_bitmap_id_count;


/* VGA register access routines */
void _vga_vsync();
void _vga_set_pallete_range(PALLETE p, int from, int to, int vsync);

extern int _crtc;


/* _read_vga_register:
 *  Reads the contents of a VGA register.
 */
__INLINE__ int _read_vga_register(int port, int index)
{
   if (port==0x3C0)
      inportb(_crtc+6); 

   outportb(port, index);
   return inportb(port+1);
}


/* _write_vga_register:
 *  Writes a byte to a VGA register.
 */
__INLINE__ void _write_vga_register(int port, int index, int v) 
{
   if (port==0x3C0) {
      inportb(_crtc+6);
      outportb(port, index);
      outportb(port, v);
   }
   else {
      outportb(port, index);
      outportb(port+1, v);
   }
}


/* _alter_vga_register:
 *  Alters specific bits of a VGA register.
 */
__INLINE__ void _alter_vga_register(int port, int index, int mask, int v)
{
   int temp;
   temp = _read_vga_register(port, index);
   temp &= (~mask);
   temp |= (v & mask);
   _write_vga_register(port, index, temp);
}


/* _vsync_out:
 *  Waits until the VGA is not in either a vertical or horizontal retrace.
 */
__INLINE__ void _vsync_out()
{
   do {
   } while (inportb(0x3DA) & 1);
}


/* _vsync_in:
 *  Waits until the VGA is in the vertical retrace period.
 */
__INLINE__ void _vsync_in()
{
   if (_timer_use_retrace) {
      int t = retrace_count; 

      do {
      } while (t == retrace_count);
   }
   else {
      do {
      } while (!(inportb(0x3DA) & 8));
   }
}


/* _write_hpp:
 *  Writes to the VGA pelpan register.
 */
__INLINE__ void _write_hpp(int value)
{
   if (_timer_use_retrace) {
      _retrace_hpp_value = value;

      do {
      } while (_retrace_hpp_value == value);
   }
   else {
      do {
      } while (!(inportb(0x3DA) & 8));

      _write_vga_register(0x3C0, 0x33, value);
   }
}


int _test_vga_register(int port, int index, int mask);
int _test_register(int port, int mask);
void _set_vga_virtual_width(int old_width, int new_width);


/* current drawing mode */
extern int _drawing_mode;
extern BITMAP *_drawing_pattern;
extern int _drawing_x_anchor;
extern int _drawing_y_anchor;
extern unsigned int _drawing_x_mask;
extern unsigned int _drawing_y_mask;


/* graphics drawing routines */
void _normal_line(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void _normal_rectfill(BITMAP *bmp, int x1, int y1, int x2, int y2, int color);
void _normal_rotate_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y, fixed angle);
void _normal_stretch_blit(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int source_width, int source_height, int dest_x, int dest_y, int dest_width, int dest_height, int masked);

int  _linear_getpixel(struct BITMAP *bmp, int x, int y);
void _linear_putpixel(struct BITMAP *bmp, int x, int y, int color);
void _linear_vline(struct BITMAP *bmp, int x, int y1, int y2, int color);
void _linear_hline(struct BITMAP *bmp, int x1, int y, int x2, int color);
void _linear_draw_sprite(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
void _linear_draw_sprite_v_flip(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
void _linear_draw_sprite_h_flip(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
void _linear_draw_sprite_vh_flip(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
void _linear_draw_rle_sprite(struct BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y);
void _linear_draw_character(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y, int color);
void _linear_textout_8x8(struct BITMAP *bmp, struct FONT_8x8 *f, char *str, int x, int y, int color);
void _linear_blit(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void _linear_blit_backward(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void _linear_clear_to_color(struct BITMAP *bitmap, int color);

int  _x_getpixel(struct BITMAP *bmp, int x, int y);
void _x_putpixel(struct BITMAP *bmp, int x, int y, int color);
void _x_vline(struct BITMAP *bmp, int x, int y1, int y2, int color);
void _x_hline(struct BITMAP *bmp, int x1, int y, int x2, int color);
void _x_draw_sprite(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
void _x_draw_sprite_v_flip(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
void _x_draw_sprite_h_flip(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
void _x_draw_sprite_vh_flip(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y);
void _x_draw_rle_sprite(struct BITMAP *bmp, struct RLE_SPRITE *sprite, int x, int y);
void _x_draw_character(struct BITMAP *bmp, struct BITMAP *sprite, int x, int y, int color);
void _x_textout_8x8(struct BITMAP *bmp, struct FONT_8x8 *f, char *str, int x, int y, int color);
void _x_blit_from_memory(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void _x_blit_to_memory(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void _x_blit(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void _x_blit_forward(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void _x_blit_backward(struct BITMAP *source, struct BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height);
void _x_clear_to_color(struct BITMAP *bitmap, int color);


/* asm helper for stretch_blit() */
void _do_stretch(BITMAP *source, BITMAP *dest, void *drawer, int sx, fixed sy, fixed syd, int dx, int dy, int dh);


/* sound lib stuff */
extern int _digi_volume;
extern int _midi_volume;
extern int _flip_pan; 
extern int _fm_port;
extern int _mpu_port;
extern int _sb_freq;
extern int _sb_port; 
extern int _sb_dma; 
extern int _sb_irq; 

extern int _fm_drum_mode;

int _sb_read_dsp_version();
int _sb_set_mixer(int digi_volume, int midi_volume);

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
