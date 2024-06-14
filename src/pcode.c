/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PCODE - all text/graphics requests (except init) are processed
		via this function.  Many E function calls such
		as Emov and Elin are actually macros defined in
		elib.d, which call pcode with a single character
		op code and perhaps some parameters.

   ==========================================================================
   Device codes (Edev):
	p		postscript
	x		X11

   Op codes:
	L		lineto
	M		moveto
	P		path to
	T, C, J		text, centered text, right justified text
	r		set current color 
	s		Fill (color)
	c		closepath (ps only)
	O		paper (ps only)
	I		text size
	F		text font 
	D		text direction (ps only)
	Y		line properties
	Z		print/eject (ps only)
	W		wait for a key or mouse click (x11 only)
	w		cycle notifier from within a loop (x11 only)
	.		dot
	z		clear screen (x11 only)
	d		make window disappear (x11 only)
	a		make window re-appear (x11 only)
	e		set text scale factor (x11 only, when user resizes window)
	Q		end of file (tells ps and gif drivers to wrap up)

	b		save window to backing store (x11 only)
	B		restore window from backing store (x11 only)


   ==========================================================================

*/

#include "graphcore.h"
#ifdef PLOTICUS
  extern int Debug;
  extern FILE *Diagfp;
#endif

static int Enew = 0, Edrawing = 0;		/* used by postscript section */
static int Evertchar = 0; 			/* true if character direction is vertical..*/
						/* ..used only by x11 */
static double Ecurx;				/* real starting place of centered &rt justified text */
static double Ewidth;				/* width of most recently drawn text line. */
static char Eprevop;				/* previous op */
static int Esquelched = 0;			/* is output being squelched? 1=yes, 0=no */
static int Epspage = 1;			/* current page # */
static int Epsvirginpage = 1;		/* 1 when nothing has yet been drawn on current page */

static int Ekeeping_bb = 1;
					
static double EBBx1 = 999;		/* coords of bounding box for entire run */
static double EBBy1 = 999;
static double EBBx2 = -999;
static double EBBy2 = -999;

static double Esbb_x1 = 999;		/* coords of "sub" bounding box available to app */
static double Esbb_y1 = 999;
static double Esbb_x2 = -999;
static double Esbb_y2 = -999;
static int keep_sub_bb = 0;
static int tightbb = 0;
static double scx1, scy1, scx2, scy2;   /* specified crop zone */
static int specifycrop = 0; /* 0 = no  1 = absolute values   2 = values relative to bounding box */

static double globalscale = 1.0;
static double globalscaley = 1.0;
static double posterxo = 0.0;
static double posteryo = 0.0;
static int postermode = 0;
static double lmlx = 0.0, lmly = 0.0;  /* last move local x and y  
	(saves location of last 'MOVE', including any effects of globalscale and poster ofs */
static int pcodedebug = 0;
static FILE *pcodedebugfp = 0;


