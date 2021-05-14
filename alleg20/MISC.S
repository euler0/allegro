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
 *      Pallete manipulation, putpixel, lines, etc.
 *
 *      See readme.txt for copyright information.
 */


#include "asmdefs.h"

.text


/* void vsync();
 *  Waits for the monitor to begin a vertical blank period.
 */
.globl _vsync 
   .align 4
_vsync:
   pushl %ebp
   movl %esp, %ebp
   movw $0x3DA, %dx           /* vsync port */
vs_loop1:
   inb %dx, %al
   testb $8, %al
   jnz vs_loop1               /* wait for retrace to end */
vs_loop2:
   inb %dx, %al
   testb $8, %al
   jz vs_loop2                /* wait for retrace to start again */
   movl %ebp, %esp
   popl %ebp
   ret                        /* end of vsync() */




/* void set_color(int index, RGB *p);
 *  Sets the specified pallete entry.
 */
.globl _set_color
   .align 4
_set_color:
   pushl %ebp
   movl %esp, %ebp
   pushl %esi

   movl ARG1, %eax            /* eax = pallete index */
   movl ARG2, %esi            /* esi is RGB data */

   movw $0x3C8, %dx           /* dx = port index */
   cld

   outb %al, %dx              /* output pallete index */
   incw %dx

   outsb                      /* output r */
   outsb                      /* output g */
   outsb                      /* output b */

   popl %esi
   movl %ebp, %esp
   popl %ebp
   ret                        /* end of set_color() */




/* void set_pallete(RGB *p);
 *  Sets the screen pallete (p should be an array of 256 RGB structures).
 */
.globl _set_pallete 
   .align 4
_set_pallete:
   pushl %ebp
   movl %esp, %ebp
   pushl %esi

   call _vsync

   movl ARG1, %esi            /* esi is start of pallete */
   movl $0, %eax              /* eax = pallete position */
   movl $256, %ecx            /* ecx = loop counter */
   movw $0x3C9, %dx           /* dx = port index */
   cld

pal_loop:
   decw %dx
   outb %al, %dx              /* output pallete index */
   incw %dx

   outsb                      /* output r */
   outsb                      /* output g */
   outsb                      /* output b */

   incl %esi
   incl %eax                  /* next pallete entry */
   loop pal_loop

   popl %esi
   movl %ebp, %esp
   popl %ebp
   ret                        /* end of set_pallete() */




/* void get_color(int index, RGB *p);
 *  Retrieves the specified pallete entry.
 */
.globl _get_color
   .align 4
_get_color:
   pushl %ebp
   movl %esp, %ebp
   pushl %edi

   movl ARG1, %eax            /* eax = pallete index */
   movl ARG2, %edi            /* edi is RGB structure */

   movw $0x3C7, %dx
   outb %al, %dx              /* output index */

   movw $0x3C9, %dx

   insb                       /* get r */
   insb                       /* get g */
   insb                       /* get b */

   popl %edi
   movl %ebp, %esp
   popl %ebp
   ret                        /* end of get_color() */




/* void putpixel(BITMAP *bmp, int x, int y, int color);
 *  Draws a pixel onto a bitmap.
 */
.globl _putpixel 
   .align 4
_putpixel:
   pushl %ebp
   movl %esp, %ebp
   pushl %esi
   pushl %ebx
   pushw %es

   xorl %eax, %eax
   xorl %edx, %edx

   movl ARG1, %edx               /* edx = bmp */
   movl ARG2, %ecx               /* ecx = x */
   movl ARG3, %eax               /* eax = y */

   cmpl $0, BMP_CLIP(%edx)       /* test clip flag */
   je putpix_noclip
   cmpl %ecx, BMP_CL(%edx)       /* test cl */
   jg putpix_done
   cmpl %ecx, BMP_CR(%edx)       /* test cr */
   jle putpix_done
   cmpl %eax, BMP_CT(%edx)       /* test ct */
   jg putpix_done
   cmpl %eax, BMP_CB(%edx)       /* test cb */
   jle putpix_done

putpix_noclip:
   movw BMP_SEG(%edx), %es       /* segment selector */
   movl ARG4, %ebx               /* bx = color */

   cmpl $0, __xor
   je putpix_no_xor

   READ_BANK(%esi)               /* read pixel from screen */
   xorb %es:(%eax, %ecx), %bl    /* xor */
   movl ARG3, %eax               /* re-read Y position */

   .align 4, 0x90
