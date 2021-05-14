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
 *      Mode-X drawing functions.
 *
 *      Dominique Biesmans wrote the mode-X <-> linear blitting routines
 *      and the mode-X version of draw_sprite().
 *
 *      See readme.txt for copyright information.
 */


#include "asmdefs.h"

.text


/* bitmasks for enabling different write planes */
x_left_mask_table:
   .byte 0x00, 0x08, 0x0C, 0x0E, 0x0F

x_right_mask_table:
   .byte 0x00, 0x01, 0x03, 0x07, 0x0F




/* void _x_putpixel(BITMAP *bmp, int x, int y, int color);
 *  Draws a pixel onto a mode-X bitmap.
 */
.globl __x_putpixel
   .align 4
__x_putpixel:
   pushl %ebp
   movl %esp, %ebp
   pushl %ebx
   pushw %es

   movl ARG1, %edx               /* edx = bmp */
   movl ARG2, %ecx               /* ecx = x */
   movl ARG3, %ebx               /* ebx = y */

   cmpl $0, BMP_CLIP(%edx)       /* test clip flag */
   je xputpix_noclip
   cmpl %ecx, BMP_CL(%edx)       /* test cl */
   jg xputpix_done
   cmpl %ecx, BMP_CR(%edx)       /* test cr */
   jle xputpix_done
   cmpl %ebx, BMP_CT(%edx)       /* test ct */
   jg xputpix_done
   cmpl %ebx, BMP_CB(%edx)       /* test cb */
   jle xputpix_done

xputpix_noclip:
   movl $0x102, %eax             /* set the write plane */
   andb $3, %cl
   shlb %cl, %ah
   movl $0x3C4, %edx
   outw %ax, %dx

   movl ARG2, %ecx               /* reload ecx = x */
   movl ARG1, %edx               /* reload edx = bmp */
   movw BMP_SEG(%edx), %es 

   cmpl $DRAW_SOLID, __drawing_mode
   je xputpix_solid_mode         /* solid draw? */

   cmpl $DRAW_XOR, __drawing_mode
   je xputpix_xor_mode           /* XOR? */

   cmpl $DRAW_TRANS, __drawing_mode
   je xputpix_trans_mode         /* translucent? */

   LOOKUP_PATTERN_POS(%ecx, %ebx, %eax)
   movb (%ebx, %ecx), %al        /* read pixel from pattern bitmap */

   movl ARG2, %ecx               /* reload clobbered registers */
   movl ARG3, %ebx 

   cmpl $DRAW_COPY_PATTERN, __drawing_mode
   je xputpix_draw_it            /* draw the pattern pixel? */

   testb %al, %al                /* test the pattern pixel */
   jz xputpix_zero_pattern
   jmp xputpix_solid_mode

   .align 4, 0x90
xputpix_zero_pattern:
   cmpl $DRAW_MASKED_PATTERN, __drawing_mode
   je xputpix_done               /* skip zero pixels in masked mode */
   jmp xputpix_draw_it           /* draw zero pixels in solid mode */

   .align 4, 0x90
xputpix_xor_mode:
   movb $4, %al                  /* set the read plane */
   movb %cl, %ah
   andb $3, %ah
   movl $0x3CE, %edx
   outw %ax, %dx

   movl ARG4, %eax               /* eax = color */
   shrl $2, %ecx                 /* divide x by 4 */
   movl ARG1, %edx               /* reload edx = bmp */
   movl BMP_LINE(%edx, %ebx, 4), %ebx
   xorb %al, %es:(%ebx, %ecx)    /* XOR */
   jmp xputpix_done

   .align 4, 0x90
xputpix_trans_mode:
   movb $4, %al                  /* set the read plane */
   movb %cl, %ah
   andb $3, %ah
   movl $0x3CE, %edx
   outw %ax, %dx

   xorl %eax, %eax
   movb ARG4, %ah                /* ah = color being drawn */
   shrl $2, %ecx                 /* divide x by 4 */
   movl ARG1, %edx               /* reload edx = bmp */
   movl BMP_LINE(%edx, %ebx, 4), %ebx
   movb %es:(%ebx, %ecx), %al    /* al = existing color */
   movl _color_map, %edx         /* color map in edx */
   movb (%edx, %eax), %al        /* lookup color in table */
   movb %al, %es:(%ebx, %ecx)    /* write the pixel */
   jmp xputpix_done

   .align 4, 0x90
xputpix_solid_mode:
   movl ARG4, %eax               /* eax = color */

xputpix_draw_it:
   shrl $2, %ecx                 /* divide x by 4 */
   movl BMP_LINE(%edx, %ebx, 4), %ebx
   movb %al, %es:(%ebx, %ecx)    /* store the pixel */

xputpix_done:
   popw %es
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _x_putpixel() */




/* int _x_getpixel(BITMAP *bmp, int x, int y);
 *  Reads a pixel from a mode-X bitmap.
 */
.globl __x_getpixel
   .align 4
