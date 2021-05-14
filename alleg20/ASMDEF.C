/* 
 *    This a little on the complex side, but I couldn't think of any 
 *    other way to do it. I was getting fed up with having to rewrite 
 *    my asm code every time I altered the layout of a C struct, but I 
 *    couldn't figure out any way to get the asm stuff to read and 
 *    understand the C headers. So I made this program. It #includes 
 *    allegro.h so it knows about everything the C code uses, and when 
 *    run it spews out a bunch of #defines containing information about 
 *    structure sizes which the asm code can refer to.
 */


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <dpmi.h>

#include "allegro.h"
#include "internal.h"


int main(int argc, char *argv[])
{
   FILE *f;

   printf("writing structure offsets into asmdef.h...\n");

   f = fopen("asmdef.h", "w");

   fprintf(f, "/* automatically generated structure offsets for use by asm code */\n\n");

   fprintf(f, "#define BMP_W        %ld\n",  offsetof(BITMAP, w));
   fprintf(f, "#define BMP_H        %ld\n",  offsetof(BITMAP, h));
   fprintf(f, "#define BMP_CLIP     %ld\n",  offsetof(BITMAP, clip));
   fprintf(f, "#define BMP_CL       %ld\n",  offsetof(BITMAP, cl));
   fprintf(f, "#define BMP_CR       %ld\n",  offsetof(BITMAP, cr));
   fprintf(f, "#define BMP_CT       %ld\n",  offsetof(BITMAP, ct));
   fprintf(f, "#define BMP_CB       %ld\n",  offsetof(BITMAP, cb));
   fprintf(f, "#define BMP_DAT      %ld\n",  offsetof(BITMAP, dat));
   fprintf(f, "#define BMP_SEG      %ld\n",  offsetof(BITMAP, seg));
   fprintf(f, "#define BMP_WBANK    %ld\n",  offsetof(BITMAP, write_bank));
   fprintf(f, "#define BMP_RBANK    %ld\n",  offsetof(BITMAP, read_bank));
   fprintf(f, "#define BMP_LINE     %ld\n",  offsetof(BITMAP, line));
   fprintf(f, "\n");
   fprintf(f, "#define RLE_W        %ld\n",  offsetof(RLE_SPRITE, w));
   fprintf(f, "#define RLE_H        %ld\n",  offsetof(RLE_SPRITE, h));
   fprintf(f, "#define RLE_DAT      %ld\n",  offsetof(RLE_SPRITE, dat));
   fprintf(f, "\n");
   fprintf(f, "#define IRQ_SIZE     %ld\n",  sizeof(_IRQ_HANDLER));
   fprintf(f, "#define IRQ_HANDLER  %ld\n",  offsetof(_IRQ_HANDLER, handler));
   fprintf(f, "#define IRQ_NUMBER   %ld\n",  offsetof(_IRQ_HANDLER, number));
   fprintf(f, "#define IRQ_OLDVEC   %ld\n",  offsetof(_IRQ_HANDLER, old_vector));
   fprintf(f, "\n");
   fprintf(f, "#define DPMI_AX      %ld\n",  offsetof(__dpmi_regs, x.ax));
   fprintf(f, "#define DPMI_BX      %ld\n",  offsetof(__dpmi_regs, x.bx));
   fprintf(f, "#define DPMI_CX      %ld\n",  offsetof(__dpmi_regs, x.cx));
   fprintf(f, "#define DPMI_DX      %ld\n",  offsetof(__dpmi_regs, x.dx));
   fprintf(f, "#define DPMI_SP      %ld\n",  offsetof(__dpmi_regs, x.sp));
   fprintf(f, "#define DPMI_SS      %ld\n",  offsetof(__dpmi_regs, x.ss));
   fprintf(f, "#define DPMI_FLAGS   %ld\n",  offsetof(__dpmi_regs, x.flags));

   fclose(f);

   return 0;
}
