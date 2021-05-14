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
#       djgpp graphics primitives (lines, rectangles, circles, etc)
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


.globl _vsync              # vsync routine
			   # void vsync();
   .align 4
_vsync:
   pushl %ebp
   movl %esp, %ebp
   movw $0x3DA, %dx        # vsync port
vs_loop1:
   inb %dx, %al
   testb $8, %al
   jnz vs_loop1            # wait for retrace to end
vs_loop2:
   inb %dx, %al
   testb $8, %al
   jz vs_loop2             # wait for retrace to start again
   movl %ebp, %esp
   popl %ebp
   ret                     # end of vsync()




.globl _set_pallete        # pallete setting routine
			   # void set_pallete(PALLETE *p);
   .align 4
_set_pallete:
   pushl %ebp
   movl %esp, %ebp
   pushl %esi

   call _vsync

   movl 8(%ebp), %esi      # esi is start of pallete
   movl $0, %eax           # eax = pallete position
   movl $128, %ecx         # ecx = loop counter
   movw $0x3C9, %dx        # dx = port index
   cld

pal_loop:
   decw %dx
   outb %al, %dx           # output pallete index
   incw %dx

   outsb                   # output r
   outsb                   # output g
   outsb                   # output b

   incl %eax               # next pallete entry
   loop pal_loop

   call _vsync

   movl $128, %eax         # eax = pallete position
   movl %eax, %ecx         # ecx = loop counter
   movw $0x3C9, %dx        # dx = port index

pal_loop2:
   decw %dx
   outb %al, %dx           # output pallete index
   incw %dx

   outsb                   # output r
   outsb                   # output g
   outsb                   # output b

   incl %eax               # next pallete entry
   loop pal_loop2

   popl %esi
   movl %ebp, %esp
   popl %ebp
   ret                     # end of set_pallete()




.globl _get_pallete        # pallete getting routine
			   # void get_pallete(PALLETE *p);
   .align 4
_get_pallete:
   pushl %ebp
   movl %esp, %ebp
   pushl %edi

   movl 8(%ebp), %edi      # edi is start of pallete
   movl $0, %eax           # eax = pallete position
   movl $256, %ecx         # ecx = loop counter
   cld

get_pal_loop:
   movw $0x3C7, %dx
   outb %al, %dx           # output index
   movw $0x3C9, %dx

   insb                    # get r
   insb                    # get g
   insb                    # get b

   incl %eax               # next pallete entry
   loop get_pal_loop

   popl %edi
   movl %ebp, %esp
   popl %ebp
   ret                     # end of get_pallete()




.globl _putpixel              # putpixel routine
			      # void putpixel(BITMAP *bmp, short x,
			      #               short y, short color);
   .align 4
_putpixel:
   pushl %ebp
   movl %esp, %ebp
   pushl %ebx
   xorl %eax, %eax
   xorl %edx, %edx
   movl 8(%ebp), %ecx         # ecx = bmp
   movw 12(%ebp), %dx         # dx = x
   movw 16(%ebp), %ax         # ax = y
   movw 20(%ebp), %bx         # bx = color

   cmpw $0, 4(%ecx)           # test clip
   je putpix_noclip
   cmpw %dx, 6(%ecx)          # test cl
   jg putpix_done
   cmpw %dx, 8(%ecx)          # test cr
   jle putpix_done
   cmpw %ax, 10(%ecx)         # test ct
   jg putpix_done
   cmpw %ax, 12(%ecx)         # test cb
   jle putpix_done

putpix_noclip:
   movl 28(%ecx, %eax, 4), %eax
   addl %edx, %eax            # get the pixel address
   movw 24(%ecx), %dx         # segment selector
   orw %dx, %dx
   jz putpix_my_seg

   pushw %es                  # write pixel to dos memory
   movw %dx, %es
   movb %bl, %es:(%eax)       # store the pixel
   popw %es
   jmp putpix_done

putpix_my_seg:                # else write pixel to go32 memory
   movb %bl, (%eax)

putpix_done:
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                        # end of putpixel()




.globl _getpixel              # getpixel routine
			      # short getpixel(BITMAP *bmp, short x, short y);
   .align 4