__x_getpixel:
   pushl %ebp
   movl %esp, %ebp
   pushl %ebx
   pushw %es

   movl ARG1, %edx               /* edx = bmp */
   movl ARG2, %ecx               /* ecx = x */
   movl ARG3, %ebx               /* ebx = y */

   testl %ecx, %ecx              /* test that we are on the bitmap */
   jl xgetpix_skip
   cmpl %ecx, BMP_W(%edx)
   jle xgetpix_skip
   testl %ebx, %ebx
   jl xgetpix_skip
   cmpl %ebx, BMP_H(%edx)
   jg xgetpix_ok

   .align 4, 0x90
xgetpix_skip:                    /* return -1 if we are off the edge */
   movl $-1, %eax
   jmp xgetpix_done

   .align 4, 0x90
xgetpix_ok:
   movb $4, %al                  /* set the correct plane */
   movb %cl, %ah
   andb $3, %ah
   movl $0x3CE, %edx
   outw %ax, %dx

   shrl $2, %ecx                 /* divide x by 4 */

   movl ARG1, %edx               /* reload edx = bmp */

   movw BMP_SEG(%edx), %es 
   movl BMP_LINE(%edx, %ebx, 4), %ebx

   xorl %eax, %eax
   movb %es:(%ebx, %ecx), %al    /* get the pixel */

xgetpix_done:
   popw %es
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _x_getpixel() */




/* void _x_hline(BITMAP *bmp, int x1, int y, int x2, int color);
 *  Draws a horizontal line onto a mode-X bitmap.
 *
 *  Patterned drawing could probably be done slightly faster by using the
 *  latch registers, but I don't think very much faster: there would be so
 *  much overhead due to the extra code complexity that it would only pay
 *  off for very long lines...
 */
.globl __x_hline 

   /* helper for patterned and xor lines, loops through each plane */
   #define HLINE_LOOP(name, putter...)                                       \
      movl %ecx, COUNTER                                                   ; \
      movl %ebx, START_X                                                   ; \
      movl %edi, SCREEN_ADDR                                               ; \
      movl $4, PLANE_COUNTER                                               ; \
									   ; \
      .align 4, 0x90                                                       ; \
   xhline_##name##_loop:                                                   ; \
      movl $0x102, %eax          /* set the write plane */                 ; \
      movb START_X, %cl                                                    ; \
      andb $3, %cl                                                         ; \
      shlb %cl, %ah                                                        ; \
      movl $0x3C4, %edx                                                    ; \
      outw %ax, %dx                                                        ; \
									   ; \
      SETUP                                                                ; \
									   ; \
      movl COUNTER, %edx                                                   ; \
      addl $3, %edx                                                        ; \
      shrl $2, %edx              /* edx = the number of bytes to alter */  ; \
									   ; \
      movl SCREEN_ADDR, %esi     /* esi = the place to write it */         ; \
									   ; \
      .align 4, 0x90                                                       ; \
   xhline_##name##_x_loop:                                                 ; \
      putter                     /* write the pixel */                     ; \
      incl %esi                                                            ; \
      decl %edx                                                            ; \
      jg xhline_##name##_x_loop                                            ; \
									   ; \
      incl START_X               /* next plane */                          ; \
      testl $3, START_X                                                    ; \
      jnz xhline_##name##_no_plane_wrap                                    ; \
      incl SCREEN_ADDR                                                     ; \
   xhline_##name##_no_plane_wrap:                                          ; \
      decl COUNTER                                                         ; \
      jle xhline_done                                                      ; \
									   ; \
      decl PLANE_COUNTER                                                   ; \
      jg xhline_##name##_loop    /* repeat for each plane */


   .align 4
__x_hline:
   pushl %ebp
   movl %esp, %ebp
   subl $16, %esp                /* 4 local variables: */

   #define COUNTER         -4(%ebp)
   #define START_X         -8(%ebp)
   #define SCREEN_ADDR     -12(%ebp)
   #define PLANE_COUNTER   -16(%ebp)

   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %es

   movl ARG1, %edx               /* edx = bmp */
   movl ARG2, %ebx               /* ebx = x1 */
   movl ARG3, %eax               /* eax = y */
   movl ARG4, %ecx               /* ecx = x2 */
   cmpl %ebx, %ecx
   jge xhline_no_xswap
   xchgl %ecx, %ebx

   .align 4, 0x90
xhline_no_xswap:
   cmpl $0, BMP_CLIP(%edx)       /* test bmp->clip */
   je xhline_noclip

   cmpl BMP_CT(%edx), %eax       /* test bmp->ct */
   jl xhline_done

   cmpl BMP_CB(%edx), %eax       /* test bmp->cb */
   jge xhline_done

   cmpl BMP_CL(%edx), %ebx       /* test x1, bmp->cl */
   jge xhline_x1_ok
   cmpl BMP_CL(%edx), %ecx       /* test x2, bmp->cl */
   jl xhline_done
   movl BMP_CL(%edx), %ebx       /* clip x1 */

   .align 4, 0x90
xhline_x1_ok:
   cmpl BMP_CR(%edx), %ecx       /* test x2, bmp->cr */
   jl xhline_noclip
   cmpl BMP_CR(%edx), %ebx       /* test x1, bmp->cr */
   jge xhline_done
   movl BMP_CR(%edx), %ecx       /* clip x2 */
   decl %ecx

