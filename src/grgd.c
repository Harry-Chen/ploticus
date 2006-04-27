/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


/* 
 Graphcore interface to gd library.
 Notes: gd renders text such that the TOP of the character box is at x, y

 This module supports these #defines: 
	GD13  (create GIF only using GD 1.3; no import)
	GD16  (create PNG only using GD 1.6; also can import PNG)
	GD18   (use GD 1.8 to create PNG, JPEG, or WBMP; also can import these formats)
	GDFREETYPE (use FreeType font rendering; may be used only when GD18 is in effect)
*/

#include <stdio.h>
#include <string.h>

extern double PLG_xsca_inv(), PLG_ysca_inv();
#define Exsca( h )      PLG_xsca( h )
#define Eysca( h )      PLG_ysca( h )
#define Exsca_inv( h )  PLG_xsca_inv( h )
#define Eysca_inv( h )  PLG_ysca_inv( h )
#define Eerr(a,b,c)  TDH_err(a,b,c)

#define VERT 1.570796  /* 90 degrees, expressed in radians */

static char g_fmt[20] = "";

/* ================================= */
/* SETIMFMT - set the image format that we will be creating.. 
   allowable values for fmt are "gif", "png", "jpeg", etc.
 */
PLGG_setimfmt( fmt )
char *fmt;
{
strcpy( g_fmt, fmt );
return( 0 );
}
/* ================================= */
/* GETIMFMT - allow other modules to find out the image format.
   Format name is copied into fmt. */
PLGG_getimfmt( fmt )
char *fmt;
{
strcpy( fmt, g_fmt );
return( 0 );
}


/* =============================================================================== */
#ifndef NOGD

#include "gd.h"
#include "gdfontg.h"
#include "gdfontl.h"
#include "gdfontmb.h"
#include "gdfonts.h"
#include "gdfontt.h"

#define MAX_D_ROWS 1000
#define NBRUSH 8
#define CHARHW 1.75 /* 2.0 */
#define MINTEXTSIZE 4 /* anything less than this is rendered as a line for thumbnails */
#define stricmp(a,b) strcasecmp(a,b)

static gdImagePtr Gm; /* image */
static gdImagePtr Gm2; /* secondary image */
static char Gm2filename[256] = "";
static double Gm2xscale = 1.0;
static double Gm2yscale = 1.0;
static gdFontPtr Gfont;  /* current font */
static int Gxmax, Gymax; /* drawing area image size */
static double Goldx = 0.0, Goldy = 0.0; /* last x,y */
static double Gcharwidth;
static int Gvertchar = 0;
static int Gtextsize;
static int Gcurcolor;
static gdPoint Gptlist[MAX_D_ROWS];
static int Gnpts = 0;
static int Gcurlinestyle = 0;
static double Gcurlinewidth = 1.0;
static double Gcurdashscale = 1.0;
static gdImagePtr Gbrush[NBRUSH]; /* brush images */
static int Gdash[10][6]= { {1}, {1,1}, {3,1}, {5,1}, {2,1,1,1}, {4,1,1,1}, {6,1,1,1},
                          {2,1,1,1,1,1}, {4,1,1,1,1,1}, {6,1,1,1,1,1} };  /* constants */
static int Gndash[10] = { 1, 2, 2, 2, 4, 4, 4, 6, 6, 6 }; /* constants */
static int Gdashpat[1000];
static int Ginitialized = 0;
static int Gtransparent_color = -1; 
static int Gblack = 0;		    
static char GFTfont[80] = "";
static int GFTbox[8];
static double GFTsize;


/* ================================= */
PLGG_initstatic() 
{
strcpy( g_fmt, "" );
strcpy( Gm2filename, "" );
Gm2xscale = 1.0;
Gm2yscale = 1.0;
Goldx = 0.0, Goldy = 0.0;
Gvertchar = 0;
Gnpts = 0;
Gcurlinestyle = 0;
Gcurlinewidth = 1.0;
Gcurdashscale = 1.0;
Ginitialized = 0;
Gtransparent_color = -1;
Gblack = 0;
strcpy( GFTfont, "" );

return( 0 );
}


