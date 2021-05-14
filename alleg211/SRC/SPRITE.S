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
 *      Linear bitmap sprite drawing (written for speed, not readability :-)
 *
 *      See readme.txt for copyright information.
 */


#include "asmdefs.h"

.text



/* these definitions are shared by the regular and flipped sprite drawers */

#define BMP       ARG1
#define SPRITE    ARG2
#define X         ARG3
#define Y         ARG4

#define TGAP   -4(%ebp)
#define LGAP   -8(%ebp)
#define SGAP   -12(%ebp)
#define W      -16(%ebp)
#define H      -20(%ebp)


/* sets up a sprite draw operation and handles the clipping */
#define START_SPRITE_DRAW(name)                                              \
   pushl %ebp                                                              ; \
   movl %esp, %ebp                                                         ; \
   subl $20, %esp                         /* five local variables */       ; \
									   ; \
   pushl %edi                                                              ; \
   pushl %esi                                                              ; \
   pushl %ebx                                                              ; \
   pushw %es                                                               ; \
									   ; \
   movl BMP, %edx                         /* edx = bitmap pointer */       ; \
   movl SPRITE, %esi                      /* esi = sprite pointer */       ; \
									   ; \
   movw BMP_SEG(%edx), %es                /* segment selector */           ; \
									   ; \
   cmpl $0, BMP_CLIP(%edx)                /* test bmp->clip */             ; \
   jz name##_no_clip                                                       ; \
									   ; \
   movl BMP_CT(%edx), %eax                /* bmp->ct */                    ; \
   subl Y, %eax                           /* eax -= y */                   ; \
   jge name##_tgap_ok                                                      ; \
   xorl %eax, %eax                                                         ; \
name##_tgap_ok:                                                            ; \
   movl %eax, TGAP                        /* set tgap */                   ; \
									   ; \
   movl BMP_H(%esi), %ebx                 /* sprite->h */                  ; \
   movl BMP_CB(%edx), %ecx                /* bmp->cb */                    ; \
   subl Y, %ecx                           /* ecx -= y */                   ; \
   cmpl %ebx, %ecx                        /* check bottom clipping */      ; \
   jg name##_height_ok                                                     ; \
   movl %ecx, %ebx                                                         ; \
name##_height_ok:                                                          ; \
   subl %eax, %ebx                        /* height -= tgap */             ; \
   jle name##_done                                                         ; \
   movl %ebx, H                           /* set h */                      ; \
									   ; \
   movl BMP_CL(%edx), %eax                /* bmp->cl */                    ; \
   subl X, %eax                           /* eax -= x */                   ; \
   jge name##_lgap_ok                                                      ; \
   xorl %eax, %eax                                                         ; \
name##_lgap_ok:                                                            ; \
   movl %eax, LGAP                        /* set lgap */                   ; \
									   ; \
   movl BMP_W(%esi), %ebx                 /* sprite->w */                  ; \
   movl BMP_CR(%edx), %ecx                /* bmp->cr */                    ; \
   subl X, %ecx                           /* ecx -= x */                   ; \
   cmpl %ebx, %ecx                        /* check left clipping */        ; \
   jg name##_width_ok                                                      ; \
   movl %ecx, %ebx                                                         ; \
name##_width_ok:                                                           ; \
   subl %eax, %ebx                        /* width -= lgap */              ; \
   jle name##_done                                                         ; \
   movl %ebx, W                           /* set w */                      ; \
									   ; \
   jmp name##_clip_done                                                    ; \
									   ; \
   .align 4, 0x90                                                          ; \
name##_no_clip:                                                            ; \
   movl $0, TGAP                                                           ; \
   movl $0, LGAP                                                           ; \
   movl BMP_W(%esi), %eax                                                  ; \
   movl %eax, W                           /* w = sprite->w */              ; \
   movl BMP_H(%esi), %eax                                                  ; \
   movl %eax, H                           /* h = sprite->h */              ; \
									   ; \
   .align 4, 0x90                                                          ; \
name##_clip_done:



/* cleans up the stack after a sprite draw operation */
#define END_SPRITE_DRAW()                                                    \
   popw %es                                                                ; \
   popl %ebx                                                               ; \
   popl %esi                                                               ; \
   popl %edi                                                               ; \
   movl %ebp, %esp                                                         ; \
   popl %ebp



/* sets up the inner sprite drawing loop, loads registers, etc */
#define SPRITE_LOOP(name)                                                    \
sprite_y_loop_##name:                                                      ; \
   movl Y, %eax                           /* load line */                  ; \
   WRITE_BANK()                           /* select bank */                ; \
   addl X, %eax                           /* add x offset */               ; \
   movl W, %ecx                           /* x loop counter */             ; \
									   ; \
   .align 4, 0x90                                                          ; \
