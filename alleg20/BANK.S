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
 *      SVGA bank switching code. These routines will be called with
 *      a line number in %eax and a pointer to the bitmap in %edx. The
 *      bank switcher should select the appropriate bank for the line,
 *      and replace %eax with a pointer to the start of the line.
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




#define ADJUST

/* Template for bank switchers. Produces a framework that checks which 
 * bank is currently selected and only executes the provided code if it
 * needs to change bank. Restores %eax: if the switch routine touches any 
 * other registers it must save and restore them itself.
 */
#define BANK_SWITCHER(name, cache, code...)                                  \
.globl name                                                                ; \
   .align 4                                                                ; \
name:                                                                      ; \
   pushl %eax                                                              ; \
									   ; \
   shll $2, %eax                                                           ; \
   addl __gfx_bank, %eax                     /* lookup which bank */       ; \
   movl (%eax), %eax                                                       ; \
   cmpl cache, %eax                          /* need to change? */         ; \
   je name##_done                                                          ; \
									   ; \
   movl %eax, cache                          /* store the new bank */      ; \
   code                                      /* and change it */           ; \
									   ; \
name##_done:                                                               ; \
   popl %eax                                                               ; \
   movl BMP_LINE(%edx, %eax, 4), %eax        /* load line address */       ; \
   ADJUST                                    /* adjust the address? */     ; \
   ret                                                                     ; \
									   ; \
.globl name##_end                                                          ; \
   .align 4                                                                ; \
name##_end:                                                                ; \
   ret 




/* Uses VESA function 05h (real mode interrupt) to select the bank in %eax.
 * Restores all registers except %eax, which is handled by BANK_SWITCHER.
 */
#define SET_VESA_BANK_RM(window)                                             \
   pushl %ebx                                /* store registers */         ; \
   pushl %ecx                                                              ; \
   pushl %edx                                                              ; \
   pushl %esi                                                              ; \
   pushl %edi                                                              ; \
   pushw %es                                                               ; \
									   ; \
   movw ___djgpp_ds_alias, %bx                                             ; \
   movw %bx, %es                                                           ; \
									   ; \
   movl $0x10, %ebx                          /* call int 0x10 */           ; \
   movl $0, %ecx                             /* no stack required */       ; \
   movl $__dpmi_reg, %edi                    /* register structure */      ; \
									   ; \
   movw $0x4F05, DPMI_AX(%edi)               /* VESA function 05h */       ; \
   movw $window, DPMI_BX(%edi)               /* which window? */           ; \
   movw %ax, DPMI_DX(%edi)                   /* which bank? */             ; \
   movw $0, DPMI_SP(%edi)                    /* zero stack */              ; \
   movw $0, DPMI_SS(%edi)                                                  ; \
   movw $0, DPMI_FLAGS(%edi)                 /* and zero flags */          ; \
									   ; \
   movl $0x0300, %eax                        /* simulate RM interrupt */   ; \
   int $0x31                                                               ; \
									   ; \
   popw %es                                  /* restore registers */       ; \
   popl %edi                                                               ; \
   popl %esi                                                               ; \
   popl %edx                                                               ; \
   popl %ecx                                                               ; \
   popl %ebx




/* Uses the VESA 2.0 protected mode interface to select the bank in %eax.
 * Restores all registers except %eax, which is handled by BANK_SWITCHER.
 */
#define SET_VESA_BANK_PM(window)                                             \
   pushl %ebx                                /* store registers */         ; \
   pushl %ecx                                                              ; \
   pushl %edx                                                              ; \
   pushl %esi                                                              ; \
   pushl %edi                                                              ; \
									   ; \
   movw $window, %bx                         /* which window? */           ; \
   movw %ax, %dx                             /* which bank? */             ; \
   movl __pm_vesa_switcher, %eax                                           ; \
   call *%eax                                /* do it! */                  ; \
									   ; \
   popl %edi                                 /* restore registers */       ; \
   popl %esi                                                               ; \
   popl %edx                                                               ; \
   popl %ecx                                                               ; \
   popl %ebx                                                               ; \




/* Uses the VESA 2.0 protected mode interface to select the bank in %eax,
 * passing a selector for memory mapped io in %es. Restores all registers 
 * except %eax, which is handled by BANK_SWITCHER.
 */
#define SET_VESA_BANK_PM_ES(window)                                          \
   pushl %ebx                                /* store registers */         ; \
   pushl %ecx                                                              ; \
   pushl %edx                                                              ; \
   pushl %esi                                                              ; \
   pushl %edi                                                              ; \
   pushw %es                                                               ; \
									   ; \
   movw $window, %bx                         /* which window? */           ; \
   movw %ax, %dx                             /* which bank? */             ; \
   movw __mmio_segment, %ax                  /* load selector into %es */  ; \
   movw %ax, %es                                                           ; \
   movl __pm_vesa_switcher, %eax                                           ; \
   call *%eax                                /* do it! */                  ; \
									   ; \
   popw %es                                  /* restore registers */       ; \
   popl %edi                                                               ; \
   popl %esi                                                               ; \
   popl %edx                                                               ; \
   popl %ecx                                                               ; \
   popl %ebx                                                               ; \




