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
 *      The grabber utility program.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <bios.h>
#include <stdio.h>
#include <dir.h>

#include "allegro.h"


#define NAME_SIZE      16

typedef struct DATAITEM
{
   char type_str[8];
   char name[NAME_SIZE];
   int type;
   long size;
   void *dat;
} DATAITEM;

DATAITEM *data = NULL;
int data_count = 0;
int data_malloced = 0;

char *load_this_file = NULL;

char data_file[80] = "";

char import_file[80] = "";

BITMAP *graphic = NULL;
PALLETE g_pallete;

PALLETE selected_pallete;

int data_obj_count = 0;
int font_obj_count = 0;
int bitmap_obj_count = 0;
int pallete_obj_count = 0;
int sample_obj_count = 0;
int midi_obj_count = 0;
int rle_obj_count = 0;

int last_bmp_w = 32;
int last_bmp_h = 32;

char obj_name[NAME_SIZE] = "";


int mouse_focus_proc(int, DIALOG *, int);
int load_proc(int, DIALOG *, int);
int save_proc(int, DIALOG *, int);
int read_proc(int, DIALOG *, int);
int view_proc(int, DIALOG *, int);
int new_proc(int, DIALOG *, int);
int export_proc(int, DIALOG *, int);
int delete_proc(int, DIALOG *, int);
int quit_proc(int, DIALOG *, int);
int list_proc(int, DIALOG *, int);
char *list_getter(int, int *);
int viewer_proc(int, DIALOG *, int);
int name_proc(int, DIALOG *, int);
int help_proc(int, DIALOG *, int);
int grab_proc(int, DIALOG *, int);
int bmp_size_proc(int, DIALOG *, int);
int rle_flag_proc(int, DIALOG *, int);


extern DIALOG new_dlg[];


DIALOG main_dlg[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)  (d2)  (dp) */
   { d_clear_proc,      0,    0,    0,    0,    255,  0,    0,    0,          0,    0,    NULL },
   { d_ctext_proc,      320,  4,    1,    1,    255,  0,    0,    0,          0,    0,    "Allegro Data File Editor, version " VERSION_STR },
   { d_ctext_proc,      320,  20,   1,    1,    255,  0,    0,    0,          0,    0,    "By Shawn Hargreaves, " DATE_STR },
   { d_text_proc,       20,   44,   1,    1,    255,  0,    0,    0,          0,    0,    "Editing file:" },
   { d_text_proc,       144,  44,   1,    1,    255,  0,    0,    0,          0,    0,    data_file },
   { d_check_proc,      20,   68,   100,  12,   255,  0,    'p',  D_SELECTED, 0,    0,    "Pack file:" },
   { mouse_focus_proc,  144,  68,   140,  12,   255,  0,    0,    D_SELECTED, 0,    0,    "Mouse focusing:" },
   { load_proc,         20,   100,  104,  16,   255,  0,    'l',  D_EXIT,     0,    0,    "Load" },
   { save_proc,         144,  100,  104,  16,   255,  0,    's',  D_EXIT,     0,    0,    "Save" },
   { read_proc,         266,  100,  104,  16,   255,  0,    'r',  D_EXIT,     0,    0,    "Read PCX" },
   { view_proc,         388,  100,  104,  16,   255,  0,    'v',  D_EXIT,     0,    0,    "View PCX" },
   { quit_proc,         510,  100,  104,  16,   255,  0,    27,   D_EXIT,     0,    0,    "Quit" },
   { new_proc,          20,   124,  104,  16,   255,  0,    'n',  D_EXIT,     0,    0,    "New Object" },
   { delete_proc,       144,  124,  104,  16,   255,  0,    8,    D_EXIT,     0,    0,    "Delete" },
   { delete_proc,       -32,  -32,  10,   10,   255,  0,    127,  D_EXIT,     0,    0,    "" },
   { export_proc,       266,  124,  104,  16,   255,  0,    'e',  D_EXIT,     0,    0,    "Export" },
   { help_proc,         510,  124,  104,  16,   255,  0,    'h',  D_EXIT,     0,    0,    "Help!" },
   { list_proc,         20,   168,  208,  299,  255,  0,    0,    D_EXIT,     0,    0,    list_getter },
   { viewer_proc,       266,  218,  1,    1,    255,  0,    0,    0,          0,    0,    NULL },
   { d_text_proc,       266,  168,  1,    1,    255,  0,    0,    0,          0,    0,    "Name:" },
   { name_proc,         314,  168,  150,  16,   255,  0,    0,    0,          15,   0,    obj_name },
   { grab_proc,         388,  124,  104,  16,   255,  0,    'g',  D_EXIT,     0,    0,    "" },
   { bmp_size_proc,     388,  196,  104,  16,   255,  0,    0,    0,          0,    0,    "xxxxxxxx" },
   { bmp_size_proc,     510,  196,  104,  16,   255,  0,    0,    0,          0,    1,    "xxxxxxxx" },
   { rle_flag_proc,     510,  168,  104,  12,   255,  0,    0,    0,          0,    0,    "RLE:" },
   { NULL }
};

#define DLG_FILENAME    4
#define DLG_PACKFILE    5
#define DLG_LIST        17
#define DLG_VIEWER      18

#define FG              main_dlg[0].fg
#define BG              main_dlg[0].bg

#define CURRENT_ITEM    main_dlg[DLG_LIST].d1

#define SAFE_CURRENT_ITEM        ((CURRENT_ITEM >= 0) ?                 \
				    ((CURRENT_ITEM < data_count) ?      \
				       CURRENT_ITEM :                   \
				       -1) :                            \
				    -1)



