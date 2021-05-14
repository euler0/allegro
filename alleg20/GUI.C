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
 *      GUI routines.
 *
 *      See readme.txt for copyright information.
 */


#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dir.h>

#include "allegro.h"
#include "internal.h"


/* if set, the input focus follows the mouse pointer */
int gui_mouse_focus = TRUE;

/* colors for the standard dialogs (alerts, file selector, etc) */
int gui_fg_color = 255;
int gui_bg_color = 0;


typedef char *(*getfuncptr)(int, int *);



/* dotted_rect:
 *  Draws a dotted rectangle, for showing an object has the input focus.
 */
static void dotted_rect(BITMAP *bmp, int x1, int y1, int x2, int y2, int color)
{
   int c;

   for (c=x1; c<x2; c+=2) {
      putpixel(bmp, c, y1, color);
      putpixel(bmp, c, y2, color);
   }

   for (c=y1; c<y2; c+=2) {
      putpixel(bmp, x1, c, color);
      putpixel(bmp, x2, c, color);
   }
}



/* d_clear_proc:
 *  Simple dialog procedure which just clears the screen. Useful as the
 *  first object in a dialog.
 */
int d_clear_proc(int msg, DIALOG *d, int c)
{
   if (msg == MSG_DRAW) {
      set_clip(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);
      clear_to_color(screen, d->bg);
   }

   return D_O_K;
}



/* d_box_proc:
 *  Simple dialog procedure: just draws a box.
 */
int d_box_proc(int msg, DIALOG *d, int c)
{
   if (msg==MSG_DRAW) {
      rectfill(screen, d->x+1, d->y+1, d->x+d->w-1, d->y+d->h-1, d->bg);
      rect(screen, d->x, d->y, d->x+d->w, d->y+d->h, d->fg);
   }

   return D_O_K;
}



/* d_shadow_box_proc:
 *  Simple dialog procedure: draws a box with a shadow.
 */
int d_shadow_box_proc(int msg, DIALOG *d, int c)
{
   if (msg==MSG_DRAW) {
      rectfill(screen, d->x+1, d->y+1, d->x+d->w-2, d->y+d->h-2, d->bg);
      rect(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, d->fg);
      vline(screen, d->x+d->w, d->y+1, d->y+d->h, d->fg);
      hline(screen, d->x+1, d->y+d->h, d->x+d->w, d->fg);
   }

   return D_O_K;
}



/* d_bitmap_proc:
 *  Simple dialog procedure: draws the bitmap which is pointed to by dp.
 */
int d_bitmap_proc(int msg, DIALOG *d, int c)
{
   BITMAP *b = (BITMAP *)d->dp;

   if (msg==MSG_DRAW)
      blit(b, screen, 0, 0, d->x, d->y, d->w, d->h);

   return D_O_K;
}



/* d_text_proc:
 *  Simple dialog procedure: draws the text string which is pointed to by dp.
 */
int d_text_proc(int msg, DIALOG *d, int c)
{
   if (msg==MSG_DRAW) {
      text_mode(d->bg);
      textout(screen, font, d->dp, d->x, d->y, d->fg);
   }

   return D_O_K;
}



/* d_ctext_proc:
 *  Simple dialog procedure: draws the text string which is pointed to by dp,
 *  centering it around the object's x coordinate.
 */
int d_ctext_proc(int msg, DIALOG *d, int c)
{
   if (msg==MSG_DRAW) {
      text_mode(d->bg);
      textout_centre(screen, font, d->dp, d->x, d->y, d->fg);
   }

   return D_O_K;
}



/* d_button_proc:
 *  A button object (the dp field points to the text string). This object
 *  can be selected by clicking on it with the mouse or by pressing its 
 *  keyboard shortcut. If the D_EXIT flag is set, selecting it will close 
 *  the dialog, otherwise it will toggle on and off.
 */
int d_button_proc(int msg, DIALOG *d, int c)
{
   int state1, state2;
   int swap;
   int g;

   switch (msg) {

      case MSG_DRAW:
	 if (d->flags & D_SELECTED) {
	    g = 1;
	    state1 = d->bg;
	    state2 = d->fg;
	 }
	 else {
	    g = 0; 
	    state1 = d->fg;
	    state2 = d->bg;
	 }

	 rectfill(screen, d->x+1+g, d->y+1+g, d->x+d->w-2+g, d->y+d->h-2+g, state2);
	 rect(screen, d->x+g, d->y+g, d->x+d->w-1+g, d->y+d->h-1+g, state1);
	 text_mode(state2);
	 textout_centre(screen, font, d->dp, d->x+d->w/2+g, d->y+d->h/2-4+g, state1);

	 if (d->flags & D_SELECTED) {
	    vline(screen, d->x, d->y, d->y+d->h-1, d->bg);
	    hline(screen, d->x, d->y, d->x+d->w-1, d->bg);
	 }
	 else {
	    vline(screen, d->x+d->w, d->y+1, d->y+d->h-1, d->fg);
	    hline(screen, d->x+1, d->y+d->h, d->x+d->w, d->fg);
	 }
	 if ((d->flags & D_GOTFOCUS) && 
	     (!(d->flags & D_SELECTED) || !(d->flags & D_EXIT)))
	    dotted_rect(screen, d->x+1+g, d->y+1+g, d->x+d->w-2+g, d->y+d->h-2+g, state1);
	 break;

      case MSG_WANTFOCUS:
	 return D_WANTFOCUS;

      case MSG_KEY:
	 /* close dialog? */
	 if (d->flags & D_EXIT)
	    return D_CLOSE;

	 /* or just toggle */
	 d->flags ^= D_SELECTED;
	 show_mouse(NULL);
	 SEND_MESSAGE(d, MSG_DRAW, 0);
	 show_mouse(screen);
	 break;

      case MSG_CLICK:
	 /* what state was the button originally in? */
	 state1 = d->flags & D_SELECTED;
	 if (d->flags & D_EXIT)
	    swap = FALSE;
	 else
	    swap = state1;

	 /* track the mouse until it is released */
	 while (mouse_b) {
	    state2 = ((mouse_x >= d->x) && (mouse_y >= d->y) &&
		     (mouse_x <= d->x + d->w) && (mouse_y <= d->y + d->h));
	    if (swap)
	       state2 = !state2;

	    /* redraw? */
	    if (((state1) && (!state2)) || ((state2) && (!state1))) {
	       d->flags ^= D_SELECTED;
	       state1 = d->flags & D_SELECTED;
	       show_mouse(NULL);
	       SEND_MESSAGE(d, MSG_DRAW, 0);
	       show_mouse(screen);
	    }
	 }

	 /* should we close the dialog? */
	 if ((d->flags & D_SELECTED) && (d->flags & D_EXIT)) {
	    d->flags ^= D_SELECTED;
	    return D_CLOSE;
	 }
	 break; 
   }

   return D_O_K;
}



