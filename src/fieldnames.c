/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


#include "tdhkit.h" /* for MAXITEMS */
#include "pl.h"
#define MAXNAMES MAXITEMS

static char fname[MAXNAMES][40];
static int nfname = 0;
static int errflag = 0;
static int showerr = 1;


/* ============================ */
PL_fieldnames_initstatic()
{
nfname = 0;
errflag = 0;
showerr = 1;
return( 0 );
}

/* ============================ */
/* DEFINEFIELDNAMES - define field names from a list 
   (space or comma-delimited).  Returns # of field names. */

PL_definefieldnames( list )
char *list;
{
int i, slen;


nfname = 0;

for( i = 0, slen = strlen( list ); i < slen; i++ ) {
	if( list[i] == ',' || list[i] == '\n' || list[i] == '"' ) list[i] = ' ';
	}

i = 0;
if( PLS.debug ) fprintf( PLS.diagfp, "Data field names are: " );
while( 1 ) {
	strcpy( fname[ nfname ], GL_getok( list, &i ) );
	if( fname[ nfname ][0] == '\0' ) break;
	if( PLS.debug ) fprintf( PLS.diagfp, "%s ", fname[nfname] ); 
	nfname++;
	}
if( PLS.debug ) fprintf( PLS.diagfp, "\n" );
return( nfname );
}

/* ============================ */
/* FREF - given a field name or number, return the field number (1st = 1) */
PL_fref( name )
char *name;
{
int i, fld, prec, stat;

fld = 0;
errflag = 0;

/* plain number.. */
i = atoi( name );

/* stat added scg 9/26/00 */
stat = GL_goodnum( name, &prec );

if( i > 0 && stat ) fld = i; 

/* @number.. */
else if( name[0] == '@' ) {
	stat = GL_goodnum( &name[1], &prec );
	i = atoi( &name[1] );
	if( i > 0 && stat ) fld = i;
	}

/* name.. */
else if( nfname > 0 ) {
	for( i = 0; i < nfname; i++ ) {
		if( strcmp( name, fname[i] ) == 0 ) {
			fld = i + 1;
			break;
			}
		}
	}

if( fld < 1 ) {  /* || fld > Nfields -- don't do this because Nfields might not be set yet
			if coming in from getdata select  -scg 10/7/00 */
	if( showerr ) Eerr( 2479, "No such data field", name );
	errflag = 1;
	return( 1 ); /* use 1 to be safe; return value is not checked by caller */
	}

return( fld );
}
/* ============================ */
/* GETFNAME - given a field number, return the field name assigned to
	that field (first is 1).
	Result will be "" if no field name has been assigned to field N. */

PL_getfname( n, result )
int n;
char *result;
{
if( n > nfname || n < 1 ) strcpy( result, "" );
else strcpy( result, fname[ n-1 ] );
return( 0 );
}

/* ============================= */
/* FREF_ERROR - get fref error flag */
PL_fref_error()
{
return( errflag );
}
/* ============================= */
/* FREF_SHOWERR - turn off/on "No such data field" message */
PL_fref_showerr( mode )
{
showerr = mode;
return( 0 );
}