/* ======================= */
Epcode( op, x, y, s )
char op; /* op code */
double x, y;  /* coordinates */
char s[];     /* optional character string */
{
char buf[512];

/* Postscript: inform driver we're beginning a new page.. */
if( Edev == 'p' && Epsvirginpage && !GL_member( op, "Q" ) ) {
	EPSnewpage( Epspage );
	Epsvirginpage = 0;
	}


/* For clear op: do it then return; don't include in bounding box calculations */
if( op == 'z' ) {
	Ekeeping_bb = 0;
	/* compensate EWinw and EWiny by globalscale because this op will
		be doubly affected by globalscale.. */
	Ecblock( 0.0, 0.0, EWinx/globalscale, EWiny/globalscaley, s, 0 );  
	Ekeeping_bb = 1;
	return( 0 );
	}



if( op == 'M' ) { Ex1 = x; Ey1 = y; } /* remember most recent 'move', untainted 
					by globalscale or posterofs.. */

/* scale x, y according to global scale, for all move, draw and text size ops */
if( globalscale != 1.0 ) {
	if( GL_member( op, "LMPI" )) { x *= globalscale; y *= globalscaley; }
	if( op == 'Y' ) y *= globalscale; /* dash scale */
	}

/* if poster offset specified, translate */
if( postermode ) {
	if( GL_member( Edev, "pcx" ) && GL_member( op, "LMP" )) { 
		x += posterxo; 
		y += posteryo; 
		}
	}

if( Debug && op == 'Q' ) 
	fprintf( Diagfp, "Done with page.  Writing out result file.  Computed bounding box is: %-5.2f, %-5.2f to %-5.2f, %-5.2f\n", EBBx1, EBBy1, EBBx2, EBBy2 );
	

/* device interfaces ======================== */

/* if output is squelched, do not do any draw operations, except textsize, which
   returns text width, which is necessary for bb calculations.. */
if( Esquelched ) {
	if( op == 'I' ) {
#ifndef NOX11
		if( Edev == 'x' ) { EXWpointsize( (int)(x), &Ecurtextwidth ); }
#endif
		}
	}


/* interface to X11 xlib driver.. */
else if( Edev == 'x' ) {
#ifndef NOX11
	switch (op) {
		case 'L' : EXWlineto( x, y ); break;
		case 'M' : EXWmoveto( x, y ); break;
		case 'P' : EXWpath( x, y ); break;
		case 'T' : if( !Evertchar ) EXWtext( s, &Ewidth ); 
			   break;
		case 'r' : EXWcolor( s ); return( 0 );
		case 's' : EXWfill( ); return( 0 );
		case 'U' : EXWflush(); return( 0 );
		case 'b' : EXWsavewin(); return( 0 );
		case 'B' : EXWrestorewin(); return( 0 );
		case 'I' : EXWpointsize( (int)(x), &Ecurtextwidth ); return( 0 );
		case 'C' : if( !Evertchar ) { 
				EXWmoveto( lmlx-6.0, lmly ); 
				EXWcentext( s, 12.0, &Ecurx, &Ewidth ); 
				}
			   break;
		case 'J' : if( !Evertchar ) {
				EXWmoveto( lmlx-12.0, lmly ); 
				EXWrightjust( s, 12.0, &Ecurx, &Ewidth );
				}
			   break;
		case 'W' : EXWwait(); return( 0 );
		case 'Y' : EXWlinetype( s, x, y ); return( 0 );
		case 'D' : if( x == 90 || x == 270 ) Evertchar = 1;
			   else Evertchar = 0;
			   return( 0 );
			
		case 'w' : EXWasync(); return( 0 );
		case '.' : EXWdot( x, y ); break;
		case 'd' : EXWdisappear(); return( 0 );
		case 'a' : EXWappear(); return( 0 );
		case 'e' : EXWscaletext( x ); return( 0 );
		}
#endif
	}
else

/* interface to postscript driver */
if( Edev == 'p'  ) {

	if( op != 'L' ) { 
		if( Edrawing ) EPSstroke(); 
		Edrawing = 0;
		}

	switch( op ) {
		case 'L' : if( Enew ) EPSmoveto( lmlx, lmly ); 
			    EPSlineto( x, y );
			    Enew = 0;
			    Edrawing = 1;
			    break;
		case 'M' : Enew = 1; break;
		case 'P' : if( Enew ) EPSmoveto( lmlx, lmly ); 
			   EPSpath( x, y ); 
			   Enew = 0;
			   break;
		case 'T' : EPStext( op, lmlx, lmly, s, 0.0 ); break;
	
		case 'C' : if( !Evertchar ) EPStext( op, lmlx - 6.0, lmly, s, 12.0 );
			   else if( Evertchar )EPStext( op, lmlx, lmly - 6.0, s, 12.0 );
			   break;
		case 'J' : if( !Evertchar ) EPStext( op, lmlx-12.0, lmly, s, 12.0 );
			   else if( Evertchar ) EPStext( op, lmlx, lmly - 12.0, s, 12.0 );
			   break;
	
		case 's' : EPSfill( ); return( 0 );
		case 'r' : EPScolor( s ); return( 0 );
		case 'c' : EPSclosepath(); return( 0 );
		case 'O' : EPSpaper( (int)x ); return( 0 );
		case 'I' : EPSpointsize( (int)x ); return( 0 );
		case 'F' : EPSfont( s ); return( 0 );
		case 'D' : EPSchardir( (int)x );
			   if( x == 90 || x == 270 ) Evertchar = 1;
			   else Evertchar = 0;
			   return( 0 );
		case 'Y' : EPSlinetype( s, x, y ); return( 0 );
		case 'Z' : EPSshow(); Epspage++; Epsvirginpage = 1; return( 0 );
		case 'Q' : if( !Epsvirginpage ) { EPSshow(); Epspage++; }
			   if( tightbb ) 
			     EPStrailer( Epspage - 1, EBBx1-0.05, EBBy1-0.05, EBBx2+0.05, EBBy2+0.05 );
			   else if( specifycrop == 1 )
			     EPStrailer( Epspage - 1, scx1, scy1, scx2, scy2 );
			   else if( specifycrop == 2 ) {
			     EPStrailer( Epspage - 1, (EBBx1-0.05)-scx1, (EBBy1-0.05)-scy1, 
							(EBBx2+0.05)+scx2, (EBBy2+0.05)+scy2 );
				}
			   else 
			     /* add 0.2" margin to be generous in cases of fat lines, etc. */
			     EPStrailer( Epspage - 1, EBBx1-0.2, EBBy1-0.2, EBBx2+0.2, EBBy2+0.2 );
			   Eresetbb();
			   return( 0 );
		/* case '.' : EPSdot( x, y ); ??? */
		}
	}

/* Added for svg support - BT 05/11/01 */
/* interface to svg driver */
else if( Edev == 's'  ) {

	if( op != 'L' ) { 
		if( Edrawing ) SVGstroke(); 
		Edrawing = 0;
		}

	switch( op ) {
		case 'L' : if( Enew ) SVGmoveto( lmlx, lmly ); 
			    SVGlineto( x, y );
			    Enew = 0;
			    Edrawing = 1;
			    break;
		case 'M' : Enew = 1; break;
		case 'P' : if( Enew ) SVGmoveto( lmlx, lmly ); 
			   SVGpath( x, y ); 
			   Enew = 0;
			   break;
	
		case 'T' : SVGtext( op, lmlx, lmly, s, 0.0 ); break;
	
		case 'C' : if( !Evertchar ) SVGtext( op, lmlx , lmly, s, 0.0 );
			   else if( Evertchar )SVGtext( op, lmlx, lmly , s, 0.0 );
			   break;
		case 'J' : if( !Evertchar ) SVGtext( op, lmlx, lmly, s, 0.0 );
			   else if( Evertchar ) SVGtext( op, lmlx, lmly , s, 0.0 );
			   break;

		case 's' : SVGfill( ); return( 0 );
		case 'r' : SVGcolor( s ); return( 0 );
		case 'c' : SVGclosepath(); return( 0 );
		case 'O' : SVGpaper( (int)x ); return( 0 );
		case 'I' : SVGpointsize( (int)x ); return( 0 );
		case 'F' : SVGfont( s ); return( 0 );
		case 'D' : SVGchardir( (int)x );
			   if( x == 90 || x == 270 ) Evertchar = 1;
			   else Evertchar = 0;
			   return( 0 );
		case 'Y' : SVGlinetype( s, x, y ); return( 0 );
		case 'Z' : SVGshow(); Epspage++; Epsvirginpage = 1; return( 0 );
		case 'Q' : if( !Epsvirginpage ) { SVGshow(); Epspage++; }
			   if( tightbb ) 
			     SVGtrailer( Epspage - 1, EBBx1-0.05, EBBy1-0.05, EBBx2+0.05, EBBy2+0.05 );
			   else if( specifycrop == 1 )
			     SVGtrailer( Epspage - 1, scx1, scy1, scx2, scy2 );
			   else if( specifycrop == 2 ) {
			     SVGtrailer( Epspage - 1, (EBBx1-0.05)-scx1, (EBBy1-0.05)-scy1, 
							(EBBx2+0.05)+scx2, (EBBy2+0.05)+scy2 );
				}
			   else 
			     /* add 0.2" margin to be generous in cases of fat lines, etc. */
			     SVGtrailer( Epspage - 1, EBBx1-0.2, EBBy1-0.2, EBBx2+0.2, EBBy2+0.2 );
			   Eresetbb();
			   return( 0 );
		}
	}

/* interface to GD driver.. */
else if( Edev == 'g' ) {
#ifndef NOGD
	switch (op) {
		case 'L' : EGlineto( x, y ); break;
		case 'M' : EGmoveto( x, y ); break;
		case 'P' : EGpathto( x, y ); break;
		case 'T' : EGtext( s ); break; 
		case 'C' : EGcentext( s ); break; 
		case 'F' : EGfont( s ); break;
		case 'J' : EGrightjust( s ); break; 
		case 's' : EGfill(); return( 0 );
		case 'r' : EGcolor( s ); return( 0 );
		case 'I' : EGtextsize( (int)x ); return( 0 );
		case 'D' : EGchardir( (int)x ); 
			   if( (int)x != 0 ) Evertchar = 1;
			   else Evertchar = 0;
			   return( 0 );
		case 'Y' : EGlinetype( s, x, y ); return( 0 );
		case 'Q' : Egetoutfilename( buf );
			   if( buf[0] == '\0' ) strcpy( buf, "out.gif" );

			   /* see if anything has been drawn, if not, return */
			   if ( EBBx2 < -998 && EBBy2 < -998 ) return( 0 );

			   if( tightbb ) EGeof( buf, EBBx1,  EBBy1, EBBx2, EBBy2 ); 
			   else if( specifycrop == 1 )
			     EGeof( buf, scx1, scy1, scx2, scy2 );
			   else if( specifycrop == 2 )
			     EGeof( buf, (EBBx1)-scx1, (EBBy1)-scy1, (EBBx2)+scx2, (EBBy2)+scy2 );
			   else EGeof( buf, EBBx1-0.2,  EBBy1-0.2, EBBx2+0.2, EBBy2+0.2 ); 
			   Eresetbb();
			   return( 0 );
		}
#endif
	}



else 	{ 
	if( Edev == '\0' ) {
		fprintf( stderr, "[pcode got %c]", op );
		Eerr( 12021, "Graphcore has not yet been initialized", "" );
		}

	else 	{
		char sdev[8];
		sprintf( sdev, "%c", Edev );
		Eerr( 12022, "Unrecognized graphic device code", sdev );
		}
	exit(1);
	}




if( op == 'M' ) { lmlx = x; lmly = y; } /* remember most recent 'move' */



/* figure approximate text dimensions */
if( Edev != 'x' && GL_member( op, "TCJ" )) {
	Ewidth = strlen( s ) * Ecurtextwidth;
	Ewidth *= globalscale;
	}

if( Ekeeping_bb ) {
	/* keep bounding box info (minima and maxima) */
	if( GL_member( op, "LP" ) ) {
		if( Eprevop == 'M' ) Ebb( lmlx, lmly );
		Ebb( x, y );
		}
	/* normal (horizontal) text operations.  (vertical text below) */
	else if( op == 'T' && !Evertchar ) {
		if( Eprevop == 'M' ) Ebb( lmlx, lmly );
		Ebb( lmlx + Ewidth+0.05, lmly + (Ecurtextheight*globalscale) );
		}
	else if( op == 'C' && !Evertchar ) { 
		Ebb( lmlx - ((Ewidth/2.0)+0.05), lmly );
		Ebb( lmlx + ((Ewidth/2.0)+0.05), lmly + (Ecurtextheight*globalscale) );
		}
	else if( op == 'J' && !Evertchar ) { 
		Ebb( lmlx - (Ewidth+0.05), lmly );
		Ebb( lmlx, lmly + (Ecurtextheight*globalscale) );
		}
	}

Eprevop = op;


/* handle vertical text .. must be simulated for x windows;
   also gets bounding box for vertical text operations (all devices) */

if( Evertchar && GL_member( op, "TCJ" )) Everttextsim( op, s );

}

