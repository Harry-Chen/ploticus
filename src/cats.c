/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* routines related to categories.. */

#include <stdlib.h>
#include "pl.h"


static char **cats[2] = { NULL, NULL };	  /* category list backbone (X, Y) */
static int ncats[2] = { 0, 0 };		  /* number of categories in list (X, Y) */
static int nextcat[2] = { 0, 0 };	  /* used by nextcat() for looping across categories (not widely used)*/
static int dont_init_ncats[2] = { 0, 0 }; /* for when items have been prepended */
static int catcompmethod[2] = { 0, 0 };	  /* category comparison method: 0 = contains   -1 = exact   >0 = compare 1st n characters */

static int sys_init[2] = { 0, 0 };	  /* 1 if category list malloced and ready to go.. */
static int req_ncats[2] = { MAXNCATS, MAXNCATS }; /* size of category lists */
static int check_uniq[2] = { 1, 1 };	  /* 1 = ensure unique cats   0 don't ensure uniqueness (faster, but dups will cause trouble) */
static int roundrobin[2] = { 1, 1 };	  /* used with roundrobin category lookup */
static int curcat[2] = { 0, 0 };	  /* used with roundrobin category lookup */


/* ================================================= */
/* CATFREE - free all malloced storage and initialize to original state.. */
PL_catfree()
{
int i, j;
for( i = 0; i < 2; i++ ) {
	if( sys_init[i] ) {
		for( j = 0; j < ncats[i]; j++ ) free( cats[i][j] );
		free( cats[i] );
		}
	}

/* these values must match the initializations at top */
cats[0] = NULL; cats[1] = NULL;
ncats[0] = 0; ncats[1] = 0;  
nextcat[0] = 0; nextcat[1] = 0;
dont_init_ncats[0] = 0; dont_init_ncats[1] = 0;
catcompmethod[0] = 0; catcompmethod[1] = 0;
sys_init[0] = 0; sys_init[1] = 0;
req_ncats[0] = MAXNCATS; req_ncats[1] = MAXNCATS;
check_uniq[0] = 1; check_uniq[1] = 1;
roundrobin[0] = 1; roundrobin[1] = 1;
curcat[0] = 0; curcat[1] = 0;

return( 0 );
}

/* ================================================= */
/* SETCATS - fill categories list */
PL_setcats( ax, bigbuf )
char ax;
char *bigbuf;
{
int df;
int axi, textloc;
int i, j;
char buf[200];
char fname[50];
int bigbuflen, buflen, ix, ixhold;
char fieldspec[80], selex[128];
int stat, result;
char *s, *t;
int tlen;


if( ax == 'x' ) axi = 0;
else axi = 1;

if( !sys_init[axi] ) {
	/* malloc the backbone - done only once per script */
	cats[axi] = (char **) malloc( req_ncats[axi] * sizeof( char *) );
	if( PLS.debug ) fprintf( PLS.diagfp, "categories in %c: list of size=%d malloced\n", ax, req_ncats[axi] );
	sys_init[axi] = 1;
	ncats[axi] = 0;
	}
else if( ncats[axi] > 0 && !dont_init_ncats[axi] ) {
	/* free malloced category labels */
	for( i = 0; i < ncats[axi]; i++ ) free( cats[axi][i] );
	}

strcpy( selex, "" );

if( strnicmp( bigbuf, "datafield", 9 )==0 ) {  /* fill cats list from a data field.. */

	if( Nrecords < 1 ) 
		return( Eerr( 3895, "cannot get categories from data field, no data has been read yet", "" ) );

	else	{
		ix = 0;

		/* datafield=xxxxx */  
		strcpy( fieldspec, GL_getok( bigbuf, &ix ) );
		if( GL_smember( fieldspec, "datafield datafields" )) /* handle old syntax 'datafield[s] xxx' */
			strcpy( fname, GL_getok( bigbuf, &ix ) ); 
		else strcpy( fname, &fieldspec[10] ); 

		/* optional selectrows=xxx xx xxx */ /* scg 2/28/02 */
		while( isspace( bigbuf[ix] ) && bigbuf[ix] != '\0' ) ix++ ;  /* advance */
		ixhold = ix;
		strcpy( buf, GL_getok( bigbuf, &ix ) );
		if( strnicmp( buf, "selectrows=", 11 )==0 ) strcpy( selex, &bigbuf[ixhold+11] );

		df = fref( fname );

		if( !dont_init_ncats[ axi ] ) ncats[ axi ] = 0;

		for( i = 0; i < Nrecords; i++ ) {

			if( selex[0] != '\0' ) { /* process against selection condition if any.. */
				stat = do_select( selex, i, &result );
				if( stat != 0 ) { Eerr( stat, "selectrows error", selex ); continue; }
                		if( result == 0 ) continue; /* reject */
				}


			t = da( i, df-1 );
			tlen = strlen( t );
			
			if( check_uniq[ axi ] ) {
				/* make sure we don't have it already.. */
				for( j = 0; j < ncats[ axi ]; j++ ) {
					if( stricmp( cats[ axi ][j], t ) ==0 ) break;
					}
				}
			else j = ncats[ axi ];

			if( j == ncats[ axi ] ) { 	/* only add it if not yet seen.. */

				if( ncats[ axi ] >= req_ncats[ axi ] ) 
					return( Eerr( 4824, "category list is full, some entries ignored (use proc categories to raise)", "" ));

				s = (char *) malloc( tlen+1 );
				cats[ axi ][ ncats[ axi ]] = s;
				strcpy( s, t );
				ncats[ axi ]++;
				}
			}
		}
	}

else	{		/* fill cats list from literal text chunk.. */
	if( !dont_init_ncats[ axi ] ) ncats[ axi ] = 0;

	textloc = 0;
	bigbuflen = strlen( bigbuf );
	while( 1 ) {
		if( textloc >= bigbuflen ) break;
		GL_getseg( buf, bigbuf, &textloc, "\n" );
		buflen = strlen( buf );

		if( ncats[ axi ] >= MAXNCATS )
			return( Eerr( 4825, "category list is full, some entries ignored (use proc categories to raise)", "" ));

		s = (char *) malloc( buflen+1 );
		cats[ axi ][ ncats[ axi ]] = s;
		strcpy( s, buf );
		ncats[ axi ]++;
		}
	}
dont_init_ncats[ axi ] = 0; /* for future go-rounds */
nextcat[ axi ] = 0;
curcat[ axi ] = 0;
return( 0 );
}