xhline_noclip:
   subl %ebx, %ecx               /* loop counter */
   incl %ecx

   movw BMP_SEG(%edx), %es       /* segment selector */
   cld

   movl %ebx, %edi               /* dest address in edi */
   shrl $2, %edi
   addl BMP_LINE(%edx, %eax, 4), %edi

   cmpl $DRAW_SOLID, __drawing_mode
   je xhline_solid_mode          /* solid draw? */

   cmpl $DRAW_XOR, __drawing_mode
   jne xhline_not_xor            /* XOR? */


   /* helper to set up an XOR mode draw */
   #define SETUP                                                             \
      movb $4, %al               /* set the read plane */                  ; \
      movb %cl, %ah                                                        ; \
      movl $0x3CE, %edx                                                    ; \
      outw %ax, %dx                                                        ; \
									   ; \
      movb ARG5, %al             /* al = color */

   HLINE_LOOP(xor, xorb %al, %es:(%esi))
   jmp xhline_done

   #undef SETUP


   .align 4, 0x90
xhline_not_xor:
   cmpl $DRAW_TRANS, __drawing_mode
   jne xhline_not_trans          /* translucent? */


   /* helper to set up a translucent draw */
   #define SETUP                                                             \
      movb $4, %al               /* set the read plane */                  ; \
      movb %cl, %ah                                                        ; \
      movl $0x3CE, %edx                                                    ; \
      outw %ax, %dx                                                        ; \
									   ; \
      xorl %eax, %eax                                                      ; \
      movb ARG5, %ah             /* ah = color */                          ; \
									   ; \
      movl _color_map, %ecx      /* color map in ecx */

   HLINE_LOOP(trans,
      movb %es:(%esi), %al       /* read existing color */
      movb (%ecx, %eax), %al     /* lookup color in table */
      movb %al, %es:(%esi)       /* write pixel */
   )
   jmp xhline_done

   #undef SETUP


   .align 4, 0x90
xhline_not_trans:
   /* helper to set up a patterned draw */
   #define SETUP                                                             \
      movl START_X, %edi         /* edi = pattern x */                     ; \
      movl ARG3, %ebx            /* ebx = pattern y */                     ; \
      LOOKUP_PATTERN_POS(%edi, %ebx, %eax)                                 ; \
      movl __drawing_x_mask, %ecx                                          ; \
      movb ARG5, %ah             /* color in ah */

   cmpl $DRAW_COPY_PATTERN, __drawing_mode
   je xhline_copy_pattern

   cmpl $DRAW_SOLID_PATTERN, __drawing_mode
   je xhline_solid_pattern

   HLINE_LOOP(masked_pattern,
      movb (%ebx, %edi), %al     /* read pixel from pattern bitmap */
      testb %al, %al             /* test it */
      jz xhline_masked_zero
      movb %ah, %es:(%esi)       /* write solid pixel */
   xhline_masked_zero:
      addl $4, %edi 
      andl %ecx, %edi            /* advance through pattern bitmap */
   )
   jmp xhline_done

   .align 4, 0x90
xhline_solid_pattern:
   HLINE_LOOP(solid_pattern, 
      movb (%ebx, %edi), %al     /* read pixel from pattern bitmap */
      testb %al, %al             /* test it */
      jz xhline_solid_zero
      movb %ah, %al              /* select a colored pixel */
   xhline_solid_zero:
      movb %al, %es:(%esi)       /* write pixel */
      addl $4, %edi 
      andl %ecx, %edi            /* advance through pattern bitmap */
   )
   jmp xhline_done

   .align 4, 0x90
xhline_copy_pattern:
   HLINE_LOOP(copy_pattern, 
      movb (%ebx, %edi), %al     /* read pixel from pattern bitmap */
      movb %al, %es:(%esi)       /* write pixel */
      addl $4, %edi 
      andl %ecx, %edi            /* advance through pattern bitmap */
   )
   jmp xhline_done


   .align 4, 0x90
xhline_solid_mode:               /* draw the line in a solid color */
   movl $0x3C4, %edx             /* preload port address */

   andl $3, %ebx                 /* draw odd pixels on the left? */
   jz xhline_laligned

   negl %ebx                     /* how many pixels to draw on the left? */
   addl $4, %ebx
   subl %ebx, %ecx
   jge xhline_l_make_mask

   negl %ecx                     /* special case for very short lines */
   subl %ecx, %ebx
   movb x_left_mask_table(%ebx), %ah
   shrb %cl, %ah
   xorl %ecx, %ecx
   jmp xhline_l_made_mask

   .align 4, 0x90
xhline_l_make_mask:
   movb x_left_mask_table(%ebx), %ah

xhline_l_made_mask:
   movb $2, %al
   outw %ax, %dx                 /* select planes */

   movb ARG5, %al                /* get color */
   stosb                         /* write odd pixels on the left */

   test %ecx, %ecx
   jle xhline_done

