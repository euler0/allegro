#include <iostream.h>
#include <string.h>
#include <fstream.h>
#include <stdio.h>
#include <dir.h>
#include "allegro.h"
#include "internal.h"
#include "testsnd.h"


#define PROGRAM_NAME    "Allegro"   //Name of the program this will be used with
#define FLIP_POSITION   17          //Do not change unless main_dlg is modified
#define NUM_DIGI_CARDS  7           //Number of digital cards to choose from
#define NUM_MIDI_CARDS  7           //Number of midi cards to choose from

//Structures to store settings for cards
typedef struct DIGISPECS
{
    char    name[17];
    short   maxfreq;                //Index in FREQ struct which is max freq
				    //Card can handle
} DIGISPECS;

typedef struct MIDICARD
{
    char    *name;
    bool    usefm;                  //Does the card use an fm port?

} MIDICARD;

typedef struct FREQ
{
    char    frequency[16];

} FREQ;


//Function prototypes
void WriteSoundCFG(void);
void CopyFile(char *s, char *d);
int  QuitProc(int, DIALOG*, int);
int  AutoDetProc(int, DIALOG*, int);
int  ChangeDigiCard(int, DIALOG*, int);
int  TestDigiCard(int, DIALOG*, int);
void ReadSettings(void);
char *DigiList(int, int*);
int  DigiListProc(int, DIALOG*, int);
int  SaveQuitProc(int, DIALOG*, int);
int  ChangeMidiCard(int, DIALOG*, int);
int  MidiListProc(int, DIALOG*, int);
char *MidiList(int, int*);
int ChangeFreq(int, DIALOG*, int);
char *FreqList(int, int*);
int FreqListProc(int, DIALOG*, int);

//List of digital soundcards supported
static DIGISPECS digi[]=
{
    {"None", 0},
    {"SB (autodetect)", 1},
    {"SoundBlaster 1.0", 1},
    {"SoundBlaster 1.5", 1},
    {"SoundBlaster 2.0", 3},
    {"SoundBlaster Pro", 2},
    {"SoundBlaster 16",  3},
};



//List of midi cards supported
static MIDICARD midi[]=
{
    {"None", 1},
    {"OPL (autodetect)", 1},
    {"OPL2", 1},
    {"Dual OPL2", 1},
    {"OPL3", 1},
    {"SB Midi", 1},
    {"MPU-401", 0},
};


static FREQ freqs[]=
{
    {"11906 (386)"},
    {"16129 (486)"},
    {"22727 (486-DX2)"},
    {"45454 (Pentium)"},
};


//Global Variables for current settings
char digi_name[30] = "None";
char midi_name[30] = "None";
char *base_address  = "220";
char *irq = "7";
char *dma = "1";
char freq[16] = "0";
int digi_allegro_num = 0;
int midi_allegro_num = 0;
char *midi_port = "388";
bool usefm = 1;
char *flip_stereo = "0";
bool sound_installed = false;

