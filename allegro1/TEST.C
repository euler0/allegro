/*
   TEST.C:
   A test program for the Allegro library graphics routines.

   By Shawn Hargreaves, 1994.
*/


#include "stdlib.h"
#include "string.h"

#include "allegro.h"

#ifdef BORLAND
short _RTLENTRY _EXPFUNC sprintf(char _FAR *__buffer, const char _FAR *__format, ...);
#else
int sprintf(char *, const char*, ...);
#define rand() random()
#endif

void message(char *s);
void make_sprite(short flags, short size);
void putpix_demo(void);
void putpix_test(short xpos, short ypos);
void hline_demo(void);
void hline_test(short xpos, short ypos);
void vline_demo(void);
void vline_test(short xpos, short ypos);
void line_demo(void);
void line_test(short xpos, short ypos);
void rect_test(short xpos, short ypos);
void rect_demo(void);
void rectfill_test(short xpos, short ypos);
void rectfill_demo(void);
void triangle_demo(void);
void triangle_test(short xpos, short ypos);
void circle_test(short xpos, short ypos);
void circle_demo(void);
void circlefill_test(short xpos, short ypos);
void circlefill_demo(void);
void textout_test(short xpos, short ypos);
void textout_demo(void);
void sprite_test(short xpos, short ypos);
void sprite_demo(void);
void getpix_demo(void);
void blit_demo(short clip_flag);
void do_sprite_stuff(short size);
void misc(void);
void mouse_test(void);
void keyboard_test(void);
void interrupt_test(void);
void show_time(long ct, BITMAP *bmp, short y);
void test_it(char *msg, void (*func)(short, short));
void do_it(char *msg, short clip_flag, void (*func)());
void do_option(short c);
short next(void);

#define TIME_SPEED   4


#define O_W          90
#define O_H          16

DIALOG title_screen[] =
{
   { d_button_proc, 115, 180, O_W, O_H, 15, 0, 27, D_EXIT, 0, 0, "Quit" },
   { d_button_proc, 10,  50,  O_W, O_H, 15, 1, 'p', D_EXIT, 0, 0, "putpixel" },
   { d_button_proc, 115, 50,  O_W, O_H, 15, 2, 'g', D_EXIT, 0, 0, "getpixel" },
   { d_button_proc, 220, 50,  O_W, O_H, 15, 3, 'h', D_EXIT, 0, 0, "hline" },
   { d_button_proc, 10,  70,  O_W, O_H, 15, 4, 'v', D_EXIT, 0, 0, "vline" },
   { d_button_proc, 115, 70,  O_W, O_H, 15, 5, 'l', D_EXIT, 0, 0, "line", },
   { d_button_proc, 220, 70,  O_W, O_H, 15, 6, 0, D_EXIT, 0, 0, "rect" },
   { d_button_proc, 10,  90,  O_W, O_H, 15, 10, 'r', D_EXIT, 0, 0, "rectfill" },
   { d_button_proc, 115, 90,  O_W, O_H, 15, 11, 't', D_EXIT, 0, 0, "triangle" },
   { d_button_proc, 220, 90,  O_W, O_H, 15, 12, 0, D_EXIT, 0, 0, "circle" },
   { d_button_proc, 10,  110, O_W, O_H, 15, 13, 'c', D_EXIT, 0, 0, "circlefill" },
   { d_button_proc, 115, 110, O_W, O_H, 15, 14, 0, D_EXIT, 0, 0, "textout" },
   { d_button_proc, 220, 110, O_W, O_H, 15, 1, 'b', D_EXIT, 0, 0, "blit" },
   { d_button_proc, 10,  130, O_W, O_H, 15, 2, 's', D_EXIT, 0, 0, "sprite 16" },
   { d_button_proc, 115, 130, O_W, O_H, 15, 3, 0, D_EXIT, 0, 0, "sprite 32" },
   { d_button_proc, 220, 130, O_W, O_H, 15, 4, 'm', D_EXIT, 0, 0, "Misc" },
   { d_button_proc, 10,  150, O_W, O_H, 15, 5, 0, D_EXIT, 0, 0, "Mouse" },
   { d_button_proc, 115, 150, O_W, O_H, 15, 6, 'k', D_EXIT, 0, 0, "Keyboard" },
   { d_button_proc, 220, 150, O_W, O_H, 15, 10, 'i', D_EXIT, 0, 0, "Interrupts" },
#ifdef BORLAND
   { d_text_proc, 40, 10, 1, 1, 15, 0, 0, 0, 0, 0, "Allegro Test Program (Borland)" },
#else
   { d_text_proc, 48, 10, 1, 1, 15, 0, 0, 0, 0, 0, "Allegro Test Program (djgpp)" },
#endif
   { d_text_proc, 60, 26, 1, 1, 15, 0, 0, 0, 0, 0, "By Shawn Hargreaves, 1994" },
   { NULL }
};



