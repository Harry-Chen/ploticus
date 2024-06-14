/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC BEVELRECT - bevelled rectange, e.g. for button */

#include "pl.h"


PLP_rect()
{
int i;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

int stat;
int align;
double adjx, adjy;

double xlo, ylo, xhi, yhi;
char color[40];
double bevelsize;
char lowbevelcolor[40], hibevelcolor[40], shadowcolor[40];
double shadowsize;
int gotrect;
char outline[128];
int ioutline;


TDH_errprog( "pl proc rect" );


/* initialize */
strcpy( color, "dullyellow" );
/* bevelsize = 0.1; */
bevelsize = 0.0;
strcpy( lowbevelcolor, "0.6" );
strcpy( hibevelcolor, "0.8" );
strcpy( shadowcolor, "black" );
shadowsize = 0.0;
strcpy( outline, "" );
ioutline = 0;
gotrect = 0;


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( stricmp( attr, "rectangle" )==0 ) {
                getbox( "box", lineval, &xlo, &ylo, &xhi, &yhi );
		gotrect = 1;
		}
	else if( stricmp( attr, "color" )==0 ) strcpy( color, val );
	else if( stricmp( attr, "bevelsize" )==0 ) bevelsize = atof( val );
	else if( stricmp( attr, "shadowsize" )==0 ) shadowsize = atof( val );
	else if( stricmp( attr, "lowbevelcolor" )==0 ) strcpy( lowbevelcolor, val );
	else if( stricmp( attr, "hibevelcolor" )==0 ) strcpy( hibevelcolor, val );
	else if( stricmp( attr, "shadowcolor" )==0 ) strcpy( shadowcolor, val );
	else if( stricmp( attr, "outline" )==0 ) {
		strcpy( outline, lineval );
		if( GL_smember( val, "no none" )==0 ) ioutline = 1;
		}

	else Eerr( 1, "attribute not recognized", attr );
	}



/* now do the plotting work.. */

if( !gotrect ) return( Eerr( 695, "No rectangle specified", "" ));

if( color[0] != '\0' ) {
	linedet( "outline", outline, 0.5 );
	Ecblock( xlo, ylo, xhi, yhi, color, ioutline );
	}

if( bevelsize > 0.0 || shadowsize > 0.0 ) 
	Ecblockdress( xlo, ylo, xhi, yhi,
       		bevelsize, lowbevelcolor, hibevelcolor, shadowsize, shadowcolor);

return( 0 );
}