//Dialog for main menu
DIALOG main_dlg[]=
{
    /* (dialog proc)    (x) (y) (w) (h) (fg)(bg)(key)   (flags) (d1)(d2)(dp) */
    { d_clear_proc,     0,  0,  0,  0,  255,1,  0,      0,      0,  0,  NULL },
    { d_box_proc,       40, 40, 160,220,255,8,  0,      0,      0,  0,  NULL},
    { d_box_proc,       440,40, 160,220,255,8,  0,      0,      0,  0,  NULL},
    { d_ctext_proc,     320,4,  1,  1,  255,1,  0,      0,      0,  0,  PROGRAM_NAME " Sound Setup" },
    { d_ctext_proc,     320,15, 1,  1,  255,1,  0,      0,      0,  0,  "by David Calvin" },
    { d_text_proc,      55, 25, 1,  1,  255,1,  0,      0,      0,  0,  "Digital Settings"},
    { d_text_proc,      470,25, 1,  1,  255,1,  0,      0,      0,  0,  "Midi Settings" },
    { QuitProc,         240,440,160,25, 255,5,  27,     D_EXIT, 0,  0,  "Exit Without Saving" },
    { AutoDetProc,      240,40, 160,25, 255,5,  'a',    D_EXIT, 0,  0,  "Autodetect Settings" },
    { d_text_proc,      50, 60, 160,15, 255,8,  0,      0,      0,  0,  "Card Name:" },
    { ChangeDigiCard,   50, 70, 140,25, 255,5,  0,      D_EXIT, 0,  0,  digi_name },
    { d_text_proc,      50, 100,160,15, 255,8,  0,      0,      0,  0,  "Base Address:"},
    { d_edit_proc,      50, 110,120,15, 255,8,  0,      0,      3,  0,  base_address},
    { d_text_proc,      50, 130,120,15, 255,8,  0,      0,      0,  0,  "IRQ:" },
    { d_edit_proc,      50, 140,120,15, 255,8,  0,      0,      1,  0,  irq},
    { d_text_proc,      50, 160,120,15, 255,8,  0,      0,      0,  0,  "DMA:" },
    { d_edit_proc,      50, 170,120,15, 255,8,  0,      0,      1,  0,  dma},
    { d_check_proc,     50, 190,140,12, 255,8,  0,      0,      0,  0,  "Flip Stereo:" },
    { TestDigiCard,     240,360,160,25, 255,5,  0,      D_EXIT, 0,  0,  "Test Settings" },
    { SaveQuitProc,     240,400,160,25, 255,5,  0,      D_EXIT, 0,  0,  "Exit and Save"},
    { d_text_proc,      450,60, 160,15, 255,8,  0,      0,      0,  0,  "Card Name:"},
    { ChangeMidiCard,   450,70, 140,25, 255,5,  0,      D_EXIT, 0,  0,  midi_name },
    { d_text_proc,      450,100,160,15, 255,8,  0,      0,      0,  0,  "Port:"},
    { d_edit_proc,      450,110,120,15, 255,8,  0,      0,      3,  0,  midi_port},
    { d_text_proc,      50, 215,120,15, 255,8,  0,      0,      0,  0,  "Mixing Frequency:"},
    { ChangeFreq,       50, 230,140,25, 255,5,  0,      D_EXIT, 0,  0,  freq},
    { NULL }
};

//Dialog to choose digital card manually 
DIALOG digi_list[] =
{
    { DigiListProc,     240,100,160,10*NUM_DIGI_CARDS,  255,    5,  0,  D_EXIT, 0,  0,  DigiList},
    { NULL },
}; 

//Dialog to choose midi card manually
DIALOG midi_list[] = 
{
    { MidiListProc,     240,100,160,10*NUM_MIDI_CARDS,  255,    5,  0,  D_EXIT, 0,  0,  MidiList},
    { NULL }
};

DIALOG freq_list[] =
{
    { FreqListProc, 240,100,160,50, 255, 5, 0,  D_EXIT, 0,  0,  FreqList},
    {NULL}
};



int main(int argc, char *argv[])
{
    bool autodetect = true;

    //Process arguments to main
    if(argc > 1)
    {
	//Check for valid commandline options, otherwise print help message
	if(strcmp("-nodetect", argv[1]) == 0)
	    autodetect = false; 
	else
	{
	    cout << "setup [options]" << endl << endl;
	    cout << "Valid options are:" << endl;
	    cout << "       -nodetect       Bypasses automatic soundcard detection" <<
		    " on startup" << endl;
	    exit(1);
	}
    }

    //setup Allegro
    allegro_init();
    install_mouse();
    install_keyboard();
    install_timer();

    if (set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0) != 0) {
      allegro_exit();
      cout << "Error setting graphics mode" << endl << allegro_error << endl;
      exit(1);
    }

    set_pallete(setup_pallete);
    gui_bg_color = 2;
    gui_fg_color = 0;

    //If user opted to skip autodetection, or autodetection fails, 
    //or sound.cfg not found, initialize variables to zero.
    if( (file_exists("sound.cfg", FA_RDONLY | FA_ARCH, NULL)) && autodetect)
    {
	if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) != 0)
	    autodetect = false;
	//Otherwise, read settings
	else
	{
	    ReadSettings();
	    //Make backup of sound.cfg in case user doesn't save changes
	    CopyFile("sound.cfg", "sound.bak");
	    sound_installed=true;
	}
    }
    else    autodetect = false;

    //start main dialog
    do_dialog(main_dlg, -1);

    exit(0);
}


