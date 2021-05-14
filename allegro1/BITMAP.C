/*
		  //  /     /     ,----  ,----.  ,----.  ,----.
		/ /  /     /     /      /    /  /    /  /    /
	      /  /  /     /     /___   /       /____/  /    /
	    /---/  /     /     /      /  __   /\      /    /
	  /    /  /     /     /      /    /  /  \    /    /
	/     /  /____ /____ /____  /____/  /    \  /____/

	Low Level Game Routines (version 1.0)

	Bitmap routines (clear, blit, textout, sprites)

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


void show(bitmap)
BITMAP *bitmap;
{
   asm {
      push ds

      lds si, bitmap
      mov cx, 14[si]          // cx = bitmap->size low word (max size is 64K)
      shr cx, 1               // halve the counter for word blits
      lds si, 22[si]          // pointer to start of data

      mov ax, 0xA000
      mov es, ax
      mov di, 0               // pointer to screen memory
      
      cld
      rep movsw               // move the data

      pop ds
   }
}



void clear(bitmap)
BITMAP *bitmap;
{
   asm {
      les di, bitmap
      mov cx, es:14[di]       // cx = bitmap->size low word (max size is 64K)
      shr cx, 1               // halve the counter for word copies
      les di, es:22[di]       // pointer to start of data
      mov ax, 0

      cld
      rep stosw               // store a bunch of zeros
   }
}



void blit(source, dest, source_x, source_y, dest_x, dest_y, width, height)
BITMAP *source, *dest;
short source_x, source_y;
short dest_x, dest_y;
short width, height;
{
   if (source_x >= source->w)
      return;

   if (source_y >= source->h)
      return;

   if (dest_x >= dest->cr)
      return;
      
   if (dest_y >= dest->cb)
      return;

   if (source_x < 0) {
      width += source_x;
      dest_x -= source_x;
      source_x = 0;
   }

   if (source_y < 0) {
      height += source_y;
      dest_y -= source_y;
      source_y = 0;
   }

   if ((width + source_x) > source->w)
      width = source->w - source_x;

   if ((height + source_y) > source->h)
      height = source->h - source_y;

   if (dest_x < dest->cl) {
      dest_x -= dest->cl;
      width += dest_x;
      source_x -= dest_x;
      dest_x = dest->cl; 
   }

   if (dest_y < dest->ct) {
      dest_y -= dest->ct;
      height += dest_y;
      source_y -= dest_y;
      dest_y = dest->ct;
   }

   if ((width + dest_x) > dest->cr)
      width = dest->cr - dest_x;

   if ((height + dest_y) > dest->cb)
      height = dest->cb - dest_y;

   if ((width <= 0) || (height <= 0))
      return;

   if ((source == dest) &&
       ((source_y < dest_y) ||
	((source_y == dest_y) && (source_x < dest_x)))) {

		     /* Reverse blit routine */
      asm {
	 push ds

	 lds si, source
	 mov ax, [si]            // ax = source width
	 sub ax, width           // ax = source gap between lines
	 mov bx, source_y
	 add bx, height
	 dec bx                  // last line = source_y + height - 1
	 shl bx, 2
	 lds si, 22[si][bx]      // pointer to start of source line
	 add si, source_x
	 add si, width
	 dec si                  // position in the source line

	 les di, dest
	 mov dx, es:[di]         // dx = dest width
	 sub dx, width           // dx = dest gap between lines
	 mov bx, dest_y
	 add bx, height
	 dec bx                  // last line = dest_y + height - 1
	 shl bx, 2
	 les di, es:22[di][bx]   // pointer to start of dest line
	 add di, dest_x
	 add di, width
	 dec di                  // position in the dest line

	 mov bx, height          // y loop counter
	 std                     // backwards copy

      blit_b_loop:
	 mov cx, width           // x loop counter
	 rep movsb               // I cant be bothered messing about with
				 // word copies - reverse blit is rare anyway

	 sub si, ax
	 sub di, dx              // move on to the next lines

	 dec bx
	 jg blit_b_loop          // another line?

	 cld
	 pop ds
      }
   }
   else {            /* Forwards blit routine */
      asm {
	 push ds

	 lds si, source
	 mov ax, [si]            // ax = source width
	 sub ax, width           // ax = source gap between lines
	 mov bx, source_y
	 shl bx, 2
	 lds si, 22[si][bx]      // pointer to start of source line
	 add si, source_x        // position in the source line

	 les di, dest
	 mov dx, es:[di]         // dx = dest width
	 sub dx, width           // dx = dest gap between lines
	 mov bx, dest_y
	 shl bx, 2
	 les di, es:22[di][bx]   // pointer to start of dest line
	 add di, dest_x          // position in the dest line

	 mov bx, height          // y loop counter
	 cld                     // forward copy

      blit_f_loop:
	 mov cx, width           // x loop counter
	 test si, 1              // are we on an odd boundary?
	 jz f_loop_aligned
      
	 movsb
	 dec cx                  // if so, copy a pixel

      f_loop_aligned:
	 shr cx, 1               // halve the counter for word aligned copy
	 jz f_no_word_copy

	 rep movsw               // copy a bunch of words

      f_no_word_copy:
	 jnc f_line_done
	 movsb                   // do we need an extra pixel?

      f_line_done:
	 add si, ax
	 add di, dx              // move on to the next lines

	 dec bx
	 jg blit_f_loop          // another line?

	 pop ds
      }
   }
}