/* ================================= */
/* GETIMG - allow API access to the image and its size */
gdImagePtr
PLGG_getimg( width, height )
int *width, *height; /* pixels */
{
if( !Ginitialized ) return( NULL );
*width = Gxmax;
*height = Gymax;
return( Gm );
}


/* ================================ */
PLGG_setup( name, pixelsinch, ux, uy, upleftx, uplefty )
char *name;
int pixelsinch;
double ux, uy; /* size of image in inches x y */
int upleftx, uplefty; /* position of window - not used by this driver */
{
int i;

if( Ginitialized ) { /* could be if a late setsize was issued.. */
	gdImageDestroy( Gm );
	for( i = 0; i < NBRUSH; i++ ) gdImageDestroy( Gbrush[i] );
	}
Ginitialized = 1;

Gxmax = (int)(ux * pixelsinch );
Gymax = (int)(uy * pixelsinch );

/* Allocate pixels.. */
Gm = gdImageCreate( Gxmax, Gymax );
if( Gm == NULL ) return( Eerr( 12003, "Cannot create working image", "" ) );
for( i = 0; i < NBRUSH; i++ ) {
	Gbrush[i] = gdImageCreate( i+1, i+1 );
	if( Gbrush[i] == NULL ) return( Eerr( 12004, "Cannot create brush image", "" ) );
	}


PLGG_color( "white" );
PLGG_color( "black" );
gdImageSetBrush( Gm, Gbrush[0] );

return( 0 );
}

/* ================================ */
PLGG_moveto( x, y )
double x, y;
{
Goldx = x;
Goldy = y;
return( 0 );
}
/* ================================ */
PLGG_lineto( x, y )
double x, y;
{
int a, b, c, d;
a = Exsca( Goldx ); b = Eysca( Goldy ); c = Exsca( x ); d = Eysca( y );
/* gdImageLine( Gm, a, b, c, d, gdStyled ); */
gdImageLine( Gm, a, b, c, d, gdStyledBrushed ); 
Goldx = x; 
Goldy = y;
return( 0 );
}

/* ================================ */
PLGG_rawline( a, b, c, d )
int a, b, c, d;
{
gdImageLine( Gm, a, b, c, d, gdStyledBrushed ); 
return( 0 );
}

/* ================================ */
PLGG_linetype( s, x, y )
char *s;
double x, y;
{
int style;
style = atoi( s );
return( PLGG_linestyle( style, x, y ) );
}

/* ================================ */
PLGG_linestyle( style, linewidth, dashscale )
int style;
double linewidth, dashscale;
{
int i, j, k, np, state, ds;

style  = style % 9;

ds = (int)(dashscale*2.0);
if( ds < 1 ) { style = 0; ds = 1; }
np = 0;

/* build array p to indicate dash pattern, and set the dash style.. */
state = 1;
for( i = 0; i < Gndash[style]; i++ ) {
	for( j = 0; j < Gdash[style][i]; j++ ) {
		for( k = 0; k < ds; k++ ) {
			if( np >= 1000 ) {
				Eerr( 12005, "img dashscale out of range", "" );
				return( 0 );
				}
			Gdashpat[np++] = state;
			}
		}
	if( state == 1 ) state = 0;
	else state = 1;
	}
gdImageSetStyle( Gm, Gdashpat, np );
Gcurlinestyle = style;
Gcurdashscale = dashscale;

/* set the line width by setting pixels in the brush image.. */
i = (int) linewidth;
if( i > (NBRUSH-1) ) i = NBRUSH - 1;
gdImageSetBrush( Gm, Gbrush[i] );
Gcurlinewidth = linewidth;


return( 0 );
}
/* ================================ */
PLGG_pathto( px, py )
double px, py;
{
if( Gnpts == 0 ) {
	Gptlist[ Gnpts ].x = Exsca( Goldx );
	Gptlist[ Gnpts ].y = Eysca( Goldy );
	Gnpts++;
	}
if( Gnpts >= MAX_D_ROWS ) { 
	Eerr( 12006, "img: too many data points", "" );
	return( 1 );
	}
Gptlist[ Gnpts ].x = Exsca( px );
Gptlist[ Gnpts ].y = Eysca( py );
Gnpts++;
return( 0 );
}
/* ================================ */
PLGG_fill()
{
int i;
if( Gnpts < 3 ) { 
	Eerr( 12007, "warning, not enough points", "" );
	return( 0 ); 
	}
Gptlist[ Gnpts ].x = Gptlist[0].x;
Gptlist[ Gnpts ].y = Gptlist[0].y;
Gnpts++;
gdImageFilledPolygon( Gm, Gptlist, Gnpts, Gcurcolor );
Gnpts = 0;
return( 0 );
}
/* ================================ */
/* note: caller must restore previous color after this routine returns. */
PLGG_rect( x1, y1, x2, y2, color )
double x1, y1, x2, y2;
char *color;
{
int a, b, c, d;
a = Exsca( x1 );
b = Eysca( y1 );
c = Exsca( x2 );
d = Eysca( y2 );
PLGG_color( color );
gdImageFilledRectangle( Gm, a, b, c, d, Gcurcolor );
return( 0 );
}