xhline_laligned:
   movl $0x0F02, %eax            /* select all four planes */
   outw %ax, %dx

   movb ARG5, %bl                /* get color */
   movb %bl, %bh
   shrdl $16, %ebx, %eax
   movw %bx, %ax                 /* get four copies of the color */

   movl %ecx, %esi               /* store width in esi */
   shrl $2, %ecx                 /* divide by four for multi-plane writes */
   jle xhline_no_block

   shrl $1, %ecx                 /* halve for 16 bit writes */
   jnc xhline_no_byte
   stosb                         /* write an odd byte (four pixels) */

xhline_no_byte:
   jle xhline_no_block
   shrl $1, %ecx                 /* halve again for 32 bit writes */
   jnc xhline_no_word
   stosw                         /* write an odd word (eight pixels) */

xhline_no_word:
   jle xhline_no_block
   rep ; stosl                   /* write 32 bit values (16 pixels each) */

   .align 4, 0x90
xhline_no_block:
   andl $3, %esi                 /* draw odd pixels on the right? */
   jz xhline_done

   movb x_right_mask_table(%esi), %ah
   movb $2, %al 
   outw %ax, %dx                 /* select planes */

   movb ARG5, %al                /* get color */
   stosb                         /* write odd pixels on the right */

   .align 4, 0x90
xhline_done:
   popw %es
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _x_hline() */




#define SOURCE       ARG1        /* parameters to the blitting functions */
#define DEST         ARG2
#define SOURCE_X     ARG3
#define SOURCE_Y     ARG4
#define DEST_X       ARG5
#define DEST_Y       ARG6
#define WIDTH        ARG7
#define HEIGHT       ARG8

#define S_ADDR    -4(%ebp)
#define D_ADDR    -8(%ebp)

#define COUNT     -4(%ebp)




/* void _x_blit_from_memory(BITMAP *source, *dest, int source_x, source_y, 
 *                          int dest_x, int dest_y, int width, int height);
 *  Blits from a memory bitmap to a mode-X screen. The area will already have 
 *  been clipped when this routine is called.
 */
.globl __x_blit_from_memory
   .align 4
__x_blit_from_memory:
   pushl %ebp 
   movl %esp, %ebp 
   subl $8, %esp                 /* 2 local variables (S_ADDR and D_ADDR) */

   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %es

   movl $0x3C4, %edx             /* load port address into edx */
   movl DEST, %eax
   movw BMP_SEG(%eax), %es       /* load segment selector */
   cld

   .align 4, 0x90
x_blit_from_mem_y_loop:          /* for each line... */
   movl SOURCE, %esi 
   movl SOURCE_Y, %eax
   movl BMP_LINE(%esi, %eax, 4), %eax
   addl SOURCE_X, %eax
   movl %eax, S_ADDR             /* got the source address */

   movl DEST, %esi 
   movl DEST_Y, %eax
   movl DEST_X, %ecx
   movl %ecx, %ebx
   shrl $2, %ebx
   addl BMP_LINE(%esi, %eax, 4), %ebx
   movl %ebx, D_ADDR             /* got the dest address */

   andb $3, %cl 
   movl $0x1102, %eax
   shlb %cl, %ah                 /* write plane mask in ax */

   xorl %ebx, %ebx               /* ebx = plane counter */ 

   .align 4, 0x90
x_blit_from_mem_plane_loop:      /* for each plane... */
   movl S_ADDR, %esi
   movl D_ADDR, %edi

   movl WIDTH, %ecx              /* calculate width */
   addl $3, %ecx
   subl %ebx, %ecx
   shrl $2, %ecx
   jz x_blit_from_mem_skip_plane

   outw %ax, %dx                 /* select the write plane */

   .align 4, 0x90
x_blit_from_mem_x_loop: 
   movsb                         /* copy a pixel */
   addl $3, %esi                 /* fix up linear memory pointer */
   decl %ecx
   jg x_blit_from_mem_x_loop

x_blit_from_mem_skip_plane:
   rolb $1, %ah                  /* advance the plane position */
   adcl $0, D_ADDR
   incl S_ADDR

   incl %ebx                     /* next plane */
   cmpl $4, %ebx
   jl x_blit_from_mem_plane_loop

   incl SOURCE_Y                 /* next line */
   incl DEST_Y
   decl HEIGHT
   jg x_blit_from_mem_y_loop     /* loop */

   popw %es
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp 
   ret                           /* end of _x_blit_from_memory() */

.globl __x_blit_from_memory_end
  .align 4
__x_blit_from_memory_end:
   ret 




/* void _x_blit_to_memory(BITMAP *source, *dest, int source_x, source_y,
 *                          int dest_x, int dest_y, int width, int height);
 *  Blits to a memory bitmap from a mode-X screen. The area will already have
 *  been clipped when this routine is called.
 */
.globl __x_blit_to_memory
   .align 4
__x_blit_to_memory:
   pushl %ebp
   movl %esp, %ebp
   subl $8, %esp                 /* 2 local variables (S_ADDR and D_ADDR) */

   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %ds

   movl $0x3CE, %edx             /* load port address into edx */
   movl SOURCE, %eax
   movw %es:BMP_SEG(%eax), %ds   /* load segment selector */
   cld

   .align 4, 0x90
