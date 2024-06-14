#include "pl.h"
#include "tdhkit.h"

/* ============================= */
/* DO_SELECT - See if a row of data meets a condex selection condition. 
   Send back result of 1 if yes, 0 if no.
   Returns 0 if ok or an error code > 1 */
do_select( selectex, row, result )
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
for( i = 0; i < Nfields[Dsel]; i++ ) df[i] = da( row, i );

stat = plvalue_subst( buf, selectex, df, 1 );
*result = 0;
if( stat > 1 ) return( stat );
*result = TDH_condex( buf, 0 );
return( 0 );
}

/* ============================= */
/* DO_SUBST - do value_subst substitution on data fields references such as @1 */
do_subst( out, in, row )
char *out; /* result */
char *in;	/* template with variable references e.g. @1 */
int row;	/* row of data to access */
{
int stat;
char *df[MAXITEMS];
int i, j, len;
char *s;
 
/* copy data items into data array and change embedded blanks to underscores.. */
for( i = 0; i < Nfields[Dsel]; i++ ) df[i] = da( row, i );

/* do substitutions.. */
stat = plvalue_subst( out, in, df, 0 );
return( stat );
}