SPRITE *sprite = NULL;

short sprite_align = FALSE;


void make_sprite(flags, size)
short flags;
short size;
{
   BITMAP *bmp;

   if (sprite)
      destroy_sprite(sprite);

   sprite = create_sprite(flags,size,size);

   bmp = create_bitmap(size,size);
   clear(bmp);
   rectfill(bmp, size/2-7, size/2-4, size/2+7, size/2+4, 6);
   line(bmp,0,0,size,size,1);
   line(bmp,0,size,size,0,1);
   circle(bmp,size>>1,size>>1,(size>>1)-1,2);
   circle(bmp,size>>1,size>>1,size>>2,3);
   if (size==32)
      circle(bmp,16,16,11,4);
   textmode(-1);
   textout(bmp, font, (size==16) ? "16" : "32", size/2-7, size/2-3, 15);

   get_sprite(sprite,bmp,0,0);

   destroy_bitmap(bmp);
}



long tm = 0;        /* counter, incremented once a second */
short _tm = 0;


void tm_tick()
{
   if (++_tm >= 10) {
      _tm = 0;
      tm++;
   }
}


char buf[80];


void show_time(t,bmp,y)
long t;
BITMAP *bmp;
short y;
{
   short cf, cl, ct, cr, cb;

   cf = bmp->clip;
   cl = bmp->cl;
   cr = bmp->cr;
   ct = bmp->ct;
   cb = bmp->cb;

   sprintf(buf,"%ld per second", t / TIME_SPEED);
   set_clip(bmp,0,0,SCREEN_W,SCREEN_H);
   textout(bmp,font,buf,160-strlen(buf)*4,y,15);
   bmp->clip = cf;
   bmp->cl = cl;
   bmp->cr = cr;
   bmp->ct = ct;
   bmp->cb = cb;
}



void message(s)
char *s;
{
   textout(screen,font,s,160-strlen(s)*4,6,15);
   textout(screen,font,"Press a key or mouse button",52,190,15);
}



short next()
{
   if (keypressed()) {
      clear_keybuf();
      return TRUE;
   }
   if (mouse_b) {
      do {
      } while (mouse_b);
      return TRUE;
   }

   return FALSE;
}



void getpix_demo()
{
   register short c;

   set_clip(screen,0,0,0,0);
   clear(screen); 
   message("getpixel test");

   for (c=0; c<16; c++)
      rectfill(screen, 100+((c&3)<<5), 50+((c>>2)<<5),
		       120+((c&3)<<5), 70+((c>>2)<<5), c);

   show_mouse(screen);

   while (!next()) {
      c = getpixel(screen,mouse_x-2,mouse_y-2);
      sprintf(buf," %d ", c);
      textout(screen,font,buf,160-strlen(buf)*4,24,15);
   }

   show_mouse(NULL);
}



void putpix_test(xpos,ypos)
short xpos, ypos;
{
   register short c = 0;
   register short x, y;

   for (c=0; c<16; c++)
      for (x=0; x<16; x+=2)
	 for (y=0; y<16; y+=2)
	    putpixel(screen, xpos+((c&3)<<4)+x, ypos+((c>>2)<<4)+y, c);
}



void hline_test(xpos,ypos)
short xpos, ypos;
{
   register short c;

   for (c=0; c<16; c++)
      hline(screen, xpos+48-c*4, ypos+c*3, xpos+48+c*4, c);
}



void vline_test(xpos,ypos)
short xpos, ypos;
{
   register short c;

   for (c=0; c<16; c++)
      vline(screen, xpos+c*4, ypos+36-c*3, ypos+36+c*3, c); 
}



