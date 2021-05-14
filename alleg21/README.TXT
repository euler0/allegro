	 ______   ___    ___
	/\  _  \ /\_ \  /\_ \
	\ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
	 \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
	  \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
	   \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
	    \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
					   /\____/
					   \_/__/     Version 2.1


	       A game programming library for djgpp

		   By Shawn Hargreaves, 1994/96



#include <std_disclaimer.h>

   "I do not accept responsibility for any effects, adverse or otherwise, 
    that this code may have on you, your computer, your sanity, your dog, 
    and anything else that you can think of. Use it at your own risk."



======================================
============ Introduction ============
======================================

   Allegro is a library of functions for use in computer games, written for 
   djgpp v2 in a mixture of C and assembly language. This is version 2.1: 
   see changes.txt for a list of differences from the previous release.

   According to the Oxford Companion to Music, 'allegro' is Italian for 
   "quick, lively, bright". Once upon a time it was also an acronym for 
   "Atari Low Level Game Routines", but it is a long time since I did any 
   programming on the Atari, and the name is pretty much the only thing left 
   of the original Atari code.



==================================
============ Features ============
==================================

   Supports VGA mode 13h, mode-X (twenty tweaked VGA resolutions plus 
   unchained 640x400 Xtended mode), and 256 color SVGA modes. Uses the VESA 
   2.0 linear framebuffer and protected mode bank switching interfaces if 
   they are available, and also contains register level drivers for Cirrus, 
   S3, and Tseng cards.

   Drawing functions including putpixel, getpixel, lines, rectangles, 
   polygons, circles, floodfill, patterned fills, masked, run length 
   encoded, and compiled sprites, blitting, bitmap scaling and rotation, and 
   text output with proportional fonts. Supports clipping, and can draw 
   directly to the screen or to memory bitmaps of any size.

   Hardware scrolling, mode-X split screens, and pallete manipulation.

   FLI/FLC animation player.

   Plays background MIDI music and up to eight simultaneous sound effects. 
   Samples can be looped, and the volume, pan, and pitch can be altered 
   while they are playing. The MIDI player responds to note on, note off, 
   main volume, pan, pitch bend, and program change messages, using the 
   General Midi patch set and drum mappings. Currently supports Adlib, SB, 
   SB Pro, SB16, and MPU-401.

   Easy access to the mouse, keyboard, joystick, and high resolution timer 
   interrupts, including a vertical retrace interrupt simulator.

   Routines for reading and writing LZSS compressed files.

   Multi-object data files and a grabber utility.

   Fixed point 16.16 math functions including lookup table trig.

   GUI dialog manager and file selector.



===================================
============ Copyright ============
===================================

   Allegro is swap-ware. You may use, modify, redistribute, and generally 
   hack it about in any way you like, but if you do you must send me 
   something in exchange. This could be a complimentary copy of a game, an 
   addition or improvement to Allegro, a bug report, some money (this is 
   particularly encouraged if you use Allegro in a commercial product), or 
   just a copy of your autoexec.bat if you don't have anything better. If 
   you redistribute parts of Allegro or make a game using it, it would be 
   nice if you mentioned me somewhere in the credits, but if you just want 
   to pinch a few routines that is OK too. I'll trust you not to rip me off.