putpix_no_xor:
   WRITE_BANK(%esi)              /* select bank */
   movb %bl, %es:(%eax, %ecx)    /* store the pixel */

putpix_done:
   popw %es
   popl %ebx
   popl %esi
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of putpixel() */




/* int getpixel(BITMAP *bmp, int x, int y);
 *  Reads a pixel from a bitmap.
 */
.globl _getpixel 
   .align 4
_getpixel:
   pushl %ebp
   movl %esp, %ebp
   pushl %ebx
   pushw %es

   xorl %eax, %eax
   xorl %edx, %edx
   movl ARG1, %edx               /* edx = bmp */
   movl ARG2, %ecx               /* ecx = x */
   movl ARG3, %eax               /* eax = y */

   testl %ecx, %ecx              /* test that we are on the bitmap */
   jl getpix_skip
   cmpl %ecx, BMP_W(%edx)
   jle getpix_skip
   testl %eax, %eax
   jl getpix_skip
   cmpl %eax, BMP_H(%edx)
   jg getpix_ok

getpix_skip:                     /* return -1 if we are off the edge */
   movl $-1, %eax
   jmp getpix_done

getpix_ok:
   READ_BANK(%ebx)               /* select the bank */

   movw BMP_SEG(%edx), %es       /* segment selector */
   movl %eax, %ebx
   xorl %eax, %eax
   movb %es:(%ebx, %ecx), %al    /* get the pixel */

getpix_done:
   popw %es
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of getpixel() */




/* void hline(BITMAP *bmp, int x1, int y, int x2, int color);
 *  Draws a horizontal line onto a bitmap.
 */
.globl _hline 
   .align 4
_hline:
   pushl %ebp
   movl %esp, %ebp
   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %es

   movl ARG1, %edx               /* edx = bmp */
   movl ARG2, %ebx               /* ebx = x1 */
   movl ARG3, %eax               /* eax = y */
   movl ARG4, %ecx               /* ecx = x2 */
   cmpl %ebx, %ecx
   jge hline_no_xswap
   xchgl %ecx, %ebx

hline_no_xswap:
   cmpl $0, BMP_CLIP(%edx)       /* test bmp->clip */
   je hline_noclip

   cmpl BMP_CT(%edx), %eax       /* test bmp->ct */
   jl hline_done

   cmpl BMP_CB(%edx), %eax       /* test bmp->cb */
   jge hline_done

   cmpl BMP_CL(%edx), %ebx       /* test x1, bmp->cl */
   jge hline_x1_ok
   cmpl BMP_CL(%edx), %ecx       /* test x2, bmp->cl */
   jl hline_done
   movl BMP_CL(%edx), %ebx       /* clip x1 */

hline_x1_ok:
   cmpl BMP_CR(%edx), %ecx       /* test x2, bmp->cr */
   jl hline_noclip
   cmpl BMP_CR(%edx), %ebx       /* test x1, bmp->cr */
   jge hline_done
   movl BMP_CR(%edx), %ecx       /* clip x2 */
   decl %ecx

hline_noclip:
   subl %ebx, %ecx               /* loop counter */
   incl %ecx

   movw BMP_SEG(%edx), %es       /* segment selector */

   cmpl $0, __xor
   je hline_no_xor

   movl %ebx, %edi               /* xor version of the hline */

   .align 4, 0x90
hline_xor_loop:
   movl ARG3, %eax               /* read a pixel */
   READ_BANK(%esi)
   movb %es:(%eax, %edi), %bl

   xorb ARG5, %bl                /* xor */

   movl ARG3, %eax
   WRITE_BANK(%esi)
   movb %bl, %es:(%eax, %edi)    /* and write the pixel */

   incl %edi
   loop hline_xor_loop
   jmp hline_done

   .align 4, 0x90
hline_no_xor:                    /* regular hline drawer */
   WRITE_BANK(%edi)              /* select bank */
   movl %eax, %edi
   addl %ebx, %edi               /* get the pixel address */

   movb ARG5, %dl                /* get color */
   movb %dl, %dh
   shrdl $16, %edx, %eax
   movw %dx, %ax                 /* get four copies of the color */
   cld

   testl $1, %edi                /* are we word aligned? */
   jz hline_w_aligned
   stosb                         /* if not, copy a pixel */
   decl %ecx
