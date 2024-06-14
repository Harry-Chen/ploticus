/* CONDEX.C
 * Copyright 1998-2002 Stephen C. Grubb  (ploticus.sourceforge.net) .
 * This code is covered under the GNU General Public License (GPL);
 * see the file ./Copyright for details. */

/* 
  This routine takes a conditional expression as a single string argument.
  If the expression is true, 1 is returned.
  If the expression is false, 0 is returned.
  If there was an error in the expression, -1 is returned.
*/
#include <stdio.h>
#define stricmp( s, t ) 	strcasecmp( s, t )
#define err(a,b,c) 		TDH_err(a,b,c)

#define MAXPARMLEN 1024

#define NUMBER 0
#define ALPHA 1

#define NCLAUSE 12
#define NTOKS 30
#define MAXTOK 256

static int evalclause(), evalstmt(), yield();
static char listsep = ',';
char *GL_getok();
static int evalflag = 0;

/* the following is necessary only for var evaluation */
extern char *TDH_dat, *TDH_recid;

/* ================================== */

TDH_condex( cond, eval )
char cond[];
int eval;  /* 1 = cond contains vars that need to be evaluated  0 = not */
{
int s[NCLAUSE], i, j, k, rtn, ii, ix, negate;
char args[NTOKS][MAXTOK], tok[MAXTOK];
int argc;
int ixtmp;
char nexttok[MAXTOK];
int typ, stat, p;
double atof();
int condlen;

evalflag = eval;

condlen = strlen( cond );


/* break cond into tokens */
ix = 0;
/* check for leading "not:" */
strcpy( tok, GL_getok( cond, &ix ) );
if( strcmp( tok, "not:" )==0 ) negate = 1;
else 	{
	negate = 0;
	ix = 0;
	}

for( i = 1; ; i++ ) {
	strcpy( args[ i ], GL_getok( cond, &ix ) );
	/* function may be multiple args - concatenate..*/
	if( args[i][0] == '$' ) {
		while( args[i][ strlen( args[i]) - 1 ] != ')' ) {
			if( ix >= condlen ) break;
			strcat( args[i], GL_getok( cond, &ix ));
			}
		}
	if( args[i][0] == '\0' ) break;
	}
argc = i;

/* for( i = 1; i < argc; i++ ) printf( "[%s]", args[i] );
 * printf( "\n" );
 */





/* do "clauses" */
for( i = 0; i < NCLAUSE; i++ ) s[i] = 0;
i = 0;
j = k = 1;
while( 1 ) {
	if( j==argc && GL_smember( args[j], "or ||" )) { 
		err( 1001, "expression error", cond );
		return( -1 ); 
		}
	if( j == argc || GL_smember( args[j], "or ||" )) {
		s[ i ] = evalclause( args, k, j-1 );
		if( s[ i ] == -1 ) {
			err( 1002, "expression error", cond );
			return( -1 );
			}
		k = j+1;
		i++;
		}
	j++;
	if( j > argc ) break;
	}
rtn = ( s[0] || s[1] || s[2] || s[3] || s[4] || s[5] || s[6] || s[7] || s[8] || s[9] || s[10] || s[11] );
if( negate ) return( ! rtn );
else return( rtn );
}

/* ================ */
/* EVALCLAUSE - evaluate a clause */
static int
evalclause( args, start, stop )
char args[NTOKS][MAXTOK];
int start, stop;
{
int s[NCLAUSE], i, j, k, rtn, ii;

for( i = 0; i < NCLAUSE; i++ ) s[i] = 1;
i = 0;
j = k = start;
while( 1 ) {
	if( j==stop && GL_smember( args[j], "and &&" )) {  return( -1 ); }

	if( j==stop || GL_smember( args[j], "and &&" )) {
		s[ i ] = evalstmt( args, k );
		if( s[ i ] == -1 ) return( -1 );
		k = j+1;
		i++;
		}
	j++;
	if( j > stop ) break;
	}
rtn = ( s[0] && s[1] && s[2] && s[3] && s[4] && s[5] && s[6] && s[7] && s[8] && s[9] && s[10] && s[11] );
return( rtn );
}