============================================
============ Supported hardware ============
============================================

   The bare minimum you need to use Allegro is a 386 with a VGA graphics 
   card, but a 486 is strongly recommended. To get into SVGA modes you will 
   need a compatible SVGA card, which means either one of the cards that is 
   supported directly, or a card with a working VESA driver. If you have a 
   VESA 2.0 implementation such as UniVBE (which you can get from 
   http://www.scitechsoft.com/), you are fine just using that. If you don't, 
   beware. For one thing, everything will be much slower if Allegro can't 
   use the sexy VBE 2.0 features. For another, I could go on all day telling 
   horror stories about the buggy and generally just pathetic VESA 
   implementations that I've come across. If you are having trouble with the 
   SVGA modes, try getting a copy of UniVBE and see if that clears things up 
   (it probably will: SciTech usually get these things right). Even better, 
   write, or help me to write, a hardware level driver for your card (see 
   below).

   On the sound front, Allegro supports sample playback on the SB (mono), 
   the SB Pro (stereo), and the SB16. It has MIDI drivers for the OPL2 FM 
   synth (Adlib and SB cards), the OPL3 (Adlib Gold, SB Pro-II and above), 
   and the pair of OPL2 chips found in the SB Pro-I, and it can also send 
   raw MIDI data to the SB MIDI interface or an MPU-401. If you feel like 
   coming up with drivers for any other cards, they would be much 
   appreciated.

   You may have noticed that this release contains VBE/AF, ATI, Trident, 
   Video-7, and Gravis Ultrasound drivers. These are not finished, and 
   probably won't work. Don't worry though, Allegro will never autodetect 
   them, so you can safely ignore them.



============================================
============ Installing Allegro ============
============================================

   To conserve space I decided to make this a source-only distribution, so 
   you will have to compile Allegro before you can use it. To do this you 
   should:

   - Make a directory for Allegro (eg. c:\allegro) and unzip everything into 
     it. Allegro contains several subdirectories, so you must specify the -d 
     flag to pkunzip.

   - Go to your Allegro directory (where the makefile is) and run make.exe. 
     Then go do something interesting while everything compiles (it takes 
     about 20 minutes on my 486-dx33). If all goes according to plan you 
     should end up with a bunch of test programs, some tools like the 
     grabber, and the library itself, liballeg.a.

   - If you want to use the sound routines it is a good idea to set up a 
     sound.cfg file: see below.

   If for some strange reason you want to compile everything by hand, watch 
   out for the asmdef.c program, which outputs the asmdef.h that is included 
   by the .S files, and also be sure to compile the .S files with a 
   capitalised .S extension so they will be run through the C preprocessor.

   See allegro.txt for instructions on linking your program with Allegro.



===========================================
============ How to contact me ============
===========================================

   Hehe. There's a slight problem here...

   I've just graduated from York University (with a degree in music: I can 
   now put a 'BA' after my name: YES! :-) and one of the effects of this is 
   that the university computing service is taking away my net account. I'm 
   also moving to London and starting work at Probe Entertainment, so I'm 
   changing both my email and snail mail addresses. Unfortunately I don't 
   yet know what they are changing to, so I'm going to be incommunicado for 
   a while.

   Until June 28, 1996, I'll be in York and on the net as:

      slh100@tower.york.ac.uk
      http://www.york.ac.uk/~slh100/

   After that, you can contact me via my parent's address:

      Snail mail:    Shawn Hargreaves,
		     1 Salisbury Road,
		     Market Drayton,
		     Shropshire,
		     England, TF9 1AJ.

      Telephone:     UK 01630 654346

      On Foot:       Coming down Shrewsbury Road from the town centre, turn 
		     off down Salisbury Road and it is the first house on 
		     the left.

   In the longer term, I'll get myself sorted out with a modem and net 
   service provider, so if you keep an eye out on comp.os.msdos.djgpp and 
   the djgpp announcement list, I promise I will be back before too long, to 
   deal with the mountain of bug reports that are sure to accumulate... :-)

   In any case, the latest version of Allegro can always be found on 
   x2ftp.oulu.fi, in /pub/msdos/programmer/djgpp2/.



===================================
============ File list ============
===================================

ALLEGRO.H         - main header file
ALLEGRO.TXT       - library documentation
CHANGES.TXT       - list of changes from previous versions
MAKEFILE          - for gnu make
README.TXT        - what you are reading now

<DEMO>
   DEMO.C         - the demonstration game
   DEMO.DAT       - grabber datafile for the demo game
   DEMO.FLI       - an animation for the demo game
   DEMO.H         - names of objects in demo.dat, produced by the grabber
   MUSIC.TXT      - info about the music in the demo game

<EXAMPLES>
   EX1.C          - simple "hello world" program
   EX2.C          - memory bitmaps
   EX3.C          - patterns and sub-bitmaps
   EX4.C          - the pallete
   EX5.C          - keyboard input
   EX6.C          - mouse input
   EX7.C          - timers
   EX8.C          - double buffering
   EX9.C          - mode-X page flipping
   EX10.C         - fixed point math
   EX11.C         - writing directly to video memory
   EX12.C         - using datafiles
   EX13.C         - the GUI routines
   EX14.C         - writing your own GUI objects
   EX15.C         - reads and displays PCX files
   EX16.C         - playing MIDI files
   EX17.C         - playing samples
   EX18.C         - random stretch_blits, by Grzegorz Ludorowski
   EX19.C         - mode-X split screens and hardware scrolling
   EX20.C         - mode-X triple buffering and retrace syncing
   EX21.C         - animation example by Grzegorz Ludorowski
   EXAMPLE.DAT    - grabber datafile used by EX12 and EX13
   EXAMPLE.H      - names of objects in example.dat, produced by the grabber
   MYSHA.PCX      - the picture used by EX18
   RUNNING.DAT    - the animation sprites used by EX21
   RUNNING.H      - names of objects in running.dat, produced by the grabber

