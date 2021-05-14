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
 *      Blitting routines (written for speed, not readability :-)
 *
 *      See readme.txt for copyright information.
 */


#include "asmdefs.h"

.text



/* void clear_to_color(BITMAP *bitmap, int color);
 *  Fills the bitmap with the specified color.
 */
.globl _clear_to_color 
   .align 4
_clear_to_color:
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
   WRITE_BANK(%edi)              /* select bank */
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
   ret                           /* end of clear_to_color() */




/* void blit(BITMAP *source, BITMAP *dest, int source_x, int source_y, 
 *                                  int dest_x, dest_y, int width, height);
 *
 *  Copies an area of the source bitmap to the destination bitmap. source_x
 *  and source_y give the top left corner of the area of the source bitmap
 *  to copy, and dest_x and dest_y give the position in the destination
 *  bitmap. width and height give the size of the area to blit. This
 *  routine respects the clipping rectangle of the destination bitmap, and
 *  will work correctly even when the two memory areas overlap (ie. source
 *  and dest are the same). 
 */
.globl _blit 

   #define SOURCE       ARG1
   #define DEST         ARG2
   #define SOURCE_X     ARG3
   #define SOURCE_Y     ARG4
   #define DEST_X       ARG5
   #define DEST_Y       ARG6
   #define WIDTH        ARG7
   #define HEIGHT       ARG8

   /* This generates a framework for each version of the inner loop... */
   #define BLIT_LOOP(name, code...)                                          \
   blit_loop_##name:                                                       ; \
      movl DEST, %edx               /* destination bitmap */               ; \
      movl DEST_Y, %eax             /* line number */                      ; \
      WRITE_BANK(%edi)              /* select bank */                      ; \
      addl DEST_X, %eax             /* x offset */                         ; \
      movl %eax, %edi                                                      ; \
									   ; \
      movl SOURCE, %edx             /* source bitmap */                    ; \
      movl SOURCE_Y, %eax           /* line number */                      ; \
      READ_BANK(%esi)               /* select bank */                      ; \
      addl SOURCE_X, %eax           /* x offset */                         ; \
      movl %eax, %esi                                                      ; \
									   ; \
      movl WIDTH, %ecx              /* x loop counter */                   ; \
      movw BMP_SEG(%edx), %ds       /* load data segment */                ; \
      code                          /* do the transfer */                  ; \
									   ; \
      movw %bx, %ds                 /* restore data segment */             ; \
      incl SOURCE_Y                                                        ; \
      incl DEST_Y                                                          ; \
      decl HEIGHT                                                          ; \
      jg blit_loop_##name           /* and loop */


   /* the actual blit routine... */
   .align 4
_blit:
   pushl %ebp
   movl %esp, %ebp
   pushw %es
   pushl %edi
   pushl %esi
   pushl %ebx

   movl SOURCE, %ebx             /* ebx = source */
   movl SOURCE_X, %ecx           /* ecx = source_x */
   cmpl %ecx, BMP_W(%ebx)        /* if source_x > source->w */
   jle blit_done

   movl SOURCE_Y, %ecx           /* ecx = source_y */
   cmpl %ecx, BMP_H(%ebx)        /* if source_y >= source->h */
   jle blit_done

   movl DEST, %ebx               /* ebx = dest */
   movl DEST_X, %eax
   cmpl %eax, BMP_CR(%ebx)       /* if dest->x >= dest->cr */
   jle blit_done

   movl DEST_Y, %eax
   cmpl %eax, BMP_CB(%ebx)       /* if dest->y >= dest->cb */
   jle blit_done

   cmpw $0, SOURCE_X             /* if source_x < 0 */
   jge blit_source_x_pve
   movl SOURCE_X, %ecx
   addl %ecx, WIDTH              /* width += source_x */
   subl %ecx, DEST_X             /* dest_x -= source_x */
   movl $0, SOURCE_X             /* source_x = 0 */

blit_source_x_pve:
   cmpl $0, SOURCE_Y             /* if source_y < 0 */
   jge blit_source_y_pve
   movl SOURCE_Y, %ebx
   addl %ebx, HEIGHT             /* height += source_y */
   subl %ebx, DEST_Y             /* dest_y -= source_y */
   movl $0, SOURCE_Y             /* source_y = 0 */

blit_source_y_pve:
   movl WIDTH, %eax              /* eax = width */
   movl SOURCE_X, %edx           /* edx = source_x */
   addl %edx, %eax
   movl SOURCE, %ecx 
   movl BMP_W(%ecx), %edx        /* edx = source->w */
   cmpl %edx, %eax               /* if (width + source_x) > source->w */
   jle blit_width_ok
   subl SOURCE_X, %edx
   movl %edx, WIDTH              /* width = source->w - source_x */

blit_width_ok:
   movl HEIGHT, %eax             /* eax = height */
   movl SOURCE_Y, %edx           /* edx = source_y */
   addl %edx, %eax
   movl SOURCE, %ecx
   movl BMP_H(%ecx), %edx        /* edx = source->h */
   cmpl %edx, %eax               /* if (height + source_y) > source->h */
   jle blit_height_ok
   subl SOURCE_Y, %edx
   movl %edx, HEIGHT             /* height = source->h - source_y */

