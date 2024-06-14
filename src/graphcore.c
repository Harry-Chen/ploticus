/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


/* small, lowlevel routines re: scaled units */
/* see also units.c which is a layer above this. */

#include "graphcore.h"


FILE *Errfp;

/* overall settings - these can be set by application program before Einit() */
char Estandard_font[60] = "/Helvetica";  /* default font */
int Estandard_textsize = 10;	 /* standard text size, relative to 10 pt. */
double Estandard_lwscale = 1.0;		 /* scaling factor for line width */
char Estandard_color[30] = "black" ;	 /* default drawing/text color */
char Estandard_bkcolor[30] = "white";	 /* default background color */

/* window size.. */
double EWinx, EWiny;
/* original window size.. */
double EWinx_0, EWiny_0;

/* basic device */
char Edev;

int  Epixelsinch;  /* number of pixels/inch */

/* current parameters.. */
char Ecurfont[60] = "";
int Ecurtextsize = 0;
double Ecurtextheight = 0.0;
double Ecurtextwidth = 0.0;
int Ecurtextdirection = 0;
int Ecurpaper = -1;
double Ecurlinewidth = -1.0;
int Ecurlinetype = -1;
double Ecurpatternfactor = 0.0;
char Ecurcolor[30]; /* current color for lines, text, shading */
char Ecurbkcolor[30]; /* current color for background (when screen clears) */
char Ecurhilitedraw[30]; /* current color for lines, text, shading in highlighted areas */
char Ecurhilitebk[30];  /* current color for background of highlighted areas */

/* event information */
int EEvent;
double EEventx, EEventy;

/* scaling.. */
double EXlo, EXhi, EYlo, EYhi;		/* graphic area bounds, absolute coords */
double EDXlo, EDXhi, EDYlo, EDYhi;		/* graphic area bounds, data coords */
double EScale_x = 1, EScale_y = 1; 		/* linear scaling factors in x and y */
int Escaletype_x = E_LINEAR;	/* either LINEAR or LOG */
int Escaletype_y = E_LINEAR;	/* either LINEAR or LOG */

double Ex1, Ey1; /* last moveto */
double Ex2, Ey2; /* last lineto */

char Eprogname[40] = ""; /* program name, used in error messages. */
int Eflip = 0;  /* if 1, reverse x and y, for doing sideways variations */
int Eblacklines = 0; /* if 1, and device is a b/w device, make non-white lines black unconditionally */
long Eflashdelay = 150000; /* flashpick delay in microseconds */


double log();

/* ============================ */
/* Select the scaling method */

Escaletype( typ, axis )
char typ[];
char axis;
{
char buf[80];
int stat;

if( stricmp( typ, "linear" )==0 ) {
	/* Esetunits( axis, "linear" ); */
	if( axis == 'x' ) Escaletype_x = E_LINEAR;
	else if( axis == 'y' ) Escaletype_y = E_LINEAR;
	return( 0 );
	}

else if( stricmp( typ, "log" )==0 ) {
	/* Esetunits( axis, "linear" ); */ /* linear ok */
	if( axis == 'x' ) Escaletype_x = E_LOG;
	else if( axis == 'y' ) Escaletype_y = E_LOG;
	return( 0 );
	}

else if( stricmp( typ, "log+1" )==0 ) {		/* log+1 added scg 11/29/00 */
	if( axis == 'x' ) Escaletype_x = E_LOGPLUS1;
	else if( axis == 'y' ) Escaletype_y = E_LOGPLUS1;
	return( 0 );
	}

else	{
	/* stat = Esetunits( axis, typ );
	 * if( stat != 0 ) {
	 *	sprintf( buf, "Invalid scaling type for %c axis", axis );
	 *	return( Eerr( 101, buf, typ ) );
	 *	}
	 */

	/* special units always use linear as the basic units.. */
	if( axis == 'x' ) Escaletype_x = E_LINEAR;
	else Escaletype_y = E_LINEAR;
	return( 0 );
	}
}

