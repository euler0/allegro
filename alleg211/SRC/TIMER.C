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
 *      Timer interrupt routines. 
 *
 *      See readme.txt for copyright information.
 */


#include <stdlib.h>
#include <dos.h>

#include "allegro.h"
#include "internal.h"


/* Error factor for retrace syncing. Reduce this to spend less time waiting
 * for the retrace, increase it to reduce the chance of missing retraces.
 */
#define VSYNC_MARGIN    1024

#define MAX_TIMERS      8

#define TIMER_INT       8

static int timer_installed = FALSE;

static long bios_counter;                 /* keep BIOS time up to date */
static long timer_delay;                  /* how long between interrupts */
static int timer_semaphore = FALSE;       /* reentrant interrupt? */

volatile int retrace_count = 0;           /* vertical retrace counter */
void (*retrace_proc)() = NULL;            /* retrace callback function */

int _timer_use_retrace = FALSE;           /* synchronise with screen? */
volatile int _retrace_hpp_value = -1;     /* to set during next retrace */

static long vsync_counter;                /* retrace position counter */
static long vsync_speed;                  /* retrace speed */

static struct {
   void ((*proc)());
   long speed;
   long counter;
} my_int_queue[MAX_TIMERS];



/* set_timer:
 *  Sets the delay time for PIT channel 1.
 */
static inline void set_timer(long time)
{
    outportb(0x43, 0x30);
    outportb(0x40, time & 0xff);
    outportb(0x40, time >> 8);
}



/* read_timer:
 *  Reads the elapsed time from PIT channel 1.
 */
static inline long read_timer()
{
    long x;

    outportb(0x43, 0x00);
    x = inportb(0x40);
    x += inportb(0x40) << 8;

    return 0xFFFF - x;
}



/* reset_timer:
 *  Resets PIT channel 1 to the normal 18.2 ticks per second.
 */
static void reset_timer()
{
    outportb(0x43, 0x34);
    outportb(0x40, 0);
    outportb(0x40, 0);
}



/* my_timerint:
 *  Hardware level timer interrupt (int 8) handler. Calls whatever user
 *  timer routines are due for dealing with, and calculates how long until
 *  we need another timer tick.
 */
static int my_timerint()
{
   long new_delay = 0x8000;
   int callback[MAX_TIMERS];
   int bios = FALSE;
   int x;

   /* reentrant interrupt? */
   if (timer_semaphore) {
      timer_delay += 0x2000;
      set_timer(0x2000);
      outportb(0x20, 0x20); 
      return 0;
   }

   timer_semaphore = TRUE;

   /* deal with retrace synchronisation */
   vsync_counter -= timer_delay; 
   if (vsync_counter <= 0) {
      set_timer(0);

      if (_timer_use_retrace) {
	 /* wait for retrace to start */
	 do { 
	 } while (!(inportb(0x3DA)&8));

	 /* update the VGA pelpan register? */
	 if (_retrace_hpp_value >= 0) {
	    inportb(0x3DA);
	    outportb(0x3C0, 0x33);
	    outportb(0x3C1, _retrace_hpp_value);

	    _retrace_hpp_value = -1;
	 }

	 /* user callback */
	 retrace_count++;
	 if (retrace_proc)
	    retrace_proc();

	 vsync_counter = vsync_speed - VSYNC_MARGIN;
	 timer_delay += read_timer();
      }
      else {
	 vsync_counter += vsync_speed;
	 retrace_count++;
	 if (retrace_proc)
	    retrace_proc();
      }
   }
   if (vsync_counter < new_delay)
      new_delay = vsync_counter;

   /* process the user callbacks */
   for (x=0; x<MAX_TIMERS; x++) { 
      callback[x] = FALSE;

      if ((my_int_queue[x].proc) && (my_int_queue[x].speed > 0)) {
	 my_int_queue[x].counter -= timer_delay;
	 if (my_int_queue[x].counter <= 0) {
	    my_int_queue[x].counter += my_int_queue[x].speed;
	    callback[x] = TRUE;
	 }
	 if (my_int_queue[x].counter < new_delay)
	    new_delay = my_int_queue[x].counter; 
      }
   }

   /* update bios time */
   bios_counter -= timer_delay; 
   if (bios_counter <= 0) {
      bios_counter += 0x10000;
      bios = TRUE;
   }
   if (bios_counter < new_delay)
      new_delay = bios_counter;

   /* fudge factor to prevent interrupts coming too close to each other */
   if (new_delay < 1024)
      timer_delay = 1024;
   else
      timer_delay = new_delay;

   /* start the timer up again */
   set_timer(timer_delay);

   if (!bios) {
      ENABLE();
      outportb(0x20, 0x20);      /* ack. the interrupt */
   }

   /* finally call the user timer routines */
   for (x=0; x<MAX_TIMERS; x++)
      if (callback[x])
	 my_int_queue[x].proc();

   if (!bios)
      DISABLE();

   timer_semaphore = FALSE;

   return bios;
}

static END_OF_FUNCTION(my_timerint);



static volatile long rest_count;

static void rest_int()
{
   rest_count--;
}

static END_OF_FUNCTION(rest_int);



/* rest:
 *  Waits for time milliseconds.
 */
void rest(long time)
{
   rest_count = time;

   if (install_int(rest_int, 1) < 0)
      return;

   do {
   } while (rest_count > 0);

   remove_int(rest_int);
}



/* timer_calibrate_retrace:
 *  Times several vertical retraces, and calibrates the retrace syncing
 *  code accordingly.
 */
