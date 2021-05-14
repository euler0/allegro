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
 *      Video driver for VESA compatible graphics cards. Supports VESA 2.0 
 *      linear framebuffers and protected mode bank switching interface.
 *
 *      See readme.txt for copyright information.
 */


#ifndef DJGPP
#error This file should only be used by the djgpp version of Allegro
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/movedata.h>
#include <sys/farptr.h>
#include <sys/segments.h>

#include "allegro.h"
#include "internal.h"


static char vesa_desc[80] = "";

static BITMAP *vesa_1_init(int w, int h, int v_w, int v_h);
static BITMAP *vesa_2b_init(int w, int h, int v_w, int v_h);
static BITMAP *vesa_2l_init(int w, int h, int v_w, int v_h);
static void vesa_exit(BITMAP *b);
static int vesa_scroll(int x, int y);
static void vesa_vsync();
static void vesa_set_pallete_range(PALLETE p, int from, int to, int vsync);



GFX_DRIVER gfx_vesa_1 = 
{
   "VESA 1.x",
   vesa_desc,
   vesa_1_init,
   vesa_exit,
   vesa_scroll,
   _vga_vsync,
   _vga_set_pallete_range,
   0, 0, FALSE, 0, 0, 0, 0
};



GFX_DRIVER gfx_vesa_2b = 
{
   "VESA 2.0 (banked)",
   vesa_desc,
   vesa_2b_init,
   vesa_exit,
   vesa_scroll,
   vesa_vsync,
   vesa_set_pallete_range,
   0, 0, FALSE, 0, 0, 0, 0
};



GFX_DRIVER gfx_vesa_2l = 
{
   "VESA 2.0 (linear)",
   vesa_desc,
   vesa_2l_init,
   vesa_exit,
   vesa_scroll,
   vesa_vsync,
   vesa_set_pallete_range,
   0, 0, FALSE, 0, 0, 0, 0
};



#define MASK_LINEAR(addr)     (addr & 0x000FFFFF)
#define RM_TO_LINEAR(addr)    (((addr & 0xFFFF0000) >> 12) + (addr & 0xFFFF))
#define RM_OFFSET(addr)       (MASK_LINEAR(addr) & 0xFFFF)
#define RM_SEGMENT(addr)      ((MASK_LINEAR(addr) & 0xFFFF0000) >> 4)



typedef struct VESA_INFO         /* VESA information block structure */
{ 
   unsigned char  VESASignature[4]     __attribute__ ((packed));
   unsigned short VESAVersion          __attribute__ ((packed));
   unsigned long  OEMStringPtr         __attribute__ ((packed));
   unsigned char  Capabilities[4]      __attribute__ ((packed));
   unsigned long  VideoModePtr         __attribute__ ((packed)); 
   unsigned short TotalMemory          __attribute__ ((packed)); 
   unsigned short OemSoftwareRev       __attribute__ ((packed)); 
   unsigned long  OemVendorNamePtr     __attribute__ ((packed)); 
   unsigned long  OemProductNamePtr    __attribute__ ((packed)); 
   unsigned long  OemProductRevPtr     __attribute__ ((packed)); 
   unsigned char  Reserved[222]        __attribute__ ((packed)); 
   unsigned char  OemData[256]         __attribute__ ((packed)); 
} VESA_INFO;



