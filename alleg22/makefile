#################################################
#                                               #
#     Allegro makefile, by Shawn Hargreaves     #
#                                               #
#     This will only work if you preserved      #
#     the directory tree when installing        #
#     Allegro (ie. if you used infozip, or      #
#     the -d flag to pkunzip).                  #
#                                               #
#     The core library should build fine on     #
#     any djgpp system, but some of the more    #
#     obscure targets (such as 'make clean'     #
#     and 'make zip') require other programs    #
#     like 4dos and infozip.                    #
#                                               #
#################################################

# -------- target OS (djgpp or linux) --------

ifdef DJDIR
TARGET = djgpp
else
TARGET = linux
endif

ifeq ($(TARGET),djgpp)

# -------- djgpp specific definitions --------
EXE = .exe
LIB = lib/djgpp/liballeg.a
INTERNAL_H = src/internal.h src/djgpp/interndj.h
LIBDEST = $(DJDIR)/lib/liballeg.a
INCDEST = $(DJDIR)/include/allegro.h
CPLIBSRC = $(subst /,\,$(LIB))
CPINCSRC = allegro.h
CPLIBDEST = $(subst /,\,$(LIBDEST))
CPINCDEST = $(subst /,\,$(INCDEST))
CP = copy
CLEAN = del /y .\obj\djgpp\*.* .\lib\djgpp\*.* .\docs\*.txt .\docs\*.html

SYSOBJS = adlib.o ati.o bank.o cirrus.o digmid.o dma.o dpmi.o gus.o irq.o \
	  irqwrap.o joystick.o keyboard.o mouse.o mpu.o paradise.o s3.o \
	  sb.o timer.o trident.o tseng.o vbeaf.o vesa.o video7.o

else
ifeq ($(TARGET),linux)

# -------- linux specific definitions --------
EXE =
LIB = lib/linux/liballeg.a
INTERNAL_H = src/internal.h src/linux/internli.h
LIBDEST = /usr/lib/liballeg.a
INCDEST = /usr/include/allegro.h
CPLIBSRC = $(LIB)
CPINCSRC = allegro.h
CPLIBDEST = $(LIBDEST)
CPINCDEST = $(INCDEST)
CP = cp
CLEAN = rm ./obj/linux/* ./lib/linux/* ./docs/*.txt ./docs/*.html

SYSOBS = joystick.o keyboard.o mouse.o svgalib.o timer.o

else
.PHONY : badtarget
badtarget :
	@echo Error: target operating system not set properly. 
	@echo Fix the makefile, or run make djgpp or make linux.
endif
endif

OBJ = obj/$(TARGET)

ifdef DEBUGMODE

OFLAGS = -g
LFLAGS = 

else

OFLAGS = -m486 -O3 -ffast-math -fomit-frame-pointer
LFLAGS = -s

endif

CFLAGS = -I. -Isrc -Isrc/$(TARGET) -I$(OBJ) -Wall -Werror $(OFLAGS)
SFLAGS = -I. -Isrc -Isrc/$(TARGET) -I$(OBJ) -Wall -Werror

VPATH = demo docs examples setup src tests tools src/$(TARGET)

PROGRAMS = demo setup mathtest play playfli test colormap dat dat2s \
	   grabber pack rgbmap examples

OBJS = allegro.o blit.o blit8.o color.o datafile.o file.o fli.o flood.o \
       gfx.o gfx8.o graphics.o gui.o inline.o math.o math3d.o midi.o \
       misc.o modex.o polygon.o readbmp.o scanline.o sound.o spline.o \
       sprite.o sprite8.o text.o vga.o xgfx.o $(SYSOBJS)

LIB_OBJS = $(addprefix $(OBJ)/, $(OBJS))

DOCS = allegro changes faq thanks

DOC_OBJ = $(addprefix docs/, $(DOCS))
DOC_OBJS = $(addsuffix .txt, $(DOC_OBJ)) $(addsuffix .html, $(DOC_OBJ))

.PRECIOUS : $(OBJ)/%.o

.PHONY : all djgpp linux debug msg lib install docs clean zip $(PROGRAMS)

all : msg $(LIB) $(PROGRAMS) docs install
	@echo All done.
	@echo To use Allegro, #include allegro.h and link with liballeg.a
	@echo Example command line: gcc foobar.c -o foobar$(EXE) -lalleg
	@echo Enjoy!

djgpp :
	make all TARGET=djgpp

linux :
	make all TARGET=linux

debug :
	make all DEBUGMODE=1

msg :
	@echo Compiling the $(TARGET) version of Allegro. Please wait...

lib : $(LIB)

install : $(LIBDEST) $(INCDEST)

docs : $(DOC_OBJS)

$(LIBDEST) : $(LIB)
	$(CP) $(CPLIBSRC) $(CPLIBDEST)

$(INCDEST) : allegro.h
	$(CP) $(CPINCSRC) $(CPINCDEST)

$(OBJ)/%.o : %.c allegro.h
	gcc $(CFLAGS) -o $@ -c $<

$(OBJ)/%.o : %.S asmdefs.h $(OBJ)/asmdef.h
	gcc $(SFLAGS) -o $@ -c $<

$(OBJ)/%.o : %.s asmdefs.h $(OBJ)/asmdef.h
	gcc -x assembler-with-cpp $(SFLAGS) -o $@ -c $<

*/%$(EXE) : $(OBJ)/%.o $(LIB)
	gcc $(LFLAGS) -o $@ $< $(LIB)

docs/%.txt docs/%.html : docs/%._tx $(OBJ)/makedoc$(EXE)
	$(OBJ)/makedoc$(EXE) $<