#ifdef NOX11
Egetclick()
{
double x, y;
int e;
if( Edev == 'p' ) {
	Eshow(); /* eject page and return.. */
	return(0); 
	}
else if( Edev == 'g' ) {
	Eendoffile();
	return( 0 );
	}
}  
#endif


/* ============================================= */
/* EBB - keep an overall bounding box for the entire image.
	 Also call Echeckbb() to maintain nested object bounding boxes.. */
/* Ebb( double x, double y ) */
Ebb( x, y )
double x, y;
{
if( Ekeeping_bb ) {
	if( pcodedebug ) {
		if( ( x < EBBx1 && x < 0.0 ) || (x > EBBx2 && x > 8.0 ) ) fprintf( pcodedebugfp, "draw out X = %g\n", x );
		if( ( y < EBBy1 && y < 0.0 ) || (y > EBBy2 && y > 8.0 ) ) fprintf( pcodedebugfp, "draw out Y = %g\n", y );
		}
	if( x < EBBx1 ) EBBx1 = x;  
	if( x > EBBx2 ) EBBx2 = x; 
	if( y < EBBy1 ) EBBy1 = y; 
	if( y > EBBy2 ) EBBy2 = y; 
	
	}
if( keep_sub_bb ) {
	if( x < Esbb_x1 ) Esbb_x1 = x;
	if( x > Esbb_x2 ) Esbb_x2 = x;
	if( y < Esbb_y1 ) Esbb_y1 = y;
	if( y > Esbb_y2 ) Esbb_y2 = y;
	}


return( 0 );
}