<SETUP>
   SETUP.CC       - sound setup program by David Calvin
   SETUP.DAT      - grabber datafile containing test sounds
   SETUP.TXT      - documentation for the sound setup program

<SRC>
   ADLIB.C        - MIDI driver for OPL synths
   ALLEGRO.C      - initialise/cleanup code
   ASMDEF.C       - writes info about structure sizes into asmdef.h
   ASMDEF.H       - automatically produced by asmdef.c
   ASMDEFS.H      - some definitions for the .S files
   ATI.C          - graphics driver for ATI cards (not finished)
   BANK.S         - SVGA bank switching code
   BLIT.S         - linear bitmap blitting functions
   CIRRUS.C       - graphics driver for Cirrus cards
   DATAFILE.C     - datafile loading routines
   DMA.C          - functions for programming the DMA controller
   FILE.C         - LZSS file compression
   FLI.C          - FLI/FLC player
   FM_INSTR.H     - FM instrument definitions for the OPL driver
   GRAPHICS.C     - lots of graphics related stuff
   GUI.C          - user interface routines
   GUS.C          - Gravis Ultrasound driver (not finished)
   INLINE.C       - compiled copies of the inline functions in allegro.h
   INTERNAL.H     - various internal definitions
   IRQ.C          - for setting up interrupt handlers
   IRQWRAP.S      - interrupt wrapper code
   JOYSTICK.C     - joystick input
   KEYBOARD.C     - keyboard input
   MATH.C         - fixed point maths
   MIDI.C         - core MIDI player
   MISC.S         - joystick polling, palletes, putpixel, lines, etc
   MODEX.C        - graphics driver for tweaked VGA mode-X
   MOUSE.C        - mouse input
   MPU.C          - MIDI driver for MPU-401 interface
   OPCODES.H      - i386 opcodes for the stretch blit and sprite compilers
   S3.C           - graphics driver for S3 cards
   SB.C           - Sound Blaster driver
   SOUND.C        - sound setup code and sample mixing
   SPRITE.S       - sprite drawing and text output
   TIMER.C        - timer interrupt routines
   TRIDENT.C      - graphics driver for Trident cards (not finished)
   TSENG.C        - graphics driver for Tseng cards
   VBEAF.C        - graphics driver using the VBE/AF interface (not finished)
   VESA.C         - graphics driver using the VESA interface
   VGA.C          - graphics driver for VGA mode 13h
   VIDEO7.C       - graphics driver for Video-7 cards (not finished)
   XGFX.S         - mode-X drawing functions

<TESTS>
   MATHTEST.C     - fixed point math test program
   PLAY.C         - sound code test program
   PLAYFLI.C      - FLI player test program
   TEST.C         - graphics test program

<TOOLS>
   GRABBER.C      - datafile editor
   GRABBER.TXT    - grabber documentation
   PACK.C         - file compression utility



=============================================
============ Sound configuration ============
=============================================

When Allegro initialises the sound routines it reads information about your 
hardware from a file called sound.cfg. If this doesn't exist it will 
autodetect (ie. guess :-) You can write your sound.cfg by hand with a text 
editor, or you can use David Calvin's setup program.

Normally setup.exe and sound.cfg will go in the same directory as the 
Allegro program they are controlling. This is fine for the end user, but it 
can be a pain for a programmer using Allegro because you may have several 
programs in different directories and want to use a single sound.cfg for all 
of them. If this is the case you can set the environment variable 'ALLEGRO' 
to the directory containing your sound.cfg, and Allegro will look there if 
there is no sound.cfg in the current directory.

Apart from comments, which begin with a '#' character and continue to the 
end of the line, sound.cfg can contain any of the statements:

digi_card = x
   Sets the driver to use for playing samples, where x is one of the values:
      0 = none            1 = SB (autodetect breed)
      2 = SB 1.0          3 = SB 1.5 
      4 = SB 2.0          5 = SB Pro
      6 = SB16

midi_card = x
   Sets the driver to use for MIDI music, where x is one of the values:
      0 = none            1 = Adlib (autodetect OPL version)
      2 = OPL2            3 = Dual OPL2 (SB Pro-1)
      4 = OPL3            5 = SB MIDI interface
      6 = MPU-401

flip_pan = x
   Toggling this between 0 and 1 reverses the left/right panning of samples, 
   which might be needed because some SB cards (including mine :-) get the 
   stereo image the wrong way round.

sb_port = x
   Sets the port address of the SB (this is usually 220).

sb_dma = x
   Sets the DMA channel for the SB (this is usually 1).

sb_irq = x
   Sets the IRQ for the SB (this is usually 7).

