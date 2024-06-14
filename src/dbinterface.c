/* DBINTERFACE.C
 * Copyright 1998-2002 Stephen C. Grubb  (ploticus.sourceforge.net) .
 * This code is covered under the GNU General Public License (GPL);
 * see the file ./Copyright for details. */

/* TDH database interface.  

   Default database is pocketsql.  Interfaces to other databases should
   be implemented here.
   
   Two database connections are available; they are identified as 0 and 1.
   Connection 0 is the default.

   Suggested strategy for managing database connections:
     Implicitly initiate a connection upon first request.
     Database name, user name, and password should be set in the project
     config file as variables, eg  
	varvalue:	_DBDATABASE=test1
	varvalue:	_DBLOGIN=sys
	varvalue:	_DBPW=mongoose47

 */
#include "tdhkit.h"

#define PSQL  1
#define MYSQL  10
#define ORACLE  20
#define SYBASE  30
#ifndef TDH_DB
#define TDH_DB 0
#endif


#if TDH_DB == PSQL
#define MAXROWS 5000
static char *rows[2][MAXROWS];
static struct selectparms sp[2];
static int firsttime[2] = { 1, 1 };
#endif

static int rowcount[2];



/* =============================================================================
   SQLCOMMAND - submit an sql command and return its execution status (0 = normal). 
 */

TDH_sqlcommand( dbc, sql )
int dbc; 	/* connection identifier (0 or 1) */
char *sql; 	/* sql command */
{
int stat;

if( dbc < 0 || dbc > 1 ) return( err( 7945, "sqlrow: invalid db connection identifier", "" ) );

#if TDH_DB == 0
  return( err( 7949, "sql support not included in this build", "" ) );
#else
  TDH_dbnewquery( dbc ); /* notify $sqlrow() function of new query */
#endif


#if TDH_DB == PSQL
  if( !firsttime[ dbc ] ) SQ_free( rows[dbc], &sp[ dbc ] ); /* free results from previous retrieval */
  else 	{
	/* PSQL doesn't require connections, 
	 * but if we had to open a db connection, we could do that here like this..
	 *  char database[80], login[20], pw[20];
	 *  TDH_getvar( "_DBDATABASE", database );
	 *  TDH_getvar( "_DBLOGIN", login );
	 *  TDH_getvar( "_DBPW", pw );
	 *  opendatabase( database, login, pw );
	 */
	sp[ dbc ].nrows = 0;
	firsttime[ dbc ] = 0;
	}
  stat = SQ_command( sql );
  stat += SQ_fetchrows( rows[ dbc], MAXROWS, &sp[ dbc ] ); 
  rowcount[ dbc ] = SQ_rowcount(); /* preliminary */
  return( stat );
#endif

}

/* ========================================================================== 
   SQLROW - get one row of results from most recent SQL SELECT.
   Return 0 if row fetched, 1 if not (no more rows), or an error code. 
   All result fields should be character strings, including "null".
   FIELDS is an array of char pointers; each will point to a result field.
   N will be set to the number of fields.
*/

TDH_sqlrow( dbc, fields, n )
int dbc;
char *fields[];  /* size should be 256 */
int *n;
{
int i, stat;

if( dbc < 0 || dbc > 1 ) return( err( 7945, "sqlrow: invalid db connection identifier", "" ) );

#if TDH_DB == PSQL
  *n = sp[ dbc ].nitems;
  stat = SQ_row( fields, rows[ dbc ], &sp[ dbc ] );
  if( stat != 0 ) rowcount[ dbc ] = sp[ dbc ].finalrowcount;
  return( stat );
#endif

}

/* ========================================================================= 
   SQLNAMES - fetch names of result fields from most recent SQL SELECT.
   Return 0 or an error code.  
   FIELDS is an array of char pointers; each will point to a name.
   N will be set to the number of names (same as number of fields).
 */

TDH_sqlnames( dbc, fields, n )
int dbc;
char *fields[]; /* size should be 256 */
int *n;
{
int stat;
int i;
if( dbc < 0 || dbc > 1 ) return( err( 7945, "sqlrow: invalid db connection identifier", "" ) );

#if TDH_DB == PSQL
  for( i = 0; i < sp[ dbc ].nitems; i++ ) fields[i] = sp[ dbc ].itemlist[i];
  *n = sp[ dbc ].nitems;
  return( 0 );
#endif

}



/* ==========================================================================
   SQLROWCOUNT - return # of rows presented or affected by last sql command 
 */

TDH_sqlrowcount( dbc )
int dbc;
{
if( dbc < 0 || dbc > 1 ) return( err( 7945, "sqlrow: invalid db connection identifier", "" ) );

return( rowcount[ dbc ] );
}



/* ======== The following are convenience routines that call the above TDH routines.  ====== */

/* ============================= */
/* SQLGET - convenience routine to retrieve one field using default db connection.  
   Return 0 or an error code. */
TDH_sqlget( sql, result )
char *sql;
char *result;
{
int stat, n;
char *f[10];

strcpy( result, "" );

stat = TDH_sqlcommand( 0, sql );
if( stat != 0 ) return( stat );

stat = TDH_sqlrow( 0, f, &n );
if( stat != 0 ) return( stat );
if( n != 1 ) return( 5 );

strcpy( result, f[0] );
return( 0 );
}

/* =============================== */
/* SQLGETS - convenience routine to retrieve multiple fields.  Return 0 or an error code. */
TDH_sqlgets( sql, fields )
char *sql;
char *fields[];
{
int stat, n;

stat = TDH_sqlcommand( 0, sql );
if( stat != 0 ) return( stat );

stat = TDH_sqlrow( 0, fields, &n );
if( stat != 0 ) return( stat );

return( 0 );
}

