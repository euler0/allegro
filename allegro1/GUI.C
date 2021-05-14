/*
		  //  /     /     ,----  ,----.  ,----.  ,----.
		/ /  /     /     /      /    /  /    /  /    /
	      /  /  /     /     /___   /       /____/  /    /
	    /---/  /     /     /      /  __   /\      /    /
	  /    /  /     /     /      /    /  /  \    /    /
	/     /  /____ /____ /____  /____/  /    \  /____/

	Low Level Game Routines (version 1.0)

	Graphical User inteface routines

	See allegro.txt for instructions and copyright conditions.
   
	By Shawn Hargreaves,
	1 Salisbury Road,
	Market Drayton,
	Shropshire,
	England TF9 1AJ
	email slh100@tower.york.ac.uk (until 1996)
*/

#include "errno.h"
#include "stdlib.h"
#include "string.h"
#include "dir.h"

#include "allegro.h"

#ifdef BORLAND
#include "alloc.h"
#endif
#ifdef GCC
#include "unistd.h"
#endif


short d_box_proc(msg, d, c)
short msg;
DIALOG *d;
unsigned long c;
{
   /* a very simple object, can only draw itself */

   if (msg==MSG_DRAW) {
      rectfill(screen, d->x+1, d->y+1, d->x+d->w-1, d->y+d->h-1, d->bg);
      rect(screen, d->x, d->y, d->x+d->w, d->y+d->h, d->fg);
   }
   return D_O_K;
}



short d_shadow_box_proc(msg, d, c)
short msg;
DIALOG *d;
unsigned long c;
{
   /* a very simple object, can only draw itself */

   if (msg==MSG_DRAW) {
      rectfill(screen, d->x+1, d->y+1, d->x+d->w-2, d->y+d->h-2, d->bg);
      rect(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, d->fg);
      vline(screen, d->x+d->w, d->y+1, d->y+d->h, d->fg);
      hline(screen, d->x+1, d->y+d->h, d->x+d->w, d->fg);
   }
   return D_O_K;
}



short d_text_proc(msg, d, c)
short msg;
DIALOG *d;
unsigned long c;
{
   /* a very simple object, can only draw itself */

   if (msg==MSG_DRAW) {
      textmode(d->bg);
      textout(screen, font, d->dp, d->x, d->y, d->fg);
   }
   return D_O_K;
}



short d_button_proc(msg, d, c)
short msg;
DIALOG *d;
unsigned long c;
{
   short flag, flag2;
   short swap;

   switch (msg) {

   case MSG_DRAW:
      if (d->flags & D_SELECTED) {
	 rectfill(screen, d->x+2, d->y+2, d->x+d->w-1, d->y+d->h-1, d->fg);
	 rect(screen, d->x+1, d->y+1, d->x+d->w, d->y+d->h, d->bg);
	 vline(screen, d->x, d->y, d->y+d->h-1, 0);
	 hline(screen, d->x, d->y, d->x+d->w-1, 0);
	 textmode(d->fg);
	 textout(screen, font, d->dp, d->x+d->w/2-strlen(d->dp)*4+1,
				      d->y+d->h/2-4+1, d->bg);
      }
      else {
	 rectfill(screen, d->x+1, d->y+1, d->x+d->w-2, d->y+d->h-2, d->bg);
	 rect(screen, d->x, d->y, d->x+d->w-1, d->y+d->h-1, d->fg);
	 vline(screen, d->x+d->w, d->y+1, d->y+d->h-1, d->fg);
	 hline(screen, d->x+1, d->y+d->h, d->x+d->w, d->fg);
	 textmode(d->bg);
	 textout(screen, font, d->dp, d->x+d->w/2-strlen(d->dp)*4,
				      d->y+d->h/2-4, d->fg);
      }
      break;
   
   case MSG_KEY:
      if (d->flags & D_EXIT)
	 return D_CLOSE;
      d->flags ^= D_SELECTED;
      show_mouse(NULL);
      (*d->proc)(MSG_DRAW,d,0);
      show_mouse(screen);
      break;

   case MSG_CLICK:
      flag = d->flags & D_SELECTED;
      if (d->flags & D_EXIT)
	 swap = FALSE;
      else
	 swap = flag;
      while (mouse_b) {
	 flag2 = ((mouse_x >= d->x) && (mouse_y >= d->y) &&
		  (mouse_x <= d->x + d->w) && (mouse_y <= d->y + d->h));
	 if (swap)
	    flag2 = !flag2;
	 if (((flag) && (!flag2)) || ((!flag) && (flag2))) {
	    d->flags ^= D_SELECTED;
	    flag = d->flags & D_SELECTED;
	    show_mouse(NULL);
	    (*d->proc)(MSG_DRAW,d,0);
	    show_mouse(screen);
	 }
      }
      if (d->flags & D_SELECTED)
	 if (d->flags & D_EXIT) {
	    d->flags ^= D_SELECTED;
	    return D_CLOSE;
	 }
      break; 
   }

   return D_O_K;
}