hline_w_aligned:
   testl $2, %edi                /* are we long aligned? */
   jz hline_l_aligned
   subl $2, %ecx
   jl hline_oops
   stosw                         /* if not, copy a word */
hline_l_aligned:
   movw %cx, %dx                 /* store low bits */
   shrl $2, %ecx                 /* for long copy */
   jz hline_no_long
   rep ; stosl                   /* do some 32 bit copies */
hline_no_long:
   testw $2, %dx
   jz hline_no_word
   stosw                         /* do we need a 16 bit copy? */
hline_no_word:
   testw $1, %dx
   jz hline_done
   stosb                         /* do we need a byte copy? */

hline_done:
   popw %es
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret

hline_oops:                      /* fix up ecx for short line? */
   addl $2, %ecx
   jmp hline_l_aligned




/* void vline(BITMAP *bmp, int x, int y1, int y2, int color);
 *  Draws a vertical line onto a bitmap.
 */
.globl _vline 
   .align 4
_vline:
   pushl %ebp
   movl %esp, %ebp
   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %es 

   movl ARG1, %edx               /* edx = bmp */
   movl ARG2, %edi               /* edi = x */
   movl ARG3, %esi               /* esi = y1 */
   movl ARG4, %ecx               /* ecx = y2 */
   cmpl %esi, %ecx
   jge vline_no_xswap
   xchgl %ecx, %esi

vline_no_xswap:
   cmpl $0, BMP_CLIP(%edx)       /* test bmp->clip */
   je vline_noclip

   cmpl BMP_CL(%edx), %edi       /* test bmp->cl */
   jl vline_done

   cmpl BMP_CR(%edx), %edi       /* test bmp->cr */
   jge vline_done

   cmpl BMP_CT(%edx), %esi       /* test y1, bmp->ct */
   jge vline_y1_ok
   cmpl BMP_CT(%edx), %ecx       /* test y2, bmp->ct */
   jl vline_done
   movl BMP_CT(%edx), %esi       /* clip y1 */

vline_y1_ok:
   cmpw BMP_CB(%edx), %ecx       /* test y2, bmp->cb */
   jl vline_noclip
   cmpw BMP_CB(%edx), %esi       /* test y1, bmp->cb */
   jge vline_done
   movl BMP_CB(%edx), %ecx       /* clip y2 */
   decl %ecx

vline_noclip:
   subl %esi, %ecx               /* loop counter */
   incl %ecx

   movw BMP_SEG(%edx), %es       /* load segment */
   movl %ecx, ARG4               /* store loop counter */

   cmpl $0, __xor
   je vline_no_xor

   .align 4, 0x90
vline_xor_loop:                  /* xor mode vline drawer */
   movl %esi, %eax
   READ_BANK(%ecx)               /* select read bank */

   movb %es:(%eax, %edi), %bl    /* read pixel */
   xorb ARG5, %bl                /* xor */

   movl %esi, %eax
   WRITE_BANK(%ecx)              /* select write bank */

   movb %bl, %es:(%eax, %edi)    /* write pixel */

   incl %esi 
   decl ARG4
   jg vline_xor_loop             /* loop */
   jmp vline_done

   .align 4, 0x90
vline_no_xor:                    /* normal vline drawer */
   movl ARG5, %ebx               /* get color */

   .align 4, 0x90
vline_loop:
   movl %esi, %eax
   WRITE_BANK(%ecx)              /* select bank */

   movb %bl, %es:(%eax, %edi)    /* write pixel */

   incl %esi 
   decl ARG4
   jg vline_loop                 /* loop */

vline_done:
   popw %es 
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of vline() */




/* void _fill_putpix(BITMAP *bmp, int x, int y)
 *  Adds a pixel to the fill table, for drawing filled polygons and circles.
 */
.globl __fill_putpix 
   .align 4
__fill_putpix:
   pushl %ebp
   movl %esp, %ebp

   movl ARG3, %eax               /* eax = y */
   movl ARG1, %edx               /* edx = bmp */

   cmpl BMP_CT(%edx), %eax       /* test ct */
   jl fputpix_done

   cmpl BMP_CB(%edx), %eax       /* test cb */
   jge fputpix_done

   movl __fill_array, %edx
   leal (%edx, %eax, 8), %edx

   movl ARG2, %eax               /* eax = x */
   cmpl (%edx), %eax             /* if (x < _fill_array[y].lpos) */
   jae fputpix_no_lpos

   movl %eax, (%edx)             /* set lpos */