void line_test(xpos,ypos)
short xpos, ypos;
{
   register short c;

   for (c=0; c<16; c++) {
      line(screen, xpos+32, ypos+32, xpos+32+((c-8)<<2), ypos+32-(8<<2), c);
      line(screen, xpos+32, ypos+32, xpos+32+((c-8)<<2), ypos+32+(8<<2), c);
      line(screen, xpos+32, ypos+32, xpos+32-(8<<2), ypos+32+((c-8)<<2), c);
      line(screen, xpos+32, ypos+32, xpos+32+(8<<2), ypos+32+((c-8)<<2), c);
   }
}



void rect_test(xpos,ypos)
short xpos, ypos;
{
   register short c;

   for (c=0; c<16; c++) {
      rect(screen, xpos+32-c*2, ypos+32-c*2, xpos+32+c*2, ypos+32+c*2, c);
   }
}



void rectfill_test(xpos,ypos)
short xpos, ypos;
{
   register short c;

   for (c=0; c<16; c++) {
      rectfill(screen, xpos+((c&3)*17), ypos+((c>>2)*17),
		       xpos+15+((c&3)*17), ypos+15+((c>>2)*17), c);
   }
}



void triangle_test(xpos,ypos)
short xpos, ypos;
{
   register short c;

   for (c=0; c<16; c++) {
      triangle(screen, xpos+22+((c&3)<<4), ypos+15+((c>>2)<<4),
		       xpos+13+((c&3)<<3), ypos+7+((c>>2)<<4),
		       xpos+7+((c&3)<<4), ypos+27+((c>>2)<<3), c);
   }
}



void circle_test(xpos,ypos)
short xpos, ypos;
{
   register short c;

   for (c=0; c<16; c++) {
      circle(screen, xpos+32, ypos+32, (c<<1), c);
   }
}



void circlefill_test(xpos,ypos)
short xpos, ypos;
{
   register short c;

   for (c=15; c>=0; c--) {
      circlefill(screen, xpos+8+((c&3)<<4), ypos+8+((c>>2)<<4), c+1, c);
   }
}



void textout_test(xpos,ypos)
short xpos, ypos;
{
   textmode(0);
   textout(screen,font,"This is a",xpos-8,ypos,1);
   textout(screen,font,"test of the",xpos+3,ypos+10,1);
   textout(screen,font,"textout",xpos+14,ypos+20,1);
   textout(screen,font,"function.",xpos+25,ypos+30,1);

   textout(screen,font,"textmode(0)",xpos,ypos+48,2);
   textout(screen,font,"textmode(0)",xpos+4,ypos+52,4);

   textmode(-1);
   textout(screen,font,"textmode(-1)",xpos,ypos+68,2);
   textout(screen,font,"textmode(-1)",xpos+4,ypos+72,4);
}



void sprite_test(xpos,ypos)
short xpos, ypos;
{
   short x,y;

   for (y=0;y<82;y++)
      for (x=0;x<82;x+=2)
	 putpixel(screen,xpos+x+(y&1),ypos+y,8);

   for (x=6; x<64; x+=sprite->w+6)
      for (y=6; y<64; y+=sprite->w+6)
	 drawsprite(screen,sprite,xpos+x,ypos+y);
}