typedef struct MODE_INFO         /* VESA information for a specific mode */
{
   unsigned short ModeAttributes       __attribute__ ((packed)); 
   unsigned char  WinAAttributes       __attribute__ ((packed)); 
   unsigned char  WinBAttributes       __attribute__ ((packed)); 
   unsigned short WinGranularity       __attribute__ ((packed)); 
   unsigned short WinSize              __attribute__ ((packed)); 
   unsigned short WinASegment          __attribute__ ((packed)); 
   unsigned short WinBSegment          __attribute__ ((packed)); 
   unsigned long  WinFuncPtr           __attribute__ ((packed)); 
   unsigned short BytesPerScanLine     __attribute__ ((packed)); 
   unsigned short XResolution          __attribute__ ((packed)); 
   unsigned short YResolution          __attribute__ ((packed)); 
   unsigned char  XCharSize            __attribute__ ((packed)); 
   unsigned char  YCharSize            __attribute__ ((packed)); 
   unsigned char  NumberOfPlanes       __attribute__ ((packed)); 
   unsigned char  BitsPerPixel         __attribute__ ((packed)); 
   unsigned char  NumberOfBanks        __attribute__ ((packed)); 
   unsigned char  MemoryModel          __attribute__ ((packed)); 
   unsigned char  BankSize             __attribute__ ((packed)); 
   unsigned char  NumberOfImagePages   __attribute__ ((packed));
   unsigned char  Reserved_page        __attribute__ ((packed)); 
   unsigned char  RedMaskSize          __attribute__ ((packed)); 
   unsigned char  RedFieldPosition     __attribute__ ((packed)); 
   unsigned char  GreenMaskSize        __attribute__ ((packed)); 
   unsigned char  GreenFieldPosition   __attribute__ ((packed));
   unsigned char  BlueMaskSize         __attribute__ ((packed)); 
   unsigned char  BlueFieldPosition    __attribute__ ((packed)); 
   unsigned char  RsvdMaskSize         __attribute__ ((packed)); 
   unsigned char  RsvdFieldPosition    __attribute__ ((packed)); 
   unsigned char  DirectColorModeInfo  __attribute__ ((packed));
   unsigned long  PhysBasePtr          __attribute__ ((packed)); 
   unsigned long  OffScreenMemOffset   __attribute__ ((packed)); 
   unsigned short OffScreenMemSize     __attribute__ ((packed)); 
   unsigned char  Reserved[206]        __attribute__ ((packed)); 
} MODE_INFO;



typedef struct PM_INFO           /* VESA 2.0 protected mode interface */
{
   unsigned short setWindow            __attribute__ ((packed)); 
   unsigned short setDisplayStart      __attribute__ ((packed)); 
   unsigned short setPalette           __attribute__ ((packed)); 
   unsigned short IOPrivInfo           __attribute__ ((packed)); 
   /* pmode code is located in this space */
} PM_INFO;



static VESA_INFO vesa_info;            /* SVGA info block */
static MODE_INFO mode_info;            /* info for this video mode */

__dpmi_regs _dpmi_reg;                 /* for calling int 10 bank switch */

int _window_2_offset = 0;              /* windows at different addresses? */

static unsigned long lb_linear = 0;    /* linear address of framebuffer */
static int lb_segment = 0;             /* descriptor for the buffer */

static PM_INFO *pm_info = NULL;        /* VESA 2.0 pmode interface */
void (*_pm_vesa_switcher)() = NULL;    /* pmode bank switch routine */
void (*_pm_vesa_scroller)() = NULL;    /* pmode scrolling routine */
void (*_pm_vesa_pallete)() = NULL;     /* pmode pallete get/set routine */

static unsigned long mmio_linear = 0;  /* linear address for mem mapped IO */
int _mmio_segment = 0;                 /* selector to pass in %es */

static int vesa_xscroll = 0;           /* current display start address */
static int vesa_yscroll = 0;

static int evilness_flag = FALSE;      /* set if we are doing dodgy things
					* with VGA registers because the
					* VESA implementation is no good.
					*/



/* get_vesa_info:
 *  Retrieves a VESA info block structure, returning 0 for success.
 */
static int get_vesa_info()
{
   int c;

   for (c=0; c<sizeof(VESA_INFO); c++)
      _farpokeb(_dos_ds, MASK_LINEAR(__tb)+c, 0);

   dosmemput("VBE2", 4, MASK_LINEAR(__tb));

   _dpmi_reg.x.ax = 0x4F00;
   _dpmi_reg.x.di = RM_OFFSET(__tb);
   _dpmi_reg.x.es = RM_SEGMENT(__tb);
   __dpmi_int(0x10, &_dpmi_reg);
   if (_dpmi_reg.h.ah)
      return -1;

   dosmemget(MASK_LINEAR(__tb), sizeof(VESA_INFO), &vesa_info);

   if (strncmp(vesa_info.VESASignature, "VESA", 4) != 0)
      return -1;

   return 0;
}



/* get_mode_info:
 *  Retrieves a mode info block structure, for a specific graphics mode.
 *  Returns 0 for success.
 */