short d_check_proc(msg, d, c)
short msg;
DIALOG *d;
unsigned long c;
{
   short x;

   if (msg==MSG_DRAW) {
      textmode(d->bg);
      textout(screen, font, d->dp, d->x, d->y+(d->h-8)/2, d->fg);
      x = d->x + strlen(d->dp)*8 + 4;
      rectfill(screen, x+1, d->y+1, x+d->h-1, d->y+d->h-1, d->bg);
      rect(screen, x, d->y, x+d->h, d->y+d->h, d->fg);
      if (d->flags & D_SELECTED) {
	 line(screen, x, d->y, x+d->h, d->y+d->h, d->fg);
	 line(screen, x, d->y+d->h, x+d->h, d->y, d->fg); 
      }
      return D_O_K;
   } 

   return d_button_proc(msg,d,0);
}



short d_edit_proc(msg, d, c)
short msg;
DIALOG *d;
unsigned long c;
{
   short l;
   char *s;
   short x;

   switch(msg) {
   
   case MSG_DRAW:
      x = d->x;
      s = d->dp;
      if (strlen(s) > (d->w/8)) {
	 textmode(d->fg);
	 textout(screen, font, "<", x, d->y, d->bg);
	 x += 8;
	 s += strlen(s) - (d->w/8-1);
      }
      textmode(d->bg);
      textout(screen, font, s, x, d->y, d->fg);
      x += strlen(s)*8;
      if (d->flags & D_GOTFOCUS) {
	 rectfill(screen, x, d->y, x+7, d->y+7, d->fg);
	 x += 8;
      }
      if (x <= d->x+d->w)
	 rectfill(screen, x, d->y, d->x+d->w, d->y+7, d->bg);
      break;

   case MSG_WANTFOCUS:
      return -1;
   
   case MSG_GOTFOCUS:
   case MSG_LOSTFOCUS:
      show_mouse(NULL);
      (*d->proc)(MSG_DRAW,d,0);
      show_mouse(screen);
      break;

   case MSG_CHAR:
      s = d->dp;
      l = strlen(s);
      c &= 0xff;
      if (c == 8) {           /* backspace */
	 if (l > 0) {
	    s[--l] = 0;
	    show_mouse(NULL);
	    (*d->proc)(MSG_DRAW,d,0);
	    show_mouse(screen);
	 }
      }
      else {                  /* insert a char */
	 if ((c >= 32) && (c <= 126)) {
	    if (l < d->d1) {
	       s[l] = (char)c;
	       s[l+1] = 0;
	       show_mouse(NULL);
	       (*d->proc)(MSG_DRAW,d,0);
	       show_mouse(screen);
	    }
	 } 
      }
      break;
   }

   return D_O_K;
}



#define UP_ARROW        18432L
#define DOWN_ARROW      20480L