char *help_text[] =
{
"An Allegro data file is a bit like a zip file in that it consists of lots of",
"different pieces of data stuck together one after another. This means that",
"your game doesn't have to clutter up the disk with hundreds of tiny files,",
"and it makes programming easier because you can load everything with a",
"single function call at program startup. Another benefit is that the file",
"compression algorithm works much better with one large file than with many",
"small ones.",
"",
"",
"Data files can contain bitmaps, palletes, fonts, samples, MIDI music, and",
"any other binary data that you import. To load one of these files into",
"memory from within your program, call the routine:",
"",
"",
"DATAFILE *load_datafile(char *filename);",
"   Loads a data file into memory, and returns a pointer to it, or NULL on",
"   error. All the objects in the file will be locked into physical memory.",
"",
"",
"void unload_datafile(DATAFILE *dat);",
"   Frees all the objects in a data file.",
"",
"",
"When you load a data file, you will obtain a pointer to an array of DATAFILE",
"structures:",
"",
"",
"typedef struct DATAFILE",
"{",
"   void *dat;     - pointer to the actual data",
"   int type;      - data type",
"   long size;     - if type == DAT_DATA, this is the size of the data",
"} DATAFILE;",
"",
"",
"The type field will be one of the values:",
"   DAT_DATA       - dat points to a block of binary data",
"   DAT_FONT_8x8   - dat points to an 8x8 fixed pitch font",
"   DAT_FONT_PROP  - dat points to a proportional font",
"   DAT_BITMAP     - dat points to a BITMAP structure",
"   DAT_PALLETE    - dat points to an array of 256 RGB structures",
"   DAT_SAMPLE     - dat points to a sample structure",
"   DAT_MIDI       - dat points to a MIDI file",
"   DAT_RLE_SPRITE - dat points to a RLE_SPRITE structure",
"   DAT_END        - special flag for the end of the data list",
"",
"",
"The grabber program will also produce a header file defining the index of",
"each object within the file. So, for example, if you have made a data file",
"which contains a bitmap called THE_IMAGE, you could display it with the code",
"fragment:",
"",
"",
"   #include \"foo.h\"",
"   DATAFILE *data = load_datafile(\"foo.dat\");",
"   draw_sprite(screen, data[THE_IMAGE].dat, x, y);",
"",
"",
"As well as being useful for including in your C code, this header file is",
"used by the grabber's load routine to retrieve the names of the objects.",
NULL,
"Grabber commands can be selected by clicking on a button or typing the",
"first letter of the command name.",
"",
"",
"Load:",
"   Loads a data file into the grabber program.",
"",
"",
"Save:",
"   Saves the data file. If you have checked the \"pack file\" button the data",
"   will be compressed, which may take a while but will reduce the amount of",
"   disk space required.",
"",
"",
"Read PCX:",
"   Loads a 256 color PCX file into the image buffer.",
"",
"",
"View PCX:",
"   Displays the contents of the image buffer.",
"",
"",
"New Object:",
"   Creates a new object and adds it to the data file. You will be asked",
"   what type of object to create (binary data, font, bitmap, pallete,",
"   sample, or MIDI). ",
"",
"",
"All the objects in the data file are listed in the box at the bottom left",
"of the screen, from which you can select one with the mouse or arrow keys.",
"The name of the current object is shown to the right of this box: click",
"here to type in a different name. Double-clicking on an object in the list",
"performs a function which varies depending on the type of the object.",
"Bitmaps and fonts are displayed full-screen, samples and MIDI files are",
"played, and palletes are selected (meaning that they will be used when",
"displaying and exporting bitmaps).",
"",
"",
"When a bitmap object is selected, the width and height of the bitmap are",
"displayed towards the right of the screen. These can be altered by clicking",
"on them with the left (to decrease) or right (to increase) mouse buttons. To",
"make large alterations, hold down the shift key. If you check the RLE",
"button, the bitmap will be saved as an RLE_SPRITE structure rather than a",
"regular BITMAP structure.",
"",
"",
"Delete:",
"   Deletes the selected object.",
"",
"",
"Export:",
"   Exports the selected object to a file. You can export all types of",
"   objects except palletes. To save a pallete into a PCX file, select it (by",
"   double clicking on it in the item list), and then export a bitmap object,",
"   which will be saved using the selected pallete.",
NULL,
"Grab/Import:",
"   If the selected object is a binary data object, a sample, or a MIDI file,",
"   this command replaces the object with new data imported from a file. If",
"   it is a bitmap, a pallete, or a font, it replaces it with data grabbed",
"   from the image buffer (so you must first read a PCX file into the image",
"   buffer). When grabbing a bitmap you can use the mouse to select the",
"   portion of the image buffer to grab. When grabbing a font, the size of",
"   each character is determined by the layout of the bitmap in the image",
"   buffer. This should be a rectangular grid containing all the ASCII",
"   characters from space (32) up to the tilde (126). The spaces between each",
"   letter should be filled with color #255. If each character is sized",
"   exactly 8x8 the grabber will create a fixed size 8x8 font, otherwise it",
"   will make a proportional font. Probably the easiest way to get to grips",
"   with how this works is to load up the demo.dat file and export the",
"   TITLE_FONT into a pcx file. That is the format a font should be in...",
"",
"",
"I'm not going to insult your intelligence by trying to explain what the Quit",
"and Help buttons do.",
"",
"",
"By Shawn Hargreaves,",
"1 Salisbury Road,",
"Market Drayton,",
"Shropshire,",
"England, TF9 1AJ.",
NULL,
NULL
};



char my_mouse_pointer_data[256] =
{
   2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
   2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
   2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
   2, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
   2, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
   2, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
   2, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 
   2, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 
   2, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 
   2, 1, 1, 1, 1, 1, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 
   2, 1, 1, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
   2, 1, 2, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 
   0, 2, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 
   0, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 
   0, 0, 0, 0, 0, 2, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 
   0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0
};


BITMAP *my_mouse_pointer = NULL;



void draw_wait(char *msg)
{
   int gap = msg ? 12 : 0;

   show_mouse(NULL);
   rectfill(screen, SCREEN_W/2-99, SCREEN_H/2-19-gap, SCREEN_W/2+99, SCREEN_H/2+19+gap, BG);
   rect(screen, SCREEN_W/2-100, SCREEN_H/2-20-gap, SCREEN_W/2+100, SCREEN_H/2+20+gap, FG);
   text_mode(BG);
   textout_centre(screen, font, "Please wait a moment", SCREEN_W/2, SCREEN_H/2-4-gap, FG);
   if (msg)
      textout_centre(screen, font, msg, SCREEN_W/2, SCREEN_H/2+8, FG);
   show_mouse(screen);
}



void select_pallete(RGB *pal)
{
   int c, x, y;
   int fg, bg, fgt, bgt, t;

   selected_pallete[0] = pal[0];

   fg = 255;
   bg = 0;
   fgt = 0xFFFF;
   bgt = 0;

   for (c=1; c<PAL_SIZE; c++) {
      selected_pallete[c] = pal[c];
      t = (int)pal[c].r + (int)pal[c].g + (int)pal[c].b;
      if (t <= fgt) {
	 fgt = t;
	 fg = c;
      }
      if (t >= bgt) {
	 bgt = t;
	 bg = c;
      }
   }

   set_pallete(selected_pallete);

   gui_fg_color = fg;
   gui_bg_color = bg;
   set_dialog_color(main_dlg, fg, bg);
   set_dialog_color(new_dlg, fg, bg);

   if (!my_mouse_pointer)
      my_mouse_pointer = create_bitmap(16, 16);

   for (y=0; y<16; y++) {
      for (x=0; x<16; x++) {
	 switch (my_mouse_pointer_data[x+y*16]) {
	    case 1:  c = fg; break;
	    case 2:  c = bg; break;
	    default: c = 0;  break;
	 }
	 putpixel(my_mouse_pointer, x, y, c);
      }
   }

   set_mouse_sprite(my_mouse_pointer);
}



void view_bitmap(BITMAP *b, RGB *pal)
{
   show_mouse(NULL);
   clear(screen);
   set_pallete(pal);
   blit(b, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);

   clear_keybuf();
   do {
   } while (mouse_b);

   do {
   } while ((!mouse_b) && (!keypressed()));

   clear_keybuf();
   do {
   } while (mouse_b);

   clear(screen);
   set_pallete(selected_pallete);
   show_mouse(screen);
}



int select_object(int index, int redraw)
{
   int i;
   int ret = D_O_K;

   if (index <= 0)
      CURRENT_ITEM = 0;
   else if (index >= data_count)
      CURRENT_ITEM = data_count - 1;
   else 
      CURRENT_ITEM = index;

   show_mouse(NULL);

   for (i=DLG_LIST; main_dlg[i].proc; i++) {
      ret |= SEND_MESSAGE(main_dlg+i, MSG_START, 0);
      if (redraw)
	 ret |= SEND_MESSAGE(main_dlg+i, MSG_DRAW, 0);
   }

   show_mouse(screen);
   return ret;
}



int dataitem_cmp(const void *p1, const void *p2)
{
   DATAITEM *d1 = (DATAITEM *)p1;
   DATAITEM *d2 = (DATAITEM *)p2;

   return strcmp(d1->name, d2->name);
}



void sort_data(int force)
{
   void *sel;
   int c;
   int old_pos = SAFE_CURRENT_ITEM;

   if (data_count > 0) {
      sel = data[CURRENT_ITEM].dat;
      qsort(data, data_count, sizeof(DATAITEM), dataitem_cmp);
   }
   else
      sel = NULL;

   for (c=0; c<data_count; c++) {
      if (data[c].dat == sel) {
	 if ((c != old_pos) || (force))
	    select_object(c, TRUE);
	 break;
      }
   }
}



void destroy_object(DATAITEM *obj)
{
   switch (obj->type) {

      case DAT_FONT_8x8:
      case DAT_FONT_PROP:
	 destroy_font(obj->dat);
	 break;

      case DAT_BITMAP:
      case DAT_RLE_SPRITE:
	 destroy_bitmap(obj->dat);
	 break;

      case DAT_SAMPLE:
	 destroy_sample(obj->dat);
	 break;

      case DAT_MIDI:
	 destroy_midi(obj->dat);
	 break;

      default:
	 free(obj->dat);
	 break;
   }
}