_getpixel:
   pushl %ebp
   movl %esp, %ebp
   xorl %eax, %eax
   xorl %edx, %edx
   movl 8(%ebp), %ecx         # ecx = bmp
   movw 12(%ebp), %ax         # ax = x
   movw 16(%ebp), %dx         # dx = y
   testw %ax, %ax             # test that we are on the bitmap
   jl getpix_skip
   cmpw %ax, (%ecx)
   jle getpix_skip
   testw %dx, %dx
   jl getpix_skip
   cmpw %dx, 2(%ecx)
   jg getpix_ok

getpix_skip:                  # return -1 if we are off the edge
   movl $-1, %eax
   jmp getpix_done

getpix_ok:
   movl 28(%ecx, %edx, 4), %edx
   addl %eax, %edx            # get the pixel address
   movw 24(%ecx), %ax         # segment selector
   orw %ax, %ax
   jz getpix_my_seg

   pushw %es                  # get pixel from dos memory
   movw %ax, %es
   xorl %eax, %eax
   movb %es:(%edx), %al
   popw %es
   jmp getpix_done

getpix_my_seg:                # else get pixel from go32 memory
   xorl %eax, %eax
   movb (%edx), %al

getpix_done:
   movl %ebp, %esp
   popl %ebp
   ret                        # end of getpixel()




.globl _hline              # horizontal line routine
			   # void hline(BITMAP *bmp, short x1, y, x2, color);
   .align 4
_hline:
   pushl %ebp
   movl %esp, %ebp
   pushl %ebx
   pushl %edi

   movl 8(%ebp), %edi      # edi = bmp
   movswl 12(%ebp), %edx   # edx = x1
   movswl 16(%ebp), %ebx   # ebx = y
   movswl 20(%ebp), %ecx   # ecx = x2
   cmpl %edx, %ecx
   jge hline_no_xswap
   xchgl %ecx, %edx

hline_no_xswap:
   cmpw $0, 4(%edi)        # test bmp->clip
   je hline_noclip

   cmpw 10(%edi), %bx      # test bmp->ct
   jl hline_done

   cmpw 12(%edi), %bx      # test bmp->cb
   jge hline_done

   cmpw 6(%edi), %dx       # test x1, bmp->cl
   jge hline_x1_ok
   cmpw 6(%edi), %cx       # test x2, bmp->cl
   jl hline_done
   movswl 6(%edi), %edx    # clip x1

hline_x1_ok:
   cmpw 8(%edi), %cx       # test x2, bmp->cr
   jl hline_noclip
   cmpw 8(%edi), %dx       # test x1, bmp->cr
   jge hline_done
   movswl 8(%edi), %ecx    # clip x2
   decl %ecx

hline_noclip:
   subl %edx, %ecx         # loop counter
   incl %ecx

   movw 24(%edi), %ax      # segment selector
   movl 28(%edi, %ebx, 4), %edi
   addl %edx, %edi         # get the pixel address

   orw %ax, %ax            # check segment
   jz hline_my_seg

   pushw %es               # write to dos memory
   movw %ax, %es           # load segment
   movb 24(%ebp), %dl      # get color
   movb %dl, %dh
   shrdl $16, %edx, %eax
   movw %dx, %ax           # get four copies of the color
   cld

   testw $1, %di           # are we word aligned?
   jz hline1_w_aligned
   stosb                   # if not, copy a pixel
   decl %ecx
hline1_w_aligned:
   testw $2, %di           # are we long aligned?
   jz hline1_l_aligned
   subl $2, %ecx
   jl hline1_oops
   stosw                   # if not, copy a word
hline1_l_aligned:
   movw %cx, %dx           # store low bits
   shrl $2, %ecx           # for long copy
   jz hline1_no_long
   rep ; stosl             # do some 32 bit copies
hline1_no_long:
   testw $2, %dx
   jz hline1_no_word
   stosw                   # do we need a 16 bit copy?
hline1_no_word:
   testw $1, %dx
   jz hline1_done
   stosb                   # do we need a byte copy?

hline1_done:
   popw %es
   popl %edi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret

hline1_oops:               # fix up ecx for short line?
   addl $2, %ecx
   jmp hline1_l_aligned

   .align 4
hline_my_seg:              # write to go32 memory
   movb 24(%ebp), %dl      # get color
   movb %dl, %dh
   shrdl $16, %edx, %eax
   movw %dx, %ax           # get four copies of the color
   cld

   testw $1, %di           # are we word aligned?
   jz hline2_w_aligned
   stosb                   # if not, copy a pixel
   decl %ecx
hline2_w_aligned:
   testw $2, %di           # are we long aligned?
   jz hline2_l_aligned
   subl $2, %ecx
   jl hline2_oops
   stosw                   # if not, copy a word
