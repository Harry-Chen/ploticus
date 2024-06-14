/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC RANGESWEEP - render a sweep */

#include "pl.h"
#define MOVE 0
#define LINE 1
#define PATH 2


PLP_rangesweep()
{
int i;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

int stat;
int align;
double adjx, adjy;

int xfield;
int lofield, hifield;
double start, stop;
double xstart;  /* when no X values are given, can be specified as to where to start */
char color[40];
int j;
int npoints;
double f;
double x, lo, hi, lastx, lastlo, lasthi;
char legendlabel[128];
char selectex[256];
int result;

TDH_errprog( "pl proc rangesweep" );

/* initialize */
xfield = -1;
lofield = -1;
hifield = -1;

start = EDXlo; stop = EDXhi;
xstart = EDXlo;

strcpy( color, "gray(0.9)" );
strcpy( legendlabel, "" );
strcpy( selectex, "" );


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( stricmp( attr, "xfield" )==0 ) xfield = fref( val ) - 1;
	else if( stricmp( attr, "lofield" )==0 ) lofield = fref( val ) - 1;
	else if( stricmp( attr, "hifield" )==0 ) hifield = fref( val ) - 1;
	else if( stricmp( attr, "legendlabel" )==0 ) strcpy( legendlabel, lineval );
	else if( stricmp( attr, "sweeprange" )==0 ) {
		getrange( lineval, &start, &stop, 'x', EDXlo, EDXhi );
		}
	else if( stricmp( attr, "xstart" )==0 ) {
		xstart = Econv( X, val );
		if( Econv_error() ) xstart = EDXlo;
		}
	else if( stricmp( attr, "select" )==0 ) strcpy( selectex, lineval );

	else if( stricmp( attr, "color" )==0 ) strcpy( color, val );

	else Eerr( 1, "rangesweep attribute not recognized", attr );
	}

/* -------------------------- */
/* overrides and degenerate cases */
/* -------------------------- */
if( Nrecords[Dsel] < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );
if( !scalebeenset() ) 
         return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );


if( (lofield < 0 || lofield >= Nfields[Dsel] )) return( Eerr( 601, "lofield out of range", "" ) );
if( (hifield < 0 || hifield >= Nfields[Dsel] )) return( Eerr( 601, "hifield out of range", "" ) );
if( xfield >= Nfields[Dsel] ) return( Eerr( 601, "xfield out of range", "" ) );
 
/* -------------------------- */
/* now do the plotting work.. */
/* -------------------------- */

/* put all values into Dat array.. */
j = 0;
f = xstart;
for( i = 0; i < Nrecords[Dsel]; i++ ) {

	if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
                stat = do_select( selectex, i, &result );
                if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
                if( result == 0 ) {  /* mark with +huge */
			/* Dat[j++] = PLHUGE;
			 * Dat[j++] = PLHUGE;
			 * Dat[j++] = PLHUGE;
			 * Changed scg 8/17/00 - these should not even go into Dat array.. 
			 */
			 continue;
			}
                }


	/* X */
	if( xfield < 0 ) {
		Dat[j] = f;
		f += 1.0;
		}
	else 	{
		Dat[j] = fda( i, xfield, X );
		if( Econv_error() ) { 
			conv_msg( i, xfield, "xfield" ); 
			Dat[j] = NEGHUGE;
			}
		}

	j++; 

	/* LO */
	Dat[j] = fda( i, lofield, Y );
	if( Econv_error() ) { 
		conv_msg( i, lofield, "yfield" ); 
		Dat[j] = NEGHUGE;
		continue;
		}
	j++;

	/* HI */
	Dat[j] = fda( i, hifield, Y );
	if( Econv_error() ) { 
		conv_msg( i, hifield, "hifield" ); 
		Dat[j] = NEGHUGE;
		continue;
		}
	j++;



	if( j >= MAXDAT-3 ) {
		Eerr( 3579, "Sorry, too many points, sweep truncated", "" );
		break;
		}
	}

npoints = j / 3;



/* draw the sweep.. */
/* ---------------- */

first = 1;
lastlo = 0.0;
lasthi = 0.0;
lastx = 0.0;
for( i = 0; i < npoints; i++ ) {
	if( !first && (hi > (NEGHUGE+1) && lo > (NEGHUGE+1) && 
	       x > (NEGHUGE+1) && x < (PLHUGE-1) ) )  { 
		lastlo = lo; 
		lasthi = hi; 
		lastx = x;
		}
	x = dat3d(i,0);
	lo = dat3d(i,1);
	hi = dat3d(i,2);

	/* fprintf( stderr, "[last: x=%g lo=%g hi=%g   current: x=%g lo=%g hi=%g]", lastx, lastlo, lasthi, x, lo, hi ); */

	/* skip bad values and places */
	if( x < (NEGHUGE+1) || lo < (NEGHUGE+1) || hi < (NEGHUGE+1) ) { 
		/* fprintf( stderr, "[skip]\n" );  */
		continue; 
		}

	/* if lo > hi reset so a new sweep can be started later.. */
	if( lo > hi || x > (PLHUGE-1) ) { 
		first = 1;
		/* fprintf( stderr, "[reset]\n" ); */
		continue;
		}

	if( x < start ) {
		/* fprintf( stderr, "[too lo]\n" ); */
		continue; /* out of range - lo */
		}
	if( x > stop ) {     /* out of range - hi */ 
		/* fprintf( stderr, "[too hi]\n" ); */
		break;
		}


	if( first ) {
		/* fprintf( stderr, "[First]\n" ); */
		first = 0;
		continue;
		}

	if( !first ) {
		/* fprintf( stderr, "[Draw]\n" ); */
		Emovu( x, lo ); Epathu( lastx, lastlo ); 
		Epathu( lastx, lasthi ); 
		Epathu( x, hi );
		Ecolorfill( color );
		continue;
		}
	}


 
if( legendlabel[0] != '\0' ) {
	add_legent( LEGEND_COLOR, legendlabel, "", color, "", "" );
	}

return( 0 );
}