//Function to copy sound.cfg, for making backups
void CopyFile(char *s, char *d)
{
    FILE *in = fopen(s, "rb");
    FILE *out = fopen(d, "wb");
    int c;

    while ((c = fgetc(in)) != EOF)
	fputc(c, out);

    fclose(in);
    fclose(out);
}


//Function to create sound.cfg from global variables.
void WriteSoundCFG(void)
{
    delete_file("sound.cfg");
    ofstream output("sound.cfg");

    output << "digi_card = " << digi_allegro_num << endl;
    output << "midi_card = " << midi_allegro_num << endl;
    output << "flip_pan = ";
    if(main_dlg[FLIP_POSITION].flags == D_SELECTED)
	output << 1 << endl;
    else 
	output << 0 << endl;
    output << "sb_port = ";
    if(midi_allegro_num == 5)           //If SB MIDI used, write address
		output << midi_port;    //to sb_port instead of fm_port
    else        output << base_address;
    output << endl;
    output << "sb_dma = " << dma << endl;
    output << "sb_irq = " << irq << endl;
    output << "sb_freq = " << freq << endl;
    output << "fm_port = ";
    if(usefm)
	output << midi_port;
    output << endl;
    output << "mpu_port = ";
    if(!usefm)
	output << midi_port;
    output << endl;
    output.close();
}


//Exit without saving
int QuitProc(int msg, DIALOG *d, int c)
{
    int ret = d_button_proc(msg, d, c);

    if(ret == D_CLOSE)
    {
	//Copy setup.bak to setup.cfg to overwrite any changes
	delete_file("sound.cfg");
	if(file_exists("sound.bak", FA_RDONLY | FA_ARCH, NULL))
	{
	    CopyFile("sound.bak", "sound.cfg");
	    delete_file("sound.bak");
	}
	return D_CLOSE;
    }

    return D_O_K;
}

//Attempt autodetection
int AutoDetProc(int msg, DIALOG *d, int c)
{
    int ret = d_button_proc(msg, d, c);

    if(ret == D_CLOSE)
    {
	if(sound_installed)
	    remove_sound();
	delete_file("sound.cfg");
	if(install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) != 0)
	{ 
	    alert("Cannot autodetect!", allegro_error, NULL, "Ok", NULL, 0, 0);
	    return(D_REDRAW);
	}
	ReadSettings();
	sound_installed = true;
	return(D_REDRAW);
    }
    return D_O_K;
}

//Change digital card
int ChangeDigiCard(int msg, DIALOG *d, int c)
{
    int ret = d_button_proc(msg, d, c);
    if(ret == D_CLOSE)
    {
	popup_dialog(digi_list, -1);
	strcpy(digi_name, digi[digi_list[0].d1].name);
	digi_allegro_num = digi_list[0].d1;
	//Copy frequency one char at a time to eliminate comment at end
	for(int i=0; i<5; i++)
	    freq[i] = freqs[digi[digi_list[0].d1].maxfreq].frequency[i];
	freq[6] = '\0';
	return(D_REDRAW);
    }
    return D_O_K;
}


//Test all soundcard settings
int TestDigiCard(int msg, DIALOG *d, int c)
{
    int ret = d_button_proc(msg, d, c);
    if(ret == D_CLOSE)
    {
	if(sound_installed)
	    remove_sound();
	WriteSoundCFG();
	if(install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, NULL) != 0)
	{
	    alert("Sound Initialization Failed!", allegro_error, NULL, "Ok", NULL, 0, 0);
	    sound_installed = false;
	    return(D_REDRAW);
	}
	else    sound_installed=true;

	ReadSettings();
	play_sample(&test_sample, 255, 0, 1000, 1);
	alert("Playing left channel", NULL, NULL, "Ok", NULL, 0, 0);
	stop_sample(&test_sample);
	play_sample(&test_sample, 255, 255, 1000, 1);
	alert("Playing right channel", NULL, NULL, "Ok", NULL, 0, 0);
	stop_sample(&test_sample);
	play_sample(&test_sample, 255, 128, 1000, 1);
	alert("Playing center channel", NULL, NULL, "Ok", NULL, 0, 0);
	stop_sample(&test_sample);
	play_midi(&test_midi, 1);
	alert("Playing Midi", NULL, NULL, "Ok", NULL, 0, 0);
	stop_midi(); 

	return(D_REDRAW);
    }
    return D_O_K;
}


