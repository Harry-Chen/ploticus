/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


/* Routines for color conversions */
#include <stdio.h>
#define Eerr(a,b,c)  TDH_err(a,b,c)

#define MAXCOLORS 34
#define stricmp( s, t )              strcasecmp( s, t )

struct PLG_colorlist {
	char *name;
	double r, g, b;
	};

/* these are constants.. */
static struct PLG_colorlist colorname[MAXCOLORS] = {
	{ "white", 1, 1, 1 },
	{ "black", 0, 0, 0 },
	{ "transparent", 1, 1, 1 },

	{ "yellow", 1, 1, 0 },
	{ "yellow2", .92, .92, 0 }, 
	{ "dullyellow", 1, .9, .6 }, 
	{ "yelloworange", 1, .85, 0 },

	{ "red", 1, 0, 0 },
	{ "magenta", 1, .3, .5 },
	{ "tan1", .9, .83, .79 },
	{ "tan2", .7, .6, .6 },
	{ "coral", 1, .6, .6 },
	{ "claret", .7, .3, .3 },
	{ "pink", 1.0, .8, .8 },

	{ "brightgreen", 0, 1, 0 },
	{ "green", 0, .7, 0 },
	{ "teal", .0, 0.5, .2 },
	{ "drabgreen", .6, .8, .6 },
	{ "kelleygreen", .3, .6, .3 },
	{ "yellowgreen", .6, .9, .6 }, 
	{ "limegreen", .8, 1, .7 },

	{ "brightblue", 0, 0, 1 },
	{ "blue", 0, .4, .8 },
	{ "skyblue", .7, .8, 1 },
	{ "darkblue", 0, 0, .60 },
	{ "oceanblue", 0, .5, .8 }, 

	{ "purple", .47, 0, .47 },
	{ "lightpurple", .67, .3, .67 },
	{ "lavender", .8, .7, .8 },
	{ "powderblue", .6, .6, 1 },
	{ "powderblue2", .7, .7, 1 },

	{ "orange", 1, .62, .14 },
	{ "redorange", 1, .5, 0 },
	{ "lightorange", 1, .80, .60 } };

/* =============================== */
PLG_colorname_to_rgb( color, r, g, b )
char *color;
double *r, *g, *b;
{
int i;
for( i = 0; i < MAXCOLORS; i++ ) {
	if( stricmp( color, colorname[i].name )==0 ) {
		*r = colorname[i].r;
		*g = colorname[i].g;
		*b = colorname[i].b;
		return( 0 );
		}
	}
Eerr( 12001, "Color not defined", color );
*r = 0; *g = 0; *b = 0;
return( 1 );
}

/* =============================== */
/* map r, g, b to a shade of gray */
double
PLG_rgb_to_gray( r, g, b )
double r, g, b;
{
double gray;
if( r == g && g == b ) return( r );
b = b / 5.0; /* 0 - 0.23 */
g = g / 3.333; /* 0 - 0.3 */
r = r / 2.5;	/* 0 - 0.4 */
gray = 0.3 + r + g + b;
if( gray > 0.95 ) gray = 0.95;
return( gray );
}

/* =============================== */
/* for i values 0 - 19 assign a usable color */
PLG_icolor( i, color )
int i;
char *color;
{
char *c;
	switch( i % 20 ) {
		case 0: c = "red"; break;
		case 1: c = "brightblue"; break;
		case 2: c = "green"; break;
		case 3: c = "yellow2"; break;
		case 4: c = "lightpurple"; break;
		case 5: c = "orange"; break;
		case 6: c = "gray(0.7)"; break;
		case 7: c = "coral"; break;
		case 8: c = "skyblue"; break;
		case 9: c = "drabgreen"; break;
		case 10: c = "lightorange"; break;
		case 11: c = "lavender"; break;
		case 12: c = "gray(0.85)"; break;
		case 13: c = "claret"; break;
		case 14: c = "darkblue"; break;
		case 15: c = "teal"; break;
		case 16: c = "yellow"; break;
		case 17: c = "powderblue2"; break;
		case 18: c = "redorange"; break;
		case 19: c = "tan1"; break; 
		}
strcpy( color, c );
return( 0 );
}