x_blit_to_mem_y_loop:            /* for each line... */
   movl DEST, %edi
   movl DEST_Y, %eax
   movl %es:BMP_LINE(%edi, %eax, 4), %eax
   addl DEST_X, %eax
   movl %eax, D_ADDR             /* got the dest address */

   movl SOURCE, %esi
   movl SOURCE_Y, %eax
   movl SOURCE_X, %ecx
   movl %ecx, %ebx
   shrl $2, %ebx
   addl %es:BMP_LINE(%esi, %eax, 4), %ebx
   movl %ebx, S_ADDR             /* got the source address */

   movb $4, %al                  /* set the correct plane */
   movb %cl, %ah
   andb $3, %ah

   xorl %ebx, %ebx               /* ebx = plane counter */

   .align 4, 0x90
x_blit_to_mem_plane_loop:        /* for each plane... */
   movl S_ADDR, %esi
   movl D_ADDR, %edi

   movl WIDTH, %ecx              /* calculate width */
   addl $3, %ecx
   subl %ebx, %ecx
   shrl $2, %ecx
   jz x_blit_to_mem_skip_plane

   outw %ax, %dx                 /* select the read plane */

   .align 4, 0x90
x_blit_to_mem_x_loop:
   movsb                         /* copy a pixel */
   addl $3, %edi                 /* fix up linear memory pointer */
   decl %ecx
   jg x_blit_to_mem_x_loop

x_blit_to_mem_skip_plane:
   addb $1, %ah                  /* advance the plane position */
   test $4, %ah
   jz x_blit_to_mem_no_add
   incl S_ADDR
   xor %ah, %ah
x_blit_to_mem_no_add:
   incl D_ADDR

   incl %ebx                     /* next plane */
   cmpl $4, %ebx
   jl x_blit_to_mem_plane_loop

   incl SOURCE_Y                 /* next line */
   incl DEST_Y
   decl HEIGHT
   jg x_blit_to_mem_y_loop       /* loop */

   popw %ds
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _x_blit_to_memory() */

.globl __x_blit_to_memory_end
  .align 4
__x_blit_to_memory_end:
   ret




/* void x_latched_blit(BITMAP *source, BITMAP *dest, int source_x, source_y, 
		       int dest_x, int dest_y, int width, int height);
 *  Blits from one part of a mode-X screen to another, using the VGA latch
 *  registers. It should only be called when the two regions have the same 
 *  plane alignment, and the area should already have been clipped.
 */
   .align 4
x_latched_blit:
   subl $8, %esp                 /* 2 local variables: */

   #define LMASK     -4(%ebp)
   #define RMASK     -8(%ebp)

   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %es
   pushw %ds

   movl $0, LMASK
   movl $0, RMASK

   movl WIDTH, %ecx              /* ecx = width */
   movl SOURCE_X, %eax           /* eax = source x */
   andl $3, %eax
   jz x_latched_blit_laligned

   negl %eax                     /* how many odd pixels on the left? */
   addl $4, %eax
   subl %eax, %ecx               /* adjust width */
   jge x_latched_blit_make_lmask

   negl %ecx                     /* special case for very narrow blits */
   subl %ecx, %eax
   movb x_left_mask_table(%eax), %ah
   shrb %cl, %ah
   xorl %ecx, %ecx
   jmp x_latched_blit_made_lmask

   .align 4, 0x90
x_latched_blit_make_lmask:
   movb x_left_mask_table(%eax), %ah

x_latched_blit_made_lmask:
   movb $2, %al                  /* store left edge mask */
   movw %ax, LMASK

   .align 4, 0x90
x_latched_blit_laligned:
   movl %ecx, %eax
   shrl $2, %ecx
   movl %ecx, %ebx               /* store main loop counter in ebx */

   andl $3, %eax                 /* how many odd pixels on the right? */
   jz x_latched_blit_raligned

   movb x_right_mask_table(%eax), %ah
   movb $2, %al
   movw %ax, RMASK

   .align 4, 0x90
x_latched_blit_raligned:
   movl DEST, %edx               /* load segment selectors */
   movl BMP_SEG(%edx), %eax 
   movw %ax, %ds
   movw %ax, %es
   cld

   movl $0x3CE, %edx
   movb $5, %al
   outb %al, %dx                 /* 3CE index 5 */
   incl %edx
   inb %dx, %al                  /* read+alter */
   andb $0xFC, %al 
   orb $1, %al 
   outb %al, %dx                 /* enable the latches */

   shrl $2, SOURCE_X             /* adjust the x offsets */
   shrl $2, DEST_X

   movl $0x3C4, %edx             /* preload port address for plane enables */

   .align 4, 0x90
x_latched_blit_loop:
   movl SOURCE, %ecx
   movl SOURCE_Y, %eax           /* get source address in esi */
   movl %ss:BMP_LINE(%ecx, %eax, 4), %esi
   addl SOURCE_X, %esi 

   movl DEST, %ecx
   movl DEST_Y, %eax             /* get dest address in edi */
   movl %ss:BMP_LINE(%ecx, %eax, 4), %edi
   addl DEST_X, %edi

   movl LMASK, %eax              /* copy odd pixels on the left? */
   orl %eax, %eax
   jz x_latched_no_left_pixels

   outw %ax, %dx                 /* copy on the left */
   movsb

   .align 4, 0x90
