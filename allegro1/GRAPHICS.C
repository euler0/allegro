/*
		  //  /     /     ,----  ,----.  ,----.  ,----.
		/ /  /     /     /      /    /  /    /  /    /
	      /  /  /     /     /___   /       /____/  /    /
	    /---/  /     /     /      /  __   /\      /    /
	  /    /  /     /     /      /    /  /  \    /    /
	/     /  /____ /____ /____  /____/  /    \  /____/

	Low Level Game Routines (version 1.0)

	Various graphics routines

	See allegro.txt for instructions and copyright conditions.
   
	By Shawn Hargreaves,
	1 Salisbury Road,
	Market Drayton,
	Shropshire,
	England TF9 1AJ
	email slh100@tower.york.ac.uk (until 1996)
*/


#include "stdlib.h"

#include "allegro.h"

#ifdef BORLAND
#include "alloc.h"
#endif

PALLETE black_pallete;

_RGB _black_rgb = { 0, 0, 0 };

PALLETE desktop_pallete = {
   { 63, 63, 63 },   { 63, 0,  0 },
   { 0,  63, 0 },    { 63, 63, 0 },
   { 0,  0,  63 },   { 63, 0,  63 },
   { 0,  63, 63 },   { 16, 16, 16 },
   { 31, 31, 31 },   { 63, 31, 31 },
   { 31, 63, 31 },   { 63, 63, 31 },
   { 31, 31, 63 },   { 63, 31, 63 },
   { 31, 63, 63 },   { 0,  0,  0 }
};

short _setup_fill_array(short size);
void _fill_line(BITMAP *bmp, short x1, short y1, short x2, short y2);
void _fill_init(BITMAP *bmp);
void _fill_putpix(BITMAP *bmp, short x, short y);
void _fill_finish(BITMAP *bmp, short color);

BITMAP *screen = NULL;

typedef struct {
   short lpos, rpos;
} FILL_STRUCT;             /* for drawing filled polygons and circles */

FILL_STRUCT *_fill_array = NULL;
short _fill_array_size = 0;



#ifdef BORLAND


void vsync()
{
   asm {
      mov dx, 0x3DA     // vsync port
   vs_loop1:
      in al, dx
      and al, 8
      jnz vs_loop1      // wait for retrace to end
   vs_loop2:
      in al, dx
      and al, 8
      jz vs_loop2       // wait for retrace to start again
   }
}



void set_pallete(p)
PALLETE p;
{
   vsync();

   asm {
      push ds
      lds si, p         // ds:[si] is start of pallete
      mov ax, 0         // ax = pallete position
      mov cx, 128       // cx = loop counter
      mov dx, 0x3C9     // dx = port index
      cld

   pal_loop:
      dec dx
      out dx, al        // output pallete index
      inc dx

      outsb             // output r
      outsb             // output g
      outsb             // output b

      inc ax            // next pallete entry
      loop pal_loop
   }

   vsync();
   
   asm {
      mov ax, 128       // ax = pallete position
      mov cx, ax        // cx = loop counter
      mov dx, 0x3C9     // dx = port index

   pal_loop2:
      dec dx
      out dx, al        // output pallete index
      inc dx

      outsb             // output r
      outsb             // output g
      outsb             // output b

      inc ax            // next pallete entry
      loop pal_loop2
      pop ds
   }
}



void get_pallete(p)
PALLETE p;
{
   asm {
      les di, p         // es:[di] is start of pallete
      mov al, 0         // al = pallete position
      mov cx, 256       // cx = loop counter
      cld

   pal_loop:
      mov dx, 0x3C7
      out dx, al        // output index
      mov dx, 0x3C9

      insb              // get r
      insb              // get g
      insb              // get b

      inc al            // next pallete entry
      loop pal_loop
   }
}


#endif   /* ifdef BORLAND */


/* djgpp vsync, getpallete and setpallete are in misc.s */



void fade_in(p, speed)
PALLETE p;
short speed;
{
   PALLETE temp;
   register short i;
   register short c;

   set_pallete(black_pallete);

   for (c=0; c<64; c+=speed) {             /* do the fade */
      for (i=0; i < PAL_SIZE; i++) { 
	 temp[i].r = ((int)p[i].r * (int)c) / 64;
	 temp[i].g = ((int)p[i].g * (int)c) / 64;
	 temp[i].b = ((int)p[i].b * (int)c) / 64;
      }
      set_pallete(temp);
   }

   set_pallete(p);                     /* just in case... */
}



