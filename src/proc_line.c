/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC LINE - draw arbitrary line(s) */

#include "pl.h"

PLP_line()
{
int i;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

char buf[256];
int stat;
int align;
double adjx, adjy;
double x, y;
char linedetails[256];
char notation[80];
char a[40], b[40], c[40], d[40];
int ix;
int buflen;


TDH_errprog( "pl proc line" );

/* initialize */
strcpy( PL_bigbuf, "" );
strcpy( linedetails, "" );
strcpy( notation, "locvalue" );
x = 0.0;
y = 0.0;

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( stricmp( attr, "points" )==0 ) 
		getmultiline( "points", lineval, MAXBIGBUF, PL_bigbuf );
		

	else if( stricmp( attr, "linedetails" )==0 ) strcpy( linedetails, lineval );
	else if( stricmp( attr, "notation" )==0 ) strcpy( notation, val );

	else Eerr( 1, "attribute not recognized", attr );
	}


/* now do the plotting work.. */

linedet( "linedetails", linedetails, 1.0 );
if( !GL_member( notation[0], "aAsSlL" )) {
	strcpy( notation, "a" );
	Eerr( 479, "notation must be one of: absolute scaled locval", notation );
	}
ix = 0;
first = 1;
buflen = strlen( PL_bigbuf );
while( 1 ) {
	GL_getchunk( buf, PL_bigbuf, &ix, "\n" );
	nt = sscanf( buf, "%s %s %s %s", a, b, c, d );

	if( nt == 4 || first ) { 
		if( notation[0] == 'a' ) { 
			Emov( atof( a ), atof( b ) ); 
			/* if( !first ) */ Elin( atof( c ), atof( d ) ); 
			}
		else if( notation[0] == 's' ) { 
			Emov( PL_u( X, a ), PL_u( Y, b ) ); 
			if( Econv_error() ) Eerr( 2945, "unplottable value(s) ", buf );
			if( nt == 4 ) { 
				Elin( PL_u( X, c ), PL_u( Y, d ) ); 
				if( Econv_error() ) Eerr( 2946, "unplottable value(s) ", buf );
				} 
			}
		else if( notation[0] == 'l' ) { 
			Eposex( a, X, &x ); Eposex( b, Y, &y ); Emov( x, y );
			if( Econv_error() ) Eerr( 2947, "unplottable value(s) ", buf );
			/* if( !first ) { */
				Eposex( c, X, &x ); Eposex( d, Y, &y ); Elin( x, y );
				if( Econv_error() ) Eerr( 2948, "unplottable value(s) ", buf );
			/* 	} */
			}
		}

	else if( nt == 2 ) { 
		if( notation[0] == 'a' ) Elin( atof( a ), atof( b ) ); 
		else if( notation[0] == 's' ) Elin( PL_u( X, a ), PL_u( Y, b ) ); 
		else if( notation[0] == 'l' ) { 
			Eposex( a, X, &x ); Eposex( b, Y, &y ); Elin( x, y );
			}
		}
	else if( nt <= 0 ) ;
	else Eerr( 2959, "warning: points must have either 4 or 2 values per line", "" );

	first = 0;

	if( ix >= buflen ) break;
	}
return( 0 );
}
