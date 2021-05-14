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
 *      Linear bitmap blitting (written for speed, not readability :-)
 *
 *      See readme.txt for copyright information.
 */


#include "asmdefs.h"

.text



/* void _linear_clear_to_color(BITMAP *bitmap, int color);
 *  Fills a linear bitmap with the specified color.
 */
.globl __linear_clear_to_color 
   .align 4
__linear_clear_to_color:
   pushl %ebp
   movl %esp, %ebp
   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %es 

   movl ARG1, %edx               /* edx = bmp */
   movl BMP_CT(%edx), %ebx       /* line to start at */

   movw BMP_SEG(%edx), %es       /* select segment */

   movl BMP_CR(%edx), %esi       /* width to clear */
   subl BMP_CL(%edx), %esi
   cld

   .align 4, 0x90
clear_loop:
   movl %ebx, %eax
   WRITE_BANK()                  /* select bank */
   movl %eax, %edi 
   addl BMP_CL(%edx), %edi       /* get line address  */

   movb ARG2, %al                /* duplicate color 4 times */
   movb %al, %ah
   shll $16, %eax
   movb ARG2, %al 
   movb %al, %ah

   movl %esi, %ecx               /* width to clear */
   shrl $1, %ecx                 /* halve for 16 bit clear */
   jnc clear_no_byte
   stosb                         /* clear an odd byte */

clear_no_byte:
   shrl $1, %ecx                 /* halve again for 32 bit clear */
   jnc clear_no_word
   stosw                         /* clear an odd word */

clear_no_word:
   jz clear_no_long 

   .align 4, 0x90
clear_x_loop:
   stosl                         /* clear the line */
   loop clear_x_loop

clear_no_long:
   incl %ebx
   cmpl %ebx, BMP_CB(%edx)
   jg clear_loop                 /* and loop */

   popw %es
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _linear_clear_to_color() */




#define SOURCE       ARG1        /* parameters to the blitting functions */
#define DEST         ARG2
#define SOURCE_X     ARG3
#define SOURCE_Y     ARG4
#define DEST_X       ARG5
#define DEST_Y       ARG6
#define WIDTH        ARG7
#define HEIGHT       ARG8




/* Framework for each version of the inner blitting loop... */
#define BLIT_LOOP(name, code...)                                             \
blit_loop_##name:                                                          ; \
   movl DEST, %edx               /* destination bitmap */                  ; \
   movl DEST_Y, %eax             /* line number */                         ; \
   WRITE_BANK()                  /* select bank */                         ; \
   addl DEST_X, %eax             /* x offset */                            ; \
   movl %eax, %edi                                                         ; \
									   ; \
   movl SOURCE, %edx             /* source bitmap */                       ; \
   movl SOURCE_Y, %eax           /* line number */                         ; \
   READ_BANK()                   /* select bank */                         ; \
   addl SOURCE_X, %eax           /* x offset */                            ; \
   movl %eax, %esi                                                         ; \
									   ; \
   movl WIDTH, %ecx              /* x loop counter */                      ; \
   movw BMP_SEG(%edx), %ds       /* load data segment */                   ; \
   code                          /* do the transfer */                     ; \
									   ; \
   movw %bx, %ds                 /* restore data segment */                ; \
   incl SOURCE_Y                                                           ; \
   incl DEST_Y                                                             ; \
   decl HEIGHT                                                             ; \
   jg blit_loop_##name           /* and loop */




/* void _linear_blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, 
 *                                  int dest_x, dest_y, int width, height);
 *  Normal forwards blitting routine for linear bitmaps.
 */
.globl __linear_blit 
   .align 4
__linear_blit:
   pushl %ebp
   movl %esp, %ebp
   pushw %es 
   pushl %edi
   pushl %esi
   pushl %ebx

   movl DEST, %edx
   movw BMP_SEG(%edx), %es       /* load destination segment */
   movw %ds, %bx                 /* save data segment selector */
   cld                           /* for forward copy */

   shrl $1, WIDTH                /* halve counter for word copies */
   jz blit_only_one_byte
   jnc blit_even_bytes

   .align 4, 0x90
   BLIT_LOOP(words_and_byte,     /* word at a time, plus leftover byte */
      rep ; movsw
      movsb
   )
   jmp blit_done

   .align 4, 0x90
blit_even_bytes: 
   shrl $1, WIDTH                /* halve counter again, for long copies */
   jz blit_only_one_word
   jnc blit_even_words

   .align 4, 0x90
   BLIT_LOOP(longs_and_word,     /* long at a time, plus leftover word */
      rep ; movsl
      movsw
   )
   jmp blit_done

   .align 4, 0x90
