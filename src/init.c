/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


/* EINIT - low level elib initialization. */
#include "graphcore.h"
#ifndef NOX11
#include <X11/Xlib.h>
#endif

/* following was width=8 height=9 scg 11/14/00 */
#define DEFAULT_WIN_WIDTH 8
#define DEFAULT_WIN_HEIGHT 9
#define MM_PER_INCH 25.3807

static int Upleftx = 0, Uplefty = 0;
static double Ux = DEFAULT_WIN_WIDTH, Uy = DEFAULT_WIN_HEIGHT;
static int Initialized = 0;
static char outfilename[120] = "";

Einit( name, dev )
char name[];
char dev;     /* 'x' for x11; 'p' for postscript monochrome; 'c' for postscript color;
		  'e' for EPS (color); 'g' for GIF */
{
char msg[100];
int yr, mon, day, hr, min, sec, i;
char host[30]; 
FILE *lfp;
char *pix, *getenv();
char sdev[8];
double sx, sy;

GL_sysdate( &mon, &day, &yr ); GL_systime( &hr, &min, &sec );

/* initialize graphics parameters.. */
Edev = dev;

if( dev == 'p' || dev == 'c' || dev == 'e' ) {
	EPSsetup( name, dev, outfilename );
	Edev = 'p'; /* driver now knows eps/color/bw, from now on call it 'p' */
	}

/* added support for svg - BT 05/11/01 */
else if( dev == 's' ) {
        Epixelsinch = 72; 
        Esetwinscale( (int)(Ux*Epixelsinch), (int)(Uy*Epixelsinch), Ux, Uy );
        SVGsetup( name, dev, outfilename,Epixelsinch, Ux, Uy, Upleftx, Uplefty );
        }

else if( dev == 'x' ) {
#ifdef NOX11
	Eerr( 12016, "X11 capability was not included in this build.", sdev ); 
	exit(1);
#else
	Epixelsinch = 75; 
	Esetwinscale( (int)(Ux*Epixelsinch), (int)(Uy*Epixelsinch), Ux, Uy );
	Egetglobalscale( &sx, &sy );
	EXWsetup( name, Epixelsinch, Ux, Uy, Upleftx, Uplefty );
#endif
	}
else if( dev == 'g' ) {
#ifdef NOGD
	Eerr( 12016, "GIF/PNG capability was not included in this build.", sdev ); 
	exit(1);
#else
	Epixelsinch = 100;
	Esetwinscale( (int)(Ux*Epixelsinch), (int)(Uy*Epixelsinch), Ux, Uy );
	EGsetup( name, Epixelsinch, Ux, Uy, Upleftx, Uplefty );
	Edev = 'g';
#endif
	}

else 	{ 
	sprintf( sdev, "%c", dev );
	Eerr( 12016, "Unsupported display device code", sdev ); 
	exit(1); 
	}

Initialized = 1;
EWinx = Ux;  EWiny = Uy;
Esetdefaults(); 
}

/* ================================== */
/* Set the size and position of the display.
   This affects X11 only.
   Should be called before Einit.
*/
Esetsize( ux, uy, upleftx, uplefty )
double ux, uy;  /* size of window in inches.. */
int upleftx, uplefty; /* point (in native window system coords ) of upper-left corner of window */
{

if( ! Initialized ) { /* getting ready to initialize-- set size parameters */
	Ux = ux; Uy = uy; Upleftx = upleftx; Uplefty = uplefty;
	return( 0 );
	}

if( Initialized ) {    /* window already exists, resize it.. */
	/* update parameters: window size and original size */
	/* do this regardless of device since any code may use EWin variables.. */
	if( ux >= 0 ) { EWinx = ux; EWinx_0 = ux; }
	if( uy >= 0 ) { EWiny = uy; EWiny_0 = uy; }
#ifndef NOX11
	if( Edev == 'x' ) {
		/* update scaling */
		Esetwinscale( (int)(ux*Epixelsinch), (int)(uy*Epixelsinch), ux, uy );
		/* resize window */
		EXWresizewin( Epixelsinch, upleftx, uplefty, ux, uy ); 
		}
#endif
#ifndef NOGD
	else if( Edev == 'g' ) {
		/* terminate existing image and start a new one with new size.. */
		Esetwinscale( (int)(Ux*Epixelsinch), (int)(Uy*Epixelsinch), Ux, Uy );
		EGsetup( "", Epixelsinch, Ux, Uy, Upleftx, Uplefty );
		}
#endif
	}
return( 0 );
}


/* ====================================== */
Esetdefaults()
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
}

/* =================== */
Esetoutfilename( name )
char *name;
{
strcpy( outfilename, name );
}
/* =================== */
Egetoutfilename( name )
char *name;
{
strcpy( name, outfilename );
}