/* ================================ */
/* set a freetype font */
PLGG_font( s )
char *s;
{
#ifdef GDFREETYPE
  if( s[0] == '/' ) return( 0 ); /* ignore postscript fonts */
  if( stricmp( s, "ascii" )==0 ) strcpy( GFTfont, "" );
  else strcpy( GFTfont, s );
#endif
return( 0 );
}

/* ================================ */
PLGG_textsize( p )
int p;
{

#ifdef GDFREETYPE
if( GFTfont[0] ) {
	GFTsize = (double)p;
	Gtextsize = p;
	Gcharwidth = 0.0; /* no top/bottom adjustment needed with FT */
	return( 0 );
	}
#endif

/* this logic is replicated in Etextsize() */
if( p <= 6 ) { Gfont = gdFontTiny; Gcharwidth = 0.05; }
else if( p >= 7 && p <= 9 ) { Gfont = gdFontSmall; Gcharwidth = 0.06; } /* was 0.0615384 */
else if( p >= 10 && p <= 12 ) { Gfont = gdFontMediumBold; Gcharwidth = 0.070; } /* was 0.0727272 */
else if( p >= 13 && p <= 15 ) { Gfont = gdFontLarge; Gcharwidth = 0.08; }
else if( p >= 15 ) { Gfont = gdFontGiant; Gcharwidth = 0.09; } /* was 0.0930232 */
Gtextsize = p;
return( 0 );
}
/* ================================ */
PLGG_chardir( d )
int d;
{
if( d == 90 ) Gvertchar = 1;
else Gvertchar = 0;
return( 0 );
}
/* ================================ */
PLGG_text( s )
char *s;
{
int a, b, c, d;
double x, y, ptsize;
char *err;

a = Exsca( Goldx ); b = Eysca( Goldy );
if( Gvertchar ) {
	if( Gtextsize < MINTEXTSIZE ) {
		x = Goldx - (Gtextsize/90.0);
		a = Exsca( x ); b = Eysca( Goldy ); 
		c = Exsca( x ); d = Eysca( Goldy + (((double)Gtextsize/100.0)*strlen(s)));
		gdImageLine( Gm, a, b, c, d, gdStyledBrushed );
		}
	else	{
		x = Goldx - (Gcharwidth*CHARHW);  /* adjust for top loc */
		a = Exsca( x ); b = Eysca( Goldy );
#ifdef GDFREETYPE
		if( GFTfont[0] ) {
			err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, VERT, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s)\n", err, GFTfont ); return( 1 ); }
			}
#endif
		if( GFTfont[0] == '\0' ) gdImageStringUp( Gm, Gfont, a, b, s, Gcurcolor );
		}
	}
