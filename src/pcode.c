/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PCODE - all text/graphics requests (except init) are processed
		via this function.  Many E function calls such
		as Emov and Elin are actually macros defined in
		plg.h, which call pcode with a single character
		op code and perhaps some parameters.

   ==========================================================================
   Device codes (Edev):
	p		postscript
	x		X11
	g		GD (png, gif, jpeg, etc.)
	s		SVG
 	f		swf (flash format)

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
	Q		end of file (tells drivers to finish up)

	b		save window to backing store (x11 only)
	B		restore window from backing store (x11 only)

	<		begin object
	>		end object


   ==========================================================================

*/

#include "plg.h"

static int ps_new = 0, ps_drawing = 0;		/* used by postscript section */
static int vertchar = 0; 			/* true if character direction is vertical..*/
						/* ..used only by x11 */
static double txt_curx;				/* real starting place of centered &rt justified text */
static double txt_width;				/* width of most recently drawn text line. */
static char prev_op;				/* previous op */
static int squelched = 0;			/* is output being squelched? 1=yes, 0=no */
static int pagenum = 1;				/* current page # */
static int virginpage = 1;		/* 1 when nothing has yet been drawn on current page */

static int keeping_bb = 1;
					
static double bb_x1 = 999;		/* coords of bounding box for entire run */
static double bb_y1 = 999;
static double bb_x2 = -999;
static double bb_y2 = -999;

static double bb2_x1 = 999;		/* coords of "sub" bounding box available to app */
static double bb2_y1 = 999;
static double bb2_x2 = -999;
static double bb2_y2 = -999;
static int keep_bb2 = 0;
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
static FILE *pcodedebugfp;
static int in_obj = 0;

static int verttextsim();

/* ======================= */
PLG_pcode_initstatic()
{
ps_new = 0; ps_drawing = 0;
vertchar = 0;
squelched = 0;
pagenum = 1;
virginpage = 1;
keeping_bb = 1;
bb_x1 = 999; bb_y1 = 999; bb_x2 = -999; bb_y2 = -999;
bb2_x1 = 999; bb2_y1 = 999; bb2_x2 = -999; bb2_y2 = -999;
keep_bb2 = 0;
tightbb = 0;
specifycrop = 0;
globalscale = 1.0;
globalscaley = 1.0;
posterxo = posteryo = 0.0;
postermode = 0;
lmlx = lmly = 0.0;
pcodedebug = 0;
in_obj = 0;

return( 0 );
}


