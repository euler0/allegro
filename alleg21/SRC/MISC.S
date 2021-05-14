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
 *      Joystick polling, pallete manipulation, linear putpixel, lines, etc.
 *
 *      See readme.txt for copyright information.
 */


#include "asmdefs.h"

.text


/* int _poll_joystick(int *x, int *y, int *x2, int *y2, int poll_mask);
 *  Polls the joystick to read the axis position. Returns raw position
 *  values, zero for success, non-zero if no joystick found.
 */
.globl __poll_joystick
   .align 4
__poll_joystick:
   pushl %ebp
   movl %esp, %ebp
   subl $16, %esp                         /* four local variables */
   pushl %ebx

   #define POLL_1       -4(%ebp)
   #define POLL_2       -8(%ebp)
   #define POLL_3       -12(%ebp)
   #define POLL_4       -16(%ebp)

   movl $0, POLL_1
   movl $0, POLL_2
   movl $0, POLL_3
   movl $0, POLL_4

   movw $0x201, %dx           /* joystick port */
   movl $100000, %ecx         /* loop counter in ecx */

   outb %al, %dx              /* write to joystick port */
   jmp poll_loop

   .align 4
poll_loop:
   inb %dx, %al               /* read joystick port */
   movl %eax, %ebx 

   shrl $1, %ebx              /* test x axis bit */
   adcl $0, POLL_1 
   shrl $1, %ebx              /* test y axis bit */
   adcl $0, POLL_2 
   shrl $1, %ebx              /* test stick 2 x axis bit */
   adcl $0, POLL_3
   shrl $1, %ebx              /* test stick 2 y axis bit */
   adcl $0, POLL_4

   testb ARG5, %al            /* repeat? */
   loopnz poll_loop

   cmpl $100000, POLL_1       /* check for timeout */
   jge poll_error

   cmpl $100000, POLL_2
   jge poll_error

   testb $4, ARG5             /* poll joystick 2 x axis? */
   jz poll_y2 

   cmpl $100000, POLL_3       /* check for timeout */
   jge poll_error

   .align 4
poll_y2:
   testb $8, ARG5             /* poll joystick 2 y axis? */
   jz poll_ok

   cmpl $100000, POLL_4
   jge poll_error

   .align 4
poll_ok:                      /* return 0 on success */
   movl $0, %eax
   jmp poll_joystick_done

   .align 4
poll_error:                   /* return -1 on error */
   movl $-1, %eax

poll_joystick_done:
   movl ARG1, %edx            /* return the results */
   movl POLL_1, %ecx
   movl %ecx, (%edx)

   movl ARG2, %edx 
   movl POLL_2, %ecx
   movl %ecx, (%edx)

   movl ARG3, %edx 
   movl POLL_3, %ecx
   movl %ecx, (%edx)

   movl ARG4, %edx 
   movl POLL_4, %ecx
   movl %ecx, (%edx)

   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                        /* end of _poll_joystick() */




/* void _vga_set_pallete_range(PALLETE p, int from, int to, int vsync);
 *  Sets part of the VGA pallete (p should be an array of 256 RGB structures).
 */
.globl __vga_set_pallete_range
   .align 4
__vga_set_pallete_range:
   pushl %ebp
   movl %esp, %ebp
   pushl %esi

   cmpl $0, ARG4              /* test vsync flag */
   jz set_pallete_no_vsync

   movl $0x3DA, %edx          /* vsync port */

vs_loop1:                     /* wait for retrace to end */
   inb %dx, %al
   testb $8, %al
   jnz vs_loop1 

   cmpl $0, __timer_use_retrace
   je vs_loop2

   movl _retrace_count, %eax
vs_synced_loop:               /* wait for timer retrace interrupt */
   cmpl _retrace_count, %eax
   je vs_synced_loop
   jmp set_pallete_no_vsync

vs_loop2:                     /* wait for retrace to start again */
   inb %dx, %al
   testb $8, %al
   jz vs_loop2 

set_pallete_no_vsync:
   movl ARG2, %eax            /* eax = start position */
   movl %eax, %esi            /* how many entries to skip */
   shll $2, %esi              /* 4 bytes per RGB struct */
   addl ARG1, %esi            /* esi is start of pallete */
   movl ARG3, %ecx            /* ecx = end position */
   movl $0x3C9, %edx          /* edx = port index */
   cld