else 	{
	if( Gtextsize < MINTEXTSIZE ) {
		a = Exsca( Goldx ); b = Eysca( Goldy ); 
		c = Exsca( Goldx + (((double)Gtextsize/100.0) * strlen(s)) ); d = Eysca( Goldy );
		gdImageLine( Gm, a, b, c, d, gdStyledBrushed );
		}
	else	{
		y = Goldy +  (Gcharwidth*CHARHW);  /* adjust for top loc */
		a = Exsca( Goldx ); b = Eysca( y );
#ifdef GDFREETYPE
		if( GFTfont[0] ) {
			err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s)\n", err, GFTfont ); return( 1 ); }
			}
#endif
		if( GFTfont[0] == '\0' ) gdImageString( Gm, Gfont, a, b, s, Gcurcolor );
		}
	}
Goldx = x;
Goldy = y;

return( 0 );
}
/* ================================ */
PLGG_centext( s )
char *s;
{
double halflen, x, y;
int a, b, c, d;
char *err;

halflen = (Gcharwidth * (double)(strlen( s ))) / 2.0;
if( Gvertchar ) {
	if( Gtextsize < MINTEXTSIZE ) {
		halflen = (double)(strlen(s))/2.0 * ((double)Gtextsize/100.0);
		x = Goldx - (Gtextsize/90.0);  /* adjust for top loc */
		a = Exsca( x ); b = Eysca( Goldy - halflen );
		c = Exsca( x ); d = Eysca( Goldy + halflen );
		gdImageLine( Gm, a, b, c, d, gdStyledBrushed );
		}
	else	{
#ifdef GDFREETYPE
		if( GFTfont[0] ) {
			a = Exsca( Goldx ); b = Eysca( Goldy );
			err = gdImageStringFT( NULL, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s) (width calc)\n", err, GFTfont ); return( 1 ); }
			b += (GFTbox[4] - GFTbox[0])/2;
			err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, VERT, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s)\n", err, GFTfont ); return( 1 ); }
			}
#endif
		if( GFTfont[0] == '\0' ) {
			x = Goldx - (Gcharwidth*CHARHW);  /* adjust for top loc */
			y = Goldy - halflen;
			a = Exsca( x ); b = Eysca( y ); 
			gdImageStringUp( Gm, Gfont, a, b, s, Gcurcolor );
			}
		}
	}
else	{
	if( Gtextsize < MINTEXTSIZE ) {
		halflen = (double)(strlen(s))/2.0 * ((double)Gtextsize/100.0);
		a = Exsca( Goldx - halflen ); b = Eysca( Goldy ); 
		c = Exsca( Goldx + halflen ); d = Eysca( Goldy ); 
		gdImageLine( Gm, a, b, c, d, gdStyledBrushed );
		}
	else	{
#ifdef GDFREETYPE
		if( GFTfont[0] ) {
			a = Exsca( Goldx ); b = Eysca( Goldy ); 
			err = gdImageStringFT( NULL, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s) (width calc)\n", err, GFTfont ); return( 1 ); }
			a -= (GFTbox[4] - GFTbox[0])/2;
			err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s)\n", err, GFTfont ); return( 1 ); }
			}
#endif
		if( GFTfont[0] == '\0' ) {    /* ascii font */
			x = (Goldx - halflen) + (Gcharwidth/4.0); /* not sure why the extra is needed..*/
			y = Goldy + (Gcharwidth*CHARHW);  /* adjust for top loc */
			a = Exsca( x ); b = Eysca( y ); 
			gdImageString( Gm, Gfont, a, b, s, Gcurcolor );
			}
		}
	}
