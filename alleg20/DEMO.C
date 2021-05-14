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
 *      A simple game demonstrating the use of the Allegro library.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <conio.h>

#include "allegro.h"
#include "demo.h"


DATAFILE *data;

BITMAP *s;

PALLETE title_pallete;

int cheat;

volatile int move_count;

volatile int score;
char score_buf[80];

volatile int dead;
volatile int pos;
volatile int xspeed, yspeed, ycounter;
volatile int ship_state;
volatile int shot;
volatile int shot_delay;
volatile int frame_count, fps;
volatile int skip_speed, skip_count;

#define MAX_SPEED       32
#define SPEED_SHIFT     3

#define MAX_STARS       100

volatile struct {
   fixed x, y, z;
   int ox, oy;
} star[MAX_STARS];

#define MAX_ALIENS      50

volatile struct {
   int x, y;
   int d;
   int state;
   int shot;
   int shot_delay;
} alien[MAX_ALIENS];

volatile int alien_count;
volatile int new_alien_count;

#define ALIEN_BMP_COUNT    16

RLE_SPRITE *alien_bmp[ALIEN_BMP_COUNT];

#define BULLET_SPEED    6

volatile int bullet_flag;
volatile int bullet_x, bullet_y; 

typedef struct RECT {
   int x, y, w, h;
} RECT;

typedef struct DIRTY_LIST {
   int count;
   RECT rect[2*(MAX_ALIENS+MAX_STARS+4)];
} DIRTY_LIST;

DIRTY_LIST dirty, old_dirty;


#define PAN(x)       (((x) * 256) / SCREEN_W)



void game_control()
{
   move_count++;
}

END_OF_FUNCTION(game_control);



void fps_proc()
{
   fps = frame_count;
   frame_count = 0;
   score++;
}

END_OF_FUNCTION(fps_proc);