sprite_x_loop_##name:



/* ends the inner (x) part of a sprite drawing loop */
#define SPRITE_END_X(name) \
   loop sprite_x_loop_##name



/* ends the outer (y) part of a sprite drawing loop */
#define SPRITE_END_Y(name)                                                   \
   addl SGAP, %esi                        /* skip sprite bytes */          ; \
   incl Y                                 /* next line */                  ; \
   decl H                                 /* loop counter */               ; \
   jg sprite_y_loop_##name




/* void _linear_draw_sprite(BITMAP *bmp, BITMAP *sprite, int x, y);
 *  Draws a sprite onto a linear bitmap at the specified x, y position, 
 *  using a masked drawing mode where zero pixels are not output.
 */
.globl __linear_draw_sprite

   /* inner loop that copies just the one byte */
   #define LOOP_ONLY_ONE_BYTE                                                \
      SPRITE_LOOP(only_one_byte)                                           ; \
      movb (%esi), %bl                       /* read pixel */              ; \
      testb %bl, %bl                         /* test */                    ; \
      jz only_one_byte_skip                                                ; \
      movb %bl, %es:(%eax)                   /* write */                   ; \
   only_one_byte_skip:                                                     ; \
      incl %esi                                                            ; \
      incl %eax                                                            ; \
      /* no x loop */                                                      ; \
      SPRITE_END_Y(only_one_byte)


   /* inner loop that copies just the one word */
   #define LOOP_ONLY_ONE_WORD                                                \
      SPRITE_LOOP(only_one_word)                                           ; \
      movw (%esi), %bx                       /* read two pixels */         ; \
      testb %bl, %bl                         /* test */                    ; \
      jz only_one_word_skip_1                                              ; \
      movb %bl, %es:(%eax)                   /* write */                   ; \
   only_one_word_skip_1:                                                   ; \
      testb %bh, %bh                         /* test */                    ; \
      jz only_one_word_skip_2                                              ; \
      movb %bh, %es:1(%eax)                  /* write */                   ; \
   only_one_word_skip_2:                                                   ; \
      addl $2, %esi                                                        ; \
      addl $2, %eax                                                        ; \
      /* no x loop */                                                      ; \
      SPRITE_END_Y(only_one_word)


   /* inner loop that copies a word at a time, plus a leftover byte */
   #define LOOP_WORDS_AND_BYTE                                               \
      SPRITE_LOOP(words_and_byte)                                          ; \
      movw (%esi), %bx                       /* read two pixels */         ; \
      testb %bl, %bl                         /* test */                    ; \
      jz words_and_byte_skip_1                                             ; \
      movb %bl, %es:(%eax)                   /* write */                   ; \
   words_and_byte_skip_1:                                                  ; \
      testb %bh, %bh                         /* test */                    ; \
      jz words_and_byte_skip_2                                             ; \
      movb %bh, %es:1(%eax)                  /* write */                   ; \
   words_and_byte_skip_2:                                                  ; \
      addl $2, %esi                                                        ; \
      addl $2, %eax                                                        ; \
      SPRITE_END_X(words_and_byte)           /* end of x loop */           ; \
      movb (%esi), %bl                       /* read pixel */              ; \
      testb %bl, %bl                         /* test */                    ; \
      jz words_and_byte_end_skip                                           ; \
      movb %bl, %es:(%eax)                   /* write */                   ; \
   words_and_byte_end_skip:                                                ; \
      incl %esi                                                            ; \
      incl %eax                                                            ; \
      SPRITE_END_Y(words_and_byte)


   /* inner loop that copies a long at a time */
   #define LOOP_LONGS_ONLY                                                   \
      SPRITE_LOOP(longs_only)                                              ; \
      movl (%esi), %ebx                      /* read four pixels */        ; \
      testb %bl, %bl                         /* test */                    ; \
      jz longs_only_skip_1                                                 ; \
      movb %bl, %es:(%eax)                   /* write */                   ; \
   longs_only_skip_1:                                                      ; \
      testb %bh, %bh                         /* test */                    ; \
      jz longs_only_skip_2                                                 ; \
      movb %bh, %es:1(%eax)                  /* write */                   ; \
   longs_only_skip_2:                                                      ; \
      shrl $16, %ebx                         /* access next two pixels */  ; \
      testb %bl, %bl                         /* test */                    ; \
      jz longs_only_skip_3                                                 ; \
      movb %bl, %es:2(%eax)                  /* write */                   ; \
   longs_only_skip_3:                                                      ; \
      testb %bh, %bh                         /* test */                    ; \
      jz longs_only_skip_4                                                 ; \
      movb %bh, %es:3(%eax)                  /* write */                   ; \
   longs_only_skip_4:                                                      ; \
      addl $4, %esi                                                        ; \
      addl $4, %eax                                                        ; \
      SPRITE_END_X(longs_only)                                             ; \
      /* no cleanup at end of line */                                      ; \
      SPRITE_END_Y(longs_only) 


   /* inner loop that copies a long at a time, plus a leftover word */
   #define LOOP_LONGS_AND_WORD                                               \
      SPRITE_LOOP(longs_and_word)                                          ; \
      movl (%esi), %ebx                      /* read four pixels */        ; \
      testb %bl, %bl                         /* test */                    ; \
      jz longs_and_word_skip_1                                             ; \
      movb %bl, %es:(%eax)                   /* write */                   ; \
   longs_and_word_skip_1:                                                  ; \
      testb %bh, %bh                         /* test */                    ; \
      jz longs_and_word_skip_2                                             ; \
      movb %bh, %es:1(%eax)                  /* write */                   ; \
   longs_and_word_skip_2:                                                  ; \
      shrl $16, %ebx                         /* access next two pixels */  ; \
      testb %bl, %bl                         /* test */                    ; \
      jz longs_and_word_skip_3                                             ; \
      movb %bl, %es:2(%eax)                  /* write */                   ; \
   longs_and_word_skip_3:                                                  ; \
      testb %bh, %bh                         /* test */                    ; \
      jz longs_and_word_skip_4                                             ; \
      movb %bh, %es:3(%eax)                  /* write */                   ; \
   longs_and_word_skip_4:                                                  ; \
      addl $4, %esi                                                        ; \
      addl $4, %eax                                                        ; \
      SPRITE_END_X(longs_and_word)           /* end of x loop */           ; \
      movw (%esi), %bx                       /* read two pixels */         ; \
      testb %bl, %bl                         /* test */                    ; \
      jz longs_and_word_end_skip_1                                         ; \
      movb %bl, %es:(%eax)                   /* write */                   ; \
   longs_and_word_end_skip_1:                                              ; \
      testb %bh, %bh                         /* test */                    ; \
      jz longs_and_word_end_skip_2                                         ; \
      movb %bh, %es:1(%eax)                  /* write */                   ; \
   longs_and_word_end_skip_2:                                              ; \
      addl $2, %esi                                                        ; \
      addl $2, %eax                                                        ; \
      SPRITE_END_Y(longs_and_word)


   /* the actual sprite drawing routine... */
   .align 4