void putpix_demo()
{
   register short c = 0;
   register short x, y;
   register long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (rand() & 255) + 32;
      y = (rand() & 127) + 40;
      putpixel(screen,x,y,c);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct,screen,16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void hline_demo()
{
   register short c = 0;
   register short x1, x2, y;
   register long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x1 = (rand() & 255) + 32;
      x2 = (rand() & 255) + 32;
      y = (rand() & 127) + 40;
      hline(screen,x1,y,x2,c);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct,screen,16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void vline_demo()
{
   register short c = 0;
   register short x, y1, y2;
   register long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (rand() & 255) + 32;
      y1 = (rand() & 127) + 40;
      y2 = (rand() & 127) + 40;
      vline(screen,x,y1,y2,c);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct,screen,16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void line_demo()
{
   register short c = 0;
   register short x1, y1, x2, y2;
   register long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x1 = (rand() & 255) + 32;
      x2 = (rand() & 255) + 32;
      y1 = (rand() & 127) + 40;
      y2 = (rand() & 127) + 40;
      line(screen,x1,y1,x2,y2,c);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct,screen,16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void rect_demo()
{
   register short c = 0;
   register short x1, y1, x2, y2;
   register long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x1 = (rand() & 255) + 32;
      y1 = (rand() & 127) + 40;
      x2 = (rand() & 255) + 32;
      y2 = (rand() & 127) + 40;
      rect(screen,x1,y1,x2,y2,c);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct,screen,16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void rectfill_demo()
{
   register short c = 0;
   register short x1, y1, x2, y2;
   register long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x1 = (rand() & 255) + 32;
      y1 = (rand() & 127) + 40;
      x2 = (rand() & 255) + 32;
      y2 = (rand() & 127) + 40;
      rectfill(screen,x1,y1,x2,y2,c);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct,screen,16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void triangle_demo()
{
   register short c = 0;
   register short x1, y1, x2, y2, x3, y3;
   register long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x1 = (rand() & 255) + 32;
      x2 = (rand() & 255) + 32;
      x3 = (rand() & 255) + 32;
      y1 = (rand() & 127) + 40;
      y2 = (rand() & 127) + 40;
      y3 = (rand() & 127) + 40;
      triangle(screen,x1,y1,x2,y2,x3,y3,c);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct,screen,16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void circle_demo()
{
   register short c = 0;
   register short x, y, r;
   register long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (rand() & 127) + 92;
      y = (rand() & 63) + 76;
      r = (rand() & 31) + 16;
      circle(screen,x,y,r,c);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct,screen,16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void circlefill_demo()
{
   register short c = 0;
   register short x, y, r;
   register long ct;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (rand() & 127) + 92;
      y = (rand() & 63) + 76;
      r = (rand() & 31) + 16;
      circlefill(screen,x,y,r,c);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct,screen,16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void textout_demo()
{
   register short c = 0;
   register short x, y;
   register long ct;

   tm = 0; _tm = 0;
   ct = 0;

   textmode(0);

   while (!next()) {

      x = (rand() & 127) + 40;
      y = (rand() & 127) + 40;
      textout(screen,font,"textout test",x,y,c);

      if (++c >= 16)
	 c = 0;

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct,screen,16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void sprite_demo(){
   register long ct;
   register short x, y;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      x = (rand() & 255) + 16;
      if (sprite_align)
	 x &= 0xfff0;
      y = (rand() & 127) + 30;
      drawsprite(screen,sprite,x,y);

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct,screen,16);
	    ct = -1;
	 }
	 else
	    ct++;
      }
   }
}



void blit_demo(clip_flag)
short clip_flag;
{
   register long ct;
   register BITMAP *b, *b2, *s;
   register short ox, oy;
   short x1, x2, y1, y2;

   set_clip(screen,0,0,SCREEN_W-1,SCREEN_H-1);

   if (clip_flag) {
      do {
	 y1 = (rand() & 127) + 40;
	 y2 = (rand() & 127) + 40;
      } while (abs(y1-y2) < 20);
      if (y1 > y2) {
	 x1 = y1;
	 y1 = y2;
	 y2 = x1;
      }
      do {
	 x1 = (rand() & 255) + 32;
	 x2 = (rand() & 255) + 32;
      } while (abs(x1-x2) < 30);
   }
   else {
      x1 = y1 = 0;
      x2 = SCREEN_W-1;
      y2 = SCREEN_H-1;
   }

   b = create_bitmap(64,32);
   if (!b) {
      clear(screen);
      textout(screen,font,"Out of memory!",50,50,15);
      while(!next())
	 ;
      return;
   }

   b2 = create_bitmap(64,32);
   if (!b2) {
      clear(screen);
      textout(screen,font,"Out of memory!",50,50,15);
      destroy_bitmap(b);
      while(!next())
	 ;
      return;
   }

   s = create_bitmap(SCREEN_W,SCREEN_H);
   if (!s) {
      destroy_bitmap(b);
      destroy_bitmap(b2);
      clear(screen);
      textout(screen,font,"Out of memory!",50,50,15);
      while(!next())
	 ;
      return;
   }

   clear(b);
   circlefill(b,32,16,16,4);
   circlefill(b,32,16,10,2);
   circlefill(b,32,16,6,1);
   line(b,0,0,63,31,5);
   line(b,0,31,63,0,5);
   rect(b,8,4,56,28,3);
   rect(b,0,0,63,31,15);

   clear(s); 
   set_clip(s,x1,MAX(y1,28),x2,y2);
   line(s,0,0,SCREEN_W-1,SCREEN_H-1,15);
   line(s,0,SCREEN_H-1,SCREEN_W-1,0,15);
   for(ox=0;ox<16;ox++)
      circle(s,160,100,((ox+1)<<3),ox);

   set_clip(s,0,0,SCREEN_W-1,SCREEN_H-1);
   if (clip_flag)
      textout(s,font,"timing blit [clipped]",76,6,15);
   else
      textout(s,font,"timing blit",116,6,15);
   textout(s,font,"(1 320x200 & 3 64x32 blits per cycle)",12,16,15);
   textout(s,font,"Press a key or mouse button",52,190,15);
   set_clip(s,x1,MAX(y1,28),x2,y2);

   ox = -160;
   oy = -100;

   tm = 0; _tm = 0;
   ct = 0;

   while (!next()) {

      set_clip(s,0,0,SCREEN_W-1,SCREEN_H-1);
      blit(b2,s,0,0,ox,oy,64,32);

      if (ct >= 0) {
	 if (tm >= TIME_SPEED) {
	    show_time(ct,s,28);
	    ct = -1;
	 }
	 else
	    ct++;
      }

      ox = mouse_x - 32;
      oy = mouse_y - 16;
      blit(s,b2,ox,oy,0,0,64,32);
      putpixel(s,ox+32,oy+16,15);
      set_clip(s,x1,y1,x2,y2);
      blit(b,s,0,0,ox,oy,64,32);
      show(s);
   }

   destroy_bitmap(b);
   destroy_bitmap(b2);
   destroy_bitmap(s);
}



void misc()
{
   register BITMAP *p;
   register long ct;
   register fixed x, y, z;

   clear(screen);
   textout(screen,font,"Timing some other routines...",44,6,15);

   p = create_bitmap(SCREEN_W,SCREEN_H);
   if (!p)
      textout(screen,font,"Out of memory!",16,50,15);
   else {
      tm = 0; _tm = 0;
      ct = 0;
      do {
	 clear(p);
	 ct++;
      } while (tm < TIME_SPEED);
      destroy_bitmap(p);
      sprintf(buf,"clear(320x200): %ld per second", ct/TIME_SPEED);
      textout(screen,font,buf,16,50,15);
   }

   if (next())
      return;

   x = y = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      z = fmul(x,y);
      x += 1317;
      y += 7143;
      ct++;
   } while (tm < TIME_SPEED);

   sprintf(buf,"fmul(): %ld per second", ct/TIME_SPEED);
   textout(screen,font,buf,16,60,15);

   if (next())
      return;

   x = y = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      z = fdiv(x,y);
      x += 1317;
      y += 7143;
      if (y==0)
	 y++;
      ct++;
   } while (tm < TIME_SPEED);

   sprintf(buf,"fdiv(): %ld per second", ct/TIME_SPEED);
   textout(screen,font,buf,16,70,15);

   if (next())
      return;

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = fsqrt(x);
      x += 7361;
      ct++;
   } while (tm < TIME_SPEED);

   sprintf(buf,"fsqrt(): %ld per second", ct/TIME_SPEED);
   textout(screen,font,buf,16,80,15);

   if (next())
      return;

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = fsin(x);
      x += 4283;
      ct++;
   } while (tm < TIME_SPEED);

   sprintf(buf,"fsin(): %ld per second", ct / TIME_SPEED);
   textout(screen,font,buf,16,90,15);

   if (next())
      return;

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = fcos(x);
      x += 4283;
      ct++;
   } while (tm < TIME_SPEED);

   sprintf(buf,"fcos(): %ld per second", ct / TIME_SPEED);
   textout(screen,font,buf,16,100,15);

   if (next())
      return;

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = ftan(x);
      x += 8372;
      ct++;
   } while (tm < TIME_SPEED);

   sprintf(buf,"ftan(): %ld per second", ct / TIME_SPEED);
   textout(screen,font,buf,16,110,15);

   if (next())
      return;

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = fasin(x);
      x += 5621;
      x &= 0xffff;
      ct++;
   } while (tm < TIME_SPEED);

   sprintf(buf,"fasin(): %ld per second", ct / TIME_SPEED);
   textout(screen,font,buf,16,120,15);

   if (next())
      return;

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = facos(x);
      x += 5621;
      x &= 0xffff;
      ct++;
   } while (tm < TIME_SPEED);

   sprintf(buf,"facos(): %ld per second", ct / TIME_SPEED);
   textout(screen,font,buf,16,130,15);

   if (next())
      return;

   x = 1;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      y = fatan(x);
      x += 7358;
      ct++;
   } while (tm < TIME_SPEED);

   sprintf(buf,"fatan(): %ld per second", ct / TIME_SPEED);
   textout(screen,font,buf,16,140,15);

   if (next())
      return;

   x = 1, y = 2;
   tm = 0; _tm = 0;
   ct = 0;

   do {
      z = fatan2(x, y);
      x += 5621;
      y += 7335;
      ct++;
   } while (tm < TIME_SPEED);

   sprintf(buf,"fatan2(): %ld per second", ct / TIME_SPEED);
   textout(screen,font,buf,16,150,15);

   textout(screen,font,"Press a key or mouse button",52,190,15);

   while(!next())
      ;
}



