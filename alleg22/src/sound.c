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
 *      Sound setup routines and sample mixing code.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dir.h>

#ifdef DJGPP
#include <go32.h>
#include <sys/farptr.h>
#endif

#include "allegro.h"
#include "internal.h"


typedef struct DIGI_DRIVER_INFO     /* info about a digital sound driver */
{
   int driver_id;                   /* integer ID defined in allegro.h */
   DIGI_DRIVER *driver;             /* the driver structure */
   int autodetect;                  /* set to allow autodetection */
} DIGI_DRIVER_INFO;


typedef struct MIDI_DRIVER_INFO     /* info about a MIDI sound driver */
{
   int driver_id;                   /* integer ID defined in allegro.h */
   MIDI_DRIVER *driver;             /* the driver structure */
   int autodetect;                  /* set to allow autodetection */
} MIDI_DRIVER_INFO;



/* Alter these tables to add or remove specific sound drivers.
 *
 * The first field is the integer ID defined in allegro.h, which is passed
 * to install_sound(). The second is a pointer to the driver structure, and 
 * the third is a flag indicating whether it is safe to autodetect this 
 * driver. Autodetection takes place if you pass DIGI_AUTODETECT or 
 * MIDI_AUTODETECT, and works down this list from top to bottom until it 
 * finds a suitable driver.
 *
 * Note that the various breeds of SB and OPL drivers are not individually
 * autodetected, only the generic one (DIGI_SB or MIDI_ADLIB), which will
 * adjust itself depending on which type of card is present. There is also
 * no autodetection for the SB MIDI output and MPU-401 drivers, because these
 * are interfaces rather than soundcards in themselves, and are present in
 * a lot of machines without being connected to anything.
 */

static DIGI_DRIVER_INFO digi_driver_list[] =
{
   /* driver ID         driver structure     autodetect flag */
   #ifdef DJGPP
      /* djgpp drivers */
      {  DIGI_GUS,         &digi_gus,           TRUE   },
      {  DIGI_SB,          &digi_sb,            TRUE   },
      {  DIGI_SB10,        &digi_sb,            FALSE  },
      {  DIGI_SB15,        &digi_sb,            FALSE  },
      {  DIGI_SB20,        &digi_sb,            FALSE  },
      {  DIGI_SBPRO,       &digi_sb,            FALSE  },
      {  DIGI_SB16,        &digi_sb,            FALSE  } ,

   #else 
      /* linux drivers */
   #endif

   {  DIGI_NONE,        &digi_none,          TRUE  },
   {  0,                NULL,                0     }
};


static MIDI_DRIVER_INFO midi_driver_list[] =
{
   /* driver ID         driver structure     autodetect flag */
   #ifdef DJGPP
      /* djgpp drivers */
      {  MIDI_GUS,         &midi_gus,           TRUE   },
      {  MIDI_ADLIB,       &midi_adlib,         TRUE   },
      {  MIDI_OPL2,        &midi_adlib,         FALSE  },
      {  MIDI_2XOPL2,      &midi_adlib,         FALSE  },
      {  MIDI_OPL3,        &midi_adlib,         FALSE  },
      {  MIDI_SB_OUT,      &midi_sb_out,        FALSE  },
      {  MIDI_MPU,         &midi_mpu401,        FALSE  },
      {  MIDI_DIGMID,      &midi_digmid,        FALSE  },

   #else 
      /* linux drivers */
   #endif

   {  MIDI_NONE,        &midi_none,          TRUE  },
   {  0,                NULL,                0     }
};



/* dummy functions for the nosound drivers */
int _dummy_detect() { return TRUE; }
int _dummy_init() { return 0; }
void _dummy_exit() { }
void _dummy_play(int voice, SAMPLE *spl, int vol, int pan, int freq, int loop) { }
void _dummy_adjust(int voice, int vol, int pan, int freq, int loop) { }
void _dummy_stop(int voice) { }
unsigned long _dummy_voice_status(int voice) { return 0; }
int _dummy_load_patches(char *patches, char *drums) { return 0; }
void _dummy_key_on(int voice, int inst, int note, int bend, int vol, int pan) { }
void _dummy_key_off(int voice) { }
void _dummy_set_volume(int voice, int vol) { }
void _dummy_set_pitch(int voice, int note, int bend) { }

