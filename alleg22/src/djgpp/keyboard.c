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
 *      DOS keyboard routines.
 *
 *      Salvador Eduardo Tropea added support for extended scancodes,
 *      keyboard LED's, capslock and numlock, and alt+numpad input.
 *
 *      See readme.txt for copyright information.
 */


#ifndef DJGPP
#error This file should only be used by the djgpp version of Allegro
#endif

#include <stdlib.h>
#include <dos.h>
#include <conio.h>
#include <dpmi.h>
#include <sys/movedata.h>

#include "allegro.h"
#include "internal.h"


#define KEYBOARD_INT          9

#define KB_SPECIAL_MASK       7
#define KB_CTRL_ALT_FLAG      (KB_CTRL_FLAG | KB_ALT_FLAG)


int three_finger_flag = TRUE;
int key_led_flag = TRUE;

static int keyboard_installed = FALSE; 

volatile char key[128];                   /* key pressed flags */

volatile int key_shifts = 0;


char key_ascii_table[128] =
{
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F             */
   0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8,   9,       /* 0 */
   'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 13,  0,   'a', 's',     /* 1 */
   'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', 39,  '`', 0,   92,  'z', 'x', 'c', 'v',     /* 2 */
   'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' ', 0,   3,   3,   3,   3,   8,       /* 3 */
   3,   3,   3,   3,   3,   0,   0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,       /* 4 */
   0,   0,   0,   127, 0,   0,   92,  3,   3,   0,   0,   0,   0,   0,   0,   0,       /* 5 */
   13,  0,   '/', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   127,     /* 6 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0        /* 7 */
};


char key_capslock_table[128] =
{
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F             */
   0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 8,   9,       /* 0 */
   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 13,  0,   'A', 'S',     /* 1 */
   'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', 39,  '`', 0,   92,  'Z', 'X', 'C', 'V',     /* 2 */
   'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   3,   3,   3,   3,   8,       /* 3 */
   3,   3,   3,   3,   3,   0,   0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,       /* 4 */
   0,   0,   0,   127, 0,   0,   92,  3,   3,   0,   0,   0,   0,   0,   0,   0,       /* 5 */
   13,  0,   '/', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   127,     /* 6 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0        /* 7 */
};


char key_shift_table[128] =
{
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F             */
   0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 126, 126,     /* 0 */
   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 126, 0,   'A', 'S',     /* 1 */
   'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', 34,  '~', 0,   '|', 'Z', 'X', 'C', 'V',     /* 2 */
   'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   1,   0,   1,   1,   1,   1,   1,       /* 3 */
   1,   1,   1,   1,   1,   0,   0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,       /* 4 */
   0,   0,   1,   127, 0,   0,   0,   1,   1,   0,   0,   0,   0,   0,   0,   0,       /* 5 */
   13,  0,   '/', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   127,     /* 6 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0        /* 7 */
};


char key_control_table[128] =
{
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F             */
   0,   0,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   0,   0,   127, 127,     /* 0 */
   17,  23,  5,   18,  20,  25,  21,  9,   15,  16,  2,   2,   10,  0,   1,   19,      /* 1 */
   4,   6,   7,   8,   10,  11,  12,  0,   0,   0,   0,   0,   26,  24,  3,   22,      /* 2 */
   2,   14,  13,  0,   0,   0,   0,   0,   0,   0,   0,   2,   2,   2,   2,   2,       /* 3 */
   2,   2,   2,   2,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 4 */
   0,   0,   2,   0,   0,   0,   0,   2,   2,   0,   0,   0,   0,   0,   0,   0,       /* 5 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 6 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0        /* 7 */
};


char key_numlock_table[128] =
{
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F             */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 0 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 1 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 2 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 3 */
   0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', 0,   '4', '5', '6', 0,   '1',     /* 4 */
   '2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 5 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 6 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0        /* 7 */
};