Goldx = x;
Goldy = y;
return( 0 );
}
/* ================================ */
PLGG_rightjust( s )
char *s;
{
double len, x, y;
int a, b, c, d;
char *err;


len = Gcharwidth * strlen( s );
if( Gvertchar ) {
	if( Gtextsize < MINTEXTSIZE ) {
		x = Goldx - (Gtextsize/90.0);
		a = Exsca(x); b = Eysca( Goldy - (((double)Gtextsize/100.0)*strlen(s)));
		c = Exsca(x); d = Eysca( Goldy );
		}
	else	{
#ifdef GDFREETYPE
		if( GFTfont[0] ) {
			a = Exsca( Goldx ); b = Eysca( Goldy );
			err = gdImageStringFT( NULL, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s) (width calc)\n", err, GFTfont ); return( 1 ); }
			b += (GFTbox[4] - GFTbox[0]);
			/* err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s ); */
			err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, VERT, a, b, s );  /* fixed 9/17/02 - Artur Zaprzala */
			if( err ) { fprintf( stderr, "%s (%s)\n", err, GFTfont ); return( 1 ); }
			}
#endif
		if( GFTfont[0] == '\0' ) {
			x = Goldx - (Gcharwidth*CHARHW);  /* adjust for top loc */
			y = Goldy - len;
			a = Exsca( x ); b = Eysca( y ); 
			gdImageStringUp( Gm, Gfont, a, b, s, Gcurcolor );
			}
		}
	}
else	{
	if( Gtextsize < MINTEXTSIZE ) {
		a = Exsca( Goldx - ((strlen(s))*(Gtextsize/100.0))); b = Eysca( Goldy ); 
		c = Exsca( Goldx ); d = Eysca( Goldy ); 
		gdImageLine( Gm, a, b, c, d, gdStyledBrushed );
		}
	else	{
#ifdef GDFREETYPE
		if( GFTfont[0] ) {
			a = Exsca( Goldx ); b = Eysca( Goldy );
			err = gdImageStringFT( NULL, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s) (width calc)\n", err, GFTfont ); return( 1 ); }
			a -= (GFTbox[4] - GFTbox[0]);
			err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s)\n", err, GFTfont ); return( 1 ); }
			}
#endif
		if( GFTfont[0] == '\0' ) {
			x = Goldx - len;
			y = Goldy + (Gcharwidth*CHARHW);  /* adjust for top loc */
			a = Exsca( x ); b = Eysca( y ); 
			gdImageString( Gm, Gfont, a, b, s, Gcurcolor );
			}
		}
	}
Goldx = x;
Goldy = y;
return( 0 );
}