/* put this after all the dummy functions, so they will all get locked */
END_OF_FUNCTION(_dummy_detect); 



DIGI_DRIVER digi_none =
{
   "No sound", "The sound of silence",
   0,
   _dummy_detect,
   _dummy_init,
   _dummy_exit,
   _dummy_play,
   _dummy_adjust,
   _dummy_stop,
   _dummy_voice_status,
   NULL
};


MIDI_DRIVER midi_none =
{
   "No sound", "The sound of silence",
   0,
   _dummy_detect,
   _dummy_init,
   _dummy_exit,
   _dummy_load_patches,
   _dummy_key_on,
   _dummy_key_off,
   _dummy_set_volume,
   _dummy_set_pitch,
   NULL,
   NULL
};


int digi_card = DIGI_AUTODETECT;
int midi_card = MIDI_AUTODETECT;

DIGI_DRIVER *digi_driver = &digi_none;
MIDI_DRIVER *midi_driver = &midi_none;

static int sound_installed = FALSE;

static SAMPLE *digi_voice[DIGI_VOICES];   /* list of active samples */


int _digi_volume = -1;                    /* current volume settings */
int _midi_volume = -1;

int _flip_pan = FALSE;                    /* reverse l/r sample panning? */


typedef struct MIXER_SAMPLE
{
   unsigned char *data;                   /* NULL = inactive */
   unsigned long len;                     /* fixed point sample length */
   unsigned long pos;                     /* fixed point position in sample */
   unsigned long diff;                    /* fixed point speed of play */
   int lvol;                              /* left channel volume */
   int rvol;                              /* right channel volume */
   int loop;                              /* loop the sample? */
} MIXER_SAMPLE;


/* the samples currently being played */
static MIXER_SAMPLE mixer_sample[MIXER_MAX_SFX]; 

/* temporary sample mixing buffer */
static unsigned short *mix_buffer = NULL; 

/* lookup table for converting sample volumes */
typedef signed short MIXER_VOL_TABLE[256];
static MIXER_VOL_TABLE *mix_vol_table = NULL;

/* lookup table for amplifying and clipping samples */
static unsigned short *mix_clip_table = NULL;

#define MIX_RES_16      14
#define MIX_RES_8       10

/* flags for the mixing code */
static int mix_size;
static int mix_freq;
static int mix_stereo;
static int mix_16bit;


static void sound_lock_mem();



/* parse_string:
 *  Splits a string into component parts, storing them in the argv pointers.
 *  Returns the number of components.
 */
int parse_string(char *buf, char *argv[])
{
   int c = 0;

   while ((*buf) && (c < 16)) {
      while ((*buf == ' ') || (*buf == '\t') || (*buf == '='))
	 buf++;

      if (*buf == '#')
	 return c; 

      if (*buf)
	 argv[c++] = buf;

      while ((*buf) && (*buf != ' ') && (*buf != '\t') && (*buf != '='))
	 buf++;

      if (*buf) {
	 *buf = 0;
	 buf++;
      }
   }

   return c;
}



/* parse_cfg_file:
 *  Reads the sound hardware configuration file (sound.cfg). See readme.txt
 *  for a description of the file format.
 */
