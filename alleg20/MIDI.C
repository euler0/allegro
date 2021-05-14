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
 *      The core MIDI file player.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "allegro.h"
#include "internal.h"


typedef struct MIDI_TRACK                       /* a track in the MIDI file */
{
   unsigned char *pos;                          /* position in track data */
   long timer;                                  /* time until next event */
   unsigned char running_status;                /* last MIDI event */
} MIDI_TRACK;


typedef struct MIDI_CHANNEL                     /* a MIDI channel */
{
   int patch;                                   /* current sound */
   int volume;                                  /* volume controller */
   int pan;                                     /* pan position */
   int pitch_bend;                              /* pitch bend position */
   int new_volume;                              /* cached volume change */
   int new_pitch_bend;                          /* cached pitch bend */
   int note[128];                               /* status of each note */
} MIDI_CHANNEL;


typedef struct MIDI_VOICE                       /* a voice on the soundcard */
{
   int channel;                                 /* MIDI channel */
   int note;                                    /* note (-1 = off) */
   int volume;                                  /* note velocity */
   long time;                                   /* when note was triggered */
} MIDI_VOICE;


typedef struct WAITING_NOTE                     /* a stored note-on request */
{
   int channel;
   int note;
   int volume;
} WAITING_NOTE;


typedef struct PATCH_TABLE                      /* GM -> external synth */
{
   int bank1;                                   /* controller #0 */
   int bank2;                                   /* controller #32 */
   int prog;                                    /* program change */
   int pitch;                                   /* pitch shift */
} PATCH_TABLE;


volatile long midi_pos = 0;                     /* position in MIDI file */
static long midi_pos_counter;                   /* delta for midi_pos */

volatile long _midi_tick = 0;                   /* counter for killing notes */

static void midi_player();                      /* core MIDI player routine */
static void prepare_to_play(MIDI *midi);
static void midi_lock_mem();

static MIDI *midifile = NULL;                   /* the file that is playing */

static int midi_loop = 0;                       /* repeat at eof? */

static int midi_timer_speed;                    /* midi_player's timer speed */
static int midi_pos_speed;                      /* MIDI delta -> midi_pos */
static int midi_speed;                          /* MIDI delta -> timer */
static int midi_new_speed;                      /* for tempo change events */

static MIDI_TRACK midi_track[MIDI_TRACKS];      /* the active tracks */
static MIDI_VOICE midi_voice[MIDI_VOICES];      /* synth voice status */
static MIDI_CHANNEL midi_channel[16];           /* MIDI channel info */
static WAITING_NOTE midi_waiting[MIDI_VOICES];  /* notes still to be played */
static PATCH_TABLE patch_table[128];            /* GM -> external synth */


#define sort_out_volume(c, vol)     ((vol * midi_channel[c].volume) / 128)



/* load_midi:
 *  Loads a standard MIDI file, returning a pointer to a MIDI structure,
 *  or NULL on error. 
 */
MIDI *load_midi(char *filename)
{
   int c;
   char buf[256];
   long data;
   PACKFILE *fp;
   MIDI *midi;
   int num_tracks;

   fp = pack_fopen(filename, F_READ);        /* open the file */
   if (!fp)
      return NULL;

   midi = malloc(sizeof(MIDI));              /* get some memory */
   if (!midi) {
      pack_fclose(fp);
      return NULL;
   }

   for (c=0; c<MIDI_TRACKS; c++) {
      midi->track[c].data = NULL;
      midi->track[c].len = 0;
   }

   pack_fread(buf, 4, fp);                   /* read midi header */
   if (memcmp(buf, "MThd", 4))
      goto err;

   pack_mgetl(fp);                           /* skip header chunk length */

   data = pack_mgetw(fp);                    /* MIDI file type */
   if ((data != 0) && (data != 1))
      goto err;

   num_tracks = pack_mgetw(fp);              /* number of tracks */
   if ((num_tracks < 1) || (num_tracks > MIDI_TRACKS))
      goto err;

   data = pack_mgetw(fp);                    /* beat divisions */
   midi->divisions = ABS(data);

   for (c=0; c<num_tracks; c++) {            /* read each track */
      pack_fread(buf, 4, fp);                /* read track header */
      if (memcmp(buf, "MTrk", 4))
	 goto err;

      data = pack_mgetl(fp);                 /* length of track chunk */
      midi->track[c].len = data;

      midi->track[c].data = malloc(data);    /* allocate memory */
      if (!midi->track[c].data)
	 goto err;
					     /* finally, read track data */
      if (pack_fread(midi->track[c].data, data, fp) != data)
	 goto err;
   }

   pack_fclose(fp);
   lock_midi(midi);
   return midi;

   /* oh dear... */
   err:
   pack_fclose(fp);
   destroy_midi(midi);
   return NULL;
}