/* d_check_proc:
 *  Hehe. Who needs C++ after all? This is derived from d_button_proc, 
 *  but overrides the drawing routine to provide a check box.
 */
int d_check_proc(int msg, DIALOG *d, int c)
{
   int x;

   if (msg==MSG_DRAW) {
      text_mode(d->bg);
      textout(screen, font, d->dp, d->x, d->y+(d->h-8)/2, d->fg);
      x = d->x + strlen(d->dp)*8 + 4;
      rectfill(screen, x+1, d->y+1, x+d->h-1, d->y+d->h-1, d->bg);
      rect(screen, x, d->y, x+d->h, d->y+d->h, d->fg);
      if (d->flags & D_SELECTED) {
	 line(screen, x, d->y, x+d->h, d->y+d->h, d->fg);
	 line(screen, x, d->y+d->h, x+d->h, d->y, d->fg); 
      }
      if (d->flags & D_GOTFOCUS)
	 dotted_rect(screen, x+1, d->y+1, x+d->h-1, d->y+d->h-1, d->fg);
      return D_O_K;
   } 

   return d_button_proc(msg, d, 0);
}



/* d_edit_proc:
 *  An editable text object (the dp field points to the string). When it
 *  has the input focus (obtained by clicking on it with the mouse), text
 *  can be typed into this object. The d1 field specifies the maximum
 *  number of characters that it will accept, and d2 is the text cursor 
 *  position within the string.
 */
int d_edit_proc(int msg, DIALOG *d, int c)
{
   int p, f, l, x, w;
   char *s;
   char buf[2];

   s = d->dp;
   l = strlen(s);
   if (d->d2 > l)
      d->d2 = l;

   switch (msg) {

      case MSG_START:
	 d->d2 = l;
	 break;

      case MSG_DRAW:
	 w = d->w / 8;
	 x = d->x;
	 p = 0;

	 while (d->d2 >= p+w-1)
	    p += w-2;

	 if (p > 0) {
	    text_mode(d->fg);
	    textout(screen, font, "<", x, d->y, d->bg);
	    p++;
	    x += 8;
	    w--;
	 }

	 while ((p <= l) && (w > 0)) { 
	    buf[0] = s[p] ? s[p] : ' ';
	    buf[1] = 0;
	    f = ((p == d->d2) && (d->flags & D_GOTFOCUS));
	    if ((w==1) && (p<l)) {
	       f = TRUE;
	       buf[0] = '>';
	    }
	    text_mode(f ? d->fg : d->bg);
	    textout(screen, font, buf, x, d->y, f ? d->bg : d->fg);
	    p++;
	    x += 8;
	    w--;
	 }
	 rectfill(screen, x, d->y, d->x+d->w, d->y+7, d->bg);
	 break;

      case MSG_CLICK:
	 w = d->w / 8;
	 p = 0;
	 while (d->d2 >= p+w-1)
	    p += w-2;
	 d->d2 = p + (mouse_x - d->x) / 8;
	 d->d2 = MID(0, d->d2, l);
	 show_mouse(NULL);
	 SEND_MESSAGE(d, MSG_DRAW, 0);
	 show_mouse(screen);
	 break;

      case MSG_WANTFOCUS:
      case MSG_LOSTFOCUS:
	 return D_WANTFOCUS;

      case MSG_CHAR:
	 if ((c >> 8) == KEY_LEFT) {
	    if (d->d2 > 0)
	       d->d2--;
	 }
	 else if ((c >> 8) == KEY_RIGHT) {
	    if (d->d2 < l)
	       d->d2++;
	 }
	 else if ((c >> 8) == KEY_HOME) {
	    d->d2 = 0;
	 }
	 else if ((c >> 8) == KEY_END) {
	    d->d2 = l;
	 }
	 else if ((c >> 8) == KEY_DEL) {
	    if (d->d2 < l)
	       for (p=d->d2; s[p]; p++)
		  s[p] = s[p+1];
	 }
	 else if ((c >> 8) == KEY_BACKSPACE) {
	    if (d->d2 > 0) {
	       d->d2--;
	       for (p=d->d2; s[p]; p++)
		  s[p] = s[p+1];
	    } 
	 }
	 else {
	    c &= 0xff;
	    if ((c >= 32) && (c <= 126)) {
	       if (l < d->d1) {
		  while (l >= d->d2) {
		     s[l+1] = s[l];
		     l--;
		  }
		  s[d->d2] = c;
		  d->d2++;
	       }
	    }
	    else
	       return D_O_K;
	 }

	 /* if we changed something, better redraw... */ 
	 show_mouse(NULL);
	 SEND_MESSAGE(d, MSG_DRAW, 0);
	 show_mouse(screen);
	 return D_USED_CHAR;
   }

   return D_O_K;
}



/* draw_listbox:
 *  Helper function to draw a listbox object.
 */
static void draw_listbox(DIALOG *d)
{
   int width, height;
   int listsize;
   int i; 
   int fg, bg;
   char *s;
   int x, y;
   int len;
   char store;

   /* draw frame */
   rect(screen, d->x, d->y, d->x+d->w, d->y+d->h, d->fg);
   if (d->flags & D_GOTFOCUS)
      dotted_rect(screen, d->x+1, d->y+1, d->x+d->w-1, d->y+d->h-1, d->fg);
   else
      rect(screen, d->x+1, d->y+1, d->x+d->w-1, d->y+d->h-1, d->bg);

   (*(getfuncptr)d->dp)(-1, &listsize);
   height = (d->h-3) / 8;
   width = (d->w-3) / 8;

   /* draw box contents */
   for (i=0; i<height; i++) {
      if (d->d2+i < listsize) {
	 if (d->d2+i == d->d1) { 
	    fg = d->bg;
	    bg = d->fg;
	 }
	 else {
	    fg = d->fg;
	    bg = d->bg;
	 }
	 if ((i == 0) && (d->d2 > 0))
	    s = "^ ^ ^ ^";
	 else if ((i == height-1) && (d->d2+height < listsize))
	    s = "v v v v";
	 else
	    s = (*(getfuncptr)d->dp)(i+d->d2, NULL);
	 x = d->x + 2;
	 y = d->y + 2 + i*8;
	 text_mode(bg);
	 rectfill(screen, x, y, x+7, y+7, bg); 
	 x += 8;
	 len = strlen(s); 
	 if (len >= width)
	    len = width-1;
	 store = s[len];
	 s[len] = 0;
	 textout(screen, font, s, x, y, fg); 
	 s[len] = store;
	 x += len*8;
	 if (x <= d->x+d->w-2) 
	    rectfill(screen, x, y, d->x+d->w-2, y+7, bg);
      }
      else
	 rectfill(screen, d->x+2,  d->y+2+i*8, d->x+d->w-2, d->y+9+i*8, d->bg);
   }
}



