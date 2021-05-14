#
#                 //  /     /     ,----  ,----.  ,----.  ,----.
#               / /  /     /     /      /    /  /    /  /    /
#             /  /  /     /     /___   /       /____/  /    /
#           /---/  /     /     /      /  __   /\      /    /
#         /    /  /     /     /      /    /  /  \    /    /
#       /     /  /____ /____ /____  /____/  /    \  /____/
#
#       Low Level Game Routines (version 1.0)
#
#       djgpp bitmap routines (clear, blit, textout, sprites)
#
#       See allegro.txt for instructions and copyright conditions.
# 
#       By Shawn Hargreaves,
#       1 Salisbury Road,
#       Market Drayton,
#       Shropshire,
#       England TF9 1AJ
#       email slh100@tower.york.ac.uk (until 1996)


.text


.globl _clear                 # bitmap clear routine
			      # void clear(BITMAP *bitmap);
   .align 4
_clear:
   pushl %ebp
   movl %esp, %ebp
   pushl %edi
   movl 8(%ebp), %eax         # eax = bmp
   movl 16(%eax), %ecx        # ecx = size
   shrl $2, %ecx              # quarter for 32 bit clear
   movl 28(%eax), %edi        # bitmap data
   movw 24(%eax), %ax         # segment selector
   orw %ax, %ax
   jz clear_my_seg

   pushw %es                  # clear dos memory
   movw %ax, %es
   xorl %eax, %eax
   rep ; stosl
   popw %es
   jmp clear_done

clear_my_seg:                 # else clear go32 memory
   xorl %eax, %eax
   rep ; stosl

clear_done:
   popl %edi
   movl %ebp, %esp
   popl %ebp
   ret                        # end of clear()




.globl _show                  # bitmap show routine
			      # void show(BITMAP *bitmap);
   .align 4
_show:
   pushl %ebp
   movl %esp, %ebp
   pushw %es
   pushl %esi
   pushl %edi
   movl _screen, %eax
   movl $0xa0000, %edi        # screen data
   movw 24(%eax), %ax
   movw %ax, %es              # screen segment selector
   movl 8(%ebp), %eax         # eax = bmp
   movl 16(%eax), %ecx        # ecx = size
   shrl $2, %ecx              # quarter for 32 bit clear
   movl 28(%eax), %esi        # bitmap data
   movw 24(%eax), %ax         # segment selector
   orw %ax, %ax
   jz show_my_seg

   pushw %ds                  # copy from dos memory
   movw %ax, %ds
   rep ; movsl
   popw %ds
   jmp show_done

show_my_seg:                  # else copy go32 memory
   rep ; movsl

show_done:
   popl %edi
   popl %esi
   popw %es
   movl %ebp, %esp
   popl %ebp
   ret                        # end of show()




.globl _blit                  # bitmap blit routine
			      # void blit(BITMAP *source, BITMAP *dest,
			      #           short source_x, source_y,
			      #           short dest_x, dest_y,
			      #           short width, height);
   .align 4
_blit:
   pushl %ebp
   movl %esp, %ebp
   pushw %ds
   pushw %es
   pushl %edi
   pushl %esi
   pushl %ebx

   movw 16(%ebp), %cx         # cx = source_x
   movl 8(%ebp), %ebx         # ebx = source
   cmpw %cx, (%ebx)           # if source_x > source->w
   jle blit_done
   movw 20(%ebp), %cx         # cx = source_y
   cmpw %cx, 2(%ebx)          # if source_y >= source->h
   jle blit_done
   movl 12(%ebp), %ebx        # ebx = dest
   movw 24(%ebp), %ax
   cmpw %ax, 8(%ebx)          # if dest->x >= dest->cr
   jle blit_done
   movw 28(%ebp), %ax
   cmpw %ax, 12(%ebx)         # if dest->y >= dest->cb
   jle blit_done

   cmpw $0, 16(%ebp)          # if source_x < 0
   jge blit_source_x_pve
   movw 16(%ebp), %cx
   addw %cx, 32(%ebp)         # width += source_x
   subw %cx, 24(%ebp)         # dest_x -= source_x
   movw $0, 16(%ebp)          # source_x = 0

blit_source_x_pve:
   cmpw $0, 20(%ebp)          # if source_y < 0
   jge blit_source_y_pve
   movw 20(%ebp), %bx
   addw %bx, 36(%ebp)         # height += source_y
   subw %bx, 28(%ebp)         # dest_y -= source_y
   movw $0, 20(%ebp)          # source_y = 0

blit_source_y_pve:
   movswl 32(%ebp), %eax      # eax = width
   movswl 16(%ebp), %edx      # edx = source_x
   addl %edx, %eax
   movl 8(%ebp), %ecx 
   movswl (%ecx), %edx        # edx = source->w
   cmpl %edx, %eax            # if (width + source_x) > source->w
   jle blit_width_ok
   movw (%ecx), %bx
   subw 16(%ebp), %bx
   movw %bx, 32(%ebp)         # width = source->w - source_x