set_pal_loop:
   decl %edx
   outb %al, %dx              /* output pallete index */
   incl %edx

   outsb                      /* output r */
   outsb                      /* output g */
   outsb                      /* output b */

   incl %esi                  /* skip padding byte */
   incl %eax                  /* next pallete entry */
   cmpl %eax, %ecx
   jge set_pal_loop

   popl %esi
   movl %ebp, %esp
   popl %ebp
   ret                        /* end of _vga_set_pallete_range() */




/* void _linear_putpixel(BITMAP *bmp, int x, int y, int color);
 *  Draws a pixel onto a linear bitmap.
 */
.globl __linear_putpixel 
   .align 4
__linear_putpixel:
   pushl %ebp
   movl %esp, %ebp
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

   cmpl $DRAW_SOLID, __drawing_mode
   je putpix_solid_mode          /* solid draw? */

   cmpl $DRAW_XOR, __drawing_mode
   je putpix_xor_mode            /* XOR? */

   LOOKUP_PATTERN_POS(%ecx, %eax, %edx)
   movb (%eax, %ecx), %bl        /* read pixel from pattern bitmap */

   movl ARG1, %edx               /* reload clobbered registers */
   movl ARG2, %ecx 
   movl ARG3, %eax 

   cmpl $DRAW_COPY_PATTERN, __drawing_mode
   je putpix_solid_mode          /* draw the pattern pixel? */

   testb %bl, %bl                /* test the pattern pixel */
   jz putpix_zero_pattern

   movl ARG4, %ebx               /* if set, draw color */
   jmp putpix_solid_mode

   .align 4, 0x90
putpix_zero_pattern:
   cmpl $DRAW_MASKED_PATTERN, __drawing_mode
   je putpix_done                /* skip zero pixels in masked mode */
   jmp putpix_solid_mode         /* draw zero pixels in solid mode */

   .align 4, 0x90
putpix_xor_mode:
   READ_BANK()                   /* read pixel from screen */
   xorb %es:(%eax, %ecx), %bl    /* XOR */
   movl ARG3, %eax               /* re-read Y position */

   .align 4, 0x90
putpix_solid_mode:
   WRITE_BANK()                  /* select bank */
   movb %bl, %es:(%eax, %ecx)    /* store the pixel */

putpix_done:
   popw %es
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _linear_putpixel() */




/* int _linear_getpixel(BITMAP *bmp, int x, int y);
 *  Reads a pixel from a linear bitmap.
 */
.globl __linear_getpixel 
   .align 4
__linear_getpixel:
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

   .align 4
getpix_skip:                     /* return -1 if we are off the edge */
   movl $-1, %eax
   jmp getpix_done

   .align 4
getpix_ok:
   READ_BANK()                   /* select the bank */

   movw BMP_SEG(%edx), %es       /* segment selector */
   movl %eax, %ebx
   xorl %eax, %eax
   movb %es:(%ebx, %ecx), %al    /* get the pixel */

getpix_done:
   popw %es
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _linear_getpixel() */




/* void _linear_hline(BITMAP *bmp, int x1, int y, int x2, int color);
 *  Draws a horizontal line onto a linear bitmap.
 */
.globl __linear_hline 
   .align 4
__linear_hline:
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

   WRITE_BANK()                  /* select write bank */
   movl %eax, %edi
   addl %ebx, %edi               /* dest address in edi */

   cmpl $DRAW_SOLID, __drawing_mode
   je hline_solid_mode           /* solid draw? */

   cmpl $DRAW_XOR, __drawing_mode
   jne hline_not_xor             /* XOR? */

   movl ARG3, %eax               /* select read bank */
   READ_BANK() 
   movl %eax, %esi
   addl %ebx, %esi               /* source address in esi */

   movb ARG5, %bl                /* read the color */

   .align 4, 0x90
hline_xor_loop:
   movb %es:(%esi), %al          /* read a pixel */
   xorb %bl, %al                 /* xor */
   movb %al, %es:(%edi)          /* and write it */
   incl %esi
   incl %edi
   loop hline_xor_loop
   jmp hline_done

   .align 4, 0x90