void destroy_data()
{
   int c;

   for (c=0; c<data_count; c++)
      destroy_object(data+c);

   if (data)
      free(data);

   data = NULL;
   data_count = data_malloced = 0;
}



FONT *create_font(FONT *f)
{
   FONT *new_f;
   int i;
   char buf[2];

   new_f = malloc(sizeof(FONT));
   new_f->flag_8x8 = FALSE;
   new_f->dat.dat_prop = malloc(sizeof(FONT_PROP));
   buf[1] = 0;
   text_mode(0);

   for (i=0; i<FONT_SIZE; i++) {
      new_f->dat.dat_prop->dat[i] = create_bitmap(8, 8);
      clear(new_f->dat.dat_prop->dat[i]);
      buf[0] = i+' ';
      textout(new_f->dat.dat_prop->dat[i], f, buf, 0, 0, 1);
   }

   return new_f;
}



FONT *fixup_font(FONT *f)
{
   /* To keep life simple, all fonts are held in memory as proportional
    * ones. This routine converts 8x8 to proportional fonts as required.
    */

   FONT *new_f;

   if (!f->flag_8x8)
      return f;

   new_f = create_font(f);
   destroy_font(f);

   return new_f;
}



BITMAP *fixup_rle_sprite(RLE_SPRITE *s)
{
   /* To keep life simple, all bitmaps are held in memory in uncompressed
    * format. This routine decompresses RLE sprites.
    */

   BITMAP *b;

   b = create_bitmap(s->w, s->h);
   clear(b);
   draw_rle_sprite(b, s, 0, 0);
   destroy_rle_sprite(s);

   return b;
}



void add_object(DATAFILE *obj, int sort)
{
   char *s;

   if (data_count <= data_malloced) {
      data_malloced = data_count+16;
      data = realloc(data, data_malloced*sizeof(DATAITEM));
   }

   data[data_count].type = obj->type;
   data[data_count].size = obj->size;
   data[data_count].dat = obj->dat;
   s = data[data_count].type_str;

   switch (obj->type) {

      case DAT_FONT_8x8:
      case DAT_FONT_PROP:
	 sprintf(s, "(font)  FONT_%d", ++font_obj_count);
	 break;

      case DAT_BITMAP:
	 sprintf(s, "(bmp)   BITMAP_%d", ++bitmap_obj_count);
	 break;

      case DAT_PALLETE:
	 sprintf(s, "(pal)   PALLETE_%d", ++pallete_obj_count);
	 break;

      case DAT_SAMPLE:
	 sprintf(s, "(spl)   SAMPLE_%d", ++sample_obj_count);
	 break;

      case DAT_MIDI:
	 sprintf(s, "(midi)  MIDI_%d", ++midi_obj_count);
	 break;

      case DAT_RLE_SPRITE:
	 sprintf(s, "(rle)   BITMAP_%d", ++rle_obj_count);
	 break;

      default:
	 sprintf(s, "(data)  DATA_%d", ++data_obj_count);
	 break;
   }

   data_count++;

   if (sort) {
      CURRENT_ITEM = data_count - 1;
      sort_data(TRUE);
   }
}



void load()
{
   DATAFILE *df;
   PACKFILE *f = NULL;
   char buf[80];
   char buf2[20];
   int c2, i;
   char *s;
   int c;

   draw_wait(NULL);
   destroy_data();

   df = load_datafile(data_file);
   if (!df) {
      alert("Error loading data file", data_file, NULL, "Oh dear", NULL, 13, 0);
      return;
   }

   data_obj_count = font_obj_count = bitmap_obj_count = pallete_obj_count = 
      sample_obj_count = midi_obj_count = rle_obj_count = 0;

   for (c=0; df[c].type != DAT_END; c++) {
      if (df[c].type == DAT_FONT_8x8)
	 df[c].dat = fixup_font(df[c].dat);
      else if (df[c].type == DAT_RLE_SPRITE)
	 df[c].dat = fixup_rle_sprite(df[c].dat);

      add_object(df+c, FALSE);
   }

   free(df);

   strcpy(buf, data_file);
   s = get_extension(buf);
   if ((s > buf) & (*(s-1)=='.'))
      strcpy(s, "H");
   else
      strcpy(s, ".H");

   f = pack_fopen(buf, F_READ);
   if (!f) {
      alert("Unable to open header file", buf, NULL, "Oh well...", NULL, 13, 0);
      return;
   }

   while (pack_fgets(buf, 80, f) != 0) {
      if (strncmp(buf, "#define ", 8) == 0) {
	 c2 = 0;
	 c = 8;

	 while ((buf[c]) && (buf[c] != ' ') && (c2 < NAME_SIZE))
	    buf2[c2++] = buf[c++];

	 buf2[c2] = 0;
	 while (buf[c]==' ')
	    c++;

	 i = 0;
	 while ((buf[c] >= '0') && (buf[c] <= '9')) {
	    i *= 10;
	    i += buf[c] - '0';
	    c++;
	 }

	 if (i < data_count)
	    strcpy(data[i].name, buf2);
      }
   }

   pack_fclose(f);
   if (errno) {
      alert("Error reading header file", NULL, NULL, "Oh dear", NULL, 13, 0);
      errno = 0;
   }
}



int load_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      char buf[80];

      if (data_count > 0)
	 if (alert("Abandon current data file?", NULL, NULL, "Yes", "Cancel", 'y', 27) == 2)
	    return D_REDRAW;

      draw_wait(NULL);
      strcpy(buf, data_file);
      *get_filename(buf) = 0;

      if (file_select("Load data file", buf, "DAT")) {
	 strcpy(data_file, buf);
	 main_dlg[DLG_FILENAME].dp = get_filename(data_file);
	 load();
	 select_object(0, FALSE);
      }

      return D_REDRAW;
   }

   return ret;
}