short d_list_proc(msg, d, c)
short msg;
DIALOG *d;
unsigned long c;
{
   typedef char * (*getfuncptr)();
   char *s;
   register short width, len;
   register short i;
   register short boxsize;
   short listsize;
   register short fg, bg;
   register short x;
   register short y = 0;

   switch(msg) {

   case MSG_START:
      d->d1 = d->d2 = 0;
      break;

   case MSG_DRAW:
      rect(screen, d->x, d->y, d->x+d->w, d->y+d->h, d->fg);
      (*(getfuncptr)d->dp)(-1,&listsize);
      boxsize = (d->h-2) / 8;
      width = ((d->w-18) / 8);
      if (listsize) {
	 i = ((d->h-4) * boxsize + listsize/2) / listsize;
	 x = d->x+d->w-10;
	 y = d->y+2;
	 vline(screen, x-2, d->y+1, d->y+d->h-1, d->fg);
	 if (d->d2 > 0) {
	    len = ((d->h)*d->d2+listsize/2)/listsize;
	    rectfill(screen, x, y, x+8, y+len-1, d->bg);
	    y += len;
	 }
	 if (y+i < d->y+d->h-2) {
	    rectfill(screen, x, y, x+8, y+i, d->fg);
	    y+=i;
	    rectfill(screen, x, y, x+8, d->y+d->h-2, d->bg);
	 }
	 else
	    rectfill(screen, x, y, x+8, d->y+d->h-2, d->fg);
      }
      else {
	 x = d->x + d->w;
	 vline(screen, x-12, d->y+1, d->y+d->h-1, d->fg);
	 rectfill(screen, x-10, d->y+2, x-2, d->y+d->h-2, d->bg);
      }
      for (i=0; i<boxsize; i++) {
	 if (d->d2+i < listsize) {
	    if ((d->d2+i == d->d1) && (d->flags & D_GOTFOCUS)) {
	       fg = d->bg;
	       bg = d->fg;
	    }
	    else {
	       fg = d->fg;
	       bg = d->bg;
	    }
	    textmode(bg);
	    s = (*(getfuncptr)d->dp)(i+d->d2,NULL);
	    x = d->x+2;
	    y = d->y+2+i*8;
	    rectfill(screen, x, y, x+7, y+7, bg);
	    x += 8;
	    textout(screen, font, s, x, y, fg);
	    len = strlen(s);
	    if (width-len >= 0) {
	       x += len*8;
	       rectfill(screen, x, y, x+(width-len)*8, y+7, bg);
	    }
	 }
	 else
	    rectfill(screen, d->x+2,  d->y+2+i*8,
		     d->x+10+width*8, d->y+9+i*8, d->bg);
      }
      break;

   case MSG_CLICK:
      (*(getfuncptr)d->dp)(-1,&listsize);
      if (listsize) {
	 if (mouse_x >= d->x+d->w-12) {
	    boxsize = (d->h-2) / 8;
	    while (mouse_b) {
	       i = ((d->h-4) * boxsize + listsize/2) / listsize;
	       len = ((d->h) * d->d2 + listsize/2) / listsize + 2;
	       if ((mouse_y >= d->y+len) && (mouse_y <= d->y+len+i)) {
		  x = mouse_y - len + 2;
		  while (mouse_b) {
		     y = (listsize * (mouse_y - x) + d->h/2) / d->h;
		     if (y > listsize-boxsize)
			y = listsize-boxsize;
		     if (y < 0)
			y = 0;
		     if (y != d->d2) {
			d->d2 = y;
			show_mouse(NULL);
			(*d->proc)(MSG_DRAW,d,0);
			show_mouse(screen);
		     }
		  }
		  break;
	       }
	       else {
		  if (mouse_y <= d->y+len)
		     y = d->d2 - boxsize;
		  else
		     if (mouse_y > d->y+len+i)
			y = d->d2 + boxsize;
		  if (y > listsize-boxsize)
		     y = listsize-boxsize;
		  if (y < 0)
		     y = 0;
		  if (y != d->d2) {
		     d->d2 = y;
		     show_mouse(NULL);
		     (*d->proc)(MSG_DRAW,d,0);
		     show_mouse(screen);
		     rest(100);
		  }
	       }
	    }
	 }
	 else {
	    while (mouse_b) {
	       if (mouse_y < d->y) {
		  (*d->proc)(MSG_CHAR,d,UP_ARROW);
		  rest(20);
	       }
	       else {
		  if (mouse_y >= d->y + d->h) {
		     (*d->proc)(MSG_CHAR,d,DOWN_ARROW);
		     rest(20);
		  }
		  else {
		     i = ((mouse_y - d->y - 2) / 8) + d->d2;
		     if (i < 0)
			i = 0;
		     else {
			if (i >= listsize)
			   i = listsize - 1;
		     }
		     if (i != d->d1) {
			d->d1 = i;
			show_mouse(NULL);
			(*d->proc)(MSG_DRAW,d,0);
			show_mouse(screen);
		     }
		  }
	       }
	    }
	 }
      }
      break;

   case MSG_DCLICK:
      if (d->flags & D_EXIT) {
	 (*(getfuncptr)d->dp)(-1,&listsize);
	 if (listsize)
	    if (mouse_x < d->x+d->w-12) {
	       i = d->d1;
	       (*d->proc)(MSG_CLICK,d,0);
	       if (i==d->d1) 
		  return D_CLOSE;
	    }
      }
      break;

   case MSG_KEY:
      (*(getfuncptr)d->dp)(-1,&listsize);
      if ((listsize) && (d->flags & D_EXIT))
	 return D_CLOSE;
      break;

   case MSG_WANTFOCUS:
      return -1;
   
   case MSG_GOTFOCUS:
   case MSG_LOSTFOCUS:
      show_mouse(NULL);
      (*d->proc)(MSG_DRAW,d,0);
      show_mouse(screen);
      break;

   case MSG_CHAR:
      (*(getfuncptr)d->dp)(-1,&listsize);
      if (listsize) {
	 if ((c == UP_ARROW) || (c == DOWN_ARROW)) {
	    boxsize = (d->h-2) / 8;
	    if (c == UP_ARROW) {
	       d->d1--;
	       if (d->d1 < 0)
		  d->d1 = 0;
	    }
	    else {
	       d->d1++;
	       if (d->d1 >= listsize)
		  d->d1 = listsize - 1;
	    }
	    if (d->d1 < d->d2)
	       d->d2 = d->d1;
	    else {
	       if (d->d1 >= d->d2 + (boxsize - 1)) {
		  d->d2 = d->d1 - (boxsize - 1);
		  if (d->d2 < 0)
		     d->d2 = 0;
	       }
	    } 
	    show_mouse(NULL);
	    (*d->proc)(MSG_DRAW,d,0);
	    show_mouse(screen); 
	 }
      }
      break;
   }

   return D_O_K;
}