/* ================================ */
PLGG_color( color )
char *color;
{
int i, n;
double r, g, b;
int ir, ig, ib, len;
int bc; /* brush color */
double atof();


/* parse graphcore color spec.. */
for( i = 0, len = strlen( color ); i < len; i++ ) {
        if( GL_member( color[i], "(),/:|-" ) ) color[i] = ' ';
        }
 
if( strncmp( color, "rgb", 3 )==0 ) {
        n = sscanf( color, "%*s %lf %lf %lf", &r, &g, &b );
        if( n != 3 ) { Eerr( 12008, "Invalid color", color ); return(1); }
        }
else if( strncmp( color, "gray", 4 )==0 || strncmp( color, "grey", 4 )==0 ) {
        n = sscanf( color, "%*s %lf", &r );
        if( n != 1 ) { Eerr( 12008, "Invalid color", color ); return(1); }
        g = b = r;
        }
else if( strcmp( color, "transparent" )==0 ) {  /* added scg 12/29/99 */
	if( Gtransparent_color < 0 ) {
		int t;
		/* allocate transparent color.. */
		Gtransparent_color = gdImageColorAllocate( Gm, 254, 254, 254 ); /* white fallbk */
		/* gdImageColorTransparent( Gm, Gtransparent_color ); */
		/* also keep brushes in sync.. */
		for( i = 0; i < NBRUSH; i++ ) gdImageColorAllocate( Gbrush[i], 254, 254, 254 );
		}
	if( Gtransparent_color >= 0 ) Gcurcolor = Gtransparent_color;
	/* Don't set brushes, etc, because we will never need to draw 
	   transparent lines.  Brushes remain set to previous color. */
	return( 0 );
	}
else if( GL_goodnum( color, &i ) ) {
        r = atof( color );
        g = b = r;
        }
else PLG_colorname_to_rgb( color, &r, &g, &b );

ir = (int)(r * 255);
ig = (int)(g * 255);
ib = (int)(b * 255);


Gcurcolor = gdImageColorExact( Gm, ir, ig, ib );
if( Gcurcolor < 0 ) {
	Gcurcolor = gdImageColorAllocate( Gm, ir, ig, ib );
	if( Gcurcolor < 0 ) {
		Gcurcolor = gdImageColorClosest( Gm, ir, ig, ib );
		if( Gcurcolor < 0 ) return( 1 ); /* Eerr( 12009, "Error on img color allocation", color ); exit(1); */
		}
	}

if( ir + ig + ib == 0 ) Gblack = Gcurcolor; /* for wbmp */


/* also set all brushes to new color.. */
for( i = 0; i < NBRUSH; i++ ) {
	/* first find color index or allocate new one.. */
	bc = gdImageColorExact( Gbrush[i], ir, ig, ib );
	if( bc < 0 ) {
		bc = gdImageColorAllocate( Gbrush[i], ir, ig, ib );
		if( bc < 0 ) {
			bc = gdImageColorClosest( Gbrush[i], ir, ig, ib );
			if( bc < 0 ) {
				Eerr( 12010, "Error on img brush color alloc", color );
				bc = 0;
				}
			}
		}
	/* next set color of each brush.. */
	gdImageFilledRectangle( Gbrush[i], 0, 0, i+1, i+1, bc ); /* bc should == Gcurcolor */
	}

/* This message occurs all the time with JPEG.. perhaps not needed..
 * if( bc != Gcurcolor ) {
 *	Eerr( 12011, "warning, img brush color does not match current color", "" );
 * 	}
 */

/* now, need to update brush image to new color using current linewidth */
i = (int) Gcurlinewidth;
if( i > (NBRUSH-1) ) i = NBRUSH - 1;
gdImageSetBrush( Gm, Gbrush[i] );

return( 0 );
}

/* ============================== */
PLGG_imload( imgname, xscale, yscale )
char *imgname;
double xscale, yscale;
{
FILE *fp;
if( strcmp( imgname, Gm2filename )==0 ) return( 0 ); /* already loaded */
fp = fopen( imgname, "rb" );
if( fp == NULL ) return( -1 );
if( Gm2filename[0] != '\0' ) gdImageDestroy( Gm2 );
#ifdef GD16
Gm2 = gdImageCreateFromPng( fp );
#endif
#ifdef GD18
if( strcmp( g_fmt, "png" )==0 ) Gm2 = gdImageCreateFromPng( fp );
else if( strcmp( g_fmt, "jpeg" )==0 ) Gm2 = gdImageCreateFromJpeg( fp );
else if( strcmp( g_fmt, "wbmp" )==0 ) Gm2 = gdImageCreateFromWBMP( fp );
#endif
strcpy( Gm2filename, imgname );
Gm2xscale = xscale;
Gm2yscale = yscale;
fclose( fp );
return( 0 );
}

/* ================================ */
/* place secondary GIF image within main image at absolute x, y 
	align may be:	 "center" to center image around x, y
			 "topleft" to put top left corner of image at x, y
			 "bottomleft" to put bottom left corner of image at x, y */