void save(char *filename, char *header_name, int pack_flag)
{
   PACKFILE *df = NULL;
   PACKFILE *hf = NULL;
   char buf[80], buf2[80];
   int c, c2, c3, c4;
   long d;
   long size, outsize;
   BITMAP *b;
   RLE_SPRITE *s;
   RGB *rgb;

   df = pack_fopen(filename, (pack_flag) ? F_WRITE_PACKED : F_WRITE_NOPACK);
   if (!df)
      goto err;

   hf = pack_fopen(header_name, F_WRITE);
   if (!hf)
      goto err;

   pack_mputl(DAT_MAGIC, df);
   pack_mputw(data_count, df);
   size = 6;

   pack_fputs("/* Allegro data file object indexes */\n/* Do not hand edit! */\n\n", hf);
   if (errno)
      goto err;

   for (c=0; c<data_count; c++) {
      draw_wait(data[c].type_str);

      pack_mputw(data[c].type, df);
      size += 2;

      switch (data[c].type) {

	 case DAT_DATA:
	    pack_mputl(data[c].size, df);
	    pack_fwrite(data[c].dat, data[c].size, df);
	    size += data[c].size + 4;
	    break;

	 case DAT_FONT_8x8: 
	    for (c2=0; c2<FONT_SIZE; c2++) {
	       b = ((FONT *)data[c].dat)->dat.dat_prop->dat[c2];
	       for (c3=0; c3<8; c3++) {
		  d = 0;
		  for (c4=0; c4<8; c4++) {
		     d <<= 1;
		     if (b->line[c3][c4])
			d |= 1;
		  }
		  pack_putc(d, df);
		  size++;
	       }
	    }
	    break;

	 case DAT_FONT_PROP: 
	    for (c2=0; c2<FONT_SIZE; c2++) {
	       b = ((FONT *)data[c].dat)->dat.dat_prop->dat[c2];
	       pack_mputw(b->w, df);
	       pack_mputw(b->h, df);
	       pack_fwrite(b->line[0], b->w * b->h, df);
	       size += b->w * b->h + 4;
	    }
	    break;

	 case DAT_BITMAP:
	    b = (BITMAP *)data[c].dat;
	    pack_mputw(b->w, df);
	    pack_mputw(b->h, df);
	    pack_fwrite(b->line[0], b->w * b->h, df);
	    size += b->w * b->h + 4;
	    break;

	 case DAT_PALLETE: 
	    rgb = (RGB *)data[c].dat;
	    for (c2=0; c2<PAL_SIZE; c2++) {
	       pack_putc(rgb->r << 2, df);
	       pack_putc(rgb->g << 2, df);
	       pack_putc(rgb->b << 2, df);
	       rgb++;
	    }
	    size += PAL_SIZE*3;
	    break;

	 case DAT_SAMPLE:
	    d = ((SAMPLE *)data[c].dat)->len;
	    pack_mputw(((SAMPLE *)data[c].dat)->bits, df);
	    pack_mputw(((SAMPLE *)data[c].dat)->freq, df);
	    pack_mputl(d, df);
	    pack_fwrite(((SAMPLE *)data[c].dat)->data, d, df);
	    size += 8 + d;
	    break;

	 case DAT_MIDI:
	    pack_mputw(((MIDI *)data[c].dat)->divisions, df);
	    size += 2;
	    for (c2=0; c2<MIDI_TRACKS; c2++) {
	       d = ((MIDI *)data[c].dat)->track[c2].len;
	       pack_mputl(d, df);
	       if (d > 0)
		  pack_fwrite(((MIDI *)data[c].dat)->track[c2].data, d, df);
	       size += 4 + d;
	    }
	    break;

	 case DAT_RLE_SPRITE:
	    b = (BITMAP *)data[c].dat;
	    s = get_rle_sprite(b);
	    pack_mputw(s->w, df);
	    pack_mputw(s->h, df);
	    pack_mputl(s->size, df);
	    pack_fwrite(s->dat, s->size, df);
	    size += 8 + s->size;
	    destroy_rle_sprite(s);
	    break;
      }

      if (errno)
	 goto err;

      sprintf(buf, "#define %-20s%-8d/* %6.6s */\n", data[c].name, c, data[c].type_str);
      pack_fputs(buf, hf);
      if (errno)
	 goto err; 
   }

   pack_fclose(df);
   pack_fclose(hf);
   df = hf = NULL;

   if (errno == 0) {
      outsize = file_size(filename);
      if (pack_flag) {
	 sprintf(buf, "%ld bytes packed into", size);
	 sprintf(buf2, "%ld bytes - %ld%%", outsize, (outsize*100L+size/2)/size);
      }
      else {
	 sprintf(buf, "%ld bytes - unpacked", outsize);
	 buf2[0] = 0;
      }
      alert("File saved:", buf, buf2, "OK", NULL, 13, 0);
      return;
   }

   err:
   alert("Error saving file", NULL, NULL, "Oh dear", NULL, 13, 0);

   if (df)
      pack_fclose(df);

   if (hf)
      pack_fclose(hf);

   errno = 0;
}



int save_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      char buf[80], buf2[80], buf3[80];
      int pack = main_dlg[DLG_PACKFILE].flags & D_SELECTED;
      int e1, e2;
      char *s;

      s = pack ? "Save data file (packed)" : "Save data file (unpacked)";
      strcpy(buf, data_file);
      draw_wait(NULL);

      if (file_select(s, buf, "DAT")) {
	 strcpy(buf2, buf);

	 s = get_extension(buf);
	 if (*s == 0)
	    strcpy(s, ".DAT");

	 s = get_extension(buf2);
	 if ((s > buf2) & (*(s-1)=='.'))
	    strcpy(s, "H");
	 else
	    strcpy(s, ".H");

	 e1 = file_exists(buf, FA_RDONLY | FA_HIDDEN, NULL);
	 e2 = file_exists(buf2, FA_RDONLY | FA_HIDDEN, NULL);
	 if ((e1) || (e2)) {
	    if (e1) {
	       strcpy(buf3, get_filename(buf));
	       if (e2) {
		  strcat(buf3, " and ");
		  strcat(buf3, get_filename(buf2));
	       }
	    }
	    else if (e2)
	       strcat(buf3, get_filename(buf2));
	    strcat(buf3, "?");
	    if (alert("Overwrite existing", buf3, NULL, "Yes", "Cancel", 'y', 27) == 2)
	       return D_REDRAW;
	 }

	 strcpy(data_file, buf);
	 main_dlg[DLG_FILENAME].dp = get_filename(data_file);
	 save(buf, buf2, pack);
      }
      return D_REDRAW;
   }

   return ret;
}



int read_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      char buf[80];
      draw_wait(NULL);
      strcpy(buf, import_file);
      *get_filename(buf) = 0;
      if (file_select("Read PCX file", buf, "PCX")) {
	 strcpy(import_file, buf);
	 draw_wait(NULL);
	 if (graphic)
	    destroy_bitmap(graphic);
	 graphic = load_pcx(import_file, g_pallete);
	 if (graphic)
	    view_bitmap(graphic, g_pallete);
	 else
	    alert("Error loading PCX file", NULL, NULL, "Oh dear", NULL, 13, 0);
      }
      return D_REDRAW;
   }

   return ret;
}



int view_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      if (graphic)
	 view_bitmap(graphic, g_pallete);
      else
	 alert("Nothing to view!",
	       "First you must read in a PCX file", 
	       NULL, "OK", NULL, 13, 0);

      return D_REDRAW;
   }

   return ret;
}



SAMPLE *make_new_sample()
{
   SAMPLE *spl;
   int c;

   spl = malloc(sizeof(SAMPLE));
   spl->bits = 8;
   spl->freq = 11025;
   spl->len = 1024;
   spl->data = malloc(1024);

   for (c=0; c<1024; c++)
      spl->data[c] = c & 0xFF;

   return spl;
}



MIDI *make_new_midi()
{
   MIDI *mid;
   int c;

   mid = malloc(sizeof(MIDI));
   mid->divisions = 120;

   for (c=0; c<MIDI_TRACKS; c++) {
      mid->track[c].data = NULL;
      mid->track[c].len = 0;
   }

   return mid;
}



DIALOG new_dlg[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp) */
   { d_shadow_box_proc, 0,    0,    160,  208,  255,  0,    0,    0,       0,    0,    NULL },
   { d_ctext_proc,      80,   8,    1,    1,    255,  0,    0,    0,       0,    0,    "New Object" },
   { d_button_proc,     16,   32,   128,  16,   255,  0,    'd',  D_EXIT,  0,    0,    "Data (binary)" },
   { d_button_proc,     16,   56,   128,  16,   255,  0,    'f',  D_EXIT,  0,    0,    "Font" },
   { d_button_proc,     16,   80,   128,  16,   255,  0,    'b',  D_EXIT,  0,    0,    "Bitmap" },
   { d_button_proc,     16,   104,  128,  16,   255,  0,    'p',  D_EXIT,  0,    0,    "Pallete" },
   { d_button_proc,     16,   128,  128,  16,   255,  0,    's',  D_EXIT,  0,    0,    "Sample" },
   { d_button_proc,     16,   152,  128,  16,   255,  0,    'm',  D_EXIT,  0,    0,    "MIDI" },
   { d_button_proc,     16,   176,  128,  16,   255,  0,    27,   D_EXIT,  0,    0,    "Cancel" },
   { NULL }
};

#define NEW_DLG_DATA       2
#define NEW_DLG_FONT       3
#define NEW_DLG_BITMAP     4
#define NEW_DLG_PALLETE    5
#define NEW_DLG_SAMPLE     6
#define NEW_DLG_MIDI       7
#define NEW_DLG_CANCEL     8



