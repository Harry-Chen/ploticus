/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC PRINT - allow printing of variable or data field contents  */

#include <string.h>
#include "pl.h"

PLP_print()
{
int i;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

int stat;
int align;
double adjx, adjy;

char buf[256];
char label[256];
char selectex[256];
int result;
char printstring[256];
char outfile[MAXPATH];
char outmode[40];
FILE *outfp;
int dontclose;
int nrecords;


TDH_errprog( "pl proc print" );


/* initialize */
strcpy( printstring, "" );
strcpy( label, "" );
strcpy( selectex, "" );
strcpy( outfile, "" );
strcpy( outmode, "w" );

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( stricmp( attr, "select" )==0 ) strcpy( selectex, lineval );
	else if( stricmp( attr, "label" )==0 ) {
		strcpy( label, lineval );
		convertnl( label );
		}

	else if( stricmp( attr, "print" )==0 ) {
		strcpy( printstring, lineval );
		convertnl( printstring );
		}

	else if( stricmp( attr, "outfile" )==0 ) {
		strcpy( outfile, val );
		}
	else if( stricmp( attr, "outmode" )==0 ) {
		strcpy( outmode, val );
		}

	else Eerr( 1, "attribute not recognized", attr );
	}


if( printstring[0] != '\0' && Nrecords < 1 ) 
	Eerr( 17, "Warning: no data has been read yet w/ proc getdata", "" );



/* now do the work.. */
dontclose = 0;
if( outfile[0] != '\0' ) {
	outfp = fopen( outfile, outmode );
	if( outfp == NULL ) {
		Eerr( 7259, "cannot open outfile", outfile );
		outfp = PLS.diagfp;
		dontclose = 1;
		}
	}
else 	{
	outfp = PLS.diagfp;
	dontclose = 1;
	}

if( label[0] != '\0' ) fprintf( outfp, "%s\n", label );

nrecords = 0;
for( i = 0; i < Nrecords; i++ ) {
	do_select( selectex, i, &result );
	if( result == 1 ) {
		if( printstring[0] != '\0' ) {
			do_subst( buf, printstring, i, NORMAL );
			fprintf( outfp, "%s\n", buf );
			}
		nrecords++;
		}
	}

setintvar( "NSELECTED", nrecords );

if( !dontclose ) fclose( outfp );

return( 0 );
}