/* scroll_listbox:
 *  Helper function to scroll through a listbox.
 */
static void scroll_listbox(DIALOG *d, int listsize)
{
   if (!listsize) {
      d->d1 = d->d2 = 0;
      return;
   }

   /* check selected item */
   if (d->d1 < 0)
      d->d1 = 0;
   else
      if (d->d1 >= listsize)
	 d->d1 = listsize -1;

   /* check scroll position */
   if (d->d2 >= d->d1) {
      if (d->d1 <= 0)
	 d->d2 = 0;
      else
	 d->d2 = d->d1 - 1;
   }
   else {
      while ((d->d2 + (d->h-3)/8 - ((d->d1 == listsize-1) ? 1 : 2)) < d->d1)
	 d->d2++;
   }
}



/* d_list_proc:
 *  A list box object. The dp field points to a function which it will call
 *  to obtain information about the list. This should follow the form:
 *     char *<list_func_name> (int index, int *list_size);
 *  If index is zero or positive, the function should return a pointer to
 *  the string which is to be displayed at position index in the list. If
 *  index is  negative, it should return null and list_size should be set
 *  to the number of items in the list. The list box object will allow the
 *  user to scroll through the list and to select items list by clicking
 *  on them, and if it has the input focus also by using the arrow keys. If 
 *  the D_EXIT flag is set, double clicking on a list item will cause it to 
 *  close the dialog. The index of the selected item is held in the d1 
 *  field, and d2 is used to store how far it has scrolled through the list.
 */
int d_list_proc(int msg, DIALOG *d, int c)
{
   int listsize;
   int i;
   int top, bottom;

   switch (msg) {

      case MSG_START:
	 (*(getfuncptr)d->dp)(-1, &listsize);
	 scroll_listbox(d, listsize);
	 break;

      case MSG_DRAW:
	 draw_listbox(d);
	 break;

      case MSG_CLICK:
	 (*(getfuncptr)d->dp)(-1,&listsize);
	 if (listsize) {
	    while (mouse_b) {
	       i = ((MID(d->y, mouse_y, d->y+d->h-1) - d->y - 2) / 8) + d->d2;
	       if (i < 0)
		  i = 0;
	       else {
		  if (i >= listsize)
		     i = listsize - 1;
	       }
	       if (i != d->d1) {
		  d->d1 = i;
		  i = d->d2;
		  scroll_listbox(d, listsize);
		  show_mouse(NULL);
		  SEND_MESSAGE(d, MSG_DRAW, 0);
		  show_mouse(screen);
		  if (i != d->d2)
		     rest(MID(10, 128-d->h, 100));
	       }
	    }
	 }
	 break;

      case MSG_DCLICK:
	 if (d->flags & D_EXIT) {
	    (*(getfuncptr)d->dp)(-1, &listsize);
	    if (listsize) {
	       i = d->d1;
	       SEND_MESSAGE(d, MSG_CLICK, 0);
	       if (i == d->d1) 
		  return D_CLOSE;
	    }
	 }
	 break;

      case MSG_KEY:
	 (*(getfuncptr)d->dp)(-1, &listsize);
	 if ((listsize) && (d->flags & D_EXIT))
	    return D_CLOSE;
	 break;

      case MSG_WANTFOCUS:
	 return D_WANTFOCUS;

      case MSG_CHAR:
	 (*(getfuncptr)d->dp)(-1,&listsize);
	 if (listsize) {
	    c >>= 8;
	    if (d->d2 > 0)
	       top = d->d2+1;
	    else
	       top = 0;
	    if (d->d2+((d->h-3)/8) < listsize) 
	       bottom = d->d2+(d->h-3)/8;
	    else
	       bottom = listsize-1;
	    if (c == KEY_UP)
	       d->d1--;
	    else if (c == KEY_DOWN)
	       d->d1++;
	    else if (c == KEY_HOME)
	       d->d1 = 0;
	    else if (c == KEY_END)
	       d->d1 = listsize-1;
	    else if (c == KEY_PGUP) {
	       if (d->d1 > top)
		  d->d1 = top;
	       else
		  d->d1 -= (bottom-top);
	    }
	    else if (c == KEY_PGDN) {
	       if (d->d1 < bottom)
		  d->d1 = bottom;
	       else
		  d->d1 += (bottom-top);
	    } 
	    else 
	       return D_O_K;

	    /* if we changed something, better redraw... */ 
	    scroll_listbox(d, listsize);
	    show_mouse(NULL);
	    SEND_MESSAGE(d, MSG_DRAW, 0);
	    show_mouse(screen); 
	    return D_USED_CHAR;
	 }
	 break;
   }

   return D_O_K;
}



/* Checking for double clicks is complicated. The user could release the
 * mouse button at almost any point, and I might miss it if I am doing some
 * other processing at the same time (eg. sending the single-click message).
 * To get around this I install a timer routine to do the checking for me,
 * so it will notice double clicks whenever they happen.
 */

static volatile int dclick_status, dclick_time;
static int dclick_install_count = 0;

#define DCLICK_START      0
#define DCLICK_RELEASE    1
#define DCLICK_AGAIN      2
#define DCLICK_NOT        3


/* dclick_check:
 *  Double click checking user timer routine.
 */
static void dclick_check()
{
   if (dclick_status==DCLICK_START) {              /* first click... */
      if (!mouse_b) {
	 dclick_status = DCLICK_RELEASE;           /* aah! released first */
	 dclick_time = 0;
	 return;
      }
   }
   else if (dclick_status==DCLICK_RELEASE) {       /* wait for second click */
      if (mouse_b) {
	 dclick_status = DCLICK_AGAIN;             /* yes! the second click */
	 dclick_time = 0;
	 return;
      }
   }
   else
      return;

   /* timeout? */
   if (dclick_time++ > 10)
      dclick_status = DCLICK_NOT;
}