/*
  Mapping:
  0 = Use the second value as scan char, just like before.
  1 = Ignore the key.
  2 = Use the scan but put the shift flags instead of the ASCII.

  Extended values:
  E0 1C = Enter
  E0 1D = RCtrl         => Ctrl
  E0 2A = ?????? generated in conjuntion with Insert!!
  E0 35 = \
  E0 38 = AltGr or RAlt => Alt
  E0 46 = Ctrl-Pause
  E0 47 = Home
  E0 48 = Up
  E0 4B = Left
  E0 4D = Right
  E0 4F = End
  E0 50 = Down
  E0 51 = Page-Down
  E0 52 = Insert
  E0 53 = Delete
*/
char key_extended_table[128] =
{
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F             */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 0 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,   0,   0,   0,       /* 1 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   0,   0,   0,   0,   0,       /* 2 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 3 */
   0,   0,   0,   0,   0,   0,   2,   2,   2,   2,   0,   2,   2,   2,   0,   2,       /* 4 */
   2,   2,   2,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 5 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 6 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0        /* 7 */
};


char key_special_table[128] =
{
/* 0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F             */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 0 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   2,   0,   0,       /* 1 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   0,   0,   0,   0,   0,       /* 2 */
   0,   0,   0,   0,   0,   0,   1,   0,   4,   0,   32,  0,   0,   0,   0,   0,       /* 3 */
   0,   0,   0,   0,   0,   16,  8,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 4 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 5 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,       /* 6 */
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0        /* 7 */
};


#define KEY_BUFFER_SIZE    256

static volatile int key_buffer[KEY_BUFFER_SIZE]; 
static volatile int key_buffer_start = 0;
static volatile int key_buffer_end = 0;
static volatile int key_extended = 0;
static volatile int kb_need_reply = 0;
static volatile int kb_ack_response = 0;
static volatile int kb_resend_response = 0;
static volatile int key_pad_seq;

static int (*keypressed_hook)() = NULL;
static int (*readkey_hook)() = NULL;



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

   if ((keypressed_hook) && (readkey_hook))
      while (keypressed_hook())
	 readkey_hook();
}



/* keypressed:
 *  Returns TRUE if there are keypresses waiting in the keyboard buffer.
 */