blit_width_ok:
   movswl 36(%ebp), %eax      # eax = height
   movswl 20(%ebp), %edx      # edx = source_y
   addl %edx, %eax
   movl 8(%ebp), %ecx
   movswl 2(%ecx), %edx       # edx = source->h
   cmpl %edx, %eax            # if (height + source_y) > source->h
   jle blit_height_ok
   movw 2(%ecx), %bx
   subw 20(%ebp), %bx
   movw %bx, 36(%ebp)         # height = source->h - source_y

blit_height_ok:
   movl 12(%ebp), %ecx
   movw 24(%ebp), %ax
   cmpw %ax, 6(%ecx)          # if dest_x < dest->cl
   jle blit_dest_x_ok
   subw 6(%ecx), %ax          # dest_x -= dest->cl
   addw %ax, 32(%ebp)         # width += dest_x
   subw %ax, 16(%ebp)         # source_x -= dest_x in here?
   movw 6(%ecx), %ax
   movw %ax, 24(%ebp)         # dest_x = dest->cl

blit_dest_x_ok:
   movl 12(%ebp), %ebx
   movw 28(%ebp), %ax
   cmpw %ax, 10(%ebx)         # if dest_y < dest->ct
   jle blit_dest_y_ok
   subw 10(%ebx), %ax         # dest_y -= dest->ct
   addw %ax, 36(%ebp)         # height += dest_y
   subw %ax, 20(%ebp)         # source_y -= dest_y
   movw 10(%ebx), %ax
   movw %ax, 28(%ebp)         # dest_y = dest->ct

blit_dest_y_ok:
   movswl 32(%ebp), %eax      # eax = width
   movswl 24(%ebp), %edx      # edx = dest_x
   addl %edx, %eax
   movl 12(%ebp), %ecx
   movswl 8(%ecx), %edx       # edx = dest->cr
   cmpl %edx, %eax            # if (width + dest_x) > dest->cr
   jle blit_width_ok2
   movw 8(%ecx), %bx
   subw 24(%ebp), %bx
   movw %bx, 32(%ebp)         # width = dest->cr - dest_x

blit_width_ok2:
   movswl 36(%ebp), %eax      # eax = height
   movswl 28(%ebp), %edx      # edx = dest_y
   addl %edx, %eax
   movl 12(%ebp), %ecx
   movswl 12(%ecx), %edx      # edx = dest->cb
   cmpl %edx, %eax            # if (height + dest_y) > dest->cb
   jle blit_height_ok2
   movw 12(%ecx), %bx
   subw 28(%ebp), %bx
   movw %bx, 36(%ebp)         # height = dest->cb - dest_y

blit_height_ok2:
   movw 32(%ebp), %ax 
   orw %ax, %ax
   jle blit_done              # if width <= 0
   
   movw 36(%ebp), %ax 
   orw %ax, %ax
   jle blit_done              # if height <= 0

   movl 8(%ebp), %eax
   cmpl %eax, 12(%ebp)        # if source == dest
   jne blit_forwards
   
   movw 20(%ebp), %ax
   cmpw %ax, 28(%ebp)         # if source_y >= dest_y
   jg blit_backwards
   jl blit_forwards

   movw 16(%ebp), %ax
   cmpw %ax, 24(%ebp)         # if source_x >= dest_x
   jle blit_forwards


   # ************ reverse blit ************

blit_backwards: 
   movl 12(%ebp), %edi        # edi = dest bitmap
   movw 24(%edi), %cx         # cx = dest segment
   orw %cx, %cx               # test segment
   jz b_blit_d_myseg
   movw %cx, %es              # load destination segment
b_blit_d_myseg:
   movswl (%edi), %edx        # edx = dest width
   movswl 32(%ebp), %ecx
   subl %ecx, %edx            # dx -= width, = dest gap between lines
   movswl 28(%ebp), %ebx      # ebx = dest_y
   addw 36(%ebp), %bx         # dest_y + height
   decl %ebx
   movl 28(%edi, %ebx, 4), %edi     # pointer to start of dest line
   movswl 24(%ebp), %ecx      # dest_x
   addw 32(%ebp), %cx         # width
   addl %ecx, %edi            # position in the dest line
   decl %edi

   movl 8(%ebp), %esi         # esi = source bitmap
   movw 24(%esi), %cx         # cx = source segment
   movswl (%esi), %eax        # ax = source width
   movswl 32(%ebp), %ebx
   subl %ebx, %eax            # ax -= width, = source gap between lines
   movswl 20(%ebp), %ebx      # ebx = source_y
   addw 36(%ebp), %bx         # source_y + height
   decl %ebx
   movl 28(%esi, %ebx, 4), %esi     # pointer to start of source line
   movswl 16(%ebp), %ebx      # source_x
   addw 32(%ebp), %bx         # width
   addl %ebx, %esi            # position in the source line
   decl %esi
   orw %cx, %cx               # test segment
   jz b_blit_s_myseg
   movw %cx, %ds              # load source segment