void fade_out(speed)
short speed;
{
   PALLETE temp, p;
   register short i;
   register short c;

   get_pallete(p);

   for (c=64; c>0; c-=speed) {            /* do the fade */
      for (i=0; i < PAL_SIZE; i++) { 
	 temp[i].r = ((int)p[i].r * (int)c) / 64;
	 temp[i].g = ((int)p[i].g * (int)c) / 64;
	 temp[i].b = ((int)p[i].b * (int)c) / 64;
      }
      set_pallete(temp);
   }

   set_pallete(black_pallete);            /* just in case... */
}



#define A_RGBTOPAL(x)   { \
			   x >>= 4; \
			   if (x & 1) \
			      x |= 0x10; \
			   x >>= 1; \
			}



_ATARI_RGB _atari_rgbtopal(rgb)
RGB rgb;
{
   register short r = rgb.r;
   register short g = rgb.g;
   register short b = rgb.b;

   A_RGBTOPAL(r);
   A_RGBTOPAL(g);
   A_RGBTOPAL(b);

   return (r << 8) + (g << 4) + b;
}



#define A_PALTORGB(x)   { \
			   x <<= 1; \
			   if (x & 0x10) { \
			      x &= 0x0f; \
			      x++; \
			   } \
			   temp = x; \
			   x <<= 4; \
			   x += temp; \
			}



RGB _atari_paltorgb(c)
_ATARI_RGB c;
{
   RGB ret;
   register short r, b, g;
   register short temp;

   b = c & 0x0f;
   c >>= 4;
   g = c & 0x0f;
   c >>= 4;
   r = c & 0x0f;

   A_PALTORGB(r);
   A_PALTORGB(g);
   A_PALTORGB(b);

   ret.r = r;
   ret.g = g;
   ret.b = b;

   return ret;
}



_PC_RGB _pc_rgbtopal(rgb)
RGB rgb;
{
   _RGB ret;

   ret.r = rgb.r >> 2;
   ret.g = rgb.g >> 2;
   ret.b = rgb.b >> 2;

   return ret;
}



RGB _pc_paltorgb(rgb)
_PC_RGB rgb;
{
   RGB ret;

   ret.r = rgb.r << 2;
   ret.g = rgb.g << 2;
   ret.b = rgb.b << 2;

   return ret;
}



short _setup_fill_array(size)
short size;
{
   short *temp;

   if (size > _fill_array_size) {
      temp = malloc(sizeof(FILL_STRUCT) * size);
      if (!temp)
	 return FALSE;
      if (_fill_array)
	 free(_fill_array);
      _fill_array = (FILL_STRUCT *)temp;
      _fill_array_size = size;
   }
   return TRUE;
}



BITMAP *create_bitmap(width, height)
short width, height;
{
   register BITMAP *bitmap;
   register short i;

   if (!_setup_fill_array(height))     /* do we need a bigger fill array? */
      return NULL;

   bitmap = malloc(sizeof(BITMAP) + (sizeof(char *) * height));
   if (!bitmap)
      return NULL;

#ifdef BORLAND
   bitmap->size = (long)width * (long)height;
   if (bitmap->size < 0x10000L)
      bitmap->dat = malloc((short)bitmap->size);
   else
      bitmap->dat = NULL;
#endif

#ifdef GCC
   bitmap->size = (long)width * (long)height;
   bitmap->dat = malloc(bitmap->size);
   bitmap->seg = 0;     /* means _go32_my_ds() */
#endif

   if (!bitmap->dat) {
      free(bitmap);
      return NULL;
   }

   bitmap->w = bitmap->cr = width;
   bitmap->h = bitmap->cb = height;
   bitmap->clip = TRUE;
   bitmap->cl = bitmap->ct = 0;

   bitmap->line[0] = bitmap->dat;
   for (i=1; i<height; i++)
      bitmap->line[i] = bitmap->line[i-1] + width;

   return bitmap;
}



void destroy_bitmap(bitmap)
BITMAP *bitmap;
{
   if (bitmap) {
      if (bitmap->dat)
	 free(bitmap->dat);
      free(bitmap);
   }
}