static int get_mode_info(int mode)
{
   int c;

   for (c=0; c<sizeof(MODE_INFO); c++)
      _farpokeb(_dos_ds, MASK_LINEAR(__tb)+c, 0);

   _dpmi_reg.x.ax = 0x4F01;
   _dpmi_reg.x.di = RM_OFFSET(__tb);
   _dpmi_reg.x.es = RM_SEGMENT(__tb);
   _dpmi_reg.x.cx = mode;
   __dpmi_int(0x10, &_dpmi_reg);
   if (_dpmi_reg.h.ah)
      return -1;

   dosmemget(MASK_LINEAR(__tb), sizeof(MODE_INFO), &mode_info);
   return 0;
}



/* _vesa_vidmem_check:
 *  Trying to autodetect the available video memory in my hardware level 
 *  card drivers is a hopeless task. Fortunately that seems to be one of 
 *  the few things that VESA usually gets right, so if VESA is available 
 *  the non-VESA drivers can use it to confirm how much vidmem is present.
 */
long _vesa_vidmem_check(long mem)
{
   if (get_vesa_info() != 0)
      return mem;

   if (vesa_info.TotalMemory <= 0)
      return mem;

   return MIN(mem, (vesa_info.TotalMemory << 16));
}



/* find_vesa_mode:
 *  Tries to find a VESA mode number for the specified screen size.
 *  Searches the mode list from the VESA info block, and if that doesn't
 *  work, uses the standard VESA mode numbers.
 */
static int find_vesa_mode(int w, int h, int vbe2)
{
   #define MAX_VESA_MODES 1024

   unsigned short mode[MAX_VESA_MODES];
   int c, modes;
   long mode_ptr;

   if (get_vesa_info() != 0) {
      strcpy(allegro_error, "VESA not found");
      return 0;
   }

   if ((vesa_info.VESAVersion < 0x200) && (vbe2)) {
      strcpy(allegro_error, "VBE 2.0 not available");
      return 0;
   }

   mode_ptr = RM_TO_LINEAR(vesa_info.VideoModePtr);
   modes = 0;

   while ((mode[modes] = _farpeekw(_dos_ds, mode_ptr)) != 0xFFFF) {
      modes++;
      mode_ptr += 2;
   }

   /* search the list of modes */
   for (c=0; c<modes; c++) {
      if (get_mode_info(mode[c]) == 0) {
	 if ((mode_info.ModeAttributes & 1) &&     /* mode is available */
	     (mode_info.ModeAttributes & 8) &&     /* and is a color mode */
	     (mode_info.ModeAttributes & 16) &&    /* and a graphics mode */
	     (mode_info.XResolution == w) &&       /* check width */
	     (mode_info.YResolution == h) &&       /* check height */
	     (mode_info.NumberOfPlanes == 1) &&    /* no planes please */
	     (mode_info.BitsPerPixel == 8) &&      /* want 256 colors */
	     (mode_info.MemoryModel == 4))         /* want packed pixels */
	    /* looks like this will do... */
	    return mode[c];
      } 
   }

   /* try the standard mode numbers */
   if ((w == 640) && (h == 400))
      c = 0x100;
   else if ((w == 640) && (h == 480))
      c = 0x101;
   else if ((w == 800) && (h == 600)) 
      c = 0x103;
   else if ((w == 1024) && (h == 768))
      c = 0x105;
   else if ((w == 1280) && (h == 1024))
      c = 0x107;
   else {
      strcpy(allegro_error, "Resolution not supported");
      return 0; 
   }

   if (get_mode_info(c) == 0)
      return c;

   strcpy(allegro_error, "Resolution not supported");
   return 0;
}



/* setup_vesa_desc:
 *  Sets up the VESA driver description string.
 */
static void setup_vesa_desc()
{
   unsigned long addr;
   int p;

   /* vesa version number */
   sprintf(vesa_desc, "%4.4s %d.%d (", vesa_info.VESASignature,
	   vesa_info.VESAVersion>>8, vesa_info.VESAVersion&0xFF);

   /* vesa description string */
   p = strlen(vesa_desc);
   addr = RM_TO_LINEAR(vesa_info.OEMStringPtr);
   _farsetsel(_dos_ds);
   while (_farnspeekb(addr) != 0) {
      vesa_desc[p++] = _farnspeekb(addr++);
      if (p > 50)
	 break;
   }

   if (evilness_flag)
      strcpy(vesa_desc+p, "), func. 0x4F06 N.A.");
   else
      strcpy(vesa_desc+p, ")");
}



/* get_pmode_functions:
 *  Attempts to use VBE 2.0 functions to get access to protected mode bank
 *  switching, set display start, and pallete routines, storing the bank
 *  switchers in the w* parameters.
 */