int new_proc(int msg, DIALOG *d, int c)
{
   int i;
   DATAFILE datafile;
   int ret;

   ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      centre_dialog(new_dlg);
      ret = do_dialog(new_dlg, -1);

      switch (ret) {

	 case NEW_DLG_DATA:
	    datafile.type = DAT_DATA;
	    datafile.size = 16;
	    datafile.dat = malloc(16);
	    memcpy(datafile.dat, "hello, everyone", 16);
	    add_object(&datafile, TRUE);
	    break;

	 case NEW_DLG_FONT:
	    datafile.type = DAT_FONT_8x8;
	    datafile.size = 0;
	    datafile.dat = create_font(font);
	    add_object(&datafile, TRUE);
	    break;

	 case NEW_DLG_BITMAP:
	    datafile.type = DAT_BITMAP;
	    datafile.size = 0;
	    datafile.dat = create_bitmap(last_bmp_w, last_bmp_h);
	    rectfill(datafile.dat, 0, 0, last_bmp_w, last_bmp_h, BG);
	    text_mode(BG);
	    textout_centre(datafile.dat, font, "bmp", last_bmp_w/2, last_bmp_h/2-4, FG);
	    add_object(&datafile, TRUE);
	    break;

	 case NEW_DLG_PALLETE:
	    datafile.type = DAT_PALLETE;
	    datafile.size = 0;
	    datafile.dat = malloc(sizeof(PALLETE));
	    for (i=0; i<PAL_SIZE; i++)
	       ((RGB *)datafile.dat)[i] = desktop_pallete[i];
	    add_object(&datafile, TRUE);
	    break;

	 case NEW_DLG_SAMPLE:
	    datafile.type = DAT_SAMPLE;
	    datafile.size = 0;
	    datafile.dat = make_new_sample();
	    add_object(&datafile, TRUE);
	    break;

	 case NEW_DLG_MIDI:
	    datafile.type = DAT_MIDI;
	    datafile.size = 0;
	    datafile.dat = make_new_midi();
	    add_object(&datafile, TRUE);
	    break;
      }

      return D_REDRAW;
   }

   return ret;
}



void export_font(char *fname, FONT_PROP *f)
{
   BITMAP *b;
   int w, h, c;

   w = 0;
   h = 0;

   for (c=0; c<FONT_SIZE; c++) {
      if (f->dat[c]->w > w)
	 w = f->dat[c]->w;
      if (f->dat[c]->h > h)
	 h = f->dat[c]->h;
   }

   w = (w+16) & 0xFFF0;
   h = (h+16) & 0xFFF0;

   b = create_bitmap(1+w*16, 1+h*((FONT_SIZE+15)/16));
   rectfill(b, 0, 0, b->w, b->h, 255);

   for (c=0; c<FONT_SIZE; c++)
      blit(f->dat[c], b, 0, 0, 1+w*(c&15), 1+h*(c/16), w, h);

   save_pcx(fname, b, desktop_pallete);
   destroy_bitmap(b);
}



void save_wav(char *fname, SAMPLE *spl)
{
   PACKFILE *f;

   f = pack_fopen(fname, F_WRITE);
   if (!f)
      return;

   pack_fputs("RIFF", f);                 /* RIFF header */
   pack_iputl(36+spl->len, f);            /* size of RIFF chunk */
   pack_fputs("WAVE", f);                 /* WAV definition */
   pack_fputs("fmt ", f);                 /* format chunk */
   pack_iputl(16, f);                     /* size of format chunk */
   pack_iputw(1, f);                      /* PCM data */
   pack_iputw(1, f);                      /* mono data */
   pack_iputl(spl->freq, f);              /* sample frequency */
   pack_iputl(spl->freq, f);              /* avg. bytes per sec */
   pack_iputw(1, f);                      /* block alignment */
   pack_iputw(8, f);                      /* bits per sample */
   pack_fputs("data", f);                 /* data chunk */
   pack_iputl(spl->len, f);               /* actual data length */
   pack_fwrite(spl->data, spl->len, f);   /* write the data */

   pack_fclose(f);
}



void save_midi(char *fname, MIDI *midi)
{
   PACKFILE *f;
   int c;
   int num_tracks;

   num_tracks = 0;
   for (c=0; c<MIDI_TRACKS; c++)
      if (midi->track[c].len > 0)
	 num_tracks++;

   f = pack_fopen(fname, F_WRITE);
   if (!f)
      return;

   pack_fputs("MThd", f);                 /* MIDI header */
   pack_mputl(6, f);                      /* size of header chunk */
   pack_mputw(1, f);                      /* type 1 */
   pack_mputw(num_tracks, f);             /* number of tracks */
   pack_mputw(midi->divisions, f);        /* beat divisions */

   for (c=0; c<MIDI_TRACKS; c++) {        /* for each track */
      if (midi->track[c].len > 0) {
	 pack_fputs("MTrk", f);           /* write track data */
	 pack_mputl(midi->track[c].len, f); 
	 pack_fwrite(midi->track[c].data, midi->track[c].len, f);
      }
   }

   pack_fclose(f);
}



int export_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      char buf[80];
      DATAITEM *item;
      PACKFILE *f;
      char *s;
      char *ext;

      if (SAFE_CURRENT_ITEM < 0) {
	 alert("Nothing to export!", NULL, NULL, "OK", NULL, 13, 0);
	 return D_REDRAW;
      }

      item = data+CURRENT_ITEM;

      switch (item->type) {

	 case DAT_FONT_8x8:
	 case DAT_FONT_PROP:
	    s = "Export font";
	    ext = "PCX";
	    break;

	 case DAT_BITMAP:
	 case DAT_RLE_SPRITE:
	    s = "Export bitmap";
	    ext = "PCX";
	    break;

	 case DAT_PALLETE:
	    alert("To save a pallete into a PCX file,",
		  "select it (by double clicking on it)",
		  "and then export a bitmap object", 
		  "Understood", NULL, 13, 0);
	    return D_REDRAW;

	 case DAT_SAMPLE:
	    s = "Export WAV";
	    ext = "WAV";
	    break;

	 case DAT_MIDI:
	    s = "Export MIDI file";
	    ext = "MID";
	    break;

	 default: 
	    s = "Export binary data"; 
	    ext = NULL;
	    break;
      }

      draw_wait(NULL);
      strcpy(buf, import_file);
      *get_filename(buf) = 0;

      if (file_select(s, buf, ext)) {
	 strcpy(import_file, buf);

	 s = get_extension(buf);
	 if ((*s == 0) && (ext)) {
	    *s = '.';
	    strcpy(s+1, ext);
	 }

	 if (file_exists(buf, FA_RDONLY | FA_HIDDEN, NULL))
	    if (alert("Overwrite existing", buf, NULL, "Yes", "Cancel", 'y', 27) == 2)
	       return D_REDRAW;

	 draw_wait(NULL);

	 switch (item->type) {

	    case DAT_DATA: 
	       f = pack_fopen(buf, F_WRITE);
	       pack_fwrite(item->dat, item->size, f);
	       pack_fclose(f);
	       break;

	    case DAT_FONT_8x8:
	    case DAT_FONT_PROP:
	       export_font(buf, ((FONT *)item->dat)->dat.dat_prop);
	       break;

	    case DAT_BITMAP:
	    case DAT_RLE_SPRITE:
	       save_pcx(buf, item->dat, selected_pallete);
	       break;

	    case DAT_SAMPLE:
	       save_wav(buf, item->dat);
	       break;

	    case DAT_MIDI:
	       save_midi(buf, item->dat);
	       break;
	 }

	 if (errno != 0) {
	    alert("Error writing", buf, NULL, "Oh dear", NULL, 13, 0);
	    errno = 0;
	 }
      }
      return D_REDRAW;
   }

   return ret;
}