/* ============================================== */
/* ERESETBB - needed for multiple pages */
Eresetbb()
{
EBBx1 = 999;
EBBy1 = 999;
EBBx2 = -999;
EBBy2 = -999;
return( 0 );
}

/* ============================================= */
/* EGETBB - get current bounding box.. */
Egetbb( xlo, ylo, xhi, yhi )
double *xlo, *ylo, *xhi, *yhi;
{
*xlo = EBBx1 / globalscale;
*ylo = EBBy1 / globalscaley;
*xhi = EBBx2 / globalscale;
*yhi = EBBy2 / globalscaley;
return( 0 );
}


/* ============================================== */
/* EGETTEXTSIZE - get width and height of last text item.. */

Egettextsize( w, h )
  double *w, *h;
{
*w = Ewidth;
*h = Ecurtextheight;
}

/* ================================================ */
/* vertical text bounding box, also simulation for X11 displays */
Everttextsim( op, s )
char op, s[];
{
double dist, y1, y2, x, y;
char let[4];
int i, len;
double w;

len = strlen( s );

if( Edev == 'x' ) dist = len * Ecurtextheight;
else dist = len * Ecurtextwidth;

if( op == 'T' ) { y1 = lmly; y2 = lmly + dist; }
else if( op == 'C' ) { y1 = lmly - (dist/2); y2 = lmly + (dist/2); }
else if( op == 'J' ) { y1 = lmly - dist; y2 = lmly; }
if( Edev == 'x' ) x = lmlx - Ecurtextwidth;
else x = lmlx;
y = y2;
#ifndef NOX11
if( Edev == 'x' ) {
	for( i = 0; i < len; i++ ) {
		sprintf( let, "%c", s[i] );
		EXWmoveto( x, y ); 
		EXWtext( let, &w ); 
		y -= Ecurtextheight;
		}
	}
#endif
Ebb( x-(Ecurtextheight*globalscale), y1 );
Ebb( x-(Ecurtextheight*globalscale), y2 );
return( 0 );
}



