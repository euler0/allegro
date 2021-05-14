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
 *      DOS joystick routines (the actual polling function is in misc.s).
 *
 *      Based on code provided by Jonathan Tarbox and Marcel de Kogel.
 *
 *      CH Flightstick Pro support by Fabian Nunez.
 *
 *      Matthew Bowie added the support for 4-button joysticks.
 *
 *      See readme.txt for copyright information.
 */


#ifndef DJGPP
#error This file should only be used by the djgpp version of Allegro
#endif

#include <stdlib.h>
#include <dos.h>
#include <errno.h>

#include "allegro.h"
#include "internal.h"


/* global joystick position variables */
int joy_x = 0;
int joy_y = 0;
int joy_left = FALSE;
int joy_right = FALSE;
int joy_up = FALSE;
int joy_down = FALSE;
int joy_b1 = FALSE;
int joy_b2 = FALSE;

int joy_type = JOY_TYPE_STANDARD;

/* CH Flightstick Pro variables */
int joy_b3 = FALSE;
int joy_b4 = FALSE;
int joy_hat = JOY_HAT_CENTRE;
int joy_throttle = 0;

/* joystick state information */
#define JOYSTICK_PRESENT            1
#define JOYSTICK_CALIB_TL           2
#define JOYSTICK_CALIB_BR           4
#define JOYSTICK_CALIB_THRTL_MIN    8
#define JOYSTICK_CALIB_THRTL_MAX    16

static int joystick_flags = 0;

/* calibrated position values */
static int joycentre_x, joycentre_y;
static int joyx_min, joyx_low_margin, joyx_high_margin, joyx_max;
static int joyy_min, joyy_low_margin, joyy_high_margin, joyy_max;
static int joy_thr_min, joy_thr_max;

/* for filtering out bad input values */
static int joy_old_x, joy_old_y;


#define MASK_1X      1
#define MASK_1Y      2
#define MASK_2X      4
#define MASK_2Y      8
#define MASK_PLAIN   (MASK_1X | MASK_1Y)



/* poll_mask:
 *  Returns a mask indicating which axes to poll.
 */
static int poll_mask()
{
   switch (joy_type) {

      case JOY_TYPE_FSPRO:
	 return MASK_PLAIN | MASK_2Y;

      default:
	 return MASK_PLAIN;
   }
}



/* poll:
 *  Wrapper function for joystick polling.
 */
static int poll(int *x, int *y, int *x2, int *y2, int poll_mask)
{
   int ret;

   enter_critical();

   ret = _poll_joystick(x, y, x2, y2, poll_mask);

   exit_critical();

   return ret;
}



/* averaged_poll:
 *  For calibration it is crucial that we get the right results, so we
 *  average out several attempts.
 */
static int averaged_poll(int *x, int *y, int *x2, int *y2, int mask)
{
   #define AVERAGE_COUNT   4

   int x_tmp, y_tmp, x2_tmp, y2_tmp;
   int x_total, y_total, x2_total, y2_total;
   int c;

   x_total = y_total = x2_total = y2_total = 0;

   for (c=0; c<AVERAGE_COUNT; c++) {
      if (poll(&x_tmp, &y_tmp, &x2_tmp, &y2_tmp, mask) != 0)
	 return -1;

      x_total += x_tmp;
      y_total += y_tmp;
      x2_total += x2_tmp;
      y2_total += y2_tmp;
   }

   *x = x_total / AVERAGE_COUNT;
   *y = y_total / AVERAGE_COUNT;
   *x2 = x2_total / AVERAGE_COUNT;
   *y2 = y2_total / AVERAGE_COUNT;

   return 0;
}



/* initialise_joystick:
 *  Calibrates the joystick by reading the axis values when the joystick
 *  is centered. You should call this before using other joystick functions,
 *  and must make sure the joystick is centered when you do so. Returns
 *  non-zero if no joystick is present.
 */
int initialise_joystick()
{
   int dummy;

   joy_old_x = joy_old_y = 0;

   if (averaged_poll(&joycentre_x, &joycentre_y, &dummy, &dummy, poll_mask()) != 0) {
      joy_x = joy_y = 0;
      joy_left = joy_right = joy_up = joy_down = FALSE;
      joy_b1 = joy_b2 = joy_b3 = joy_b4 = FALSE;
      joy_hat = JOY_HAT_CENTRE;
      joy_throttle = 0;
      joystick_flags = 0;
      return -1;
   }

   joystick_flags = JOYSTICK_PRESENT;

   return 0;
}



/* sort_out_middle_values:
 *  Sets up the values used by sort_out_analogue() to create a 'dead'
 *  region in the centre of the joystick range.
 */
static void sort_out_middle_values()
{
   joyx_low_margin  = joycentre_x - (joycentre_x - joyx_min) / 8;
   joyx_high_margin = joycentre_x + (joyx_max - joycentre_x) / 8;
   joyy_low_margin  = joycentre_y - (joycentre_y - joyy_min) / 8;
   joyy_high_margin = joycentre_y + (joyy_max - joycentre_y) / 8;
}