b_blit_s_myseg:
   movswl 36(%ebp), %ebx      # height, y loop counter
   std                        # for backwards copy

   .align 4,0x90
blit_b_loop:
   movswl 32(%ebp), %ecx      # ecx = width = x loop counter
   rep ; movsb                # no point being clever using larger copies
			      # for reverse blit. It is rare anyway.
   subl %eax, %esi
   subl %edx, %edi            # move on to the next lines
   decl %ebx
   jg blit_b_loop             # another line?
   cld
   jmp blit_done              # finished


   # ************ forwards blit ************
   # warning: contains unrolled loops

blit_forwards:
   movl 12(%ebp), %edi        # edi = dest bitmap
   movw 24(%edi), %cx         # cx = dest segment
   orw %cx, %cx               # test segment
   jz f_blit_d_myseg
   movw %cx, %es              # load destination segment
f_blit_d_myseg:
   movswl (%edi), %edx        # edx = dest width
   movswl 32(%ebp), %ecx
   subl %ecx, %edx            # dx -= width, = dest gap between lines
   movswl 28(%ebp), %ebx      # ebx = dest_y
   movl 28(%edi, %ebx, 4), %edi     # pointer to start of dest line
   movswl 24(%ebp), %ecx
   addl %ecx, %edi            # + dest_x, = position in the dest line

   movl 8(%ebp), %esi         # esi = source bitmap
   movw 24(%esi), %cx         # cx = source segment
   movswl (%esi), %eax        # ax = source width
   movswl 32(%ebp), %ebx
   subl %ebx, %eax            # ax -= width, = source gap between lines
   movswl 20(%ebp), %ebx      # ebx = source_y
   movl 28(%esi, %ebx, 4), %esi     # pointer to start of source line
   movswl 16(%ebp), %ebx
   addl %ebx, %esi            # + source_x, = position in the source line
   orw %cx, %cx               # test segment
   jz f_blit_s_myseg
   movw %cx, %ds              # load source segment

f_blit_s_myseg:
   movswl 36(%ebp), %ebx      # height, y loop counter
   cld                        # for forward copy

   .align 4, 0x90
   shrl $1, 32(%ebp)          # halve the counter for word copy
   jz blit_f_byte_only        # and unroll the loops
   jnc blit_f_no_byte

   shrl $1, 32(%ebp)          # halve the counter again for long copy
   jz blit_f_word_byte
   jnc blit_f_long_byte

   .align 4,0x90
blit_f_long_word_byte:
   movswl 32(%ebp), %ecx      # ecx = x loop counter
   rep ; movsl
   movsw
   movsb
   addl %eax, %esi
   addl %edx, %edi            # move on to the next lines
   decl %ebx
   jg blit_f_long_word_byte   # another line?
   jmp blit_done

   .align 4,0x90
blit_f_long_byte:
   movswl 32(%ebp), %ecx      # ecx = x loop counter
   rep ; movsl
   movsb
   addl %eax, %esi
   addl %edx, %edi            # move on to the next lines
   decl %ebx
   jg blit_f_long_byte        # another line?
   jmp blit_done

   .align 4,0x90
blit_f_word_byte:
   movswl 32(%ebp), %ecx      # ecx = x loop counter
   movsw
   movsb
   addl %eax, %esi
   addl %edx, %edi            # move on to the next lines
   decl %ebx
   jg blit_f_word_byte        # another line?
   jmp blit_done

   .align 4,0x90
blit_f_byte_only:
   movswl 32(%ebp), %ecx      # ecx = x loop counter
   movsb
   addl %eax, %esi
   addl %edx, %edi            # move on to the next lines
   decl %ebx
   jg blit_f_byte_only        # another line?
   jmp blit_done

   .align 4,0x90
blit_f_no_byte:
   shrl $1, 32(%ebp)          # halve the counter again for long copy
   jz blit_f_word_only
   jnc blit_f_long_only

   .align 4,0x90
blit_f_long_word:
   movswl 32(%ebp), %ecx      # ecx = x loop counter
   rep ; movsl
   movsw
   addl %eax, %esi
   addl %edx, %edi            # move on to the next lines
   decl %ebx
   jg blit_f_long_word        # another line?
   jmp blit_done

   .align 4,0x90