void set_clip(bitmap, x1, y1, x2, y2)
register BITMAP *bitmap;
register short x1, y1, x2, y2;
{
   register short t;
   
   if ((x1==0) && (y1==0) && (x2==0) && (y2==0)) {
      bitmap->clip = FALSE;
      bitmap->cl = bitmap->ct = 0;
      bitmap->cr = bitmap->w;
      bitmap->cb = bitmap->h;
      return;
   }
   
   if (x2 < x1) {
      t = x1;
      x1 = x2;
      x2 = t;
   }
   
   if (y2 < y1) {
      t = y1;
      y1 = y2;
      y2 = t;
   }

   x2++;
   y2++;

   bitmap->clip = TRUE;
   bitmap->cl = MID(0,x1,bitmap->w-1);
   bitmap->ct = MID(0,y1,bitmap->h-1);
   bitmap->cr = MID(0,x2,bitmap->w);
   bitmap->cb = MID(0,y2,bitmap->h);
}



void polyline(bmp, n, points, color)
register BITMAP *bmp;
register short n;
register short *points;
register short color;
{
   register short i;

   n = (n << 1) - 2;

   for (i=0; i < n; i += 2)
      line(bmp, points[i], points[i+1], points[i+2], points[i+3], color);
}



void rect(bmp, x1, y1, x2, y2, color)
register BITMAP *bmp;
register short x1, y1, x2, y2;
register short color;
{
   hline(bmp, x1, y1, x2, color);
   hline(bmp, x1, y2, x2, color);
   vline(bmp, x1, y1, y2, color);
   vline(bmp, x2, y1, y2, color);
}



void rectfill(bmp, x1, y1, x2, y2, color)
BITMAP *bmp;
short x1, y1, x2, y2;
short color;
{
   short t;

   if (y1 > y2) {
      t = y1;
      y1 = y2;
      y2 = t;
   }

   do {
      hline(bmp, x1, y1++, x2, color);
   } while (y1 <= y2);
}



void triangle(bmp, x1, y1, x2, y2, x3, y3, color)
BITMAP *bmp;
short x1, y1, x2, y2, x3, y3;
short color;
{
   _fill_init(bmp);
   _fill_line(bmp, x1, y1, x2, y2);
   _fill_line(bmp, x2, y2, x3, y3);
   _fill_line(bmp, x3, y3, x1, y1);
   _fill_finish(bmp, color);
}



void circle(bmp, x, y, radius, color)
BITMAP *bmp;
short x, y;
short radius;
short color;
{
   short cx = 0;
   short cy = radius;
   short d = 3 - (radius << 1);

   do {
      putpixel(bmp, x + cx, y + cy, color);
      putpixel(bmp, x + cx, y - cy, color);
      putpixel(bmp, x - cx, y - cy, color);
      putpixel(bmp, x - cx, y + cy, color);
      putpixel(bmp, x + cy, y + cx, color);
      putpixel(bmp, x + cy, y - cx, color);
      putpixel(bmp, x - cy, y - cx, color);
      putpixel(bmp, x - cy, y + cx, color);
      putpixel(bmp, x - cy, y + cx, color);

      if (d < 0)
	 d += (cx << 2) + 6;
      else {
	 d += ((cx - cy) << 2) + 10;
	 cy--;
      }

      cx++;

   } while (cx <= cy);
}



void circlefill(bmp, x, y, radius, color)
BITMAP *bmp;
short x, y;
short radius;
short color;
{
   short cx = 0;
   short cy = radius;
   short d = 3 - (radius << 1);

   _fill_init(bmp);

   do {
      _fill_putpix(bmp, x + cx, y + cy);
      _fill_putpix(bmp, x + cx, y - cy);
      _fill_putpix(bmp, x - cx, y - cy);
      _fill_putpix(bmp, x - cx, y + cy);
      _fill_putpix(bmp, x + cy, y + cx);
      _fill_putpix(bmp, x + cy, y - cx);
      _fill_putpix(bmp, x - cy, y - cx);
      _fill_putpix(bmp, x - cy, y + cx);
      _fill_putpix(bmp, x - cy, y + cx);

      if (d < 0)
	 d += (cx << 2) + 6;
      else {
	 d += ((cx - cy) << 2) + 10;
	 cy--;
      }

      cx++;

   } while (cx <= cy);

   _fill_finish(bmp, color);
}


 
void _fill_init(bmp)
BITMAP *bmp;
{
   short y;
   
   for (y=bmp->ct; y<=bmp->cb; y++) {
      _fill_array[y].lpos = 0x7fff;
      _fill_array[y].rpos = -0x7fff;
   }
}