x_latched_no_left_pixels:
   orl %ebx, %ebx                /* main copy loop? */
   movl %ebx, %ecx
   jz x_latched_no_middle_pixels

   movl $0x0F02, %eax            /* enable all planes */
   outw %ax, %dx

   rep ; movsb                   /* do the bulk of the copy */

x_latched_no_middle_pixels:
   movl RMASK, %eax              /* copy odd pixels on the right? */
   orl %eax, %eax
   jz x_latched_no_right_pixels

   outw %ax, %dx                 /* copy on the right */
   movsb

   .align 4, 0x90
x_latched_no_right_pixels:
   incl SOURCE_Y                 /* next line */
   incl DEST_Y
   decl HEIGHT
   jg x_latched_blit_loop        /* loop */

   movl $0x3CE, %edx 
   movb $5, %al                  /* 3CE index 5 */
   outb %al, %dx
   incl %edx
   inb %dx, %al                  /* read+alter */
   andb $0xFC, %al 
   outb %al, %dx                 /* disable the latches */

   popw %ds
   popw %es
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of x_latched_blit() */




/* void _x_blit(BITMAP *source, BITMAP *dest, int source_x, source_y, 
		int dest_x, int dest_y, int width, int height);
 *  Blits from one part of a mode-X screen to another, drawing plane-by-plane
 *  (only safe when the two areas don't overlap). The area will already have 
 *  been clipped when this routine is called.
 */
.globl __x_blit
   .align 4
__x_blit:
   pushl %ebp
   movl %esp, %ebp

   movl SOURCE_X, %eax           /* use latches if the areas are aligned */
   andl $3, %eax
   movl DEST_X, %edx
   andl $3, %edx
   cmpl %eax, %edx
   je x_latched_blit

   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %es
   pushw %ds

   movl DEST, %edx               /* load segment selectors */
   movl BMP_SEG(%edx), %eax 
   movw %ax, %ds
   movw %ax, %es
   cld

   .align 4, 0x90
x_blit_loop:
   xorl %ebx, %ebx               /* ebx = current plane */

   .align 4, 0x90
x_blit_next_plane:
   movl SOURCE_X, %esi
   addl %ebx, %esi               /* esi = source_x coordinate */

   movl %esi, %ecx
   andb $3, %cl
   movb %cl, %ah
   movb $4, %al
   movl $0x3CE, %edx
   outw %ax, %dx                 /* set the read plane */

   movl DEST_X, %edi
   addl %ebx, %edi               /* edi = dest_x coordinate */

   movl %edi, %ecx
   andb $3, %cl
   movl $0x102, %eax
   shlb %cl, %ah
   movl $0x3C4, %edx
   outw %ax, %dx                 /* set the write plane */

   shrl $2, %esi                 /* get source address in esi */
   movl SOURCE, %edx
   movl SOURCE_Y, %eax
   addl %ss:BMP_LINE(%edx, %eax, 4), %esi

   shrl $2, %edi                 /* get dest address in edi */
   movl DEST, %edx
   movl DEST_Y, %eax
   addl %ss:BMP_LINE(%edx, %eax, 4), %edi

   movl WIDTH, %ecx              /* get counter in ecx */
   addl $3, %ecx
   subl %ebx, %ecx
   shrl $2, %ecx
   jz x_blit_skip_plane

   shrl $1, %ecx                 /* halve for word copy */
   jnc x_blit_no_byte

   movsb                         /* copy an odd byte? */

   .align 4, 0x90
x_blit_no_byte:
   shrl $1, %ecx                 /* halve again for long copy */
   jnc x_blit_no_word

   movsw                         /* copy an odd word? */

   .align 4, 0x90
x_blit_no_word:
   jz x_blit_skip_plane

   rep ; movsl;                  /* copy a plane of data */

x_blit_skip_plane:
   incl %ebx                     /* next plane */
   cmpl $4, %ebx
   jl x_blit_next_plane

   incl SOURCE_Y                 /* next line */
   incl DEST_Y
   decl HEIGHT
   jg x_blit_loop                /* loop */

   popw %ds
   popw %es
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _x_blit() */




/* void _x_blit_forward(BITMAP *source, BITMAP *dest, int source_x, source_y, 
			int dest_x, int dest_y, int width, int height);
 *  Blits from one part of a mode-X screen to another, drawing from top to
 *  bottom (much slower than doing it plane-by-plane, but sometimes required
 *  if the regions overlap). The area will already have been clipped when 
 *  this routine is called.
 */
.globl __x_blit_forward
   .align 4
__x_blit_forward:
   pushl %ebp
   movl %esp, %ebp

   movl SOURCE_X, %eax           /* use latches if the areas are aligned */
   andl $3, %eax
   movl DEST_X, %edx
   andl $3, %edx
   cmpl %eax, %edx
   je x_latched_blit

   subl $4, %esp                 /* 1 local variable (COUNT) */

   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %es

   movl DEST, %edx               /* load segment selector */
   movl BMP_SEG(%edx), %es

   .align 4, 0x90