/* destroy_midi:
 *  Frees the memory being used by a MIDI file.
 */
void destroy_midi(MIDI *midi)
{
   int c;

   if (midi == midifile)
      stop_midi();

   if (midi) {
      for (c=0; c<MIDI_TRACKS; c++)
	 if (midi->track[c].data)
	    free(midi->track[c].data);

      free(midi);
   }
}



/* lock_midi:
 *  Locks a MIDI file into physical memory. Pretty important, since they
 *  are only ever really accessed inside interrupt handlers.
 */
void lock_midi(MIDI *midi)
{
   int c;

   _go32_dpmi_lock_data(midi, sizeof(MIDI));

   for (c=0; c<MIDI_TRACKS; c++)
      if (midi->track[c].data)
	 _go32_dpmi_lock_data(midi->track[c].data, midi->track[c].len);
}



/* parse_var_len:
 *  The MIDI file format is a strange thing. Time offsets are only 32 bits,
 *  yet they are compressed in a weird variable length format. This routine 
 *  reads a variable length integer from a MIDI data stream. It returns the 
 *  number read, and alters the data pointer according to the number of
 *  bytes it used.
 */
static unsigned long parse_var_len(unsigned char **data)
{
   unsigned long val = **data & 0x7F;

   while (**data & 0x80) {
      (*data)++;
      val <<= 7;
      val += (**data & 0x7F);
   }

   (*data)++;
   return val;
}

static END_OF_FUNCTION(parse_var_len);



/* raw_program_change:
 *  Sends a program change message to a device capable of handling raw
 *  MIDI data, using patch mapping tables. Assumes that midi_driver->raw_midi
 *  isn't NULL, so check before calling it!
 */
static void raw_program_change(int channel, int patch)
{
   if (channel != 9) {
      if (patch_table[patch].bank1 >= 0) {
	 midi_driver->raw_midi(0xB0+channel);
	 midi_driver->raw_midi(0);
	 midi_driver->raw_midi(patch_table[patch].bank1);
      }

      if (patch_table[patch].bank2 >= 0) {
	 midi_driver->raw_midi(0xB0+channel);
	 midi_driver->raw_midi(32);
	 midi_driver->raw_midi(patch_table[patch].bank2);
      }

      midi_driver->raw_midi(0xC0+channel);
      midi_driver->raw_midi(patch_table[patch].prog);
   }
}

static END_OF_FUNCTION(raw_program_change);



/* midi_note_off:
 *  Processes a MIDI note-off event.
 */
static void midi_note_off(int channel, int note)
{
   int voice = midi_channel[channel].note[note];
   int c;

   /* can we send raw MIDI data? */
   if (midi_driver->raw_midi) {
      if (channel != 9)
	 note += patch_table[midi_channel[channel].patch].pitch;

      midi_driver->raw_midi(0x80+channel);
      midi_driver->raw_midi(note);
      midi_driver->raw_midi(0);
      return;
   }

   /* oh well, have to do it the long way... */
   if (voice >= 0) {
      midi_driver->key_off(voice);
      midi_voice[voice].note = -1;
      midi_channel[channel].note[note] = -1; 
   }
   else {
      for (c=0; c<MIDI_VOICES; c++) {
	 if ((midi_waiting[c].channel == channel) && 
	    (midi_waiting[c].note == note)) {
	    midi_waiting[c].note = -1;
	    break;
	 }
      }
   }
}