#endif      /* ifdef BORLAND */


/* djgpp stuff in bmp.s */



FONT _font =
{
  {
   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   /* ' ' */
   { 0x18, 0x3c, 0x3c, 0x18, 0x18, 0x00, 0x18, 0x00 },   /* '!' */
   { 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00 },   /* '"' */
   { 0x6c, 0x6c, 0xfe, 0x6c, 0xfe, 0x6c, 0x6c, 0x00 },   /* '#' */
   { 0x18, 0x7e, 0xc0, 0x7c, 0x06, 0xfc, 0x18, 0x00 },   /* '$' */
   { 0x00, 0xc6, 0xcc, 0x18, 0x30, 0x66, 0xc6, 0x00 },   /* '%' */
   { 0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x76, 0x00 },   /* '&' */
   { 0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00 },   /* ''' */
   { 0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00 },   /* '(' */
   { 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00 },   /* ')' */
   { 0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00 },   /* '*' */
   { 0x00, 0x18, 0x18, 0x7e, 0x18, 0x18, 0x00, 0x00 },   /* '+' */
   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30 },   /* ',' */
   { 0x00, 0x00, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00 },   /* '-' */
   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00 },   /* '.' */
   { 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x00 },   /* '/' */
   { 0x7c, 0xce, 0xde, 0xf6, 0xe6, 0xc6, 0x7c, 0x00 },   /* '0' */
   { 0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x00 },   /* '1' */
   { 0x78, 0xcc, 0x0c, 0x38, 0x60, 0xcc, 0xfc, 0x00 },   /* '2' */
   { 0x78, 0xcc, 0x0c, 0x38, 0x0c, 0xcc, 0x78, 0x00 },   /* '3' */
   { 0x1c, 0x3c, 0x6c, 0xcc, 0xfe, 0x0c, 0x1e, 0x00 },   /* '4' */
   { 0xfc, 0xc0, 0xf8, 0x0c, 0x0c, 0xcc, 0x78, 0x00 },   /* '5' */
   { 0x38, 0x60, 0xc0, 0xf8, 0xcc, 0xcc, 0x78, 0x00 },   /* '6' */
   { 0xfc, 0xcc, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00 },   /* '7' */
   { 0x78, 0xcc, 0xcc, 0x78, 0xcc, 0xcc, 0x78, 0x00 },   /* '8' */
   { 0x78, 0xcc, 0xcc, 0x7c, 0x0c, 0x18, 0x70, 0x00 },   /* '9' */
   { 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x00 },   /* ':' */
   { 0x00, 0x18, 0x18, 0x00, 0x00, 0x18, 0x18, 0x30 },   /* ';' */
   { 0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x00 },   /* '<' */
   { 0x00, 0x00, 0x7e, 0x00, 0x7e, 0x00, 0x00, 0x00 },   /* '=' */
   { 0x60, 0x30, 0x18, 0x0c, 0x18, 0x30, 0x60, 0x00 },   /* '>' */
   { 0x3c, 0x66, 0x0c, 0x18, 0x18, 0x00, 0x18, 0x00 },   /* '?' */
   { 0x7c, 0xc6, 0xde, 0xde, 0xdc, 0xc0, 0x7c, 0x00 },   /* '@' */
   { 0x30, 0x78, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0x00 },   /* 'A' */
   { 0xfc, 0x66, 0x66, 0x7c, 0x66, 0x66, 0xfc, 0x00 },   /* 'B' */
   { 0x3c, 0x66, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x00 },   /* 'C' */
   { 0xf8, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0xf8, 0x00 },   /* 'D' */
   { 0xfe, 0x62, 0x68, 0x78, 0x68, 0x62, 0xfe, 0x00 },   /* 'E' */
   { 0xfe, 0x62, 0x68, 0x78, 0x68, 0x60, 0xf0, 0x00 },   /* 'F' */
   { 0x3c, 0x66, 0xc0, 0xc0, 0xce, 0x66, 0x3a, 0x00 },   /* 'G' */
   { 0xcc, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0xcc, 0x00 },   /* 'H' */
   { 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 },   /* 'I' */
   { 0x1e, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78, 0x00 },   /* 'J' */
   { 0xe6, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0xe6, 0x00 },   /* 'K' */
   { 0xf0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xfe, 0x00 },   /* 'L' */
   { 0xc6, 0xee, 0xfe, 0xfe, 0xd6, 0xc6, 0xc6, 0x00 },   /* 'M' */
   { 0xc6, 0xe6, 0xf6, 0xde, 0xce, 0xc6, 0xc6, 0x00 },   /* 'N' */
   { 0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00 },   /* 'O' */
   { 0xfc, 0x66, 0x66, 0x7c, 0x60, 0x60, 0xf0, 0x00 },   /* 'P' */
   { 0x7c, 0xc6, 0xc6, 0xc6, 0xd6, 0x7c, 0x0e, 0x00 },   /* 'Q' */
   { 0xfc, 0x66, 0x66, 0x7c, 0x6c, 0x66, 0xe6, 0x00 },   /* 'R' */
   { 0x7c, 0xc6, 0xe0, 0x78, 0x0e, 0xc6, 0x7c, 0x00 },   /* 'S' */
   { 0xfc, 0xb4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 },   /* 'T' */
   { 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xfc, 0x00 },   /* 'U' */
   { 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00 },   /* 'V' */
   { 0xc6, 0xc6, 0xc6, 0xc6, 0xd6, 0xfe, 0x6c, 0x00 },   /* 'W' */
   { 0xc6, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0xc6, 0x00 },   /* 'X' */
   { 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x30, 0x78, 0x00 },   /* 'Y' */
   { 0xfe, 0xc6, 0x8c, 0x18, 0x32, 0x66, 0xfe, 0x00 },   /* 'Z' */
   { 0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00 },   /* '[' */
   { 0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x02, 0x00 },   /* '\' */
   { 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00 },   /* ']' */
   { 0x10, 0x38, 0x6c, 0xc6, 0x00, 0x00, 0x00, 0x00 },   /* '^' */
   { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff },   /* '_' */
   { 0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00 },   /* '`' */
   { 0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00 },   /* 'a' */
   { 0xe0, 0x60, 0x60, 0x7c, 0x66, 0x66, 0xdc, 0x00 },   /* 'b' */
   { 0x00, 0x00, 0x78, 0xcc, 0xc0, 0xcc, 0x78, 0x00 },   /* 'c' */
   { 0x1c, 0x0c, 0x0c, 0x7c, 0xcc, 0xcc, 0x76, 0x00 },   /* 'd' */
   { 0x00, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00 },   /* 'e' */
   { 0x38, 0x6c, 0x64, 0xf0, 0x60, 0x60, 0xf0, 0x00 },   /* 'f' */
   { 0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8 },   /* 'g' */
   { 0xe0, 0x60, 0x6c, 0x76, 0x66, 0x66, 0xe6, 0x00 },   /* 'h' */
   { 0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00 },   /* 'i' */
   { 0x0c, 0x00, 0x1c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78 },   /* 'j' */
   { 0xe0, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0xe6, 0x00 },   /* 'k' */
   { 0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00 },   /* 'l' */
   { 0x00, 0x00, 0xcc, 0xfe, 0xfe, 0xd6, 0xd6, 0x00 },   /* 'm' */
   { 0x00, 0x00, 0xb8, 0xcc, 0xcc, 0xcc, 0xcc, 0x00 },   /* 'n' */
   { 0x00, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0x78, 0x00 },   /* 'o' */
   { 0x00, 0x00, 0xdc, 0x66, 0x66, 0x7c, 0x60, 0xf0 },   /* 'p' */
   { 0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0x1e },   /* 'q' */
   { 0x00, 0x00, 0xdc, 0x76, 0x62, 0x60, 0xf0, 0x00 },   /* 'r' */
   { 0x00, 0x00, 0x7c, 0xc0, 0x70, 0x1c, 0xf8, 0x00 },   /* 's' */
   { 0x10, 0x30, 0xfc, 0x30, 0x30, 0x34, 0x18, 0x00 },   /* 't' */
   { 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x76, 0x00 },   /* 'u' */
   { 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00 },   /* 'v' */
   { 0x00, 0x00, 0xc6, 0xc6, 0xd6, 0xfe, 0x6c, 0x00 },   /* 'w' */
   { 0x00, 0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00 },   /* 'x' */
   { 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8 },   /* 'y' */
   { 0x00, 0x00, 0xfc, 0x98, 0x30, 0x64, 0xfc, 0x00 },   /* 'z' */
   { 0x1c, 0x30, 0x30, 0xe0, 0x30, 0x30, 0x1c, 0x00 },   /* '{' */
   { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00 },   /* '|' */
   { 0xe0, 0x30, 0x30, 0x1c, 0x30, 0x30, 0xe0, 0x00 },   /* '}' */
   { 0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }    /* '~' */
  }
};