sb_freq = x
   Sets the sample frequency, which defaults to 16129. Possible values are:
      11906 - works with any SB
      16129 - works with any SB
      22727 - on SB 2.0 and above
      45454 - only on SB 2.0 or SB16 (not the stereo SB Pro driver)

fm_port = x
   Sets the port address of the OPL synth (this is usually 388).

mpu_port = x
   Sets the port address of the MPU-401 MIDI interface (this is usually 330).

If you are using the SB MIDI output or MPU-401 drivers with an external 
synthesiser that is not General MIDI compatible, you can also use sound.cfg 
to specify a patch mapping table for converting GM patch numbers into 
whatever bank and program change messages will select the appropriate sound 
on your synth. This is a real piece of self-indulgence. I have a Yamaha 
TG500, which has some great sounds but no GM patch set, and I just had to 
make it work somehow... If you want to use the patch mapping table, add a 
series of lines to sound.cfg, following the format:

   p<x>  <b0>  <b1>  <prog>   <pitch>

where x is the GM program change number (1-128), b0 and b1 are the two bank 
change messages to send to your synth (on controllers #0 and #32), prog is 
the program change to send to your synth, and pitch is the number of 
semitones to shift everything played with that sound. Setting the bank 
change numbers to -1 will prevent them from being sent.

For example, the line:

   p36      0        34       9        12

specifies that whenever GM program 36 (which happens to be a fretless bass) 
is selected, Allegro should send a bank change message #0 with a parameter 
of 0, a bank change message #32 with a parameter of 34, a program change 
with a parameter of 9, and then should shift everything up by an octave.



================================================
============ Notes for the musician ============
================================================

The OPL2 synth chip can provide either nine voice polyphony or six voices 
plus five drum channels. How to make music sound good on the OPL2 is left as 
an exercise for the reader :-) On an SB Pro or above you will have eighteen 
voices, or fifteen plus drums. Allegro decides whether to use drum mode 
individually for each MIDI file you play, based on whether it contains any 
drum sounds or not. If you have an orchestral piece with just the odd cymbal 
crash, you might be better removing the drums altogether as that will let 
Allegro use the non-drum mode and give you an extra three notes polyphony.

When Allegro is playing a MIDI file in looped mode, it jumps back to the 
start of the file when it reaches the end of the piece. To control the exact 
loop point, you may need to insert a dummy marker event such as a controller 
message on an unused channel.

All the OPL chips have very limited stereo capabilities. On an OPL2, 
everything is of course played in mono. On the SB Pro-I, sounds can only be 
panned hard left or right. With the OPL3 chip in the SB Pro-II and above, 
they can be panned left, right, or centre. I could use two voices per note 
to provide more flexible panning, but that would reduce the available 
polyphony and I don't want to do that. So don't try to move sounds around 
the stereo image with streams of pan controller messages, because they will 
jerk horribly. It is also worth thinking out the panning of each channel so 
that the music will sound ok on both SB Pro-I and OPL3 cards. If you want a 
sound panned left or right, use a pan value less than 48 or greater than 80. 
If you want it centred, use a pan value between 48 and 80, but put it 
slightly to one side of the exactly central 64 to control which speaker will 
be used if the central panning isn't possible.



===================================
============ Wish list ============
===================================

Support for the VBE/AF hardware accelerator API.

More graphics card drivers. These are not generally very hard to write, and 
I have specs for most of the cards out there, but there is no point in me 
churning out millions of drivers that I have no way of testing. If you have 
a card which isn't already supported (ie. most cards at the moment :-), you 
are welcome to write a driver for it yourself (in which case I would love 
you for ever!), or just to contact me and ask me to make one. If you are 
willing to put some time into testing it, I can probably come up with 
something.

Some functions for doing semi-transparent overlays and lighting effects 
using color lookup tables would be cool.

Texture mapping and gouraud shading routines.

Hi/truecolor modes. Argh, sounds like hard work. Not likely to happen any 
time soon, but maybe someday...

I want a Linux version. I can't see myself ever having time to do it, but I 
want one anyway :-)

Any other ideas? I await your suggestions...



======================================
============ Contributors ============
======================================