static END_OF_FUNCTION(midi_note_off);



/* sort_out_pitch_bend:
 *  MIDI pitch bend range is + or - two semitones. The low-level soundcard
 *  drivers can only handle bends up to +1 semitone. This routine converts
 *  pitch bends from MIDI format to our own format.
 */
static void sort_out_pitch_bend(int *bend, int *note)
{
   if (*bend == 0x2000) {
      *bend = 0;
      return;
   }

   (*bend) -= 0x2000;

   while (*bend < 0) {
      (*note)--;
      (*bend) += 0x1000;
   }

   while (*bend >= 0x1000) {
      (*note)++;
      (*bend) -= 0x1000;
   }
}

static END_OF_FUNCTION(sort_out_pitch_bend);



/* midi_note_on:
 *  Processes a MIDI note-on event. Tries to find a free soundcard voice,
 *  and if it can't either cuts off an existing note, or if 'polite' is
 *  set, just stores the channel, note and volume in the waiting list.
 */
static void midi_note_on(int channel, int note, int vol, int polite)
{
   int c;
   int voice = -1;
   int best_time = LONG_MAX;
   int inst, bend, corrected_note;
   int start_voice, end_voice;

   /* macro to avoid using the OPL drum voices for regular instruments */
   #define OK_VOICE(c)  ((!_fm_drum_mode) || (c < 6) || (c > 8))

   /* it's easy if the driver can handle raw MIDI data */
   if (midi_driver->raw_midi) {
      if (channel != 9)
	 note += patch_table[midi_channel[channel].patch].pitch;

      midi_driver->raw_midi(0x90+channel);
      midi_driver->raw_midi(note);
      midi_driver->raw_midi(vol);
      return;
   }

   /* if the note is already on, turn it off */
   if (midi_channel[channel].note[note] >= 0) {
      midi_note_off(channel, note);
      return;
   }

   /* evil hack: the OPL synth has special dedicated drum channels */
   if ((_fm_drum_mode) && (channel == 9)) {
      if (vol > 0)
	 midi_driver->key_on(-1, note+128, 60, 0, sort_out_volume(9, vol), 63);
      return;
   }

   /* another evil hack: SB Pro-I always pans voices 0-9 left and 9-18 right */
   if (midi_card == MIDI_2XOPL2) {
      if (midi_channel[channel].pan < 64) {
	 start_voice = 0;
	 end_voice = 6;
      }
      else {
	 start_voice = 9;
	 end_voice = 18;
      }
   }
   else {
      start_voice = 0;
      end_voice = midi_driver->voices;
   }

   /* find a free voice */
   for (c=start_voice; c<end_voice; c++) {
      if ((midi_voice[c].note < 0) && 
	  (midi_voice[c].time < best_time) && (OK_VOICE(c))) {
	 voice = c;
	 best_time = midi_voice[c].time;
      }
   }

   /* if there are no free voices... */
   if (voice < 0) {
      if (polite) {                    /* remember note for later */
	 for (c=0; c<MIDI_VOICES; c++) {
	    if (midi_waiting[c].note < 0) {
	       midi_waiting[c].channel = channel;
	       midi_waiting[c].note = note;
	       midi_waiting[c].volume = vol;
	       break;
	    }
	 }
	 return;
      }
      else {                           /* kill a note to make room */
	 voice = 0;
	 best_time = LONG_MAX;
	 for (c=start_voice; c<end_voice; c++) {
	    if ((midi_voice[c].time < best_time) && (OK_VOICE(c))) {
	       voice = c;
	       best_time = midi_voice[c].time;
	    }
	 }
	 midi_note_off(midi_voice[voice].channel, midi_voice[voice].note);
      }
   }

   /* drum sound? */
   if (channel == 9) {
      inst = 128+note;
      corrected_note = 60;
      bend = 0;
   }
   else {
      inst = midi_channel[channel].patch;
      corrected_note = note;
      bend = midi_channel[channel].pitch_bend;
      sort_out_pitch_bend(&bend, &corrected_note);
   }

   /* play the note */
   midi_driver->key_on(voice, inst, corrected_note, bend,
	       sort_out_volume(channel, vol), midi_channel[channel].pan);

   midi_voice[voice].channel = channel;
   midi_voice[voice].note = note;
   midi_voice[voice].volume = vol;
   midi_voice[voice].time = _midi_tick;
   midi_channel[channel].note[note] = voice; 
}