int delete_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      char buf[80];
      int i;

      if (data_count) {
	 strcpy(buf, data[CURRENT_ITEM].name);
	 strcat(buf, "?");
	 if (alert("Really delete", buf, NULL, "OK", "Cancel", 0, 27) == 1) {
	    destroy_object(data+CURRENT_ITEM);
	    for (i=CURRENT_ITEM; i<data_count-1; i++)
	       data[i] = data[i+1];
	    data_count--;
	 }
	 select_object(CURRENT_ITEM, FALSE);
      }
      else
	 alert("Nothing to delete!", NULL, NULL, "OK", NULL, 13, 0);

      return D_REDRAW;
   }

   return ret;
}



int help_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);
   int i, y;

   if (ret == D_CLOSE) {
      show_mouse(NULL);
      text_mode(d->bg);
      i = 0;
      while (help_text[i]) {
	 clear_to_color(screen, d->bg);
	 y = 0;

	 while (help_text[i]) {
	    textout(screen, font, help_text[i], 0, y, d->fg);
	    y += 8;
	    i++;
	 }
	 i++;

	 clear_keybuf();
	 do {
	 } while (mouse_b);

	 do {
	 } while ((!mouse_b) && (!keypressed()));

	 clear_keybuf();
	 do {
	 } while (mouse_b);
      }

      show_mouse(screen);
      return D_REDRAW;
   }

   return ret;
}



int quit_proc(int msg, DIALOG *d, int c)
{
   int ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      if (alert("Really want to quit?", NULL, NULL, "Quit", "Cancel", 'q', 27) == 1)
	 return D_CLOSE;
      else
	 return D_REDRAW;
   }

   return ret;
}



int mouse_focus_proc(int msg, DIALOG *d, int c)
{
   int ret = d_check_proc(msg, d, c);

   if (d->flags & D_SELECTED)
      gui_mouse_focus = TRUE;
   else
      gui_mouse_focus = FALSE;

   return ret;
}



void view_font(FONT *f)
{
   int c;
   int x, y;
   char buf[2];

   show_mouse(NULL);
   clear_to_color(screen, BG);
   text_mode(BG);
   buf[1] = 0;

   for (c=0; c<FONT_SIZE; c++) {
      buf[0] = c+' ';
      x = (c&15) * 40;
      y = (c/16) * 80;
      textout(screen, font, buf, x, y, FG);
      textout(screen, f, buf, x+8, y+8, FG);
   }

   do {
   } while (mouse_b);
   clear_keybuf();

   do {
   } while ((!mouse_b) && (!keypressed()));

   do {
   } while (mouse_b);
   clear_keybuf();

   show_mouse(screen);
}



int list_proc(int msg, DIALOG *d, int c)
{
   int sel, ret;
   static int recurse_counter = 0;

   sel = SAFE_CURRENT_ITEM;

   recurse_counter++;
   ret = d_list_proc(msg, d, c);
   recurse_counter--;

   if ((recurse_counter == 0) && (sel != SAFE_CURRENT_ITEM))
      ret |= select_object(CURRENT_ITEM, !(ret & D_REDRAW));

   if (ret == D_EXIT) {
      if (SAFE_CURRENT_ITEM >= 0) {
	 if ((data[CURRENT_ITEM].type == DAT_BITMAP) ||
	     (data[CURRENT_ITEM].type == DAT_RLE_SPRITE)) {
	    view_bitmap(data[CURRENT_ITEM].dat, selected_pallete);
	    return D_REDRAW;
	 }
	 if (data[CURRENT_ITEM].type == DAT_PALLETE) {
	    select_pallete(data[CURRENT_ITEM].dat);
	    return D_REDRAW;
	 }
	 if ((data[CURRENT_ITEM].type == DAT_FONT_8x8) ||
	     (data[CURRENT_ITEM].type == DAT_FONT_PROP)) {
	    view_font(data[CURRENT_ITEM].dat);
	    return D_REDRAW;
	 }
	 if (data[CURRENT_ITEM].type == DAT_SAMPLE) {
	    play_sample(data[CURRENT_ITEM].dat, 255, 127, 1000, FALSE);
	    return D_O_K;
	 }
	 if (data[CURRENT_ITEM].type == DAT_MIDI) {
	    play_midi(data[CURRENT_ITEM].dat, FALSE);
	    return D_O_K;
	 }
      }
      return D_O_K;
   }

   return ret;
}



char *list_getter(int index, int *list_size)
{
   if (index < 0) {
      if (list_size)
	 *list_size = data_count;
      return NULL;
   }

   return data[index].type_str;
}



int compare_palletes(RGB *p1, RGB *p2)
{
   int c;

   for (c=0; c<PAL_SIZE; c++) {
      if ((p1[c].r != p2[c].r) || 
	  (p1[c].g != p2[c].g) || 
	  (p1[c].b != p2[c].b))
      return TRUE;
   }

   return FALSE;
}



int viewer_proc(int msg, DIALOG *d, int c)
{
   DATAITEM *item;
   char buf[80];
   int c1, c2;
   int ret = D_O_K;

   if (msg == MSG_DRAW) {
      rectfill(screen, d->x, d->y, SCREEN_W, SCREEN_H, d->bg);
      text_mode(d->bg);

      if (SAFE_CURRENT_ITEM >= 0) {
	 item = data+CURRENT_ITEM;

	 switch (item->type) {

	    case DAT_DATA:
	       sprintf(buf, "data (%ld bytes)", item->size);
	       textout(screen, font, buf, d->x, d->y, d->fg);
	       for (c1=0; c1<8; c1++) {
		  for (c2=0; c2<32; c2++) {
		     if ((c1*32+c2) >= item->size)
			buf[c2] = ' ';
		     else
			buf[c2] = ((char *)item->dat)[c1*32+c2];
		     if ((buf[c2] < 32) || (buf[c2] > 126))
			buf[c2] = ' ';
		  }
		  buf[32] = 0;
		  textout(screen, font, buf, d->x+24, d->y+32+c1*8, d->fg);
	       }
	       if (item->size > 32*8)
		  textout(screen, font, "...", d->x+24+32*8, d->y+108, d->fg);
	       break;

	    case DAT_FONT_8x8:
	    case DAT_FONT_PROP:
	       if (item->type == DAT_FONT_8x8)
		  textout(screen, font, "fixed pitch 8x8 font", d->x, d->y, d->fg);
	       else
		  textout(screen, font, "proportional font", d->x, d->y, d->fg);
	       text_mode(-1);
	       textout(screen, item->dat, " !\"#$%&'()*+,-./", d->x, d->y+32, d->fg);
	       textout(screen, item->dat, "0123456789:;<=>?", d->x, d->y+64, d->fg);
	       textout(screen, item->dat, "@ABCDEFGHIJKLMNO", d->x, d->y+96, d->fg);
	       textout(screen, item->dat, "PQRSTUVWXYZ[\\]^_", d->x, d->y+128, d->fg);
	       textout(screen, item->dat, "`abcdefghijklmno", d->x, d->y+160, d->fg);
	       textout(screen, item->dat, "pqrstuvwxyz{|}~", d->x, d->y+192, d->fg);
	       break;

	    case DAT_BITMAP:
	    case DAT_RLE_SPRITE:
	       c1 = ((BITMAP *)item->dat)->w;
	       c2 = ((BITMAP *)item->dat)->h;
	       if (item->type == DAT_BITMAP)
		  textout(screen, font, "bitmap", d->x, d->y, d->fg);
	       else
		  textout(screen, font, "RLE bitmap", d->x, d->y, d->fg);
	       rect(screen, d->x, d->y+16, d->x+c1+1, d->y+c2+17, d->fg);
	       blit(item->dat, screen, 0, 0, d->x+1, d->y+17, c1, c2);
	       break;

	    case DAT_PALLETE:
	       textout(screen, font, "pallete", d->x, d->y, d->fg);
	       if (compare_palletes(item->dat, selected_pallete)) {
		  textout(screen, font, "A different pallete is currently in use.", d->x, d->y+16, d->fg);
		  textout(screen, font, "To select this one, double-click on it", d->x, d->y+24, d->fg);
		  textout(screen, font, "in the item list.", d->x, d->y+32, d->fg);
	       }
	       else {
		  for (c1=0; c1<PAL_SIZE; c1++)
		     rectfill(screen, d->x+(c1&15)*8, d->y+(c1/16)*8, 
			      d->x+(c1&15)*8+7, d->y+(c1/16)*8+7, c1);
	       }
	       break;

	    case DAT_SAMPLE:
	       textout(screen, font, "sample", d->x, d->y, d->fg);
	       sprintf(buf, "freq = %d, length %ld bytes",
		     ((SAMPLE *)item->dat)->freq, ((SAMPLE *)item->dat)->len);
	       textout(screen, font, buf, d->x, d->y+16, d->fg);
	       textout(screen, font, "Double-click in the item list to play it", d->x, d->y+32, d->fg);
	       break;

	    case DAT_MIDI:
	       textout(screen, font, "MIDI file", d->x, d->y, d->fg);
	       textout(screen, font, "Double-click in the item list to play it", d->x, d->y+16, d->fg);
	       break;
	 }
      }
   }

   return ret;
}