$(OBJ)/makedoc$(EXE) : makedoc.c
	gcc $(CFLAGS) $(LFLAGS) -o $@ $<

$(OBJ)/asmdef.h : $(OBJ)/asmdef$(EXE)
	$(OBJ)/asmdef$(EXE) $(OBJ)/asmdef.h

$(OBJ)/asmdef$(EXE) : asmdef.c allegro.h $(INTERNAL_H)
	gcc $(CFLAGS) $(LFLAGS) -o $@ $<

$(OBJ)/testsnd.s $(OBJ)/testsnd.h : setup/setup.dat setup/setup.h tools/dat2s$(EXE)
	tools/dat2s$(EXE) setup/setup.dat -o $(OBJ)/testsnd.s -h $(OBJ)/testsnd.h

$(OBJ)/testsnd.o : $(OBJ)/testsnd.s
	gcc $(SFLAGS) -o $(OBJ)/testsnd.o -c $(OBJ)/testsnd.s

$(OBJ)/setup.o : setup/setup.cc allegro.h $(OBJ)/testsnd.h
	gcc $(CFLAGS) -o $(OBJ)/setup.o -c setup/setup.cc

setup/setup$(EXE) : $(OBJ)/setup.o $(OBJ)/testsnd.o $(LIB)
	gcc $(LFLAGS) -o setup/setup$(EXE) $(OBJ)/setup.o $(OBJ)/testsnd.o $(LIB) -liostr

tools/dat$(EXE) : $(OBJ)/dat.o $(OBJ)/datedit.o $(LIB)
	gcc $(LFLAGS) -o tools/dat$(EXE) $(OBJ)/dat.o $(OBJ)/datedit.o $(LIB)

tools/dat2s$(EXE) : $(OBJ)/dat2s.o $(OBJ)/datedit.o $(LIB)
	gcc $(LFLAGS) -o tools/dat2s$(EXE) $(OBJ)/dat2s.o $(OBJ)/datedit.o $(LIB)

tools/grabber$(EXE) : $(OBJ)/grabber.o $(OBJ)/datedit.o $(LIB)
	gcc $(LFLAGS) -o tools/grabber$(EXE) $(OBJ)/grabber.o $(OBJ)/datedit.o $(LIB)

$(LIB) : $(LIB_OBJS)
	ar rs $(LIB) $(LIB_OBJS)

clean:
	$(CLEAN)

ZIPMSG = This file is needed because some unzip programs skip empty directories.

zip:
	@del /q allegro.zip
	@del /q allegro.mft
	@echo $(ZIPMSG) > obj\djgpp\tmp.txt
	@echo $(ZIPMSG) > obj\linux\tmp.txt
	@echo $(ZIPMSG) > lib\djgpp\tmp.txt
	@echo $(ZIPMSG) > lib\linux\tmp.txt
	zip -9 allegro.zip @zipup.scr
	unzip -Z -1 allegro.zip > allegro.mft
	echo allegro.mft >> allegro.mft
	zip -9 allegro.zip allegro.mft

demo : demo/demo$(EXE)
setup : setup/setup$(EXE)
mathtest : tests/mathtest$(EXE)
play : tests/play$(EXE)
playfli : tests/playfli$(EXE)
test : tests/test$(EXE)
colormap : tools/colormap$(EXE)
dat : tools/dat$(EXE)
dat2s : tools/dat2s$(EXE)
grabber : tools/grabber$(EXE)
pack : tools/pack$(EXE)
rgbmap : tools/rgbmap$(EXE)

examples : examples/ex1$(EXE) examples/ex2$(EXE) examples/ex3$(EXE) \
	   examples/ex4$(EXE) examples/ex5$(EXE) examples/ex6$(EXE) \
	   examples/ex7$(EXE) examples/ex8$(EXE) examples/ex9$(EXE) \
	   examples/ex10$(EXE) examples/ex11$(EXE) examples/ex12$(EXE) \
	   examples/ex13$(EXE) examples/ex14$(EXE) examples/ex15$(EXE) \
	   examples/ex16$(EXE) examples/ex17$(EXE) examples/ex18$(EXE) \
	   examples/ex19$(EXE) examples/ex20$(EXE) examples/ex21$(EXE) \
	   examples/ex22$(EXE) examples/ex23$(EXE) examples/ex24$(EXE) \
	   examples/ex25$(EXE) examples/ex26$(EXE)

$(OBJ)/demo.o : demo.h
$(OBJ)/adlib.o : fm_instr.h
$(OBJ)/irq.o : asmdefs.h $(OBJ)/asmdef.h
$(OBJ)/blit.o $(OBJ)/sprite.o : opcodes.h
$(OBJ)/dat.o $(OBJ)/datedit.o $(OBJ)/dat2s.o $(OBJ)/grabber.o : datedit.h
$(OBJ)/ex12.o $(OBJ)/ex13.o : example.h
$(OBJ)/ex21.o : running.h

INTERNAL_DEPS = adlib.o allegro.o ati.o blit.o cirrus.o datafile.o digmid.o \
		dma.o file.o fli.o flood.o gfx.o graphics.o gui.o gus.o \
		inline.o irq.o joystick.o keyboard.o midi.o modex.o mouse.o \
		mpu.o paradies.o polygon.o readbmp.o sb.o sprite.o s3.o \
		sound.o spline.o svgalib.o text.o timer.o tseng.o vbeaf.o \
		vesa.o vga.o video7.o setup.o

$(addprefix $(OBJ)/, $(INTERNAL_DEPS)) : $(INTERNAL_H)