hline2_l_aligned:
   movw %cx, %dx           # store low bits
   shrl $2, %ecx           # for long copy
   jz hline2_no_long
   rep ; stosl             # do some 32 bit copies
hline2_no_long:
   testw $2, %dx
   jz hline2_no_word
   stosw                   # do we need a 16 bit copy?
hline2_no_word:
   testw $1, %dx
   jz hline_done
   stosb                   # do we need a byte copy?

hline_done:
   popl %edi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                     # end of hline()

hline2_oops:               # fix up ecx for short line?
   addl $2, %ecx
   jmp hline2_l_aligned




.globl _vline              # vertical line routine
			   # void vline(BITMAP *bmp, short x, y1, y2, color);
   .align 4
_vline:
   pushl %ebp
   movl %esp, %ebp
   pushl %ebx
   pushl %esi
   pushl %edi

   movl 8(%ebp), %esi      # esi = bmp
   movswl 12(%ebp), %ebx   # ebx = x
   movswl 16(%ebp), %edx   # edx = y1
   movswl 20(%ebp), %ecx   # ecx = y2
   cmpl %edx, %ecx
   jge vline_no_xswap
   xchgl %ecx, %edx

vline_no_xswap:
   cmpw $0, 4(%esi)        # test bmp->clip
   je vline_noclip

   cmpw 6(%esi), %bx       # test bmp->cl
   jl vline_done
   cmpw 8(%esi), %bx       # test bmp->cr
   jge vline_done
   cmpw 10(%esi), %dx      # test y1, bmp->ct
   jge vline_y1_ok
   cmpw 10(%esi), %cx      # test y2, bmp->ct
   jl vline_done
   movswl 10(%esi), %edx   # clip y1

vline_y1_ok:
   cmpw 12(%esi), %cx      # test y2, bmp->cb
   jl vline_noclip
   cmpw 12(%esi), %dx      # test y1, bmp->cb
   jge vline_done
   movswl 12(%esi), %ecx   # clip y2
   decl %ecx

vline_noclip:
   subl %edx, %ecx         # loop counter
   incl %ecx
   movl 28(%esi, %edx, 4), %edi
   addl %ebx, %edi         # get the pixel address
   movzwl (%esi), %edx     # gap between lines
   movw 24(%esi), %ax      # segment selector
   orw %ax, %ax            # check segment
   jz vline_my_seg

   pushw %es               # write to dos memory
   movw %ax, %es           # load segment
   movb 24(%ebp), %al      # get color

   .align 4, 0x90
vline1_loop:
   movb %al, %es:(%edi)    # store a pixel
   addl %edx, %edi         # next line
   loop vline1_loop

   popw %es
   jmp vline_done

   .align 4
vline_my_seg:              # write to go32 memory
   movb 24(%ebp), %al      # get color

   .align 4, 0x90
vline2_loop:
   movb %al, (%edi)        # store a pixel
   addl %edx, %edi         # next line
   loop vline2_loop

vline_done:
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                     # end of vline()




.globl _line            # fixed point line draw routine
			# void line(BITMAP *bmp, short x1, y1, x2, y2, color)
   .align 4
_line:
   pushl %ebp
   movl %esp, %ebp
   pushl %edi
   pushl %esi
   pushl %ebx

   movl 8(%ebp), %esi         # esi = bmp
   movw 20(%ebp), %ax         # ax = x2
   subw 12(%ebp), %ax         # x2 - x1
   jnz line_not_v

   movswl 28(%ebp), %eax
   pushl %eax                 # push color
   movswl 24(%ebp), %ecx
   pushl %ecx                 # push y2
   movswl 16(%ebp), %eax
   pushl %eax                 # push y1
   movswl 12(%ebp), %ecx
   pushl %ecx                 # push x1
   pushl %esi                 # push bmp
   call _vline                # draw a vertical line
   addl $20, %esp             # restore stack
   jmp line_done

   .align 4, 0x90
line_not_v:
   jge line_dx_pve
   negw %ax                   # ax = abs(dx)
line_dx_pve:
   movw 24(%ebp), %bx         # bx = y2
   subw 16(%ebp), %bx         # x2 - x1
   jnz line_not_h

   movswl 28(%ebp), %eax
   pushl %eax                 # push color
   movswl 20(%ebp), %ecx
   pushl %ecx                 # push x2
   movswl 16(%ebp), %eax
   pushl %eax                 # push y1
   movswl 12(%ebp), %ecx
   pushl %ecx                 # push x1
   pushl %esi                 # push bmp
   call _hline                # draw a horizontal line
   addl $20, %esp             # restore stack
   jmp line_done

   .align 4, 0x90