__linear_draw_sprite:
   START_SPRITE_DRAW(sprite)

   movl BMP_W(%esi), %eax        /* sprite->w */
   subl W, %eax                  /* - w */
   movl %eax, SGAP               /* store sprite gap */

   movl LGAP, %eax
   addl %eax, X                  /* X += lgap */

   movl TGAP, %eax 
   addl %eax, Y                  /* Y += tgap */

   movl BMP_LINE(%esi, %eax, 4), %esi
   addl LGAP, %esi               /* esi = sprite data ptr */

   shrl $1, W                    /* halve counter for word copies */
   jz sprite_only_one_byte
   jnc sprite_even_bytes

   .align 4, 0x90
   LOOP_WORDS_AND_BYTE           /* word at a time, plus leftover byte */
   jmp sprite_done

   .align 4, 0x90
sprite_even_bytes: 
   shrl $1, W                    /* halve counter again, for long copies */
   jz sprite_only_one_word
   jnc sprite_even_words

   .align 4, 0x90
   LOOP_LONGS_AND_WORD           /* long at a time, plus leftover word */
   jmp sprite_done

   .align 4, 0x90
sprite_even_words: 
   LOOP_LONGS_ONLY               /* copy a long at a time */
   jmp sprite_done

   .align 4, 0x90
sprite_only_one_byte: 
   LOOP_ONLY_ONE_BYTE            /* copy just the one byte */
   jmp sprite_done

   .align 4, 0x90
sprite_only_one_word: 
   LOOP_ONLY_ONE_WORD            /* copy just the one word */

   .align 4, 0x90
sprite_done:
   END_SPRITE_DRAW()
   ret                           /* end of _linear_draw_sprite() */

.globl __linear_draw_sprite_end
   .align 4
__linear_draw_sprite_end:
   ret




