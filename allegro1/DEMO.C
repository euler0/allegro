/*
   DEMO.C:
   The Allegro library demonstration game.

   By Shawn Hargreaves, 1994.
*/


#include "stdlib.h"
#include "string.h"

#include "allegro.h"
#include "demo.h"

#ifdef BORLAND
short _RTLENTRY _EXPFUNC sprintf(char _FAR *__buffer, const char _FAR *__format, ...);
#else
int sprintf(char *, const char*, ...);
#define rand() random()
#endif

short title_screen(void);
void play_game(void);
void draw_screen(void);

DATAFILE *data;

BITMAP *screen2, *s;

PALLETE title_pallete;

long score;
char score_buf[80];
short dead;
short pos;
short xspeed, yspeed, ycounter;
short ship_state;
short shot;
short shot_delay;
short frame_count, fps;

#define MAX_SPEED       32
#define SPEED_SHIFT     3

#define MAX_STARS       100

struct {
   fixed x, y, z;
   short ox, oy;
} star[MAX_STARS];

#define MAX_ALIENS      20

struct {
   short x, y;
   short d;
   short state;
   short shot;
   short shot_delay;
} alien[MAX_ALIENS];

short alien_count;

#define MAX_BULLETS     20
#define BULLET_DELAY    8
#define BULLET_SPEED    6

short bullet_count;
short bullet_delay;

struct {
   short x, y; 
} bullet[MAX_BULLETS];



void game_control()
{
   register short c, c2;

   if (shot) {
      if (++shot_delay > 8) {
	 shot_delay = 0;
	 ship_state++;
	 if (ship_state >= EXPLODE5)
	    dead = TRUE;
      }
      if (yspeed)
	 yspeed--;
   }

   else { 
      if (key[KEY_LEFT]) {
	 if (xspeed > -MAX_SPEED)
	    xspeed-=2;
      }
      else
	 if (key[KEY_RIGHT]) {
	    if (xspeed < MAX_SPEED)
	       xspeed+=2;
	 }
	 else
	    if (xspeed > 0)
	       xspeed-=2;
	    else
	       if (xspeed < 0)
		  xspeed+=2;
   
      pos += xspeed;
      if (pos < 0)
	 pos = xspeed = 0;
      else
	 if (pos >= ((SCREEN_W-32)<<SPEED_SHIFT)) {
	    pos = ((SCREEN_W-32)<<SPEED_SHIFT);
	    xspeed = 0;
	 }
   
      if (key[KEY_UP]) {
	 if (yspeed < MAX_SPEED)
	    yspeed++;
	 ship_state = SHIP_GO;
      }
      else {
	 ship_state = SHIP_STILL;
	 if (yspeed)
	    yspeed--;
      }
   }

   if (yspeed) {
      ycounter += yspeed;
      while (ycounter >= (1<<SPEED_SHIFT)) {
	 for (c=0; c<bullet_count; c++)
	    bullet[c].y++;
	 for (c=0; c<MAX_STARS; c++)
	    if (++star[c].oy >= SCREEN_H)
	       star[c].oy = 0;
	 for (c=0; c<alien_count; c++)
	    alien[c].y++;
	 ycounter -= (1<<SPEED_SHIFT);
      }
   }

   for (c=0; c<bullet_count; c++) {
      bullet[c].y -= BULLET_SPEED;
      if (bullet[c].y < 8) {
	 for (c2=c; c2<bullet_count-1; c2++)
	    bullet[c2] = bullet[c2+1];
	 bullet_count--;
      }
      else {
	 for (c2=0; c2<alien_count; c2++) {
	    if ((bullet[c].y < alien[c2].y+30) &&
		(bullet[c].y > alien[c2].y) &&
		(bullet[c].x+30 > alien[c2].x) &&
		(bullet[c].x < alien[c2].x+30) &&
		(!alien[c2].shot)) {
	       alien[c2].shot = TRUE;
	       alien[c2].state = EXPLODE1;
	       for (c2=c; c2<bullet_count-1; c2++)
		  bullet[c2] = bullet[c2+1];
	       bullet_count--;
	       break; 
	    }
	 }
      }
   }

   for (c=0; c<MAX_STARS; c++) {
      if ((star[c].oy+=((short)star[c].z>>1)+1) >= SCREEN_H)
	 star[c].oy = 0;
   }

   if (!shot) {
      if (bullet_delay)
	 bullet_delay--;
      else {
	 if (key[KEY_SPACE]) {
	    if (bullet_count < MAX_BULLETS) {
	       bullet_delay = BULLET_DELAY;
	       bullet[bullet_count].x = (pos>>SPEED_SHIFT);
	       bullet[bullet_count].y = SCREEN_H-32;
	       bullet_count++;
	    } 
	 }
      }
   }

   for (c=0; c<alien_count; c++) {
      if (alien[c].shot) {
	 if (++alien[c].shot_delay > 2) {
	    alien[c].shot_delay = 0;
	    if (alien[c].state < EXPLODE5)
	       alien[c].state++;
	    else {
	       alien[c].x = rand() % (SCREEN_W-32);
	       alien[c].y = -32 - (rand() & 0x3f);
	       alien[c].d = (rand()&1) ? 1 : -1;
	       alien[c].shot = FALSE;
	       alien[c].state = ALIEN;
	       score += 4;
	    }
	 }
      }
      else {
	 alien[c].x += alien[c].d;
	 if (alien[c].x < -32)
	    alien[c].x = SCREEN_W;
	 else
	    if (alien[c].x > SCREEN_W)
	       alien[c].x = - 32;
      }
      alien[c].y++;
      if (alien[c].y > SCREEN_H-60) {
	 if (alien[c].y > SCREEN_H) {
	    if (!alien[c].shot) {
	       alien[c].x = rand() % (SCREEN_W-32);
	       alien[c].y = -32 - (rand() & 0x3f);
	       alien[c].d = (rand()&1) ? 1 : -1;
	    }
	 }
	 else {
	    if ((alien[c].x+30 >= (pos>>SPEED_SHIFT)) &&
		(alien[c].x < (pos>>SPEED_SHIFT)+30) &&
		(alien[c].y < SCREEN_H-24)) {
	       if (!shot) {
		  ship_state = EXPLODE1;
		  shot = TRUE;
		  shot_delay = 0;
	       }
	       if (!alien[c].shot) {
		  alien[c].shot = TRUE;
		  alien[c].state = EXPLODE1;
	       }
	    }
	 }
      } 
   }
}