FONT *font = &_font;


short _textmode = 0;


void textmode(mode)
short mode;
{
   if ((mode >= 0) && (mode < PAL_SIZE))
      _textmode = mode;
   else
      if (mode < 0)
	 _textmode = -1;
      else
	 _textmode = 0;
}



#ifdef BORLAND         /* borland version of textout() */


void textout(bmp, font, str, x, y, color)
BITMAP *bmp;
FONT *font;
char *str;
short x, y;
short color;
{
   short tgap, height, lgap, rgap; /* for the clipping routines */
   short char_w;                   /* width of the current character */
   short c;                        /* y loop counter */
   short tm = _textmode;           /* so we can read it after changing ds */

   if (bmp->clip) {
      tgap = bmp->ct - y;
      if (tgap >= 8)
	 return;
      if (tgap < 0)
	 tgap = 0;

      y += tgap;

      height = bmp->cb - y;
      if (height <= 0)
	 return;
      if (height > (8 - tgap))
	 height = 8 - tgap;

      lgap = bmp->cl - x;
      if (lgap < 0)
	 lgap = 0;

      while (lgap >= 8) {     /* skip any characters that are off the left */
	 if (!*(str++))
	    return;
	 x += 8;
	 lgap -= 8;
      }
      
      rgap = bmp->cr - x;
      if (rgap < 0)
	 return;
   }
   else {
      tgap = lgap = 0;
      height = 8;
      rgap = 0x7fff;
   }

   asm {
      push ds
      jmp char_loop_start

   char_loop:                          // for each character...
      cmp al, 126                      // the char will already be in al
      ja char_no_good
      sub al, 32                       // convert to table offset
      jl char_no_good
      cbw
      jmp char_sorted

   char_no_good:
      mov ax, 0                        // oops - not ASCII
   
   char_sorted:
      les si, font
      shl ax, 3
      add si, ax
      add si, tgap                     // es:[si] = position in font bitmap

      mov ax, rgap
      cmp ax, 0
      jle finished                     // have we gone off the right?
      cmp ax, 8
      jle rgap_ok
      mov ax, 8                        // dont want chars wider than 8!

   rgap_ok:
      mov char_w, ax
      mov bx, 0
      jmp y_loop_start

   y_loop:                             // for each y...
      add bx, y                        // c will already be in bx
      shl bx, 2
      lds di, bmp
      lds di, 22[di][bx]
      add di, x                        // ds:[di] = bmp->line[y+c] + x

      mov dl, es:[si]                  // dl = bit mask
      inc si

      mov cx, lgap
      cmp cx, 0
      jz no_lgap                       // do we need to clip on the left?

      shl dl, cl                       // shift the mask
      add di, cx                       // move the screen position
      neg cx

   no_lgap:
      add cx, char_w                   // cx = x loop counter
      jle no_x_loop

      mov ax, color                    // ax = text color
      mov bx, tm                       // bx = background color

   x_loop:                             // for each x...
      shl dl, 1                        // shift the mask
      jc put_bit

      cmp bx, 0
      jl put_done
      mov [di], bl                     // draw background pixel
      jmp put_done
      
   put_bit: 
      mov [di], al                     // draw a pixel

   put_done:
      inc di
      loop x_loop                      // and loop

   no_x_loop:
      mov bx, c                        // increment loop counter
      inc bx
   y_loop_start:
      mov c, bx
      cmp bx, height
      jl y_loop

   y_loop_done:
      mov lgap, 0                      // sort out a load of variables
      sub rgap, 8
      add x, 8

      inc word ptr str                 // move on to the next character
   char_loop_start:
      lds bx, str                      // read a char into al
      mov al, [bx]
      cmp al, 0
      jz finished
      jmp char_loop

   finished:
      pop ds
   }
}