x_blit_forward_y_loop:
   movl WIDTH, %eax              /* initialise x loop counter */
   movl %eax, COUNT

   movl SOURCE_X, %esi           /* esi = source_x coordinate */
   movl DEST_X, %edi             /* edi = dest_x coordinate */

   movl %esi, %ebx               /* get read plane mask in bx */
   andb $3, %bl
   movb %bl, %bh
   movb $4, %bl 

   movl %edi, %ecx               /* get write plane mask in cx */
   andb $3, %cl
   movb $0x11, %ch
   shlb %cl, %ch
   movb $2, %cl 

   shrl $2, %esi                 /* get source address in esi */
   movl SOURCE, %edx
   movl SOURCE_Y, %eax
   addl BMP_LINE(%edx, %eax, 4), %esi

   shrl $2, %edi                 /* get dest address in edi */
   movl DEST, %edx
   movl DEST_Y, %eax
   addl BMP_LINE(%edx, %eax, 4), %edi

   .align 4, 0x90
x_blit_forward_x_loop:
   movl $0x3CE, %edx             /* set the read plane */
   movl %ebx, %eax
   outw %ax, %dx 

   movl $0x3C4, %edx             /* set the write plane */
   movl %ecx, %eax
   outw %ax, %dx 

   movb %es:(%esi), %al          /* copy a pixel */
   movb %al, %es:(%edi) 

   incb %bh                      /* advance the read position */
   testb $4, %bh
   jz x_blit_forward_no_read_wrap

   xorb %bh, %bh                 /* wraps every four pixels */
   incl %esi

   .align 4, 0x90
x_blit_forward_no_read_wrap:
   rolb $1, %ch                  /* advance the write position */
   adcl $0, %edi

   .align 4, 0x90
x_blit_forward_no_write_wrap:
   decl COUNT
   jg x_blit_forward_x_loop      /* x loop */

   incl SOURCE_Y                 /* move on to the next line */
   incl DEST_Y
   decl HEIGHT
   jg x_blit_forward_y_loop      /* y loop */

   popw %es
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _x_blit_forward() */




/* void _x_blit_backward(BITMAP *source, BITMAP *dest, int source_x, source_y, 
			int dest_x, int dest_y, int width, int height);
 *  Blits from one part of a mode-X screen to another, drawing from bottom 
 *  to top (much slower than doing it plane-by-plane, but sometimes required
 *  if the regions overlap). The area will already have been clipped when 
 *  this routine is called.
 */
.globl __x_blit_backward
   .align 4
__x_blit_backward:
   pushl %ebp
   movl %esp, %ebp
   subl $4, %esp                 /* 1 local variable (COUNT) */

   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %es

   movl HEIGHT, %eax             /* y adjust for starting at the bottom */
   decl %eax
   addl %eax, SOURCE_Y
   addl %eax, DEST_Y

   movl WIDTH, %eax              /* x adjust for starting at the right */
   decl %eax
   addl %eax, SOURCE_X
   addl %eax, DEST_X

   movl DEST, %edx               /* load segment selector */
   movl BMP_SEG(%edx), %es

   .align 4, 0x90
x_blit_backward_y_loop:
   movl WIDTH, %eax              /* initialise x loop counter */
   movl %eax, COUNT

   movl SOURCE_X, %esi           /* esi = source_x coordinate */
   movl DEST_X, %edi             /* edi = dest_x coordinate */

   movl %esi, %ebx               /* get read plane mask in bx */
   andb $3, %bl
   movb %bl, %bh
   movb $4, %bl 

   movl %edi, %ecx               /* get write plane mask in cx */
   andb $3, %cl
   movb $0x11, %ch
   shlb %cl, %ch
   movb $2, %cl 

   shrl $2, %esi                 /* get source address in esi */
   movl SOURCE, %edx
   movl SOURCE_Y, %eax
   addl BMP_LINE(%edx, %eax, 4), %esi

   shrl $2, %edi                 /* get dest address in edi */
   movl DEST, %edx
   movl DEST_Y, %eax
   addl BMP_LINE(%edx, %eax, 4), %edi

   .align 4, 0x90
x_blit_backward_x_loop:
   movl $0x3CE, %edx             /* set the read plane */
   movl %ebx, %eax
   outw %ax, %dx 

   movl $0x3C4, %edx             /* set the write plane */
   movl %ecx, %eax
   outw %ax, %dx 

   movb %es:(%esi), %al          /* copy a pixel */
   movb %al, %es:(%edi) 

   decb %bh                      /* advance the read position */
   jge x_blit_backward_no_read_wrap

   movb $3, %bh                  /* wraps every four pixels */
   decl %esi

   .align 4, 0x90
x_blit_backward_no_read_wrap:
   rorb $1, %ch                  /* advance the write position */
   sbbl $0, %edi

   .align 4, 0x90
x_blit_backward_no_write_wrap:
   decl COUNT
   jg x_blit_backward_x_loop     /* x loop */

   decl SOURCE_Y                 /* move on to the next line */
   decl DEST_Y
   decl HEIGHT
   jg x_blit_backward_y_loop     /* y loop */

   popw %es
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _x_blit_backward() */