void score_proc()
{
   fps = frame_count;
   frame_count = 0;
   score++;
   if ((score>>6) > alien_count)
      if (alien_count < MAX_ALIENS)
	 alien_count++;
}



void draw_screen()
{
   register short c;
   
   clear(s);

   for (c=0; c<MAX_STARS; c++)
      putpixel(s, star[c].ox, star[c].oy, 15-(short)star[c].z);

   drawsprite(s, data[ship_state].dat, pos>>SPEED_SHIFT, SCREEN_H-32); 

   for (c=0; c<alien_count; c++)
      drawsprite(s, data[alien[c].state].dat, alien[c].x, alien[c].y);

   for (c=0; c<bullet_count; c++)
      drawsprite(s, data[BULLET].dat, bullet[c].x, bullet[c].y);

   if (fps)
      sprintf(score_buf, "Score: %ld - (%ld frames per second)", (long)score, (long)fps);
   else
      sprintf(score_buf, "Score: %ld", (long)score);

   textout(s, data[TITLE_FONT].dat, score_buf, 0, 0, 6);

   show(s);
}



void play_game()
{
   short c;
   
   dead = shot = FALSE;
   pos = (SCREEN_W/2-16)<<SPEED_SHIFT;
   xspeed = yspeed = ycounter = 0;
   ship_state = SHIP_STILL;
   frame_count = fps = 0;
   bullet_count = bullet_delay = 0;
   score = 0;
   for (c=0; c<MAX_STARS; c++) {
      star[c].ox = rand() % SCREEN_W;
      star[c].oy = rand() % SCREEN_H;
      star[c].z = rand() & 7;
   }
   for (c=0; c<MAX_ALIENS; c++) {
      alien[c].x = rand() % (SCREEN_W-32);
      alien[c].y = -32 - (rand() & 0x3f);
      alien[c].d = (rand()&1) ? 1 : -1;
      alien[c].state = ALIEN;
      alien[c].shot = FALSE;
      alien[c].shot_delay = 0;
   }
   alien_count = 2;

   draw_screen();
   fade_in(data[GAME_PAL].dat, 3);
   install_int(score_proc, 1000);
   install_int(game_control, 20);
   
   while (!dead) {
      draw_screen();
      frame_count++;
      if (key[KEY_ESC]) {
	 remove_int(score_proc);
	 remove_int(game_control);
	 do {
	 } while (key[KEY_ESC]);
	 fade_out(3);
	 return;
      } 
   }
   
   remove_int(game_control);
   remove_int(score_proc);

   fps = 0;
   draw_screen();
   circlefill(screen, 160, 100, 64, 0);
   textout(screen, data[TITLE_FONT].dat, "GAME OVER", 124, 88, 6);
   textout(screen, data[TITLE_FONT].dat, score_buf, 160-strlen(score_buf)*4, 104, 6);

   clear_keybuf();
   readkey();
   clear_keybuf();

   fade_out(3);
}