hline_not_xor:
   movl ARG3, %eax               /* get position in pattern bitmap */
   LOOKUP_PATTERN_POS(%ebx, %eax, %edx)
   movl %eax, %esi
   movl __drawing_x_mask, %edx

   cmpl $DRAW_COPY_PATTERN, __drawing_mode
   je hline_copy_pattern_loop

   movb ARG5, %ah                /* color in ah */

   cmpl $DRAW_SOLID_PATTERN, __drawing_mode
   je hline_solid_pattern_loop

   .align 4, 0x90
hline_masked_pattern_loop:
   movb (%esi, %ebx), %al        /* read a pixel */
   testb %al, %al                /* test it */
   jz hline_masked_skip
   movb %ah, %es:(%edi)          /* write a colored pixel */
hline_masked_skip:
   incl %ebx
   andl %edx, %ebx
   incl %edi
   loop hline_masked_pattern_loop
   jmp hline_done

   .align 4, 0x90
hline_copy_pattern_loop:
   movb (%esi, %ebx), %al        /* read a pixel */
   movb %al, %es:(%edi)          /* and write it */
   incl %ebx
   andl %edx, %ebx
   incl %edi
   loop hline_copy_pattern_loop
   jmp hline_done

   .align 4, 0x90
hline_solid_pattern_loop:
   movb (%esi, %ebx), %al        /* read a pixel */
   testb %al, %al                /* test it */
   jz hline_solid_zero

   movb %ah, %es:(%edi)          /* write a colored pixel */
   incl %ebx
   andl %edx, %ebx
   incl %edi
   loop hline_solid_pattern_loop
   jmp hline_done

   .align 4, 0x90
hline_solid_zero:
   movb $0, %es:(%edi)           /* write a zero pixel */
   incl %ebx
   andl %edx, %ebx
   incl %edi
   loop hline_solid_pattern_loop
   jmp hline_done

   .align 4, 0x90
hline_solid_mode:                /* regular hline drawer */
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




/* void _linear_vline(BITMAP *bmp, int x, int y1, int y2, int color);
 *  Draws a vertical line onto a linear bitmap.
 */
.globl __linear_vline 
   .align 4
__linear_vline:
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

   cmpl $DRAW_SOLID, __drawing_mode
   je vline_solid_mode           /* solid draw? */

   movl BMP_CLIP(%edx), %eax     /* store destination clipping flag */
   pushl %eax
   movl $0, BMP_CLIP(%edx)       /* turn clipping off (we already did it) */

   movl ARG5, %eax
   pushl %eax                    /* push color */
   pushl %esi                    /* push y */
   pushl %edi                    /* push x */
   pushl %edx                    /* push bitmap */

   movl %ecx, %ebx               /* ebx = loop counter */

   .align 4, 0x90
vline_special_mode_loop:         /* vline drawer for funny modes */
   call __linear_putpixel        /* draw the pixel */
   incl 8(%esp)                  /* next y */
   decl %ebx
   jg vline_special_mode_loop    /* loop */

   popl %edx                     /* clean up stack */
   addl $12, %esp
   popl %eax 
   movl %eax, BMP_CLIP(%edx)     /* restore bitmap clipping flag */
   jmp vline_done

   .align 4, 0x90
vline_solid_mode:                /* normal vline drawer */
   movl ARG5, %ebx               /* get color */

   .align 4, 0x90
vline_loop:
   movl %esi, %eax
   WRITE_BANK()                  /* select bank */

   movb %bl, %es:(%eax, %edi)    /* write pixel */

   incl %esi 
   decl %ecx
   jg vline_loop                 /* loop */

vline_done:
   popw %es 
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _linear_vline() */




/* void do_line(BITMAP *bmp, int x1, y1, x2, y2, int d, void (*proc)())
 *  Calculates all the points along a line between x1, y1 and x2, y2, 
 *  calling the supplied function for each one. This will be passed a 
 *  copy of the bmp parameter, the x and y position, and a copy of the 
 *  d parameter (so do_line() can be used with putpixel()).
 */
.globl _do_line 

   #define X1     ARG2
   #define Y1     ARG3
   #define X2     ARG4
   #define Y2     ARG5
   #define D      ARG6

   #define FIX_SHIFT       20
   #define ROUND_ERROR     ((1<<(FIX_SHIFT-1))-1)

   .align 4
_do_line:
   pushl %ebp
   movl %esp, %ebp
   pushl %edi
   pushl %esi
   pushl %ebx

   movl ARG1, %esi                  /* esi = bmp */
   movl ARG7, %edi                  /* edi = proc */

   movl X2, %eax                    /* eax = x2 */ 
   subl X1, %eax                    /* x2 - x1 */ 
   jge line_dx_pve 
   negl %eax                        /* eax = abs(dx) */ 