blit_height_ok:
   movl DEST, %ecx
   movl DEST_X, %eax
   cmpl %eax, BMP_CL(%ecx)       /* if dest_x < dest->cl */
   jle blit_dest_x_ok
   subl BMP_CL(%ecx), %eax       /* dest_x -= dest->cl */
   addl %eax, WIDTH              /* width += dest_x */
   subl %eax, SOURCE_X           /* source_x -= dest_x in here? */
   movl BMP_CL(%ecx), %eax
   movl %eax, DEST_X             /* dest_x = dest->cl */

blit_dest_x_ok:
   movl DEST, %ebx
   movl DEST_Y, %eax
   cmpl %eax, BMP_CT(%ebx)       /* if dest_y < dest->ct */
   jle blit_dest_y_ok
   subl BMP_CT(%ebx), %eax       /* dest_y -= dest->ct */
   addl %eax, HEIGHT             /* height += dest_y */
   subl %eax, SOURCE_Y           /* source_y -= dest_y */
   movl BMP_CT(%ebx), %eax
   movl %eax, DEST_Y             /* dest_y = dest->ct */

blit_dest_y_ok:
   movl WIDTH, %eax              /* eax = width */
   movl DEST_X, %edx             /* edx = dest_x */
   addl %edx, %eax
   movl DEST, %ecx
   movl BMP_CR(%ecx), %edx       /* edx = dest->cr */
   cmpl %edx, %eax               /* if (width + dest_x) > dest->cr */
   jle blit_width_ok2
   movl BMP_CR(%ecx), %ebx
   subl DEST_X, %ebx
   movl %ebx, WIDTH              /* width = dest->cr - dest_x */

blit_width_ok2:
   movl HEIGHT, %eax             /* eax = height */
   movl DEST_Y, %edx             /* edx = dest_y */
   addl %edx, %eax
   movl DEST, %ecx
   movl BMP_CB(%ecx), %edx       /* edx = dest->cb */
   cmpl %edx, %eax               /* if (height + dest_y) > dest->cb */
   jle blit_height_ok2
   movl BMP_CB(%ecx), %ebx
   subl DEST_Y, %ebx
   movl %ebx, HEIGHT             /* height = dest->cb - dest_y */

blit_height_ok2:
   movl WIDTH, %eax 
   orl %eax, %eax
   jle blit_done                 /* if width <= 0 */

   movl HEIGHT, %eax 
   orw %eax, %eax
   jle blit_done                 /* if height <= 0 */

   movl SOURCE, %eax
   cmpl %eax, DEST               /* if source == dest */
   jne blit_forwards

   cmpl %eax, _screen            /* if writing to the screen bitmap */
   jne blit_banks_ok

   movl BMP_RBANK(%eax), %edx
   cmpl %edx, BMP_WBANK(%eax)
   jne blit_banks_ok             /* ok if different read & write banks */

   cmpl $__stub_bank_switch, %edx
   je blit_banks_ok              /* and ok if linear access to memory */

   pushl HEIGHT                  /* have to use a temporary bitmap, since */
   pushl WIDTH                   /* no seperate read and write banks */
   call _create_bitmap
   addl $8, %esp

   movl %eax, %ebx               /* ebx = temp bitmap pointer */
   testl %ebx, %ebx
   jz blit_done

   pushl HEIGHT
   pushl WIDTH
   pushl $0
   pushl $0
   pushl SOURCE_Y
   pushl SOURCE_X
   pushl %ebx
   pushl SOURCE
   call _blit                    /* blit to temporary bitmap */
   addl $32, %esp

   pushl HEIGHT
   pushl WIDTH
   pushl DEST_Y
   pushl DEST_X
   pushl $0
   pushl $0
   pushl DEST
   pushl %ebx
   call _blit                    /* blit to destination */
   addl $32, %esp

   pushl %ebx
   call _destroy_bitmap          /* destroy temp bitmap */
   addl $4, %esp
   jmp blit_done

   .align 4
blit_banks_ok: 
   movl SOURCE_Y, %eax
   cmpl %eax, DEST_Y             /* if source_y >= dest_y */
   jg blit_backwards
   jl blit_forwards

   movl SOURCE_X, %eax
   cmpl %eax, DEST_X             /* if source_x >= dest_x */
   jle blit_forwards


   /* ************ reverse blit, for overlapping memory blocks ************ */

blit_backwards: 
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
   WRITE_BANK(%edi)              /* select bank */
   addl DEST_X, %eax             /* x offset */
   movl %eax, %edi

   movl SOURCE, %edx             /* source bitmap */
   movl SOURCE_Y, %eax           /* line number */
   READ_BANK(%esi)               /* select bank */
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
   jmp blit_done


   /* ************ normal forwards blit ************ */

   .align 4
blit_forwards:
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
   ret                           /* end of blit() */

.globl _blit_end
   .align 4
_blit_end:
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
   READ_BANK(%ecx)
   movl %eax, %esi
   addl SX, %esi

   movl DEST, %edx               /* get dest line (in edi) and bank */
   movl DY, %eax
   WRITE_BANK(%ecx)
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