static END_OF_FUNCTION(dclick_check);



/* centre_dialog:
 *  Moves all the objects in a dialog so that the dialog is centered in
 *  the screen.
 */
void centre_dialog(DIALOG *dialog)
{
   int min_x = SCREEN_W;
   int min_y = SCREEN_H;
   int max_x = 0;
   int max_y = 0;
   int xc, yc;
   int c;

   /* find the extents of the dialog */ 
   for (c=0; dialog[c].proc; c++) {
      if (dialog[c].x < min_x)
	 min_x = dialog[c].x;

      if (dialog[c].y < min_y)
	 min_y = dialog[c].y;

      if (dialog[c].x + dialog[c].w > max_x)
	 max_x = dialog[c].x + dialog[c].w;

      if (dialog[c].y + dialog[c].h > max_y)
	 max_y = dialog[c].y + dialog[c].h;
   }

   /* how much to move by? */
   xc = (SCREEN_W - (max_x - min_x)) / 2 - min_x;
   yc = (SCREEN_H - (max_y - min_y)) / 2 - min_y;

   /* move it */
   for (c=0; dialog[c].proc; c++) {
      dialog[c].x += xc;
      dialog[c].y += yc;
   }
}



/* set_dialog_color:
 *  Sets the foreground and background colors of all the objects in a dialog.
 */
void set_dialog_color(DIALOG *dialog, int fg, int bg)
{
   int c;

   for (c=0; dialog[c].proc; c++) {
      dialog[c].fg = fg;
      dialog[c].bg = bg;
   }
}



/* dialog_message:
 *  Sends a message to all the objects in a dialog. If any of the objects
 *  return values other than D_O_K, returns the value and sets obj to the 
 *  object which produced it.
 */
int dialog_message(DIALOG *dialog, int msg, int c, int *obj)
{
   int count;
   int res;
   int r;

   if (msg == MSG_DRAW)
      show_mouse(NULL);

   res = D_O_K;

   for (count=0; dialog[count].proc; count++) { 
      if (!(dialog[count].flags & D_HIDDEN)) {
	 r = SEND_MESSAGE(dialog+count, msg, c);
	 if (r != D_O_K) {
	    res |= r;
	    *obj = count;
	 }
      }
   }

   if (msg == MSG_DRAW)
      show_mouse(screen);

   return res;
}



/* find_mouse_object:
 *  Finds which object the mouse is on top of.
 */
static int find_mouse_object(DIALOG *d)
{
   /* finds which object the mouse is on top of */

   int mouse_object = -1;
   int c;

   for (c=0; d[c].proc; c++)
      if ((mouse_x >= d[c].x) && (mouse_y >= d[c].y) &&
	  (mouse_x < d[c].x + d[c].w) && (mouse_y < d[c].y + d[c].h) &&
	  (!(d[c].flags & D_HIDDEN)))
	 mouse_object = c;

   return mouse_object;
}



/* offer_focus:
 *  Offers the input focus to a particular object.
 */
static int offer_focus(DIALOG *d, int obj, int *focus_obj, int force)
{
   int res = D_O_K;

   if ((obj == *focus_obj) || ((obj >= 0) && (d[obj].flags & D_HIDDEN)))
      return D_O_K;

   /* check if object wants the focus */
   if (obj >= 0) {
      res = SEND_MESSAGE(d+obj, MSG_WANTFOCUS, 0);
      if (res & D_WANTFOCUS)
	 res ^= D_WANTFOCUS;
      else
	 obj = -1;
   }

   if ((obj > 0) || (force)) {
      /* take focus away from old object */
      if (*focus_obj >= 0) {
	 res |= SEND_MESSAGE(d+*focus_obj, MSG_LOSTFOCUS, 0);
	 if (res & D_WANTFOCUS) {
	    if (obj < 0)
	       return D_O_K;
	    else
	       res &= ~D_WANTFOCUS;
	 }
	 d[*focus_obj].flags &= ~D_GOTFOCUS;
	 show_mouse(NULL);
	 res |= SEND_MESSAGE(d+*focus_obj, MSG_DRAW, 0);
	 show_mouse(screen);
      }

      *focus_obj = obj;

      /* give focus to new object */
      if (obj >= 0) {
	 show_mouse(NULL);
	 d[obj].flags |= D_GOTFOCUS;
	 res |= SEND_MESSAGE(d+obj, MSG_GOTFOCUS, 0);
	 res |= SEND_MESSAGE(d+obj, MSG_DRAW, 0);
	 show_mouse(screen);
      }
   }

   return res;
}



typedef struct DLG_SORT_STRUCT
{
   int i;
   DIALOG *d;
} DLG_SORT_STRUCT;



/* sort routine for right arrow movement through a dialog */
static int cmp_x_y(const void *e1, const void *e2)
{
   DIALOG *d1 = ((DLG_SORT_STRUCT *)e1)->d;
   DIALOG *d2 = ((DLG_SORT_STRUCT *)e2)->d;

   return ((d1->y - d2->y) << 16) + (d1->x - d2->x);
}



/* sort routine for down arrow movement through a dialog */
static int cmp_y_x(const void *e1, const void *e2)
{
   DIALOG *d1 = ((DLG_SORT_STRUCT *)e1)->d;
   DIALOG *d2 = ((DLG_SORT_STRUCT *)e2)->d;

   return (d1->y - d2->y) + ((d1->x - d2->x) << 16);
}



/* sort routine for left arrow movement through a dialog */
static int cmp_x_y_minus(const void *e1, const void *e2)
{
   DIALOG *d1 = ((DLG_SORT_STRUCT *)e1)->d;
   DIALOG *d2 = ((DLG_SORT_STRUCT *)e2)->d;

   return ((d2->y - d1->y) << 16) + (d2->x - d1->x);
}



/* sort routine for up arrow movement through a dialog */
static int cmp_y_x_minus(const void *e1, const void *e2)
{
   DIALOG *d1 = ((DLG_SORT_STRUCT *)e1)->d;
   DIALOG *d2 = ((DLG_SORT_STRUCT *)e2)->d;

   return (d2->y - d1->y) + ((d2->x - d1->x) << 16);
}



#define MAX_OBJECTS     512


