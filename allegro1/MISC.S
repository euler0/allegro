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
#       Various routines for djgpp (fixed point math + interrupt handlers)
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


.globl _fmul               # fixed point multiply
			   # fixed fmul(fixed x, fixed y);
   .align 4
_fmul:
   pushl %ebp
   movl %esp, %ebp
   movl 8(%ebp), %eax
   movl 12(%ebp), %edx
   xorl %ecx, %ecx
   orl %eax, %eax
   jns mul_no_x_neg
   negl %eax
   movl $1, %ecx
mul_no_x_neg:
   orl %edx, %edx
   jns mul_no_y_neg
   negl %edx
   negl %ecx
   addl $1, %ecx
mul_no_y_neg:
   mull %edx
   shrdl $16, %edx, %eax
   andl $0xffff8000, %edx
   jz mul_done
   movw $34, _errno        # ERANGE
   movl $0x7fffffff, %eax
mul_done:
   orl %ecx, %ecx
   jz mul_no_neg
   negl %eax
mul_no_neg:
   movl %ebp,%esp
   popl %ebp
   ret                     # end of fmul()




.globl _fdiv               # fixed point divide
			   # fixed fdiv(fixed x, fixed y);
   .align 4
_fdiv:
   pushl %ebp
   movl %esp, %ebp
   pushl %ebx
   movl 8(%ebp), %eax
   movl 12(%ebp), %ebx
   xorl %ecx, %ecx
   orl %eax, %eax
   jns div_no_x_neg
   negl %eax
   movl $1, %ecx
div_no_x_neg:
   orl %ebx, %ebx
   jns div_no_y_neg
   negl %ebx
   negl %ecx
   addl $1, %ecx
div_no_y_neg:
   xorl %edx, %edx
   shldl $16, %eax, %edx
   shll $16, %eax
   movl $-1, __fdiv_flag   # trap overflows
   divl %ebx
   movl $0, __fdiv_flag
   test $0x80000000, %eax
   jz div_done
div_overflow:
   movw $34, _errno        # ERANGE
   movl $0x7fffffff, %eax
div_done:
   orl %ecx, %ecx
   jz div_no_neg
   negl %eax
div_no_neg:
   popl %ebx
   movl %ebp, %esp
   popl %ebp
   ret                     # end of fdiv()




.globl __my_timerint       # timer interrupt handler
			   # void _my_timerint(_go32_dpmi_registers *regs);
   .align 4
__my_timerint:
   pushl %ebp
   movl %esp,%ebp
   pushl %ebx
   pushl %esi
   pushl %edi

   decl __countdown        # should we update the bios clock?
   jg timer_no_bios

   movl $11, __countdown   # if yes...
   incl __bios_counter

timer_no_bios:
   xorl %esi, %esi
   cmpw $0, __my_int_q_size
   jle timer_no_hooks
   movl $__my_int_queue, %edi
   xorl %ebx, %ebx

   .align 4,0x90              # loop through timer interrupt hooks
timer_hook_loop:
   decw 6(%edi, %ebx)
   cmpw $0, 6(%edi, %ebx)
   jg timer_no_call

   movw 4(%edi, %ebx), %ax    # call them if required
   movw %ax, 6(%edi, %ebx)
   movl __my_int_queue(%ebx), %eax
   call *%eax

timer_no_call:
   addl $8, %ebx
   incl %esi
   movswl __my_int_q_size, %eax
   cmpl %eax, %esi
   jl timer_hook_loop

timer_no_hooks:
   cmpl $0, _mouse_callback         # do we have a mouse callback?
   je timer_no_mouse_callback
   movl __callback_flag, %eax
   testl %eax, %eax
   je timer_no_mouse_callback

   movl $0, __callback_flag         # call it if required
   movl _mouse_callback, %eax
   call *%eax

timer_no_mouse_callback:
   movw $0x20, %ax         # ack. the interrupt
   movw %ax, %dx
   outb %al, %dx

   popl %edi
   popl %esi
   popl %ebx
   movl %ebp,%esp
   popl %ebp
   ret                     # end of _my_timerint()

