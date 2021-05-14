	 ______   ___    ___
	/\  _  \ /\_ \  /\_ \
	\ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
	 \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
	  \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
	   \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
	    \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
					   /\____/
					   \_/__/     Version 2.0


	       A game programming library for djgpp

		   By Shawn Hargreaves, 1994/96



#include <std_disclaimer.h>

   "I do not accept responsibility for any effects, adverse or otherwise, 
    that this code may have on you, your computer, your sanity, your dog, 
    and anything else that you can think of. Use it at your own risk."



=====================================
============ Allegro FAQ ============
=====================================

1: What is Allegro?
   ~~~~~~~~~~~~~~~~
   Allegro is a library of functions for use in computer games, written for 
   djgpp v2 in a mixture of C and 32 bit asm.


2: What does the name 'Allegro' mean?
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Once upon a time I used to program on the Atari ST, and Allegro began 
   life on that, hence the acronym 'Atari Low Level Game Routines'. The name 
   is just about the only thing left of the original Atari code, though.


3: What can Allegro do?
   ~~~~~~~~~~~~~~~~~~~~
   Supports SVGA graphics modes from 320x200 up to 1024x768. Uses VESA 2.0 
   linear framebuffers and protected mode bank switching if they are 
   available, and also contains register level drivers for Cirrus, S3, and 
   Tseng cards.

   Graphics routines including putpixel, getpixel, lines, rectangles, 
   polygons, circles, masked and run length encoded sprites, blitting, 
   bitmap scaling and rotation, and text output with user-defined 
   proportional fonts. All of these support clipping, and can draw to the 
   screen or to memory bitmaps of any size.

   Hardware scrolling and pallete manipulation.

   Plays background MIDI music and up to eight simultaneous sound effects. 
   Samples can be looped, and the volume, pan, and pitch can be altered 
   while they are playing. The MIDI player responds to note on, note off, 
   main volume, pan, pitch bend, and program change messages, using the 
   General Midi patch set and drum mappings. Currently supports Adlib, SB, 
   SB Pro, SB16, and MPU-401.

   Easy access to the mouse, keyboard, joystick, and high resolution timer 
   interrupts.

   Routines for reading and writing LZSS compressed files.

   Multi-object data files and a bitmap grabber utility.

   Fixed point 16.16 math routines including lookup table trig.

   GUI dialog manager and file selector.


4: What about copyright?
   ~~~~~~~~~~~~~~~~~~~~~
   Allegro is swap-ware. You may use, modify, redistribute, and generally 
   hack it about in any way you like, but if you do you must send me 
   something in exchange. This could be a complimentary copy of a game, an 
   addition or improvement to Allegro, a bugfix, some money (this is 
   particularly encouraged if you use Allegro in a commercial product), or 
   just a copy of your autoexec.bat if you don't have anything better. If 
   you redistribute parts of Allegro or make a game using it, it would be 
   nice if you mentioned me somewhere in the credits, but if you just want 
   to pinch a few routines that is OK too. I'll trust you not to rip me off.


5: What hardware does it require/support?
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   The bare minimum is a 386 with a VGA graphics card, but a 486 is strongly 
   recommended. To get into SVGA modes you will need a compatible SVGA card, 
   which means either one of the cards that is supported directly or a card 
   with a working VESA driver. If you have a VESA 2.0 implementation such as 
   UniVBE (which you can get from http://www.scitechsoft.com/), you are fine 
   just using that. If you don't, beware. For one thing, everything will be 
   much slower if Allegro can't use the sexy VESA 2.0 features. For another, 
   I could go on all day telling horror stories about all the buggy and 
   generally just pathetic VESA implementations that I've come across. Like 
   the one that quite happily let me set a 2048x1024 virtual screen on a 
   card with only one megabyte of video ram. And the one that uses a random 
   number generator to calculate the byte offset for the 'set display start 
   address' function. If you are having trouble with the SVGA modes, try 
   getting a copy of UniVBE and see if that clears things up. Even better, 
   write, or help me to write, a hardware level driver for your card (see 
   below).

   On the sound front, Allegro supports sample playback on the SB (mono), 
   the SB Pro (stereo), and the SB16. It has MIDI drivers for the OPL2 FM 
   synth (Adlib and SB cards), the OPL3 (Adlib Gold, SB Pro-II and above), 
   and the pair of OPL2 chips found in the SB Pro-I. It can also send raw 
   MIDI data to the SB MIDI interface or an MPU-401. Jesse Morris 
   (cuth@gryphon.ccs.brandeis.edu, jrmorris@macalstr.edu) has offered to 
   write some GUS code. If you feel like coming up with drivers for any 
   other soundcards, they would be much appreciated.


6: How do I install Allegro?
   ~~~~~~~~~~~~~~~~~~~~~~~~~
   To conserve space I decided to make this a source-only distribution, so 
   you will have to compile Allegro before you can use it. The supplied 
   makefile will do this for you: just unzip all the Allegro files into a 
   single directory (eg. c:\allegro) and run make. Allegro will only work 
   with v2 of djgpp, so if you are still using v1 or a beta release of v2, 
   you are out of luck. If for some strange reason you want to compile it by 
   hand, watch out for the asmdef.c program, which outputs the asmdef.h that 
   is included by the .S files, and also be sure to compile the .S files 
   with a capitalised .S extension so they will be run through the C 
   preprocessor.

   If all goes according to plan, you should end up with a bunch of test 
   .exe files, the grabber.exe utility, and the library itself, liballeg.a. 
   If you have any trouble, mail me and I'll send you compiled versions of 
   everything.


7: How can I get in touch with you?
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   Email:         slh100@tower.york.ac.uk
   WWW:           http://www.york.ac.uk/~slh100/

      Note: these addresses will only exist until the end of June 1996, at 
      which point I will be graduating and therefore getting a new address.
      I will also be looking for a job. Hint hint...

      Also I'm going to be away from next week (March 1996) until the end of 
      April. By all means go ahead and write to me, but don't expect an 
      immediate reply. How am I going to manage without my daily net intake, 
      I wonder? I really need to buy a modem :-)

      If you want to get in touch with me after this summer, go download the 
      latest version of Allegro and my new address will be in there.

   Snail mail:    Shawn Hargreaves,
		  1 Salisbury Road,
		  Market Drayton,
		  Shropshire,
		  England, TF9 1AJ.

   Telephone:     UK 01630 654346

   On Foot:       Coming down Shrewsbury Road from the town centre, turn off 
		  down Salisbury Road and it is the first house on the left.