void mouse_test()
{
   clear(screen);
   textout(screen,font,"Mouse test",120,6,15);
   textout(screen,font,"Press a key",116,190,15);

   do {
      sprintf(buf,"X=%3d, Y=%3d", mouse_x, mouse_y);
      if (mouse_b & 1)
	 strcat(buf," left");
      else
	 strcat(buf,"     ");
      if (mouse_b & 2)
	 strcat(buf," right"); 
      else
	 strcat(buf,"      ");
      textout(screen,font,buf,68,86,15);
   } while (!keypressed());

   clear_keybuf();
}



void keyboard_test()
{
   long k;

   clear(screen);
   textout(screen,font,"Keyboard test",100,6,15);
   textout(screen,font,"Press a mouse button",80,190,15);

   do {
      if (keypressed()) {
	 blit(screen,screen,0,48,0,40,320,112);
	 k = readkey();
	 sprintf(buf,"%ld - 0x%lx            ", k, k);
	 textout(screen,font,buf,80,152,15);
      }
   } while (!mouse_b);

   do {
   } while (mouse_b);
}



void short1()
{
   static short c = 0;

   if (++c >= 16)
      c = 0;

   rectfill(screen,110,90,130,110,c);
}



void short2()
{
   static short c = 0;

   if (++c >= 16)
      c = 0;

   rectfill(screen,150,90,170,110,c);
}



