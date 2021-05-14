/*
   MATHTEST.C:
   Fixed point math test program for the Allegro library.
   
   By Shawn Hargreaves, 1994.
*/

#include "math.h"
#include "errno.h"
#include "stdlib.h"
#include "string.h"

#include "allegro.h"

#define PI  M_PI
#ifdef BORLAND
short _RTLENTRY _EXPFUNC printf(const char * __format, ...);
#else
int  printf(const char*, ...);
#endif

void usage(void);
void show_bin(char *s, double fx, double fy, double fz, fixed x, fixed y, fixed z);
void show_trig(char *s, double fx, double fy, double fz, fixed x, fixed y, fixed z);
void show_itrig(char *s, double fx, double fy, fixed x, fixed y);
void show_itrig2(char *s, double fx, double fy, double fz, fixed x, fixed y, fixed z);
void show_unry(char *s, double fx, double fy, fixed x, fixed y);


void usage()
{
   printf("\nMATHTEST: fixed point math test program\n");
   printf("A test program for the Allegro Library, by Shawn Hargreaves, 1994\n\n");
   printf("Usage: 'mathtest x + y'        / add\n");
   printf("       'mathtest x - y'        / subtract\n");
   printf("       'mathtest x * y'        / multiply\n");
   printf("       'mathtest x / y'        / divide\n");
   printf("       'mathtest sin x'        / sin\n");
   printf("       'mathtest cos x'        / cosine\n");
   printf("       'mathtest tan x'        / tangent\n");
   printf("       'mathtest asin x'       / inverse sin\n");
   printf("       'mathtest acos x'       / inverse cosine\n");
   printf("       'mathtest atan x'       / inverse tangent\n");
   printf("       'mathtest atan2 x y'    / inverse tangent 2\n");
   printf("       'mathtest sqrt x'       / square root\n");
   remove_divzero();
   exit(1);
}



void show_bin(s,fx,fy,fz,x,y,z)
char *s;
double fx, fy, fz;
fixed x, y, z;
{
   printf("Float: %f %s %f = %f\n", fx, s, fy, fz);
   printf("Fixed: %f %s %f = %f\n", fixtof(x), s, fixtof(y), fixtof(z));
   if (errno)
      printf("Error %d\n", errno);
   remove_divzero();
   exit(0);
}



void show_trig(s,fx,fy,fz,x,y,z)
char *s;
double fx, fy, fz;
fixed x, y, z;
{
   printf("Float: %s %f deg (%f rad) = %f\n", s, fx, fy, fz);
   printf("Fixed: %s %f deg (%f bin) = %f\n", s, fixtof(x), fixtof(y),
							    fixtof(z));
   if (errno)
      printf("Error %d\n", errno);
   remove_divzero();
   exit(0);
}



void show_itrig(s,fx,fy,x,y)
char *s;
double fx, fy;
fixed x, y;
{
   printf("Float: %s %f = %f deg (%f rad)\n", s, fx, fy * 180.0 / PI, fy);
   printf("Fixed: %s %f = %f deg (%f bin)\n", s, fixtof(x),
				       fixtof(y) * 360.0 / 256.0, fixtof(y));
   if (errno)
      printf("Error %d\n", errno);
   remove_divzero();
   exit(0);
}



void show_itrig2(s,fx,fy,fz,x,y,z)
char *s;
double fx, fy, fz;
fixed x, y, z;
{
   printf("Float: %s %f %f = %f deg (%f rad)\n", s, fx, fy,
						fz * 180.0 / PI, fz);
   printf("Fixed: %s %f %f = %f deg (%f bin)\n", s, fixtof(x), fixtof(y),
				       fixtof(z) * 360.0 / 256.0, fixtof(z));
   if (errno)
      printf("Error %d\n", errno);
   remove_divzero();
   exit(0);
}



void show_unry(s,fx,fy,x,y)
char *s;
double fx, fy;
fixed x, y;
{
   printf("Float: %s %f = %f\n", s, fx, fy);
   printf("Fixed: %s %f = %f\n", s, fixtof(x), fixtof(y));
   if (errno)
      printf("Error %d\n", errno);
   remove_divzero();
   exit(0);
}



void main(argc,argv)
short argc;
char *argv[];
{
   fixed x,y;
   double fx,fy;

   install_divzero();

   if (argc==4) {
      fx = atof(argv[1]);
      fy = atof(argv[3]);
      x = ftofix(fx);
      y = ftofix(fy);
      
      if (argv[2][1]==0) {

	 if (argv[2][0]=='+')
	    show_bin("+", fx, fy, fx+fy, x, y, x+y);
   
	 if (argv[2][0]=='-')
	    show_bin("-", fx, fy, fx-fy, x, y, x-y);
   
	 if (argv[2][0]=='*')
	    show_bin("*", fx, fy, fx*fy, x, y, fmul(x,y));
   
	 if (argv[2][0]=='/')
	    show_bin("/", fx, fy, fx/fy, x, y, fdiv(x,y));
      }

      if (strcmp(argv[1],"atan2")==0) {
	 fx = atof(argv[2]);
	 fy = atof(argv[3]);
	 x = ftofix(fx);
	 y = ftofix(fy);
	 show_itrig2("atan2", fx, fy, atan2(fx,fy), x, y, fatan2(x,y));
      }
   }

   if (argc==3) {
      fx = atof(argv[2]);
      fy = fx * PI / 180.0;
      x = ftofix(fx);
      y = ftofix(fx * 256.0 / 360);
	 
      if (strcmp(argv[1],"sin")==0)
	 show_trig("sin", fx, fy, sin(fy), x, y, fsin(y));

      if (strcmp(argv[1],"cos")==0)
	 show_trig("cos", fx, fy, cos(fy), x, y, fcos(y));

      if (strcmp(argv[1],"tan")==0)
	 show_trig("tan", fx, fy, tan(fy), x, y, ftan(y));

      if (strcmp(argv[1],"asin")==0)
	 show_itrig("asin", fx, asin(fx), x, fasin(x));

      if (strcmp(argv[1],"acos")==0)
	 show_itrig("acos", fx, acos(fx), x, facos(x));

      if (strcmp(argv[1],"atan")==0)
	 show_itrig("atan", fx, atan(fx), x, fatan(x));

      if (strcmp(argv[1],"sqrt")==0)
	 show_unry("sqrt", fx, sqrt(fx), x, fsqrt(x));
   }

   usage();
}