int keypressed()
{
   if (key_buffer_start == key_buffer_end) {
      if (keypressed_hook)
	 return keypressed_hook();
      else
	 return FALSE;
   }
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

   if ((!keyboard_installed) && (!readkey_hook))
      return 0;

   if ((readkey_hook) && (key_buffer_start == key_buffer_end))
      return readkey_hook();

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



/* kb_wait_for_ready:
 *  Wait for the keyboard controller to set the ready bit.
 */
static inline void kb_wait_for_ready(void)
{
   long i = 1000000L;

   while ((i--) && (inportb(0x64) & 0x02))
      ; /* wait */
}



/* kb_send_data:
 *  Sends a byte to the keyboard controller. Returns 1 if all OK.
 */
static inline int kb_send_data(unsigned char data)
{
   long i;
   int resends = 4;

   do {
      kb_wait_for_ready();
      kb_need_reply = 1;
      kb_ack_response = kb_resend_response = 0;

      outportb(0x60, data);
      i = 2000000L;

      while (--i) {
	 inportb(0x64);

	 if (kb_ack_response)
	    return 1;

	 if (kb_resend_response)
	    break;
      }
   }
   while ((resends-- > 0) && (i));

   return 0;
}



/* update_leds:
 *  Sets the state of the keyboard LED indicators.
 */
static inline void update_leds()
{
   if ((!kb_send_data(0xED)) || (!kb_send_data(key_shifts>>3)))
      kb_send_data(0xF4);
}



/* my_keyint:
 *  Hardware level keyboard interrupt (int 9) handler.
 */
static int my_keyint()
{
   int t, temp, release, flag;
   char newchar;

   temp = inportb(0x60);            /* read keyboard byte */

   if (kb_need_reply) {
      if (temp == 0xFA) {
	 kb_ack_response = 1;
	 kb_need_reply = 0;
	 goto exit_keyboard_handler;
      }
      else if (temp == 0xFE) {
	 kb_resend_response = 1;
	 kb_need_reply = 0;
	 goto exit_keyboard_handler;
      }
   }

   if (temp == 0xE0) {
      key_extended = 1; 
   }
   else {
      release = (temp & 0x80);      /* bit 7 means key was released */
      temp &= 0x7F;                 /* bits 0-6 is the scan code */

      if (key_extended) {           /* if is an extended code */
	 key_extended = 0;

	 if (((temp == KEY_END) || (temp == KEY_DEL)) && 
	     ((key_shifts & KB_CTRL_ALT_FLAG) == KB_CTRL_ALT_FLAG) && 
	     (!release) && (three_finger_flag)) {
	    asm (
	       "  movb $0x79, %%al ; "
	       "  call ___djgpp_hw_exception "
	    : : : "%eax", "%ebx", "%ecx", "%edx", "%esi", "%edi", "memory"
	    );
	    goto exit_keyboard_handler;
	 }

	 switch (key_extended_table[temp]) {

	    case 1:
	       /* ignore the key */
	       goto exit_keyboard_handler; 

	    case 2:
	       if (release) {
		  key[temp] = FALSE;
	       }
	       else { 
		  /* report the scan code + the shift state */
		  key[temp] = TRUE;
		  t = (temp<<8) | (key_shifts & KB_SPECIAL_MASK);
		  add_key(t);
	       }
	       goto exit_keyboard_handler;

	    default:
	       /* process as normal */
	       break;
	 }
      } 

      if (release) {                /* key was released */
	 key[temp] = FALSE;

	 if ((flag = key_special_table[temp]) != 0) {
	    if (flag < KB_ALT_FLAG) {
		key_shifts &= ~flag; 
	    }
	    else if (flag == KB_ALT_FLAG) {
	       key_shifts &= ~flag;
	       if (key_shifts & KB_INALTSEQ_FLAG) {
		  key_shifts &= ~KB_INALTSEQ_FLAG;
		  add_key(key_pad_seq & 0xFF);
	       }
	    }
	 }
      }
      else {                        /* key was pressed */
	 key[temp] = TRUE;

	 if ((flag = key_special_table[temp]) != 0) {
	    if (flag > KB_ALT_FLAG) {
	       if (key_led_flag) {
		  key_shifts ^= flag;
		  goto set_keyboard_leds;
	       }
	    }
	    else
	       key_shifts |= flag;
	 }
	 else {                     /* normal key */
	    if (key_shifts & KB_ALT_FLAG) {
	       if ((temp >= 0x47) && (key_extended_table[temp] == 2)) { 
		  if (key_shifts & KB_INALTSEQ_FLAG) {
		     key_pad_seq = key_pad_seq*10 + key_numlock_table[temp]-'0';
		  }
		  else {
		     key_shifts |= KB_INALTSEQ_FLAG;
		     key_pad_seq = key_numlock_table[temp] - '0';
		  }
		  goto exit_keyboard_handler;
	       }
	       else
		  t = SCANCODE_TO_ALT(temp);
	    }
	    else if (key_shifts & KB_CTRL_FLAG)
	       t = SCANCODE_TO_CONTROL(temp);
	    else if (key_shifts & KB_SHIFT_FLAG)
	       t = SCANCODE_TO_SHIFT(temp);
	    else if ((key_shifts & KB_NUMLOCK_FLAG) &&
		     (newchar = key_numlock_table[temp]) != 0)
	       t = (KEY_PAD<<8) | newchar;
	    else if (key_shifts & KB_CAPSLOCK_FLAG)
	       t = SCANCODE_TO_CAPS(temp);
	    else
	       t = SCANCODE_TO_KEY(temp);

	    key_shifts &= ~KB_INALTSEQ_FLAG;

	    add_key(t);
	 }
      }
   }

   exit_keyboard_handler:

   outportb(0x20,0x20);       /* ack. the interrupt */
   return 0;

   /* this part is for the LED update. We need responses from the keyboard
    * so we need the interrupts enabled.
    */

   set_keyboard_leds:

   outportb(0x20,0x20);
   ENABLE();

   update_leds();

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
   unsigned short shifts;

   if (keyboard_installed)
      return -1;

   LOCK_VARIABLE(three_finger_flag);
   LOCK_VARIABLE(key_led_flag);
   LOCK_VARIABLE(key_shifts);
   LOCK_VARIABLE(key);
   LOCK_VARIABLE(key_ascii_table);
   LOCK_VARIABLE(key_capslock_table);
   LOCK_VARIABLE(key_shift_table);
   LOCK_VARIABLE(key_control_table);
   LOCK_VARIABLE(key_numlock_table);
   LOCK_VARIABLE(key_extended_table);
   LOCK_VARIABLE(key_special_table);
   LOCK_VARIABLE(key_buffer);
   LOCK_VARIABLE(key_buffer_start);
   LOCK_VARIABLE(key_buffer_end);
   LOCK_VARIABLE(key_extended);
   LOCK_VARIABLE(kb_need_reply);
   LOCK_VARIABLE(kb_ack_response);
   LOCK_VARIABLE(kb_resend_response);
   LOCK_VARIABLE(key_pad_seq);
   LOCK_FUNCTION(my_keyint);

   clear_keybuf();

   /* transfer keys from keyboard buffer */
   while ((kbhit()) && (key_buffer_end < KEY_BUFFER_SIZE-1))
      key_buffer[key_buffer_end++] = getch();

   /* get state info from the BIOS */
   _dosmemgetw(0x417, 1, &shifts);

   key_shifts = 0;

   if (shifts & 1) {
      key_shifts |= KB_SHIFT_FLAG;
      key[KEY_RSHIFT] = TRUE;
   }
   if (shifts & 2) {
      key_shifts |= KB_SHIFT_FLAG;
      key[KEY_LSHIFT] = TRUE;
   }
   if (shifts & 4) {
      key_shifts |= KB_CTRL_FLAG;
      key[KEY_CONTROL] = TRUE;
   }
   if (shifts & 8) {
      key_shifts |= KB_ALT_FLAG;
      key[KEY_ALT] = TRUE;
   }
   if (shifts & 16)
      key_shifts |= KB_SCROLOCK_FLAG;
   if (shifts & 32)
      key_shifts |= KB_NUMLOCK_FLAG;
   if (shifts & 64)
      key_shifts |= KB_CAPSLOCK_FLAG;

   _install_irq(KEYBOARD_INT, my_keyint);

   update_leds();

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
   unsigned short shifts;

   if (!keyboard_installed)
      return;

   _remove_irq(KEYBOARD_INT);

   /* transfer state info to the BIOS */
   shifts = 0;

   if (key[KEY_RSHIFT])
      shifts |= 1;
   if (key[KEY_LSHIFT])
      shifts |= 2;
   if (key[KEY_CONTROL])
      shifts |= 4;
   if (key[KEY_ALT])
      shifts |= 8;
   if (key_shifts & KB_SCROLOCK_FLAG)
      shifts |= 16;
   if (key_shifts & KB_NUMLOCK_FLAG)
      shifts |= 32;
   if (key_shifts & KB_CAPSLOCK_FLAG)
      shifts |= 64;

   _dosmemputw(&shifts, 1, 0x417);

   clear_keybuf();

   _remove_exit_func(remove_keyboard);
   keyboard_installed = FALSE;
}



/* install_keyboard_hooks:
 *  You should only use this function if you *aren't* using the rest of the 
 *  keyboard handler. It can be called in the place of install_keyboard(), 
 *  and lets you provide callback routines to detect and read keypresses, 
 *  which will be used by the main keypressed() and readkey() functions. This 
 *  can be useful if you want to use Allegro's GUI code with a custom 
 *  keyboard handler, as it provides a way for the GUI to access keyboard 
 *  input from your own code.
 */
void install_keyboard_hooks(int (*keypressed)(), int (*readkey)())
{
   keypressed_hook = keypressed;
   readkey_hook = readkey;
}