#ifdef BORLAND        /* Borland C specific routines */



void putpixel(bmp, x, y, color)
BITMAP *bmp;
short x, y;
short color;
{
   asm {
      push ds
      mov ax, x
      mov bx, y
      mov cx, color
      lds si, bmp

      cmp word ptr 4[si], 0   // test bmp->clip
      je putpix_noclip

      cmp ax, 6[si]           // test bmp->cl
      jl putpix_done

      cmp ax, 8[si]           // test bmp->cr
      jge putpix_done

      cmp bx, 10[si]          // test bmp->ct
      jl putpix_done

      cmp bx, 12[si]          // test bmp->cb
      jge putpix_done

   putpix_noclip:
      shl bx, 2               // offset into list of pointers
      lds si, 22[si][bx]      // pointer to start of the line
      add si, ax              // position within the line
      mov [si], cl            // move the byte

   putpix_done:
      pop ds
   }
}



short getpixel(bmp, x, y)
BITMAP *bmp;
short x, y;
{
   if ((x < 0) || (x >= bmp->w) || (y < 0) || (y >= bmp->h))
      return -1;

   return (*(bmp->line[y]+x));
}



void line(bmp, x1, y1, x2, y2, color)
BITMAP *bmp;
short x1, y1, x2, y2;
short color;
{
   short clip;
   short xdiff, ydiff, direction;
   short t, t2;

   if (x1 == x2) {
      vline(bmp, x1, y1, y2, color);
      return;
   }

   if (y1 == y2) {
      hline(bmp, x1, y1, x2, color);
      return;
   }

   clip = bmp->clip;
   if (clip)
      if ((x1 >= bmp->cl) && (x2 >= bmp->cl) &&
	  (x1 < bmp->cr) && (x2 < bmp->cr) &&
	  (y1 >= bmp->ct) && (y2 >= bmp->ct) &&
	  (y1 < bmp->cb) && (y2 < bmp->cb))
	 clip = FALSE;

   asm {
      push ds
      lds si, bmp             // ds:[si] = bitmap pointer
      mov ax, x1              // ax = x1
      mov bx, y1              // bx = y1
      mov cx, x2
      sub cx, ax              // cx = xdiff
      mov xdiff, cx
      jge xdiff_pve
      neg cx                  // cx = abs(xdiff)
   xdiff_pve:
      mov dx, y2
      sub dx, bx              // dx = ydiff
      mov ydiff, dx
      jge ydiff_pve
      neg dx                  // dx = abs(ydiff)

   ydiff_pve:
      cmp cx, dx              // compare abs(xdiff) with abs (ydiff)
      jge x_driven 
      jmp y_driven

   x_driven:
      // x driven line draw routine

      mov cx, xdiff           // cx = xdiff
      mov dx, ydiff           // dx = ydiff
      
      cmp cx, 0
      jge x_xdiff_ok          // do we need to swap x?

      xchg ax, x2
      xchg bx, y2
      neg cx
      neg dx

   x_xdiff_ok:
      cmp dx, 0
      jge x_ydiff_pve         // do we need to swap y?

      neg dx
      mov direction, -1
      jmp x_ydiff_done
   
   x_ydiff_pve:
      mov direction, 1
   
   x_ydiff_done:
      mov xdiff, cx
      mov ydiff, dx
      shl dx, 1
      mov t, dx               // t = ydiff << 1
      sub dx, cx              // dx = d = t - xdiff
      neg cx
      add cx, ydiff
      shl cx, 1
      mov t2, cx              // t2 = (ydiff - xdiff) << 1
      mov cx, color           // cx = color to draw

   x_loop:
      cmp clip, 0
      je x_noclip

      cmp ax, 6[si]           // test bmp->cl
      jl x_skip

      cmp ax, 8[si]           // test bmp->cr
      jge x_skip

      cmp bx, 10[si]          // test bmp->ct
      jl x_skip

      cmp bx, 12[si]          // test bmp->cb
      jge x_skip

   x_noclip:
      shl bx, 2               // offset into list of pointers
      les di, 22[si][bx]      // pointer to start of the line
      add di, ax              // position within the line
      mov es:[di], cl         // move the byte
      shr bx, 2               // put y back again

   x_skip:
      cmp dx, 0               // test d
      jl x_no_ychange

      add dx, t2
      add bx, direction       // change y
      jmp x_y_done

   x_no_ychange:
      add dx, t

   x_y_done:
      inc ax                  // change x
      dec xdiff
      jge x_loop
      jmp done

   y_driven:
      // else the y variable is in control

      mov cx, xdiff           // cx = xdiff
      mov dx, ydiff           // dx = ydiff
      
      cmp dx, 0
      jge y_ydiff_ok          // do we need to swap y?

      xchg ax, x2
      xchg bx, y2
      neg cx
      neg dx

   y_ydiff_ok:
      cmp cx, 0
      jge y_xdiff_pve         // do we need to swap x?

      neg cx
      mov direction, -1
      jmp y_xdiff_done
   
   y_xdiff_pve:
      mov direction, 1
   
   y_xdiff_done:
      mov xdiff, cx
      mov ydiff, dx
      shl cx, 1
      mov t, cx               // t = xdiff << 1
      sub cx, dx              // cx = d = t - ydiff
      neg dx
      add dx, xdiff
      shl dx, 1
      mov t2, dx              // t2 = (xdiff - ydiff) << 1
      mov dx, color           // dx = color to draw

   y_loop:
      cmp clip, 0
      je y_noclip

      cmp ax, 6[si]           // test bmp->cl
      jl y_skip

      cmp ax, 8[si]           // test bmp->cr
      jge y_skip

      cmp bx, 10[si]          // test bmp->ct
      jl y_skip

      cmp bx, 12[si]          // test bmp->cb
      jge y_skip

   y_noclip:
      shl bx, 2               // offset into list of pointers
      les di, 22[si][bx]      // pointer to start of the line
      add di, ax              // position within the line
      mov es:[di], dl         // move the byte
      shr bx, 2               // put y back again

   y_skip:
      cmp cx, 0               // test d
      jl y_no_xchange

      add cx, t2
      add ax, direction       // change x
      jmp y_x_done

   y_no_xchange:
      add cx, t

   y_x_done:
      inc bx                  // change y
      dec ydiff
      jge y_loop

   done:
      pop ds
   }
}