int name_proc(int msg, DIALOG *d, int c)
{
   int obj = SAFE_CURRENT_ITEM;
   int ch;

   if (obj < 0) {
      ((char *)d->dp)[0] = 0;
      if (msg == MSG_WANTFOCUS)
	 return D_O_K;
   }
   else {
      if (msg == MSG_START) {
	 strcpy(d->dp, data[obj].name);
      }
      else {
	 if ((msg == MSG_LOSTFOCUS) || (msg == MSG_KEY)) {
	    strcpy(data[obj].name, d->dp);
	    sort_data(FALSE);
	 }
      }
   }

   if (msg == MSG_CHAR) {
      ch = c & 0xff;
      if ((ch >= 'a') && (ch <= 'z'))
	 c = (c & 0xffffff00L) | (ch - 'a' + 'A');
      else if (((ch < 'A') || (ch > 'Z')) && (ch != 0) && (ch != 8) &&
	       ((ch < '0') || (ch > '9')) && (ch != '_') && (ch != 127))
	 return D_O_K;
   }

   return d_edit_proc(msg, d, c);
}



void import_data(DATAITEM *item)
{
   char buf[80];
   PACKFILE *f;

   draw_wait(NULL);
   strcpy(buf, import_file);
   *get_filename(buf) = 0;

   if (file_select("Import binary data", buf, NULL)) {
      strcpy(import_file, buf);
      draw_wait(NULL);
      f = pack_fopen(buf, F_READ);
      if (!f) {
	 alert("Error opening file", NULL, NULL, "Oh dear", NULL, 13, 0);
	 return;
      }
      item->size = file_size(buf);
      if (item->size <= 0) {
	 pack_fclose(f);
	 alert("Error reading file", NULL, NULL, "Oh dear", NULL, 13, 0);
	 return;
      } 
      free(item->dat);
      item->dat = malloc(item->size);
      pack_fread(item->dat, item->size, f);
      pack_fclose(f);
      if (errno) {
	 alert("Error reading file", NULL, NULL, "Oh dear", NULL, 13, 0);
	 errno = 0;
      } 
   }
}



void import_sample(DATAITEM *item)
{
   char buf[80];

   draw_wait(NULL);
   strcpy(buf, import_file);
   *get_filename(buf) = 0;

   if (file_select("Import Sample", buf, "WAV")) {
      strcpy(import_file, buf);
      draw_wait(NULL);
      destroy_sample(item->dat);
      item->dat = load_sample(buf);
      if (!item->dat) {
	 item->dat = make_new_sample();
	 alert("Error reading WAV file", NULL, NULL, "Oh dear", NULL, 13, 0);
      }
   }
}



void import_midi(DATAITEM *item)
{
   char buf[80];

   draw_wait(NULL);
   strcpy(buf, import_file);
   *get_filename(buf) = 0;

   if (file_select("Import MIDI file", buf, "MID")) {
      strcpy(import_file, buf);
      draw_wait(NULL);
      destroy_midi(item->dat);
      item->dat = load_midi(buf);
      if (!item->dat) {
	 item->dat = make_new_midi();
	 alert("Error reading MIDI file", NULL, NULL, "Oh dear", NULL, 13, 0);
      }
   }
}



void find_character(int *x, int *y, int *w, int *h)
{
   /* look for top left corner of character */
   while ((getpixel(graphic, *x, *y) != 255) || 
	  (getpixel(graphic, *x+1, *y) != 255) ||
	  (getpixel(graphic, *x, *y+1) != 255) ||
	  (getpixel(graphic, *x+1, *y+1) == 255)) {
      (*x)++;
      if (*x >= graphic->w) {
	 *x = 0;
	 (*y)++;
	 if (*y >= graphic->h) {
	    *w = 0;
	    *h = 0;
	    return;
	 }
      }
   }

   /* look for right edge of character */
   *w = 0;
   while ((getpixel(graphic, *x+*w+1, *y) == 255) &&
	  (getpixel(graphic, *x+*w+1, *y+1) != 255) &&
	  (*x+*w+1 <= graphic->w))
      (*w)++;

   /* look for bottom edge of character */
   *h = 0;
   while ((getpixel(graphic, *x, *y+*h+1) == 255) &&
	  (getpixel(graphic, *x+1, *y+*h+1) != 255) &&
	  (*y+*h+1 <= graphic->h))
      (*h)++;
}



void grab_font(FONT *f, int *type)
{
   int x, y, w, h, c;

   *type = DAT_FONT_8x8;
   x = 0;
   y = 0;

   for (c=0; c<FONT_SIZE; c++) {

      find_character(&x, &y, &w, &h);
      destroy_bitmap(f->dat.dat_prop->dat[c]);

      if ((w <= 0) || (h <= 0)) {
	 w = 8;
	 h = 8;
      }

      f->dat.dat_prop->dat[c] = create_bitmap(w, h);
      clear(f->dat.dat_prop->dat[c]);
      blit(graphic, f->dat.dat_prop->dat[c], x+1, y+1, 0, 0, w, h);

      if ((w != 8) || (h != 8))
	 *type = DAT_FONT_PROP;

      x += w;
   }
}



void dotrect(int x1, int y1, int x2, int y2, int c1, int c2)
{
   int c;

   for (c=x1; c<x2; c++) {
      putpixel(screen, c, y1, (c&1) ? c1 : c2);
      putpixel(screen, c, y2, (c&1) ? c2 : c1);
   }

   for (c=y1; c<y2; c++) {
      putpixel(screen, x1, c, (c&1) ? c1 : c2);
      putpixel(screen, x2, c, (c&1) ? c2 : c1);
   }
}



void grab_bitmap(BITMAP *bmp)
{
   int ox, oy;
   int x, y;

   show_mouse(NULL);
   select_pallete(g_pallete);
   clear(screen);
   set_clip(screen, 0, 0, graphic->w-1, graphic->h-1);

   do {
   } while (mouse_b);

   ox = oy = -1;

   while (!mouse_b) {
      x = mouse_x & 0xFFF0;
      if (x + bmp->w > graphic->w)
	 x = (graphic->w - bmp->w) & 0xFFF0;
      if (x < 0)
	 x = 0;
      y = mouse_y & 0xFFF0;
      if (y + bmp->h > graphic->h)
	 y = (graphic->h - bmp->h) & 0xFFF0;
      if (y < 0)
	 y = 0;
      if ((x != ox) || (y != oy)) {
	 blit(graphic, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H);
	 dotrect(x-1, y-1, x+bmp->w, y+bmp->h, FG, BG);
	 ox = x;
	 oy = y;
      }
   }

   do {
   } while (mouse_b);

   set_clip(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);
   clear(bmp);
   blit(graphic, bmp, ox, oy, 0, 0, bmp->w, bmp->h);
   show_mouse(screen);
}



