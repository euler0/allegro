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
 *      Video driver for ATI graphics cards.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pc.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/movedata.h>
#include <sys/farptr.h>

#include "allegro.h"
#include "internal.h"


#include <stdarg.h>
void trace(char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
void trace(char *fmt, ...)
{
/* debugging code for the unfinished mach64 driver */
#if 0
   static int virgin = TRUE;

   va_list args;
   char buf[1024];
   FILE *f; 

   va_start(args, fmt);
   vsprintf(buf, fmt, args);
   va_end(args);

   f = fopen("log.txt", virgin ? "w" : "a");
   fputs(buf, f);
   fclose(f);

   virgin = FALSE;
#endif
}


static BITMAP *ati_init(int w, int h, int v_w, int v_h);
static int ati_scroll(int x, int y);

static BITMAP *mach64_init(int w, int h, int v_w, int v_h);
static int mach64_scroll(int x, int y);

#define ATI_18800       1
#define ATI_18800_1     2
#define ATI_28800_2     3
#define ATI_28800_4     4
#define ATI_28800_5     5

static int ati_type;

int _ati_port = 0x1CE;           /* ATI 18800/28800 base address */

int _mach64_wp_sel;              /* mach64 write bank address */
int _mach64_rp_sel;              /* mach64 read bank address */
int _mach64_off_pitch;           /* mach64 display start address */



GFX_DRIVER gfx_ati = 
{
   "ATI 18800/28800",
   "",
   ati_init,
   NULL,
   ati_scroll,
   _vga_vsync,
   _vga_set_pallete_range,
   0, 0, FALSE, 
   0x10000,       /* 64k banks */
   0x10000,       /* 64k granularity */
   0
};



GFX_DRIVER gfx_mach64 = 
{
   "ATI mach64",
   "",
   mach64_init,
   NULL,
   mach64_scroll,
   _vga_vsync,
   _vga_set_pallete_range,
   0, 0, FALSE, 
   0x10000,       /* 64k banks */
   0x8000,        /* 32k granularity */
   0
};



static GFX_MODE_INFO ati_18800_mode_list[] =
{
   {  640,  400,  0x61,  0  },
   {  640,  480,  0x62,  0  },
   {  800,  600,  0x63,  0  },
   {  0,    0,    0,     0  }
};



static GFX_MODE_INFO ati_28800_mode_list[] =
{
   {  640,  400,  0x61,  0  },
   {  640,  480,  0x62,  0  },
   {  800,  600,  0x63,  0  },
   {  1024, 768,  0x64,  0  },
   {  0,    0,    0,     0  }
};



static GFX_MODE_INFO mach64_mode_list[] =
{
   {  640,  480,  0x101,  0x4F02  },
   {  800,  600,  0x103,  0x4F02  },
   {  1024, 768,  0x105,  0x4F02  },
   {  1280, 1024, 0x107,  0x4F02  },
   {  0,    0,    0,      0       }
};



/* ati_detect:
 *  Detects the presence of a ATI card.
 */
static int ati_detect()
{
   char buf[16];

   dosmemget(0xC0031, 9, buf);                  /* check ID string */
   if (memcmp(buf, "761295520", 9) != 0)
      return FALSE;

   _ati_port = _farpeekw(_dos_ds, 0xC0010);     /* read port address */
   if (!_ati_port)
      _ati_port = 0x1CE;

   switch (_farpeekb(_dos_ds, 0xC0043)) {       /* check ATI type */

      case '1':
	 ati_type = ATI_18800;
	 gfx_ati.desc = "18800";
	 break;

      case '2': 
	 ati_type = ATI_18800_1;
	 gfx_ati.desc = "18800-1";
	 break;

      case '3': 
	 ati_type = ATI_28800_2;
	 gfx_ati.desc = "28800-2";
	 break;

      case '4': 
	 ati_type = ATI_28800_4;
	 gfx_ati.desc = "28800-4";
	 break;

      case '5': 
	 ati_type = ATI_28800_5;
	 gfx_ati.desc = "28800-5";
	 break;

      default:
	 /* unknown sort of ATI */
	 return FALSE;
   }

   if (ati_type < ATI_28800_2)
      gfx_ati.vid_mem = _vesa_vidmem_check(512*1024);
   else
      gfx_ati.vid_mem = _vesa_vidmem_check(1024*1024);

   return TRUE;
}



/* ati_init:
 *  Tries to enter the specified graphics mode, and makes a screen bitmap
 *  for it.
 */
static BITMAP *ati_init(int w, int h, int v_w, int v_h)
{
   BITMAP *b;

   b = _gfx_mode_set_helper(w, h, v_w, v_h, &gfx_ati, ati_detect, 
			    (ati_type <= ATI_18800_1) ? ati_18800_mode_list : 
						   ati_28800_mode_list, NULL);
   if (!b)
      return NULL;

   LOCK_VARIABLE(_ati_port);
   LOCK_FUNCTION(_ati_bank);

   if (ati_type >= ATI_28800_4)                 /* allow display past 512k */
      _alter_vga_register(_ati_port, 0xB6, 1, 1);

   if (ati_type >= ATI_18800_1)                 /* select single bank mode */
      _alter_vga_register(_ati_port, 0xBE, 8, 0);

   b->write_bank = b->read_bank = _ati_bank;

   return b;
}



/* ati_scroll:
 *  Hardware scrolling for ATI cards.
 */
static int ati_scroll(int x, int y)
{
   long a = x + (y * VIRTUAL_W);

   DISABLE();

   _vsync_out();

   if (ati_type < ATI_28800_2) {
      _alter_vga_register(_ati_port, 0xB0, 0xC0, a>>12);
   }
   else {
      _alter_vga_register(_ati_port, 0xB0, 0x40, a>>12);
      _alter_vga_register(_ati_port, 0xA3, 0x10, a>>15);
   }

   /* write to normal VGA address registers */
   _write_vga_register(_crtc, 0x0D, (a>>2) & 0xFF);
   _write_vga_register(_crtc, 0x0C, (a>>10) & 0xFF);

   ENABLE();

   _vsync_in();

   return 0;
}



/* get_mach64_port:
 *  Calculates the port address for accessing a specific mach64 register.
 */
static int get_mach64_port(int io_type, int io_base, int io_sel, int mm_sel)
{
   trace("get_mach64_port(0x%X, 0x%X, 0x%X, 0x%X)\n", io_type, io_base, io_sel, mm_sel);

   if (io_type) {
      trace("returning 0x%X\n", (mm_sel << 2) + io_base);
      return (mm_sel << 2) + io_base;
   }
   else {
      if (!io_base)
	 io_base = 0x2EC;

      trace("returning 0x%X\n", (io_sel << 10) + io_base);
      return (io_sel << 10) + io_base;
   }
}



/* mach64_detect:
 *  Detects the presence of a mach64 card.
 */
static int mach64_detect()
{
   __dpmi_regs r;
   int scratch_reg;
   unsigned long old;

   trace("start of mach64_detect\n");

   /* query mach64 BIOS for the I/O base address */
   r.x.ax = 0xA012; 
   r.x.cx = 0;
   __dpmi_int(0x10, &r);

   trace("called bios\n");

   if (r.h.ah) {
      trace("r.h.ah != 0\n");
      return FALSE;
   }

   /* test scratch register to confirm we have a mach64 */
   scratch_reg = get_mach64_port(r.x.cx, r.x.dx, 0x11, 0x21);
   old = inportl(scratch_reg);

   trace("old = 0x%lX\n", old);

   outportl(scratch_reg, 0x55555555);
   if (inportl(scratch_reg) != 0x55555555) {
      outportl(scratch_reg, old);
      trace("scratch 0x5x failed\n");
      return FALSE;
   }

   outportl(scratch_reg, 0xAAAAAAAA);
   if (inportl(scratch_reg) != 0xAAAAAAAA) {
      outportl(scratch_reg, old);
      trace("scratch 0xAx failed\n");
      return FALSE;
   }

   outportl(scratch_reg, old);

   /* calculate some port addresses... */
   _mach64_wp_sel = get_mach64_port(r.x.cx, r.x.dx, 0x15, 0x2D);
   _mach64_rp_sel = get_mach64_port(r.x.cx, r.x.dx, 0x16, 0x2E);
   _mach64_off_pitch = get_mach64_port(r.x.cx, r.x.dx, 5, 5);

   gfx_mach64.vid_mem = _vesa_vidmem_check(8192*1024);

   trace("mach64 detected\n");

   return TRUE;
}



/* set_mach64_width:
 *  Sets the scanline width of the mach64 crtc.
 */
static void set_mach64_width(int width)
{
   trace("set_mach64_width(%d)\n", width);
   trace("current off pitch = 0x%lX\n", inportl(_mach64_off_pitch));

   outportl(_mach64_off_pitch, 
	    (inportl(_mach64_off_pitch) & 0xFFFFF) | (width << 25));

   trace("changed off pitch = 0x%lX\n", inportl(_mach64_off_pitch));
}



/* mach64_init:
 *  Tries to enter the specified graphics mode, and makes a screen bitmap
 *  for it.
 */
static BITMAP *mach64_init(int w, int h, int v_w, int v_h)
{
   BITMAP *b;

   trace("start of mach64_init\n");

   b = _gfx_mode_set_helper(w, h, v_w, v_h, &gfx_mach64, mach64_detect, mach64_mode_list, set_mach64_width);
   if (!b)
      return NULL;

   trace("got screen bitmap\n");

   LOCK_VARIABLE(_mach64_wp_sel);
   LOCK_VARIABLE(_mach64_rp_sel);
   LOCK_VARIABLE(_mach64_off_pitch);
   LOCK_FUNCTION(_mach64_write_bank);
   LOCK_FUNCTION(_mach64_read_bank);

   b->write_bank = _mach64_write_bank;
   b->read_bank = _mach64_read_bank;

   trace("ok, sorted!\n");

   return b;
}



/* mach64_scroll:
 *  Hardware scrolling for mach64 cards.
 */
static int mach64_scroll(int x, int y)
{
   long a = x + (y * VIRTUAL_W);

   _vsync_out();

   outportl(_mach64_off_pitch,
	    (inportl(_mach64_off_pitch) & 0xFFF00000) | (a >> 3));

   _vsync_in();

   return 0;
}