blit_f_long_only:
   movswl 32(%ebp), %ecx      # ecx = x loop counter
   rep ; movsl
   addl %eax, %esi
   addl %edx, %edi            # move on to the next lines
   decl %ebx
   jg blit_f_long_only        # another line?
   jmp blit_done

   .align 4,0x90
blit_f_word_only:
   movswl 32(%ebp), %ecx      # ecx = x loop counter
   movsw
   addl %eax, %esi
   addl %edx, %edi            # move on to the next lines
   decl %ebx
   jg blit_f_word_only        # another line?

   .align 4,0x90
blit_done:
   popl %ebx
   popl %esi
   popl %edi
   popw %es
   popw %ds
   movl %ebp, %esp
   popl %ebp
   ret                        # end of blit()




.globl _get_sprite            # get sprite from bitmap
			      # void get_sprite(SPRITE *sprite, BITMAP *bmp,
			      # short x, y)
   .align 4
_get_sprite:
   pushl %ebp
   movl %esp, %ebp
   subl $12, %esp             # three local variables:
			      # -4 = yend, -8 = swidth, -12 = sgap
   pushl %edi
   pushl %esi
   pushl %ebx
   pushw %ds
   
   cld
   movl 12(%ebp), %esi        # esi = bmp
   movw 24(%esi), %ax         # segment selector
   orw %ax, %ax
   jz getsprite_myseg
   movw %ax, %ds              # read from DOS memory
getsprite_myseg:
   movl 8(%ebp), %edi         # edi = sprite pointer
   movswl 20(%ebp), %ebx      # ebx = y
   movswl %es:4(%edi), %eax   # eax = sprite->h
   addl %ebx, %eax
   movl %eax, -4(%ebp)        # calculate y end point

   movswl %es:2(%edi), %edx   # edx = sprite->w
   movl %edx, -8(%ebp)        # swidth
   movl $0, -12(%ebp)         # clear sgap
   addl $8, %edi              # start address of sprite data

   movswl 16(%ebp), %eax      # eax = x
   cmpl $0, %eax
   jae getsprite_l_ok

   negl %eax                  # clip the left
   movl %eax, -12(%ebp)
   addl %eax, %edi
   subl %eax, %edx
   jl getsprite_done
   xorl %eax, %eax

getsprite_l_ok:
   movl %eax, %ecx            # x
   addl -8(%ebp), %ecx        # + width
   cmpw %es:(%esi), %cx       # compare with bmp->w
   jl getsprite_y_loop_start

   subw %es:(%esi), %cx       # clip the right
   subl %ecx, %edx
   jle getsprite_done
   addl %ecx, -12(%ebp)
   jmp getsprite_y_loop_start

   .align 4,0x90
getsprite_y_loop:
   movl 12(%ebp), %esi        # esi = bmp
   cmpw %es:2(%esi), %bx      # bx is y pos, compare with bmp->h
   jae getsprite_bad_y
   cmpw $0, %bx
   jl getsprite_bad_y

   movl %es:28(%esi, %ebx, 4), %esi
   addl %eax, %esi            # get the pixel address

   movl %edx, %ecx            # cx = sprite->w
   rep ; movsb                # get the data
   addl -12(%ebp), %edi

getsprite_line_done:
   incl %ebx                  # increment y loop counter
getsprite_y_loop_start:
   cmpl -4(%ebp), %ebx        # loop?
   jl getsprite_y_loop

getsprite_done:
   popw %ds
   popl %ebx
   popl %esi
   popl %edi
   movl %ebp, %esp
   popl %ebp
   ret                        # end of get_sprite()

   .align 4,0x90
getsprite_bad_y:              # for if the line is not on the bitmap
   addl -8(%ebp), %edi
   jmp getsprite_line_done





   # Warning: this routine contains heavily
   # expanded inner loops which make it totally
   # unreadable. Sorry but that was the only
   # way I could get it fast enough.

.globl _drawsprite            # draw sprite to bitmap
			      # void drawsprite(BITMAP *bmp, SPRITE *sprite,
			      # short x, y)
   .align 4
_drawsprite:
   pushl %ebp
   movl %esp, %ebp
   subl $16, %esp             # three local variables:
			      # -4 = tgap, -8 = lgap, -12 = w, -16 = h
   pushl %edi
   pushl %esi
   pushl %ebx
   pushw %es

   movl 8(%ebp), %edi         # edi = bitmap pointer
   movl 12(%ebp), %esi        # esi = sprite pointer

   movw 24(%edi), %ax         # segment selector
   orw %ax, %ax
   jz sprite_my_seg
   movw %ax, %es              # write to DOS memory

sprite_my_seg:
   cmpw $0, 4(%edi)           # test bmp->clip
   jz sprite_no_clip

   movw 10(%edi), %ax         # bmp->ct
   subw 20(%ebp), %ax         # ax -= y
   jge sprite_tgap_ok
   xorw %ax, %ax
