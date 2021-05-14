/* 
 *    Example program for the Allegro library, by Shawn Hargreaves.
 *
 *    This program demonstrates how to use the 3d matrix functions.
 *    It isn't a very elegant or efficient piece of code, but it does 
 *    show the stuff in action. I'll leave it to you to design a proper 
 *    model structure and rendering pipeline: after all, the best way to 
 *    do that sort of stuff varies hugely from one game to another.
 */


#include <stdlib.h>
#include <stdio.h>

#include "allegro.h"


#define NUM_SHAPES         8     /* number of bouncing cubes */

#define NUM_VERTICES       8     /* a cube has eight corners */
#define NUM_FACES          6     /* a cube has six faces */


typedef struct VTX
{
   fixed x, y, z;
} VTX;


typedef struct QUAD              /* four vertices makes a quad */
{
   VTX *vtxlist;
   int v1, v2, v3, v4;
} QUAD;


typedef struct SHAPE             /* store position of a shape */
{
   fixed x, y, z;                /* x, y, z position */
   fixed rx, ry, rz;             /* rotations */
   fixed dz;                     /* speed of movement */
   fixed drx, dry, drz;          /* speed of rotation */
} SHAPE;


VTX points[] =                   /* a cube, centered on the origin */
{
   /* vertices of the cube */
   { -32 << 16, -32 << 16, -32 << 16 },
   { -32 << 16,  32 << 16, -32 << 16 },
   {  32 << 16,  32 << 16, -32 << 16 },
   {  32 << 16, -32 << 16, -32 << 16 },
   { -32 << 16, -32 << 16,  32 << 16 },
   { -32 << 16,  32 << 16,  32 << 16 },
   {  32 << 16,  32 << 16,  32 << 16 },
   {  32 << 16, -32 << 16,  32 << 16 },
};


QUAD faces[] =                   /* group the vertices into polygons */
{
   { points, 0, 3, 2, 1 },
   { points, 4, 5, 6, 7 },
   { points, 0, 1, 5, 4 },
   { points, 2, 3, 7, 6 },
   { points, 0, 4, 7, 3 },
   { points, 1, 2, 6, 5 }
};


SHAPE shapes[NUM_SHAPES];        /* a list of shapes */


/* somewhere to put translated vertices */
VTX output_points[NUM_VERTICES * NUM_SHAPES];
QUAD output_faces[NUM_FACES * NUM_SHAPES];


enum { 
   wireframe, 
   flat_shaded, 
   gouraud,
   textured, 
   lit_texture,
   last_mode
} render_mode = wireframe;


char *mode_desc[] = {
   "Wireframe",
   "Flat shaded",
   "Gouraud shaded",
   "Texture mapped",
   "Lit texture map"
};


BITMAP *texture;



/* initialise shape positions */
void init_shapes()
{
   int c;

   for (c=0; c<NUM_SHAPES; c++) {
      shapes[c].x = (random() & 0xFFFFFF) - 0x800000;
      shapes[c].y = (random() & 0xFFFFFF) - 0x800000;
      shapes[c].z = itofix(768);
      shapes[c].rx = 0;
      shapes[c].ry = 0;
      shapes[c].rz = 0;
      shapes[c].dz =  (random() & 0xFFFFF) - 0x80000;
      shapes[c].drx = (random() & 0x1FFFF) - 0x10000;
      shapes[c].dry = (random() & 0x1FFFF) - 0x10000;
      shapes[c].drz = (random() & 0x1FFFF) - 0x10000;
   }
}


/* update shape positions */
void animate_shapes()
{
   int c;

   for (c=0; c<NUM_SHAPES; c++) {
      shapes[c].z += shapes[c].dz;

      if ((shapes[c].z > itofix(1024)) ||
	  (shapes[c].z < itofix(192)))
	 shapes[c].dz = -shapes[c].dz;

      shapes[c].rx += shapes[c].drx;
      shapes[c].ry += shapes[c].dry;
      shapes[c].rz += shapes[c].drz;
   }
}