static void parse_cfg_file(char *cfg_path)
{
   char buf[128];
   PACKFILE *f;
   char *argv[16];
   int argc;
   char *s;

   if (cfg_path) {
      strcpy(buf, cfg_path);
      strcpy(get_filename(buf), "sound.cfg");
   }
   else
      strcpy(buf, "sound.cfg");

   if (!file_exists(buf, FA_RDONLY | FA_ARCH, NULL)) {
      s = getenv("ALLEGRO");
      if (s) {
	 strcpy(buf, s);
	 put_backslash(buf);
	 strcat(buf, "sound.cfg");
      }
   }

   f = pack_fopen(buf, F_READ);
   if (!f)
      return;

   while (pack_fgets(buf, 128, f) != 0) {

      argc = parse_string(buf, argv);

      if (argc >= 2) {

	 if (stricmp(argv[0], "digi_card") == 0) {
	    if (digi_card == DIGI_AUTODETECT)
	       digi_card = atoi(argv[1]);
	 }
	 else if (stricmp(argv[0], "midi_card") == 0) {
	    if (midi_card == MIDI_AUTODETECT)
	       midi_card = atoi(argv[1]);
	 }
	 else if (stricmp(argv[0], "flip_pan") == 0) {
	    _flip_pan = atoi(argv[1]);
	 }
	 else if (stricmp(argv[0], "sb_port") == 0) {
	    _sb_port = strtol(argv[1], NULL, 16);
	 }
	 else if (stricmp(argv[0], "sb_dma") == 0) {
	    _sb_dma = atoi(argv[1]);
	 }
	 else if (stricmp(argv[0], "sb_irq") == 0) {
	    _sb_irq = atoi(argv[1]);
	 }
	 else if (stricmp(argv[0], "sb_freq") == 0) {
	    _sb_freq = atoi(argv[1]);
	 }
	 else if (stricmp(argv[0], "fm_port") == 0) {
	    _fm_port = strtol(argv[1], NULL, 16);
	 }
	 else if (stricmp(argv[0], "mpu_port") == 0) {
	    _mpu_port = strtol(argv[1], NULL, 16);
	 }
	 else if ((argc >= 5) && 
		  ((argv[0][0] == 'p') || (argv[0][0] == 'P')) &&
		  (argv[0][1] >= '0') && (argv[0][1] <= '9')) {
	    _midi_map_program(atoi(argv[0]+1), atoi(argv[1]), atoi(argv[2]), 
					       atoi(argv[3]), atoi(argv[4]));
	 }
      }
   }

   pack_fclose(f);
}



/* install_sound:
 *  Initialises the sound module, returning zero on success. The two card 
 *  parameters should use the DIGI_* and MIDI_* constants defined in 
 *  allegro.h. Pass DIGI_AUTODETECT and MIDI_AUTODETECT if you don't know 
 *  what the soundcard is. The cfg_path is the name of the directory to
 *  load sound.cfg from. Pass NULL to use the current directory, otherwise
 *  just passing argv[0] will work.
 */
int install_sound(int digi, int midi, char *cfg_path)
{
   int c;

   if (sound_installed)
      return 0;

   /* initialise the midi file player */
   if (_midi_init() != 0)
      return -1;

   register_datafile_object(DAT_SAMPLE, NULL, (void (*)(void *))destroy_sample);
   register_datafile_object(DAT_MIDI, NULL, (void (*)(void *))destroy_midi);

   digi_card = digi;
   midi_card = midi;

   _digi_volume = _midi_volume = -1;
   _flip_pan = FALSE;
   _fm_port = _mpu_port = _sb_port = _sb_dma = _sb_irq = -1;
   _sb_freq = 16129;

   parse_cfg_file(cfg_path);

   for (c=0; c<DIGI_VOICES; c++)
      digi_voice[c] = NULL;

   sound_lock_mem();
   _dma_lock_mem();

   digi_driver = NULL;

   /* search table for a specific digital driver */
   for (c=0; digi_driver_list[c].driver; c++) { 
      if (digi_driver_list[c].driver_id == digi_card) {
	 digi_driver = digi_driver_list[c].driver;
	 break;
      }
   }

   /* autodetect digital driver */
   if (!digi_driver) {
      for (c=0; digi_driver_list[c].driver; c++) {
	 if ((digi_driver_list[c].autodetect) &&
	     (digi_driver_list[c].driver->detect())) {
	    digi_card = digi_driver_list[c].driver_id;
	    digi_driver = digi_driver_list[c].driver;
	    break;
	 }
      }
   }

   midi_driver = NULL;

   /* search table for a specific MIDI driver */
   for (c=0; midi_driver_list[c].driver; c++) { 
      if (midi_driver_list[c].driver_id == midi_card) {
	 midi_driver = midi_driver_list[c].driver;
	 break;
      }
   }

   /* autodetect MIDI driver */
   if (!midi_driver) {
      for (c=0; midi_driver_list[c].driver; c++) {
	 if ((midi_driver_list[c].autodetect) &&
	     (midi_driver_list[c].driver->detect())) {
	    midi_card = midi_driver_list[c].driver_id;
	    midi_driver = midi_driver_list[c].driver;
	    break;
	 }
      }
   }

   /* initialise the digital sound driver */
   if (digi_driver->init() != 0) {
      digi_driver = &digi_none; 
      midi_driver = &midi_none; 
      _midi_exit();
      return -1;
   }

   /* initialise the midi driver */
   if (midi_driver->init() != 0) {
      digi_driver->exit();
      digi_driver = &digi_none; 
      midi_driver = &midi_none; 
      _midi_exit();
      return -1;
   }

   _add_exit_func(remove_sound);
   sound_installed = TRUE;
   return 0;
}