void vline(bmp, x, y1, y2, color)
BITMAP *bmp;
short x, y1, y2;
short color;
{
   short t;

   if (y1 > y2) {
      t = y1;
      y1 = y2;
      y2 = t;
   }

   if (bmp->clip) {
      if ((x < bmp->cl) || (x >= bmp->cr))
	 return;
      if (y1 < bmp->ct) {
	 if (y2 < bmp->ct)
	    return;
	 y1 = bmp->ct;
      }
      if (y2 >= bmp->cb) {
	 if (y1 >= bmp->cb)
	    return;
	 y2 = bmp->cb-1;
      }
   }
   
   asm {
      push ds
      mov bx, y1
      mov cx, y2
      sub cx, bx              // loop counter
      inc cx
      shl bx, 2               // offset into list of pointers
      lds di, bmp
      mov dx, [di]            // bitmap width
      lds di, 22[di][bx]      // pointer to start of the line
      add di, x               // position within the line
      mov ax, color

   vline_loop:
      mov [di], al            // put the pixel
      add di, dx              // next line
      loop vline_loop

      pop ds
   }
}



void hline(bmp, x1, y, x2, color)
BITMAP *bmp;
short x1, y, x2;
short color;
{
   asm {
      push ds
      mov bx, y               // bx = y
      mov cx, x2              // cx = x2
      mov dx, x1              // dx = x1
      lds si, bmp             // ds:[si] = bmp
      cmp cx, dx
      jge no_xswap
      xchg cx, dx

   no_xswap:
      cmp word ptr 4[si], 0   // test bmp->clip
      je line_noclip

      cmp bx, 10[si]          // test bmp->ct
      jl line_done

      cmp bx, 12[si]          // test bmp->cb
      jge line_done

      cmp dx, 6[si]           // test x1, bmp->cl
      jge x1_ok

      cmp cx, 6[si]           // test x2, bmp->cl
      jl line_done

      mov dx, 6[si]           // clip x1

   x1_ok:
      cmp cx, 8[si]           // test x2, bmp->cr
      jl line_noclip

      cmp dx, 8[si]           // test x1, bmp->cr
      jge line_done

      mov cx, 8[si]           // clip x2
      dec cx

   line_noclip:
      sub cx, dx              // loop counter
      inc cx
      shl bx, 2               // offset into list of pointers
      les di, 22[si][bx]      // pointer to start of the line
      add di, dx              // position within the line
      mov ax, color
      mov ah, al              // so we can store two pixels at a time
      cld

      test di, 1              // are we on an odd boundary?
      jz aligned
      
      stosb
      dec cx                  // if so, copy a pixel

   aligned:
      shr cx, 1               // halve the counter for word aligned copy
      jz no_word_copy

      rep stosw               // copy a bunch of words

   no_word_copy:
      jnc line_done
      stosb                   // do we need an extra pixel?

   line_done:
      pop ds
   }
}