/* void _linear_draw_sprite_v_flip(BITMAP *bmp, BITMAP *sprite, int x, y);
 *  Draws a sprite to a linear bitmap, flipping vertically.
 */
.globl __linear_draw_sprite_v_flip 
   .align 4
__linear_draw_sprite_v_flip:
   START_SPRITE_DRAW(sprite_v_flip)

   movl BMP_W(%esi), %eax        /* sprite->w */
   addl W, %eax                  /* + w */
   negl %eax
   movl %eax, SGAP               /* store sprite gap */

   movl LGAP, %eax
   addl %eax, X                  /* X += lgap */

   movl TGAP, %eax 
   addl %eax, Y                  /* Y += tgap */

   negl %eax                     /* - tgap */
   addl BMP_H(%esi), %eax        /* + sprite->h */
   decl %eax
   movl BMP_LINE(%esi, %eax, 4), %esi
   addl LGAP, %esi               /* esi = sprite data ptr */

   .align 4, 0x90
   SPRITE_LOOP(v_flip) 
   movb (%esi), %bl              /* read pixel */
   testb %bl, %bl                /* test */
   jz sprite_v_flip_skip 
   movb %bl, %es:(%eax)          /* write */
sprite_v_flip_skip: 
   incl %esi 
   incl %eax 
   SPRITE_END_X(v_flip)
   SPRITE_END_Y(v_flip)

sprite_v_flip_done:
   END_SPRITE_DRAW()
   ret                           /* end of _linear_draw_sprite_v_flip() */




/* void _linear_draw_sprite_h_flip(BITMAP *bmp, BITMAP *sprite, int x, y);
 *  Draws a sprite to a linear bitmap, flipping horizontally.
 */
.globl __linear_draw_sprite_h_flip 
   .align 4
__linear_draw_sprite_h_flip:
   START_SPRITE_DRAW(sprite_h_flip)

   movl BMP_W(%esi), %eax        /* sprite->w */
   addl W, %eax                  /* + w */
   movl %eax, SGAP               /* store sprite gap */

   movl LGAP, %eax
   addl %eax, X                  /* X += lgap */

   movl TGAP, %eax 
   addl %eax, Y                  /* Y += tgap */

   movl BMP_W(%esi), %ecx 
   movl BMP_LINE(%esi, %eax, 4), %esi
   addl %ecx, %esi
   subl LGAP, %esi 
   decl %esi                     /* esi = sprite data ptr */

   .align 4, 0x90
   SPRITE_LOOP(h_flip) 
   movb (%esi), %bl              /* read pixel */
   testb %bl, %bl                /* test  */
   jz sprite_h_flip_skip 
   movb %bl, %es:(%eax)          /* write */
sprite_h_flip_skip: 
   decl %esi 
   incl %eax 
   SPRITE_END_X(h_flip)
   SPRITE_END_Y(h_flip)

sprite_h_flip_done:
   END_SPRITE_DRAW()
   ret                           /* end of _linear_draw_sprite_h_flip() */




/* void _linear_draw_sprite_vh_flip(BITMAP *bmp, BITMAP *sprite, int x, y);
 *  Draws a sprite to a linear bitmap, flipping both vertically and horizontally.
 */
.globl __linear_draw_sprite_vh_flip 
   .align 4
__linear_draw_sprite_vh_flip:
   START_SPRITE_DRAW(sprite_vh_flip)

   movl W, %eax                  /* w */
   subl BMP_W(%esi), %eax        /* - sprite->w */
   movl %eax, SGAP               /* store sprite gap */

   movl LGAP, %eax
   addl %eax, X                  /* X += lgap */

   movl TGAP, %eax 
   addl %eax, Y                  /* Y += tgap */

   negl %eax                     /* - tgap */
   addl BMP_H(%esi), %eax        /* + sprite->h */
   decl %eax
   movl BMP_W(%esi), %ecx 
   movl BMP_LINE(%esi, %eax, 4), %esi
   addl %ecx, %esi
   subl LGAP, %esi 
   decl %esi                     /* esi = sprite data ptr */

   .align 4, 0x90
   SPRITE_LOOP(vh_flip) 
   movb (%esi), %bl              /* read pixel */
   testb %bl, %bl                /* test  */
   jz sprite_vh_flip_skip 
   movb %bl, %es:(%eax)          /* write */
sprite_vh_flip_skip: 
   decl %esi 
   incl %eax 
   SPRITE_END_X(vh_flip)
   SPRITE_END_Y(vh_flip)

sprite_vh_flip_done:
   END_SPRITE_DRAW()
   ret                           /* end of _linear_draw_sprite_vh_flip() */




