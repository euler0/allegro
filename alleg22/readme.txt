	 ______   ___    ___
	/\  _  \ /\_ \  /\_ \
	\ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___ 
	 \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
	  \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
	   \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
	    \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
					   /\____/
					   \_/__/     Version 2.2


	       A game programming library for djgpp

		   By Shawn Hargreaves, 1994/97



#include <std_disclaimer.h>

   "I do not accept responsibility for any effects, adverse or otherwise, 
    that this code may have on you, your computer, your sanity, your dog, 
    and anything else that you can think of. Use it at your own risk."



======================================
============ Introduction ============
======================================

   Allegro is a library of functions for use in computer games, written for 
   djgpp in a mixture of C and assembly language. This is version 2.2: see 
   changes.txt for a list of differences from the previous release.

   According to the Oxford Companion to Music, Allegro is the Italian for 
   "quick, lively, bright". Once upon a time it was also an acronym for 
   "Atari Low Level Game Routines", but it is a long time since I did any 
   programming on the Atari, and the name is pretty much the only thing left 
   of the original Atari code.



==================================
============ Features ============
==================================

   Supports VGA mode 13h, mode-X (twenty two tweaked VGA resolutions plus 
   unchained 640x400 Xtended mode), and 256 color SVGA modes. Uses VESA 2.0 
   linear framebuffers if they are available, and also contains register 
   level drivers for Cirrus, Paradise, S3, Trident, Tseng, and Video-7 cards.

   Drawing functions including putpixel, getpixel, lines, rectangles, flat 
   shaded, gouraud shaded, and texture mapped polygons, circles, floodfill, 
   bezier splines, patterned fills, masked, run length encoded, and compiled 
   sprites, blitting, bitmap scaling and rotation, translucency/lighting, 
   and text output with proportional fonts. Supports clipping, and can draw 
   directly to the screen or to memory bitmaps of any size.

   Hardware scrolling, mode-X split screens, and palette manipulation.

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

   Math functions including fixed point arithmetic, lookup table trig, and 
   3d vector/matrix manipulation.

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
   (it probably will: SciTech usually get these things right).

   On the sound front, Allegro supports sample playback on the SB (mono), 
   the SB Pro (stereo), and the SB16. It has MIDI drivers for the OPL2 FM 
   synth (Adlib and SB cards), the OPL3 (Adlib Gold, SB Pro-II and above), 
   and the pair of OPL2 chips found in the SB Pro-I, and it can also send 
   raw MIDI data to the SB MIDI interface or an MPU-401. If you feel like 
   coming up with drivers for any other cards, they would be much 
   appreciated.

   You may have noticed that this release contains ATI, Gravis Ultrasound, 
   and digital MIDI drivers. These are not finished, and won't work. Don't 
   worry though, Allegro will never autodetect them, so you can safely 
   ignore them. There is also some code and makefile instructions for 
   building a Linux version, but don't bother trying this: it won't work! A 
   _lot_ more work is needed before Allegro will be usable under Linux.

   There is also the first part of a VBE/AF driver in this package. VBE/AF 
   is an extension to the VBE 2.0 API which provides a standard way of 
   accessing hardware accelerator features. My driver implements all the 
   basic mode set and bank switch functions, so it works as well as the 
   regular VESA drivers, but it doesn't yet support any accelerated drawing 
   operations. This is because SciTech have, so far, only implemented VBE/AF 
   on the ATI Mach64 chipset, and I don't have access to a Mach64. I'll 
   finish the driver as soon as they implement VBE/AF on the Matrox Mystique.



============================================
============ Installing Allegro ============
============================================

   To conserve space I decided to make this a source-only distribution, so 
   you will have to compile Allegro before you can use it. To do this you 
   should:

   - Make a directory for Allegro (eg. c:\allegro) and unzip everything into 
     it. Allegro contains several subdirectories, so you must specify the -d 
     flag to pkunzip.

   - Go to your Allegro directory and run make.exe. Then go do something 
     interesting while everything compiles. If all goes according to plan 
     you will end up with a bunch of test programs, some tools like the 
     grabber, and the library itself, liballeg.a.

     If you have any trouble with the build, look at docs/faq.txt for the 
     solutions to some of the more common problems.

     Note: make.exe is not a part of Allegro, it is part of djgpp. It is in 
     v2gnu/mak*b.zip (whatever the latest version number is) in the same 
     place as the rest of djgpp.

   - If you want to use the sound routines it is a good idea to set up a 
     sound.cfg file: see below.

   To use Allegro in your programs you should:

   - Put the following line at the beginning of all C or C++ files that use 
     Allegro:

	 #include <allegro.h>

   - If you compile from the command line or with a makefile, add '-lalleg' 
     to the end of the gcc command, eg:

	 gcc foo.c -o foo.exe -lalleg

   - If you are using Rhide, go to the Options/Libraries menu, type 'alleg' 
     into the first empty space, and make sure the box next to it is checked.

   See docs\allegro.txt for details of how to use the Allegro functions.



===========================================
============ How to contact me ============
===========================================

   Email:               shawn@talula.demon.co.uk

   WWW:                 http://www.talula.demon.co.uk/

   Allegro homepage:    http://www.talula.demon.co.uk/allegro/

   Snail mail:          Shawn Hargreaves,
			1 Salisbury Road,
			Market Drayton,
			Shropshire,
			England, TF9 1AJ.

   Telephone:           UK 01630 654346

   On Foot:             Coming down Shrewsbury Road from the town centre, 
			turn off down Salisbury Road and it is the first 
			house on the left.

   If all else fails:   52 deg 54' N
			2 deg 29' W

   The latest version of Allegro can always be found on x2ftp.oulu.fi, in 
   /pub/msdos/programmer/djgpp2/, and on the Allegro homepage, 
   http://www.taula.demon.co.uk/allegro/.



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
      0 = none                   1 = SB (autodetect breed)
      2 = SB 1.0                 3 = SB 1.5 
      4 = SB 2.0                 5 = SB Pro
      6 = SB16                   7 = GUS (unfinished)

midi_card = x
   Sets the driver to use for MIDI music, where x is one of the values:
      0 = none                   1 = Adlib (autodetect OPL version)
      2 = OPL2                   3 = Dual OPL2 (SB Pro-1)
      4 = OPL3                   5 = SB MIDI interface
      6 = MPU-401                7 = GUS (unfinished)
      8 = DigMidi (unfinished)

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

More graphics and soundcard drivers. Due to a lack of free time and lack of 
hardware for testing, I've more or less given up on writing these myself, 
but I'd be delighted to assist anyone who feels like working on one. 
Alternatively, if you buy me a card I can probably make a driver that will 
work on it...

Hi/truecolor video modes. Coming in the next version.

GUS driver, sample-based MIDI driver for SB cards, and better sound API. 
Tero Parvinen and Tom Novelli are working on this.

3d polygon clipping and Z-buffering.

I want a Linux version. No promises as to when I'll have time to do it, 
though...

Windows? Direct-X? Not going to happen anytime soon, but I can still dream 
about it :-)

Any other ideas? I await your suggestions...



By Shawn Hargreaves,
1 Salisbury Road,
Market Drayton,
Shropshire,
England, TF9 1AJ.

shawn@talula.demon.co.uk
http://www.talula.demon.co.uk/