/* ======================================================= */
/* ADDCAT - prepend or append a category to the cat list */
/*          If prepend, this must be called before main cats list is set up */
PL_addcat( ax, pos, name )
char ax;  	/* 'x' or 'y' */
char *pos;	/* "pre" or "post" */
char *name;	/* category name */
{
int axi, buflen, i;
char *s;

if( ax == 'x' ) axi = 0;
else axi = 1;

buflen = strlen( name );

if( strcmp( pos, "pre" )==0 ) {
	if( !sys_init[axi] ) {
		cats[axi] = (char **) malloc( req_ncats[axi] * sizeof( char *) );
		sys_init[axi] = 1;
		ncats[axi] = 0;
		}
	if( ! dont_init_ncats[ axi ] ) ncats[ axi ] = 0;
	dont_init_ncats[ axi ] = 1;
	}
if( strcmp( pos, "post" )==0 ) dont_init_ncats[ axi ] = 0;

s = (char *) malloc( buflen+1 );
cats[ axi ][ ncats[ axi ]] = s;
strcpy( s, name );
ncats[ axi ]++;
return( 0 );
}


/* =============================================== */
/* NEXTCAT - for getting categories sequentially.. get next category in list.
      nextcat var maintains current position. */
PL_nextcat( ax, result, maxlen )
char ax;
char *result;
int maxlen;
{
int axi;
if( ax == 'x' ) axi = 0;
else axi = 1;

if( nextcat[ axi ] >= ncats[ axi ] ) { strcpy( result, "" ); return( 0 ); }

strncpy( result, cats[ axi ][ nextcat[ axi] ], maxlen );
result[maxlen] = '\0';
nextcat[ axi ]++;
return( 0 );
}

/* ================================================ */
/* GETCAT - get category name for slot n */
char *
PL_getcat( ax, n )
char ax;
int n;
{
int axi;
if( ax == 'x' ) axi = 0;
else axi = 1;
if( n >= ncats[ axi ] ) return( NULL );
return( cats[ axi ][ n ] );
}

/* ================================================ */
/* NCATS - return number of categories for an axis */
PL_ncats( ax )
char ax;
{
int axi;
if( ax == 'x' ) axi = 0;
else axi = 1;
return( ncats[ axi ] );
}


/* ================================================ */
/* FINDCAT - category look up.  Return slot (0 .. max) or -1 if not found */
/*    roundrobin search option is more efficient when categories will tend to be accessed in order */
PL_findcat( ax, s )
char ax, *s;
{
int axi, j, slen, recurs;
if( ax == 'x' ) axi = 0;
else axi = 1;

recurs = 0;
slen = strlen( s );

if( roundrobin[ axi ] ) {
	j = curcat[ axi ];
	if( j == -1 ) { recurs = 1; j = curcat[axi] = 0; }
	}
else j = 0;


/* contains */
if( catcompmethod[axi] == 0 ) { for( ; j < ncats[ axi ]; j++ ) { if( strnicmp( s, cats[axi][j], slen )==0 ) break; }}

/* exact */
else if( catcompmethod[axi] == -1 ) { for( ; j < ncats[ axi ]; j++ ) { if( stricmp( s, cats[axi][j] )==0 ) break; }}

/* specified length */
else if( catcompmethod[axi] > 0 ) { for( ; j < ncats[ axi ]; j++ ) { if( strnicmp( s, cats[axi][j], catcompmethod[axi] )==0 ) break; }}

if( j >= ncats[ axi ] ) {
	if( roundrobin[ axi ] && !recurs ) { 
		/* if working in round robin mode and we reach the end of the list, 
		   we need to search ONE more time from beginning of list.. */
		curcat[ axi ] = -1;
		return( PL_findcat( ax, s ) );
		}
	return( -1 );
	}
else 	{
	curcat[ axi ] = j;
	return( j );
	}
}

/* ================================================ */
/* SETCATPARMS - set the category comparison method */
PL_setcatparms( ax, what, parm )
char ax;
char *what;
int parm;
{
int axi;
if( ax == 'x' ) axi = 0;
else axi = 1;

if( strcmp( what, "compmethod" )==0 ) catcompmethod[axi] = parm;
else if( strcmp( what, "listsize" )==0 ) {
	if( sys_init[axi] ) return( Eerr( 2750, "categories already defined; listsize ignored", "" ));
	req_ncats[axi] = parm;
	}
else if( strcmp( what, "checkuniq" )==0 ) check_uniq[axi] = parm;
else if( strcmp( what, "roundrobin" )==0 ) roundrobin[axi] = parm;
return( 0 );
}