line_not_h:
   jge line_dy_pve
   negw %bx                   # bx = abs(dy)
line_dy_pve:
   cmpw %ax, %bx              # if abs(dx) > abs(dy)
   jg line_y_driven
			      # x-driven fixed point line draw
   pushw %es
   movw 24(%esi), %ax         # segment selector
   orw %ax, %ax
   jz linex_myseg
   movw %ax, %es              # load dos segment
linex_myseg:
   movswl 12(%ebp), %edx      # edx = x1
   movswl 16(%ebp), %ebx      # ebx = y1
   movswl 20(%ebp), %ecx      # ecx = x2
   movswl 24(%ebp), %eax      # eax = y2
   subl %edx, %ecx            # ecx = x2 - x1 = counter
   jg linex_noswap
   negl %ecx                  # make count positive
   xchgl %eax, %ebx           # swap y values
   movw 20(%ebp), %dx
   movw %dx, 12(%ebp)         # swap start x value
linex_noswap:
   sall $20, %eax             # make y2 fixed point
   sall $20, %ebx             # make y1 fixed point
   subl %ebx, %eax            # eax = y2 - y1 = dy
   cdq                        # make edx:eax
   idivl %ecx                 # eax = fixed point y change
   incl %ecx

   .align 4, 0x90
linex_loop:
   movl %ebx, %edx
   sarl $20, %edx             # edx = y
   cmpw %dx, 10(%esi)         # test ct
   jg linex_skip
   cmpw %dx, 12(%esi)         # test cb
   jle linex_skip
   movl 28(%esi, %edx, 4), %edi  # start of line pointer
   movswl 12(%ebp), %edx      # edx = x
   cmpw %dx, 6(%esi)          # test cl
   jg linex_skip
   cmpw %dx, 8(%esi)          # test cr
   jle linex_skip
   addl %edx, %edi            # pixel address
   movw 28(%ebp), %dx         # color
   movb %dl, %es:(%edi)       # store the pixel
linex_skip:
   addl %eax, %ebx            # y += dy
   incw 12(%ebp)              # x++
   loop linex_loop            # more?
   popw %es
   jmp line_done

   .align 4, 0x90
line_y_driven:                # y-driven fixed point line draw
   pushw %es
   movw 24(%esi), %ax         # segment selector
   orw %ax, %ax
   jz liney_myseg
   movw %ax, %es              # load dos segment
liney_myseg:
   movswl 16(%ebp), %edx      # edx = y1
   movswl 12(%ebp), %ebx      # ebx = x1
   movswl 24(%ebp), %ecx      # ecx = y2
   movswl 20(%ebp), %eax      # eax = x2
   subl %edx, %ecx            # ecx = y2 - y1 = counter
   jg liney_noswap
   negl %ecx                  # make count positive
   xchgl %eax, %ebx           # swap x values
   movw 24(%ebp), %dx
   movw %dx, 16(%ebp)         # swap start y value
liney_noswap:
   sall $20, %eax             # make x2 fixed point
   sall $20, %ebx             # make x1 fixed point
   subl %ebx, %eax            # eax = x2 - x1 = dx
   cdq                        # make edx:eax
   idivl %ecx                 # eax = fixed point x change
   incl %ecx

   .align 4, 0x90
liney_loop:
   movswl 16(%ebp), %edx      # edx = y
   cmpw %dx, 10(%esi)         # test ct
   jg liney_skip
   cmpw %dx, 12(%esi)         # test cb
   jle liney_skip
   movl 28(%esi, %edx, 4), %edi  # start of line pointer
   movl %ebx, %edx
   sarl $20, %edx             # edx = x
   cmpw %dx, 6(%esi)          # test cl
   jg liney_skip
   cmpw %dx, 8(%esi)          # test cr
   jle liney_skip
   addl %edx, %edi            # pixel address
   movw 28(%ebp), %dx         # color
   movb %dl, %es:(%edi)       # store the pixel
liney_skip:
   addl %eax, %ebx            # x += dx
   incw 16(%ebp)              # y++
   loop liney_loop            # more?
   popw %es

line_done:
   popl %ebx
   popl %esi
   popl %edi
   movl %ebp, %esp
   popl %ebp
   ret                        # end of the line()




