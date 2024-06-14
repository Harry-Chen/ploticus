/* VARIABLE.C - assign/ get value of a variable
 * Copyright 1998-2002 Stephen C. Grubb  (ploticus.sourceforge.net) .
 * This code is covered under the GNU General Public License (GPL);
 * see the file ./Copyright for details. */

#include "tdhkit.h"

static int Ns = 0;
static char Name[MAXVAR][30];
static char Value[MAXVAR][VARMAXLEN+1];
static int badvarwarn = 0;

/* =================================== */
/* SETVAR - set the named variable to the given value.
	Allocates a space in the vars list if necessary.
 */
TDH_setvar( name, value )
char *name, *value;
{
int i;
for( i = 0; i < Ns; i++ ) if( strcmp( Name[i], name )==0 ) break;
if( i == Ns ) {
	/* if not found, allocate a space in vars list.. scg 2/15/01 */
	if( Ns >= MAXVAR ) return( err( 1324, "Sorry, var table full", name ) );
	strcpy( Name[Ns], name );
	Ns++;
	}
if( strlen( value ) > VARMAXLEN ) return( 1321 ); /* value too long */
strcpy( Value[i], value );
return( 0 );
}
/* =================================== */
/* GETVAR - get the value of the named variable */
TDH_getvar( name, value )
char *name, *value;
{
int i;
for( i = 0; i < Ns; i++ ) if( strcmp( Name[i], name )==0 ) break;
if( i == Ns ) {
	if( badvarwarn )  err( 1322, "Undeclared variable", name ); /* we don't always want a message */
	return( 1322 ); /* variable not declared */
	}
strcpy( value, Value[i] );
return( 0 );
}
/* =================================== */
/* GETVARP - return a pointer to the value of the named variable */
char *
TDH_getvarp( name )
char *name;
{
int i;
if( name == NULL ) return( NULL );
for( i = 0; i < Ns; i++ ) if( strcmp( Name[i], name )==0 ) break;
if( i == Ns ) return( NULL );
return( Value[i] );
}

/* =================================== */
/* CLEARALLVAR - clear the list of variables. */
TDH_clearallvar( )
{
Ns = 0;
return( 0 );
}

/* ===================================== */
/* WARN_ON_BADVAR - turn on warning when undeclared vars are encountered. */
TDH_warn_on_badvar()
{
badvarwarn = 1;
return( 0 );
}