static END_OF_FUNCTION(midi_note_on);



/* all_notes_off:
 *  Turns off all active notes.
 */
static void all_notes_off(int channel)
{
   if (midi_driver->raw_midi) {
      midi_driver->raw_midi(0xB0+channel);
      midi_driver->raw_midi(123);
      midi_driver->raw_midi(0);
      return;
   }
   else {
      int note;

      for (note=0; note<128; note++)
	 if (midi_channel[channel].note[note] >= 0)
	    midi_note_off(channel, note);
   }
}

static END_OF_FUNCTION(all_notes_off);



/* reset_controllers:
 *  Resets volume, pan, pitch bend, etc, to default positions.
 */
static void reset_controllers(int channel)
{
   midi_channel[channel].new_volume = 128;
   midi_channel[channel].new_pitch_bend = 0x2000;

   switch (channel % 3) {
      case 0:  midi_channel[channel].pan = ((channel/3) & 1) ? 60 : 68; break;
      case 1:  midi_channel[channel].pan = 104; break;
      case 2:  midi_channel[channel].pan = 24; break;
   }

   if (midi_driver->raw_midi) {
      midi_driver->raw_midi(0xB0+channel);
      midi_driver->raw_midi(10);
      midi_driver->raw_midi(midi_channel[channel].pan);
   }
}

static END_OF_FUNCTION(reset_controllers);



/* update_controllers:
 *  Checks cached controller information and updates active voices.
 */
static void update_controllers()
{
   int c, c2;

   for (c=0; c<16; c++) {
      /* check for volume controller change */
      if (midi_channel[c].volume != midi_channel[c].new_volume) {
	 midi_channel[c].volume = midi_channel[c].new_volume;
	 if (midi_driver->raw_midi) {
	    midi_driver->raw_midi(0xB0+c);
	    midi_driver->raw_midi(7);
	    midi_driver->raw_midi(midi_channel[c].volume-1);
	 }
	 else {
	    for (c2=0; c2<MIDI_VOICES; c2++) {
	       if ((midi_voice[c2].channel == c) &&
		  (midi_voice[c2].note >= 0)) {
		  int vol = sort_out_volume(c, midi_voice[c2].volume);
		  midi_driver->set_volume(c2, vol);
	       }
	    }
	 }
      }

      /* check for pitch bend change */
      if (midi_channel[c].pitch_bend != midi_channel[c].new_pitch_bend) {
	 midi_channel[c].pitch_bend = midi_channel[c].new_pitch_bend;
	 if (midi_driver->raw_midi) {
	    midi_driver->raw_midi(0xE0+c);
	    midi_driver->raw_midi(midi_channel[c].pitch_bend & 0x7F);
	    midi_driver->raw_midi(midi_channel[c].pitch_bend >> 7);
	 }
	 else {
	    for (c2=0; c2<MIDI_VOICES; c2++) {
	       if ((midi_voice[c2].channel == c) &&
		  (midi_voice[c2].note >= 0)) {
		  int bend = midi_channel[c].pitch_bend;
		  int note = midi_voice[c2].note;
		  sort_out_pitch_bend(&bend, &note);
		  midi_driver->set_pitch(c2, note, bend);
	       }
	    }
	 }
      }
   }
}