/* move_focus:
 *  Handles arrow key and tab movement through a dialog, deciding which
 *  object should be given the input focus.
 */
static int move_focus(DIALOG *d, long ch, int *focus_obj)
{
   int (*cmp)(const void *e1, const void *e2);
   DLG_SORT_STRUCT object[MAX_OBJECTS];
   int count;
   int start;
   int pos;
   int res = D_O_K;
   int fobj = *focus_obj;

   /* choose a comparison function */ 
   switch (ch >> 8) {
      case KEY_TAB:     /* same as right arrow */
      case KEY_RIGHT:   cmp = cmp_x_y;          break;
      case KEY_DOWN:    cmp = cmp_y_x;          break;
      case KEY_LEFT:    cmp = cmp_x_y_minus;    break;
      case KEY_UP:      cmp = cmp_y_x_minus;    break;
      default: return D_O_K;
   }

   /* fill and sort the temporary table */ 
   for (count=0; (d[count].proc) && (count < MAX_OBJECTS); count++) {
      object[count].i = count;
      object[count].d = d+count;
   }

   if (count <= 1)
      return D_O_K;

   qsort(object, count, sizeof(DLG_SORT_STRUCT), cmp);

   /* find the starting point */
   for (pos=count-1; pos>=0; pos--)
      if (object[pos].i == fobj)
	 break;

   start = pos;

   /* find an object to give the focus to */
   do {
      pos++;
      if (pos >= count)
	 pos = 0;

      /* gone all the way round? noone wants the focus then... */
      if (pos == start)
	 return res;

      res |= offer_focus(d, object[pos].i, focus_obj, FALSE);

   } while (fobj == *focus_obj);

   return res;
}



#define MESSAGE(i, msg, c) {                 \
   r = SEND_MESSAGE(dialog+i, msg, c);       \
   if (r != D_O_K) {                         \
      res |= r;                              \
      obj = i;                               \
   }                                         \
}



/* do_dialog:
 *  The basic dialog manager. The list of dialog objects should be
 *  terminated by one with a null dialog procedure. Returns the index of 
 *  the object which caused it to exit.
 */
int do_dialog(DIALOG *dialog, int focus_obj)
{
   int res = D_REDRAW;
   int obj;
   int mouse_obj;
   int c, r;
   long ch;
   BITMAP *old_mouse_bmp = _mouse_screen;

   /* set up dclick checking code */
   if (dclick_install_count <= 0) {
      LOCK_VARIABLE(dclick_status);
      LOCK_VARIABLE(dclick_time);
      LOCK_FUNCTION(dclick_check);
      install_int(dclick_check, 20);
      dclick_install_count = 1;
   }
   else
      dclick_install_count++;

   /* initialise the dialog */
   set_clip(screen, 0, 0, SCREEN_W-1, SCREEN_H-1);
   res |= dialog_message(dialog, MSG_START, 0, &obj);

   mouse_obj = find_mouse_object(dialog);
   if (mouse_obj >= 0)
      dialog[mouse_obj].flags |= D_GOTMOUSE;

   for (c=0; dialog[c].proc; c++)
      if (c == focus_obj)
	 dialog[c].flags |= D_GOTFOCUS;
      else
	 dialog[c].flags &= ~D_GOTFOCUS;

   /* while dialog is active */ 
   while (!(res & D_CLOSE)) {

      /* need to draw it? */
      if (res & D_REDRAW) {
	 res ^= D_REDRAW;
	 res |= dialog_message(dialog, MSG_DRAW, 0, &obj);
      }

      /* need to give the input focus to someone? */
      if (res & D_WANTFOCUS) {
	 res ^= D_WANTFOCUS;
	 res |= offer_focus(dialog, obj, &focus_obj, FALSE);
      }

      /* has mouse object changed? */
      c = find_mouse_object(dialog);
      if (c != mouse_obj) {
	 if (mouse_obj >= 0) {
	    dialog[mouse_obj].flags &= ~D_GOTMOUSE;
	    MESSAGE(mouse_obj, MSG_LOSTMOUSE, 0);
	 }
	 if (c >= 0) {
	    dialog[c].flags |= D_GOTMOUSE;
	    MESSAGE(c, MSG_GOTMOUSE, 0);
	 }
	 mouse_obj = c; 

	 /* move the input focus as well? */
	 if ((gui_mouse_focus) && (mouse_obj != focus_obj))
	    res |= offer_focus(dialog, mouse_obj, &focus_obj, TRUE);
      }

      /* deal with mouse button clicks */
      if (mouse_b) {
	 res |= offer_focus(dialog, mouse_obj, &focus_obj, FALSE);

	 if (mouse_obj >= 0) {
	    dclick_time = 0;
	    dclick_status = DCLICK_START;

	    /* send click message */
	    MESSAGE(mouse_obj, MSG_CLICK, 0);

	    if (res==D_O_K) {
	       do {
	       } while ((dclick_status != DCLICK_AGAIN) &&
			(dclick_status != DCLICK_NOT));

	       /* double click! */
	       if ((dclick_status==DCLICK_AGAIN) &&
		   (mouse_x >= dialog[mouse_obj].x) && 
		   (mouse_y >= dialog[mouse_obj].y) &&
		   (mouse_x <= dialog[mouse_obj].x + dialog[mouse_obj].w) &&
		   (mouse_y <= dialog[mouse_obj].y + dialog[mouse_obj].h)) {
		  MESSAGE(mouse_obj, MSG_DCLICK, 0);
	       }
	    }
	 }
	 continue;
      }

      /* deal with keyboard input */
      if (keypressed()) {
	 ch = readkey();

	 /* let object deal with the key? */
	 if (focus_obj >= 0) {
	    MESSAGE(focus_obj, MSG_CHAR, ch);
	    if (res & D_USED_CHAR) {
	       res ^= D_USED_CHAR;
	       continue;
	    }
	 }

	 /* keyboard shortcut? */
	 if (ch & 0xff) {
	    for (c=0; dialog[c].proc; c++) {
	       if ((dialog[c].key == (ch & 0xff)) && 
		   (!(dialog[c].flags & D_HIDDEN))) {
		  MESSAGE(c, MSG_KEY, ch);
		  ch = 0;
		  break;
	       }
	    }
	    if (!ch)
	       continue;
	 }

	 /* pass <CR> or <SPACE> to selected object? */
	 if ((((ch & 0xff) == 10) || ((ch & 0xff) == 13) || 
	      ((ch & 0xff) == 32)) && (focus_obj >= 0)) {
	    MESSAGE(focus_obj, MSG_KEY, ch);
	    continue;
	 }

	 /* ESC closes dialog? */
	 if ((ch & 0xff) == 27) {
	    res |= D_CLOSE;
	    obj = -1;
	    continue;
	 }

	 /* move focus around the dialog? */
	 res |= move_focus(dialog, ch, &focus_obj);
      }

      /* send idle messages */
      res |= dialog_message(dialog, MSG_IDLE, 0, &obj);
   }

   /* send the finish messages */
   dialog_message(dialog, MSG_END, 0, &obj);

   /* remove the double click handler */
   dclick_install_count--;
   if (dclick_install_count <= 0)
      remove_int(dclick_check);

   if (mouse_obj >= 0)
      dialog[mouse_obj].flags &= ~D_GOTMOUSE;

   if (focus_obj >= 0)
      dialog[focus_obj].flags &= ~D_GOTFOCUS;

   show_mouse(old_mouse_bmp);
   return obj;
}