.globl __fill_putpix    # fill putpixel routine
			# void _fill_putpix(BITMAP *bmp, short x, y)
   .align 4
__fill_putpix:
   pushl %ebp
   movl %esp, %ebp

   movswl 16(%ebp), %eax   # eax = y
   movl 8(%ebp), %edx      # edx = bmp
   cmpw 10(%edx), %ax      # test ct
   jl fputpix_done
   cmpw 12(%edx), %ax      # test cb
   jge fputpix_done

   movl __fill_array, %edx
   leal (%edx, %eax, 4), %edx

   movw 12(%ebp), %ax      # ax = x
   cmpw (%edx), %ax        # if (x < _fill_array[y].lpos)
   jae fputpix_no_lpos

   movw %ax, (%edx)        # set lpos

fputpix_no_lpos:
   cmpw 2(%edx), %ax       # if (x > _fill_array[y].rpos)
   jle fputpix_done

   movw %ax, 2(%edx)       # set rpos

fputpix_done:
   movl %ebp, %esp
   popl %ebp
   ret                     # end of _fill_putpix()




.globl __fill_finish       # draw lines stored in _fill_array
			   # void _fill_finish(BITMAP *bmp, short color);
   .align 4
__fill_finish:
   pushl %ebp
   movl %esp, %ebp
   pushl %ebx
   pushl %esi
   pushl %edi
   pushw %es

   movl 8(%ebp), %esi      # esi = bmp
   movw 24(%esi), %ax      # segment selector
   orw %ax, %ax            # check segment
   jz ff_my_seg
   movw %ax, %es           # write to DOS memory

ff_my_seg:
   movswl 10(%esi), %ebx   # ebx = y = bmp->ct
   cld

   .align 4, 0x90
ff_loop:
   movl __fill_array, %edi    # get _fill_array pointer
   leal (%edi, %ebx, 4), %edi

   movswl (%edi), %edx     # edx = x1
   movswl 2(%edi), %ecx    # ecx = x2
   cmpl %edx, %ecx 
   jl ff_skip              # should we draw this line?

   cmpw 6(%esi), %dx       # test x1, bmp->cl
   jge ff_x1_ok
   cmpw 6(%esi), %cx       # test x2, bmp->cl
   jl ff_skip
   movswl 6(%esi), %edx    # clip x1

ff_x1_ok:
   cmpw 8(%esi), %cx       # test x2, bmp->cr
   jl ff_x2_ok
   cmpw 8(%esi), %dx       # test x1, bmp->cr
   jge ff_skip
   movswl 8(%esi), %ecx    # clip x2
   decl %ecx

ff_x2_ok:
   subl %edx, %ecx         # loop counter
   incl %ecx

   movl 28(%esi, %ebx, 4), %edi
   addl %edx, %edi         # get the pixel address

   movb 12(%ebp), %dl      # get color
   movb %dl, %dh
   shrdl $16, %edx, %eax
   movw %dx, %ax           # get four copies of the color

   testw $1, %di           # are we word aligned?
   jz ff_w_aligned
   stosb                   # if not, copy a pixel
   decl %ecx
ff_w_aligned:
   testw $2, %di           # are we long aligned?
   jz ff_l_aligned
   subl $2, %ecx
   jl ff_oops
   stosw                   # if not, copy a word
ff_l_aligned:
   movw %cx, %dx           # store low bits
   shrl $2, %ecx           # for long copy
   jz ff_no_long
   rep ; stosl             # do some 32 bit copies
ff_no_long:
   testw $2, %dx
   jz ff_no_word
   stosw                   # do we need a 16 bit copy?
ff_no_word:
   testw $1, %dx
   jz ff_skip
   stosb                   # do we need a byte copy?

ff_skip:
   incl %ebx               # next line
   cmpw 12(%esi), %bx      # if y < bmp->cb
   jl ff_loop

ff_done:
   popw %es
   popl %edi
   popl %esi
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                        # end of _fill_finish()

   .align 4, 0x90
ff_oops:                      # fix up ecx for short line?
   addl $2, %ecx
   jmp ff_l_aligned




.globl __fill_line      # fixed point fill line plotting routine
			# void _fill_line(BITMAP *bmp, short x1, y1, x2, y2)
   .align 4