sprite_tgap_ok: 
   cwde
   movl %eax, -4(%ebp)        # set tgap
   movw 4(%esi), %bx          # sprite->h
   movw 12(%edi), %cx         # bmp->cb
   subw 20(%ebp), %cx         # cx -= y
   cmpw %bx, %cx              # check bottom clipping
   jg sprite_height_ok
   movw %cx, %bx
sprite_height_ok:
   subw %ax, %bx              # height -= tgap
   jle sprite_done
   movswl %bx, %ebx
   movl %ebx, -16(%ebp)       # set h

   movw 6(%edi), %ax          # bmp->cl
   subw 16(%ebp), %ax         # ax -= x
   jge sprite_lgap_ok
   xorw %ax, %ax
sprite_lgap_ok: 
   cwde
   movl %eax, -8(%ebp)        # set lgap

   movw 2(%esi), %bx          # sprite->w
   movw 8(%edi), %cx          # bmp->cr
   subw 16(%ebp), %cx         # cx -= x
   cmpw %bx, %cx              # check bottom clipping
   jg sprite_width_ok
   movw %cx, %bx
sprite_width_ok:
   subw %ax, %bx              # width -= lgap
   jle sprite_done
   movswl %bx, %ebx
   movl %ebx, -12(%ebp)       # set w
   jmp sprite_clip_done

   .align 4,0x90
sprite_no_clip:
   movl $0, -4(%ebp)
   movl $0, -8(%ebp)
   movswl 2(%esi), %eax
   movl %eax, -12(%ebp)       # w = sprite->w
   movswl 4(%esi), %eax
   movl %eax, -16(%ebp)       # h = sprite->h

sprite_clip_done:
   testw $1, (%esi)           # sprite masked?
   jnz sprite_masked_draw


   # --------------------------
   # opaque sprite draw routine
   # --------------------------

   movl -12(%ebp), %ebx       # ebx = w
   movswl (%edi), %edx        # bmp->w
   subl %ebx, %edx            # edx = bitmap gap

   movswl 2(%esi), %ecx       # sprite->w
   movl %ecx, %eax
   subl %ebx, %eax            # eax = sprite gap

   movl -4(%ebp), %ebx        # ebx = tgap
sprite_o_tgap_loop:
   orl %ebx, %ebx
   jz sprite_o_no_tgap
   addl %ecx, %esi            # + width
   decl %ebx
   jmp sprite_o_tgap_loop

   .align 4,0x90
sprite_o_no_tgap:
   addl -8(%ebp), %esi        # +lgap
   addl $8, %esi              # get position in sprite data

   movswl 20(%ebp), %ebx      # ebx = y
   addl -4(%ebp), %ebx        # + tgap

   movl 28(%edi, %ebx, 4), %edi
   movswl 16(%ebp), %ebx
   addl %ebx, %edi
   addl -8(%ebp), %edi        # get position in bitmap

   movl -16(%ebp), %ebx       # y loop counter
   cld

   shrl $1, -12(%ebp)         # convert byte count to word count
   jz sprite_o_no_word_loop   # which draw should we use?
   jnc sprite_o_no_byte       # the inner loop is expanded for all
			      # the different combinations of byte,
			      # word and long copies. Ugly but fast.

   shrl $1, -12(%ebp)         # convert word count to long count
   jz sprite_o_no_long_word_byte_loop
   jnc sprite_o_no_word_one_byte_loop

   .align 4,0x90 
sprite_o_one_word_one_byte_loop:
   movl -12(%ebp), %ecx       # x loop counter
   rep ; movsl                # copy the longs
   movsw                      # copy a word
   movsb                      # and copy a byte
   addl %eax, %esi            # skip some bytes
   addl %edx, %edi
   decl %ebx
   jg sprite_o_one_word_one_byte_loop
   jmp sprite_done

   .align 4,0x90 
sprite_o_no_word_one_byte_loop:
   movl -12(%ebp), %ecx       # x loop counter
   rep ; movsl                # copy the longs
   movsb                      # and copy a byte
   addl %eax, %esi            # skip some bytes
   addl %edx, %edi
   decl %ebx
   jg sprite_o_no_word_one_byte_loop
   jmp sprite_done

   .align 4,0x90
sprite_o_no_byte:             # fast routine for even number of bytes
   shrl $1, -12(%ebp)         # convert word count to long count
   jz sprite_o_one_word_loop
   jnc sprite_o_no_word_or_byte_loop

   .align 4,0x90
sprite_o_long_word_no_byte_loop:
   movl -12(%ebp), %ecx       # x loop counter
   rep ; movsl                # copy the longs
   movsw                      # copy the word
   addl %eax, %esi            # skip some bytes
   addl %edx, %edi
   decl %ebx
   jg sprite_o_long_word_no_byte_loop 
   jmp sprite_done

   .align 4,0x90