/* popup_dialog:
 *  Like do_dialog(), but it stores the data on the screen before drawing
 *  the dialog and restores it when the dialog is closed. The screen area
 *  to be stored is calculated from the dimensions of the first object in
 *  the dialog, so all the other objects should lie within this one.
 */
int popup_dialog(DIALOG *dialog, int focus_obj)
{
   BITMAP *bmp;
   int ret;
   BITMAP *old_mouse_bmp = _mouse_screen;

   bmp = create_bitmap(dialog->w+1, dialog->h+1); 

   if (bmp) {
      show_mouse(NULL);
      blit(screen, bmp, dialog->x, dialog->y, 0, 0, dialog->w+1, dialog->h+1);
   }
   else
      errno = ENOMEM;

   ret = do_dialog(dialog, focus_obj);

   if (bmp) {
      show_mouse(NULL);
      blit(bmp, screen, 0, 0, dialog->x, dialog->y, dialog->w+1, dialog->h+1);
      destroy_bitmap(bmp);
   }

   show_mouse(old_mouse_bmp);
   return ret;
}



static DIALOG alert_dialog[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp) */
   { d_shadow_box_proc, 0,    0,    0,    64,   0,    0,    0,    0,       0,    0,    NULL },
   { d_ctext_proc,      0,    8,    1,    8,    0,    0,    0,    0,       0,    0,    NULL },
   { d_ctext_proc,      0,    16,   1,    8,    0,    0,    0,    0,       0,    0,    NULL },
   { d_ctext_proc,      0,    24,   1,    8,    0,    0,    0,    0,       0,    0,    NULL },
   { d_button_proc,     0,    40,   0,    16,   0,    0,    0,    D_EXIT,  0,    0,    NULL },
   { d_button_proc,     0,    40,   0,    16,   0,    0,    0,    D_EXIT,  0,    0,    NULL },
   { NULL }
};

#define A_S1  1
#define A_S2  2
#define A_S3  3
#define A_B1  4
#define A_B2  5



/* alert:
 *  Displays a simple alert box, containing three lines of text (s1-s3),
 *  and with either one or two buttons. The text for these buttons is passed
 *  in b1 and b2 (b2 may be null), and the keyboard shortcuts in c1 and c2.
 *  Returns 1 or 2 depending on which button was selected.
 */
int alert(char *s1, char *s2, char *s3, char *b1, char *b2, int c1, int c2)
{
   int maxlen = 0;
   int len1, len2;

   alert_dialog[A_S1].dp = alert_dialog[A_S2].dp = alert_dialog[A_S3].dp = 
			   alert_dialog[A_B1].dp = alert_dialog[A_B2].dp = "";

   if (s1) {
      alert_dialog[A_S1].dp = s1;
      maxlen = strlen(s1);
   }

   if (s2) {
      alert_dialog[A_S2].dp = s2;
      len1 = strlen(s2);
      if (len1 > maxlen)
	 maxlen = len1;
   }

   if (s3) {
      alert_dialog[A_S3].dp = s3;
      len1 = strlen(s3);
      if (len1 > maxlen)
	 maxlen = len1;
   }

   if (b1) {
      alert_dialog[A_B1].flags &= ~D_HIDDEN;
      alert_dialog[A_B1].key = c1;
      alert_dialog[A_B1].dp = b1;
      len1 = strlen(b1);
   }
   else {
      alert_dialog[A_B1].flags |= D_HIDDEN; 
      len1 = 0;
   }

   if (b2) {
      alert_dialog[A_B2].flags &= ~D_HIDDEN;
      alert_dialog[A_B2].key = c2;
      alert_dialog[A_B2].dp = b2;
      len2 = strlen(b2);
   }
   else {
      alert_dialog[A_B2].flags |= D_HIDDEN; 
      len2 = 0;
   }

   len1 = MAX(len1, len2) + 3;
   if (len1*2 > maxlen)
      maxlen = len1*2;

   maxlen += 4;
   alert_dialog[0].w = maxlen*8;
   alert_dialog[A_S1].x = alert_dialog[A_S2].x = alert_dialog[A_S3].x = 
						alert_dialog[0].x + maxlen*4;

   alert_dialog[A_B1].w = alert_dialog[A_B2].w = len1*8;

   if ((b1) && (b2)) {
      alert_dialog[A_B1].x = alert_dialog[0].x + maxlen*4 - len1*8 - 8;
      alert_dialog[A_B2].x = alert_dialog[0].x + maxlen*4 + 8;
   }
   else
      alert_dialog[A_B1].x = alert_dialog[A_B2].x = 
				    alert_dialog[0].x + maxlen*4 - len1*4;

   centre_dialog(alert_dialog);
   set_dialog_color(alert_dialog, gui_fg_color, gui_bg_color);

   clear_keybuf();

   do {
   } while (mouse_b);

   if (popup_dialog(alert_dialog, A_B1) == A_B1)
      return 1;
   else
      return 2;
}



static int fs_edit_proc(int, DIALOG *, int );
static int fs_flist_proc(int, DIALOG *, int );
static int fs_dlist_proc(int, DIALOG *, int );
static char *fs_flist_getter(int, int *);
static char *fs_dlist_getter(int, int *);


#define FLIST_SIZE      128

typedef struct FLIST
{
   char dir[80];
   int size;
   char f[FLIST_SIZE][14];
} FLIST;

static FLIST *flist = NULL;

