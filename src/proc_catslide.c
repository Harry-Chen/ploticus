/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC CATSLIDE - adjust categories left or right, for doing pairs */

#include "pl.h"

PLP_catslide()
{
int i;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

int stat;
int align;
double adjx, adjy;

char axis;
double amount;

TDH_errprog( "pl proc catslide" );

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( stricmp( attr, "axis" )==0 ) axis = tolower( val[0] );
	else if( stricmp( attr, "amount" )==0 ) amount = atof( val );
	else Eerr( 1, "attribute not recognized", attr );
	}

Esetcatslide( axis, amount );

return( 0 );
}