/* ==================================================== */
/* 1 = squelch all display activity, 0 restore to normal */
/* Used to calculate bounding box without displaying */
/* handles nested calls. */
Esquelch_display( mode )
int mode;
{
static int snest = 0;
if( mode == 1 ) {
	snest++;
	Esquelched = 1;
	}
else if( mode == 0 ) { 
	if( snest > 0 ) snest--;
	if( snest == 0 )Esquelched = 0;
	}
}

/* ==================================================== */
Ekeep_sub_bb( mode )
int mode;
{
keep_sub_bb = mode;

if( mode == 0 ) {
 	Esbb_x1 = 999.0;
 	Esbb_y1 = 999.0;
 	Esbb_x2 = -999.0;
 	Esbb_y2 = -999.0;
	}

return( 0 );
}
/* ==================================================== */
/* ETIGHTBB - switch ON=don't add margin when doing final BB crop */
Etightbb( mode )
int mode;
{
tightbb = mode;
return( 0 );
}
/* ==================================================== */
/* ESPECIFYCROP -  */
Especifycrop( mode, x1, y1, x2, y2 )
int mode; /* 0=off   1=absolute values   2=relative to tightcrop values  */
double x1, y1, x2, y2;
{
specifycrop = mode;
if( specifycrop ) {
	scx1 = x1;
	scy1 = y1;
	scx2 = x2;
	scy2 = y2;
	}
return( 0 );
}