void move_everyone()
{
   int c;

   /* player dead? */
   if (shot) {
      if (skip_count <= 0) {
	 if (++shot_delay > 8) {
	    shot_delay = 0;
	    if (ship_state >= EXPLODE5)
	       dead = TRUE;
	    else
	       ship_state++;
	 }
	 if (yspeed)
	    yspeed--;
      }
   }

   else { 
      if (skip_count <= 0) {
	 /* moving left */
	 if ((key[KEY_LEFT]) || (joy_left)) {
	    if (xspeed > -MAX_SPEED)
	       xspeed -= 2;
	 }
	 else {
	    /* moving right */
	    if ((key[KEY_RIGHT]) || (joy_right)) {
	       if (xspeed < MAX_SPEED)
		  xspeed += 2;
	    }
	    else
	       /* decelerate */
	       if (xspeed > 0)
		  xspeed -= 2;
	       else
		  if (xspeed < 0)
		     xspeed += 2;
	 }
      }

      /* move player */ 
      pos += xspeed;
      if (pos < 0)
	 pos = xspeed = 0;
      else
	 if (pos >= ((SCREEN_W-32) << SPEED_SHIFT)) {
	    pos = ((SCREEN_W-32) << SPEED_SHIFT);
	    xspeed = 0;
	 }

      if (skip_count <= 0) {
	 /* fire thrusters? */
	 if ((key[KEY_UP]) || (joy_up)) {
	    if (yspeed < MAX_SPEED) {
	       if (yspeed == 0) {
		  play_sample(data[ENGINE_SPL].dat, 0, PAN(pos>>SPEED_SHIFT),
			      1000, TRUE);
	       }
	       else
		  /* fade in sample while speeding up */
		  adjust_sample(data[ENGINE_SPL].dat, yspeed*64/MAX_SPEED, 
				PAN(pos>>SPEED_SHIFT), 1000, TRUE);

	       yspeed++;
	    }
	    else
	       /* adjust pan while the sample is looping */
	       adjust_sample(data[ENGINE_SPL].dat, 64, PAN(pos>>SPEED_SHIFT),
			     1000, TRUE);

	    ship_state = SHIP_GO;
	    score++;
	 }
	 else {
	    ship_state = SHIP_STILL;
	    if (yspeed) {
	       yspeed--;
	       if (yspeed == 0)
		  stop_sample(data[ENGINE_SPL].dat);
	       else
		  /* fade out and reduce frequency when slowing down */
		  adjust_sample(data[ENGINE_SPL].dat, yspeed*64/MAX_SPEED, 
		     PAN(pos>>SPEED_SHIFT), 500+yspeed*500/MAX_SPEED, TRUE);
	    }
	 }
      }
   }

   /* going fast? move everyone else down to compensate */
   if (yspeed) {
      ycounter += yspeed;
      while (ycounter >= (1 << SPEED_SHIFT)) {
	 if (bullet_flag)
	    bullet_y++;
	 for (c=0; c<MAX_STARS; c++)
	    if (++star[c].oy >= SCREEN_H)
	       star[c].oy = 0;
	 for (c=0; c<alien_count; c++)
	    alien[c].y++;
	 ycounter -= (1 << SPEED_SHIFT);
      }
   }

   /* move bullet */
   if (bullet_flag) {
      bullet_y -= BULLET_SPEED;
      if (bullet_y < 8)
	 bullet_flag = FALSE;
      else {
	 /* shot an alien? */
	 for (c=0; c<alien_count; c++) {
	    if ((bullet_y < alien[c].y+30) && (bullet_y > alien[c].y) &&
		(bullet_x+30 > alien[c].x) && (bullet_x < alien[c].x+30) &&
		(!alien[c].shot)) {
	       alien[c].shot = TRUE;
	       alien[c].state = EXPLODE1;
	       bullet_flag = FALSE;
	       score += 10;
	       play_sample(data[BOOM_SPL].dat, 255, PAN(bullet_x), 1000, FALSE);
	       break; 
	    }
	 }
      }
   }

   /* move stars */
   for (c=0; c<MAX_STARS; c++) {
      if ((star[c].oy += ((int)star[c].z>>1)+1) >= SCREEN_H)
	 star[c].oy = 0;
   }

   /* fire bullet? */
   if (!shot) {
      if ((key[KEY_SPACE]) || (joy_b1) || (joy_b2)) {
	 if (!bullet_flag) {
	    bullet_x = (pos>>SPEED_SHIFT);
	    bullet_y = SCREEN_H-32;
	    bullet_flag = TRUE;
	    play_sample(data[SHOOT_SPL].dat, 100, PAN(bullet_x), 1000, FALSE);
	 } 
      }
   }

   /* move aliens */
   for (c=0; c<alien_count; c++) {
      if (alien[c].shot) {
	 /* dying alien */
	 if (skip_count <= 0) {
	    if (++alien[c].shot_delay > 2) {
	       alien[c].shot_delay = 0;
	       if (alien[c].state < EXPLODE5)
		  alien[c].state++;
	       else {
		  alien[c].x = random() % (SCREEN_W-32);
		  alien[c].y = -32 - (random() & 0x3f);
		  alien[c].d = (random()&1) ? 1 : -1;
		  alien[c].shot = FALSE;
		  alien[c].state = -1;
	       }
	    }
	 }
      }
      else {
	 /* move alien sideways */
	 alien[c].x += alien[c].d;
	 if (alien[c].x < -32)
	    alien[c].x = SCREEN_W;
	 else
	    if (alien[c].x > SCREEN_W)
	       alien[c].x = - 32;
      }
      /* move alien vertically */
      alien[c].y += 1;
      if (alien[c].y > SCREEN_H-60) {
	 if (alien[c].y > SCREEN_H) {
	    if (!alien[c].shot) {
	       alien[c].x = random() % (SCREEN_W-32);
	       alien[c].y = -32 - (random() & 0x3f);
	       alien[c].d = (random()&1) ? 1 : -1;
	    }
	 }
	 else {
	    /* alien collided with player? */
	    if ((alien[c].x+30 >= (pos>>SPEED_SHIFT)) &&
		(alien[c].x < (pos>>SPEED_SHIFT)+30) &&
		(alien[c].y < SCREEN_H-24)) {
	       if (!shot) {
		  if (!cheat) {
		     ship_state = EXPLODE1;
		     shot = TRUE;
		     shot_delay = 0;
		     stop_sample(data[ENGINE_SPL].dat);
		  }
		  if ((!cheat) || (!alien[c].shot))
		     play_sample(data[DEATH_SPL].dat, 255, PAN(pos>>SPEED_SHIFT), 1000, FALSE);
	       }
	       if (!alien[c].shot) {
		  alien[c].shot = TRUE;
		  alien[c].state = EXPLODE1;
	       }
	    }
	 }
      } 
   }

   if (skip_count <= 0) {
      skip_count = skip_speed;

      /* make a new alien? */
      new_alien_count++;
      if ((new_alien_count > 600) && (alien_count < MAX_ALIENS)) {
	 alien_count++;
	 new_alien_count = 0;
      }
   }
   else
      skip_count--;

   /* Yes, I know that isn't a very pretty piece of code :-) */
}