sprite_o_no_word_or_byte_loop:
   movl -12(%ebp), %ecx       # x loop counter
   rep ; movsl                # copy the words
   addl %eax, %esi            # skip some bytes
   addl %edx, %edi
   decl %ebx
   jg sprite_o_no_word_or_byte_loop 
   jmp sprite_done

   .align 4,0x90
sprite_o_one_word_loop:       # in case there are no longs at all
   movsw                      # copy a word
   addl %eax, %esi            # skip some bytes
   addl %edx, %edi
   decl %ebx
   jg sprite_o_one_word_loop
   jmp sprite_done

   .align 4,0x90
sprite_o_no_word_loop:        # in case there are no words at all
   movsb                      # copy a byte
   addl %eax, %esi            # skip some bytes
   addl %edx, %edi
   decl %ebx
   jg sprite_o_no_word_loop
   jmp sprite_done

   .align 4,0x90              # in case there are no longs at all
sprite_o_no_long_word_byte_loop:
   movsw                      # copy a word
   movsb                      # copy a byte
   addl %eax, %esi            # skip some bytes
   addl %edx, %edi
   decl %ebx
   jg sprite_o_no_long_word_byte_loop
   jmp sprite_done


   # --------------------------
   # masked sprite draw routine
   # --------------------------

   .align 4,0x90
sprite_masked_draw:
   movl -12(%ebp), %eax       # eax = w
   movswl (%edi), %edx        # bmp->w
   subl %eax, %edx            # edx = bitmap gap

   movswl 2(%esi), %ecx       # sprite->w
   movl %ecx, %ebx
   subl %eax, %ebx            # ebx = sprite gap

   movl -4(%ebp), %eax        # eax = tgap
sprite_m_tgap_loop:
   orl %eax, %eax
   jz sprite_m_no_tgap
   addl %ecx, %esi            # + width
   decl %eax
   jmp sprite_m_tgap_loop

   .align 4,0x90
sprite_m_no_tgap:
   addl -8(%ebp), %esi        # +lgap
   addl $8, %esi              # get position in sprite data

   movswl 20(%ebp), %eax      # eax = y
   addl -4(%ebp), %eax        # + tgap

   movl 28(%edi, %eax, 4), %edi
   movswl 16(%ebp), %eax
   addl %eax, %edi
   addl -8(%ebp), %edi        # get position in bitmap
   cld

   shrl $1, -12(%ebp)         # halve counter for word copies
   jz sprite_m_no_word_loop
   jnc sprite_m_no_byte
   
sprite_m_byte_word_loop:      # for an odd number of bytes
   movl -12(%ebp), %ecx       # x loop counter
sprite_m_byte_word_x_loop:
   movw (%esi), %ax           # read two pixels
   orb %al, %al               # test pixel 1
   jz sprite_m_byte_word_skip1
   movb %al, %es:(%edi)       # write pixel 1
sprite_m_byte_word_skip1:
   incl %edi
   orb %ah, %ah               # test pixel 2
   jz sprite_m_byte_word_skip2
   movb %ah, %es:(%edi)       # write pixel 2
sprite_m_byte_word_skip2:
   incl %edi
   addl $2, %esi
   loop sprite_m_byte_word_x_loop

   movb (%esi), %al           # read the odd pixels
   orb %al, %al               # test pixel
   jz sprite_m_byte_word_oddskip
   movb %al, %es:(%edi)       # write pixel
sprite_m_byte_word_oddskip:
   incl %esi
   incl %edi

   addl %ebx, %esi            # skip some bytes
   addl %edx, %edi
   decl -16(%ebp)             # y loop counter
   jg sprite_m_byte_word_loop
   jmp sprite_done

   .align 4,0x90
sprite_m_no_byte:             # for an even number of bytes
   shrl $1, -12(%ebp)         # halve counter for long copies
   jz sprite_m_no_long_loop
   jnc sprite_m_long_loop

   .align 4,0x90
sprite_m_word_loop:           # for longs + word
   movl -12(%ebp), %ecx       # x loop counter
sprite_m_word_x_loop:
   movl (%esi), %eax          # read four pixels
   orb %al, %al               # test pixel 1
   jz sprite_m_word_skip1
   movb %al, %es:(%edi)       # write pixel 1
sprite_m_word_skip1:
   incl %edi
   orb %ah, %ah               # test pixel 2
   jz sprite_m_word_skip2
   movb %ah, %es:(%edi)       # write pixel 2
sprite_m_word_skip2:
   incl %edi
   shrl $16, %eax
   orb %al, %al               # test pixel 3
   jz sprite_m_word_skip3
   movb %al, %es:(%edi)       # write pixel 3