/* calibrate_joystick_tl:
 *  For analogue access to the joystick, call this after 
 *  initialise_joystick(), with the joystick at the top left 
 *  extreme, and also call calibrate_joystick_br();
 */
int calibrate_joystick_tl()
{
   int dummy;

   if (!(joystick_flags & JOYSTICK_PRESENT)) {
      joystick_flags &= (~JOYSTICK_CALIB_TL);
      return -1;
   }

   if (averaged_poll(&joyx_min, &joyy_min, &dummy, &dummy, MASK_PLAIN) != 0) {
      joystick_flags &= (~JOYSTICK_CALIB_TL);
      return -1;
   }

   if (joystick_flags & JOYSTICK_CALIB_BR)
      sort_out_middle_values();

   joystick_flags |= JOYSTICK_CALIB_TL;

   return 0;
}



/* calibrate_joystick_br:
 *  For analogue access to the joystick, call this after 
 *  initialise_joystick(), with the joystick at the bottom right 
 *  extreme, and also call calibrate_joystick_tl();
 */
int calibrate_joystick_br()
{
   int dummy;

   if (!(joystick_flags & JOYSTICK_PRESENT)) {
      joystick_flags &= (~JOYSTICK_CALIB_BR);
      return -1;
   }

   if (averaged_poll(&joyx_max, &joyy_max, &dummy, &dummy, MASK_PLAIN) != 0) {
      joystick_flags &= (~JOYSTICK_CALIB_BR);
      return -1;
   }

   if (joystick_flags & JOYSTICK_CALIB_TL)
      sort_out_middle_values();

   joystick_flags |= JOYSTICK_CALIB_BR;

   return 0;
}



/* calibrate_joystick_throttle_min:
 *  For analogue access to the FSPro's throttle, call this after 
 *  initialise_joystick(), with the throttle at the "minimum" extreme
 *  (the user decides whether this is all the way forwards or all the
 *  way back), and also call calibrate_joystick_throttle_max().
 */
int calibrate_joystick_throttle_min()
{
   int dummy;

   if (!(joystick_flags & JOYSTICK_PRESENT)) {
      joystick_flags &= (~JOYSTICK_CALIB_THRTL_MIN);
      return -1;
   }

   if (averaged_poll(&dummy, &dummy, &dummy, &joy_thr_min, MASK_2Y) != 0) {
      joystick_flags &= (~JOYSTICK_CALIB_THRTL_MIN);
      return -1;
   }

   joystick_flags |= JOYSTICK_CALIB_THRTL_MIN;

   /* prevent division by zero errors if user miscalibrated */
   if ((joystick_flags & JOYSTICK_CALIB_THRTL_MAX) &&
       (joy_thr_min == joy_thr_max))
     joy_thr_min = 255 - joy_thr_max;

   return 0;
}



/* calibrate_joystick_throttle_max:
 *  For analogue access to the FSPro's throttle, call this after 
 *  initialise_joystick(), with the throttle at the "maximum" extreme
 *  (the user decides whether this is all the way forwards or all the
 *  way back), and also call calibrate_joystick_throttle_min().
 */
int calibrate_joystick_throttle_max()
{
   int dummy;

   if (!(joystick_flags & JOYSTICK_PRESENT)) {
      joystick_flags &= (~JOYSTICK_CALIB_THRTL_MAX);
      return -1;
   }

   if (averaged_poll(&dummy, &dummy, &dummy, &joy_thr_max, MASK_2Y) != 0) {
      joystick_flags &= (~JOYSTICK_CALIB_THRTL_MAX);
      return -1;
   }

   joystick_flags |= JOYSTICK_CALIB_THRTL_MAX;

   /* prevent division by zero errors if user miscalibrated */
   if ((joystick_flags & JOYSTICK_CALIB_THRTL_MIN) &&
       (joy_thr_min == joy_thr_max))
     joy_thr_max = 255 - joy_thr_min;

   return 0;
}



/* sort_out_analogue:
 *  There are a couple of problems with reading analogue input from the PC
 *  joystick. For one thing, joysticks tend not to centre repeatably, so
 *  we need a small 'dead' zone in the middle. Also a lot of joysticks aren't
 *  linear, so the positions less than centre need to be handled differently
 *  to those above the centre.
 */
static int sort_out_analogue(int x, int min, int low_margin, int centre, int high_margin, int max)
{
   if (x < min) {
      return -128;
   }
   else if (x < low_margin) {
      return -128 + (x - min) * 128 / (low_margin - min);
   }
   else if (x <= high_margin) {
      return 0;
   }
   else if (x <= max) {
      return 128 - (max - x) * 128 / (max - high_margin);
   }
   else
      return 128;
}



/* poll_joystick:
 *  Updates the global joystick position variables. You must call
 *  calibrate_joystick() before using this.
 */