/* void __linear_draw_character(BITMAP *bmp, BITMAP *sprite, int x, y, color);
 *  For proportional font output onto a linear bitmap: uses the sprite as 
 *  a mask, replacing all set pixels with the specified color.
 */
.globl __linear_draw_character 

   #define COLOR  ARG5

   .align 4
__linear_draw_character:
   START_SPRITE_DRAW(draw_char)

   movl BMP_W(%esi), %eax        /* sprite->w */
   subl W, %eax                  /* - w */
   movl %eax, SGAP               /* store sprite gap */

   movl LGAP, %eax
   addl %eax, X                  /* X += lgap */

   movl TGAP, %eax 
   addl %eax, Y                  /* Y += tgap */

   movl BMP_LINE(%esi, %eax, 4), %esi
   addl LGAP, %esi               /* esi = sprite data ptr */

   movb COLOR, %bl               /* bl = text color */
   movb __textmode, %bh          /* bh = background color */
   cmpl $0, __textmode
   jl draw_masked_char

   /* opaque (text_mode >= 0) character output */
   SPRITE_LOOP(draw_opaque_char) 
   cmpb $0, (%esi)               /* test pixel */
   jz draw_opaque_background
   movb %bl, %es:(%eax)          /* write pixel */
   jmp draw_opaque_done
draw_opaque_background: 
   movb %bh, %es:(%eax)          /* write background */
draw_opaque_done:
   incl %esi 
   incl %eax 
   SPRITE_END_X(draw_opaque_char)
   SPRITE_END_Y(draw_opaque_char)
   jmp draw_char_done

   /* masked (text_mode -1) character output */
   .align 4, 0x90
draw_masked_char:
   SPRITE_LOOP(draw_masked_char) 
   cmpb $0, (%esi)               /* test pixel */
   jz draw_masked_skip
   movb %bl, %es:(%eax)          /* write pixel */
draw_masked_skip:
   incl %esi 
   incl %eax 
   SPRITE_END_X(draw_masked_char)
   SPRITE_END_Y(draw_masked_char)

draw_char_done:
   END_SPRITE_DRAW()
   ret                           /* end of _linear_draw_character() */



#undef BMP
#undef X 
#undef Y 
#undef TGAP
#undef LGAP
#undef HEIGHT
#undef COLOR



/* void _linear_textout_8x8(BITMAP *bmp, FONT_8x8 *font, char *str, int x, y, color);
 *  Fast text output routine for 8x8 fonts onto linear bitmaps.
 */
.globl __linear_textout_8x8 

   #define BMP       ARG1
   #define FONT      ARG2
   #define STR       ARG3
   #define X         ARG4
   #define Y         ARG5
   #define COLOR     ARG6

   .align 4
__linear_textout_8x8:
   pushl %ebp
   movl %esp, %ebp
   subl $24, %esp                /* 6 local variables: */

   #define TGAP      -4(%ebp)
   #define HEIGHT    -8(%ebp)
   #define LGAP      -12(%ebp)
   #define RGAP      -16(%ebp)
   #define CHAR_W    -20(%ebp)
   #define C         -24(%ebp)

   pushl %edi
   pushl %esi
   pushl %ebx
   pushw %es

   movl BMP, %esi                /* esi = bmp */
   movw BMP_SEG(%esi), %es       /* segment selector */

   cmpl $0, BMP_CLIP(%esi)       /* test bmp->clip */
   jz text_no_clip 

   movl Y, %eax                  /* eax = y */
   movl BMP_CT(%esi), %edx       /* edx = bmp->ct */
   subl %eax, %edx               /* edx -= y */
   cmpl $8, %edx
   jge text_done
   orl %edx, %edx
   jge text_tgap_pve
   xorl %edx, %edx
text_tgap_pve:
   movl %edx, TGAP               /* set tgap */
   addl %edx, %eax               /* y += tgap */
   movl %eax, Y                  /* store y */

   movl BMP_CB(%esi), %ebx       /* ebx = bmp->cb */
   subl %eax, %ebx               /* ebx -= y */
   jle text_done

   negl %edx
   addl $8, %edx                 /* edx = 8 - tgap */
   cmpl %ebx, %edx               /* if height > 8 - tgap */
   jg text_height_small
   movl %edx, %ebx
text_height_small:
   movl %ebx, HEIGHT             /* store height */

   movl BMP_CL(%esi), %eax       /* eax = bmp->cl */
   movl X, %edx                  /* edx = x */
   subl %edx, %eax               /* eax = bmp->cl - x */
   jge text_lgap_pve
   xorl %eax, %eax