/* ======================= */
PLG_pcode( op, x, y, s )
char op; /* op code */
double x, y;  /* coordinates */
char s[];     /* optional character string */
{
char buf[512];
int stat;

/* postscript: inform driver we're beginning a new page.. */
if( Edev == 'p' && virginpage && !GL_member( op, "Q" ) ) {
#ifndef NOPS
	PLGP_newpage( pagenum );
#endif
	virginpage = 0;
	}


/* For clear op: do it then return; don't include in bounding box calculations */
if( op == 'z' ) {
	keeping_bb = 0;
	/* compensate EWinw and EWiny by globalscale because this op will
		be doubly affected by globalscale.. */
	Ecblock( 0.0, 0.0, EWinx/globalscale, EWiny/globalscaley, s, 0 );  
	keeping_bb = 1;
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

if( pcodedebug && op == 'Q' ) 
	fprintf( pcodedebugfp, "Done with page.  Writing out result file.  Computed bounding box is: %-5.2f, %-5.2f to %-5.2f, %-5.2f\n", bb_x1, bb_y1, bb_x2, bb_y2 );
	

/* reject endobj's without companion beginobj */
if( op == '<' ) in_obj = 1;
else if( op == '>' ) {
	if( ! in_obj ) return( 0 );
	else in_obj = 0;
	}

/* device interfaces ======================== */

/* if output is squelched, do not do any draw operations, except textsize, which
   returns text width, which is necessary for bb calculations.. */
if( squelched ) {
	if( op == 'I' ) {
#ifndef NOX11
		if( Edev == 'x' ) { PLGX_pointsize( (int)(x), &Ecurtextwidth ); }
#endif
		}
	}


/*********** interface to X11 xlib driver.. */
else if( Edev == 'x' ) {
#ifndef NOX11
	switch (op) {
		case 'L' : PLGX_lineto( x, y ); break;
		case 'M' : PLGX_moveto( x, y ); break;
		case 'P' : PLGX_path( x, y ); break;
		case 'T' : if( !vertchar ) PLGX_text( s, &txt_width ); 
			   break;
		case 'r' : PLGX_color( s ); return( 0 );
		case 's' : PLGX_fill( ); return( 0 );
		case 'U' : PLGX_flush(); return( 0 );
		case 'b' : PLGX_savewin(); return( 0 );
		case 'B' : PLGX_restorewin(); return( 0 );
		case 'I' : PLGX_pointsize( (int)(x), &Ecurtextwidth ); return( 0 );
		case 'C' : if( !vertchar ) { 
				PLGX_moveto( lmlx-6.0, lmly ); 
				PLGX_centext( s, 12.0, &txt_curx, &txt_width ); 
				}
			   break;
		case 'J' : if( !vertchar ) {
				PLGX_moveto( lmlx-12.0, lmly ); 
				PLGX_rightjust( s, 12.0, &txt_curx, &txt_width );
				}
			   break;
		case 'W' : PLGX_wait(); return( 0 );
		case 'Y' : PLGX_linetype( s, x, y ); return( 0 );
		case 'D' : if( x == 90 || x == 270 ) vertchar = 1;
			   else vertchar = 0;
			   return( 0 );
			
		case 'w' : PLGX_async(); return( 0 );
		case '.' : PLGX_dot( x, y ); break;
		case 'd' : PLGX_disappear(); return( 0 );
		case 'a' : PLGX_appear(); return( 0 );
		case 'e' : PLGX_scaletext( x ); return( 0 );
		}
#endif
	}


/*************** interface to GD driver.. */
else if( Edev == 'g' ) {
#ifndef NOGD
	switch (op) {
		case 'L' : PLGG_lineto( x, y ); break;
		case 'M' : PLGG_moveto( x, y ); break;
		case 'P' : PLGG_pathto( x, y ); break;
		case 'T' : PLGG_text( s ); break; 
		case 'C' : PLGG_centext( s ); break; 
		case 'F' : PLGG_font( s ); break;
		case 'J' : PLGG_rightjust( s ); break; 
		case 's' : PLGG_fill(); return( 0 );
		case 'r' : PLGG_color( s ); return( 0 );
		case 'I' : PLGG_textsize( (int)x ); return( 0 );
		case 'D' : PLGG_chardir( (int)x ); 
			   if( (int)x != 0 ) vertchar = 1;
			   else vertchar = 0;
			   return( 0 );
		case 'Y' : PLGG_linetype( s, x, y ); return( 0 );
		case 'Q' : PLG_getoutfilename( buf );
			   if( buf[0] == '\0' ) strcpy( buf, "unnamed_result_image" ); /* fallback */

			   /* see if anything has been drawn, if not, return */
			   if ( bb_x2 < -998 && bb_y2 < -998 ) return( 0 );

			   if( tightbb ) stat = PLGG_eof( buf, bb_x1,  bb_y1, bb_x2, bb_y2 ); 
			   else if( specifycrop == 1 )
			     stat = PLGG_eof( buf, scx1, scy1, scx2, scy2 );
			   else if( specifycrop == 2 )
			     stat = PLGG_eof( buf, (bb_x1)-scx1, (bb_y1)-scy1, (bb_x2)+scx2, (bb_y2)+scy2 );
			   else stat = PLGG_eof( buf, bb_x1-0.2,  bb_y1-0.2, bb_x2+0.2, bb_y2+0.2 ); 
			   Eresetbb();
			   return( stat );
		}
#endif
	}


/*************** interface to svg driver */
else if( Edev == 's'  ) {
#ifndef NOSVG
	if( op != 'L' ) { 
		if( ps_drawing ) PLGS_stroke(); 
		ps_drawing = 0;
		}

	switch( op ) {
		case 'L' : if( ps_new ) PLGS_moveto( lmlx, lmly ); 
			    PLGS_lineto( x, y );
			    ps_new = 0;
			    ps_drawing = 1;
			    break;
		case 'M' : ps_new = 1; break;
		case 'P' : if( ps_new ) PLGS_moveto( lmlx, lmly ); 
			   PLGS_path( x, y ); 
			   ps_new = 0;
			   break;
	
		case 'T' : PLGS_text( op, lmlx, lmly, s, 0.0 ); break;
	
		case 'C' : if( !vertchar ) PLGS_text( op, lmlx , lmly, s, 0.0 );
			   else if( vertchar ) PLGS_text( op, lmlx, lmly , s, 0.0 );
			   break;
		case 'J' : if( !vertchar ) PLGS_text( op, lmlx, lmly, s, 0.0 );
			   else if( vertchar ) PLGS_text( op, lmlx, lmly , s, 0.0 );
			   break;

		case 's' : PLGS_fill( ); return( 0 );
		case 'r' : PLGS_color( s ); return( 0 );
		case 'I' : PLGS_pointsize( (int)x ); return( 0 );
		case 'F' : PLGS_font( s ); return( 0 );
		case 'D' : PLGS_chardir( (int)x );
			   if( x == 90 || x == 270 ) vertchar = 1;
			   else vertchar = 0;
			   return( 0 );
		case 'Y' : PLGS_linetype( s, x, y ); return( 0 );
		case '<' : PLGS_objbegin( s ); return( 0 );
		case '>' : PLGS_objend(); return( 0 );
		case 'Z' : pagenum++; virginpage = 1; return( 0 );
		case 'Q' : if( !virginpage ) pagenum++; 
			   if( tightbb ) 
			     stat = PLGS_trailer( bb_x1-0.05, bb_y1-0.05, bb_x2+0.05, bb_y2+0.05 );
			   else if( specifycrop == 1 )
			     stat = PLGS_trailer( scx1, scy1, scx2, scy2 );
			   else if( specifycrop == 2 ) {
			     stat = PLGS_trailer( (bb_x1-0.05)-scx1, (bb_y1-0.05)-scy1, (bb_x2+0.05)+scx2, (bb_y2+0.05)+scy2 );
				}
			   else 
			     /* add 0.2" margin to be generous in cases of fat lines, etc. */
			     stat = PLGS_trailer( bb_x1-0.2, bb_y1-0.2, bb_x2+0.2, bb_y2+0.2 );
			   Eresetbb();
			   return( stat );
		}
#endif
	}

/*************** interface to swf driver */
else if( Edev == 'f'  ) {
#ifndef NOSWF
 	if( op != 'L' ) { 
 		if( ps_drawing ) PLGF_stroke(); 
 		ps_drawing = 0;
 		}
 
 	switch( op ) {
 		case 'L' : if( ps_new ) PLGF_moveto( lmlx, lmly ); 
 			    PLGF_lineto( x, y );
 			    ps_new = 0;
 			    ps_drawing = 1;
 			    break;
 		case 'M' : ps_new = 1; break;
 		case 'P' : if( ps_new ) PLGF_moveto( lmlx, lmly ); 
 			   PLGF_path( x, y ); 
 			   ps_new = 0;
 			   break;
 	
 		case 'T' : PLGF_text( op, lmlx, lmly, s, 0.0 ); break;
 	
 		case 'C' : if( !vertchar ) PLGF_text( op, lmlx , lmly, s, 0.0 );
 			   else if( vertchar ) PLGF_text( op, lmlx, lmly , s, 0.0 );
 			   break;
 		case 'J' : if( !vertchar ) PLGF_text( op, lmlx, lmly, s, 0.0 );
 			   else if( vertchar ) PLGF_text( op, lmlx, lmly , s, 0.0 );
 			   break;
 
 		case 's' : PLGF_fill( ); return( 0 );
 		case 'r' : PLGF_color( s ); return( 0 );
 		case 'I' : PLGF_pointsize( (int)x ); return( 0 );
 		case 'F' : PLGF_font( s ); return( 0 );
 		case 'D' : PLGF_chardir( (int)x );
 			   if( x == 90 || x == 270 ) vertchar = 1;
 			   else vertchar = 0;
 			   return( 0 );
 		case 'Y' : PLGF_linetype( s, x, y ); return( 0 );
 		case '<' : PLGF_objbegin( s ); return( 0 );
 		case '>' : PLGF_objend(); return( 0 );
 		case 'Z' : pagenum++; virginpage = 1; return( 0 );
 		case 'Q' : if( !virginpage ) pagenum++; 
 			   if( tightbb ) 
 			     stat = PLGF_trailer( bb_x1-0.05, bb_y1-0.05, bb_x2+0.05, bb_y2+0.05 );
 			   else if( specifycrop == 1 )
 			     stat = PLGF_trailer( scx1, scy1, scx2, scy2 );
 			   else if( specifycrop == 2 ) {
 			     stat = PLGF_trailer( (bb_x1-0.05)-scx1, (bb_y1-0.05)-scy1, (bb_x2+0.05)+scx2, (bb_y2+0.05)+scy2 );
 				}
 			   else 
 			     /* add 0.2" margin to be generous in cases of fat lines, etc. */
 			     stat = PLGF_trailer( bb_x1-0.2, bb_y1-0.2, bb_x2+0.2, bb_y2+0.2 );
 			   Eresetbb();
 			   return( stat );
 		}
#endif
 	}



/* interface to postscript driver */
else if( Edev == 'p'  ) {
#ifndef NOPS

	if( op != 'L' ) { 
		if( ps_drawing ) PLGP_stroke(); 
		ps_drawing = 0;
		}

	switch( op ) {
		case 'L' : if( ps_new ) PLGP_moveto( lmlx, lmly ); 
			    PLGP_lineto( x, y );
			    ps_new = 0;
			    ps_drawing = 1;
			    break;
		case 'M' : ps_new = 1; break;
		case 'P' : if( ps_new ) PLGP_moveto( lmlx, lmly ); 
			   PLGP_path( x, y ); 
			   ps_new = 0;
			   break;
		case 'T' : PLGP_text( op, lmlx, lmly, s, 0.0 ); break;
	
		case 'C' : if( !vertchar ) PLGP_text( op, lmlx - 6.0, lmly, s, 12.0 );
			   else if( vertchar ) PLGP_text( op, lmlx, lmly - 6.0, s, 12.0 );
			   break;
		case 'J' : if( !vertchar ) PLGP_text( op, lmlx-12.0, lmly, s, 12.0 );
			   else if( vertchar ) PLGP_text( op, lmlx, lmly - 12.0, s, 12.0 );
			   break;
	
		case 's' : PLGP_fill( ); return( 0 );
		case 'r' : PLGP_color( s ); return( 0 );
		case 'c' : PLGP_closepath(); return( 0 );
		case 'I' : PLGP_pointsize( (int)x ); return( 0 );
		case 'F' : PLGP_font( s ); return( 0 );
		case 'D' : PLGP_chardir( (int)x );
			   if( x == 90 || x == 270 ) vertchar = 1;
			   else vertchar = 0;
			   return( 0 );
		case 'Y' : PLGP_linetype( s, x, y ); return( 0 );
		case 'Z' : PLGP_show(); pagenum++; virginpage = 1; return( 0 );
		case 'O' : PLGP_paper( (int)x ); return( 0 );
		case 'Q' : if( !virginpage ) { PLGP_show(); pagenum++; }
			   if( tightbb ) 
			     PLGP_trailer( pagenum - 1, bb_x1-0.05, bb_y1-0.05, bb_x2+0.05, bb_y2+0.05 );
			   else if( specifycrop == 1 )
			     PLGP_trailer( pagenum - 1, scx1, scy1, scx2, scy2 );
			   else if( specifycrop == 2 ) {
			     PLGP_trailer( pagenum - 1, (bb_x1-0.05)-scx1, (bb_y1-0.05)-scy1, 
							(bb_x2+0.05)+scx2, (bb_y2+0.05)+scy2 );
				}
			   else 
			     /* add 0.2" margin to be generous in cases of fat lines, etc. */
			     PLGP_trailer( pagenum - 1, bb_x1-0.2, bb_y1-0.2, bb_x2+0.2, bb_y2+0.2 );
			   Eresetbb();
			   return( 0 );
		}
#endif
	}


else 	{ 
	if( Edev == '\0' ) return( Eerr( 12021, "Graphics subsystem never initialized", "" ) );
	else 	{
		char sdev[8];
		sprintf( sdev, "%c", Edev );
		return( Eerr( 12022, "Unrecognized graphic device code", sdev ) );
		}
	}




if( op == 'M' ) { lmlx = x; lmly = y; } /* remember most recent 'move' */



/* figure approximate text dimensions */
if( Edev != 'x' && GL_member( op, "TCJ" )) {
	txt_width = strlen( s ) * Ecurtextwidth;
	txt_width *= globalscale;
	}

if( keeping_bb ) {
	/* keep bounding box info (minima and maxima) */
	if( GL_member( op, "LP" ) ) {
		if( prev_op == 'M' ) Ebb( lmlx, lmly );
		Ebb( x, y );
		}
	/* normal (horizontal) text operations.  (vertical text below) */
	else if( op == 'T' && !vertchar ) {
		if( prev_op == 'M' ) Ebb( lmlx, lmly );
		Ebb( lmlx + txt_width+0.05, lmly + (Ecurtextheight*globalscale) );
		}
	else if( op == 'C' && !vertchar ) { 
		Ebb( lmlx - ((txt_width/2.0)+0.05), lmly );
		Ebb( lmlx + ((txt_width/2.0)+0.05), lmly + (Ecurtextheight*globalscale) );
		}
	else if( op == 'J' && !vertchar ) { 
		Ebb( lmlx - (txt_width+0.05), lmly );
		Ebb( lmlx, lmly + (Ecurtextheight*globalscale) );
		}
	}

prev_op = op;


/* handle vertical text .. must be simulated for x windows;
   also gets bounding box for vertical text operations (all devices) */

if( vertchar && GL_member( op, "TCJ" )) verttextsim( op, s );

}

#ifdef NOX11
PLG_getclick()
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
/* bb_ - keep an overall bounding box for the entire image.
	 Also call Echeckbb() to maintain nested object bounding boxes.. */
/* Ebb( double x, double y ) */
PLG_bb( x, y )
double x, y;
{
if( keeping_bb ) {
	if( pcodedebug ) {
		if( ( x < bb_x1 && x < 0.0 ) || (x > bb_x2 && x > 8.0 ) ) fprintf( pcodedebugfp, "draw out X = %g\n", x );
		if( ( y < bb_y1 && y < 0.0 ) || (y > bb_y2 && y > 8.0 ) ) fprintf( pcodedebugfp, "draw out Y = %g\n", y );
		}
	if( x < bb_x1 ) bb_x1 = x;  
	if( x > bb_x2 ) bb_x2 = x; 
	if( y < bb_y1 ) bb_y1 = y; 
	if( y > bb_y2 ) bb_y2 = y; 
	
	}
if( keep_bb2 ) {
	if( x < bb2_x1 ) bb2_x1 = x;
	if( x > bb2_x2 ) bb2_x2 = x;
	if( y < bb2_y1 ) bb2_y1 = y;
	if( y > bb2_y2 ) bb2_y2 = y;
	}


return( 0 );
}

/* ============================================== */
/* RESETBB - needed for multiple pages */
PLG_resetbb()
{
bb_x1 = 999;
bb_y1 = 999;
bb_x2 = -999;
bb_y2 = -999;
return( 0 );
}

/* ============================================= */
/* GETBB - get current bounding box.. */
PLG_getbb( xlo, ylo, xhi, yhi )
double *xlo, *ylo, *xhi, *yhi;
{
*xlo = bb_x1 / globalscale;
*ylo = bb_y1 / globalscaley;
*xhi = bb_x2 / globalscale;
*yhi = bb_y2 / globalscaley;
return( 0 );
}


/* ============================================== */
/* GETTEXTSIZE - get width and height of last text item.. */

PLG_gettextsize( w, h )
  double *w, *h;
{
*w = txt_width;
*h = Ecurtextheight;
}

/* ================================================ */
/* VERTTEXTSIM - vertical text bounding box, also simulation for X11 displays */
static int
verttextsim( op, s )
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
		PLGX_moveto( x, y ); 
		PLGX_text( let, &w ); 
		y -= Ecurtextheight;
		}
	}