static void timer_calibrate_retrace()
{
   int c;
   int ot = _timer_use_retrace;

   if (!timer_installed)
      return;

   #define AVERAGE_COUNT   4

   _timer_use_retrace = FALSE;
   vsync_speed = 0;

   /* time several retraces */
   for (c=0; c<AVERAGE_COUNT; c++) {
      enter_critical();
      _vga_vsync();
      set_timer(0);
      _vga_vsync();
      vsync_speed += read_timer();
      exit_critical();
   }

   set_timer(timer_delay);

   vsync_speed /= AVERAGE_COUNT;

   /* sanity check to discard bogus values */
   if ((vsync_speed > BPS_TO_TIMER(40)) ||
       (vsync_speed < BPS_TO_TIMER(110)))
      vsync_speed = BPS_TO_TIMER(70);

   vsync_counter = vsync_speed;
   _timer_use_retrace = ot;
}



/* timer_simulate_retrace:
 *  Turns retrace simulation on or off, and if turning it on, calibrates
 *  the retrace timer.
 */
void timer_simulate_retrace(int enable)
{
   if (!timer_installed)
      return;

   if (enable) {
      timer_calibrate_retrace();
      _timer_use_retrace = TRUE;
   }
   else {
      _timer_use_retrace = FALSE;
      vsync_counter = vsync_speed = BPS_TO_TIMER(70);
   }


}



/* install_timer:
 *  Installs the timer interrupt handler. You must do this before installing
 *  any user timer routines. You must set up the timer before trying to 
 *  display a mouse pointer or using any of the GUI routines.
 */
int install_timer()
{
   int x;

   if (timer_installed)
      return -1;

   for (x=0; x<MAX_TIMERS; x++) {
      my_int_queue[x].proc = NULL;
      my_int_queue[x].speed = 0;
      my_int_queue[x].counter = 0;
   }

   LOCK_VARIABLE(bios_counter);
   LOCK_VARIABLE(timer_delay);
   LOCK_VARIABLE(my_int_queue);
   LOCK_VARIABLE(timer_semaphore);
   LOCK_VARIABLE(vsync_counter);
   LOCK_VARIABLE(vsync_speed);
   LOCK_VARIABLE(retrace_count);
   LOCK_VARIABLE(_timer_use_retrace);
   LOCK_VARIABLE(_retrace_hpp_value);
   LOCK_VARIABLE(retrace_proc);
   LOCK_VARIABLE(rest_count);
   LOCK_FUNCTION(rest_int);
   LOCK_FUNCTION(my_timerint);

   retrace_proc = NULL;
   _timer_use_retrace = FALSE;
   _retrace_hpp_value = -1;
   vsync_counter = vsync_speed = BPS_TO_TIMER(70);

   bios_counter = 0x10000;
   timer_delay = 0x10000;

   if (_install_irq(TIMER_INT, my_timerint) != 0)
      return -1;

   DISABLE();

   /* windoze doesn't seem to notice if we only do this once... */
   for (x=0; x<16; x++)
      set_timer(timer_delay);

   ENABLE();

   _add_exit_func(remove_timer);
   timer_installed = TRUE;
   return 0;
}



/* remove_timer:
 *  Removes our timer handler and resets the BIOS clock. You don't normally
 *  need to call this, because allegro_exit() will do it for you.
 */
void remove_timer()
{
   if (!timer_installed)
      return;

   _timer_use_retrace = FALSE;

   DISABLE();

   reset_timer();

   _remove_irq(TIMER_INT);

   reset_timer();

   ENABLE();

   _remove_exit_func(remove_timer);
   timer_installed = FALSE;
}



/* find_timer_slot:
 *  Searches the list of user timer callbacks for a specified function, 
 *  returning the position at which it was found, or -1 if it isn't there.
 */
static int find_timer_slot(void (*proc)())
{
   int x;

   for (x=0; x<MAX_TIMERS; x++)
      if (my_int_queue[x].proc == proc)
	 return x;

   return -1;
}



/* install_int_ex:
 *  Installs a function into the list of user timers, or if it is already 
 *  installed, adjusts its speed. This function will be called once every 
 *  speed timer ticks. Returns a negative number if there was no room to 
 *  add a new routine (there is only room for eight). Note that your 
 *  routine is called by the Allegro interrupt handler and not directly 
 *  by the processor, so you should return normally rather than using an 
 *  iret. Your interrupt routine must finish quickly, and you should not 
 *  use large amounts of stack or make any calls to the operating system. 
 */
int install_int_ex(void (*proc)(), long speed)
{
   int x = find_timer_slot(proc);      /* find the handler position */

   if (x < 0)                          /* if not there, find free slot */
      x = find_timer_slot(NULL);

   if (x < 0)                          /* are there any free slots? */
      return -1;

   my_int_queue[x].speed = my_int_queue[x].counter = speed;
   my_int_queue[x].proc = proc; 

   return 0;
}



/* install_int:
 *  Wrapper for install_int_ex, which takes the speed in milliseconds.
 */
int install_int(void (*proc)(), long speed)
{
   return install_int_ex(proc, MSEC_TO_TIMER(speed));
}



/* remove_int:
 *  Removes a function from the list of user timers.
 */
void remove_int(void (*proc)())
{
   int x = find_timer_slot(proc);

   if (x >= 0) {
      my_int_queue[x].proc = NULL;
      my_int_queue[x].speed = 0;
      my_int_queue[x].counter = 0;
   }
}