line_dx_pve: 
   movl Y2, %ebx                    /* ebx = y2 */ 
   subl Y1, %ebx                    /* x2 - x1 */ 
   jge line_dy_pve 
   negl %ebx                        /* ebx = abs(dy) */ 
line_dy_pve: 
   cmpl %eax, %ebx                  /* if abs(dy) > abs(dx) */ 
   jg line_y_driven 
   testl %eax, %eax                 /* skip if zero length */ 
   jz line_done 

   /* x-driven fixed point line draw */ 
   movl X1, %edx                    /* edx = x1 */ 
   movl Y1, %ebx                    /* ebx = y1 */ 
   movl X2, %ecx                    /* ecx = x2 */ 
   movl Y2, %eax                    /* eax = y2 */ 
   subl %edx, %ecx                  /* ecx = x2 - x1 = counter */ 
   jg line_x_noswap 

   negl %ecx                        /* make count positive */ 
   xchgl %eax, %ebx                 /* swap y values */ 
   movl X2, %edx 
   movl %edx, X1                    /* swap start x value */ 

line_x_noswap: 
   sall $FIX_SHIFT, %eax            /* make y2 fixed point */ 
   sall $FIX_SHIFT, %ebx            /* make y1 fixed point */ 
   subl %ebx, %eax                  /* eax = y2 - y1 = dy */ 
   cdq                              /* make edx:eax */ 
   idivl %ecx                       /* eax = fixed point y change */ 
   incl %ecx 
   movl %ecx, X2 
   movl %eax, Y1 

   addl $ROUND_ERROR, %ebx          /* compensate for rounding errors */

   .align 4, 0x90 
line_x_loop: 
   movl %ebx, %edx                  /* calculate y coord */ 
   sarl $FIX_SHIFT, %edx 

   pushl D                          /* data item */ 
   pushl %edx                       /* y coord */ 
   pushl X1                         /* x coord */ 
   pushl %esi                       /* bitmap */ 
   call *%edi                       /* call the draw routine */
   addl $16, %esp

   addl Y1, %ebx                    /* y += dy */ 
   incl X1                          /* x++ */ 
   decl X2 
   jg line_x_loop                   /* more? */ 
   jmp line_done 

   /* y-driven fixed point line draw */ 
   .align 4, 0x90 
line_y_driven: 
   movl Y1, %edx                    /* edx = y1 */ 
   movl X1, %ebx                    /* ebx = x1 */ 
   movl Y2, %ecx                    /* ecx = y2 */ 
   movl X2, %eax                    /* eax = x2 */ 
   subl %edx, %ecx                  /* ecx = y2 - y1 = counter */ 
   jg line_y_noswap 

   negl %ecx                        /* make count positive */ 
   xchgl %eax, %ebx                 /* swap x values */ 
   movl Y2, %edx 
   movl %edx, Y1                    /* swap start y value */ 

line_y_noswap: 
   sall $FIX_SHIFT, %eax            /* make x2 fixed point */ 
   sall $FIX_SHIFT, %ebx            /* make x1 fixed point */ 
   subl %ebx, %eax                  /* eax = x2 - x1 = dx */ 
   cdq                              /* make edx:eax */ 
   idivl %ecx                       /* eax = fixed point x change */ 
   incl %ecx 
   movl %ecx, Y2 
   movl %eax, X1 

   addl $ROUND_ERROR, %ebx          /* compensate for rounding errors */

   .align 4, 0x90 
line_y_loop: 
   movl %ebx, %edx 
   sarl $FIX_SHIFT, %edx            /* calculate x coord */ 

   pushl D                          /* data item */ 
   pushl Y1                         /* y coord */ 
   pushl %edx                       /* x coord */ 
   pushl %esi                       /* bitmap */ 
   call *%edi                       /* call the draw routine */ 
   addl $16, %esp

   addl X1, %ebx                    /* x += dx */ 
   incl Y1                          /* y++ */ 
   decl Y2 
   jg line_y_loop                   /* more? */ 

line_done:
   popl %ebx
   popl %esi
   popl %edi
   movl %ebp, %esp
   popl %ebp
   ret                              /* end of the line() */