/* remove_sound:
 *  Sound module cleanup routine.
 */
void remove_sound()
{
   int c;

   if (sound_installed) {
      for (c=0; c<DIGI_VOICES; c++)
	 if (digi_voice[c])
	    digi_driver->stop(c);

      _midi_exit();

      midi_driver->exit();
      midi_driver = &midi_none; 

      digi_driver->exit();
      digi_driver = &digi_none; 

      _remove_exit_func(remove_sound);
      sound_installed = FALSE;
   }
}



/* set_volume:
 *  Alters the global sound output volume. Specify volumes for both digital
 *  samples and MIDI playback, as integers from 0 to 255. If possible this
 *  routine will use a hardware mixer to control the volume, otherwise it
 *  will tell the sample mixer and MIDI player to simulate a mixer in
 *  software.
 */
void set_volume(int digi_volume, int midi_volume)
{
   digi_volume = MID(0, digi_volume, 255);

   if ((digi_driver->mixer_volume) && 
       (digi_driver->mixer_volume(digi_volume) == 0))
      _digi_volume = -1;
   else
      _digi_volume = digi_volume;

   midi_volume = MID(0, midi_volume, 255);

   if ((midi_driver->mixer_volume) && 
       (midi_driver->mixer_volume(midi_volume) == 0))
      _midi_volume = -1;
   else
      _midi_volume = midi_volume;
}



/* lock_sample:
 *  Locks a SAMPLE struct into physical memory. Pretty important, since 
 *  they are mostly accessed inside interrupt handlers.
 */
void lock_sample(SAMPLE *spl)
{
   #ifdef DJGPP
      _go32_dpmi_lock_data(spl, sizeof(SAMPLE));
      _go32_dpmi_lock_data(spl->data, spl->len*spl->bits/8);
   #endif
}



/* load_sample:
 *  Loads a sample from disk.
 */
SAMPLE *load_sample(char *filename)
{
   if (stricmp(get_extension(filename), "wav") == 0)
      return load_wav(filename);
   else
      return NULL;
}



/* load_wav:
 *  Reads a mono RIFF WAV format sample file, returning a SAMPLE structure, 
 *  or NULL on error.
 */
