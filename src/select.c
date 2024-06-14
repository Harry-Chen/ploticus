#include "pl.h"
#include "tdhkit.h"

/* ============================= */
/* DO_SELECT - See if a row of data meets a condex selection condition. 
   Send back result of 1 if yes, 0 if no.
   Returns 0 if ok or an error code > 1 */
PL_do_select( selectex, row, result )
char *selectex;
int row;
int *result;
{
int stat;
char recordid[10];
int i, j, len;
char *s;
char buf[256];
char *df[MAXITEMS];

 
strcpy( recordid, "" ); /* not used */
 
/* set up pointers to data items.. */
for( i = 0; i < Nfields; i++ ) df[i] = da( row, i );

stat = PL_value_subst( buf, selectex, df, 1 );
*result = 0;
if( stat > 1 ) return( stat );
*result = TDH_condex( buf, 0 );
return( 0 );
}

/* ============================= */
/* DO_SUBST - do value_subst substitution on data fields references such as @1 */
PL_do_subst( out, in, row, mode )
char *out; /* result */
char *in;	/* template with variable references e.g. @1 */
int row;	/* row of data to access */
int mode;	/* either FOR_CONDEX (1) or NORMAL (0) */
{
int stat;
char *df[MAXITEMS];
int i, j, len;
char *s;

if( in[0] == '\0' ) {
	out[0] = '\0';
	return( 0 );
	}
 
/* set pointers to data items.. */
for( i = 0; i < Nfields; i++ ) df[i] = da( row, i );

/* do substitutions.. */
stat = PL_value_subst( out, in, df, mode );
return( stat );
}