blit_even_words: 
   BLIT_LOOP(even_words,         /* copy a long at a time */
      rep ; movsl
   )
   jmp blit_done

   .align 4, 0x90
blit_only_one_byte: 
   BLIT_LOOP(only_one_byte,      /* copy just the one byte */
      movsb
   )
   jmp blit_done

   .align 4, 0x90
blit_only_one_word: 
   BLIT_LOOP(only_one_word,      /* copy just the one word */
      movsw
   )

   .align 4, 0x90
blit_done:
   popl %ebx
   popl %esi
   popl %edi
   popw %es
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _linear_blit() */




/* void _linear_blit_backward(BITMAP *source, BITMAP *dest, int source_x, 
 *                      int source_y, int dest_x, dest_y, int width, height);
 *  Reverse blitting routine, for overlapping linear bitmaps.
 */
.globl __linear_blit_backward
   .align 4
__linear_blit_backward:
   pushl %ebp
   movl %esp, %ebp
   pushw %es 
   pushl %edi
   pushl %esi
   pushl %ebx

   movl HEIGHT, %eax             /* y values go from high to low */
   decl %eax
   addl %eax, SOURCE_Y
   addl %eax, DEST_Y

   movl WIDTH, %eax              /* x values go from high to low */
   decl %eax
   addl %eax, SOURCE_X
   addl %eax, DEST_X

   movl DEST, %edx
   movw BMP_SEG(%edx), %es       /* load destination segment */
   movw %ds, %bx                 /* save data segment selector */

   .align 4
blit_backwards_loop:
   movl DEST, %edx               /* destination bitmap */
   movl DEST_Y, %eax             /* line number */
   WRITE_BANK()                  /* select bank */
   addl DEST_X, %eax             /* x offset */
   movl %eax, %edi

   movl SOURCE, %edx             /* source bitmap */
   movl SOURCE_Y, %eax           /* line number */
   READ_BANK()                   /* select bank */
   addl SOURCE_X, %eax           /* x offset */
   movl %eax, %esi

   movl WIDTH, %ecx              /* x loop counter */
   movw BMP_SEG(%edx), %ds       /* load data segment */
   std                           /* backwards */
   rep ; movsb                   /* copy the line */

   movw %bx, %ds                 /* restore data segment */
   decl SOURCE_Y
   decl DEST_Y
   decl HEIGHT
   jg blit_backwards_loop        /* and loop */

   cld                           /* finished */

   popl %ebx
   popl %esi
   popl %edi
   popw %es
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _linear_blit_backward() */



.globl __linear_blit_end
   .align 4
__linear_blit_end:
   ret




#undef SOURCE 
#undef DEST 



/* void _do_stretch(BITMAP *source, BITMAP *dest, void *drawer, 
 *                  int sx, fixed sy, fixed syd, int dx, int dy, int dh);
 *
 *  Helper function for stretch_blit(), calls the compiled line drawer.
 */
.globl __do_stretch

   #define SOURCE       ARG1
   #define DEST         ARG2
   #define DRAWER       ARG3
   #define SX           ARG4
   #define SY           ARG5
   #define SYD          ARG6
   #define DX           ARG7
   #define DY           ARG8
   #define DH           ARG9

   .align 4
__do_stretch:
   pushl %ebp
   movl %esp, %ebp
   pushw %es
   pushl %edi
   pushl %esi
   pushl %ebx

   movl DEST, %edx
   movw BMP_SEG(%edx), %es       /* load destination segment */
   movl DRAWER, %ebx             /* the actual line drawer */

   .align 4, 0x90
stretch_y_loop:
   movl SOURCE, %edx             /* get source line (in esi) and bank */
   movl SY, %eax
   shrl $16, %eax
   READ_BANK()
   movl %eax, %esi
   addl SX, %esi

   movl DEST, %edx               /* get dest line (in edi) and bank */
   movl DY, %eax
   WRITE_BANK()
   movl %eax, %edi
   addl DX, %edi

   call *%ebx                    /* draw the line (clobbers eax and ecx) */

   movl SYD, %eax                /* next line in source bitmap */
   addl %eax, SY
   incl DY                       /* next line in dest bitmap */
   decl DH
   jg stretch_y_loop

   popl %ebx
   popl %esi
   popl %edi
   popw %es
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _do_stretch() */