text_lgap_pve:
   movl STR, %ebx                /* ebx = str */
text_lclip_loop:
   cmpl $8, %eax                 /* while eax >= 8 */
   jl text_lclip_done
   cmpb $0, (%ebx)               /* if !*str */
   jz text_done
   incl %ebx
   addl $8, X
   subl $8, %eax
   jmp text_lclip_loop

   .align 4, 0x90
text_lclip_done:
   movl %ebx, STR                /* store str */
   movl %eax, LGAP               /* store lgap */

   movl X, %eax                  /* x */
   movl BMP_CR(%esi), %edx       /* bmp->cr */
   subl %eax, %edx
   jl text_done
   movl %edx, RGAP               /* set rgap */
   jmp text_char_loop_start

   .align 4, 0x90
text_no_clip:
   movl $0, TGAP
   movl $8, HEIGHT
   movl $0, LGAP
   movl $0x7fff, RGAP
   jmp text_char_loop_start

   .align 4, 0x90
text_char_loop:                  /* for each character... */
   cmpb $126, %al                /* the char will already be in %al */
   ja text_char_no_good
   subb $32, %al                 /* convert to table offset */
   jl text_char_no_good
   andl $0xff, %eax
   jmp text_char_sorted

   .align 4, 0x90
text_char_no_good:
   xorl %eax, %eax               /* oops - not ASCII */

   .align 4, 0x90
text_char_sorted:
   movl FONT, %esi               /* esi = font */
   shll $3, %eax
   addl %eax, %esi
   addl TGAP, %esi               /* esi = position in font bitmap */

   movl RGAP, %eax               /* rgap */
   cmpl $0, %eax
   jle text_done                 /* have we gone off the right? */
   cmpl $8, %eax
   jle text_rgap_ok
   movl $8, %eax                 /* dont want chars wider than 8! */
text_rgap_ok:
   movl %eax, CHAR_W             /* set char width */
   xorl %ebx, %ebx
   jmp text_y_loop_start

   .align 4, 0x90
text_y_loop:                     /* for each y... */
   addl Y, %ebx                  /* add y, c will already be in ebx */

   movl BMP, %edx                /* bmp */
   movl %ebx, %eax               /* line number */
   WRITE_BANK()                  /* get bank */
   movl %eax, %edi

   movl X, %edx                  /* x */
   addl %edx, %edi

   movb (%esi), %dl              /* dl = bit mask */
   incl %esi

   movl LGAP, %ecx               /* lgap */
   orl %ecx, %ecx
   jz text_no_lgap               /* do we need to clip on the left? */

   shlb %cl, %dl                 /* shift the mask */
   addl %ecx, %edi               /* move the screen position */
   negl %ecx

   .align 4, 0x90
text_no_lgap:
   addl CHAR_W, %ecx             /* ecx = x loop counter */
   jle text_no_x_loop

   movl COLOR, %eax              /* ax = text color */
   movl __textmode, %ebx         /* ebx = background color */

   .align 4, 0x90
text_x_loop:                     /* for each x... */
   shlb $1, %dl                  /* shift the mask */
   jc text_put_bit

   orl %ebx, %ebx
   jl text_put_done
   movb %bl, %es:(%edi)          /* draw background pixel */
   jmp text_put_done

   .align 4, 0x90
text_put_bit: 
   movb %al, %es:(%edi)          /* draw a pixel */

text_put_done:
   incl %edi
   loop text_x_loop              /* and loop */

text_no_x_loop:
   movl C, %ebx                  /* increment loop counter */
   incl %ebx
text_y_loop_start:
   movl %ebx, C
   cmpl HEIGHT, %ebx
   jl text_y_loop

text_y_loop_done:
   movl $0, LGAP                 /* sort out a load of variables */
   subl $8, RGAP
   addl $8, X

   incl STR                      /* move on to the next character */
text_char_loop_start:
   movl STR, %ebx                /* read a char into al */
   movb (%ebx), %al
   orb %al, %al
   jz text_done
   jmp text_char_loop

text_done:
   popw %es
   popl %ebx
   popl %esi
   popl %edi
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _textout_8x8() */



#undef BMP
#undef X
#undef Y
#undef LGAP
#undef W
#undef H



/* void _linear_draw_rle_sprite(BITMAP *bmp, RLE_SPRITE *sprite, int x, int y)
 *  Draws an RLE sprite onto a linear bitmap at the specified position.
 */
.globl __linear_draw_rle_sprite

   #define BMP       ARG1
   #define SPRITE    ARG2
   #define X         ARG3
   #define Y         ARG4

   .align 4