short text_pos, pix_pos;


void scroll_int()
{
   static char buf[43];
   short buf_pos;
   char *d = (char *)data[TITLE_TEXT].dat+text_pos;

   if (++pix_pos >= 8) {
      pix_pos = 0;
      for (buf_pos=0; buf_pos<41; buf_pos++)
	 buf[buf_pos] = d[buf_pos];

      buf[buf_pos++] = ' ';
      buf[buf_pos] = 0;

      if (++text_pos >= data[TITLE_TEXT].size-42)
	 text_pos = 0;
   }
   
   textout(screen, data[TITLE_FONT].dat, buf, -pix_pos, 180, 7);
}



short title_screen()
{
   short c;
   register fixed x, y;
   register short ix, iy;
   short star_count = 0;
   short star_count_count = 0;
   RGB rgb;
   static short color = 0;

   text_pos = 0;
   pix_pos = 8;

   for (c=0; c<MAX_STARS; c++) {
      star[c].z = 0;
      star[c].ox = star[c].oy = -1;
   }

   for (c=0; c<8; c++)
      title_pallete[c] = ((_RGB *)data[TITLE_PAL].dat)[c];
   for (c=8; c<PAL_SIZE; c++) {
      rgb = paltorgb(((_RGB *)data[TITLE_PAL].dat)[c]);
      switch(color) {
	 case 0:
	    rgb.b = rgb.r;
	    rgb.r = 0;
	    break;
	 case 1:
	    rgb.g = rgb.r;
	    rgb.r = 0;
	    break;
	 case 3:
	    rgb.g = rgb.r;
	    break;
      }
      title_pallete[c] = rgbtopal(rgb);
   }
   color++;
   if (color > 3)
      color = 0;

   blit(data[TITLE_BMP].dat,screen,0,0,0,0,SCREEN_W,SCREEN_H);
   fade_in(title_pallete, 1);

   install_int(scroll_int, 10);

   clear_keybuf();

   do {
      for (c=0; c<MAX_STARS; c++) {
	 if (star[c].z <= itofix(1)) {
	    if (rand()&1) {
	       if (rand()&1)
		  star[c].x = -itofix(SCREEN_W/2);
	       else
		  star[c].x = itofix(SCREEN_W/2);
	       star[c].y = itofix((rand() % SCREEN_H) - SCREEN_H/2);
	    }
	    else {
	       if (rand()&1)
		  star[c].y = -itofix(SCREEN_H/2);
	       else
		  star[c].y = itofix(SCREEN_H/2);
	       star[c].x = itofix((rand() % SCREEN_W) - SCREEN_W/2);
	    }
	    star[c].z = itofix((rand() & 0xf) + 1);
	 }
	 x = fdiv(star[c].x, star[c].z);
	 y = fdiv(star[c].y, star[c].z);
	 ix = (short)(x>>16) + SCREEN_W/2;
	 iy = (short)(y>>16) + SCREEN_H/2;
	 putpixel(screen, star[c].ox, star[c].oy, 0);
	 if (getpixel(screen, ix, iy)==0) {
	    if (c < star_count)
	       putpixel(screen, ix, iy, 7-(short)(star[c].z>>17));
	    star[c].ox = ix;
	    star[c].oy = iy;
	 }
#ifdef GCC
	 star[c].z -= 1024;
#else
	 star[c].z -= 4096;
#endif
      }
      if (star_count < MAX_STARS) {
	 if (star_count_count++ >= 32) {
	    star_count_count = 0;
	    star_count++;
	 }
      }

   } while (!keypressed());

   remove_int(scroll_int);
   fade_out(3);

   while (keypressed())
      if ((readkey() & 0xff) == 27)
	 return FALSE;

   return TRUE;
}



void main()
{
   allegro_init();
   install_keyboard();
   set_pallete(black_pallete);
   data = load_datafile("DEMO.DAT");
   if (data) {
      screen2 = s = create_bitmap(SCREEN_W, SCREEN_H);
      if (screen2) {
	 while (title_screen())
	    play_game(); 
	 destroy_bitmap(screen2);
      }
      else {
	 set_pallete(desktop_pallete);
	 alert("Out of memory!", NULL, NULL, "Oh dear", NULL, 13, 0);
      }
      unload_datafile(data);
   }
   else {
      set_pallete(desktop_pallete);
      alert("Error loading DEMO.DAT", NULL, NULL, "Oh dear", NULL, 13, 0);
   } 
   allegro_exit();
   exit(0);
}


