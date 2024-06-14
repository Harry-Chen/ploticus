/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


#include "graphcore.h"

/* event modes */
#define STRING 2	/* getting a \n terminated string */
#define EVENTS 3	/* getting any mouse-button or keyboard event */


/* info re: most recent event.. */
static double Eevx, Eevy;
static int Eeid;
static double savex = 0.0, savey = 0.0;
static int savec = 0;
static char semfile[128] = "";



/* ==================================== */
/* get mouse position (x, y) and event code.. */
/* when using postscript, this will do a showpage, endoffile, and exit. */

Egetkey( x, y, e )
double *x, *y;
int *e;
{
int i;
FILE *fp;

if( Edev == 'p' ) { /* postscript-- eject page and return.. */
	Eshow(); 
	return( 0 );
	}
if( Edev == 'g' ) { /* gif, finish up and return */
	Eendoffile(); 
	return( 0 );
	}  

/* Esit();  */

/* Esit replaced by the following code.. */
Eeid = 0;
i = 0;

/* note: the following loop has to run quick enough so as not to
   miss any keystrokes of a fast typist.. */
while( 1 ) {
	Easync();
	if( Eeid != 0 && Eeid < 1005 ) {  /* interested in all keys and mouse buttons
						but no expose, resize, etc. */
		*x = Eevx; *y = Eevy; *e = Eeid;
		break;
		}

	/* every few seconds check the semaphore file.. */
	/* if( i >= 250 ) {
	 *	if( semfile[0] != '\0' ) Eprocess_drawfile( semfile );
	 *	i = 0;
	 *	}
	 */
	i++;

	/* loop delay */
	Eusleep( 20000 ); /* 50 cycles per second */
	}
/*  */

return( 0 );
}
  

/* =================================== */
/* wait until a key or button is hit.. */
/* when using postscript, this will do a showpage and return. */

Egetclick()
{
double x, y;
int e;
if( Edev == 'p' ) { /* postscript-- eject page and return.. */
	Eshow(); 
	return(0);
	}
else if( Edev == 'g' ) {
	Eendoffile();
	exit(0);
	}

Egetkey( &x, &y, &e );
}

/* ================================ */
/* EHE This gets called by the X11 driver when we are waiting for an event 
   and then a key, mouse, expose, or resize event happens. Never called 
   directly by applications.  
*/

Ehe( x, y, e )
double x, y;
int e;
{
int i;
char id[4];

/* set global vars for async processes.. */
EEvent = e; EEventx = x; EEventy = y;

/* call the application's event handler, which must be named Ehandle_events().. */
if( e == E_RESIZE ) {
	Esetwinscale( (int)x, (int)y, x/Epixelsinch, y/Epixelsinch );
	x /= Epixelsinch;
	y /= Epixelsinch;
	EWinx = x;
	EWiny = y;
	}
if( e >= 1010 ) Ehandle_events( x, y, e );

/* user is clicking any mouse button or key.. */
Eevx = x; Eevy = y; Eeid = e;
return( 1 );
}
/* ==================== */
/* the following routines provide a place to save/retrieve an event */
Esavekey( lx, ly, c )
double lx, ly;
int c;
{
savex = lx;
savey = ly;
savec = c;
return( 0 );
}

Eretrievekey( lx, ly, c )
double *lx, *ly;
int *c;
{
*lx = savex;
*ly = savey;
*c = savec;
return( 0 );
}

/* =================================== */
/* Indicate that a semiphore file is to be checked and executed regularly
	when blocking for input.
   s is the full path name of the semiphore file. 
*/
   
Esetsemfile( s )
char *s;
{
strcpy( semfile, s );
return( 0 );
}

/* ==================================== */
/* Execute the semfile */
Esemfile()
{
FILE *fp;
if( Edev == 'p' ) return(0); /* postscript-- just return.. */
/* if( semfile[0] != '\0' ) Eprocess_drawfile( semfile ); */
return( 0 );
}