volatile short _d_click_status, _d_click_time;
volatile short _d_click_installed = FALSE;

#define D_CLICK_START      0
#define D_CLICK_RELEASE    1
#define D_CLICK_AGAIN      2
#define D_CLICK_NOT        3


void _d_click_check()
{
   if (_d_click_status==D_CLICK_START) {
      if (mouse_b)
	 goto inc_time;
      else {
	 _d_click_status = D_CLICK_RELEASE;
	 _d_click_time = 0;
      }
   }
   else
      if (_d_click_status==D_CLICK_RELEASE) {
	 if (mouse_b)
	    _d_click_status = D_CLICK_AGAIN;
	 else
	    goto inc_time;
      }
   
   return;
   
   inc_time:
   if (_d_click_time++ > 10)
      _d_click_status = D_CLICK_NOT;
}



short do_dialog(dialog)
register DIALOG *dialog;
{
   extern BITMAP *_mouse_bmp;
   register short c, c2;
   short end;
   short obj = 0;
   short res = D_O_K;
   short r;
   BITMAP *old_mouse_bmp = _mouse_bmp;
   unsigned long ch;

   set_clip(screen,0,0,SCREEN_W-1,SCREEN_H-1);

   for (c=0; dialog[c].proc; c++) {       /* send the start messages */
      if ((r=(*dialog[c].proc)(MSG_START,dialog+c,0)) != D_O_K) {
	 res = r;
	 obj = c;
      }
   }
   end = c-1;

   if (res != D_CLOSE)
      res = D_REDRAW;

   do {
   } while (mouse_b);

   while (res != D_CLOSE) {

      if (res == D_REDRAW) {
	 show_mouse(NULL); 
	 res = D_O_K; 
	 for (c=0; c<=end; c++)                 /* draw all the objects */
	    if ((r=(*dialog[c].proc)(MSG_DRAW,dialog+c,0)) != D_O_K) {
	       res = r;
	       obj = c;
	    }
	 show_mouse(screen);
      }

      if (mouse_b) {
	 for (c=end; c>=0; c--) {
	    if ((mouse_x >= dialog[c].x) && (mouse_y >= dialog[c].y) &&
		(mouse_x <= dialog[c].x + dialog[c].w) &&
		(mouse_y <= dialog[c].y + dialog[c].h)) {
	       if (!(dialog[c].flags & D_GOTFOCUS)) {
		  if ((*dialog[c].proc)(MSG_WANTFOCUS,dialog+c,0)) {
		     for (c2=end; c2>=0; c2--) {
			if (dialog[c2].flags & D_GOTFOCUS) {
			   dialog[c2].flags ^= D_GOTFOCUS;
			   if ((r=(*dialog[c2].proc)(MSG_LOSTFOCUS,dialog+c2,0)) != D_O_K) {
			      res = r;
			      obj = c;
			   }
			}
		     }
		     dialog[c].flags |= D_GOTFOCUS;
		     if ((r=(*dialog[c].proc)(MSG_GOTFOCUS,dialog+c,0)) != D_O_K) {
			res = r;
			obj = c;
		     }
		  }
	       }
	       _d_click_time = 0;
	       _d_click_status = D_CLICK_START;
	       if (!_d_click_installed) {
		  install_int(_d_click_check,20);
		  _d_click_installed = TRUE;
	       }
	       if ((r=(*dialog[c].proc)(MSG_CLICK,dialog+c,0)) != D_O_K) {
		  res = r;
		  obj = c;
	       }
	       if (res==D_O_K)
		  do {
		  } while ((_d_click_status != D_CLICK_AGAIN) &&
			   (_d_click_status != D_CLICK_NOT));
	       remove_int(_d_click_check);
	       _d_click_installed = FALSE;

	       if ((res==D_O_K) && (_d_click_status==D_CLICK_AGAIN))
		  if ((mouse_x >= dialog[c].x) && (mouse_y >= dialog[c].y) &&
		      (mouse_x <= dialog[c].x + dialog[c].w) &&
		      (mouse_y <= dialog[c].y + dialog[c].h))
		     if ((r=(*dialog[c].proc)(MSG_DCLICK,dialog+c,0)) != D_O_K) {
			res = r;
			obj = c;
		     }
	       break;
	    }
	 }
      }

      if (keypressed()) {
	 ch = readkey();
	 if ((ch & 0xff) == 9) {       /* tab */
	    for (c=end; c>=0; c--)
	       if (dialog[c].flags & D_GOTFOCUS)
		  break;
	    c2 = c + 1;
	    if (c2 > end)
	       c2 = 0;
	    while (c2 != c) {
	       if ((*dialog[c2].proc)(MSG_WANTFOCUS,dialog+c,0)) {
		  if (c >= 0) {
		     dialog[c].flags ^= D_GOTFOCUS;
		     if ((r=(*dialog[c].proc)(MSG_LOSTFOCUS,dialog+c,0)) != D_O_K) {
			res = r;
			obj = c;
		     }
		  }
		  dialog[c2].flags |= D_GOTFOCUS;
		  if ((r=(*dialog[c2].proc)(MSG_GOTFOCUS,dialog+c2,0)) != D_O_K) {
		     res = r;
		     obj = c2;
		  }
		  goto keydone;
	       }
	       if (c2++ > end)
		  c2 = (c==-1) ? -1 : 0;
	    } 
	 }
	 else {
	    if (ch & 0xff) {
	       for (c=end; c>=0; c--) {
		  if (dialog[c].key == (ch & 0xff)) {
		     if ((r=(*dialog[c].proc)(MSG_KEY,dialog+c,0)) != D_O_K) {
			res = r;
			obj = c;
		     }
		     goto keydone;
		  }
	       }
	    }
	    for (c=end; c>=0; c--) {
	       if (dialog[c].flags & D_GOTFOCUS) {
		  if ((r=(*dialog[c].proc)(MSG_CHAR,dialog+c,ch)) != D_O_K) {
		     res = r;
		     obj = c;
		  }
		  break;
	       }
	    }
	 }
	 keydone:
	    ;
      }
   }

   for (c=end; c>=0; c--)                 /* send the end messages */
      (*dialog[c].proc)(MSG_END,dialog+c,0);

   show_mouse(old_mouse_bmp);

   return obj;
}