__linear_draw_rle_sprite:
   pushl %ebp
   movl %esp, %ebp

   subl $12, %esp                /* 3 local variables: */

   #define LGAP      -4(%ebp)
   #define W         -8(%ebp)
   #define H         -12(%ebp)

   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %es 

   movl $0, LGAP                 /* normally zero gap on left */
   movl SPRITE, %esi             /* esi = sprite pointer */
   movl RLE_W(%esi), %eax        /* read sprite width */
   movl %eax, W 
   movl RLE_H(%esi), %eax        /* read sprite height */
   movl %eax, H
   addl $RLE_DAT, %esi           /* points to start of RLE data */

   movl BMP, %edx                /* edx = bitmap pointer */
   movw BMP_SEG(%edx), %es       /* select segment */
   cld

   cmpl $0, BMP_CLIP(%edx)       /* test clip flag */
   je rle_noclip_y_loop

   movl Y, %ecx                  /* ecx = Y */

rle_clip_top:
   cmpl %ecx, BMP_CT(%edx)       /* test top clipping */
   jle rle_top_ok

   incl %ecx                     /* increment Y */
   decl H                        /* decrement height */
   jle rle_done

   .align 4, 0x90
rle_clip_top_loop:
   lodsb                         /* find zero EOL marker in RLE data */
   testb %al, %al
   jnz rle_clip_top_loop

   jmp rle_clip_top

   .align 4, 0x90
rle_top_ok:
   movl %ecx, Y                  /* store clipped Y */

   addl H, %ecx                  /* ecx = Y + height */
   subl BMP_CB(%edx), %ecx       /* test bottom clipping */
   jl rle_bottom_ok

   subl %ecx, H                  /* clip on the bottom */
   jle rle_done

   .align 4, 0x90
rle_bottom_ok:
   movl BMP_CL(%edx), %eax       /* check left clipping */
   subl X, %eax
   jle rle_left_ok

   movl %eax, LGAP               /* clip on the left */
   addl %eax, X
   subl %eax, W
   jle rle_done

   .align 4, 0x90
rle_left_ok:
   movl X, %eax                  /* check right clipping */
   addl W, %eax
   subl BMP_CR(%edx), %eax
   jle rle_no_right_clip

   subl %eax, W
   jl rle_done
   jmp rle_clip_y_loop

   .align 4, 0x90
rle_no_right_clip:
   cmpl $0, LGAP                 /* can we use the fast noclip drawer? */
   je rle_noclip_y_loop


   /* slower version of the drawer for sprites that need clipping */
   .align 4, 0x90 
rle_clip_y_loop: 
   movl Y, %eax
   WRITE_BANK()                  /* bank switch */
   movl %eax, %edi
   addl X, %edi                  /* offset within the line */

   movl W, %ebx
   movl LGAP, %ecx

rle_clip_lgap_loop:
   lodsb                         /* read a command byte */
   testb %al, %al                /* and test it */
   js rle_clip_lgap_zeros

   movzbl %al, %eax              /* skip a solid run */
   addl %eax, %esi
   subl %eax, %ecx
   jge rle_clip_lgap_loop

   negl %ecx
   subl %ecx, %esi               /* oops, we overshot */
   movl %ecx, %eax
   jmp rle_clip_x_loop

   .align 4, 0x90
rle_clip_lgap_zeros:
   movsbl %al, %eax              /* skip a run of zeros */
   addl %eax, %ecx
   jge rle_clip_lgap_loop

   movl %ecx, %eax               /* oops, we overshot */

   .align 4, 0x90
rle_clip_x_loop:
   testb %al, %al                /* test command byte */
   jz rle_clip_x_done
   js rle_clip_skip_zeros

   movzbl %al, %ecx              /* write a string of pixels */
   subl %ecx, %ebx
   jle rle_clip_string

   rep ; movsb
   lodsb                         /* read next command byte */
   jmp rle_clip_x_loop

   .align 4, 0x90
rle_clip_string:
   addl %ebx, %ecx               /* only write part of the string */
   rep ; movsb
   subl %ebx, %esi
   jmp rle_clip_skip_rgap

   .align 4, 0x90
rle_clip_skip_zeros:
   negb %al                      /* skip over a string of zeros */
   movzbl %al, %eax
   addl %eax, %edi
   subl %eax, %ebx
   jle rle_clip_skip_rgap

   lodsb                         /* read next command byte */
   jmp rle_clip_x_loop

   .align 4, 0x90