/* translate shapes from 3d world space to 2d screen space */
void translate_shapes()
{
   int c, d;
   MATRIX matrix;
   VTX *outpoint = output_points;
   QUAD *outface = output_faces;

   for (c=0; c<NUM_SHAPES; c++) {
      /* build a transformation matrix */
      get_transformation_matrix(&matrix, itofix(1),
				shapes[c].rx, shapes[c].ry, shapes[c].rz,
				shapes[c].x, shapes[c].y, shapes[c].z);

      /* output the vertices */
      for (d=0; d<NUM_VERTICES; d++) {
	 apply_matrix(&matrix, points[d].x, points[d].y, points[d].z, &outpoint[d].x, &outpoint[d].y, &outpoint[d].z);
	 persp_project(outpoint[d].x, outpoint[d].y, outpoint[d].z, &outpoint[d].x, &outpoint[d].y);
      }

      /* output the faces */
      for (d=0; d<NUM_FACES; d++) {
	 outface[d] = faces[d];
	 outface[d].vtxlist = outpoint;
      }

      outpoint += NUM_VERTICES;
      outface += NUM_FACES;
   }
}


/* draw a line (for wireframe display) */
void wire(BITMAP *b, VTX *v1, VTX *v2)
{
   int col = MID(128, 255 - fixtoi(v1->z+v2->z) / 16, 255);
   line(b, fixtoi(v1->x), fixtoi(v1->y), fixtoi(v2->x), fixtoi(v2->y), col);
}


/* draw a quad */
void quad(BITMAP *b, VTX *v1, VTX *v2, VTX *v3, VTX *v4, int mode)
{
   int col;
   fixed x, y, z;

   /* four vertices */
   V3D vtx1 = { v1->x, v1->y, v1->z, 0,      0,      0 };
   V3D vtx2 = { v2->x, v2->y, v2->z, 31<<16, 0,      0 };
   V3D vtx3 = { v3->x, v3->y, v3->z, 31<<16, 31<<16, 0 };
   V3D vtx4 = { v4->x, v4->y, v4->z, 0,      31<<16, 0 };

   /* use the cross-product to cull backfaces */
   cross_product(v2->x-v1->x, v2->y-v1->y, 0, v3->x-v2->x, v3->y-v2->y, 0, &x, &y, &z);
   if (z < 0)
      return;

   /* set up the vertex color, differently for each rendering mode */
   switch (mode) {

      case POLYTYPE_FLAT:
	 col = MID(128, 255 - fixtoi(v1->z+v2->z) / 16, 255);
	 vtx1.c = vtx2.c = vtx3.c = vtx4.c = col;
	 break;

      case POLYTYPE_GCOL:
	 vtx1.c = 0xD0;
	 vtx2.c = 0x80;
	 vtx3.c = 0xB0;
	 vtx4.c = 0xFF;
	 break;

      case POLYTYPE_ATEX_LIT:
      case POLYTYPE_PTEX_LIT:
	 vtx1.c = MID(0, 255 - fixtoi(v1->z) / 4, 255);
	 vtx2.c = MID(0, 255 - fixtoi(v2->z) / 4, 255);
	 vtx3.c = MID(0, 255 - fixtoi(v3->z) / 4, 255);
	 vtx4.c = MID(0, 255 - fixtoi(v4->z) / 4, 255);
	 break; 
   }

   /* draw the quad */
   quad3d(b, mode, texture, &vtx1, &vtx2, &vtx3, &vtx4);
}


/* callback for qsort() */
int quad_cmp(const void *e1, const void *e2)
{
   QUAD *q1 = (QUAD *)e1;
   QUAD *q2 = (QUAD *)e2;

   fixed d1 = q1->vtxlist[q1->v1].z + q1->vtxlist[q1->v2].z +
	      q1->vtxlist[q1->v3].z + q1->vtxlist[q1->v4].z;

   fixed d2 = q2->vtxlist[q2->v1].z + q2->vtxlist[q2->v2].z +
	      q2->vtxlist[q2->v3].z + q2->vtxlist[q2->v4].z;

   return d2 - d1;
}