void short3()
{
   static short c = 0;

   if (++c >= 16)
      c = 0;

   rectfill(screen,190,90,210,110,c);
}



void interrupt_test()
{
   clear(screen);
   message("Interrupt test");

   textout(screen,font,"1/4",108,78,15);
   textout(screen,font,"1",156,78,15);
   textout(screen,font,"5",196,78,15);

   install_int(short1,250);
   install_int(short2,1000);
   install_int(short3,5000);

   while (!next())
      ;

   remove_int(short1);
   remove_int(short2);
   remove_int(short3);
}



void do_sprite_stuff(size)
short size;
{
   make_sprite(SPRITE_OPAQUE, size);
   test_it("opaque sprite test", sprite_test);

   make_sprite(SPRITE_MASKED, size);
   test_it("masked sprite test", sprite_test);

   make_sprite(SPRITE_OPAQUE, size);
   do_it("timing opaque sprite", FALSE, sprite_demo);

   sprite_align = TRUE;
   do_it("timing opaque word-aligned sprite", FALSE, sprite_demo);
   sprite_align = FALSE;

   make_sprite(SPRITE_MASKED, size);
   do_it("timing masked sprite", FALSE, sprite_demo);

   make_sprite(SPRITE_OPAQUE, size);
   do_it("timing opaque sprite [clipped]", TRUE, sprite_demo);

   make_sprite(SPRITE_MASKED, size);
   do_it("timing masked sprite [clipped]", TRUE, sprite_demo);

   destroy_sprite(sprite);
   sprite = NULL;
}