void add_to_list(DIRTY_LIST *list, int x, int y, int w, int h)
{
   list->rect[list->count].x = x;
   list->rect[list->count].y = y;
   list->rect[list->count].w = w;
   list->rect[list->count].h = h;
   list->count++; 
}



int rect_cmp(const void *p1, const void *p2)
{
   RECT *r1 = (RECT *)p1;
   RECT *r2 = (RECT *)p2;

   return (r1->y - r2->y);
}



void draw_screen()
{
   int c;
   int x, y;

   for (c=0; c<dirty.count; c++) {
      if ((dirty.rect[c].w == 1) && (dirty.rect[c].h == 1))
	 putpixel(s, dirty.rect[c].x, dirty.rect[c].y, 0);
      else
	 rectfill(s, dirty.rect[c].x, dirty.rect[c].y, 
		     dirty.rect[c].x + dirty.rect[c].w, 
		     dirty.rect[c].y+dirty.rect[c].h, 0);
   }

   old_dirty = dirty;
   dirty.count = 0;

   for (c=0; c<MAX_STARS; c++) {
      x = star[c].ox;
      y = star[c].oy;
      putpixel(s, x, y, 145+(int)star[c].z*8);
      add_to_list(&dirty, x, y, 1, 1);
   }

   x = pos>>SPEED_SHIFT;
   draw_rle_sprite(s, data[ship_state].dat, x, SCREEN_H-32); 
   add_to_list(&dirty, x, SCREEN_H-32, 32, 32);

   for (c=0; c<alien_count; c++) {
      x = alien[c].x;
      y = alien[c].y;
      if (alien[c].state >= 0)
	 draw_rle_sprite(s, data[alien[c].state].dat, x, y);
      else
	 draw_rle_sprite(s, alien_bmp[c%ALIEN_BMP_COUNT], x, y);
      add_to_list(&dirty, x, y, 32, 32);
   }

   if (bullet_flag) {
      x = bullet_x;
      y = bullet_y;
      draw_rle_sprite(s, data[BULLET].dat, x, y);
      add_to_list(&dirty, x, y, 32, 12);
   }

   if (fps)
      sprintf(score_buf, "Score: %ld - (%ld frames per second)", (long)score, (long)fps);
   else
      sprintf(score_buf, "Score: %ld", (long)score);
   textout(s, font, score_buf, 0, 0, 3);
   add_to_list(&dirty, 0, 0, text_length(font, score_buf), 8);

   for (c=0; c<dirty.count; c++)
      add_to_list(&old_dirty, dirty.rect[c].x, dirty.rect[c].y, 
			      dirty.rect[c].w, dirty.rect[c].h);

   /* sorting the objects really cuts down on bank switching */
   if (!gfx_driver->linear)
      qsort(old_dirty.rect, old_dirty.count, sizeof(RECT), rect_cmp);

   for (c=0; c<old_dirty.count; c++) {
      if ((old_dirty.rect[c].w == 1) && (old_dirty.rect[c].h == 1))
	 putpixel(screen, old_dirty.rect[c].x, old_dirty.rect[c].y, 
		  getpixel(s, old_dirty.rect[c].x, old_dirty.rect[c].y));
      else
	 blit(s, screen, old_dirty.rect[c].x, old_dirty.rect[c].y, 
			 old_dirty.rect[c].x, old_dirty.rect[c].y, 
			 old_dirty.rect[c].w, old_dirty.rect[c].h);
   }
}



RLE_SPRITE *draw_alien()
{
   /* Randomly generated alien bitmaps. Saves me the trouble of drawing 
    * them, and some of the results are surprisingly good...
    */
   int x, y;
   int x1, y1, x2, y2, x3, y3, color;
   int ax = 16;
   int ay = 16;
   int flag = FALSE;
   int c;
   BITMAP *bmp;
   RLE_SPRITE *s;

   bmp = create_bitmap(32, 32);
   clear(bmp);

   for (c=(random()&7)+12; c>0; c--) {
      x1 = (random()&7) + 8;
      x2 = (random()&15) + 8;
      x3 = (random()&15) + (flag ? 16 : 0);
      y1 = random()&31;
      y2 = random()&31;
      y3 = random()&31;
      color = random()&255;
      x = (x1+x2+x3)/3;
      y = (y1+y2+y3)/3;
      line(bmp, ax, ay, x, y, color);
      ax = x;
      ay = y;
      triangle(bmp, x1, y1, x2, y2, x3, y3, color);
      flag = !flag;
   }

   for (x=16; x<32; x++)
      for (y=0; y<32; y++)
	 bmp->line[y][x] = bmp->line[y][31-x];

   s = get_rle_sprite(bmp);
   destroy_bitmap(bmp);
   return s;
}



