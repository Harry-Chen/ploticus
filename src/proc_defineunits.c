/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


/* PROC DEFINEUNITS - allow special units to be defined for an axis separate from AREADEF */
	
#include "pl.h"

PLP_defineunits()
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
char units[40];

TDH_errprog( "pl proc defineunits" );

/* initialize */
axis = '?';
strcpy( units, "linear" );

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( stricmp( attr, "axis" )==0 ) axis = tolower(val[0]);
	else if( stricmp( attr, "units" )==0 ) strcpy( units, lineval );

	else Eerr( 1, "attribute not recognized", attr );
	}

Esetunits( axis, units );

return( 0 );
}