static END_OF_FUNCTION(update_controllers);



/* process_controller:
 *  Deals with a MIDI controller message on the specified channel.
 */
static void process_controller(int channel, int ctrl, int data)
{
   switch (ctrl) {

      case 7:                                   /* main volume */
	 midi_channel[channel].new_volume = data+1;
	 break;

      case 10:                                  /* pan */
	 midi_channel[channel].pan = data;
	 if (midi_driver->raw_midi) {
	    midi_driver->raw_midi(0xB0+channel);
	    midi_driver->raw_midi(10);
	    midi_driver->raw_midi(data);
	 }
	 break;

      case 121:                                 /* reset all controllers */
	 reset_controllers(channel);
	 break;

      case 123:                                 /* all notes off */
      case 124:                                 /* omni mode off */
      case 125:                                 /* omni mode on */
      case 126:                                 /* poly mode off */
      case 127:                                 /* poly mode on */
	 all_notes_off(channel);
	 break;
   }
}

static END_OF_FUNCTION(process_controller);



/* process_meta_event:
 *  Processes the next meta-event on the specified track.
 */
static void process_meta_event(int track)
{
   unsigned char metatype = *(midi_track[track].pos++);
   long length = parse_var_len(&midi_track[track].pos);
   long tempo;

   if (metatype == 0x2F) {                      /* end of track */
      midi_track[track].pos = NULL;
      midi_track[track].timer = LONG_MAX;
      return;
   }

   if (metatype == 0x51) {                      /* tempo change */
      tempo = midi_track[track].pos[0] * 0x10000L +
	       midi_track[track].pos[1] * 0x100 + midi_track[track].pos[2];
      midi_new_speed = (tempo/1000) * (TIMERS_PER_SECOND/1000);
      midi_new_speed /= midifile->divisions;
   }

   midi_track[track].pos += length;
}

static END_OF_FUNCTION(process_meta_event);



/* process_midi_event:
 *  Processes the next MIDI event on the specified track.
 */
static void process_midi_event(int track)
{
   unsigned char byte1, byte2; 
   int channel;
   unsigned char event;
   long l;

   event = *(midi_track[track].pos++); 

   if (event & 0x80) {                          /* regular message */
      /* no running status for sysex and meta-events! */
      if ((event != 0xF0) && (event != 0xF7) && (event != 0xFF))
	 midi_track[track].running_status = event;
      byte1 = midi_track[track].pos[0];
      byte2 = midi_track[track].pos[1];
   }
   else {                                       /* use running status */
      byte1 = event; 
      byte2 = midi_track[track].pos[0];
      event = midi_track[track].running_status; 
      midi_track[track].pos--;
   }

   channel = event & 0x0F;

   switch (event>>4) {

      case 0x08:                                /* note off */
	 midi_note_off(channel, byte1);
	 midi_track[track].pos += 2;
	 break;

      case 0x09:                                /* note on */
	 midi_note_on(channel, byte1, byte2, 1);
	 midi_track[track].pos += 2;
	 break;

      case 0x0A:                                /* note aftertouch */
	 midi_track[track].pos += 2;
	 break;

      case 0x0B:                                /* control change */
	 process_controller(channel, byte1, byte2);
	 midi_track[track].pos += 2;
	 break;

      case 0x0C:                                /* program change */
	 midi_channel[channel].patch = byte1;
	 if (midi_driver->raw_midi)
	    raw_program_change(channel, byte1);
	 midi_track[track].pos += 1;
	 break;

      case 0x0D:                                /* channel aftertouch */
	 midi_track[track].pos += 1;
	 break;

      case 0x0E:                                /* pitch bend */
	 midi_channel[channel].new_pitch_bend = byte1 + (byte2<<7);
	 midi_track[track].pos += 2;
	 break;

      case 0x0F:                                /* special event */
	 switch (event) {
	    case 0xF0:                          /* sysex */
	    case 0xF7: 
	       l = parse_var_len(&midi_track[track].pos);
	       midi_track[track].pos += l;
	       break;

	    case 0xF2:                          /* song position */
	       midi_track[track].pos += 2;
	       break;

	    case 0xF3:                          /* song select */
	       midi_track[track].pos++;
	       break;

	    case 0xFF:                          /* meta-event */
	       process_meta_event(track);
	       break;

	    default:
	       /* the other special events don't have any data bytes,
		  so we don't need to bother skipping past them */
	       break;
	 }
	 break;

      default:
	 /* something has gone badly wrong if we ever get to here */
	 break;
   }
}