/* =========================== */
/* ESCALE_X - for setting up scaling in x */

Escale_x( xlow, xhi, datalow, datahi )
double 	xlow, 	/* absolute x location of left side of the area */
	xhi, 	/* absolute x location of the right side of the area */
	datalow, /* data-units x at the left side */
	datahi;	 /* data-units x at the right side */
{
char msgbuf[100];

EXlo = xlow;
EXhi = xhi;
EDXlo = datalow;
EDXhi = datahi;

if( datahi-datalow <= 0 ) {
	sprintf( msgbuf, "Left portion of xrange must be < right (%g %g)", datalow, datahi );
	return( Eerr( 100, msgbuf, "" ) );
	}

if( xhi-xlow <= 0 ) {
	sprintf( msgbuf, "Error in x absolute plot area dimensions (%g and %g)", xlow, xhi);
	return( Eerr( 101, msgbuf, "" ) );
	}
	
if( Escaletype_x == E_LINEAR ) EScale_x = (xhi-xlow) / (datahi-datalow) ;
else if( Escaletype_x == E_LOG ) {
	if( datalow <= 0.0 ) datalow = 0.01; /* ??? */
	EScale_x = (xhi-xlow) / (log( datahi ) - log( datalow ));
	}
else if( Escaletype_x == E_LOGPLUS1 ) {
	if( (datalow) < 0.0 ) datalow = 0.0; 
	EScale_x = (xhi-xlow) / (log( datahi+1.0 ) - log( datalow+1.0 ));
	}
return( 0 );
}

/* =========================== */
/* ESCALE_Y - for setting up scaling in y */

Escale_y( ylow, yhi, datalow, datahi )
double 	ylow, 	/* absolute y location of low side of the area */
	yhi, 	/* absolute y location of high side of the area */
	datalow, /* data-units y at the low side */
	datahi;	 /* data-units y at the high side */
{
double logpart, linpart;
char msgbuf[100];

EYlo = ylow;
EYhi = yhi;
EDYlo = datalow;
EDYhi = datahi;

if( datahi-datalow <= 0 ) {
	sprintf( msgbuf, "Left portion of yrange must be < right (%g %g)", datalow, datahi );
	return( Eerr( 100, msgbuf, "" ) );
	}

if( yhi-ylow <= 0 ) {
	sprintf( msgbuf, "Error in y absolute plot area dimensions (%g and %g)", ylow, yhi);
	return( Eerr( 101, msgbuf, "" ) );
	}


if( Escaletype_y == E_LINEAR ) EScale_y = (yhi-ylow) / (datahi-datalow) ;

else if( Escaletype_y == E_LOG ) {
	if( datalow <= 0.0 ) datalow = 0.01; /* ??? */
	EScale_y = (yhi-ylow) / (log( datahi ) - log( datalow ));
	}
else if( Escaletype_y == E_LOGPLUS1 ) {
	if( (datalow) < 0.0 ) datalow = 0.0; 
	EScale_y = (yhi-ylow) / (log( datahi+1.0 ) - log( datalow+1.0 ));
	}
return( 0 );
}

/* =========================== */
/* EA -  Returns an absolute location from a data value in xory.
   This is the preferred function to use because it handles flip. */

double Ea( xory, d )
char xory;
double d;
{
if( Eflip ) {
	if( xory == 'x' ) return( Eay( d ) );
	else if( xory == 'y' ) return( Eax( d ) );
	}
else	{
	if( xory == 'x' ) return( Eax( d ) );
	else if( xory == 'y' ) return( Eay( d ) );
	}
Eerr( 15, "Ea: nonsensical parameters", "" );
}


/* =========================== */
/* EAX - returns an absolute x location from a data value */