/* draw the shapes calculated by translate_shapes() */
void draw_shapes(BITMAP *b)
{
   int c;
   QUAD *face = output_faces;
   VTX *v1, *v2, *v3, *v4;

   /* depth sort */
   qsort(output_faces, NUM_FACES * NUM_SHAPES, sizeof(QUAD), quad_cmp);

   for (c=0; c < NUM_FACES * NUM_SHAPES; c++) {
      /* find the vertices used by the face */
      v1 = face->vtxlist + face->v1;
      v2 = face->vtxlist + face->v2;
      v3 = face->vtxlist + face->v3;
      v4 = face->vtxlist + face->v4;

      /* draw the face */
      switch (render_mode) {

	 case wireframe:
	    wire(b, v1, v2);
	    wire(b, v2, v3);
	    wire(b, v3, v4);
	    wire(b, v4, v1);
	    break;

	 case flat_shaded:
	 default:
	    quad(b, v1, v2, v3, v4, POLYTYPE_FLAT);
	    break;

	 case gouraud:
	    quad(b, v1, v2, v3, v4, POLYTYPE_GCOL);
	    break;

	 case textured:
	    quad(b, v1, v2, v3, v4, POLYTYPE_ATEX);
	    break;

	 case lit_texture:
	    quad(b, v1, v2, v3, v4, POLYTYPE_ATEX_LIT);
	    break;
      }

      face++;
   }
}


void print_progress(int pos)
{
   if ((pos & 3) == 3) {
      printf("*");
      fflush(stdout);
   }
}


void main()
{
   BITMAP *buffer;
   PALLETE pal;
   int c, w, h;
   int last_retrace_count;

   allegro_init();
   install_keyboard();
   install_mouse();
   install_timer();

   /* color 0 = black */
   pal[0].r = pal[0].g = pal[0].b = 0;

   /* copy the desktop pallete */
   for (c=1; c<64; c++)
      pal[c] = desktop_pallete[c];

   /* make a red gradient */
   for (c=64; c<96; c++) {
      pal[c].r = (c-64)*2;
      pal[c].g = pal[c].b = 0;
   }

   /* make a green gradient */
   for (c=96; c<128; c++) {
      pal[c].g = (c-96)*2;
      pal[c].r = pal[c].b = 0;
   }

   /* set up a greyscale in the top half of the pallete */
   for (c=128; c<256; c++)
      pal[c].r = pal[c].g = pal[c].b = (c-128)/2;

   /* build a lighting table */
   printf("Generating lighting table:\n");
   printf("<................................................................>\r<");
   color_map = malloc(sizeof(COLOR_MAP));
   create_light_table(color_map, pal, 0, 0, 0, print_progress);

   /* set the graphics mode */
   set_gfx_mode(GFX_VGA, 320, 200, 0, 0);
   set_pallete(desktop_pallete);

   if (!gfx_mode_select(&c, &w, &h)) {
      allegro_exit();
      exit(1);
   }

   if (set_gfx_mode(c, w, h, 0, 0) != 0) {
      allegro_exit();
      printf("Error setting graphics mode\n%s\n\n", allegro_error);
      exit(1);
   }

   set_pallete(pal);

   /* make a bitmap for use as a texture map */
   texture = create_bitmap(32, 32);
   clear_to_color(texture, 64);
   line(texture, 0, 0, 31, 31, 1);
   line(texture, 0, 31, 31, 0, 1);
   rect(texture, 0, 0, 31, 31, 1);
   text_mode(-1);
   textout(texture, font, "dead", 0, 0, 2);
   textout(texture, font, "pigs", 0, 8, 2);
   textout(texture, font, "cant", 0, 16, 2);
   textout(texture, font, "fly.", 0, 24, 2);

   /* double buffer the animation */
   buffer = create_bitmap(SCREEN_W, SCREEN_H);

   /* set up the viewport for the perspective projection */
   set_projection_viewport(0, 0, SCREEN_W, SCREEN_H);

   /* initialise the bouncing shapes */
   init_shapes();

   last_retrace_count = retrace_count;

   for (;;) {
      clear(buffer);

      while (last_retrace_count < retrace_count) {
	 animate_shapes();
	 last_retrace_count++;
      }

      translate_shapes();
      draw_shapes(buffer);

      textout(buffer, font, mode_desc[render_mode], 0, 0, 192);
      textout(buffer, font, "Press a key to change", 0, 12, 192);

      vsync();
      blit(buffer, screen, 0, 0, 0, 0, SCREEN_W, SCREEN_H); 

      if (keypressed()) {
	 if ((readkey() & 0xFF) == 27)
	    break;
	 else {
	    render_mode++;
	    if (render_mode >= last_mode)
	       render_mode = wireframe;
	 }
      }
   }

   destroy_bitmap(buffer);
   destroy_bitmap(texture);
   free(color_map);

   exit(0);
}