/* ============ */
/* EVALSTMT - evaluate a statement */
static int
evalstmt( args, start )
char args[NTOKS][MAXTOK];
int start;
{
char r1[MAXTOK], r2[MAXTOK], op[MAXTOK], val1[MAXTOK], val2[MAXTOK];
int i, t1, t2, i1, i2;
double diff;
double atof(), a2f();
int slen;


strcpy( r1, args[start] ); 
strcpy( op, args[start+1] );
strcpy( r2, args[start+2] );
if( GL_smember( r2, "and && or ||" ) || r2[0] == '\0' ) { return( -1 ); } /* got off track */


/* assign data type for value-- types are alpha, number, date */
i1 = yield( r1, &t1 );
i2 = yield( r2, &t2 );
if( i1 < 0 || i2 < 0 ) return( -1 );

if( t1 == NUMBER && t2 == NUMBER ) diff = atof( r1 ) - atof( r2 );
else diff = (double) strcmp( r1, r2 );
	
/* number compared against alpha always unequal */
if(( GL_smember( op, "ne !=" )) &&  ((t1 == NUMBER && t2 == ALPHA) || (t1 == ALPHA && t2 == NUMBER )) ) 
	return( 1 );

else if( GL_smember( op, "in" )) {
	for( i = 0, slen = strlen( r2 ); i < slen; i++ ) if( r2[i] == listsep ) r2[i] = ' ' ;
	return( GL_smember( r1, r2 ) );
	}
else if( GL_smember( op, "!in notin" )) {
	for( i = 0, slen = strlen( r2 ); i < slen; i++ ) if( r2[i] == listsep ) r2[i] = ' ' ;
	return( ! GL_smember( r1, r2 ) );
	}

else if( GL_smember( op, "like" )) return( ! GL_wildcmp( r1, r2, strlen( r2 ), 0 ) );
else if( GL_smember( op, "!like notlike" )) return( ! GL_slmember( r1, r2 ) );  /* scg 11/27/00 */

else if( GL_smember( op, "inlike" )) {
	for( i = 0, slen = strlen( r2 ); i < slen; i++ ) if( r2[i] == listsep ) r2[i] = ' ' ;
	return( GL_slmember( r1, r2 ) );
	}
else if( GL_smember( op, "!inlike notinlike" )) {
	for( i = 0, slen = strlen( r2 ); i < slen; i++ ) if( r2[i] == listsep ) r2[i] = ' ' ;
	return( ! GL_slmember( r1, r2 ) );
	}


/* for the rest of the relational operators, operands must be of same type for comparison .. */
else if( (t1 == NUMBER && t2 == ALPHA) || (t1 == ALPHA && t2 == NUMBER ) ) return( 0 );

else if( GL_smember( op, "eq is = ==" )) { if( diff == 0 ) return( 1 ); else return( 0 ); }
else if( GL_smember( op, "ne isnot != <>" )) { if( diff != 0 ) return( 1 ); else return( 0 ); }
else if( GL_smember( op, "gt >" )) { if( diff > 0 ) return( 1 ); else return( 0 ); }
else if( GL_smember( op, "ge >=" )) { if( diff >= 0 ) return( 1 ); else return( 0 ); }
else if( GL_smember( op, "lt <" )) { if( diff < 0 ) return( 1 ); else return( 0 ); }
else if( GL_smember( op, "le <=" )) { if( diff <= 0 ) return( 1 ); else return( 0 ); }

return( -1 );
}


/* ========================= */
/* YIELD - determine data type, evaluate functions.
   		Yield is a recursive function. 
   		Returns -1 for bad expression */
static int
yield( v, t )
char v[];
int *t;
{
int t2, i, i1, p;
double atof(), a2f();
int status;
char tok[256];

/* if evalflag, v likely contains one or more unevaluated vars */


*t = -1;

/* if v is a $function call, evaluate it .. */
if( v[0] == '$' && (isalpha( v[1] ) || v[1] == '$' ) ) {
#ifdef PSQLONLY
	status = 1;
#else
	status = TDH_function_call( v, t, evalflag ); /* v will be modified here */
#endif
	if( status != 0 ) { 
		err( 1003, "function error in condex", v ); 
		return( -1 ); 
		}
	}

/* variable.. evaluate it.. */
else if( v[0] == '@' && v[1] != '@' && evalflag ) {
	status = TDH_getvalue( tok, &v[1], TDH_dat, TDH_recid );
	if( status == 0 ) strcpy( v, tok );
	/* else @var appears verbatim */
	}

/* determine basic type of v */
if( *t >= 0 ) ;   /* already known from function */
else if( GL_goodnum( v, &p ) ) *t = NUMBER;
else *t = ALPHA;


return( 1 );
}


/* ============================== */
/* CONDEX_LISTSEP - allow setting of list delimiter character in case comma is unacceptable */
TDH_condex_listsep( c )
char c;
{
listsep = c;
return( 0 );
}
