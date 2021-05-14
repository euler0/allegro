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
 *      Digitized sample driver for the MIDI player (unfinished).
 *
 *      By Tom Novelli
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <dos.h>

#include "allegro.h"
#include "internal.h"


/* external interface to the Digmid driver */
static int digmid_detect();
static int digmid_init();
static void digmid_exit();
static void digmid_key_on(int voice, int inst, int note, int bend, int vol, int pan);
static void digmid_key_off(int voice);
static void digmid_set_volume(int voice, int vol);
static void digmid_set_pitch(int voice, int note, int bend);
static int digmid_mixer_volume(int volume);

static char digmid_desc[80] = "not initialised";


MIDI_DRIVER midi_digmid =
{
   "DigMid",
   digmid_desc,
   MIXER_MAX_SFX,       /* Could make this user-adjustable */
   digmid_detect,
   digmid_init,
   digmid_exit,
   _dummy_load_patches,
   digmid_key_on,
   digmid_key_off,
   digmid_set_volume,
   digmid_set_pitch,
   digmid_mixer_volume,
   NULL
};

/* Which digi voice each midi voice is using (-1 if none) */
int voice_active[MIXER_MAX_SFX] = {-1};

/* Table to link MIDI patches to samples */
SAMPLE *inst_to_smp[128] = {NULL};


#define FM_HH     1
#define FM_CY     2
#define FM_TT     4
#define FM_SD     8
#define FM_BD     16


/* Frequency table - C5 is original pitch */
int ftbl[] = {
	31,    33,    35,    37,    39,    42,          /* C0 */
	44,    47,    50,    53,    56,    59,
	62,    66,    70,    74,    79,    83,          /* C1 */
	88,    94,    99,    105,   111,   118,
	125,   132,   140,   149,   157,   167,         /* C2 */
	177,   187,   198,   210,   223,   236,
	250,   265,   281,   297,   315,   334,         /* C3 */
	354,   375,   397,   420,   445,   472,
	500,   530,   561,   595,   630,   667,         /* C4 */
	707,   749,   794,   841,   891,   944,
	1000,  1059,  1122,  1189,  1260,  1335,        /* C5 */
	1414,  1498,  1587,  1682,  1782,  1888,
	2000,  2119,  2245,  2378,  2520,  2670,        /* C6 */
	2828,  2997,  3175,  3364,  3564,  3776,
	4000,  4238,  4490,  4757,  5040,  5339,        /* C7 */
	5657,  5993,  6350,  6727,  7127,  7551,
	8000,  8476,  8980,  9514,  10080, 10679,       /* C8 */
	11314, 11987, 12699, 13455, 14255, 15102,
	16000, 16951, 17959, 19027, 20159, 21357,       /* C9 */
	22627, 23973, 25398, 26909, 28509, 30204
};



/* digmid_reset:
 * Should we just use digi_driver->reset()?
 */
static void digmid_reset(int enable)
{
}

static END_OF_FUNCTION(digmid_reset);



/* digmid_key_on:
 *  Triggers the specified voice. The instrument is specified as a GM
 *  patch number, pitch as a midi note number, and volume from 0-127.
 *  The bend parameter is _not_ expressed as a midi pitch bend value.
 *  It ranges from 0 (no pitch change) to 0xFFF (almost a semitone sharp).
 *  Drum sounds are indicated by passing an instrument number greater than
 *  128, in which case the sound is GM percussion key #(inst-128).
 *
 * To do:
 *  Implement pitch bends
 */
static void digmid_key_on(int voice, int inst, int note, int bend, int vol, int pan)
{
      /* make sure the voice isn't sounding */
      if (voice_active[voice] >= 0) {
	digi_driver->stop(voice_active[voice]);
	voice_active[voice] = -1;
      }

      /* MIDI pan & volume ranges are 0-127; samples use 0-255 */
      /* Could use bitshift to multiply by 2...? */
      pan *= 2;
      vol *= 2;

      /* and play the note */
      voice_active[voice] = 0;

      /* play sample should return a voice number! coming up when the planned
       * sound API improvements get done...
       */
      play_sample(inst_to_smp[inst], vol, pan, ftbl[note], FALSE);
}

static END_OF_FUNCTION(digmid_key_on);



/* digmid_key_off:
 *  Hey, guess what this does :-)
 */
static void digmid_key_off(int voice)
{
   digi_driver->stop(voice_active[voice]);
   voice_active[voice] = -1;
}

static END_OF_FUNCTION(digmid_key_off);



/* digmid_set_volume:
 *  Sets the volume of the specified voice (vol range 0-127).
 */
static void digmid_set_volume(int voice, int vol)
{
}

static END_OF_FUNCTION(digmid_set_volume);



/* digmid_set_pitch:
 *  Sets the pitch of the specified voice.
 */
static void digmid_set_pitch(int voice, int note, int bend)
{
}

static END_OF_FUNCTION(digmid_set_pitch);


/* digmid_mixer_volume:
 *  Sets global music volume
 */
static int digmid_mixer_volume(int volume)
{
   /* THIS NEEDS CHANGING... */
   return _sb_set_mixer(-1, volume);
}



/* digmid_detect:
 *  Just look at digi_driver settings for this? Determine if digmid is the
 *  optimal driver - if real MIDI is available, use that - if DSP is
available,
 *  use digmid.
 */
static int digmid_detect()
{
   sprintf(digmid_desc, "Digitized sample MIDI output on sound card");

   midi_digmid.voices = MIXER_MAX_SFX;

   return TRUE;
}



/* digmid_init:
 *  Setup the digmid driver.
 */
static int digmid_init()
{
   if (!digmid_detect())
      return -1;

   digmid_reset(1);

   LOCK_VARIABLE(midi_digmid);
   LOCK_FUNCTION(digmid_reset);
   LOCK_FUNCTION(digmid_key_on);
   LOCK_FUNCTION(digmid_key_off);
   LOCK_FUNCTION(digmid_set_volume);
   LOCK_FUNCTION(digmid_set_pitch);

   return 0;
}



/* digmid_exit:
 *  Cleanup when we are finished.
 */
static void digmid_exit()
{
   digmid_reset(0);
}