static END_OF_FUNCTION(process_midi_event);



/* midi_player:
 *  The core MIDI player: to be used as a timer callback.
 */
static void midi_player()
{
   int c;
   long l;
   int active;

   _midi_tick++;

   do_it_all_again:

   for (c=0; c<MIDI_VOICES; c++)
      midi_waiting[c].note = -1;

   /* deal with each track in turn... */
   for (c=0; c<MIDI_TRACKS; c++) { 
      if (midi_track[c].pos) {
	 midi_track[c].timer -= midi_timer_speed;

	 while (midi_track[c].timer <= 0) {     /* while events waiting */
	    process_midi_event(c);              /* process them */

	    if (midi_track[c].pos) {            /* read next time offset */
	       l = parse_var_len(&midi_track[c].pos);
	       l *= midi_speed;
	       midi_track[c].timer += l;
	    }
	 }
      }
   }

   /* update global position value */
   midi_pos_counter -= midi_timer_speed;
   while (midi_pos_counter <= 0) {
      midi_pos_counter += midi_pos_speed;
      midi_pos++;
   }

   /* tempo change? */
   if (midi_new_speed > 0) {
      for (c=0; c<MIDI_TRACKS; c++) {
	 if (midi_track[c].pos) {
	    midi_track[c].timer /= midi_speed;
	    midi_track[c].timer *= midi_new_speed;
	 }
      }
      midi_pos_counter /= midi_speed;
      midi_pos_counter *= midi_new_speed;

      midi_speed = midi_new_speed;
      midi_pos_speed = midi_new_speed * midifile->divisions;
      midi_new_speed = -1;
   }

   /* figure out how long until we need to be called again */
   active = 0;
   midi_timer_speed = LONG_MAX;
   for (c=0; c<MIDI_TRACKS; c++) {
      if (midi_track[c].pos) {
	 active = 1;
	 if (midi_track[c].timer < midi_timer_speed)
	    midi_timer_speed = midi_track[c].timer;
      }
   }

   /* end of the music? */
   if (!active) {
      if (midi_loop) {
	 for (c=0; c<16; c++)
	    all_notes_off(c);
	 prepare_to_play(midifile);
	 goto do_it_all_again;
      }
      else {
	 stop_midi(); 
	 return;
      }
   }

   /* reprogram the timer */
   if (midi_timer_speed < BPS_TO_TIMER(40))
      midi_timer_speed = BPS_TO_TIMER(40);

   install_int_ex(midi_player, midi_timer_speed);

   /* controller changes are cached and only processed here, so we can 
      condense streams of controller data into just a few voice updates */ 
   update_controllers();

   /* and deal with any notes that are still waiting to be played */
   for (c=0; c<MIDI_VOICES; c++)
      if (midi_waiting[c].note >= 0)
	 midi_note_on(midi_waiting[c].channel, midi_waiting[c].note,
		   midi_waiting[c].volume, 0);
}

static END_OF_FUNCTION(midi_player);



/* _midi_init:
 *  Sets up the MIDI player ready for use. Returns non-zero on failure.
 */