/* VESA window 1 switching routines */
BANK_SWITCHER( __vesa_window_1, __last_bank_1, SET_VESA_BANK_RM(0) )
BANK_SWITCHER( __vesa_pm_window_1, __last_bank_1, SET_VESA_BANK_PM(0) )
BANK_SWITCHER( __vesa_pm_es_window_1, __last_bank_1, SET_VESA_BANK_PM_ES(0) )



/* VESA window 2 switching routines */
#undef ADJUST
#define ADJUST addl __window_2_offset, %eax
BANK_SWITCHER( __vesa_window_2, __last_bank_2, SET_VESA_BANK_RM(1) )
BANK_SWITCHER( __vesa_pm_window_2, __last_bank_2, SET_VESA_BANK_PM(1) )
BANK_SWITCHER( __vesa_pm_es_window_2, __last_bank_2, SET_VESA_BANK_PM_ES(1) )
#undef ADJUST
#define ADJUST



/* Cirrus 64xx write bank selector */
BANK_SWITCHER( __cirrus64_write_bank, __last_bank_1,
   pushl %edx
   pushl %eax

   movl $0x3CE, %edx          /* 3CE index 0xF */
   movb $0xF, %al
   outb %al, %dx

   popl %eax                  /* write bank number */
   incl %edx
   outb %al, %dx

   popl %edx
)



/* Cirrus 64xx read bank selector */
BANK_SWITCHER( __cirrus64_read_bank, __last_bank_2, 
   pushl %edx
   pushl %eax

   movl $0x3CE, %edx          /* 3CE index 0xE */
   movb $0xE, %al
   outb %al, %dx

   popl %eax                  /* write bank number */
   incl %edx
   outb %al, %dx

   popl %edx
)



/* Cirrus 54xx bank selector */
BANK_SWITCHER( __cirrus54_bank, __last_bank_1,
   pushl %edx
   pushl %eax

   movl $0x3CE, %edx          /* 3CE index 0x9 */
   movb $0x9, %al
   outb %al, %dx

   popl %eax                  /* write bank number */
   incl %edx
   outb %al, %dx

   popl %edx
)



/* S3 bank selector */
BANK_SWITCHER( __s3_bank, __last_bank_1,
   pushl %edx
   pushl %eax
   movl __crtc, %edx

   movb $0x38, %al            /* enable extensions */
   outb %al, %dx
   incl %edx
   movb $0x48, %al
   outb %al, %dx
   decl %edx

   movb $0x31, %al            /* read register 0x31 */
   outb %al, %dx 
   incl %edx
   inb %dx, %al
   decl %edx

   movb %al, %ah              /* set bank write and vid mem > 256K flags */
   movb $0x31, %al
   outb %al, %dx
   movb %ah, %al
   orb $9, %al 
   incl %edx
   outb %al, %dx
   decl %edx

   movb $0x35, %al            /* write the bank number */
   outb %al, %dx
   incl %edx
   popl %eax
   outb %al, %dx 
   decl %edx

   movb $0x38, %al            /* disable extensions */
   outb %al, %dx
   incl %edx
   xorb %al, %al
   outb %al, %dx
   decl %edx

   popl %edx
)



/* Tseng ET3000 write bank selector */
BANK_SWITCHER( __et3000_write_bank, __last_bank_1,
   pushl %edx

   movb __last_bank_2, %dl
   shlb $3, %dl
   orb %dl, %al               /* mask with read bank */
   orb $0x40, %al             /* select 64k segments */

   movw $0x3CD, %dx
   outb %al, %dx              /* write to the card */

   popl %edx
)



/* Tseng ET3000 read bank selector */
BANK_SWITCHER( __et3000_read_bank, __last_bank_2, 
   pushl %edx

   shlb $3, %al
   orb __last_bank_1, %al     /* mask with write bank */
   orb $0x40, %al             /* select 64k segments */

   movw $0x3CD, %dx
   outb %al, %dx              /* write to the card */

   popl %edx
)



/* Tseng ET4000 write bank selector */
BANK_SWITCHER( __et4000_write_bank, __last_bank_1,
   pushl %edx

   movb __last_bank_2, %dl
   shlb $4, %dl
   orb %dl, %al               /* mask with read bank */

   movw $0x3CD, %dx
   outb %al, %dx              /* write to the card */

   popl %edx
)



/* Tseng ET4000 read bank selector */
BANK_SWITCHER( __et4000_read_bank, __last_bank_2, 
   pushl %edx

   shlb $4, %al
   orb __last_bank_1, %al     /* mask with write bank */

   movw $0x3CD, %dx
   outb %al, %dx              /* write to the card */

   popl %edx
)


