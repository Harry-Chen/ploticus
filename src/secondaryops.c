/* SECONDARYOPS.C
 * Copyright 1998-2002 Stephen C. Grubb  (ploticus.sourceforge.net) .
 * This code is covered under the GNU General Public License (GPL);
 * see the file ./Copyright for details. */

/* secondaryops can be multi-line and are 
 * subordinate to flow-of-control operators.
 */
#include "tdhkit.h"
#define SHELL 0
#define SQL 1

static char bigbuf[ MAXRECORDLEN ];
static int bblen;

TDH_secondaryops( buf, ss, recordid, data )
char *buf;
struct sinterpstate *ss;
char *recordid;
char data[][DATAMAXLEN+1];
{
int i, j;
char tok[ DATAMAXLEN+1 ];
int ix;
int buflen;
int stat;
int len;

buflen = strlen( buf );

strcpy( tok, "" );
sscanf( buf, "%s", tok );

if( strcmp( tok, "#sql" )==0 || strcmp( tok, "#sql2" )==0 || strcmp( tok, "#shell" )==0 ) {  
	/* #sql [ load | processrows | dump | dumptab | dumphtml | dumpsilent] sql command [#endsql] */
	/* #shell [ #processrows | #dump | #dumptab | #dumphtml | #dumpsilent ] command [#endshell] */
	int ixhold, capmode, opmode;
	if( strncmp( tok, "#sql", 3 )==0 ) {
		opmode = SQL;
		capmode = 0;   /* if no #sql mode given, default is processrows */
		if( tok[4] == '2' ) ss->dbc = 1;
		else ss->dbc = 0;
		}
	else 	{
		opmode = SHELL;
		capmode = 'n';  /* if no #shell mode given, default is dump */
		}
	ix = 0;
	GL_getok( buf, &ix ); /* skip over 1st token..*/
	ixhold = ix;
	strcpy( tok, GL_getok( buf, &ix ) );  

	if( opmode == SQL && ( strcmp( tok, "load" )==0 || strcmp( tok, "#load" )==0 ) ) {
		capmode = 'l';
		ixhold = ix;
		strcpy( tok, GL_getok( buf, &ix ) );  
		}

	else if( ( opmode == SQL && strncmp( tok, "dump", 4 )==0 ) || strncmp( tok, "#dump", 5 )==0 ) {  
		if( tok[0] == '#' ) capmode = tok[5];
		else capmode = tok[4];
		if( capmode == '\0' ) capmode = 'n'; /* default is nosep */
		ixhold = ix;
		strcpy( tok, GL_getok( buf, &ix ) );  
		}

	else if( ( opmode == SQL && strcmp( tok, "processrows" )==0 )  || strcmp( tok, "#processrows" )==0 ) {
		capmode = 0;
		ixhold = ix;
		strcpy( tok, GL_getok( buf, &ix ) );  
		}
		 

	if( tok[0] != '\0' ) {
		strcpy( bigbuf, &buf[ixhold+1] ); /* 1 line command.. */

		if( opmode == SHELL ) {
			ss->doingshellresult = capmode;

			/* submit the shell command.. */
			stat = TDH_shellcommand( bigbuf );
			if( stat != 0 ) ss->doingshellresult = 0;
			}

		else if( opmode == SQL ) {
			ss->doingsqlresult = capmode;

			/* submit the sql.. */
			stat = TDH_sqlcommand( ss->dbc, bigbuf ); 
			if( stat != 0 ) ss->doingsqlresult = 0;

			/* save the return status.. */
			sprintf( tok, "%d", stat );
			TDH_setvar( "_STATUS", tok );
			}

		return( SINTERP_END ); /* no more calls to sec needed this time.. */
		}
		
	bblen = 0;
	while( 1 ) {
		stat = TDH_sinterp( buf, ss, recordid, data );
		if( stat != SINTERP_MORE ) return( stat );
		buflen = strlen( buf );
		strcpy( tok, "" );
		sscanf( buf, "%s", tok );

		if( strcmp( tok, "#endsql" )==0 ) {

			/* printf( "submitting sql:\n%s\n\n", bigbuf ); */
			ss->doingsqlresult = capmode;

			/* submit the sql.. */
			stat = TDH_sqlcommand( ss->dbc, bigbuf ); 
			if( stat != 0 ) ss->doingsqlresult = 0;

			/* save the return status.. */
			sprintf( tok, "%d", stat );
			TDH_setvar( "_STATUS", tok );
			return( SINTERP_END ); /* no more calls to sec needed this time */
			}

		else if( strcmp( tok, "#endshell" )==0 ) {
			ss->doingshellresult = capmode;

			/* submit the shell command.. */
			stat = TDH_shellcommand( bigbuf );
			if( stat != 0 ) ss->doingshellresult = 0;

			return( SINTERP_END );
			}


		else	{
			/* add more sql to bigbuf.. */
			if( bblen + buflen > (MAXRECORDLEN-2) ) return( err( 1263, "missing #endsql or #endshell", "" ) );
			strcpy( &bigbuf[bblen], buf );
			bblen += buflen;
			continue;
			}
		}
	}


else 	{
	/* other #ops will just be ignored.. control returns to the caller */
	return( SINTERP_END_BUT_PRINT );  /* no more calls to sec needed this time */
	}
}
