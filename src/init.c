/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


/* routines related to low level graphic initialization */

#include <string.h>
#include "plg.h"

#ifndef NOX11
/* #include <X11/Xlib.h> */ /* ??? */
#endif

#define DEFAULT_WIN_WIDTH 8
#define DEFAULT_WIN_HEIGHT 9
#define MM_PER_INCH 25.3807

static int uplefttx = 0, upleftty = 0;
static double pagewidth = DEFAULT_WIN_WIDTH, pageheight = DEFAULT_WIN_HEIGHT;
static int initialized = 0;
static char outfilename[ MAXPATH ] = "";
static char outlabel[80] = "ploticus-graphic";


/* ========================================== */
PLG_init_initstatic()
{
uplefttx = 0;
upleftty = 0;
pagewidth = DEFAULT_WIN_WIDTH;
pageheight = DEFAULT_WIN_HEIGHT;
initialized = 0;
strcpy( outfilename, "" );
strcpy( outlabel, "ploticus-graphic" );
return( 0 );
}

/* ========================================== */
/* INIT - initialize device */

PLG_init( dev )
char dev;     /* device code */
{
char msg[100];
int yr, mon, day, hr, min, sec, i;
char host[30]; 
FILE *lfp;
char sdev[8];
double sx, sy;
int stat;

GL_sysdate( &mon, &day, &yr ); GL_systime( &hr, &min, &sec );

/* initialize graphics parameters.. */
Edev = dev;

if( dev == 'p' || dev == 'c' || dev == 'e' ) {
#ifdef NOPS
	return( Eerr( 12016, "PostScript capability was not included in this build.", "" ) );  
#else

	stat = PLGP_setup( outlabel, dev, outfilename );
	if( stat ) return( stat );
	Edev = 'p'; /* driver now knows eps/color/bw, from now on call it 'p' */
#endif
	}

/* added support for svg - BT 05/11/01 */
else if( dev == 's' ) {
#ifdef NOSVG
	return( Eerr( 12016, "SVG capability was not included in this build.", "" ) );  
#else
        Epixelsinch = 72; 
        Esetwinscale( (int)(pagewidth*Epixelsinch), (int)(pageheight*Epixelsinch), pagewidth, pageheight );
        stat = PLGS_setup( outlabel, dev, outfilename, Epixelsinch, pagewidth, pageheight, uplefttx, upleftty );
	if( stat ) return( stat );
#endif
        }

else if( dev == 'x' ) {
#ifdef NOX11
	return( Eerr( 12016, "X11 capability was not included in this build.", "" ) );  
#else
	Epixelsinch = 75; 
	Esetwinscale( (int)(pagewidth*Epixelsinch), (int)(pageheight*Epixelsinch), pagewidth, pageheight );
	Egetglobalscale( &sx, &sy );
	stat = PLGX_setup( outlabel, Epixelsinch, pagewidth, pageheight, uplefttx, upleftty );
	if( stat ) return( stat );
#endif
	}
else if( dev == 'g' ) {
#ifdef NOGD
	return( Eerr( 12016, "GIF/PNG capability was not included in this build.", "" ) );  
	
#else
	Epixelsinch = 100;
	Esetwinscale( (int)(pagewidth*Epixelsinch), (int)(pageheight*Epixelsinch), pagewidth, pageheight );
	stat = PLGG_setup( outlabel, Epixelsinch, pagewidth, pageheight, uplefttx, upleftty );
	if( stat ) return( stat );
	Edev = 'g';
#endif
	}

else if( dev == 'f' ) {
#ifdef NOSWF
        return( Eerr( 12016, "SWF capability was not included in this build.", "" ) );  
#else
         Epixelsinch = 72; 
         Esetwinscale( (int)(pagewidth*Epixelsinch), (int)(pageheight*Epixelsinch), pagewidth, pageheight );
         stat = PLGF_setup( outlabel, dev, outfilename, Epixelsinch, pagewidth, pageheight, uplefttx, upleftty );
         if( stat ) return( stat );
#endif
         }


else 	{ 
	sprintf( sdev, "%c", dev );
	return( Eerr( 12016, "Unsupported display device code", sdev ) );  
	}

initialized = 1;
EWinx = pagewidth;  EWiny = pageheight;
PLG_setdefaults(); 
return( 0 );
}

/* ================================== */
/* SETSIZE - set the size and position of the display.  Should be called before init()
*/
PLG_setsize( ux, uy, upleftx, uplefty )
double ux, uy;  /* size of window in inches.. */
int upleftx, uplefty; /* point (in native window system coords ) of upper-left corner of window */
{


if( ! initialized ) { /* getting ready to initialize-- set size parameters */
	pagewidth = ux; pageheight = uy; uplefttx = upleftx; upleftty = uplefty;
	return( 0 );
	}

if( initialized ) {    /* window already exists, resize it.. */
	/* update parameters: window size and original size */
	/* do this regardless of device since any code may use EWin variables.. */
	if( ux >= 0 ) { EWinx = ux; EWinx_0 = ux; }
	if( uy >= 0 ) { EWiny = uy; EWiny_0 = uy; }
#ifndef NOX11
	if( Edev == 'x' ) {
		/* update scaling */
		Esetwinscale( (int)(ux*Epixelsinch), (int)(uy*Epixelsinch), ux, uy );
		/* resize window */
		PLGX_resizewin( Epixelsinch, upleftx, uplefty, ux, uy ); 
		}
#endif
#ifndef NOGD
	else if( Edev == 'g' ) {
		/* terminate existing image and start a new one with new size.. */
		Esetwinscale( (int)(pagewidth*Epixelsinch), (int)(pageheight*Epixelsinch), pagewidth, pageheight );
		PLGG_setup( "", Epixelsinch, pagewidth, pageheight, uplefttx, upleftty );
		}
#endif
	}
return( 0 );
}


/* ====================================== */
PLG_setdefaults()
{
Efont( Estandard_font );
Etextsize( 10 );
Etextdir( 0 );
Elinetype( 0, 0.6, 1.0 );
Ecolor( Estandard_color );
Ebackcolor( Estandard_bkcolor );
Escaletype( "linear", 'x' );
Escaletype( "linear", 'y' );
Epaper( 0 );
Eblacklines = 1; /* set to 0 only when half-tone lines desired when displaying on b/w device */
EEvent = 0;
return( 0 );
}

/* =================== */
PLG_setoutfilename( name )
char *name;
{
strncpy( outfilename, name, MAXPATH-1 );
outfilename[ MAXPATH-1 ] = '\0';
return( 0 );
}

/* =================== */
PLG_getoutfilename( name )
char *name;
{
strcpy( name, outfilename );
return( 0 );
}

/* ==================== */
PLG_setoutlabel( name )
char *name;
{
strncpy( outlabel, name, 78 );
outlabel[78] = '\0';
return( 0 );
}


/* ========================= */
PLG_handle_events( x, y, e )
double x, y;
int e;
{
if( e == E_EXPOSE || e == E_RESIZE ) Erestorewin();
return( 0 );
}