volatile int scroll_count;

void scroll_counter()
{
   scroll_count++;
}

END_OF_FUNCTION(scroll_counter);



void draw_intro_item(int item, int size)
{
   BITMAP *b = (BITMAP *)data[item].dat;
   int w = MIN(SCREEN_W, (SCREEN_W * 2 / size));
   int h = SCREEN_H / size;

   clear(screen);
   stretch_blit(b, screen, 0, 0, b->w, b->h, (SCREEN_W-w)/2, (SCREEN_H-h)/2, w, h);
}



void fade_intro_item(int music_pos, int fade_speed)
{
   do {
   } while (midi_pos < music_pos);

   set_pallete(data[GAME_PAL].dat);
   fade_out(fade_speed);
}



void play_game()
{
   int c;
   BITMAP *b, *b2;
   int esc = FALSE;

   stop_midi();

   /* set up a load of globals */
   dead = shot = FALSE;
   pos = (SCREEN_W/2-16)<<SPEED_SHIFT;
   xspeed = yspeed = ycounter = 0;
   ship_state = SHIP_STILL;
   frame_count = fps = 0;
   bullet_flag = FALSE;
   score = 0;
   old_dirty.count = dirty.count = 0;

   skip_count = 0;
   if (SCREEN_W < 400)
      skip_speed = 0;
   else if (SCREEN_W < 700)
      skip_speed = 1;
   else if (SCREEN_W < 1000)
      skip_speed = 2;
   else
      skip_speed = 3;

   for (c=0; c<MAX_STARS; c++) {
      star[c].ox = random() % SCREEN_W;
      star[c].oy = random() % SCREEN_H;
      star[c].z = random() & 7;
   }

   for (c=0; c<MAX_ALIENS; c++) {
      alien[c].x = random() % (SCREEN_W-32);
      alien[c].y = -32 - (random() & 0x3f);
      alien[c].d = (random()&1) ? 1 : -1;
      alien[c].state = -1;
      alien[c].shot = FALSE;
      alien[c].shot_delay = 0;
   }
   alien_count = 2;
   new_alien_count = 0;

   for (c=0; c<ALIEN_BMP_COUNT; c++)
      alien_bmp[c] = draw_alien();

   /* introduction synced to the music */
   draw_intro_item(INTRO_BMP_1, 5);
   play_midi(data[GAME_MUSIC].dat, TRUE);
   fade_intro_item(-1, 2);

   draw_intro_item(INTRO_BMP_2, 4);
   fade_intro_item(5, 2);

   draw_intro_item(INTRO_BMP_3, 3);
   fade_intro_item(9, 4);

   draw_intro_item(INTRO_BMP_4, 2);
   fade_intro_item(11, 4);

   draw_intro_item(GO_BMP, 1);
   fade_intro_item(13, 16);
   fade_intro_item(14, 16);
   fade_intro_item(15, 16);
   fade_intro_item(16, 16);

   text_mode(0);

   clear(screen);
   clear(s);
   draw_screen();

   do {
   } while (midi_pos < 17);

   set_pallete(data[GAME_PAL].dat);
   position_mouse(SCREEN_W/2, SCREEN_H/2);

   /* set up the interrupt routines... */
   move_count = 0;
   install_int(fps_proc, 1000);
   install_int(game_control, 6400/SCREEN_W);

   /* main game loop */
   while (!dead) {
      poll_joystick();

      while (move_count > 0) {
	 move_everyone();
	 move_count--;
      }

      draw_screen();
      frame_count++;

      if (key[KEY_ESC])
	 esc = dead = TRUE;
   }

   /* cleanup, display score, etc */
   remove_int(game_control);
   remove_int(fps_proc);
   stop_sample(data[ENGINE_SPL].dat);

   if (esc) {
      do {
      } while (key[KEY_ESC]);
      fade_out(5);
      return;
   }

   fps = 0;
   draw_screen();

   for (c=0; c<ALIEN_BMP_COUNT; c++)
      destroy_rle_sprite(alien_bmp[c]);

   b = create_bitmap(150, 150);
   b2 = create_bitmap(150, 150);
   clear(b);
   textout_centre(b, data[TITLE_FONT].dat, "GAME OVER", 75, 48, 160);
   textout_centre(b, data[TITLE_FONT].dat, score_buf, 75, 80, 160);
   clear_keybuf();
   scroll_count = -150;
   install_int(scroll_counter, 6000/SCREEN_W);
   c = 0;

   while ((!keypressed()) && (!joy_b1) && (!joy_b2)) {
      if (scroll_count > c+8)
	 c = scroll_count = c+8;
      else
	 c = scroll_count;

      blit(s, b2, c, c, 0, 0, 150, 150);
      rotate_sprite(b2, b, 0, 0, itofix(c)/4);
      blit(b2, screen, 0, 0, c, c, 150, 150);

      if ((c > SCREEN_W) || (c > SCREEN_H))
	 scroll_count = -150;

      poll_joystick();
   }

   remove_int(scroll_counter);
   destroy_bitmap(b);
   destroy_bitmap(b2);
   clear_keybuf();
   fade_out(5);
}



