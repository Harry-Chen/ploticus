/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


#include "plg.h"

#define NVARIAT 18
#define NSHAPE 9
#define TORAD 0.0174532


/* MARK - draw a geometric data point using current line type and current color for line elements. */
/* point styles are selected by the code string 
		"symNS[C]" 
   where	N is an integer 1-8 selecting the shape (1=up tri 2=down tri 3=diam 4=square
			5=pent  6=circ  7=right tri  8=left tri)
		S is either 's' for spokes, 'a' for perimeter, or 'n' for no line (fill only)
		C is a fill color which may be different from current line color
			(if omitted, no filling takes place)

   Performance is best when point style and radius remains the same for consecutive points.
*/

static char prevcode[30] = "";
static double prev_r;
static char color[30];
static int inc, variation;
static int nc[] =    {  3,  3,  4,  4,  5, 12, 3, 3, 20 };	/* number of corners (constant) */
static int nt[] =    { 90,270,  0, 45, 90, 90, 0, 180, 90 };    /* location (in degrees) of first corner (constant) */
static double h[24][2]; /* the offsets */

/* ==================================== */
PLG_mark_initstatic()
{
strcpy( prevcode, "" );
return( 0 );
}

/* ==================================== */
PLG_mark( x, y, code, r )
double x, y; 	/* point location in abs space */
char code[];	/* pre-set symbol name */
double r; 	/* radius of dot in absolute units */
{
int i;
double g, theta;

/* no-op code */
if( strcmp( code, "sym00" ) == 0 ) return( 0 );

/* gif image as symbol */
if( strcmp( code, "gif" )==0 ) {
	Eimplace( x, y, "centered", 1.0, 1.0 );
	return( 0 );
	}

if( strcmp( code, prevcode ) != 0 || r != prev_r ) {
	strcpy( prevcode, code );
	prev_r = r;
	inc = ((code[3] - '0') -1 ) % NSHAPE;
	if( code[4] == '\0' ) code[4] = 'a';
	variation = code[4];
	if( strlen( code ) > 5 )strcpy( color, &code[5] );
	else color[0] = '\0';

	theta = 360.0 / (double)nc[inc];
	/* get offsets */
	g = nt[inc];
	for( i = 0; i < nc[inc]; i++ ) {
		h[i][0] = r * cos( g * TORAD );
		h[i][1] = r * sin( g * TORAD );
		g += theta;
		}
	}
/* color point */
if( color[0] != '\0' ) {
	Emov( x+h[0][0], y+h[0][1] );
	for( i = 1; i < nc[inc]; i++ ) Epath( x+h[i][0], y+h[i][1] );
	Ecolorfill( color );
	}

/* draw spokes */
if( variation == 's' ) 
	for( i = 0; i < nc[inc]; i++ ) { 
		Emov( x, y ); Elin( x+h[i][0], y+h[i][1] ); 
		}

else if( variation == 'n' ) ;

/* draw perimeter */
else 	{
	Emov( x+h[0][0], y+h[0][1] );
	for( i = 1; i < nc[inc]; i++ ) Elin( x+h[i][0], y+h[i][1] );
	Elin( x+h[0][0], y+h[0][1] ); /* close to starting point.. */
	Elin( x+h[1][0], y+h[1][1] ); /* and do one more so last corner is done right.. */
	}

return( 0 );
}

#ifdef SUSPENDED
/* ======================================= */
/* CIRCLE - draw a circle of n "sides" */
PLG_circle( cx, cy, r, color, outline )
double cx, cy; 	/* point location in abs space */
double r; 	/* radius of dot in absolute units */
char *color;	/* color name, or "" for no fill */
int outline;    /* if 1, circle will be outlined. */
{
double theta, x, y, g;
int i;
static double hx[32], hy[32];
static int first = 1;
int n;

n = 25;


if( first ) {
	first = 0;
	theta = 360.0 / (double)n;
	/* get offsets */
	g = 0.0;
	for( i = 0; i < n+1; i++ ) {
		hx[i] = cos( (g * TORAD );
		hy[i] = sin( (g * TORAD );
		g += theta;
		}
	}
if( color[0] != '\0' ) { /* (was strlen(color) >= 0 ) (?) */
	for( i = 0; i < n+1; i++ ) {
		if( i == 0 ) Emov( cx+(r*hx[i]), cy+(r*hy[i]) );
		else Epath( cx+(r*hx[i]), cy+(r*hy[i]) );
		}
	Ecolorfill( color );
	}
if( outline ) {
	for( i = 0; i < n+1; i++ ) {
		if( i == 0 ) Emov( cx+(r*hx[i]), cy+(r*hy[i]) );
		else Elin( cx+(r*hx[i]), cy+(r*hy[i]) );
		}
	}
}
#endif

/* ========================================= */
/* ELLIPSE  - draw an elipse */
PLG_ellipse( cx, cy, r1, r2, color, outline )
double cx, cy, r1, r2;
char *color;	/* color name, or "" for no fill */
int outline;    /* if 1, circle will be outlined. */
{
double theta, x, y, g;
int i;
static double hx[32], hy[32];
int n;

n = 25;

theta = 360.0 / (double)n;
/* get offsets */
g = 0.0;
for( i = 0; i < n+1; i++ ) {
        hx[i] = cos( (g * TORAD ) );
        hy[i] = sin( (g * TORAD ) );
        g += theta;
        }

if( color[0] != '\0' ) {
	for( i = 0; i < n+1; i++ ) {
       		if( i == 0 ) Emov( cx+(r1*hx[i]), cy+(r2*hy[i]) );
        	else Epath( cx+(r1*hx[i]), cy+(r2*hy[i]) );
		}
	Ecolorfill( color );
        }

if( outline ) {
	for( i = 0; i < n+1; i++ ) {
	        if( i == 0 ) Emov( cx+(r1*hx[i]), cy+(r2*hy[i]) );
	        else Elin( cx+(r1*hx[i]), cy+(r2*hy[i]) );
	        }
	}
}