I hope I've remembered everyone who ought to be mentioned here. If your name 
should be included, my apologies, and please tell me so that I can correct 
the oversight.

   Arne Steinarson (arst@ludd.luth.se).
   The fixed point square root routine came from his fix-float library.

   David Calvin (calvid@rpi.edu).
   Wrote the sound setup utility.

   Dominique Biesmans (Dominique.Biesmans@ping.be).
   Wrote the mode-X version of draw_sprite() and the mode-X <-> linear 
   blitting functions.

   Fabian Nunez (fnunez@cs.uct.ac.za).
   Added support for the CH Flightstick Pro joystick and 3-button mice.

   Garret Thomson (gart@terraport.net).
   Wrote the music used in the demo game.

   Grzegorz Ludorowski (pajonk@ajax.umcs.lublin.pl).
   Wrote several of the example programs.

   Haruhiko Okumura, 12-2-404 Green Heights, 580 Nagasawa, Yokosuka 239, JP.
   This guy wrote the original version of the LZSS compression code.

   Jonathan Tarbox (battle.axe@pegasuz.com).
   Wrote the mode-X setup code, the FLI/FLC player, and contributed parts of 
   the joystick handler.

   Marcel de Kogel (m.dekogel@student.utwente.nl).
   Not content with fixing my broken MPU-401 driver, Marcel went on to 
   provide a set of vastly improved drum sounds for the OPL driver, to help 
   me sort out some problems with reentrant interrupts, to supply the half 
   of the joystick code that didn't come from Jonathan, and to locate a 
   stupid mistake in my VESA linear framebuffer code.

   Mark Wodrich (mwodric@eleceng.uct.ac.za).
   The brain behind sub-bitmaps, flicker-free mouse pointers, and the 
   ability to import GRX .FNT files into the grabber.

   Nathan Albury, aka Rubicant (gt4558a@prism.gatech.edu).
   Improved the fire routine in examples/ex11.c (my original version didn't 
   really look very much like flames :-)

   Peter Monks (Peter_Monks@australia.notes.pw.com).
   Wrote the Video-7 graphics driver, and showed me how to set up the 
   unchained 640x400 mode.



=================================
============ Thanks! ============
=================================

   First, a big thank you to all the people who helped me test and debug 
   this code. It sometimes gets frustrating to receive hundreds of messages 
   saying "it doesn't work!", but they are useful all the same...

   Andr‚ Baresel (baresel@informatik.hu-berlin.de), and Craig Jackson 
   (Craig.Jackson@launchpad.unc.edu), provided a tremendous amount of 
   information about SB hardware.

   Charles Sandmann (sandmann@clio.rice.edu), DJ Delorie (dj@delorie.com), 
   and everyone else who has contributed to djgpp. I love it.

   C. Schwerdtfeger (schwerdt@physics.ubc.ca), for his help (and enourmous 
   patience!) in getting the SB Pro-I MIDI driver to work.

   Finn Thoegersen, Nordbanevej 3 C, DK-7800 Skive, Denmark.
   Most of my SVGA hardware information came from his VGADOC package.

   Eric Jorgensen (smeagol@rt66.com).
   Varmint's Audio Tools (VAT) gave me many useful ideas about how to play 
   MIDI files.

   Jean-Paul Mikkers (mikmak@stack.urc.tue.nl).
   MikMod was the source of a lot of information about programming the SB, 
   and also gave me the idea of constantly reprogramming the PIT to get a 
   really high resolution timer.

   Joel H. Hunter (jhunter@kendaco.telebyte.com).
   His SB library for djgpp is excellent, and helped me a lot.

   John Pollard (74723.1626@compuserve.com).
   The FM instrument definitions are based on his MID-KIT library.

   Kendall Bennett and all the other cool people at SciTech Software.
   These guys gave the world UniVBE, the VBE/AF API, and a lot of free 
   information and example VESA code. Plus they very kindly sent me a copy 
   of the VBE/AF spec when I wanted one.

   Kris Heidenstrom (kheidens@actrix.gen.nz).
   His PC timing FAQ was a big help.

   Mark Feldman.
   It goes without saying that the PCGPE was an invaluable resource.

   Michael Abrash.
   You just gotta love that man...

   Paul Fenwick (bg914@freenet.carleton.ca).
   Various bits of the mode-X code (notably the split screen stuff) came 
   from his XLIBDJ library.

   Robert Schmidt (robert@stud.unit.no).
   The register values for the 400x* mode-X resolutions came from his TWEAK 
   program.

   Vladimir Arnost (xarnos00@dcse.fee.vutbr.cz).
   Provided hardware specs for the OPL3 chip.

   Pink Floyd, the Doors, Tori Amos, and all the other great musicians who 
   provide me with things to listen to while I am programming.

   My parents, John and Nancy.
   I would never have made it through all those late night coding sessions 
   without the cafetiere they gave me last Christmas.



By Shawn Hargreaves,
1 Salisbury Road,
Market Drayton,
Shropshire,
England, TF9 1AJ.

Email sadly not available right now: see "How to contact me" above.
