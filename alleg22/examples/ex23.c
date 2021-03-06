/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates the use of spline curves to create smooth 
 *    paths connecting a number of node points. This can be useful for 
 *    constructing realistic motion and animations.
 *
 *    The technique is to connect the series of guide points p1..p(n) with
 *    spline curves from p1-p2, p2-p3, etc. Each spline must pass though
 *    both of its guide points, so they must be used as the first and fourth
 *    of the spline control points. The fun bit is coming up with sensible
 *    values for the second and third spline control points, such that the
 *    spline segments will have equal gradients where they meet. I came
 *    up with the following solution:
 *
 *    For each guide point p(n), calculate the desired tangent to the curve
 *    at that point. I took this to be the vector p(n-1) -> p(n+1), which 
 *    can easily be calculated with the inverse tangent function, and gives 
 *    decent looking results. One implication of this is that two dummy 
 *    guide points are needed at each end of the curve, which are used in 
 *    the tangent calculations but not connected to the set of splines.
 *
 *    Having got these tangents, it becomes fairly easy to calculate the
 *    spline control points. For a spline between guide points p(a) and
 *    p(b), the second control point should lie along the positive tangent
 *    from p(a), and the third control point should lie along the negative
 *    tangent from p(b). How far they are placed along these tangents 
 *    controls the shape of the curve: I found that applying a 'curviness'
 *    scaling factor to the distance between p(a) and p(b) works well.
 *
 *    One thing to note about splines is that the generated points are
 *    not all equidistant. Instead they tend to bunch up nearer to the
 *    ends of the spline, which means you will need to apply some fudges
 *    to get an object to move at a constant speed. On the other hand,
 *    in situations where the curve has a noticable change of direction 
 *    at each guide point, the effect can be quite nice because it makes
 *    the object slow down for the curve.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


typedef struct NODE
{
   int x, y;
   fixed tangent;
} NODE;


#define MAX_NODES    1024

NODE nodes[MAX_NODES];

int node_count;

fixed curviness;

int show_tangents;
int show_control_points;



/* calculates the distance between two nodes */
fixed node_dist(NODE n1, NODE n2)
{
   #define SCALE  64

   fixed dx = itofix(n1.x - n2.x) / SCALE;
   fixed dy = itofix(n1.y - n2.y) / SCALE;

   return fsqrt(fmul(dx, dx) + fmul(dy, dy)) * SCALE;
}


/* constructs nodes to go at the ends of the list, for tangent calculations */
NODE dummy_node(NODE node, NODE prev)
{
   NODE n;

   n.x = node.x - (prev.x - node.x) / 8;
   n.y = node.y - (prev.y - node.y) / 8;

   return n;
}


/* calculates a set of node tangents */
void calc_tangents()
{
   int i;

   nodes[0] = dummy_node(nodes[1], nodes[2]);
   nodes[node_count] = dummy_node(nodes[node_count-1], nodes[node_count-2]);
   node_count++;

   for (i=1; i<node_count-1; i++)
      nodes[i].tangent = fatan2(itofix(nodes[i+1].y - nodes[i-1].y),
				itofix(nodes[i+1].x - nodes[i-1].x));
}


/* draws one of the path nodes */
void draw_node(int n)
{
   char b[8];

   circlefill(screen, nodes[n].x, nodes[n].y, 2, 1);

   sprintf(b, "%d", n);
   text_mode(-1);
   textout(screen, font, b, nodes[n].x-7, nodes[n].y-7, 255);
}


/* calculates the control points for a spline segment */
void get_control_points(NODE n1, NODE n2, int points[8])
{
   fixed dist = fmul(node_dist(n1, n2), curviness);

   points[0] = n1.x;
   points[1] = n1.y;

   points[2] = n1.x + fixtoi(fmul(fcos(n1.tangent), dist));
   points[3] = n1.y + fixtoi(fmul(fsin(n1.tangent), dist));

   points[4] = n2.x - fixtoi(fmul(fcos(n2.tangent), dist));
   points[5] = n2.y - fixtoi(fmul(fsin(n2.tangent), dist));

   points[6] = n2.x;
   points[7] = n2.y;
}


/* draws a spline curve connecting two nodes */
void draw_spline(NODE n1, NODE n2)
{
   int points[8];
   int i;

   get_control_points(n1, n2, points);
   spline(screen, points, 255);

   if (show_control_points)
      for (i=1; i<=2; i++)
	 circlefill(screen, points[i*2], points[i*2+1], 2, 2);
}