8: Why is this question here?
   ~~~~~~~~~~~~~~~~~~~~~~~~~~
   Because otherwise I would have a list of seven, which is a prime number 
   and not aesthetically pleasing.



===================================
============ File list ============
===================================

<documentation>
README.TXT        - what you are reading now
ALLEGRO.TXT       - the main documentation
CHANGES.TXT       - list of changes from previous versions
MUSIC.TXT         - information about the music in the demo game

<misc>
MAKEFILE          - you'll probably need gnu make to use this
SOUND.CFG         - soundcard configuration file (see below)

<header files>
ALLEGRO.H         - include this in all programs that use Allegro
ALLEGRO1.H        - for compatibility with old programs

<internal header files>
INTERNAL.H        - some internal definitions
ASMDEF.C          - writes structure sizes into asmdef.h
ASMDEF.H          - structure sizes, automatically produced by asmdef.exe
ASMDEFS.H         - definitions for use by the asm code
FM_INSTR.H        - FM instrument definitions for the Adlib driver 

<library code>
ADLIB.C           - driver for OPL FM synth chips
ALLEGRO.C         - various bits and pieces, initialise/shutdown routines
BANK.S            - SVGA bank switching code
BLIT.S            - bitmap clear and blitting routines
CIRRUS.C          - graphics drivers for Cirrus 64xx and 54xx
DATAFILE.C        - the datafile loader
DMA.C             - routines for programming the DMA controller
FILE.C            - read/write LZSS compressed files
GRAPHICS.C        - pallete fading, circles, proportional fonts
GUI.C             - GUI routines
GUS.C             - stub GUS driver (the real code isn't written yet)
IRQ.C             - hardware interrupt manager
IRQWRAP.S         - asm wrappers for the hardware interrupt routines
JOYSTICK.C        - joystick routines
KEYBOARD.C        - keyboard module
MATH.C            - fixed point math routines
MIDI.C            - MIDI file player
MISC.S            - pallete, putpixel, lines, etc.
MOUSE.C           - mouse module
MPU.C             - driver for MPU-401 MIDI interface
S3.C              - graphics driver for S3 cards
SB.C              - Sound Blaster driver
SOUND.C           - sound setup and sample mixing code
SPRITE.S          - sprite and text output routines
TIMER.C           - timer module
TSENG.C           - graphics drivers for Tseng ET3000 and ET4000
VESA.C            - VESA graphics driver
VGA.C             - standard VGA (mode 13h) graphics driver

<test programs>
CP.C              - file compression test program
DEMO.C            - demonstration game
DEMO.DAT          - datafile for the demo game
DEMO.H            - produced by the grabber
GRABBER.C         - grabber utility
MATHTEST.C        - fixed point math test program
PLAY.C            - sound test program
TEST.C            - graphics test program



=============================================
============ Sound configuration ============
=============================================

When Allegro initialises the sound routines, it reads a file called 
sound.cfg from the current directory. If the soundcard autodetection doesn't 
work properly, you can use this file to control the sound code.

The '#' character begins a comment. Apart from comments, sound.cfg can 
contain any of the statements:

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
   Sets the sample playback frequency. This defaults to 16000. Other 
   possible values are 8000, 11025, and 22050.

fm_port = x
   Sets the port address of the OPL synth (this is usually 388).

mpu_port = x
   Sets the port address of the MPU-401 MIDI interface (this is usually 330).

If you are using the SB MIDI output or MPU-401 drivers with an external 
synthesiser which is not General MIDI compatible, you can also use sound.cfg 
to specify a patch mapping table for converting GM patch numbers into 
whatever bank and program change messages will select the appropriate sound 
on your synth. This is a pure piece of self-indulgence. I have a Yamaha 
TG500, which has some cool sounds but no GM patch set, and I just had to 
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

The OPL2 synth chip can provides either nine voice polyphony or six voices 
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
the stereo image with streams of pan controller messages, because they are 
will jerk horribly. It is also worth thinking out the panning of each 
channel so that the music will sound ok on both SB Pro-I and OPL3 cards. If 
you want a sound panned left or right, use a pan value less than 48 or 
greater than 80. If you want it centred, use a pan value between 48 and 80, 
but put it slightly to one side of the exactly central 64 to control which 
speaker will be used if central panning isn't possible.



===================================
============ Wish list ============
===================================

More graphics card drivers. These are not generally very hard to write, and 
I have specs for most of the cards out there, but there is no point in me 
churning out millions of drivers that I have no way of testing. If you have 
a card which isn't already supported (ie. most cards at the moment :-), you 
are welcome to write a driver for it yourself (in which case I would love 
you for ever!), or just to contact me and ask me to make one. If you are 
willing to put some time into testing it, I can probably come up with 
something.

Add some menu support to the GUI code.

Some functions for doing transparent bitmap overlays with a pallete table 
would be cool.

A mod player would be really nice. But it would be a lot of work, and I have 
a degree to finish, so it might have to wait a while.

Maybe some 3d stuff?

Any other ideas? I await your suggestions...



============================================
============ Creative Labs suck ============
============================================

Once upon a time Creative Labs made a soundcard called the AWE32. And the 
people said "Lo, this card is cool, but where are the programs that support 
it?". And they said unto the programmers that they should add AWE code to 
all their games. And in their ignorance the programmers went to Creative 
Labs and asked them how to play sounds on the AWE. And Creative Labs said 
"bugger off, that's proprietary information!".

All that Creative Labs provides is a precompiled library of functions to 
access the AWE hardware. Of course this library won't work with djgpp, and 
when I asked them to port it their response was that I should switch to 
Watcom! I even offered to port it for them myself, but they refused to let 
me have the sources. So if you would like to see an AWE32 driver in Allegro, 
please write lots of angry mail to Creative Labs and try to persuade them 
that djgpp is a compiler worth supporting.



======================================
============ Contributors ============
======================================

I hope I've remembered everyone who ought to be included here. If your name 
should be in this list, my apologies, and please tell me so that I can 
correct the oversight.

   Arne Steinarson (arst@ludd.luth.se).
   The fixed point square root routine came from his fix-float library. 

   Haruhiko Okumura, 12-2-404 Green Heights, 580 Nagasawa, Yokosuka 239, JP.
   This guy wrote the original version of the LZSS compression code.

   Jonathan Tarbox (battle.axe@pegasuz.com).
   A lot of the joystick stuff is based on some code he sent me.

   Marcel de Kogel (m.dekogel@student.utwente.nl).
   Not content with fixing my broken MPU-401 driver, Marcel went on to 
   provide a set of vastly improved drum sounds for the OPL driver, to help 
   me sort out some problems with reentrant interrupts, to supply the half 
   of the joystick code that didn't come from Jonathan, and to locate a 
   stupid mistake in my VESA linear framebuffer code.



=================================
============ Thanks! ============
=================================

   First, a big thak you to all the people who helped me test and debug this 
   code. It sometimes gets frustrating to receive hundreds of messages 
   saying "it doesn't work!", but they are useful all the same :-)

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
   MikMod was the source of a lot of ideas and information about programming 
   the SB, and also gave me the idea of constantly reprogramming the PIT to 
   get a really high resolution timer.

   Joel H. Hunter (jhunter@kendaco.telebyte.com).
   His SB library for djgpp is excellent, and helped me a lot.

   John Pollard (74723.1626@compuserve.com).
   The FM instrument definitions are based on his MID-KIT library.

   Mark Feldman (address unknown: you can find him on rec.games.programmer).
   It goes without saying that the PCGPE was an invaluable resource.

   Vladimir Arnost (xarnos00@dcse.fee.vutbr.cz).
   Provided hardware specs for the OPL3 chip.

   Pink Floyd, the Doors, Bela Bartok, Stravinsky, and all the other 
   musicians who provide me with things to listen to while I am programming.

   My parents, John and Nancy.
   I would never have made it through all those late night coding sessions 
   without the cafetiere they gave me for Christmas.



By Shawn Hargreaves,
1 Salisbury Road,
Market Drayton,
Shropshire,
England, TF9 1AJ.

Email slh100@tower.york.ac.uk. This will only exist until summer 1996! If 
you want to contact me after this summer, go download the latest version of 
Allegro, and my new address will be in there.