static char *fext = NULL;



static DIALOG file_selector[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp) */
   { d_shadow_box_proc, 0,    0,    304,  160,  0,    0,    0,    0,       0,    0,    NULL },
   { d_ctext_proc,      152,  8,    1,    1,    0,    0,    0,    0,       0,    0,    NULL },
   { d_button_proc,     208,  107,  80,   16,   0,    0,    0,    D_EXIT,  0,    0,    "OK" },
   { d_button_proc,     208,  129,  80,   16,   0,    0,    27,   D_EXIT,  0,    0,    "Cancel" },
   { fs_edit_proc,      16,   28,   272,  8,    0,    0,    0,    0,       79,   0,    NULL },
   { fs_flist_proc,     16,   46,   176,  99,   0,    0,    0,    D_EXIT,  0,    0,    fs_flist_getter },
   { fs_dlist_proc,     208,  46,   80,   51,   0,    0,    0,    D_EXIT,  0,    0,    fs_dlist_getter },
   { NULL }
};

#define FS_MESSAGE   1
#define FS_OK        2
#define FS_CANCEL    3
#define FS_EDIT      4
#define FS_FILES     5
#define FS_DISKS     6



/* fs_dlist_getter:
 *  Listbox data getter routine for the file selector disk list.
 */
static char *fs_dlist_getter(int index, int *list_size)
{
   static char d[] = "A:\\";

   if (index < 0) {
      if (list_size)
	 *list_size = 26;
      return NULL;
   }

   d[0] = 'A' + index;
   return d;
}



/* fs_dlist_proc:
 *  Dialog procedure for the file selector disk list.
 */
static int fs_dlist_proc(int msg, DIALOG *d, int c)
{
   int ret;
   char *s = file_selector[FS_EDIT].dp;

   if (msg == MSG_START)
      d->d1 = d->d2 = 0;

   ret = d_list_proc(msg, d, c);

   if (ret == D_CLOSE) {
      *(s++) = 'A' + d->d1;
      *(s++) = ':';
      *(s++) = '\\';
      *s = 0;
      show_mouse(NULL);
      SEND_MESSAGE(file_selector+FS_FILES, MSG_START, 0);
      SEND_MESSAGE(file_selector+FS_FILES, MSG_DRAW, 0);
      SEND_MESSAGE(file_selector+FS_EDIT, MSG_START, 0);
      SEND_MESSAGE(file_selector+FS_EDIT, MSG_DRAW, 0);
      show_mouse(screen);
      return D_O_K;
   }

   return ret;
}



/* fs_edit_proc:
 *  Dialog procedure for the file selector editable string.
 */
static int fs_edit_proc(int msg, DIALOG *d, int c)
{
   char *s = d->dp;
   int ch;
   int attr;
   int x;
   char b[80];

   if (msg == MSG_START) {
      if (s[0]) {
	 _fixpath(s, b);
	 errno = 0;

	 x = s[strlen(s)-1];
	 if ((x=='/') || (x=='\\'))
	    put_backslash(b);

	 for (x=0; b[x]; x++) {
	    if (b[x] == '/')
	       s[x] = '\\';
	    else
	       s[x] = b[x];
	 }
	 s[x] = 0;
      }
   }

   if (msg == MSG_KEY) {
      if (*s)
	 ch = s[strlen(s)-1];
      else
	 ch = 0;
      if (ch == ':')
	 put_backslash(s);
      else {
	 if ((ch != '/') && (ch != '\\')) {
	    if (file_exists(s, FA_RDONLY | FA_HIDDEN | FA_DIREC, &attr)) {
	       if (attr & FA_DIREC)
		  put_backslash(s);
	       else
		  return D_CLOSE;
	    }
	    else
	       return D_CLOSE;
	 }
      }
      show_mouse(NULL);
      SEND_MESSAGE(file_selector+FS_FILES, MSG_START, 0);
      SEND_MESSAGE(file_selector+FS_FILES, MSG_DRAW, 0);
      SEND_MESSAGE(d, MSG_START, 0);
      SEND_MESSAGE(d, MSG_DRAW, 0);
      show_mouse(screen);
      return D_O_K;
   }

   if (msg==MSG_CHAR) {
      ch = c & 0xff;
      if ((ch >= 'a') && (ch <= 'z'))
	 c = (c & 0xffffff00L) | (ch - 'a' + 'A');
      else if (ch == '/')
	 c = (c & 0xffffff00L) | '\\';
      else if ((ch != '\\') && (ch != '_') && (ch != ':') && (ch != '.') && 
	       ((ch < 'A') || (ch > 'Z')) && ((ch < '0') || (ch > '9')) &&
	       (ch != 8) && (ch != 127) && (ch != 0))
	 return D_O_K;
   }

   return d_edit_proc(msg, d, c); 
}



/* fs_flist_putter:
 *  Callback routine for for_each_file() to fill the file selector listbox.
 */
static void fs_flist_putter(char *str, int attrib)
{
   int c, c2;
   char *s, *ext, *tok;
   static char ext_tokens[] = " ,;";

   s = get_filename(str);
   strupr(s);

   if ((fext) && (!(attrib & FA_DIREC))) {
      ext = get_extension(s);
      tok = strtok(fext, ext_tokens);
      while (tok) {
	 if (stricmp(ext, tok) == 0)
	    break;
	 tok = strtok(NULL, ext_tokens);
      }
      if (!tok)
	 return;
   }

   if ((flist->size < FLIST_SIZE) && (strcmp(s,".")!=0)) {
      for (c=0; c<flist->size; c++) {
	 if (flist->f[c][strlen(flist->f[c])-1]=='\\') {
	    if (attrib & FA_DIREC)
	       if (strcmp(s, flist->f[c]) < 0)
		  break;
	 }
	 else {
	    if (attrib & FA_DIREC)
	       break;
	    if (strcmp(s, flist->f[c]) < 0)
	       break;
	 }
      }
      for (c2=flist->size; c2 > c; c2--)
	 strcpy(flist->f[c2], flist->f[c2-1]);

      strcpy(flist->f[c], s);
      if (attrib & FA_DIREC)
	 put_backslash(flist->f[c]);
      flist->size++;
   }
}



/* fs_flist_getter:
 *  Listbox data getter routine for the file selector list.
 */
static char *fs_flist_getter(int index, int *list_size)
{
   if (index < 0) {
      if (list_size)
	 *list_size = flist->size;
      return NULL;
   }
   return flist->f[index];
}