int grab_proc(int msg, DIALOG *d, int c)
{
   int ret, obj;
   int i;

   obj = SAFE_CURRENT_ITEM;

   if (msg == MSG_START) {
      if ((obj >= 0) && (data[obj].type == DAT_DATA)) {
	 d->dp = "Import Data";
	 d->key = 'i';
      }
      else if ((obj >= 0) && (data[obj].type == DAT_SAMPLE)) {
	 d->dp = "Import WAV";
	 d->key = 'i';
      }
      else if ((obj >= 0) && (data[obj].type == DAT_MIDI)) {
	 d->dp = "Import MIDI";
	 d->key = 'i';
      }
      else {
	 d->dp = "Grab";
	 d->key = 'g';
      }
   }

   ret = d_button_proc(msg, d, c);

   if (ret == D_CLOSE) {
      if (obj < 0) {
	 alert("You must create an object to contain",
	       "the data before you can grab it",
	       NULL, "OK", NULL, 13, 0);
	 return D_REDRAW;
      }

      if (data[obj].type == DAT_DATA) {
	 import_data(data+obj);
	 return D_REDRAW;
      }

      if (data[obj].type == DAT_SAMPLE) {
	 import_sample(data+obj);
	 return D_REDRAW;
      }

      if (data[obj].type == DAT_MIDI) {
	 import_midi(data+obj);
	 return D_REDRAW;
      }

      if (!graphic) {
	 alert("You must read in a PCX file",
	       "before you can grab data from it",
	       NULL, "OK", NULL, 13, 0);
	 return D_REDRAW;
      }

      if (data[obj].type == DAT_PALLETE) {
	 for (i=0; i<PAL_SIZE; i++)
	    ((RGB *)data[obj].dat)[i] = g_pallete[i];
	 alert("Pallete data grabbed from the PCX file",
	       NULL, NULL, "OK", NULL, 13, 0);
	 select_pallete(data[obj].dat);
	 return D_REDRAW;
      }

      if ((data[obj].type == DAT_FONT_8x8) || 
	  (data[obj].type == DAT_FONT_PROP)) {
	 grab_font(data[obj].dat, &data[obj].type);
	 alert("Font data grabbed from the PCX file",
	       NULL, NULL, "OK", NULL, 13, 0);
	 return D_REDRAW;
      }

      if ((data[obj].type == DAT_BITMAP) ||
	  (data[obj].type == DAT_RLE_SPRITE)) {
	 grab_bitmap(data[obj].dat);
	 return D_REDRAW;
      }
   }

   return ret;
}



int bmp_size_proc(int msg, DIALOG *d, int c)
{
   int obj;
   int x;
   int first;
   BITMAP *b;
   int w, h;

   obj = SAFE_CURRENT_ITEM;

   if ((obj < 0) || ((data[obj].type != DAT_BITMAP) && 
		     (data[obj].type != DAT_RLE_SPRITE))) {
      if (msg == MSG_DRAW)
	 rectfill(screen, d->x, d->y, d->x+d->w, d->y+d->h, d->bg);

      return D_O_K;
   }

   if (msg == MSG_CLICK) {
      first = TRUE;

      while (mouse_b) {
	 x = d->d1;

	 if (mouse_b == 2) {
	    if (key[KEY_LSHIFT] || key[KEY_RSHIFT])
	       d->d1 += 16;
	    else
	       d->d1++;
	 }
	 else if (mouse_b & 1) {
	    if (key[KEY_LSHIFT] || key[KEY_RSHIFT])
	       d->d1 -= 16;
	    else
	       d->d1--;
	 }

	 if (d->d1 < 1)
	    d->d1 = 1;
	 else 
	    if (d->d1 > 4096)
	       d->d1 = 4096;

	 if (x != d->d1) {
	    b = data[obj].dat;
	    if (d->d2) {
	       w = b->w;
	       h = d->d1;
	    }
	    else {
	       w = d->d1;
	       h = b->h;
	    }
	    data[obj].dat = create_bitmap(w, h);
	    clear(data[obj].dat);
	    blit(b, data[obj].dat, 0, 0, 0, 0, w, h);
	    destroy_bitmap(b);

	    show_mouse(NULL);
	    SEND_MESSAGE(d, MSG_DRAW, 0);
	    SEND_MESSAGE(main_dlg+DLG_VIEWER, MSG_DRAW, 0);
	    show_mouse(screen);

	    last_bmp_w = w;
	    last_bmp_h = h;

	    for (x=0; x < (first ? 32 : 2); x++) {
	       rest(10);
	       if (!mouse_b)
		  break;
	    }
	    first = FALSE;
	 }
      }

      return D_O_K;
   }

   if (msg == MSG_DRAW) {
      if (d->d2) {
	 d->d1 = ((BITMAP *)data[obj].dat)->h;
	 sprintf(d->dp, "h=%d", d->d1);
      }
      else {
	 d->d1 = ((BITMAP *)data[obj].dat)->w;
	 sprintf(d->dp, "w=%d", d->d1);
      }
   }

   return d_button_proc(msg, d, c);
}



int rle_flag_proc(int msg, DIALOG *d, int c)
{
   int obj;
   int ret;
   int new_type;

   obj = SAFE_CURRENT_ITEM;

   if ((obj < 0) || ((data[obj].type != DAT_BITMAP) && 
		     (data[obj].type != DAT_RLE_SPRITE))) {
      if (msg == MSG_DRAW)
	 rectfill(screen, d->x, d->y, d->x+d->w, d->y+d->h, d->bg);

      return D_O_K;
   }

   if (msg == MSG_START)
      if (data[obj].type == DAT_BITMAP)
	 d->flags &= ~D_SELECTED;
      else
	 d->flags |= D_SELECTED;

   ret = d_check_proc(msg, d, c);

   if (d->flags & D_SELECTED)
      new_type = DAT_RLE_SPRITE;
   else
      new_type = DAT_BITMAP;

   if (new_type != data[obj].type) {
      data[obj].type = new_type;
      if (new_type == DAT_BITMAP)
	 memcpy(data[obj].type_str+1, "bmp", 3);
      else
	 memcpy(data[obj].type_str+1, "rle", 3);
      show_mouse(NULL);
      SEND_MESSAGE(main_dlg+DLG_LIST, MSG_DRAW, 0);
      SEND_MESSAGE(main_dlg+DLG_VIEWER, MSG_DRAW, 0);
      show_mouse(screen);
   }

   return ret;
}



void main(int argc, char *argv[])
{
   int c;

   allegro_init();
   install_keyboard();
   install_mouse();
   install_timer();

   if (set_gfx_mode(GFX_AUTODETECT, 640, 480, 0, 0) != 0) {
      allegro_exit();
      printf("Error setting graphics mode\n%s\n\n", allegro_error);
      exit(1);
   }

   if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, argv[0]) != 0) {
      allegro_exit();
      printf("Error initialising sound\n%s\n\n", allegro_error);
      exit(1);
   }

   gui_fg_color = 255;
   gui_bg_color = 16;
   set_pallete(desktop_pallete);
   for (c=0; c<PAL_SIZE; c++)
      selected_pallete[c] = g_pallete[c] = desktop_pallete[c];

   if (argc > 1) {
      strcpy(data_file, argv[1]);
      strupr(data_file);
      main_dlg[DLG_FILENAME].dp = get_filename(data_file);
      load();
   }

   do_dialog(main_dlg, DLG_LIST);

   allegro_exit();

   destroy_data();

   if (graphic)
      destroy_bitmap(graphic);

   if (my_mouse_pointer)
      destroy_bitmap(my_mouse_pointer);

   exit(0);
}