rle_clip_skip_rgap:
   lodsb                         /* skip forward to zero EOL marker */
   testb %al, %al
   jnz rle_clip_skip_rgap

rle_clip_x_done:
   incl Y
   decl H
   jg rle_clip_y_loop
   jmp rle_done


   /* fast drawer for sprites that don't need clipping */
   .align 4, 0x90 
rle_noclip_y_loop: 
   movl Y, %eax
   WRITE_BANK()                  /* bank switch */
   movl %eax, %edi
   addl X, %edi                  /* offset within the line */

   .align 4, 0x90
rle_noclip_x_loop:
   lodsb                         /* read a command byte */
   testb %al, %al                /* and test it */
   jz rle_noclip_x_done
   js rle_noclip_skip_zeros

   movzbl %al, %ecx              /* write a string of pixels */

   shrl $1, %ecx
   jnc rle_noclip_no_byte
   movsb                         /* copy odd byte? */
rle_noclip_no_byte:
   jz rle_noclip_x_loop
   shrl $1, %ecx
   jnc rle_noclip_no_word
   movsw                         /* copy odd word? */
rle_noclip_no_word:
   jz rle_noclip_x_loop
   rep ; movsl                   /* 32 bit string copy */
   jmp rle_noclip_x_loop

   .align 4, 0x90
rle_noclip_skip_zeros:
   negb %al                      /* skip over a string of zeros */
   movzbl %al, %eax
   addl %eax, %edi
   jmp rle_noclip_x_loop

   .align 4, 0x90
rle_noclip_x_done:
   incl Y
   decl H
   jg rle_noclip_y_loop


rle_done:
   popw %es 
   popl %edi
   popl %esi
   popl %ebx

   movl %ebp, %esp
   popl %ebp
   ret                           /* end of _linear_draw_rle_sprite() */



#undef BMP
#undef SPRITE
#undef X
#undef Y



/* void draw_compiled_sprite(BITMAP *bmp, COMPILED_SPRITE *sprite, int x, y)
 *  Draws a compiled sprite onto the specified bitmap at the specified
 *  position, _ignoring_ clipping. The bitmap must be in the same format
 *  that the sprite was compiled for.
 */
.globl _draw_compiled_sprite

   #define BMP       ARG1
   #define SPRITE    ARG2
   #define X         ARG3
   #define Y         ARG4

   .align 4
_draw_compiled_sprite:
   pushl %ebp
   movl %esp, %ebp
   subl $4, %esp                 /* 1 local variables */

   #define PLANE     -4(%ebp)

   pushl %ebx
   pushl %esi
   pushl %edi

   movl BMP, %edx                /* bitmap pointer in edx */
   movw BMP_SEG(%edx), %fs       /* load segment selector into fs */

   movl SPRITE, %ebx
   cmpl $0, CMP_PLANAR(%ebx)     /* is the sprite planar or linear? */
   je linear_compiled_sprite

   movl X, %ecx                  /* get write plane mask in bx */
   andb $3, %cl
   movl $0x1102, %ebx
   shlb %cl, %bh

   movl BMP_LINE+4(%edx), %ecx   /* get line width in ecx */
   subl BMP_LINE(%edx), %ecx

   movl X, %esi                  /* get destination address in edi */
   shrl $2, %esi
   movl Y, %edi
   movl BMP_LINE(%edx, %edi, 4), %edi
   addl %esi, %edi

   movl $0x3C4, %edx             /* port address in dx */

   movl $0, PLANE                /* zero the plane counter */

   .align 4, 0x90
planar_compiled_sprite_loop:
   movl %ebx, %eax               /* set the write plane */
   outw %ax, %dx 

   movl %edi, %eax               /* get address in eax */

   movl PLANE, %esi              /* get the drawer function in esi */
   shll $2, %esi
   addl SPRITE, %esi
   movl CMP_DRAW(%esi), %esi

   call *%esi                    /* and draw the plane! */

   incl PLANE                    /* next plane */
   cmpl $4, PLANE
   jge draw_compiled_sprite_done

   rolb $1, %bh                  /* advance the plane position */
   adcl $0, %edi
   jmp planar_compiled_sprite_loop

   .align 4, 0x90
linear_compiled_sprite:
   movl X, %ecx                  /* x coordinate in ecx */
   movl Y, %edi                  /* y coordinate in edi */
   movl BMP_WBANK(%edx), %esi    /* bank switch function in esi */
   movl CMP_DRAW(%ebx), %ebx     /* drawer function in ebx */

   call *%ebx                    /* and draw it! */

draw_compiled_sprite_done:
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                           /* end of draw_compiled_sprite() */