int title_screen()
{
   static int color = 0;
   int c;
   fixed x, y;
   int ix, iy;
   int star_count = 0;
   int star_count_count = 0;
   RGB rgb;
   char buf[2] = " ";
   int text_char = 0xFFFF;
   int text_width = 0;
   int text_pix = 0;
   BITMAP *text_bmp;

   play_midi(data[TITLE_MUSIC].dat, TRUE);
   play_sample(data[WELCOME_SPL].dat, 255, 127, 1000, FALSE);

   for (c=0; c<MAX_STARS; c++) {
      star[c].z = 0;
      star[c].ox = star[c].oy = -1;
   }

   for (c=0; c<8; c++)
      title_pallete[c] = ((RGB *)data[TITLE_PAL].dat)[c];

   /* set up the colors differently each time we display the title screen */
   for (c=8; c<PAL_SIZE; c++) {
      rgb = ((RGB *)data[TITLE_PAL].dat)[c];
      switch (color) {
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
      title_pallete[c] = rgb;
   }

   color++;
   if (color > 3)
      color = 0;

   text_bmp = create_bitmap(SCREEN_W, 24);
   clear(text_bmp);

   clear(screen);
   set_pallete(title_pallete);

   scroll_count = 1;
   install_int(scroll_counter, 5);

   while ((c=scroll_count) < 160)
      stretch_blit(data[TITLE_BMP].dat, screen, 0, 0, 320, 128,
		   SCREEN_W/2-c, SCREEN_H/2-c*64/160-32, c*2, c*128/160);

   remove_int(scroll_counter);
   blit(data[TITLE_BMP].dat, screen, 0, 0, SCREEN_W/2-160, SCREEN_H/2-96, 320, 128);

   clear_keybuf();

   scroll_count = 0;
   install_int(scroll_counter, 6);

   do {
      /* animate the starfield */
      for (c=0; c<MAX_STARS; c++) {
	 if (star[c].z <= itofix(1)) {
	    if (random()&1) {
	       if (random()&1)
		  star[c].x = -itofix(SCREEN_W/2);
	       else
		  star[c].x = itofix(SCREEN_W/2);
	       star[c].y = itofix((random() % SCREEN_H) - SCREEN_H/2);
	    }
	    else {
	       if (random()&1)
		  star[c].y = -itofix(SCREEN_H/2);
	       else
		  star[c].y = itofix(SCREEN_H/2);
	       star[c].x = itofix((random() % SCREEN_W) - SCREEN_W/2);
	    }
	    star[c].z = itofix((random() & 0xf) + 1);
	 }

	 x = fdiv(star[c].x, star[c].z);
	 y = fdiv(star[c].y, star[c].z);
	 ix = (int)(x>>16) + SCREEN_W/2;
	 iy = (int)(y>>16) + SCREEN_H/2;
	 putpixel(screen, star[c].ox, star[c].oy, 0);
	 if (getpixel(screen, ix, iy) == 0) {
	    if (c < star_count)
	       putpixel(screen, ix, iy, 7-(int)(star[c].z>>17));
	    star[c].ox = ix;
	    star[c].oy = iy;
	 }
	 star[c].z -= 1024;
      }

      if (star_count < MAX_STARS) {
	 if (star_count_count++ >= 32) {
	    star_count_count = 0;
	    star_count++;
	 }
      }

      /* wait a bit if we need to */
      do {
      } while (scroll_count <= 0);

      /* and move the text scroller */
      c = scroll_count;
      scroll_count = 0;
      blit(text_bmp, text_bmp, c, 0, 0, 0, SCREEN_W, 24);
      rectfill(text_bmp, SCREEN_W-c, 0, SCREEN_W, 24, 0);

      while (c > 0) {
	 text_pix += c;
	 textout(text_bmp, data[TITLE_FONT].dat, buf, SCREEN_W-text_pix, 0, 7);
	 if (text_pix >= text_width) {
	    c = text_pix - text_width;
	    text_char++;
	    if (text_char >= data[TITLE_TEXT].size)
	       text_char = 0;
	    buf[0] = ((char *)data[TITLE_TEXT].dat)[text_char];
	    text_pix = 0;
	    text_width = text_length(data[TITLE_FONT].dat, buf);
	 }
	 else
	    c = 0;
      }
      blit(text_bmp, screen, 0, 0, 0, SCREEN_H-24, SCREEN_W, 24);

      poll_joystick();

   } while ((!keypressed()) && (!joy_b1) && (!joy_b2));

   remove_int(scroll_counter);
   fade_out(5);

   while (keypressed())
      if ((readkey() & 0xff) == 27)
	 return FALSE;

   destroy_bitmap(text_bmp);

   return TRUE;
}



void main(int argc, char *argv[])
{
   int card, w, h;
   char buf[80];

   if ((argc > 1) && (stricmp(argv[1], "cheat") == 0))
      cheat = TRUE;
   else
      cheat = FALSE;

   allegro_init();
   install_keyboard();
   install_mouse();
   install_timer();

   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);
   set_pallete(desktop_pallete);

   if (!gfx_mode_select(&card, &w, &h)) {
      allegro_exit();
      exit(1);
   }

   if (set_gfx_mode(card, w, h, 0, 0) != 0) {
      allegro_exit();
      printf("Error setting graphics mode\n%s\n\n", allegro_error);
      exit(1);
   }

   set_pallete(black_pallete);

   strcpy(buf, argv[0]);
   strcpy(get_filename(buf), "DEMO.DAT");
   data = load_datafile(buf);
   if (!data) {
      allegro_exit();
      printf("Error loading demo.dat\n\n");
      exit(1);
   }

   if (install_sound(DIGI_AUTODETECT, MIDI_AUTODETECT, argv[0]) != 0) {
      allegro_exit();
      printf("Error initialising sound\n%s\n\n", allegro_error);
      exit(1);
   }

   LOCK_VARIABLE(cheat);
   LOCK_VARIABLE(scroll_count);
   LOCK_VARIABLE(move_count);
   LOCK_VARIABLE(score);
   LOCK_VARIABLE(dead);
   LOCK_VARIABLE(pos);
   LOCK_VARIABLE(xspeed);
   LOCK_VARIABLE(yspeed);
   LOCK_VARIABLE(ycounter);
   LOCK_VARIABLE(ship_state);
   LOCK_VARIABLE(skip_count);
   LOCK_VARIABLE(skip_speed);
   LOCK_VARIABLE(shot);
   LOCK_VARIABLE(shot_delay);
   LOCK_VARIABLE(frame_count);
   LOCK_VARIABLE(fps);
   LOCK_VARIABLE(star);
   LOCK_VARIABLE(alien);
   LOCK_VARIABLE(alien_count);
   LOCK_VARIABLE(new_alien_count);
   LOCK_VARIABLE(bullet_flag);
   LOCK_VARIABLE(bullet_x);
   LOCK_VARIABLE(bullet_y);
   LOCK_FUNCTION(scroll_counter);
   LOCK_FUNCTION(fps_proc);
   LOCK_FUNCTION(game_control);

   text_mode(0);

   s = create_bitmap(SCREEN_W, SCREEN_H);

   initialise_joystick();

   while (title_screen())
      play_game(); 

   allegro_exit();
   destroy_bitmap(s);
   cputs(data[END_TEXT].dat);
   unload_datafile(data);
   exit(0);
}