sprite_m_word_skip3:
   incl %edi
   orb %ah, %ah               # test pixel 4
   jz sprite_m_word_skip4
   movb %ah, %es:(%edi)       # write pixel 4
sprite_m_word_skip4:
   incl %edi
   addl $4, %esi
   loop sprite_m_word_x_loop

   movw (%esi), %ax           # read two odd pixels
   orb %al, %al               # test pixel 1
   jz sprite_m_word_skip11
   movb %al, %es:(%edi)       # write pixel 1
sprite_m_word_skip11:
   incl %edi
   orb %ah, %ah               # test pixel 2
   jz sprite_m_word_skip22
   movb %ah, %es:(%edi)       # write pixel 2
sprite_m_word_skip22:
   incl %edi
   addl $2, %esi
   addl %ebx, %esi            # skip some bytes
   addl %edx, %edi
   decl -16(%ebp)             # y loop counter
   jg sprite_m_word_loop
   jmp sprite_done

   .align 4,0x90
sprite_m_long_loop:           # for even number of longs
   movl -12(%ebp), %ecx       # x loop counter
sprite_m_long_x_loop:
   movl (%esi), %eax          # read four pixels
   orb %al, %al               # test pixel 1
   jz sprite_m_long_skip1
   movb %al, %es:(%edi)       # write pixel 1
sprite_m_long_skip1:
   incl %edi
   orb %ah, %ah               # test pixel 2
   jz sprite_m_long_skip2
   movb %ah, %es:(%edi)       # write pixel 2
sprite_m_long_skip2:
   incl %edi
   shrl $16, %eax
   orb %al, %al               # test pixel 3
   jz sprite_m_long_skip3
   movb %al, %es:(%edi)       # write pixel 3
sprite_m_long_skip3:
   incl %edi
   orb %ah, %ah               # test pixel 4
   jz sprite_m_long_skip4
   movb %ah, %es:(%edi)       # write pixel 4
sprite_m_long_skip4:
   incl %edi
   addl $4, %esi
   loop sprite_m_long_x_loop

   addl %ebx, %esi            # skip some bytes
   addl %edx, %edi
   decl -16(%ebp)             # y loop counter
   jg sprite_m_long_loop
   jmp sprite_done

   .align 4,0x90
sprite_m_no_word_loop:        # for if we only have one byte
   movb (%esi), %al           # read a pixel
   orb %al, %al               # test it
   jz sprite_m_no_word_skip
   movb %al, %es:(%edi)       # write it
sprite_m_no_word_skip:
   incl %esi
   incl %edi
   addl %ebx, %esi            # skip some bytes
   addl %edx, %edi
   decl -16(%ebp)             # y loop counter
   jg sprite_m_no_word_loop
   jmp sprite_done

   .align 4,0x90
sprite_m_no_long_loop:        # for if we only have one word
   movw (%esi), %ax           # read a pixel
   orb %al, %al               # test 1
   jz sprite_m_no_long_skip1
   movb %al, %es:(%edi)       # write 1
sprite_m_no_long_skip1:
   incl %edi
   orb %ah, %ah               # test 2
   jz sprite_m_no_long_skip2
   movb %ah, %es:(%edi)       # write 2
sprite_m_no_long_skip2:
   incl %edi
   addl $2, %esi
   addl %ebx, %esi            # skip some bytes
   addl %edx, %edi
   decl -16(%ebp)             # y loop counter
   jg sprite_m_no_long_loop


   .align 4,0x90
sprite_done:
   popw %es
   popl %ebx
   popl %esi
   popl %edi
   movl %ebp, %esp
   popl %ebp
   ret                        # end of drawsprite()




.globl _textout               # text output routine
			      # void textout(BITMAP *bmp, FONT *font,
			      # char *str, short x, y, color)
   .align 4
_textout:
   pushl %ebp
   movl %esp, %ebp
   subl $24, %esp             # 6 local variables:
			      # -4 = tgap, -8 = height, -12 = lgap,
			      # -16 = rgap, -20 = char_w, -24 = c
   pushl %edi
   pushl %esi
   pushl %ebx
   pushw %es
   
   movl 8(%ebp), %esi         # esi = bmp
   movw 24(%esi), %ax         # segment selector
   orw %ax, %ax
   jz text_my_seg
   movw %ax, %es              # write to DOS memory

text_my_seg:
   cmpw $0, 4(%esi)           # test bmp->clip
   jz text_no_clip 

   movswl 24(%ebp), %eax      # eax = y
   movswl 10(%esi), %edx      # edx = bmp->ct
   subl %eax, %edx            # edx -= y
   cmpl $8, %edx
   jge text_done
   orl %edx, %edx
   jge text_tgap_pve
   xorl %edx, %edx