double Eax( d )
double d;
{
double f;


if( Escaletype_x == E_LINEAR ) 
	return( EXlo + (( d - EDXlo ) * EScale_x ));
else if( Escaletype_x == E_LOG ) {
	if( d <= 0.0 ) return( EXlo );
	else if( EDXlo <= 0.0 ) return( EXlo + (( log( d ) - log( 1.0 ) ) * EScale_x ) );
	else return( EXlo + (( log( d ) - log( EDXlo ) ) * EScale_x ) );
	}
else if( Escaletype_x == E_LOGPLUS1 ) {
	if( d <= 0.0 ) return( EXlo );
	else if( EDXlo <= 0.0 ) return( EXlo + (( log( d+1.0 ) - log( 1.0 ) ) * EScale_x ) );
	else return( EXlo + (( log( d+1.0 ) - log( EDXlo ) ) * EScale_x ) );
	}
}

/* =========================== */
/* EAY - returns an absolute y location from a data value */
double Eay( d )
double d;
{
if( Escaletype_y == E_LINEAR ) return( EYlo + (( d - EDYlo ) * EScale_y ));
else if( Escaletype_y == E_LOG ) {
	if( d <= 0.0 ) return( EYlo );
	else if( EDYlo <= 0.0 ) return( EYlo + (( log( d ) - log( 1.0 ) ) * EScale_y ) );
	else return( EYlo + (( log( d ) - log( EDYlo ) ) * EScale_y ) );
	}
else if( Escaletype_y == E_LOGPLUS1 ) {
	if( d <= 0.0 ) return( EYlo );
	else if( EDYlo <= 0.0 ) return( EYlo + (( log( d+1.0 ) - log( 1.0 ) ) * EScale_y ) );
	else return( EYlo + (( log( d+1.0 ) - log( EDYlo ) ) * EScale_y ) );
	}
}




/* =========================== */
/* EDX - given an abs coord in X, returns a value in data space */

double Edx( a )
double a;
{
double h;
if( Escaletype_x == E_LINEAR ) {
	h = a - EXlo;
	return( EDXlo + ( h / EScale_x ) );
	}
else if( Escaletype_x == E_LOG ) {
	if( a < EXlo ) return( EDXlo );
	h = log( a ) - log( EDXlo );
	return( EDXlo + ( h / EScale_x ) );
	}
else if( Escaletype_x == E_LOGPLUS1 ) {
	if( a < EXlo ) return( EDXlo );
	h = log( a ) - log( EDXlo );
	return( (EDXlo + ( h / EScale_x ) ) - 1.0 ); /* ??? */
	}
}

/* =========================== */
/* EDY - given an abs coord in Y, returns a value in data space */

double Edy( a )
double a;
{
double h;
if( Escaletype_y == E_LINEAR ) {
	h = a - EYlo;
	return( EDYlo + ( h / EScale_y ) );
	}
else if( Escaletype_y == E_LOG ) {
	if( a < EYlo ) return( EDYlo );
	h = log( a ) - log( EDYlo );
	return( EDYlo + ( h / EScale_y ) );
	}
else if( Escaletype_y == E_LOGPLUS1 ) {
	if( a < EYlo ) return( EDYlo );
	h = log( a ) - log( EDYlo );
	return( (EDYlo + ( h / EScale_y ) ) - 1.0 ); /* ??? */
	}
}


/* ====================== */
/* ELIMIT - Get minima or maxima of either axis, 
   in either absolute or scaled units.. */

double Elimit( axis, end, units )
char axis;
char end; /* either 'l' == lo  or 'h' == hi */
char units; /* either 'a' == absolute or 's' == scaled */
{
if( axis == 'x' ) {
	if( end == 'l' && units == 's' ) return( EDXlo );
	else if( end == 'h' && units == 's' ) return( EDXhi );
	if( end == 'l' && units == 'a' ) return( EXlo );
	else if( end == 'h' && units == 'a' ) return( EXhi );
	}
if( axis == 'y' ) {
	if( end == 'l' && units == 's' ) return( EDYlo );
	else if( end == 'h' && units == 's' ) return( EDYhi );
	if( end == 'l' && units == 'a' ) return( EYlo );
	else if( end == 'h' && units == 'a' ) return( EYhi );
	}
Eerr( 12015, "warning, bad values passed to Elimit", "" );
return( 0.0 );
}