SAMPLE *load_wav(char *filename)
{
   PACKFILE *f;
   char buffer[25];
   int i;
   int length, len;
   int freq = 22050;
   int bits = 8;
   signed short s;
   SAMPLE *spl = NULL;

   f = pack_fopen(filename, F_READ);
   if (!f)
      return NULL;

   pack_fread(buffer, 12, f);          /* check RIFF header */
   if (memcmp(buffer, "RIFF", 4) || memcmp(buffer+8, "WAVE", 4))
      goto getout;

   while (!pack_feof(f)) {
      if (pack_fread(buffer, 4, f) != 4)
	 break;

      length = pack_igetl(f);          /* read chunk length */

      if (memcmp(buffer, "fmt ", 4) == 0) {
	 i = pack_igetw(f);            /* should be 1 for PCM data */
	 length -= 2;
	 if (i != 1) 
	    goto getout;

	 i = pack_igetw(f);            /* should be 1 for mono data */
	 length -= 2;
	 if (i != 1)
	    goto getout;

	 freq = pack_igetl(f);         /* sample frequency */
	 length -= 4;

	 pack_igetl(f);                /* skip six bytes */
	 pack_igetw(f);
	 length -= 6;

	 bits = pack_igetw(f);         /* 8 or 16 bit data? */
	 length -= 2;
	 if ((bits != 8) && (bits != 16))
	    goto getout;
      }
      else if (memcmp(buffer, "data", 4) == 0) {
	 len = length;
	 if (bits == 16)
	    len /= 2;

	 spl = malloc(sizeof(SAMPLE)); 

	 if (spl) {                    /* initialise the sample struct */
	    spl->bits = 8;
	    spl->freq = freq;
	    spl->len = len;

	    spl->data = malloc(len);
	    if (!spl->data) {
	       free(spl);
	       spl = NULL;
	    }
	    else {                     /* read the actual sample data */
	       if (bits == 8) {
		  pack_fread(spl->data, len, f);
		  length -= len;
	       }
	       else {
		  for (i=0; i<spl->len; i++) {
		     s = pack_igetw(f);
		     length -= 2;
		     ((unsigned char *)spl->data)[i] = (s^0x8000) >> 8;
		  }
	       }
	       if (errno) {
		  free(spl->data);
		  free(spl);
		  spl = NULL;
	       }
	    }
	 }
      }

      while (length > 0) {             /* skip the remainder of the chunk */
	 if (pack_getc(f) == EOF)
	    break;

	 length--;
      }
   }

   getout:

   pack_fclose(f);

   if (spl)
      lock_sample(spl);

   return spl;
}



/* destroy_sample:
 *  Frees a SAMPLE struct, checking whether the sample is currently playing, 
 *  and stopping it if it is.
 */
void destroy_sample(SAMPLE *spl)
{
   if (spl) {
      stop_sample(spl);

      if (spl->data) {
	 _unlock_dpmi_data(spl->data, spl->len*spl->bits/8);
	 free(spl->data);
      }

      _unlock_dpmi_data(spl, sizeof(SAMPLE));
      free(spl);
   }
}



/* play_sample:
 *  Triggers a sample at the specified volume, pan position, and frequency.
 *  The volume and pan range from 0 (min/left) to 255 (max/right), although
 *  the resolution actually used by the playback routines is likely to be
 *  less than this. Frequency is relative rather than absolute: 1000 
 *  represents the frequency that the sample was recorded at, 2000 is 
 *  twice this, etc. If loop is true the sample will repeat until you call 
 *  stop_sample(), and can be manipulated while it is playing by calling
 *  adjust_sample().
 */
void play_sample(SAMPLE *spl, int vol, int pan, int freq, int loop)
{
   int c;
   int best = 0;
   unsigned long best_time = 0xFFFFFFFFL;
   unsigned long t;

   if (freq == 1000)
      freq = spl->freq;
   else
      freq = ((long)spl->freq * (long)freq) / 1000;

   vol = MID(0, vol, 255);
   pan = MID(0, pan, 255);

   /* find good sfx channel to use */ 
   for (c=0; c<digi_driver->voices; c++) { 
      if (!digi_voice[c]) {
	 best = c;
	 break;
      }
      t = digi_driver->voice_status(c);
      if (t < best_time) {
	 best = c;
	 best_time = t;
      }
   }

   if (digi_voice[best])
      digi_driver->stop(best);

   digi_driver->play(best, spl, vol, pan, freq, loop);
   digi_voice[best] = spl;
}

END_OF_FUNCTION(play_sample);