int _midi_init()
{
   int c, c2;

   midi_lock_mem();

   for (c=0; c<16; c++) {
      midi_channel[c].volume = 128;
      midi_channel[c].pitch_bend = 0x2000;
      for (c2=0; c2<128; c2++)
	 midi_channel[c].note[c2] = -1;
   }

   for (c=0; c<MIDI_VOICES; c++) {
      midi_voice[c].note = -1;
      midi_voice[c].time = 0;
   }

   for (c=0; c<128; c++) {
      patch_table[c].bank1 = -1;
      patch_table[c].bank2 = -1;
      patch_table[c].prog = c;
      patch_table[c].pitch = 0;
   }

   return 0;
}



/* _midi_exit:
 *  Turns off all active notes and removes the timer handler.
 */
void _midi_exit()
{
   stop_midi();
   remove_int(midi_player);
}



/* _midi_map_program:
 *  Sets up the table used for mapping GM program changes so they can work
 *  on non-GM synths (like my Yamaga TG500: this is a real piece of self
 *  indulgence).
 */
void _midi_map_program(int program, int bank1, int bank2, int prog, int pitch)
{
   if ((program < 1) || (program > 128))
      return;

   patch_table[program-1].bank1 = bank1;
   patch_table[program-1].bank2 = bank2;
   patch_table[program-1].prog = prog;
   patch_table[program-1].pitch = pitch;
}



/* load_patches:
 *  Scans through a MIDI file and identifies which patches it uses, passing
 *  them to the soundcard driver so it can load whatever samples are
 *  neccessary.
 */
static int load_patches(MIDI *midi)
{
   char patches[128], drums[128];
   unsigned char *p, *end;
   unsigned char running_status, event;
   long l;
   int c;

   for (c=0; c<128; c++)                        /* initialise to unused */
      patches[c] = drums[c] = FALSE;

   patches[0] = TRUE;                           /* always load the piano */

   for (c=0; c<MIDI_TRACKS; c++) {              /* for each track... */
      p = midi->track[c].data;
      end = p + midi->track[c].len;
      running_status = 0;

      while (p < end) {                         /* work through data stream */
	 event = *p; 
	 if (event & 0x80) {                    /* regular message */
	    p++;
	    if ((event != 0xF0) && (event != 0xF7) && (event != 0xFF))
	       running_status = event;
	 }
	 else                                   /* use running status */
	    event = running_status; 

	 switch (event>>4) {

	    case 0x0C:                          /* program change! */
	       patches[*p] = TRUE;
	       p++;
	       break;

	    case 0x09:                          /* note on, is it a drum? */
	       if ((event & 0x0F) == 9)
		  drums[*p] = TRUE;
	       p += 2;
	       break;

	    case 0x08:                          /* note off */
	    case 0x0A:                          /* note aftertouch */
	    case 0x0B:                          /* control change */
	    case 0x0E:                          /* pitch bend */
	       p += 2;
	       break;

	    case 0x0D:                          /* channel aftertouch */
	       p += 1;
	       break;

	    case 0x0F:                          /* special event */
	       switch (event) {
		  case 0xF0:                    /* sysex */
		  case 0xF7: 
		     l = parse_var_len(&p);
		     p += l;
		     break;

		  case 0xF2:                    /* song position */
		     p += 2;
		     break;

		  case 0xF3:                    /* song select */
		     p++;
		     break;

		  case 0xFF:                    /* meta-event */
		     p++;
		     l = parse_var_len(&p);
		     p += l;
		     break;

		  default:
		     /* the other special events don't have any data bytes,
			so we don't need to bother skipping past them */
		     break;
	       }
	       break;

	    default:
	       /* something has gone badly wrong if we ever get to here */
	       break;
	 }

	 if (p < end)                           /* skip time offset */
	    parse_var_len(&p);
      }
   }

   /* tell the driver to do its stuff */ 
   return midi_driver->load_patches(patches, drums);
}



/* prepare_to_play:
 *  Sets up all the global variables needed to play the specified file.
 */