//Set global variables from allegro variables
void ReadSettings(void)
{
    digi_allegro_num = digi_card;
    midi_allegro_num = midi_card;
    sprintf(flip_stereo, "%d", _flip_pan);
    usefm = midi[midi_allegro_num].usefm;
    if(usefm)
	sprintf(midi_port,"%x", _fm_port);
    else
	sprintf(midi_port, "%x", _mpu_port);
    sprintf(freq, "%d", _sb_freq);
    sprintf(base_address, "%x", _sb_port);
    sprintf(dma, "%d", _sb_dma);
    sprintf(irq, "%d", _sb_irq);
    strcpy(digi_name, digi[digi_allegro_num].name);
    strcpy(midi_name, midi[midi_allegro_num].name);

    if(_flip_pan)
	main_dlg[FLIP_POSITION].flags = D_SELECTED;

}


//List to choose new digital card
int DigiListProc(int msg, DIALOG *d, int c)
{
    d_list_proc(msg, d, c);
    if(msg == MSG_DCLICK)
    {
	return(D_EXIT);
    }
    return D_O_K;
}

char *DigiList(int index, int *list_size)
{
    if(index < 0)
    {
	*list_size = NUM_DIGI_CARDS;
	return NULL;
    }
    return(digi[index].name);

}


//Save and exit
int SaveQuitProc(int msg, DIALOG *d, int c)
{
    int ret = d_button_proc(msg, d, c);

    if(ret == D_CLOSE)
    {
	WriteSoundCFG();
	delete_file("sound.bak");
	return(D_EXIT);
    }
    return D_O_K;
}


//Change midi card
int ChangeMidiCard(int msg, DIALOG *d, int c)
{
    int ret = d_button_proc(msg, d, c);
    if(ret == D_CLOSE)
    {
	popup_dialog(midi_list, -1);
	strcpy(midi_name, midi[midi_list[0].d1].name);
	midi_allegro_num = midi_list[0].d1;
	usefm = midi[midi_list[0].d1].usefm;
	return(D_REDRAW);
    }
    return D_O_K;
}


//List to choose new midi card
char *MidiList(int index, int *list_size)
{
    if(index < 0)
    {
	*list_size = NUM_MIDI_CARDS;
	return NULL;
    }
    return(midi[index].name);

}

int MidiListProc(int msg, DIALOG *d, int c)
{
    d_list_proc(msg, d, c);
    if(msg == MSG_DCLICK)
    {
	return(D_EXIT);
    }
    return D_O_K;
}

int ChangeFreq(int msg, DIALOG* d, int c)
{
    int list_size;
    int ret = d_button_proc(msg, d, c);
    if(ret == D_CLOSE)
    {
	FreqList(-1, &list_size);
	if(list_size == 1)
	{
	    alert("Frequency cannot be changed", "for this card!",
					     NULL, "OK", 0, 0, 0);
	    return D_REDRAW;
	}
	popup_dialog(freq_list, -1);
	//Copy frequency one char at a time to eliminate comment at end
	for(int i=0; i<5; i++)
	    freq[i] = freqs[freq_list[0].d1].frequency[i];
	freq[6] = '\0';
	return(D_REDRAW);
    }
    return D_O_K;
}

char *FreqList(int index, int *list_size)
{
    if(index < 0)
    {
	*list_size = digi[digi_allegro_num].maxfreq + 1;
	return NULL;
    }
    return(freqs[index].frequency);
}

int FreqListProc(int msg, DIALOG *d, int c)
{
    d_list_proc(msg, d, c);
    if(msg == MSG_DCLICK)
    {
	return(D_EXIT);
    }
    return D_O_K;
}
