
		//  /     /     ,----  ,----.  ,----.  ,----.
	      / /  /     /     /      /    /  /    /  /    /
	    /  /  /     /     /___   /       /____/  /    /
	  /---/  /     /     /      /  __   /\      /    /
	/    /  /     /     /      /    /  /  \    /    /
      /     /  /____ /____ /____  /____/  /    \  /____/


      A mode 13h game programming library for Borland C and djgpp.
      Version 1.02, 1994/95.


See allegro.txt for an introduction to Allegro, documentation on the 
functions that Allegro provides, and copyright conditions.

This version has been tested with djgpp v2 (beta release 3) and with
Borland C 4.5. The test programs which are prefixed with underbars were
compiled with Borland C, the others with djgpp. Note that the djgpp
executables will only work under a DPMI environment. To run them from DOS,
put Charles Sandmann's dpmi provider (cwsdpmi), in one of the directories
named in your PATH.

Files you should have recieved:

<documentation>
README.TXT   - What you are currently reading.
ALLEGRO.TXT  - Main documentation file.
GRABBER.TXT  - Documentation for the grabber utility.
TIMINGS.TXT  - Some timings produced by the test programs.

<header file>
ALLEGRO.H    - Main header file, needed to build the library, and which
	       should be included in any programs that use Allegro.

<library code: a mixture of C and inline 8086 asm for the Borland version>
ALLEGRO.C    - Initialise, terminate, interrupt handlers etc.
BITMAP.C     - Bitmap, sprite, and text routines.
GRAPHICS.C   - Other graphics routines.
GUI.C        - GUI routines.
FILE.C       - File access and compression routines.
MATH.C       - Fixed point math routines.

<386 asm code for the djgpp version>
BMP.S        - Bitmap, sprite, and text routines.
GRAPH.S      - Other graphics routines.
MISC.S       - Maths, interrupts, etc.

<grabber utility>
GRABBER.C    - Source code.
GRABBER.EXE  - Executable compiled with djgpp.
_GRABBER.EXE - Executable compiled with Borland.

<demonstration game>
DEMO.C       - Source code.
DEMO.H       - Constant file produced by the grabber utility.
DEMO.DAT     - Data file produced by the grabber utility.
DEMO.EXE     - Demo game compiled with djgpp.
_DEMO.EXE    - Demo game compiled with Borland.

<test program>
TEST.C       - Source code.
TEST.EXE     - Executable compiled with djgpp.
_TEST.EXE    - Executable compiled with Borland.

<file copy/pack utility>
CP.C         - Source code.
CP.EXE       - Executable compiled with djgpp.
_CP.EXE      - Executable compiled with Borland.

<fixed point maths test program>
MATHTEST.C   - Source code.
MATHTEST.EXE - Executable compiled with djgpp.
_MATHTST.EXE - Executable compiled with Borland.

<djgpp library>
ALLEGRO.A    - Library file for djgpp, built with version 2 beta3.
MAKEFILE     - Makefile for djgpp

<Borland library>
ALLEGRO.LIB  - Library file for Borland C 4.5, large model, 286 or above.
ALLEGRO.IDE,
ALLEGRO.DSW  - For the Borland IDE.

<DMPI provider by Charles Sandmann>
CWSDPMI.EXE,
CWSDPMI.DOC  - DPMI provider, required to run djgpp executables under
	       16 bit DOS (you don't need this if you are running in a
	       DOS session under Win 3.1/95/NT, OS/2, etc). If you want
	       to get hold of the source for this program, it is available
	       from ftp.idsoftware.com/djgpp/v2.


One potential area of confusion is that the code for the Borland and djgpp 
versions is all mixed up in the same files. This is because a lot of stuff 
is common to both, and it makes maintaining the library a lot easier. The 
.C files contain both C code (some used by both versions, some only by one 
or the other) and inline 80x86 asm for the Borland version. The 386 asm 
code for the djgpp version is separate, in the .S files.


By Shawn Hargreaves,
1 Salisbury Road,
Market Drayton,
Shropshire,
England TF9 1AJ
email slh100@tower.york.ac.uk (until 1996)


History:

   v1.0  - first release, Borland C and djgpp 1.14

   v1.0  - re-release a week later, patched to work with the beta version
	   of djgpp 2 (mainly this just involved changing which system
	   header files it included).

   v1.01 - couple of fixes to get it to work with djgpp 2 beta release 3.
	   Specifically, I replaced the int86 calls with __dpmi_int in the
	   djgpp version, and _my_fopen now clears errno after creating
	   files. I also fixed some problems with the mouse input routines
	   by replacing the djgpp serial port code with calls to the int 33
	   mouse driver.

   v1.02 - added allegro_use_timer global variable, fixed igetl() routine.