#endif         /* ifdef BORLAND */



SPRITE *create_sprite(flags, width, height)
short flags;
short width, height;
{
   register SPRITE *sprite;
   short size;

   size = width * height;           /* easy: one byte per pixel */

   sprite = malloc(sizeof(BITMAP) + size);
   if (!sprite)
      return NULL;

   sprite->flags = flags;
   sprite->w = width;
   sprite->h = height;

   return sprite;
}



void destroy_sprite(sprite)
SPRITE *sprite;
{
   if (sprite)
      free(sprite);
}



#ifdef BORLAND 


void get_sprite(sprite, bmp, x, y)
SPRITE *sprite;
BITMAP *bmp;
short x;
short y;
{
   short yend, swidth, sgap;

   asm {
      push ds

      cld
      les di, sprite          // es:[di] = sprite pointer
      mov bx, y
      mov ax, bx
      add ax, es:4[di]        // sprite->h
      mov yend, ax

      mov dx, es:2[di]        // dx = sprite->w
      mov swidth, dx
      mov sgap, 0
      add di, 6               // start of sprite data

      mov ax, x
      cmp ax, 0
      jae l_ok

      neg ax                  // clip the left
      mov sgap, ax
      add di, ax
      sub dx, ax
      jl finished
      mov ax, 0

   l_ok:
      mov cx, ax
      add cx, swidth
      lds si, bmp
      cmp cx, [si]            // bmp->w
      jl y_loop_start

      sub cx, [si]            // clip the right
      sub dx, cx
      jl finished
      add sgap, cx
      jmp y_loop_start

   y_loop:
      lds si, bmp
      cmp bx, 2[si]           // bx is y pos, compare with bmp->h
      jae bad_y
      cmp bx, 0
      jl bad_y

      shl bx, 2               // offset into list of pointers
      lds si, 22[si][bx]      // pointer to start of the line
      add si, ax              // position within the line
      shr bx, 2               // put y value back

      mov cx, dx              // cx = sprite->w
      rep movsb               // get the data
      add di, sgap
      jmp line_done

   bad_y:                     // if the line is not on the bitmap
      add di, swidth

   line_done:
      inc bx                  // increment y loop counter
   y_loop_start:
      cmp bx, yend            // loop?
      jl y_loop

   finished:
      pop ds
   }
}



