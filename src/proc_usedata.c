/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC USEDATA */

/* Note: if no attributes given, this does the same as the old 'proc originaldata' */

#include "pl.h"

PLP_usedata( )
{
int i;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

int stat;
int align;
double adjx, adjy;


TDH_errprog( "usedata" );

if( PLD.curds <= 0 ) return( Eerr( 2478, "no data have been read yet", "" ));

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( stricmp( attr, "element" )== 0 ) {
		if( atoi( val ) <= PLD.curds ) PLD.curds = atoi( val );
		}
	else if( stricmp( attr, "pop" )== 0 ) PLD.curds -= atoi( val );
	else if( stricmp( attr, "original" )== 0 ) PLD.curds = 0;
	else if( stricmp( attr, "fieldnames" )==0 ) definefieldnames( lineval );
	}

if( first == 1 ) PLD.curds = 0;   /* no attributes specified - (originaldata) */

if( PLD.curds < 0 ) PLD.curds = 0;

if( PLS.debug ) fprintf( PLS.diagfp, "using data set %d\n", PLD.curds );
setintvar( "NRECORDS", Nrecords );
setintvar( "NFIELDS", Nfields );

return( 0 );
}

