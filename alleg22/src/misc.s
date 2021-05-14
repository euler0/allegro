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
 *      Joystick polling, pallete manipulation, etc.
 *
 *      See readme.txt for copyright information.
 */


#include "asmdefs.h"

.text



/* empty bank switch routine for the standard VGA mode and memory bitmaps */
.globl __stub_bank_switch 
   .align 4
__stub_bank_switch:
   movl BMP_LINE(%edx, %eax, 4), %eax
   ret

.globl __stub_bank_switch_end
   .align 4
__stub_bank_switch_end:
   ret




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
   subl %eax, %ecx
   incl %ecx                  /* ecx = nr of colours to change */
   movl $0x3C8, %edx          /* edx = port index */
   outb %al, %dx              /* output pallete index */
   incl %edx                  /* edx = port index */
   cld

set_pal_loop: 
   outsb                      /* output r */
   outsb                      /* output g */
   outsb                      /* output b */
   incl %esi                  /* skip filler byte */
   decl %ecx                  /* next pallete entry */
   jnz set_pal_loop

   popl %esi
   movl %ebp, %esp
   popl %ebp
   ret                        /* end of _vga_set_pallete_range() */




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




/* void apply_matrix_f(MATRIX_f *m, float x, float y, float z, 
 *                                  float *xout, float *yout, float *zout);
 *  Floating point vector by matrix multiplication routine.
 */
.globl _apply_matrix_f

   #define MTX    ARG1
   #define X      ARG2
   #define Y      ARG3
   #define Z      ARG4
   #define XOUT   ARG5
   #define YOUT   ARG6
   #define ZOUT   ARG7

   .align 4
_apply_matrix_f:
   pushl %ebp
   movl %esp, %ebp
   pushl %ebx

   movl MTX, %edx 
   movl XOUT, %eax 
   movl YOUT, %ebx 
   movl ZOUT, %ecx 

   flds  M_V00(%edx) 
   fmuls X 
   flds  M_V01(%edx) 
   fmuls Y 
   flds  M_V02(%edx) 
   fmuls Z 
   fxch  %st(2) 

   faddp %st(0), %st(1) 
   flds  M_V10(%edx) 
   fxch  %st(2) 

   faddp %st(0), %st(1) 
   fxch  %st(1) 

   fmuls X 
   fxch  %st(1) 

   fadds M_T0(%edx) 
   flds  M_V11(%edx) 

   fmuls Y 
   flds  M_V12(%edx) 

   fmuls Z 
   fxch  %st(1) 

   faddp %st(0), %st(3) 
   flds  M_V20(%edx) 
   fxch  %st(3) 

   faddp %st(0), %st(1) 
   fxch  %st(2) 

   fmuls X 
   fxch  %st(2) 

   fadds M_T1(%edx) 
   flds  M_V21(%edx) 

   fmuls Y 
   flds  M_V22(%edx) 

   fmuls Z 
   fxch  %st(4) 

   faddp %st(0), %st(1) 
   fxch  %st(1) 
   fstps (%ebx) 

   faddp %st(0), %st(2) 
   fstps (%eax) 

   fadds M_T2(%edx) 
   fstps (%ecx)

   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                              /* end of apply_matrix_f() */