PLGG_implace( x, y, align, xscale, yscale )
double x, y;
char *align;
double xscale, yscale; /* usually specified as 1.0 1.0 but may be used to influence size */
{
int gx, gy;
double PLG_xsca_inv(), PLG_ysca_inv();
int neww, newh;


if( Gm2filename[0] == '\0' ) return( -1 ); /* no 2ndary image loaded */

/* calculate new scaled size */
neww =  (Gm2->sx) * Gm2xscale * xscale;
newh =  (Gm2->sy) * Gm2yscale * yscale;

if( tolower( align[0] ) == 'c' ) { 	/* center it */
	gx = Exsca( x ) - (neww/2);
	if( gx < 0 ) gx = 0;
	gy = Eysca( y ) - (newh/2);
	if( gy < 0 ) gy = 0;
	}
else if( tolower( align[0] ) == 't' ) {
	gx = Exsca( x );
	gy = Eysca( y );
	}
else if( tolower( align[0] ) == 'b' ) {
	gx = Exsca( x );
	gy = Eysca( y ) - newh;
	}

/* gdImageCopy( Gm, Gm2, gx, gy, 0, 0, Gm2->sx, Gm2->sy ); */
gdImageCopyResized( Gm, Gm2, gx, gy, 0, 0, neww, newh, Gm2->sx, Gm2->sy );

/* add to app bounding box */
PLG_bb( Exsca_inv( gx ), Eysca_inv( gy ) );
PLG_bb( Exsca_inv( gx + neww ), Eysca_inv( gy + newh ) );

return( 0 );
}

/* ================================ */
/* EOF - crop image to bounding box size, and create output file */
/* scg 11/23/01 added click map support */

PLGG_eof( filename, x1, y1, x2, y2 )
char *filename;
double x1, y1, x2, y2;
{
int i, width, height, ux, uy;
gdImagePtr outim;
FILE *outfp;
int t;


if( x1 < 0.0 ) x1 = 0.0;
if( y1 < 0.0 ) y1 = 0.0;
if( x2 < 0.0 ) x2 = 0.0;
if( y2 < 0.0 ) y2 = 0.0;

if( PL_clickmap_getdemomode() ) PL_clickmap_show( 'g' ); /* 11/23/01 */

width = Exsca( x2 ) - Exsca( x1 );
height = Eysca( y1 ) - Eysca( y2 );
if( height < 10 || width < 10 ) return( Eerr( 12012, "Result image is too small - not created", "" ) );

ux = Exsca( x1 );
uy = Eysca( y2 );


/* copy to smaller img sized by bounding box.. */
outim = gdImageCreate( width, height );
if( outim == NULL ) return( Eerr( 12013, "Error on creation of image output", "" ) );
gdImageCopy( outim, Gm, 0, 0, ux, uy, width, height );
/* fprintf( stderr, "new im w:%d h:%d   ux:%d uy:%d\n", width, height, ux, uy ); */

/* if a transparent color was used, set it now in outim.. */
if( Gtransparent_color >= 0 ) {
	t = gdImageColorExact( outim, 254, 254, 254 );
	gdImageColorTransparent( outim, t );
	}

/* Open a file for writing. "wb" means "write binary", important
   under MSDOS, harmless under Unix. */
if( stricmp( filename, "stdout" )==0 ) outfp = stdout;
else outfp = fopen( filename, "wb");
if( outfp == NULL ) return( Eerr( 12014, "Cannot open for write", filename ) );

/* Output the image to the disk file. */
#ifdef GD13
gdImageGif( outim, outfp );    
#endif
#ifdef GD16
gdImagePng( outim, outfp );    
#endif
#ifdef GD18
if( strcmp( g_fmt, "png" )==0 ) gdImagePng( outim, outfp );
else if( strcmp( g_fmt, "jpeg" )==0 ) gdImageJpeg( outim, outfp, 75 );
else if( strcmp( g_fmt, "wbmp" )==0 ) gdImageWBMP( outim, Gblack, outfp );
#endif

if( stricmp( filename, "stdout" )!=0 ) {
	fclose( outfp );
#ifndef WIN32
	chmod( filename, 00644 );
#endif
	}

/* free memory (other ims freed in EGSetup() for subsequent pages) */
gdImageDestroy( Gm );
for( i = 0; i < NBRUSH; i++ ) gdImageDestroy( Gbrush[i] );
gdImageDestroy( outim );
Ginitialized = 0;

/* write map file */
if( PL_clickmap_inprogress() ) PL_clickmap_out( ux, uy ); /* 11/23/01 */

return( 0 );
}




#endif /* NOGD */