static int get_pmode_functions(void (**w1)(), void (**w2)())
{
   unsigned short *p;

   if (vesa_info.VESAVersion < 0x200)        /* have we got VESA 2.0? */
      return -1;

   _dpmi_reg.x.ax = 0x4F0A;                  /* retrieve pmode interface */
   _dpmi_reg.x.bx = 0;
   __dpmi_int(0x10, &_dpmi_reg);
   if (_dpmi_reg.h.ah)
      return -1;

   if (pm_info)
      free(pm_info);

   pm_info = malloc(_dpmi_reg.x.cx);         /* copy into our address space */
   _go32_dpmi_lock_data(pm_info, _dpmi_reg.x.cx);
   dosmemget((_dpmi_reg.x.es*16)+_dpmi_reg.x.di, _dpmi_reg.x.cx, pm_info);

   _mmio_segment = 0;

   if (pm_info->IOPrivInfo) {                /* need memory mapped IO? */
      p = (unsigned short *)((char *)pm_info + pm_info->IOPrivInfo);
      while (*p != 0xFFFF)                   /* skip the port table */
	 p++;
      p++;
      if (*p != 0xFFFF) {                    /* get descriptor */
	 if (_create_physical_mapping(&mmio_linear, &_mmio_segment,
				      *((unsigned long *)p), *(p+2)) != 0)
	    return -1;
      }
   }

   _pm_vesa_switcher = (void *)((char *)pm_info + pm_info->setWindow);
   _pm_vesa_scroller = (void *)((char *)pm_info + pm_info->setDisplayStart);
   _pm_vesa_pallete  = (void *)((char *)pm_info + pm_info->setPalette);

   if (_mmio_segment) {
      *w1 = _vesa_pm_es_window_1;
      *w2 = _vesa_pm_es_window_2;
   }
   else {
      *w1 = _vesa_pm_window_1;
      *w2 = _vesa_pm_window_2;
   }

   return 0;
}



/* sort_out_vesa_windows:
 *  Checks the mode info block structure to determine which VESA windows
 *  should be used for reading and writing to video memory.
 */
static int sort_out_vesa_windows(BITMAP *b, void (*w1)(), void (*w2)())
{
   if ((mode_info.WinAAttributes & 5) == 5)        /* write to window 1? */
      b->write_bank = w1;
   else if ((mode_info.WinBAttributes & 5) == 5)   /* write to window 2? */
      b->write_bank = w2;
   else
      return -1;

   if ((mode_info.WinAAttributes & 3) == 3)        /* read from window 1? */
      b->read_bank = w1;
   else if ((mode_info.WinBAttributes & 3) == 3)   /* read from window 2? */
      b->read_bank = w2;
   else
      return -1;

   /* are the windows at different places in memory? */
   _window_2_offset = (mode_info.WinBSegment - mode_info.WinASegment) * 16;

   return 0;
}



/* make_linear_bitmap:
 *  Creates a screen bitmap for a linear framebuffer mode, creating the
 *  required physical address mappings and allocating an LDT descriptor
 *  to access the memory.
 */
static BITMAP *make_linear_bitmap(GFX_DRIVER *driver, int width, int height)
{
   BITMAP *b;

   if (_create_physical_mapping(&lb_linear, &lb_segment, 
				mode_info.PhysBasePtr, driver->vid_mem) != 0)
      return NULL;

   b = _make_bitmap(width, height, 0, driver);
   if (!b) {
      _remove_physical_mapping(&lb_linear, &lb_segment);
      return NULL;
   }

   driver->vid_phys_base = mode_info.PhysBasePtr;

   b->seg = lb_segment;
   return b;
}



/* vesa_init:
 *  Tries to enter the specified graphics mode, and makes a screen bitmap
 *  for it. Will use a linear framebuffer if one is available.
 */