void drawsprite(bmp, sprite, x, y)
BITMAP *bmp;
SPRITE *sprite;
short x;
short y;
{
   short tgap, lgap;              /* top and left gaps */
   short w, h;                    /* width and height */

   asm {
      push ds
      
      lds si, sprite             // ds:[si] = sprite pointer
      les di, bmp                // es:[di] = bitmap pointer

      cmp word ptr es:4[di], 0   // test bmp->clip
      jz no_clip

      mov ax, es:10[di]          // bmp->ct
      sub ax, y
      jge tgap_ok
      mov ax, 0
   tgap_ok: 
      mov tgap, ax

      mov bx, 4[si]              // sprite->h
      mov cx, es:12[di]          // bmp->cb
      sub cx, y
      cmp cx, bx                 // check bottom clipping
      jg height_ok
      mov bx, cx
   height_ok:
      sub bx, ax                 // height -= tgap
      jle dont_draw
      mov h, bx

      mov ax, es:6[di]           // bmp->cl
      sub ax, x
      jge lgap_ok
      mov ax, 0
   lgap_ok: 
      mov lgap, ax

      mov bx, 2[si]              // sprite->w
      mov cx, es:8[di]           // bmp->cr
      sub cx, x
      cmp cx, bx                 // check bottom clipping
      jg width_ok
      mov bx, cx
   width_ok:
      sub bx, ax                 // width -= lgap
      jle dont_draw
      mov w, bx
      jmp clip_done

   dont_draw:
      jmp finished

   no_clip:
      mov tgap, 0
      mov lgap, 0
      mov ax, 2[si]
      mov w, ax                  // w = sprite->w
      mov ax, 4[si]
      mov h, ax                  // h = sprite->h

   clip_done:
      test word ptr [si], SPRITE_MASKED
      jnz masked_draw


      // --------------------------
      // opaque sprite draw routine
      // --------------------------

      mov bx, w
      mov dx, es:[di]            // bmp->w
      sub dx, bx                 // dx = bitmap gap

      mov cx, 2[si]              // sprite->w
      mov ax, cx
      sub ax, bx                 // ax = sprite gap

      mov bx, tgap
   o_tgap_loop:
      or bx, bx
      jz o_no_tgap
      add si, cx                 // + width
      dec bx
      jmp o_tgap_loop

   o_no_tgap:
      add si, lgap
      add si, 6                  // get position in sprite

      mov bx, y
      add bx, tgap
      shl bx, 2
      les di, es:22[di][bx]      // get position in bitmap
      add di, x
      add di, lgap

      mov bx, h                  // y loop counter
      cld

      shr w, 1                   // convert byte count to word count
      jz o_no_word_loop          // which draw should we use?
      jnc o_no_byte_loop

   o_word_byte_loop:             // for an odd number of bytes
      mov cx, w                  // x loop counter
      rep movsw                  // copy the words
      movsb                      // and copy a byte
      add si, ax                 // skip some bytes
      add di, dx
      dec bx
      jg o_word_byte_loop        // another line?
      jmp finished

   o_no_byte_loop:               // fast routine for even number of bytes
      mov cx, w                  // x loop counter
      rep movsw                  // copy the words
      add si, ax                 // skip some bytes
      add di, dx
      dec bx
      jg o_no_byte_loop          // another line?
      jmp finished

   o_no_word_loop:               // in case there are no words at all
      movsb                      // copy a byte
      add si, ax                 // skip some bytes
      add di, dx
      dec bx
      jg o_no_word_loop          // another line?
      jmp finished


      // --------------------------
      // masked sprite draw routine
      // --------------------------

   masked_draw:
      mov bx, w
      mov dx, es:[di]            // bmp->w
      sub dx, bx                 // dx = bitmap gap

      mov cx, 2[si]              // sprite->w
      mov ax, cx
      sub ax, bx                 // ax = sprite gap

      mov bx, tgap
   m_tgap_loop:
      or bx, bx
      jz m_no_tgap
      add si, cx                 // + width
      dec bx
      jmp m_tgap_loop

   m_no_tgap:
      add si, lgap
      add si, 6                  // get position in sprite 

      mov bx, y
      add bx, tgap
      shl bx, 2
      les di, es:22[di][bx]      // get position in bitmap
      add di, x
      add di, lgap

      mov bx, ax                 // sprite gap
      cld

      shr w, 1                   // convert byte count to word count
      jz m_no_word_loop          // which draw should we use?
      jnc m_no_byte_loop

   m_word_byte_loop:             // for an odd number of bytes
      mov cx, w                  // x loop counter
   word_byte_x_loop:
      mov ax, [si]               // read two bytes
      add si, 2
      or al, al                  // test the first
      jz word_byte_skip1
      mov es:[di], al            // write the first
   word_byte_skip1:
      inc di
      or ah, ah                  // test the second
      jz word_byte_skip2
      mov es:[di], ah            // write the second
   word_byte_skip2:
      inc di
      loop word_byte_x_loop      // x loop
      mov al, [si]               // read one extra byte
      inc si
      or al, al                  // test it
      jz word_byte_skip
      mov es:[di], al            // write it
   word_byte_skip:
      inc di
      add si, bx                 // skip some bytes
      add di, dx
      dec h
      jg m_word_byte_loop        // another line?
      jmp finished

   m_no_byte_loop:               // fast routine for even number of bytes
      mov cx, w                  // x loop counter
   no_byte_x_loop:
      mov ax, [si]               // read two bytes
      add si, 2
      or al, al                  // test the first
      jz no_byte_skip1
      mov es:[di], al            // write the first
   no_byte_skip1:
      inc di
      or ah, ah                  // test the second
      jz no_byte_skip2
      mov es:[di], ah            // write the second
   no_byte_skip2:
      inc di
      loop no_byte_x_loop        // x loop
      add si, bx                 // skip some bytes
      add di, dx
      dec h
      jg m_no_byte_loop          // another line?
      jmp finished

   m_no_word_loop:               // in case there are no words at all
      mov al, [si]               // read the byte
      inc si
      or al, al                  // test it
      jz no_word_skip
      mov es:[di], al            // write it
   no_word_skip:
      inc di
      add si, bx                 // skip some bytes
      add di, dx
      dec h
      jg m_no_word_loop          // another line?

   finished:
      pop ds
   }
}


#endif      /* ifdef BORLAND */