void poll_joystick()
{
   int x, y, x2, y2;
   unsigned char status;

   if (joystick_flags & JOYSTICK_PRESENT) {
      poll(&x, &y, &x2, &y2, poll_mask());

      status = inportb(0x201);

      if ((ABS(x-joy_old_x) < x/4) && (ABS(y-joy_old_y) < y/4)) {
	 if ((joystick_flags & JOYSTICK_CALIB_TL) && 
	     (joystick_flags & JOYSTICK_CALIB_BR)) {
	    joy_x = sort_out_analogue(x, joyx_min, joyx_low_margin, joycentre_x, joyx_high_margin, joyx_max);
	    joy_y = sort_out_analogue(y, joyy_min, joyy_low_margin, joycentre_y, joyy_high_margin, joyy_max);
	 }
	 else {
	    joy_x = x - joycentre_x;
	    joy_y = y - joycentre_y;
	 }

	 joy_left  = (x < (joycentre_x/2));
	 joy_right = (x > (joycentre_x*3/2));
	 joy_up    = (y < (joycentre_y/2));
	 joy_down  = (y > ((joycentre_y*3)/2));
      }

      joy_b1 = ((status & 0x10) == 0);
      joy_b2 = ((status & 0x20) == 0);

      if (joy_type == JOY_TYPE_FSPRO) {
	 if ((joystick_flags & JOYSTICK_CALIB_THRTL_MIN) && 
	     (joystick_flags & JOYSTICK_CALIB_THRTL_MAX)) {
	    joy_throttle = (y2 - joy_thr_min) * 255 / (joy_thr_max - joy_thr_min);
	    if (joy_throttle < 0) joy_throttle = 0;
	    if (joy_throttle > 255) joy_throttle = 255;
	 } 

	 joy_b3 = ((status & 0x40) == 0);
	 joy_b4 = ((status & 0x80) == 0);

	 if ((status & 0x30) == 0) {
	    joy_b1 = joy_b2 = joy_b3 = joy_b4 = FALSE;

	    joy_hat = 1 + ((status ^ 0xff) >> 6);
	 }
	 else
	    joy_hat = JOY_HAT_CENTRE;
      } 
      else if (joy_type == JOY_TYPE_4BUTTON) {
	 joy_b3 = ((status & 0x40) == 0);
	 joy_b4 = ((status & 0x80) == 0);
      }

      joy_old_x = x;
      joy_old_y = y;
   }
}



#define JOYSTICK_OLD_MAGIC    0x6F6A6C61
#define JOYSTICK_MAGIC        0x796A6C61



/* save_joystick_data:
 *  After calibrating a joystick, this function can be used to save the
 *  information into the specified file, from where it can later be 
 *  restored by calling load_joystick_data().
 */
int save_joystick_data(char *filename)
{
   PACKFILE *f;

   f = pack_fopen(filename, F_WRITE);
   if (!f)
      return -1;

   pack_iputl(JOYSTICK_MAGIC, f);
   pack_iputl(joy_type, f);
   pack_iputl(joystick_flags, f);
   pack_iputl(joycentre_x, f);
   pack_iputl(joycentre_y, f);
   pack_iputl(joyx_min, f);
   pack_iputl(joyx_low_margin, f);
   pack_iputl(joyx_high_margin, f);
   pack_iputl(joyx_max, f);
   pack_iputl(joyy_min, f);
   pack_iputl(joyy_low_margin, f);
   pack_iputl(joyy_high_margin, f);
   pack_iputl(joyy_max, f);
   pack_iputl(joy_thr_min, f);
   pack_iputl(joy_thr_max, f);

   pack_fclose(f);

   return errno;
}



/* load_joystick_data:
 *  Restores a set of joystick calibration data previously saved by
 *  save_joystick_data().
 */
int load_joystick_data(char *filename)
{
   PACKFILE *f;
   long type;

   f = pack_fopen(filename, F_READ);
   if (!f)
      return -1;

   type = pack_igetl(f);

   if ((type != JOYSTICK_MAGIC) && (type != JOYSTICK_OLD_MAGIC)) {
      pack_fclose(f);
      return -1;
   }

   if (type != JOYSTICK_OLD_MAGIC)
      joy_type = pack_igetl(f);
   else
      joy_type = JOY_TYPE_STANDARD;

   joystick_flags = pack_igetl(f);
   joycentre_x = pack_igetl(f);
   joycentre_y = pack_igetl(f);
   joyx_min = pack_igetl(f);
   joyx_low_margin = pack_igetl(f);
   joyx_high_margin = pack_igetl(f);
   joyx_max = pack_igetl(f);
   joyy_min = pack_igetl(f);
   joyy_low_margin = pack_igetl(f);
   joyy_high_margin = pack_igetl(f);
   joyy_max = pack_igetl(f);

   if (type != JOYSTICK_OLD_MAGIC) {
      joy_thr_min = pack_igetl(f);
      joy_thr_max = pack_igetl(f);
   }

   pack_fclose(f);

   joy_old_x = joy_old_y = 0;

   return errno;
}