#undef SOURCE
#undef DEST
#undef SOURCE_X
#undef SOURCE_Y
#undef DEST_X 
#undef DEST_Y 
#undef WIDTH 
#undef HEIGHT 
#undef S_ADDR 
#undef D_ADDR

#define DEST         ARG1        /* parameters to the sprite draw functions */
#define SOURCE       ARG2
#define DEST_X       ARG3
#define DEST_Y       ARG4

#define S_ADDR    -4(%ebp)
#define D_ADDR    -8(%ebp)
#define SOURCE_Y  -12(%ebp)
#define WIDTH     -16(%ebp)
#define HEIGHT    -20(%ebp)
#define SOURCE_X  -24(%ebp)




/* void _x_draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, int y);
 *  Draws a sprite onto a mode-X screen.
 */
.globl __x_draw_sprite
   .align 4
__x_draw_sprite:
   pushl %ebp
   movl %esp, %ebp
   subl $24, %esp                 /* 6 local variables */

   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %es

   movl $0x3C4, %edx             /* load port address into edx */
   movl DEST, %eax
   movw BMP_SEG(%eax), %es       /* load segment selector */
   cld

   movl SOURCE, %esi
   movl BMP_W(%esi), %eax
   movl %eax, WIDTH
   movl BMP_H(%esi), %eax
   movl %eax, HEIGHT
   movl $0, SOURCE_X
   movl $0, SOURCE_Y             /* set source variables */

   movl DEST, %esi

   cmpl $0, BMP_CLIP(%esi)       /* clipping -> ... */
   jz x_draw_sprite_noclip

   movl BMP_CL(%esi), %eax
   subl DEST_X, %eax
   jle x_drspr_noclip_lx
   addl %eax, DEST_X
   addl %eax, SOURCE_X
   subl %eax, WIDTH
   jle x_draw_sprite_not
x_drspr_noclip_lx:
   movl BMP_CR(%esi), %eax
   subl DEST_X, %eax
   subl WIDTH, %eax
   jge x_drspr_noclip_rx
   addl %eax, WIDTH
   jle x_draw_sprite_not
x_drspr_noclip_rx:
   movl BMP_CT(%esi), %eax
   subl DEST_Y, %eax
   jle x_drspr_noclip_ty
   addl %eax, DEST_Y
   addl %eax, SOURCE_Y
   subl %eax, HEIGHT
   jle x_draw_sprite_not
x_drspr_noclip_ty:
   movl BMP_CB(%esi), %eax
   subl DEST_Y, %eax
   subl HEIGHT, %eax
   jge x_drspr_noclip_by
   addl %eax, HEIGHT
   jle x_draw_sprite_not

   .align 4, 0x90
x_drspr_noclip_by:
x_draw_sprite_noclip:            /* <--- */
x_draw_sprite_y_loop:            /* for each line... */
   movl SOURCE, %esi
   movl SOURCE_Y, %eax
   movl BMP_LINE(%esi,%eax,4), %eax
   addl SOURCE_X, %eax
   movl %eax, S_ADDR             /* got the source address */

   movl DEST, %esi
   movl DEST_Y, %eax
   movl DEST_X, %ecx
   movl %ecx, %ebx
   shrl $2, %ebx
   addl BMP_LINE(%esi, %eax, 4), %ebx
   movl %ebx, D_ADDR             /* got the dest address */

   andb $3, %cl
   movb $0x11, %ah
   shlb %cl, %ah                 /* write plane mask in ax */

   xorl %ebx, %ebx               /* ebx = plane counter */

   .align 4, 0x90
x_draw_sprite_plane_loop:        /* for each plane... */
   movl S_ADDR, %esi
   movl D_ADDR, %edi

   movl WIDTH, %ecx              /* calculate width */
   addl $3, %ecx
   subl %ebx, %ecx
   shrl $2, %ecx
   jz x_draw_sprite_skip_plane

   movb $2, %al
   outw %ax, %dx                 /* select the write plane */

   .align 4, 0x90
x_draw_sprite_x_loop:
   lodsb                         /* load a pixel */
   orb %al, %al 
   jz x_draw_sprite_no_draw      /* skip if zero */

   stosb                         /* store a pixel */
   addl $3, %esi                 /* fix up linear memory pointer */
   decl %ecx
   jg x_draw_sprite_x_loop
   jmp x_draw_sprite_skip_plane

   .align 4, 0x90
x_draw_sprite_no_draw:
   incl %edi                     /* skip a pixel */
   addl $3, %esi                 /* fix up linear memory pointer */
   decl %ecx
   jg x_draw_sprite_x_loop

x_draw_sprite_skip_plane:
   rolb $1, %ah                  /* advance the plane position */
   adcl $0, D_ADDR
   incl S_ADDR

   incl %ebx                     /* next plane */
   cmpl $4, %ebx
   jl x_draw_sprite_plane_loop

   incl SOURCE_Y                 /* next line */
   incl DEST_Y
   decl HEIGHT
   jg x_draw_sprite_y_loop       /* loop */

x_draw_sprite_not:
   popw %es
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _x_draw_sprite() */

.globl __x_draw_sprite_end
  .align 4
__x_draw_sprite_end:
   ret