/* adjust_sample:
 *  Alters the parameters of a sample while it is playing, useful for
 *  manipulating looped sounds. You can alter the volume, pan, and
 *  frequency, and can also remove the looping flag, which will stop
 *  the sample when it next reaches the end of its loop. If there are
 *  several copies of the same sample playing, this will adjust the
 *  first one it comes across. If the sample is not playing it has no
 *  effect.
 */
void adjust_sample(SAMPLE *spl, int vol, int pan, int freq, int loop)
{
   int c;

   if (freq == 1000)
      freq = spl->freq;
   else
      freq = ((long)spl->freq * (long)freq) / 1000;

   vol = MID(0, vol, 255);
   pan = MID(0, pan, 255);

   for (c=0; c<digi_driver->voices; c++) { 
      if (digi_voice[c] == spl) {
	 digi_driver->adjust(c, vol, pan, freq, loop);
	 return;
      }
   }
}

END_OF_FUNCTION(adjust_sample);



/* stop_sample:
 *  Kills off a sample, which is required if you have set a sample going 
 *  in looped mode. If there are several copies of the sample playing,
 *  it will stop them all.
 */
void stop_sample(SAMPLE *spl)
{
   int c;

   for (c=0; c<DIGI_VOICES; c++) {
      if (digi_voice[c] == spl) {
	 digi_driver->stop(c);
	 digi_voice[c] = NULL;
      }
   }
}

END_OF_FUNCTION(stop_sample);



/* _mixer_init:
 *  Initialises the sample mixing code, returning 0 on success. You should
 *  pass it number of samples you want it to mix each time the refill
 *  buffer routine is called, the sample rate to mix at, and two flags 
 *  indicating whether the mixing should be done in stereo or mono and with 
 *  eight or sixteen bits. The bufsize parameter is the number of samples,
 *  not bytes. It should take into account whether you are working in stereo 
 *  or not (eg. double it if in stereo), but it should not be affected by
 *  whether each sample is 8 or 16 bits.
 */
int _mixer_init(int bufsize, int freq, int stereo, int is16bit)
{
   int i, j;
   int clip_size;
   int clip_scale;
   int clip_max;

   mix_size = bufsize;
   mix_freq = freq;
   mix_stereo = stereo;
   mix_16bit = is16bit;

   for (i=0; i<MIXER_MAX_SFX; i++)
      mixer_sample[i].data = NULL;

   /* temporary buffer for sample mixing */
   mix_buffer = malloc(mix_size*sizeof(short));
   if (!mix_buffer)
      return -1;

   _go32_dpmi_lock_data(mix_buffer, mix_size*sizeof(short));

   /* volume table for mixing samples into the temporary buffer */
   mix_vol_table = malloc(sizeof(MIXER_VOL_TABLE) * MIXER_VOLUME_LEVELS);
   if (!mix_vol_table) {
      free(mix_buffer);
      return -1;
   }

   _go32_dpmi_lock_data(mix_vol_table, sizeof(MIXER_VOL_TABLE) * MIXER_VOLUME_LEVELS);

   for (j=0; j<MIXER_VOLUME_LEVELS; j++)
      for (i=0; i<256; i++)
	 mix_vol_table[j][i] = (i-128) * j * 0x100 / MIXER_VOLUME_LEVELS / MIXER_MAX_SFX;

   /* lookup table for amplifying and clipping sample buffers */
   if (mix_16bit) {
      clip_size = 1 << MIX_RES_16;
      clip_scale = 18 - MIX_RES_16;
      clip_max = 0xFFFF;
   }
   else {
      clip_size = 1 << MIX_RES_8;
      clip_scale = 10 - MIX_RES_8;
      clip_max = 0xFF;
   }

   mix_clip_table = malloc(sizeof(short) * clip_size);
   if (!mix_clip_table) {
      free(mix_buffer);
      free(mix_vol_table);
      return -1;
   }

   _go32_dpmi_lock_data(mix_clip_table, sizeof(short) * clip_size);

   for (i=0; i < clip_size*3/8; i++) {
      mix_clip_table[i] = 0;
      mix_clip_table[clip_size-1-i] = clip_max;
   }

   for (i=0; i < clip_size/4; i++)
      mix_clip_table[clip_size*3/8 + i] = i<<clip_scale;

   return 0;
}