text_tgap_pve:
   movl %edx, -4(%ebp)        # set tgap
   addl %edx, %eax            # y += tgap
   movw %ax, 24(%ebp)         # store y

   movswl 12(%esi), %ebx      # ebx = bmp->cb
   subl %eax, %ebx            # ebx -= y
   jle text_done

   negl %edx
   addl $8, %edx              # edx = 8 - tgap
   cmpl %ebx, %edx            # if height > 8 - tgap
   jg text_height_small
   movl %edx, %ebx
text_height_small:
   movl %ebx, -8(%ebp)        # store height

   movswl 6(%esi), %eax       # eax = bmp->cl
   movswl 20(%ebp), %edx      # edx = x
   subl %edx, %eax            # eax = bmp->cl - x
   jge text_lgap_pve
   xorl %eax, %eax
text_lgap_pve:
   movl 16(%ebp), %ebx        # ebx = str
text_lclip_loop:
   cmpl $8, %eax              # while eax >= 8
   jl text_lclip_done
   cmpb $0, (%ebx)            # if !*str
   jz text_done
   incl %ebx
   addw $8, 20(%ebp)
   subl $8, %eax
   jmp text_lclip_loop
   .align 4,0x90
text_lclip_done:
   movl %ebx, 16(%ebp)        # store str
   movl %eax, -12(%ebp)       # store lgap

   movswl 20(%ebp), %eax      # x
   movswl 8(%esi), %edx       # bmp->cr
   subl %eax, %edx
   jl text_done
   movl %edx, -16(%ebp)       # set rgap
   jmp text_char_loop_start

   .align 4,0x90
text_no_clip:
   movl $0, -4(%ebp)
   movl $8, -8(%ebp)
   movl $0, -12(%ebp)
   movl $0x7fff, -16(%ebp)
   jmp text_char_loop_start

   .align 4,0x90
text_char_loop:               # for each character...
   cmpb $126, %al             # the char will already be in al
   ja text_char_no_good
   subb $32, %al              # convert to table offset
   jl text_char_no_good
   andl $0xff, %eax
   jmp text_char_sorted

   .align 4,0x90
text_char_no_good:
   xorl %eax, %eax            # oops - not ASCII

   .align 4,0x90
text_char_sorted:
   movl 12(%ebp), %esi        # esi = font
   shll $3, %eax
   addl %eax, %esi
   addl -4(%ebp), %esi        # esi = position in font bitmap

   movl -16(%ebp), %eax       # rgap
   cmpl $0, %eax
   jle text_done              # have we gone off the right?
   cmpl $8, %eax
   jle text_rgap_ok
   movl $8, %eax              # dont want chars wider than 8!
text_rgap_ok:
   movl %eax, -20(%ebp)       # set char width
   xorl %ebx, %ebx
   jmp text_y_loop_start

   .align 4,0x90
text_y_loop:                  # for each y...
   movswl 24(%ebp), %edx      # y
   addl %edx, %ebx            # c will already be in ebx
   movl 8(%ebp), %edi         # bmp
   movl 28(%edi, %ebx, 4), %edi
   movswl 20(%ebp), %edx      # x
   addl %edx, %edi

   movb (%esi), %dl           # dl = bit mask
   incl %esi

   movl -12(%ebp), %ecx       # lgap
   orl %ecx, %ecx
   jz text_no_lgap            # do we need to clip on the left?

   shlb %cl, %dl              # shift the mask
   addl %ecx, %edi            # move the screen position
   negl %ecx

   .align 4,0x90
text_no_lgap:
   addl -20(%ebp), %ecx       # cx = x loop counter
   jle text_no_x_loop

   movw 28(%ebp), %ax         # ax = text color
   movw __textmode, %bx       # bx = background color

   .align 4,0x90
text_x_loop:                  # for each x...
   shlb $1, %dl               # shift the mask
   jc text_put_bit

   orw %bx, %bx
   jl text_put_done
   movb %bl, %es:(%edi)       # draw background pixel
   jmp text_put_done
 
   .align 4,0x90
text_put_bit: 
   movb %al, %es:(%edi)       # draw a pixel

text_put_done:
   incl %edi
   loop text_x_loop           # and loop

text_no_x_loop:
   movl -24(%ebp), %ebx       # increment loop counter
   incl %ebx
text_y_loop_start:
   movl %ebx, -24(%ebp)
   cmpl -8(%ebp), %ebx
   jl text_y_loop

text_y_loop_done:
   movl $0, -12(%ebp)         # sort out a load of variables
   subl $8, -16(%ebp)
   addw $8, 20(%ebp)

   incl 16(%ebp)              # move on to the next character
text_char_loop_start:
   movl 16(%ebp), %ebx        # read a char into al
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
   ret                        # end of textout()