void _fill_line(BITMAP *bmp, short x1, short y1, short x2, short y2)
{
   short xdiff, ydiff, direction;
   short t, t2;

   asm {
      push ds
      lds si, dword ptr _fill_array
      les di, bmp             // es:[di] = bitmap pointer
      mov ax, x1              // ax = x1
      mov bx, y1              // bx = y1
      mov cx, x2
      sub cx, ax              // cx = xdiff
      mov xdiff, cx
      jge xdiff_pve
      neg cx                  // cx = abs(xdiff)
   xdiff_pve:
      mov dx, y2
      sub dx, bx              // dx = ydiff
      mov ydiff, dx
      jge ydiff_pve
      neg dx                  // dx = abs(ydiff)

   ydiff_pve:
      cmp cx, dx              // compare abs(xdiff) with abs (ydiff)
      jge x_driven 
      jmp y_driven

   x_driven:
      // x driven line draw routine

      mov cx, xdiff           // cx = xdiff
      mov dx, ydiff           // dx = ydiff
      
      cmp cx, 0
      jge x_xdiff_ok          // do we need to swap x?

      xchg ax, x2
      xchg bx, y2
      neg cx
      neg dx

   x_xdiff_ok:
      cmp dx, 0
      jge x_ydiff_pve         // do we need to swap y?

      neg dx
      mov direction, -1
      jmp x_ydiff_done
   
   x_ydiff_pve:
      mov direction, 1
   
   x_ydiff_done:
      mov xdiff, cx
      mov ydiff, dx
      shl dx, 1
      mov t, dx               // t = ydiff << 1
      sub dx, cx              // dx = d = t - xdiff
      neg cx
      add cx, ydiff
      shl cx, 1
      mov t2, cx              // t2 = (ydiff - xdiff) << 1
      mov cx, xdiff           // cx = loop counter

   x_loop:
      cmp bx, es:10[di]       // test ct
      jl x_skip
      cmp bx, es:12[di]       // test cb
      jge x_skip

      shl bx, 2
      cmp ax, [si][bx]        // if (x < _fill_array[y].lpos)
      jae x_no_lpos

      mov [si][bx], ax        // set lpos

   x_no_lpos:
      cmp ax, 2[si][bx]       // if (x > _fill_array[y].rpos)
      jle x_no_rpos

      mov 2[si][bx], ax       // set rpos

   x_no_rpos:
      shr bx, 2

   x_skip:
      cmp dx, 0               // test d
      jl x_no_ychange

      add dx, t2
      add bx, direction       // change y
      jmp x_y_done

   x_no_ychange:
      add dx, t

   x_y_done:
      inc ax                  // change x
      dec cx
      jge x_loop
      jmp done

   y_driven:
      // else the y variable is in control

      mov cx, xdiff           // cx = xdiff
      mov dx, ydiff           // dx = ydiff
      
      cmp dx, 0
      jge y_ydiff_ok          // do we need to swap y?

      xchg ax, x2
      xchg bx, y2
      neg cx
      neg dx

   y_ydiff_ok:
      cmp cx, 0
      jge y_xdiff_pve         // do we need to swap x?

      neg cx
      mov direction, -1
      jmp y_xdiff_done
   
   y_xdiff_pve:
      mov direction, 1
   
   y_xdiff_done:
      mov xdiff, cx
      mov ydiff, dx
      shl cx, 1
      mov t, cx               // t = xdiff << 1
      sub cx, dx              // cx = d = t - ydiff
      neg dx
      add dx, xdiff
      shl dx, 1
      mov t2, dx              // t2 = (xdiff - ydiff) << 1
      mov dx, cx              // dx = d
      mov cx, ydiff

   y_loop:
      cmp bx, es:10[di]       // test ct
      jl y_skip
      cmp bx, es:12[di]       // test cb
      jge y_skip

      shl bx, 2
      cmp ax, [si][bx]        // if (x < _fill_array[y].lpos)
      jae y_no_lpos

      mov [si][bx], ax        // set lpos

   y_no_lpos:
      cmp ax, 2[si][bx]       // if (x > _fill_array[y].rpos)
      jle y_no_rpos

      mov 2[si][bx], ax       // set rpos

   y_no_rpos:
      shr bx, 2

   y_skip:
      cmp dx, 0               // test d
      jl y_no_xchange

      add dx, t2
      add ax, direction       // change x
      jmp y_x_done

   y_no_xchange:
      add dx, t

   y_x_done:
      inc bx                  // change y
      dec cx
      jge y_loop

   done:
      pop ds
   }
}