/* _mixer_exit:
 *  Cleans up the sample mixer code when you are done with it.
 */
void _mixer_exit()
{
   free(mix_buffer);
   mix_buffer = NULL;

   free(mix_vol_table);
   mix_vol_table = NULL;

   free(mix_clip_table);
   mix_clip_table = NULL;
}



/* mix_mono_samples:
 *  Mixes from a sample into a mono buffer, until either len samples have
 *  been mixed or until the end of the sample is reached.
 */
static void mix_mono_samples(MIXER_SAMPLE *spl, unsigned short *buf, int len)
{
   signed short *vol = (short *)(mix_vol_table + spl->lvol);

   while (len-- > 0) {
      *(buf++) += vol[spl->data[spl->pos>>MIXER_FIX_SHIFT]];
      spl->pos += spl->diff;
      if (spl->pos >= spl->len) {
	 if (spl->loop)
	    spl->pos -= spl->len;
	 else {
	    spl->data = NULL;
	    return;
	 }
      }
   }
}

static END_OF_FUNCTION(mix_mono_samples);



/* mix_stereo_samples:
 *  Mixes from a sample into a stereo buffer, until either len samples have
 *  been mixed or until the end of the sample is reached.
 */
static void mix_stereo_samples(MIXER_SAMPLE *spl, unsigned short *buf, int len)
{
   signed short *lvol = (short *)(mix_vol_table + spl->lvol);
   signed short *rvol = (short *)(mix_vol_table + spl->rvol);

   len >>= 1;

   while (len-- > 0) {
      *(buf++) += lvol[spl->data[spl->pos>>MIXER_FIX_SHIFT]];
      *(buf++) += rvol[spl->data[spl->pos>>MIXER_FIX_SHIFT]];
      spl->pos += spl->diff;
      if (spl->pos >= spl->len) {
	 if (spl->loop)
	    spl->pos -= spl->len;
	 else {
	    spl->data = NULL;
	    return;
	 }
      }
   }
}

static END_OF_FUNCTION(mix_stereo_samples);



/* _mix_some_samples:
 *  Mixes samples into a buffer in conventional memory (the buf parameter
 *  should be a linear offset into the specified segment), using the buffer 
 *  size, sample frequency, etc, set when you called _mixer_init(). This 
 *  should be called by the hardware end-of-buffer interrupt routine to 
 *  get the next buffer full of samples to DMA to the card.
 */
void _mix_some_samples(unsigned long buf, unsigned short seg)
{
   int i;
   unsigned short *p = mix_buffer;
   unsigned long *l = (unsigned long *)p;

   for (i=0; i<mix_size/2; i++)                 /* clear buffer */
      *(l++) = 0x80008000;

   for (i=0; i<MIXER_MAX_SFX; i++) {            /* mix samples */
      if (mixer_sample[i].data) {
	 if (mix_stereo) 
	    mix_stereo_samples(mixer_sample+i, p, mix_size);
	 else
	    mix_mono_samples(mixer_sample+i, p, mix_size);
      }
   }

   _farsetsel(seg);

   if (mix_16bit) {
      for (i=0; i<mix_size; i++) {
	 _farnspokew(buf, mix_clip_table[*p >> (16-MIX_RES_16)]);
	 buf += 2;
	 p++;
      }
   }
   else {
      for (i=0; i<mix_size; i++) {
	 _farnspokeb(buf, mix_clip_table[*p >> (16-MIX_RES_8)]);
	 buf++;
	 p++;
      }
   }
}

END_OF_FUNCTION(_mix_some_samples);



/* _mixer_play:
 *  Sample trigger routine for drivers that use the mixing routines: just 
 *  sets a few pointers to flag that the sample should be included next
 *  time we do some mixing.
 */
void _mixer_play(int voice, SAMPLE *spl, int vol, int pan, int freq, int loop)
{
   _mixer_adjust(voice, vol, pan, freq, loop);
   mixer_sample[voice].len = spl->len << MIXER_FIX_SHIFT;
   mixer_sample[voice].pos = 0;
   mixer_sample[voice].data = spl->data;
}