short popup_dialog(dialog)
DIALOG *dialog;
{
   extern BITMAP *_mouse_bmp;
   BITMAP *bmp;
   short x, y, w, h;
   short ret;
   BITMAP *old_mouse_bmp = _mouse_bmp;

   x = dialog->x & 0xfff0;
   y = dialog->y;
   w = (dialog->w+1 + (dialog->x & 0x0f) + 15) & 0xfff0;
   h = dialog->h+1;
   bmp = create_bitmap(w,h); 

   if (bmp) {
      show_mouse(NULL);
      blit(screen, bmp, x, y, 0, 0, w, h);
   }
   else
      errno = ENOMEM;

   ret = do_dialog(dialog);
   
   if (bmp) {
      show_mouse(NULL);
      blit(bmp, screen, 0, 0, x, y, w, h);
      destroy_bitmap(bmp);
   }
   show_mouse(old_mouse_bmp);
   return ret;
}



DIALOG _alert_dialog[] =
{
   { d_shadow_box_proc, 0, 68, 0, 64, 15, 0, 0, 0, 0, 0, NULL },
   { d_text_proc, 0, 76, 1, 1, 15, 0, 0, 0, 0, 0, NULL },
   { d_text_proc, 0, 84, 1, 1, 15, 0, 0, 0, 0, 0, NULL },
   { d_text_proc, 0, 92, 1, 1, 15, 0, 0, 0, 0, 0, NULL },
   { d_button_proc, 0, 108, 0, 16, 15, 0, 0, D_EXIT, 0, 0, NULL },
   { d_button_proc, 0, 108, 0, 16, 15, 0, 0, D_EXIT, 0, 0, NULL },
   { NULL }
};