/* fs_flist_proc:
 *  Dialog procedure for the file selector list.
 */
static int fs_flist_proc(int msg, DIALOG *d, int c)
{
   int ret;
   int sel = d->d1;
   char *s = file_selector[FS_EDIT].dp;
   static int recurse_flag = 0;

   if (msg == MSG_START) {
      if (!flist)
	 flist = malloc(sizeof(FLIST));
      if (!flist) {
	 errno = ENOMEM;
	 return D_CLOSE; 
      }
      flist->size = 0;
      strcpy(flist->dir, s);
      *get_filename(flist->dir) = 0;
      put_backslash(flist->dir);
      strcat(flist->dir,"*.*");
      for_each_file(flist->dir, FA_RDONLY | FA_DIREC | FA_ARCH, fs_flist_putter, 0);
      if (errno)
	 alert(NULL, "Disk error", NULL, "OK", NULL, 13, 0);
      *get_filename(flist->dir) = 0;
      d->d1 = d->d2 = 0;
      sel = 0;
   }

   if (msg == MSG_END) {
      if (flist)
	 free(flist);
      flist = NULL;
   }

   recurse_flag++;
   ret = d_list_proc(msg,d,c);     /* call the parent procedure */
   recurse_flag--;

   if (((sel != d->d1) || (ret == D_CLOSE)) && (recurse_flag == 0)) {
      strcpy(s, flist->dir);
      *get_filename(s) = 0;
      put_backslash(s);
      strcat(s, flist->f[d->d1]);
      show_mouse(NULL);
      SEND_MESSAGE(file_selector+FS_EDIT, MSG_START, 0);
      SEND_MESSAGE(file_selector+FS_EDIT, MSG_DRAW, 0);
      show_mouse(screen);

      if (ret == D_CLOSE)
	 return SEND_MESSAGE(file_selector+FS_EDIT, MSG_KEY, 0);
   }

   return ret;
}



/* file_select:
 *  Displays the Allegro file selector, with the message as caption. 
 *  Allows the user to select a file, and stores the selection in path 
 *  (which should have room for at least 80 characters). The files are
 *  filtered according to the file extensions in ext. Passing NULL
 *  includes all files, "PCX;BMP" includes only files with .PCX or .BMP
 *  extensions. Returns zero if it was closed with the Cancel button, 
 *  non-zero if it was OK'd.
 */
int file_select(char *message, char *path, char *ext)
{
   int ret;
   char *p;

   file_selector[FS_MESSAGE].dp = message;
   file_selector[FS_EDIT].dp = path;
   fext = ext;

   if (!path[0]) {
      getcwd(path, 80);
      for (p=path; *p; p++)
	 if (*p=='/')
	    *p = '\\';
	 else
	    if ((*p >= 'a') && (*p <= 'z'))
	       *p += ('A' - 'a');
      put_backslash(path);
   }

   clear_keybuf();

   do {
   } while (mouse_b);

   centre_dialog(file_selector);
   set_dialog_color(file_selector, gui_fg_color, gui_bg_color);
   ret = do_dialog(file_selector, FS_EDIT);

   if ((ret == FS_CANCEL) || (*get_filename(path) == 0))
      return FALSE;
   else
      return TRUE; 
}



/* gfx_mode_getter:
 *  Listbox data getter routine for the graphics mode list.
 */
static char *gfx_mode_getter(int index, int *list_size)
{
   static char *mode[] = {
      "320x200",
      "640x400",
      "640x480",
      "800x600",
      "1024x768"
   };

   if (index < 0) {
      if (list_size)
	 *list_size = 5;
      return NULL;
   }

   return mode[index];
}



/* gfx_card_getter:
 *  Listbox data getter routine for the graphics card list.
 */
static char *gfx_card_getter(int index, int *list_size)
{
   static char *card[] = {
      "Autodetect",
      "VGA",
      "VESA 1.x",
      "VESA 2.0 (banked)",
      "VESA 2.0 (linear)",
      "Cirrus 64xx",
      "Cirrus 54xx",
      "S3",
      "Tseng ET3000",
      "Tseng ET4000"
   };

   if (index < 0) {
      if (list_size)
	 *list_size = 10;
      return NULL;
   }

   return card[index];
}



static DIALOG gfx_mode_dialog[] =
{
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp) */
   { d_shadow_box_proc, 0,    0,    280,  134,  0,    0,    0,    0,       0,    0,    NULL },
   { d_ctext_proc,      140,  8,    1,    1,    0,    0,    0,    0,       0,    0,    "Screen Mode" },
   { d_button_proc,     184,  81,   80,   16,   0,    0,    0,    D_EXIT,  0,    0,    "OK" },
   { d_button_proc,     184,  103,  80,   16,   0,    0,    27,   D_EXIT,  0,    0,    "Cancel" },
   { d_list_proc,       16,   28,   152,  91,   0,    0,    0,    D_EXIT,  0,    0,    gfx_card_getter },
   { d_list_proc,       184,  28,   80,   43,   0,    0,    0,    D_EXIT,  2,    0,    gfx_mode_getter },
   { NULL }
};

#define GFX_CANCEL         3
#define GFX_DRIVER_LIST    4
#define GFX_MODE_LIST      5



/* gfx_mode_select:
 *  Displays the Allegro graphics mode selection dialog, which allows the
 *  user to select a screen mode and graphics card. Stores the selection
 *  in the three variables, and returns zero if it was closed with the 
 *  Cancel button, or non-zero if it was OK'd.
 */
int gfx_mode_select(int *card, int *w, int *h)
{
   int ret;

   clear_keybuf();

   do {
   } while (mouse_b);

   centre_dialog(gfx_mode_dialog);
   set_dialog_color(gfx_mode_dialog, gui_fg_color, gui_bg_color);
   ret = do_dialog(gfx_mode_dialog, GFX_DRIVER_LIST);

   switch (gfx_mode_dialog[GFX_MODE_LIST].d1) {
      case 1:  *w = 640;  *h = 400;  break;
      case 2:  *w = 640;  *h = 480;  break;
      case 3:  *w = 800;  *h = 600;  break;
      case 4:  *w = 1024; *h = 768;  break;
      default: *w = 320;  *h = 200;  break;
   }

   *card = gfx_mode_dialog[GFX_DRIVER_LIST].d1;

   if (ret == GFX_CANCEL)
      return FALSE;
   else 
      return TRUE;
}