END_OF_FUNCTION(_mixer_play);



/* _mixer_adjust:
 *  Alters the parameters of an active sample.
 */
void _mixer_adjust(int voice, int vol, int pan, int freq, int loop)
{
   int lvol, rvol;

   /* convert pan position to L/R volumes */
   if (mix_stereo) {
      if (_flip_pan)
	 pan = 255-pan;

      lvol = vol * (255-pan) * MIXER_VOLUME_LEVELS / 32768;
      rvol = vol * pan * MIXER_VOLUME_LEVELS / 32768;

      if (lvol >= MIXER_VOLUME_LEVELS)
	 lvol = MIXER_VOLUME_LEVELS-1;

      if (rvol >= MIXER_VOLUME_LEVELS)
	 rvol = MIXER_VOLUME_LEVELS-1;
   }
   else
      lvol = rvol = vol * MIXER_VOLUME_LEVELS / 256;

   /* global volume adjustment? */
   if (_digi_volume >= 0) {
      lvol = (lvol * _digi_volume) / 256;
      rvol = (rvol * _digi_volume) / 256;
   }

   /* and set the new data */
   mixer_sample[voice].diff = ((long)freq<<MIXER_FIX_SHIFT) / mix_freq;
   mixer_sample[voice].lvol = lvol;
   mixer_sample[voice].rvol = rvol;
   mixer_sample[voice].loop = loop;
}

END_OF_FUNCTION(_mixer_adjust);



/* _mixer_stop:
 *  Stop a sample from playing (pretty easy to do :-)
 */
void _mixer_stop(int voice)
{
   mixer_sample[voice].data = NULL; 
}

END_OF_FUNCTION(_mixer_stop);



/* _mixer_voice_status:
 *  Returns a priority value for the specified voice, used by play_sample()
 *  to figure out which voice to cut off if sample polyphony is overloaded.
 */
unsigned long _mixer_voice_status(int voice)
{
   if (!mixer_sample[voice].data)
      return 0;

   if (mixer_sample[voice].loop) 
      return LONG_MAX - mixer_sample[voice].pos;

   return LONG_MAX/2 - mixer_sample[voice].pos;
}

END_OF_FUNCTION(_mixer_voice_status);



/* sound_lock_mem:
 *  Locks memory used by the functions in this file.
 */
static void sound_lock_mem()
{
   LOCK_VARIABLE(digi_none);
   LOCK_VARIABLE(midi_none);
   LOCK_VARIABLE(digi_card);
   LOCK_VARIABLE(midi_card);
   LOCK_VARIABLE(digi_driver);
   LOCK_VARIABLE(midi_driver);
   LOCK_VARIABLE(digi_voice);
   LOCK_VARIABLE(_digi_volume);
   LOCK_VARIABLE(_midi_volume);
   LOCK_VARIABLE(_flip_pan);
   LOCK_VARIABLE(mixer_sample);
   LOCK_VARIABLE(mix_buffer);
   LOCK_VARIABLE(mix_vol_table);
   LOCK_VARIABLE(mix_clip_table);
   LOCK_VARIABLE(mix_size);
   LOCK_VARIABLE(mix_freq);
   LOCK_VARIABLE(mix_stereo);
   LOCK_VARIABLE(mix_16bit);
   LOCK_FUNCTION(_dummy_detect);
   LOCK_FUNCTION(play_sample);
   LOCK_FUNCTION(adjust_sample);
   LOCK_FUNCTION(stop_sample);
   LOCK_FUNCTION(mix_mono_samples);
   LOCK_FUNCTION(mix_stereo_samples);
   LOCK_FUNCTION(_mix_some_samples);
   LOCK_FUNCTION(_mixer_play);
   LOCK_FUNCTION(_mixer_adjust);
   LOCK_FUNCTION(_mixer_stop);
   LOCK_FUNCTION(_mixer_voice_status);
}