short alert(s1, s2, s3, b1, b2, c1, c2)
char *s1, *s2, *s3;
char *b1, *b2;
char c1, c2;
{
   short maxlen = 0;
   short len;
   short boxsize;

   if (s1) {
      len = maxlen = strlen(s1);
      _alert_dialog[1].dp = s1;
      _alert_dialog[1].x = 160 - len*4;
   }
   else
      _alert_dialog[1].dp = "";

   if (s2) {
      len = strlen(s2);
      if (len > maxlen)
	 maxlen = len;
      _alert_dialog[2].dp = s2;
      _alert_dialog[2].x = 160 - len*4;
   }
   else
      _alert_dialog[2].dp = "";

   if (s3) {
      len = strlen(s3);
      if (len > maxlen)
	 maxlen = len;
      _alert_dialog[3].dp = s3;
      _alert_dialog[3].x = 160 - len*4;
   }
   else
      _alert_dialog[3].dp = "";

   boxsize = maxlen;

   if (b2) {
      _alert_dialog[5].proc = d_button_proc;
      maxlen = strlen(b1);
      len = strlen(b2);
      if (len > maxlen)
	 maxlen = len;
      _alert_dialog[4].dp = b1;
      _alert_dialog[4].key = c1;
      _alert_dialog[5].dp = b2;
      _alert_dialog[5].key = c2;
      _alert_dialog[4].x = 120 - len*8;
      _alert_dialog[4].w = 32 + maxlen*8;
      _alert_dialog[5].x = 168;
      _alert_dialog[5].w = 32 + maxlen*8;
      maxlen <<= 1;
      maxlen += 10;
      if (maxlen > boxsize)
	 boxsize = maxlen;
   }
   else {
      len = strlen(b1);
      _alert_dialog[4].dp = b1;
      _alert_dialog[4].key = c1;
      _alert_dialog[4].x = 144 - len*4;
      _alert_dialog[4].w = 32 + len*8;
      _alert_dialog[5].proc = d_text_proc;
      _alert_dialog[5].dp = "";
      _alert_dialog[5].x = 0;
      len += 4;
      if (len > boxsize)
	 boxsize = len;
   }

   _alert_dialog[0].x = 144 - boxsize*4;
   _alert_dialog[0].w = 32 + boxsize*8;

   return (popup_dialog(_alert_dialog) - 3);
}



short _flist_proc(short, DIALOG *, unsigned long);
short _dlist_proc(short, DIALOG *, unsigned long);
short _dedit_proc(short, DIALOG *, unsigned long);
extern char *_flist_getter(), *_dlist_getter();