__fill_line:
   pushl %ebp
   movl %esp, %ebp
   pushl %edi
   pushl %esi
   pushl %ebx

   movl 8(%ebp), %esi         # esi = bmp
   movw 20(%ebp), %ax         # ax = x2
   subw 12(%ebp), %ax         # x2 - x1
   jge fline_dx_pve
   negw %ax                   # ax = abs(dx)
fline_dx_pve:
   movw 24(%ebp), %bx         # bx = y2
   subw 16(%ebp), %bx         # x2 - x1
   jge fline_dy_pve
   negw %bx                   # bx = abs(dy)
fline_dy_pve:
   cmpw %ax, %bx              # if abs(dx) > abs(dy)
   jg fline_y_driven
   jz fline_done
			      # x-driven fixed point line draw
   movswl 12(%ebp), %edx      # edx = x1
   movswl 16(%ebp), %ebx      # ebx = y1
   movswl 20(%ebp), %ecx      # ecx = x2
   movswl 24(%ebp), %eax      # eax = y2
   subl %edx, %ecx            # ecx = x2 - x1 = counter
   jg flinex_noswap
   negl %ecx                  # make count positive
   xchgl %eax, %ebx           # swap y values
   movw 20(%ebp), %dx
   movw %dx, 12(%ebp)         # swap start x value
flinex_noswap:
   sall $20, %eax             # make y2 fixed point
   sall $20, %ebx             # make y1 fixed point
   subl %ebx, %eax            # eax = y2 - y1 = dy
   cdq                        # make edx:eax
   idivl %ecx                 # eax = fixed point y change
   incl %ecx

   .align 4, 0x90
flinex_loop:
   movl %ebx, %edx
   sarl $20, %edx             # edx = y
   cmpw %dx, 10(%esi)         # test ct
   jg flinex_skip
   cmpw %dx, 12(%esi)         # test cb
   jle flinex_skip
   movl __fill_array, %edi
   leal (%edi, %edx, 4), %edi # fill array pointer
   movswl 12(%ebp), %edx      # edx = x
   cmpw (%edi), %dx           # if (x < _fill_array[y].lpos)
   jae flinex_no_lpos
   movw %dx, (%edi)           # set lpos
flinex_no_lpos:
   cmpw 2(%edi), %dx          # if (x > _fill_array[y].rpos)
   jle flinex_skip
   movw %dx, 2(%edi)          # set rpos

flinex_skip:
   addl %eax, %ebx            # y += dy
   incw 12(%ebp)              # x++
   loop flinex_loop           # more?
   jmp fline_done

   .align 4, 0x90
fline_y_driven:               # y-driven fixed point line draw
   movswl 16(%ebp), %edx      # edx = y1
   movswl 12(%ebp), %ebx      # ebx = x1
   movswl 24(%ebp), %ecx      # ecx = y2
   movswl 20(%ebp), %eax      # eax = x2
   subl %edx, %ecx            # ecx = y2 - y1 = counter
   jg fliney_noswap
   negl %ecx                  # make count positive
   xchgl %eax, %ebx           # swap x values
   movw 24(%ebp), %dx
   movw %dx, 16(%ebp)         # swap start y value
fliney_noswap:
   sall $20, %eax             # make x2 fixed point
   sall $20, %ebx             # make x1 fixed point
   subl %ebx, %eax            # eax = x2 - x1 = dx
   cdq                        # make edx:eax
   idivl %ecx                 # eax = fixed point x change
   incl %ecx

   .align 4, 0x90
fliney_loop:
   movswl 16(%ebp), %edx      # edx = y
   cmpw %dx, 10(%esi)         # test ct
   jg fliney_skip
   cmpw %dx, 12(%esi)         # test cb
   jle fliney_skip
   movl __fill_array, %edi
   leal (%edi, %edx, 4), %edi # fill array pointer
   movl %ebx, %edx
   sarl $20, %edx             # edx = x
   cmpw (%edi), %dx           # if (x < _fill_array[y].lpos)
   jae fliney_no_lpos
   movw %dx, (%edi)           # set lpos
fliney_no_lpos:
   cmpw 2(%edi), %dx          # if (x > _fill_array[y].rpos)
   jle fliney_skip
   movw %dx, 2(%edi)          # set rpos

fliney_skip:
   addl %eax, %ebx            # x += dx
   incw 16(%ebp)              # y++
   loop fliney_loop           # more?

fline_done:
   popl %ebx
   popl %esi
   popl %edi
   movl %ebp, %esp
   popl %ebp
   ret                        # end of _fill_line()