static BITMAP *vesa_init(GFX_DRIVER *driver, int linear, int vbe2, int w, int h, int v_w, int v_h)
{
   BITMAP *b;
   int width, height;
   int vesa_mode;
   void (*w1)() = _vesa_window_1;
   void (*w2)() = _vesa_window_2;

   vesa_mode = find_vesa_mode(w, h, vbe2);      /* find mode number to use */
   if (vesa_mode == 0)
      return NULL;

   LOCK_FUNCTION(_vesa_window_1);
   LOCK_FUNCTION(_vesa_window_2);
   LOCK_FUNCTION(_vesa_pm_window_1);
   LOCK_FUNCTION(_vesa_pm_window_2);
   LOCK_FUNCTION(_vesa_pm_es_window_1);
   LOCK_FUNCTION(_vesa_pm_es_window_2);
   LOCK_VARIABLE(_window_2_offset);
   LOCK_VARIABLE(_dpmi_reg);
   LOCK_VARIABLE(_pm_vesa_switcher);
   LOCK_VARIABLE(_mmio_segment);

   driver->vid_mem = vesa_info.TotalMemory << 16;

   if (linear) {
      if (mode_info.ModeAttributes & 0x80) {    /* linear framebuffer? */
	 driver->linear = TRUE;
	 driver->bank_size = driver->bank_gran = 0;
      }
      else {
	 strcpy(allegro_error, "VBE 2.0 linear framebuffer not available");
	 return NULL;
      }
   }
   else {
      driver->linear = FALSE;
      driver->bank_size = mode_info.WinSize * 1024;
      driver->bank_gran = mode_info.WinGranularity * 1024;
   }

   _dpmi_reg.x.ax = 0x4F02;                     /* set screen mode */
   _dpmi_reg.x.bx = vesa_mode;
   if (driver->linear)
      _dpmi_reg.x.bx |= 0x4000;
   __dpmi_int(0x10, &_dpmi_reg);
   if (_dpmi_reg.h.ah) {
      strcpy(allegro_error, "VESA function 0x4F02 failed");
      return NULL;
   }

   width = MAX(mode_info.BytesPerScanLine, v_w);
   _sort_out_virtual_width(&width, driver);

   if (width <= mode_info.BytesPerScanLine) {
      height = driver->vid_mem / width;
      evilness_flag = FALSE;
   }
   else {                                       /* increase logical width */
      _dpmi_reg.x.ax = 0x4F06;
      _dpmi_reg.x.bx = 0;
      _dpmi_reg.x.cx = width;
      __dpmi_int(0x10, &_dpmi_reg);

      if ((_dpmi_reg.h.ah) || (width != _dpmi_reg.x.bx)) {
	 /* Evil, evil, evil. I really wish I didn't have to do this, but 
	  * some crappy VESA implementations can't handle the set logical 
	  * width call, which Allegro depends on. This register write will 
	  * work on 99% of cards, if VESA lets me down.
	  */
	 if (width != 1024) {
	    strcpy(allegro_error, "VESA function 0x4F06 failed");
	    return NULL;
	 }
	 height = driver->vid_mem / width;
	 _write_vga_register(_crtc, 0x13, width/8);
	 evilness_flag = TRUE;
      }
      else {
	 /* Some VESA drivers don't report the available virtual height 
	  * properly, so we do a sanity check based on the total amount of 
	  * video memory.
	  */
	 height = MIN(_dpmi_reg.x.dx, driver->vid_mem / width);
	 evilness_flag = FALSE;
      }
   }

   if ((width < v_w) || (width < w) || (height < v_h) || (height < h)) {
      strcpy(allegro_error, "Virtual screen size too large");
      return NULL;
   }

   if (vbe2) {
      if (get_pmode_functions(&w1, &w2) != 0) {
	 strcpy(allegro_error, "VBE 2.0 protected mode interface not available");
	 return NULL;
      }
   }

   if (driver->linear) {                        /* make linear bitmap? */
      b = make_linear_bitmap(driver, width, height);
      if (!b) {
	 strcpy(allegro_error, "Can't make linear screen bitmap");
	 return NULL;
      }
   }
   else {                                       /* or make bank switcher */
      b = _make_bitmap(width, height, mode_info.WinASegment*16, driver);
      if (!b)
	 return NULL;

      if (sort_out_vesa_windows(b, w1, w2) != 0) {
	 destroy_bitmap(b);
	 return NULL;
      }
   }

   vesa_xscroll = vesa_yscroll = 0;

   driver->w = b->cr = w;
   driver->h = b->cb = h;

   setup_vesa_desc();

   return b;
}



/* vesa_1_init:
 *  Initialises a VESA 1.x screen mode.
 */
static BITMAP *vesa_1_init(int w, int h, int v_w, int v_h)
{
   return vesa_init(&gfx_vesa_1, FALSE, FALSE, w, h, v_w, v_h);
}