void test_it(msg, func)
char *msg;
void (*func)(short, short);
{ 
   clear(screen); 
   set_clip(screen,0,0,0,0);
   message(msg);

   textout(screen,font,"unclipped:",48,50,15);
   (*func)(60,70);

   textout(screen,font,"clipped:",180,62,15);
   rect(screen,191,83,240,104,15);
   set_clip(screen,192,84,239,103);
   (*func)(180,70);

   while (!next())
      ;
}



void do_it(msg, clip_flag, func)
char *msg;
short clip_flag;
void (*func)();
{ 
   register short x1, y1, x2, y2;

   set_clip(screen,0,0,0,0);
   clear(screen);
   message(msg);

   if (clip_flag) {
      do {
	 x1 = (rand() & 255) + 32;
	 x2 = (rand() & 255) + 32;
      } while (abs(x1-x2) < 30);
      do {
	 y1 = (rand() & 127) + 40;
	 y2 = (rand() & 127) + 40;
      } while (abs(y1-y2) < 20);
      set_clip(screen,x1,y1,x2,y2);
   }

   (*func)();
}



void do_option(c)
short c;
{
   switch(c) {
      case 1:
	 test_it("putpixel test", putpix_test);
	 do_it("timing putpixel", FALSE, putpix_demo);
	 do_it("timing putpixel [clipped]", TRUE, putpix_demo);
	 break;

      case 2:
	 getpix_demo();
	 break;

      case 3:
	 test_it("hline test", hline_test);
	 do_it("timing hline", FALSE, hline_demo);
	 do_it("timing hline [clipped]", TRUE, hline_demo);
	 break;

      case 4:
	 test_it("vline test", vline_test);
	 do_it("timing vline", FALSE, vline_demo);
	 do_it("timing vline [clipped]", TRUE, vline_demo);
	 break;

      case 5:
	 test_it("line test", line_test);
	 do_it("timing line", FALSE, line_demo);
	 do_it("timing line [clipped]", TRUE, line_demo);
	 break;

      case 6:
	 test_it("rect test", rect_test);
	 do_it("timing rect", FALSE, rect_demo);
	 do_it("timing rect [clipped]", TRUE, rect_demo);
	 break;

      case 7:
	 test_it("rectfill test", rectfill_test);
	 do_it("timing rectfill", FALSE, rectfill_demo);
	 do_it("timing rectfill [clipped]", TRUE, rectfill_demo);
	 break;

      case 8:
	 test_it("triangle test", triangle_test);
	 do_it("timing triangle", FALSE, triangle_demo);
	 do_it("timing triangle [clipped]", TRUE, triangle_demo);
	 break;

      case 9:
	 test_it("circle test", circle_test);
	 do_it("timing circle", FALSE, circle_demo);
	 do_it("timing circle [clipped]", TRUE, circle_demo);
	 break;

      case 10:
	 test_it("circlefill test", circlefill_test);
	 do_it("timing circlefill", FALSE, circlefill_demo);
	 do_it("timing circlefill [clipped]", TRUE, circlefill_demo);
	 break;

      case 11:
	 test_it("textout test", textout_test);
	 do_it("timing textout", FALSE, textout_demo);
	 do_it("timing textout [clipped]", TRUE, textout_demo);
	 break;

      case 12:
	 blit_demo(FALSE);
	 blit_demo(TRUE);
	 break;

      case 13:
	 do_sprite_stuff(16);
	 break;

      case 14:
	 do_sprite_stuff(32);
	 break;

      case 15:
	 misc();
	 break;

      case 16:
	 mouse_test();
	 break;

      case 17:
	 keyboard_test();
	 break;

      case 18:
	 interrupt_test();
	 break;
   }
}



void main()
{
   short c;

   allegro_init();
   set_pallete(desktop_pallete);
   install_int(tm_tick, 100);
   install_mouse();
   install_keyboard();

   do {
      clear(screen);
      c = do_dialog(title_screen);
      textmode(0);
      if (c)
	 do_option(c);
   } while (c);

   allegro_exit();
   exit(0);
}