/* draws the spline paths */
void draw_splines()
{
   char b[80];
   int i;

   clear(screen);

   text_mode(0);
   textout_centre(screen, font, "Spline curve path", SCREEN_W/2, 8, 255);
   sprintf(b, "Curviness = %.2f", fixtof(curviness));
   textout_centre(screen, font, b, SCREEN_W/2, 32, 255);
   textout_centre(screen, font, "Up/down keys to alter", SCREEN_W/2, 44, 255);
   textout_centre(screen, font, "Space to walk", SCREEN_W/2, 68, 255);
   textout_centre(screen, font, "C to display control points", SCREEN_W/2, 92, 255);
   textout_centre(screen, font, "T to display tangents", SCREEN_W/2, 104, 255);

   for (i=1; i<node_count-2; i++)
      draw_spline(nodes[i], nodes[i+1]);

   for (i=1; i<node_count-1; i++) {
      draw_node(i);

      if (show_tangents) {
	 line(screen, nodes[i].x - fixtoi(fcos(nodes[i].tangent) * 24),
		      nodes[i].y - fixtoi(fsin(nodes[i].tangent) * 24),
		      nodes[i].x + fixtoi(fcos(nodes[i].tangent) * 24),
		      nodes[i].y + fixtoi(fsin(nodes[i].tangent) * 24), 2);
      }
   }
}


/* let the user input a list of path nodes */
void input_nodes()
{
   clear(screen);

   text_mode(0);
   textout_centre(screen, font, "Click the left mouse button to add path nodes", SCREEN_W/2, 8, 255);
   textout_centre(screen, font, "Right mouse button or any key to finish", SCREEN_W/2, 24, 255);

   node_count = 1;

   show_mouse(screen);

   do {
   } while (mouse_b);

   clear_keybuf();

   for (;;) {
      if (mouse_b & 1) {
	 if (node_count < MAX_NODES-1) {
	    nodes[node_count].x = mouse_x;
	    nodes[node_count].y = mouse_y;

	    show_mouse(NULL);
	    draw_node(node_count);
	    show_mouse(screen);

	    node_count++;
	 }

	 do {
	 } while (mouse_b & 1);
      }

      if ((mouse_b & 2) || (keypressed())) {
	 if (node_count < 3)
	    alert("You must enter at least two nodes", NULL, NULL, "OK", NULL, 13, 0);
	 else
	    break;
      }
   }

   show_mouse(NULL);

   do {
   } while (mouse_b);

   clear_keybuf();
}


/* moves a sprite along the spline path */
void walk()
{
   #define MAX_POINTS    256

   int points[8];
   int x[MAX_POINTS], y[MAX_POINTS];
   int n, i;
   int npoints;
   int ox, oy;

   clear(screen);

   for (i=1; i<node_count-1; i++)
      draw_node(i);

   do {
   } while (mouse_b);

   clear_keybuf();

   ox = -16;
   oy = -16;

   xor_mode(TRUE);

   for (n=1; n < node_count-2; n++) {
      npoints = (fixtoi(node_dist(nodes[n], nodes[n+1]))+3) / 4;
      if (npoints < 1)
	 npoints = 1;
      else if (npoints > MAX_POINTS)
	 npoints = MAX_POINTS;

      get_control_points(nodes[n], nodes[n+1], points);
      calc_spline(points, npoints, x, y);

      for (i=1; i<npoints; i++) {
	 vsync();
	 circlefill(screen, ox, oy, 6, 2);
	 circlefill(screen, x[i], y[i], 6, 2);
	 ox = x[i];
	 oy = y[i];

	 if ((keypressed()) || (mouse_b))
	    goto getout;
      }
   }

   getout:

   xor_mode(FALSE);

   do {
   } while (mouse_b);

   clear_keybuf();
}


/* main program */
void main()
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

   set_pallete(desktop_pallete);

   input_nodes();
   calc_tangents();

   curviness = ftofix(0.25);
   show_tangents = FALSE;
   show_control_points = FALSE;

   draw_splines();

   for (;;) {
      if (keypressed()) {
	 c = readkey() >> 8;
	 if (c == KEY_ESC)
	    break;
	 else if (c == KEY_UP) {
	    curviness += ftofix(0.05);
	    draw_splines();
	 }
	 else if (c == KEY_DOWN) {
	    curviness -= ftofix(0.05);
	    draw_splines();
	 }
	 else if (c == KEY_SPACE) {
	    walk();
	    draw_splines();
	 }
	 else if (c == KEY_T) {
	    show_tangents = !show_tangents;
	    draw_splines();
	 }
	 else if (c == KEY_C) {
	    show_control_points = !show_control_points;
	    draw_splines();
	 }
      }
   }

   exit(0);
}