DIALOG _file_selector[] =
{
   { d_shadow_box_proc, 16, 24, 289, 160, 15, 0, 0, 0, 0, 0, NULL },
   { d_text_proc, 0, 32, 1, 1, 15, 0, 0, 0, 0, 0, NULL },
   { d_button_proc, 200, 132, 80, 16, 15, 0, 0, D_EXIT, 0, 0, "OK" },
   { d_button_proc, 200, 156, 80, 16, 15, 0, 27, D_EXIT, 0, 0, "Cancel" },
   { _dedit_proc, 24, 52, 279, 8, 15, 0, 13, D_GOTFOCUS, 79, 0, NULL },
   { _flist_proc, 40, 70, 136, 99, 15, 0, 0, D_EXIT, 0, 0, _flist_getter },
   { _dlist_proc, 200, 70, 56, 51, 15, 0, 0, D_EXIT, 0, 0, _dlist_getter },
   { NULL }
};



char *_dlist_getter(index, list_size)
short index;
short *list_size;
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



short _dlist_proc(msg, d, c)
short msg;
DIALOG *d;
unsigned long c;
{
   short ret;
   char *s = _file_selector[4].dp;

   if (msg==MSG_GOTFOCUS)
      d->key = 13;
   else
      if (msg==MSG_LOSTFOCUS)
	 d->key = 0;

   ret = d_list_proc(msg,d,c);     /* wey hey! class inheritance... */

   if (ret==D_CLOSE) {
      ret = D_O_K;
      *(s++) = 'A' + d->d1;
      *(s++) = ':';
      *(s++) = '\\';
      *s = 0;
      show_mouse(NULL);
      (*_file_selector[5].proc)(MSG_START,_file_selector+5,0);
      (*_file_selector[5].proc)(MSG_DRAW,_file_selector+5,0);
      (*_file_selector[4].proc)(MSG_DRAW,_file_selector+4,0);
      show_mouse(screen);
   }
   return ret;
}



short _dedit_proc(msg, d, c)
short msg;
DIALOG *d;
unsigned long c;
{
   char *s;
   char ch;
   short attr;

   if (msg==MSG_GOTFOCUS)
      d->key = 13;
   else
      if (msg==MSG_LOSTFOCUS)
	 d->key = 0;
      else
	 if (msg==MSG_KEY) {
	    s = d->dp;
	    if (*s)
	       ch = s[strlen(s)-1];
	    else
	       ch = 0;
	    if (ch==':')
	       put_backslash(s);
	    else
	       if (ch!='\\') {
		  if (file_exists(s, F_RDONLY | F_HIDDEN | F_SUBDIR, &attr)) {
		     if (attr & F_SUBDIR)
			put_backslash(s);
		     else
			return D_CLOSE;
		  }
		  else
		     return D_CLOSE;
	       }
	    show_mouse(NULL);
	    (*_file_selector[5].proc)(MSG_START,_file_selector+5,0);
	    (*_file_selector[5].proc)(MSG_DRAW,_file_selector+5,0);
	    (*d->proc)(MSG_DRAW,d,0);
	    show_mouse(screen);
	    return D_O_K;
	 }
	 else
	    if (msg==MSG_CHAR) {
	       ch = (char)(c & 0xff);
	       if ((ch >= 'a') && (ch <= 'z')) {
		  c &= 0xffffff00L;
		  c |= (ch - 'a' + 'A');
	       }
	       else
		  if (((ch < 'A') || (ch > 'A')) &&
		      ((ch < '0') || (ch > '9')) &&
		      (ch != '_') && (ch != '.') &&
		      (ch != '*') && (ch != '?') &&
		      (ch != ':') && (ch != '\\') &&
		      (ch != 8))
		     return D_O_K;
	    }

   return d_edit_proc(msg,d,c);     /* who needs C++ after all... */
}



#define FLIST_SIZE      128

typedef struct FLIST
{
   char dir[80];
   short size;
   char f[FLIST_SIZE][14];
} FLIST;

FLIST *_flist = NULL;



void _flist_putter(str,attrib)
char *str;
short attrib;
{
   register short c, c2;
   register char *s = get_filename(str);
   strupr(s);

   if ((_flist->size < FLIST_SIZE) && (strcmp(s,".")!=0)) {
      for (c=0; c<_flist->size; c++) {
	 if (_flist->f[c][strlen(_flist->f[c])-1]=='\\') {
	    if (attrib & F_SUBDIR)
	       if (strcmp(s,_flist->f[c]) < 0)
		  break;
	 }
	 else {
	    if (attrib & F_SUBDIR)
	       break;
	    if (strcmp(s,_flist->f[c]) < 0)
	       break;
	 }
      }
      for (c2=_flist->size; c2 > c; c2--)
	 strcpy(_flist->f[c2], _flist->f[c2-1]);

      strcpy(_flist->f[c], s);
      if (attrib & F_SUBDIR)
	 put_backslash(_flist->f[c]);
      _flist->size++;
   }
}