fputpix_no_lpos:
   cmpl 4(%edx), %eax            /* if (x > _fill_array[y].rpos) */
   jle fputpix_done

   movl %eax, 4(%edx)            /* set rpos */

fputpix_done:
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _fill_putpix() */




/* Template for fixed point line drawing routines. Uses a fixed point line
 * algorithm to calculate points from X1, Y1 to X2, Y2, calling the macro
 * PUT_PIXEL(name, x, y) for each point. Clobbers %eax, %ebx, %ecx and %edx, 
 * but * %esi and %edi are untouched.
 */
#define DO_LINE(name)                                                        \
   movl X2, %eax                    /* eax = x2 */                         ; \
   subl X1, %eax                    /* x2 - x1 */                          ; \
   jge name##_dx_pve                                                       ; \
   negl %eax                        /* eax = abs(dx) */                    ; \
name##_dx_pve:                                                             ; \
   movl Y2, %ebx                    /* ebx = y2 */                         ; \
   subl Y1, %ebx                    /* x2 - x1 */                          ; \
   jge name##_dy_pve                                                       ; \
   negl %ebx                        /* ebx = abs(dy) */                    ; \
name##_dy_pve:                                                             ; \
   cmpl %eax, %ebx                  /* if abs(dy) > abs(dx) */             ; \
   jg name##_y_driven                                                      ; \
   testl %eax, %eax                 /* skip if zero length */              ; \
   jz name##_done                                                          ; \
									   ; \
   /* x-driven fixed point line draw */                                    ; \
   movl X1, %edx                    /* edx = x1 */                         ; \
   movl Y1, %ebx                    /* ebx = y1 */                         ; \
   movl X2, %ecx                    /* ecx = x2 */                         ; \
   movl Y2, %eax                    /* eax = y2 */                         ; \
   subl %edx, %ecx                  /* ecx = x2 - x1 = counter */          ; \
   jg name##_x_noswap                                                      ; \
									   ; \
   negl %ecx                        /* make count positive */              ; \
   xchgl %eax, %ebx                 /* swap y values */                    ; \
   movl X2, %edx                                                           ; \
   movl %edx, X1                    /* swap start x value */               ; \
									   ; \
name##_x_noswap:                                                           ; \
   sall $20, %eax                   /* make y2 fixed point */              ; \
   sall $20, %ebx                   /* make y1 fixed point */              ; \
   subl %ebx, %eax                  /* eax = y2 - y1 = dy */               ; \
   cdq                              /* make edx:eax */                     ; \
   idivl %ecx                       /* eax = fixed point y change */       ; \
   incl %ecx                                                               ; \
   movl %ecx, X2                                                           ; \
   movl %eax, Y1                                                           ; \
									   ; \
   .align 4, 0x90                                                          ; \
name##_x_loop:                                                             ; \
   movl %ebx, %edx                  /* calculate y coord */                ; \
   sarl $20, %edx                                                          ; \
									   ; \
   PUT_PIXEL(x, X1, %edx)           /* output a pixel */                   ; \
									   ; \
   addl Y1, %ebx                    /* y += dy */                          ; \
   incl X1                          /* x++ */                              ; \
   decl X2                                                                 ; \
   jg name##_x_loop                 /* more? */                            ; \
   jmp name##_done                                                         ; \
									   ; \
   /* y-driven fixed point line draw */                                    ; \
   .align 4, 0x90                                                          ; \
name##_y_driven:                                                           ; \
   movl Y1, %edx                    /* edx = y1 */                         ; \
   movl X1, %ebx                    /* ebx = x1 */                         ; \
   movl Y2, %ecx                    /* ecx = y2 */                         ; \
   movl X2, %eax                    /* eax = x2 */                         ; \
   subl %edx, %ecx                  /* ecx = y2 - y1 = counter */          ; \
   jg name##_y_noswap                                                      ; \
									   ; \
   negl %ecx                        /* make count positive */              ; \
   xchgl %eax, %ebx                 /* swap x values */                    ; \
   movl Y2, %edx                                                           ; \
   movl %edx, Y1                    /* swap start y value */               ; \
									   ; \