void _fill_putpix(bmp, x, y)
BITMAP *bmp;
short x, y;
{
   asm {
      push ds
      mov ax, y               // ax = y
      les bx, bmp             // es:[bx] = bitmap pointer
      cmp ax, es:10[bx]       // test ct
      jl getout
      cmp ax, es:12[bx]       // test cb
      jge getout

      lds bx, dword ptr _fill_array
      shl ax, 2
      add bx, ax              // ds:[bx] = _fill_array[y]

      mov ax, x               // if (x < _fill_array[y].lpos)
      cmp ax, [bx]
      jae no_lpos

      mov [bx], ax            // set lpos

   no_lpos:
      add bx, 2               // if (x > _fill_array[y].rpos)
      cmp ax, [bx]
      jle getout 

      mov [bx], ax            // set rpos

   getout:
      pop ds
   }
}



void _fill_finish(bmp, color)
BITMAP *bmp;
short color;
{
   FILL_STRUCT *temp = _fill_array;    // put it on stack so we can change ds

   asm {
      push ds
      lds si, bmp             // ds:[si] = bitmap pointer
      mov bx, 10[si]          // bx = y = bmp->ct
      mov ax, color
      mov ah, al              // so we can store two pixels at a time
      cld

   y_loop:
      les di, temp            // es:[di] = _fill_array
      shl bx, 2
      mov dx, es:[di][bx]     // dx = x1
      mov cx, es:2[di][bx]    // cx = x2
      cmp cx, dx              // do we need to draw this line?
      jl no_line

      cmp word ptr 4[si], 0   // test bmp->clip
      je line_noclip

      cmp dx, 6[si]           // test x1, bmp->cl
      jge x1_ok

      cmp cx, 6[si]           // test x2, bmp->cl
      jl no_line

      mov dx, 6[si]           // clip x1

   x1_ok:
      cmp cx, 8[si]           // test x2, bmp->cr
      jl line_noclip

      cmp dx, 8[si]           // test x1, bmp->cr
      jge no_line

      mov cx, 8[si]           // clip x2
      dec cx

   line_noclip:
      sub cx, dx              // loop counter
      inc cx
      les di, 22[si][bx]      // pointer to start of the line
      add di, dx              // position within the line

      test di, 1              // are we on an odd boundary?
      jz aligned
      
      stosb
      dec cx                  // if so, copy a pixel

   aligned:
      shr cx, 1               // halve the counter for word aligned copy
      jz no_word_copy

      rep stosw               // copy a bunch of words

   no_word_copy:
      jnc no_line
      stosb                   // do we need an extra pixel?

   no_line:
      shr bx, 2               // restore y value
      inc bx                  // y++
      cmp bx, 12[si]          // test bmp->cb
      jl y_loop               // another line?

      pop ds 
   }
}


#endif      /* ifdef BORLAND */



/* djgpp specific routines are in graph.s */