char *_flist_getter(index, list_size)
short index;
short *list_size;
{
   if (index < 0) {
      if (list_size)
	 *list_size = _flist->size;
      return NULL;
   }
   return _flist->f[index];
}



short _flist_proc(msg, d, c)
short msg;
DIALOG *d;
unsigned long c;
{
   short ret;
   short sel = d->d1;
   char *s = _file_selector[4].dp;
   char *s2;

   switch(msg) {

   case MSG_START:
      if (!_flist)
	 _flist = malloc(sizeof(FLIST));
      if (!_flist) {
	 errno = ENOMEM;
	 return D_CLOSE; 
      }
      _flist->size = 0;
      strcpy(_flist->dir, s);
      *get_filename(_flist->dir) = 0;
      put_backslash(_flist->dir);
      strcat(_flist->dir,"*.*");
      for_each_file(_flist->dir, F_RDONLY | F_SUBDIR | F_ARCH, _flist_putter, 0);
      if (errno)
	 alert(NULL,"Disk error",NULL,"OK",NULL,13,0);
      *get_filename(_flist->dir) = 0;
      sel = 0;
      break;

   case MSG_GOTFOCUS:
      d->key = 13;
      break;

   case MSG_LOSTFOCUS:
      d->key = 0;
      break;

   case MSG_END:
      if (_flist)
	 free(_flist);
      _flist = NULL;
      break;
   }

   ret = d_list_proc(msg,d,c);     /* call the parent procedure */

   if (ret==D_CLOSE) {
      strcpy(s,_flist->dir);
      *get_filename(s) = 0;
      put_backslash(s);
      strcat(s,_flist->f[d->d1]);
      s2 = s+strlen(s)-1;
      if ((s[0]) && (*s2=='\\')) {
	 ret = D_O_K;
	 *s2 = 0;
	 s2 = get_filename(s);
	 if (strcmp(s2,".")==0)
	    *s2 = 0;
	 else {
	    if (strcmp(s2,"..")==0) {
	       *s2 = 0;
	       if (s2 > s) {
		  *(--s2) = 0;
		  *get_filename(s) = 0;
	       }
	    }
	 }
	 put_backslash(s);
	 show_mouse(NULL);
	 (*d->proc)(MSG_START,d,0);
	 (*d->proc)(MSG_DRAW,d,0);
	 (*_file_selector[4].proc)(MSG_DRAW,_file_selector+4,0);
	 show_mouse(screen);
      }
   }
   else {
      if (sel != d->d1) {
	 strcpy(s,_flist->dir);
	 *get_filename(s) = 0;
	 put_backslash(s);
	 strcat(s,_flist->f[d->d1]);
	 show_mouse(NULL);
	 (*_file_selector[4].proc)(MSG_DRAW,_file_selector+4,0);
	 show_mouse(screen);
      }
   }
   return ret;
}



short file_select(message, path)
char *message;
char *path;
{
   short ret;
#ifdef GCC
   char *p;
#endif

   _file_selector[1].dp = message;
   _file_selector[1].x = 160 - strlen(message)*4;
   _file_selector[4].dp = path;
  
   _file_selector[4].key = 13;
   _file_selector[5].key = _file_selector[6].key = 0;
   _file_selector[4].flags = D_GOTFOCUS;
   _file_selector[5].flags = _file_selector[6].flags = D_EXIT;

   if (!path[0]) {
      getcwd(path,80);
#ifdef GCC
      for (p=path; *p; p++)
	 if (*p=='/')
	    *p = '\\';
	 else
	    if ((*p >= 'a') && (*p <= 'z'))
	       *p += ('A' - 'a');
#endif
      put_backslash(path);
   }

   ret = do_dialog(_file_selector);

   if (ret==3)          /* cancel */
      return 0;
   else
      return -1;        /* OK'ed */
}

