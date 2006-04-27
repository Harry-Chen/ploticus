/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

#include <string.h>
#include "tdhkit.h"

/* This is similar to TDH_value_subst, but is stripped down and  accepts a data array that 
 * is an array of pointers rather than a 2d array.  It also knows about ploticus field 
 * names PL_fref()
 */

/* VALUE_SUBST - take a text line and substitute values for variables.  */

PL_value_subst( out, in, data, mode )
char *out; /* result buffer */
char *in;  /* input buffer */
char *data[ MAXITEMS ];
int mode;  /* either FOR_CONDEX (1), indicating that the line will be passed to condex(),
		   (minor hooks related to this) or NORMAL (0) */
{
int i, k, f;
char itemname[512];
char value[255];
int len;
int found;
int stat;
int infunction;
int ifld;
int inlen;
int outlen;
int vallen;
int inamelen;

found = 0;

infunction = 0;
inlen = strlen( in );
outlen = 0;
strcpy( out, "" );
for( i = 0; i < inlen; i++ ) {

	/* handle @@ (escape of @) */
	if( in[i] == '@' && in[i+1] == '@' ) {
		strcat( out, "@" );
		i ++;
		continue;
		}
		
	/* @item or @1 */
	if( in[i] == '@' ) {

		sscanf( &in[i+1], "%s", itemname );
		found = 1;

		/* truncate itemname at first char which is not alphanumeric or _ */
		inamelen = strlen( itemname );
		for( k = 0; k < inamelen; k++ ) {
			if( !isalnum( itemname[k] ) && itemname[k] != '_' ) { 
				itemname[k] = '\0';
				break;
				}
			}
		inamelen = strlen( itemname );

		/* @1, @2, etc... */
		ifld = atoi( itemname );
		if( ifld > 0 && ifld < MAXITEMS ) {
			strcpy( value, data[ ifld-1 ] );
			}

		/* @fieldname .. */
		else	{
			ifld = PL_fref( itemname );
			strcpy( value, data[ ifld -1 ] );
			}

		/* 
		 * else	{
		 *	stat = getvalue( value, itemname, data, recordid );
		 *	if( stat != 0 ) return( stat );
		 *	}
		 */

		/* special case of 0 length data item when in a condex expression but 
		   not within a function arg list.. to prevent condex syntax errors  */
		if( strcmp( value, "" )==0 && mode == FOR_CONDEX && !infunction )
			strcpy( value, "_null_" ); 


		/* strcat( out, value ); */
		/* strcpy( &out[outlen], value );  */
		vallen = strlen( value );
		for( k = 0; k < vallen; k++ ) {
			if( value[k] == ' ' && mode == FOR_CONDEX ) out[ outlen + k ] = '_';
			else out[ outlen + k ] = value[k];
			}
		out[ outlen + k ] = '\0';

		outlen += vallen;

		i+= inamelen; /* advance past @itemname */
		}

	else	{
		if( in[i] == '$' && isalpha( in[i+1] ) ) infunction = 1;
		if( isspace( in[i] ) ) infunction = 0;
		/* len = strlen( out ); */
		out[ outlen ] = in[i];
		out[ outlen +1 ] = '\0';
		outlen++;
		}
	}
return( found );
}
