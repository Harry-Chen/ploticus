/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


/* routines related to categories.. */
/* scg 12/6/00 - vectorized curcat
   scg 12/6/00 - added addcat and dont_init_ncats
 */
#include "pl.h"

static int curcat[2] = { 0, 0 };
static int dont_init_ncats[2] = { 0, 0 };

/* --------------------------------------------- */
/* SETCATS - fill categories list */
setcats( ax, bigbuf )
char ax;
char *bigbuf;
{
int df;
int axsel, textloc;
int i, j;
char buf[200];
char fname[50];
int buflen, ix, ixhold;
char fieldspec[80], selex[128];
int stat, result;


if( ax == 'x' ) axsel = 0;
else axsel = 1;

strcpy( selex, "" );

if( strnicmp( bigbuf, "datafield", 9 )==0 ) {  /* fill cats list from a data field.. */

	if( Nrecords[Dsel] < 1 ) 
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
		if( !dont_init_ncats[axsel] ) Ncats[axsel] = 0;

		for( i = 0; i < Nrecords[Dsel]; i++ ) {

			if( selex[0] != '\0' ) { /* process against selection condition if any.. */
				stat = do_select( selex, i, &result );
				if( stat != 0 ) { Eerr( stat, "selectrows error", selex ); continue; }
                		if( result == 0 ) continue; /* reject */
				}

			strcpy( buf, da( i, df-1 ) );
			/* first make sure we don't have it already.. */
			for( j = 0; j < Ncats[axsel]; j++ ) {
				if( stricmp( Cats[axsel][j], buf ) ==0 ) break;
				}
			if( j == Ncats[axsel] ) { /* only add it if not yet seen.. */
				strncpy( Cats[axsel][Ncats[axsel]], buf, MAXCATLEN-1 );
				Cats[axsel][Ncats[axsel]][MAXCATLEN] = '\0';
				Ncats[axsel]++;
				}
			}
		}
	}

else	{		/* fill cats list from literal text chunk.. */
	if( !dont_init_ncats[axsel] ) Ncats[axsel] = 0;
	textloc = 0;
	buflen = strlen( bigbuf );
	while( 1 ) {
		if( textloc >= buflen ) break;
		GL_getseg( buf, bigbuf, &textloc, "\n" );
		strncpy( Cats[axsel][Ncats[axsel]], buf, MAXCATLEN-1 );
		Cats[axsel][Ncats[axsel]][MAXCATLEN] = '\0';
		Ncats[axsel]++;
		}
	}
dont_init_ncats[axsel] = 0; /* for future go-rounds */
curcat[axsel] = 0;
return( 0 );
}

/* --------------------------------------------- */
/* ADDCAT - prepend or append a category to the cat list */
/*          If prepend, this must be called before main cats list is set up */
addcat( ax, pos, name )
char ax;  	/* 'x' or 'y' */
char *pos;	/* "pre" or "post" */
char *name;	/* category name */
{
int axsel;

if( ax == 'x' ) axsel = 0;
else axsel = 1;

if( strcmp( pos, "pre" )==0 ) {
	if( ! dont_init_ncats[axsel] ) Ncats[axsel] = 0;
	dont_init_ncats[axsel] = 1;
	}
if( strcmp( pos, "post" )==0 ) dont_init_ncats[axsel] = 0;
strncpy( Cats[axsel][Ncats[axsel]], name, MAXCATLEN-1 );
Cats[axsel][Ncats[axsel]][MAXCATLEN] = '\0';
Ncats[axsel]++;
return( 0 );
}



/* --------------------------------------------- */
/* NEXTCAT - for getting categories sequentially.. get next category in list.
      curcat var maintains current position. */
nextcat( ax, result, maxlen )
char ax;
char *result;
int maxlen;
{
int axsel;
if( ax == 'x' ) axsel = 0;
else axsel = 1;

if( curcat[axsel] >= Ncats[axsel] ) {
	strcpy( result, "" );
	return( 0 );
	}
strncpy( result, Cats[axsel][ curcat[axsel] ], maxlen );
result[maxlen] = '\0';
curcat[axsel]++;
return( 0 );
}