name##_y_noswap:                                                           ; \
   sall $20, %eax                   /* make x2 fixed point */              ; \
   sall $20, %ebx                   /* make x1 fixed point */              ; \
   subl %ebx, %eax                  /* eax = x2 - x1 = dx */               ; \
   cdq                              /* make edx:eax */                     ; \
   idivl %ecx                       /* eax = fixed point x change */       ; \
   incl %ecx                                                               ; \
   movl %ecx, Y2                                                           ; \
   movl %eax, X1                                                           ; \
									   ; \
   .align 4, 0x90                                                          ; \
name##_y_loop:                                                             ; \
   movl %ebx, %edx                                                         ; \
   sarl $20, %edx                   /* calculate x coord */                ; \
									   ; \
   PUT_PIXEL(y, %edx, Y1)           /* output a pixel */                   ; \
									   ; \
   addl X1, %ebx                    /* x += dx */                          ; \
   incl Y1                          /* y++ */                              ; \
   decl Y2                                                                 ; \
   jg name##_y_loop                 /* more? */                            ; \
									   ; \
name##_done:




/* void do_line(BITMAP *bmp, int x1, y1, x2, y2, int d, void (*proc)())
 *  Calculates all the points along a line between x1, y1 and x2, y2, 
 *  calling the supplied function for each one. This will be passed a 
 *  copy of the bmp parameter, the x and y position, and a copy of the 
 *  d parameter (so do_line() can be used with putpixel()).
 */
.globl _do_line 

   #define BMP    ARG1
   #define X1     ARG2
   #define Y1     ARG3
   #define X2     ARG4
   #define Y2     ARG5
   #define D      ARG6
   #define PROC   ARG7

   #define PUT_PIXEL(name, x, y)                                             \
      pushl D                       /* data item */                        ; \
      pushl y                       /* y coord */                          ; \
      pushl x                       /* x coord */                          ; \
      pushl %esi                    /* bitmap */                           ; \
      call *%edi                    /* call the draw routine */            ; \
      addl $16, %esp


   .align 4
_do_line:
   pushl %ebp
   movl %esp, %ebp
   pushl %edi
   pushl %esi
   pushl %ebx

   movl BMP, %esi                /* esi = bmp */
   movl PROC, %edi               /* edi = proc */

   DO_LINE(line)

   popl %ebx
   popl %esi
   popl %edi
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of the line() */



#undef X1 
#undef Y1 
#undef X2 
#undef Y2 
#undef PUT_PIXEL



/* void _fill_line(int x1, y1, x2, y2, min, max);
 *  Draw a line from x1, y1, to x2, y2, storing all the points in the
 *  fill array. The min and max parameters are the y clipping values.
 */
.globl __fill_line 

   #define X1     ARG1
   #define Y1     ARG2
   #define X2     ARG3
   #define Y2     ARG4
   #define MIN    ARG5
   #define MAX    ARG6

   #define PUT_PIXEL(name, x, y)                                             \
      movl y, %esi                  /* esi = y */                          ; \
									   ; \
      cmpl MIN, %esi                /* test ct */                          ; \
      jl name##_fputpix_done                                               ; \
									   ; \
      cmpl MAX, %esi                /* test cb */                          ; \
      jge name##_fputpix_done                                              ; \
									   ; \
      movl __fill_array, %edi                                              ; \
      leal (%edi, %esi, 8), %edi                                           ; \
									   ; \
      movl x, %esi                  /* esi = x */                          ; \
      cmpl (%edi), %esi             /* if (x < _fill_array[y].lpos) */     ; \
      jae name##_fputpix_no_lpos                                           ; \
									   ; \
      movl %esi, (%edi)             /* set lpos */                         ; \
									   ; \
   name##_fputpix_no_lpos:                                                 ; \
      cmpl 4(%edi), %esi            /* if (x > _fill_array[y].rpos) */     ; \
      jle name##_fputpix_done                                              ; \
									   ; \
      movl %esi, 4(%edi)            /* set rpos */                         ; \
									   ; \
   name##_fputpix_done:


   .align 4
__fill_line:
   pushl %ebp
   movl %esp, %ebp
   pushl %edi
   pushl %esi
   pushl %ebx

   movl BMP, %esi                /* esi = bmp */
   movl PROC, %edi               /* edi = proc */

   DO_LINE(fill_line)

   popl %ebx
   popl %esi
   popl %edi
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _fill_line() */