#endif
Ebb( x-(Ecurtextheight*globalscale), y1 );
Ebb( x-(Ecurtextheight*globalscale), y2 );
return( 0 );
}


/* ==================================================== */
/* SQUELCH_DISPLAY - 1 = squelch all display activity, 0 restore to normal */
/* Used to calculate bounding box without displaying */
/* handles nested calls. */
PLG_squelch_display( mode )
int mode;
{
static int snest = 0;
if( mode == 1 ) {
	snest++;
	squelched = 1;
	}
else if( mode == 0 ) { 
	if( snest > 0 ) snest--;
	if( snest == 0 )squelched = 0;
	}
}

#ifdef SUSPENDED
/* ==================================================== */
/* KEEP_BB2 - a second bounding box available to app  */
PLG_keep_bb2( mode )
int mode;
{
keep_bb2 = mode;

if( mode == 0 ) { bb2_x1 = 999.0; bb2_y1 = 999.0; bb2_x2 = -999.0; bb2_y2 = -999.0; }
return( 0 );
}
#endif

/* ==================================================== */
/* TIGHTBB - switch ON=don't add margin when doing final BB crop */
PLG_tightbb( mode )
int mode;
{
tightbb = mode;
return( 0 );
}
/* ==================================================== */
/* SPECIFYCROP -  */
PLG_specifycrop( mode, x1, y1, x2, y2 )
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
/* GIFRECT - direct interface to GD driver for better efficiency on rectangles */
PLG_gifrect( xlo, yhi, xhi, ylo, color )
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
PLGG_rect( xlo, yhi, xhi, ylo, color );
Ebb( xlo, ylo );
Ebb( xhi, yhi );
Ecolor( oldcolor );
#endif
return( 0 );
}
/* ==================================================== */
/* IMLOAD - tell the gif driver to load an image */
PLG_imload( filename, scalex, scaley )
char *filename;
double scalex, scaley;
{
#ifndef NOGD
if( globalscale != 1.0 ) {
	scalex *= globalscale;
	scaley *= globalscaley;
	}
return( PLGG_imload( filename, scalex, scaley ) );
#else
return( 1 );
#endif
}

/* ==================================================== */
/* IMPLACE - tell the gif driver to place a GIF image */
PLG_implace( x, y, imalign, xscale, yscale )
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
return( PLGG_implace( x, y, imalign, xscale, yscale ) );
#else
return( 1 );
#endif
}


/* ===================================================== */
/* SETGLOBALSCALE - set global scale factor */
PLG_setglobalscale( sx, sy )
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
/* GETGLOBALSCALE - get global scale factor */
PLG_getglobalscale( sx, sy )
double *sx, *sy;
{
*sx = globalscale;
*sy = globalscaley;
return( 0 );
}

/* ======================================= */
/* SETPOSTEROFS - set poster offset (paginated postscript only).
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
PLG_setposterofs( x, y )
double x, y;
{
postermode = 1;
posterxo = x * (-1.0);
posteryo = y * (-1.0);
}

/* ========================================== */
/* PCODEDEBUG - turn on/off local debugging */
PLG_pcodedebug( mode, fp )
int mode;
FILE *fp;  /* stream for diagnostic output */
{
pcodedebug = mode;
pcodedebugfp = fp;
return( 0 );
}
