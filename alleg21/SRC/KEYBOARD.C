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
 *      Keyboard routines.
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <dos.h>
#include <conio.h>

#include "allegro.h"
#include "internal.h"


#define KEYBOARD_INT    9

int three_finger_flag = TRUE;

static int keyboard_installed = FALSE; 

volatile char key[128];                   /* key pressed flags */


char key_ascii_table[128] =
{
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F             */
   0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8,   9,       /* 0 */
   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13,  0,   'a', 's',     /* 1 */
   'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 39,  '`', 0,   92,  'z', 'x', 'c', 'v',     /* 2 */
   'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,       /* 3 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,       /* 4 */
   0,   0,   0,   127, 0,   0,   92,  0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 5 */
   13,  0,   '/', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   127,     /* 6 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0        /* 7 */
};


char key_shift_table[128] =
{
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F             */
   0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 8,   9,       /* 0 */
   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 13,  0,   'A', 'S',     /* 1 */
   'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', 34,  '~', 0,   '|', 'Z', 'X', 'C', 'V',     /* 2 */
   'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,       /* 3 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,       /* 4 */
   0,   0,   0,   127, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 5 */
   13,  0,   '/', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   127,     /* 6 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0        /* 7 */
};


char key_control_table[128] =
{
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F             */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 0 */
   17,  23,  5,   18,  20,  25,  21,  9,   15,  16,  0,   0,   0,   0,   1,   19,      /* 1 */
   4,   6,   7,   8,   10,  11,  12,  0,   0,   0,   0,   0,   26,  24,  3,   22,      /* 2 */
   2,   14,  13,  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 3 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 4 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 5 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 6 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0        /* 7 */
};


#define KEY_BUFFER_SIZE    256

static volatile int key_buffer[KEY_BUFFER_SIZE]; 
static volatile int key_buffer_start = 0;
static volatile int key_buffer_end = 0;



/* add_key:
 *  Helper function to add a keypress to the buffer.
 */
static inline void add_key(int c)
{
   key_buffer[key_buffer_end] = c;

   key_buffer_end++;
   if (key_buffer_end >= KEY_BUFFER_SIZE)
      key_buffer_end = 0;
   if (key_buffer_end == key_buffer_start) {    /* buffer full */
      key_buffer_start++;
      if (key_buffer_start >= KEY_BUFFER_SIZE)
	 key_buffer_start = 0;
   }
}



/* clear_keybuf:
 *  Clears the keyboard buffer.
 */
void clear_keybuf()
{
   int c;

   DISABLE();

   key_buffer_start = 0;
   key_buffer_end = 0;

   for (c=0; c<128; c++)
      key[c] = FALSE;

   ENABLE();
}



/* keypressed:
 *  Returns TRUE if there are keypresses waiting in the keyboard buffer.
 */
int keypressed()
{
   if (key_buffer_start == key_buffer_end)
      return FALSE;
   else
      return TRUE;
}



/* readkey:
 *  Returns the next character code from the keyboard buffer. If the
 *  buffer is empty, it waits until a key is pressed. The low byte of
 *  the return value contains the ASCII code of the key, and the high
 *  byte the scan code. 
 */
int readkey()
{
   int r;

   if (!keyboard_installed)
      return 0;

   do {
   } while (key_buffer_start == key_buffer_end);  /* wait for a press */

   DISABLE();

   r = key_buffer[key_buffer_start];
   key_buffer_start++;
   if (key_buffer_start >= KEY_BUFFER_SIZE)
      key_buffer_start = 0;

   ENABLE();

   return r;
}



/* simulate_keypress:
 *  Pushes a key into the keyboard buffer, as if it has just been pressed.
 */
void simulate_keypress(int key)
{
   DISABLE();

   add_key(key);

   ENABLE();
}



/* my_keyint:
 *  Hardware level keyboard interrupt (int 9) handler.
 */
static int my_keyint()
{
   int t, temp;

   temp = inportb(0x60);         /* read keyboard byte */

   if (temp & 0x80) {            /* key was released */
      key[temp&0x7f] = FALSE; 
   }
   else {                        /* key was pressed */
      temp &= 0x7f;
      key[temp] = TRUE;

      if ((temp != KEY_LSHIFT) && (temp != KEY_RSHIFT) &&
	  (temp != KEY_CONTROL) && (temp != KEY_ALT) &&
	  (temp != KEY_CAPSLOCK) && (temp != KEY_NUMLOCK) &&
	  (temp != KEY_SCRLOCK)) {

	 if (key[KEY_ALT])
	    t = SCANCODE_TO_ALT(temp);
	 else if (key[KEY_CONTROL])
	    t = SCANCODE_TO_CONTROL(temp);
	 else if ((key[KEY_LSHIFT]) || (key[KEY_RSHIFT]))
	    t = SCANCODE_TO_SHIFT(temp);
	 else
	    t = SCANCODE_TO_KEY(temp);

	 add_key(t);

	 if ((three_finger_flag) && (key[KEY_CONTROL]) && (key[KEY_ALT]) &&
	     ((temp == KEY_DEL) || (temp == KEY_END))) {
	    asm (
	       "  movb $0x79, %%al ; "
	       "  call ___djgpp_hw_exception "
	    : : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi", "memory"
	    );
	 }
      }
   }

   outportb(0x20,0x20);       /* ack. the interrupt */

   return 0;
}

static END_OF_FUNCTION(my_keyint);



/* install_keyboard:
 *  Installs Allegro's keyboard handler. You must call this before using 
 *  any of the keyboard input routines. Note that Allegro completely takes 
 *  over the keyboard, so the debugger will not work properly, and under 
 *  DOS even ctrl-alt-del will have no effect. Returns -1 on failure.
 */
int install_keyboard()
{
   if (keyboard_installed)
      return -1;

   LOCK_VARIABLE(key);
   LOCK_VARIABLE(key_ascii_table);
   LOCK_VARIABLE(key_shift_table);
   LOCK_VARIABLE(key_control_table);
   LOCK_VARIABLE(key_buffer);
   LOCK_VARIABLE(key_buffer_start);
   LOCK_VARIABLE(key_buffer_end);
   LOCK_VARIABLE(three_finger_flag);
   LOCK_FUNCTION(my_keyint);

   clear_keybuf();

   /* transfer keys waiting in BIOS keyboard buffer */
   while ((kbhit()) && (key_buffer_end < KEY_BUFFER_SIZE-1))
      key_buffer[key_buffer_end++] = getch();

   _install_irq(KEYBOARD_INT, my_keyint);

   _add_exit_func(remove_keyboard);
   keyboard_installed = TRUE;
   return 0;
}



/* remove_keyboard:
 *  Removes the keyboard handler, returning control to the BIOS. You don't
 *  normally need to call this, because allegro_exit() will do it for you.
 */
void remove_keyboard()
{
   if (!keyboard_installed)
      return;

   _remove_irq(KEYBOARD_INT);

   clear_keybuf();

   _remove_exit_func(remove_keyboard);
   keyboard_installed = FALSE;
}