/* ==================================================== */
/* special GD calls... */

/* ==================================================== */
/* EGIFRECT - direct interface to GD driver for better efficiency on rectangles */
Egifrect( xlo, yhi, xhi, ylo, color )
double xlo, yhi, xhi, ylo;
char *color;
{
#ifndef NOGD
char oldcolor[40];
strcpy( oldcolor, Ecurcolor );
if( globalscale != 1.0 ) { 
	xlo *= globalscale; ylo *= globalscaley; 
	xhi *= globalscale; yhi *= globalscaley;
	}
EGrect( xlo, yhi, xhi, ylo, color );
Ebb( xlo, ylo );
Ebb( xhi, yhi );
Ecolor( oldcolor );
#endif
return( 0 );
}
/* ==================================================== */
/* EIMLOAD - tell the gif driver to load a GIF image */
Eimload( filename, scalex, scaley )
char *filename;
double scalex, scaley;
{
#ifndef NOGD
if( globalscale != 1.0 ) {
	scalex *= globalscale;
	scaley *= globalscaley;
	}
return( EGimload( filename, scalex, scaley ) );
#else
return( 1 );
#endif
}

/* ==================================================== */
/* EIMPLACE - tell the gif driver to place a GIF image */
Eimplace( x, y, imalign, xscale, yscale )
double x, y;
char *imalign;
double xscale, yscale;
{
#ifndef NOGD
if( globalscale != 1.0 ) {
	x *= globalscale;
	y *= globalscaley;
	/* xscale and yscale are always passed as 1.0; 
		do not scale here as image is scaled when read */
	}
return( EGimplace( x, y, imalign, xscale, yscale ) );
#else
return( 1 );
#endif
}


/* ===================================================== */
/* ESETGLOBALSCALE - set global scale factor */
Esetglobalscale( sx, sy )
double sx, sy;
{
if( sx < 0.01 || sx > 20.0 ) return( Eerr( 20815, "Invalid global scaling", "" ) );
if( sy < 0.01 || sy > 20.0 ) return( Eerr( 20815, "Invalid global scaling", "" ) );
globalscale = sx;
globalscaley = sy;
Estandard_lwscale = 1.0 * sx;
return( 0 );
}
/* ===================================================== */
/* EGETGLOBALSCALE - get global scale factor */
Egetglobalscale( sx, sy )
double *sx, *sy;
{
*sx = globalscale;
*sy = globalscaley;
return( 0 );
}

/* ======================================= */
/* ESETPOSTEROFS - set poster offset (paginated postscript only).
   x, y are in absolute units, and are where the lower-left of the page will be.
   So if I have a poster made of 4 8.5x11 sheets held portrait style,
	the lowerleft would use 0,0
	the lowerright would use 8,0
	the upperleft would use 0,10.5
	the lowerright would use 8,10.5
   (8 and 10.5 are used because of print margins).
   The four pages can then be trimmed w/ a paper cutter and butted up against 
   one another to create a poster.
*/
Esetposterofs( x, y )
double x, y;
{
postermode = 1;
posterxo = x * (-1.0);
posteryo = y * (-1.0);
}

/* ========================================== */
/* EPCODEDEBUG - turn on/off local debugging */
Epcodedebug( mode, fp )
int mode;
FILE *fp;  /* stream for diagnostic output */
{
pcodedebug = mode;
pcodedebugfp = fp;
return( 0 );
}