/* vesa_2b_init:
 *  Initialises a VESA 2.0 banked screen mode.
 */
static BITMAP *vesa_2b_init(int w, int h, int v_w, int v_h)
{
   return vesa_init(&gfx_vesa_2b, FALSE, TRUE, w, h, v_w, v_h);
}



/* vesa_2l_init:
 *  Initialises a VESA 2.0 linear framebuffer mode.
 */
static BITMAP *vesa_2l_init(int w, int h, int v_w, int v_h)
{
   return vesa_init(&gfx_vesa_2l, TRUE, TRUE, w, h, v_w, v_h);
}



/* vesa_vsync:
 *  VBE 2.0 vsync routine, needed for cards that don't emulate the VGA 
 *  blanking registers. VBE doesn't provide a vsync function, but we 
 *  can emulate it by changing the display start with the vsync flag set.
 */
static void vesa_vsync()
{
   vesa_scroll(vesa_xscroll, vesa_yscroll);
}



/* vesa_scroll:
 *  Hardware scrolling routine.
 */
static int vesa_scroll(int x, int y)
{
   int ret, seg;
   long a;

   vesa_xscroll = x;
   vesa_yscroll = y;

   if (_pm_vesa_scroller) {            /* use protected mode interface? */
      seg = _mmio_segment ? _mmio_segment : _my_ds();
      a = (x + (y * VIRTUAL_W)) / 4;

      asm (
	 "  pushw %%es ; "
	 "  movw %w1, %%es ; "         /* set the IO segment */
	 "  call *%0 ; "               /* call the VESA function */
	 "  popw %%es "

      :                                /* no outputs */

      : "S" (_pm_vesa_scroller),       /* function pointer in esi */
	"a" (seg),                     /* IO segment in eax */
	"b" (0x80),                    /* mode in ebx */
	"c" (a & 0xFFFF),              /* low word of address in ecx */
	"d" (a >> 16)                  /* high word of address in edx */

      : "memory", "%edi", "%cc"        /* clobbers edi and flags */
      );

      ret = 0;
   }
   else {                              /* use a real mode interrupt call */
      _dpmi_reg.x.ax = 0x4F07;
      _dpmi_reg.x.bx = 0;
      _dpmi_reg.x.cx = x;
      _dpmi_reg.x.dx = y;

      __dpmi_int(0x10, &_dpmi_reg); 
      ret = _dpmi_reg.h.ah;

      _vsync_in();
   }

   return (ret ? -1 : 0); 
}



/* vesa_set_pallete_range:
 *  Uses VESA function #9 (VBE 2.0 only) to set the pallete.
 */
static void vesa_set_pallete_range(PALLETE p, int from, int to, int vsync)
{
   if (_pm_vesa_pallete) {             /* use protected mode interface? */
      int c;
      int mode = (vsync) ? 0x80 : 0;
      int seg = (_mmio_segment) ? _mmio_segment : _my_ds();
      PALLETE tmp;

      /* swap the pallete into the funny order VESA uses */
      for (c=from; c<=to; c++) {
	 tmp[c].r = p[c].b;
	 tmp[c].g = p[c].g;
	 tmp[c].b = p[c].r;
      }

      asm (
	 "  pushw %%ds ; "
	 "  movw %w1, %%ds ; "         /* set the IO segment */
	 "  call *%0 ; "               /* call the VESA function */
	 "  popw %%ds "

      :                                /* no outputs */

      : "S" (_pm_vesa_pallete),        /* function pointer in esi */
	"a" (seg),                     /* IO segment in eax */
	"b" (mode),                    /* mode in ebx */
	"c" (to-from+1),               /* how many colors in ecx */
	"d" (from),                    /* first color in edx */
	"D" (tmp+from)                 /* pallete data pointer in edi */

      : "memory", "%cc"                /* clobbers flags */
      );
   }
   else 
      _vga_set_pallete_range(p, from, to, vsync);
}



/* vesa_exit:
 *  Shuts down the VESA driver.
 */
static void vesa_exit(BITMAP *b)
{
   destroy_bitmap(b);

   _remove_physical_mapping(&lb_linear, &lb_segment);
   _remove_physical_mapping(&mmio_linear, &_mmio_segment);

   if (pm_info) {
      free(pm_info);
      pm_info = NULL;
   }

   _pm_vesa_switcher = _pm_vesa_scroller = _pm_vesa_pallete = NULL;
}