static void prepare_to_play(MIDI *midi)
{
   int c;

   for (c=0; c<16; c++)
      reset_controllers(c);

   update_controllers();

   midifile = midi;
   midi_pos = 0;
   midi_pos_counter = 0;
   midi_speed = TIMERS_PER_SECOND / 2 / midifile->divisions;   /* 120 bpm */
   midi_new_speed = -1;
   midi_pos_speed = midi_speed * midifile->divisions;
   midi_timer_speed = 0;

   for (c=0; c<16; c++) {
      midi_channel[c].patch = 0;
      if (midi_driver->raw_midi)
	 raw_program_change(c, 0);
   }

   for (c=0; c<MIDI_TRACKS; c++) {
      if (midi->track[c].data) {
	 midi_track[c].pos = midi->track[c].data;
	 midi_track[c].timer = parse_var_len(&midi_track[c].pos);
	 midi_track[c].timer *= midi_speed;
      }
      else {
	 midi_track[c].pos = NULL;
	 midi_track[c].timer = LONG_MAX;
      }
      midi_track[c].running_status = 0;
   }
}

static END_OF_FUNCTION(prepare_to_play);



/* play_midi:
 *  Starts playing the specified MIDI file. If loop is set, the MIDI file 
 *  will be repeated until replaced with something else, otherwise it will 
 *  stop at the end of the file. Passing a NULL MIDI file will stop whatever 
 *  music is currently playing: allegro.h defines the macro stop_midi() to 
 *  be play_midi(NULL, FALSE); Returns non-zero if an error occurs (this
 *  may happen if a patch-caching wavetable driver is unable to load the
 *  required samples).
 */
int play_midi(MIDI *midi, int loop)
{
   int c;

   remove_int(midi_player);

   for (c=0; c<16; c++)
      all_notes_off(c);

   if (midi) {
      if (load_patches(midi) != 0)
	 return -1;

      midi_loop = loop;
      prepare_to_play(midi);

      /* arbitrary speed, midi_player() will adjust it */
      install_int(midi_player, 20);
   }
   else
      midifile = NULL;

   return 0;
}

END_OF_FUNCTION(play_midi);



/* stop_midi:
 *  Stops whatever MIDI file is currently playing.
 */
void stop_midi()
{
   play_midi(NULL, FALSE);
}

END_OF_FUNCTION(stop_midi);



/* midi_lock_mem:
 *  Locks all the memory that the midi player touches inside the timer
 *  interrupt handler (which is most of it).
 */
static void midi_lock_mem()
{
   LOCK_VARIABLE(midi_pos);
   LOCK_VARIABLE(midi_pos_counter);
   LOCK_VARIABLE(_midi_tick);
   LOCK_VARIABLE(midifile);
   LOCK_VARIABLE(midi_loop);
   LOCK_VARIABLE(midi_timer_speed);
   LOCK_VARIABLE(midi_pos_speed);
   LOCK_VARIABLE(midi_speed);
   LOCK_VARIABLE(midi_new_speed);
   LOCK_VARIABLE(midi_track);
   LOCK_VARIABLE(midi_voice);
   LOCK_VARIABLE(midi_channel);
   LOCK_VARIABLE(midi_waiting);
   LOCK_VARIABLE(patch_table);
   LOCK_FUNCTION(parse_var_len);
   LOCK_FUNCTION(raw_program_change);
   LOCK_FUNCTION(midi_note_off);
   LOCK_FUNCTION(sort_out_pitch_bend);
   LOCK_FUNCTION(midi_note_on);
   LOCK_FUNCTION(all_notes_off);
   LOCK_FUNCTION(reset_controllers);
   LOCK_FUNCTION(update_controllers);
   LOCK_FUNCTION(process_controller);
   LOCK_FUNCTION(process_meta_event);
   LOCK_FUNCTION(process_midi_event);
   LOCK_FUNCTION(midi_player);
   LOCK_FUNCTION(prepare_to_play);
   LOCK_FUNCTION(play_midi);
   LOCK_FUNCTION(stop_midi);
}

