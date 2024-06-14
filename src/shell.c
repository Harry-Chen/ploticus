/* SHELL.C  - script shell command interface
 * Copyright 1998-2002 Stephen C. Grubb  (ploticus.sourceforge.net) .
 * This code is covered under the GNU General Public License (GPL);
 * see the file ./Copyright for details. */


#include "tdhkit.h"
#define NL 0
#define WS 1
#define TAB 2

static FILE *shellfp = NULL;
static char *fn[MAXITEMS];
static char namebuf[SCRIPTLINELEN];
static int nfn = 0;
static int indelim = NL;
static int nrows = 0;

static int parsefields(), checkexit();


/* =================================== */
TDH_shellcommand( command )
char *command;
{
FILE *popen();
char *s;
int stat;

nfn = 0;
nrows = 0;

strcat( command, "\necho \"%-exitcode-% $?\" \n" );

shellfp = popen( command, "r" );
if( shellfp == NULL ) return( 1 );

return( 0 );
}

/* =================================== */
TDH_shellreadheader( )
{
int i, stat;
char *s;

s = fgets( namebuf, SCRIPTLINELEN-1, shellfp );

stat = checkexit( s, namebuf );
if( stat != 0 ) return( stat );

parsefields( namebuf, fn, &nfn );
return( 0 );
}

/* =================================== */
TDH_shellresultrow( buf, fields, nfields, maxlen )
char *buf;
char *fields[MAXITEMS];
int *nfields;
int maxlen;
{
char *s;
int stat;

if( shellfp == NULL ) return( 1 );

s = fgets( buf, maxlen, shellfp );

stat = checkexit( s, buf );
if( stat != 0 ) return( stat );

nrows++;

parsefields( buf, fields, nfields );

return( 0 );
}

/* ===================================== */
TDH_shellclose()
{
if( shellfp == NULL ) return( 1 );
pclose( shellfp ); shellfp = NULL;
return( 0 );
}

/* ===================================== */
static int
checkexit( s, buf )
char *s; /* return from an fgets */
char *buf;
{
char exitcode[80];
if( s == NULL ) {
	pclose( shellfp ); shellfp = NULL;
	return( 1 );
	}
if( strncmp( buf, "%-exitcode-% ", 13 )==0 ) {
	sscanf( buf, "%*s %s", exitcode );
	TDH_setvar( "_STATUS", exitcode );
	pclose( shellfp ); shellfp = NULL;
	return( 1 );
	}
return( 0 );
}

/* =================================== */
static int
parsefields( buf, f, nf )
char *buf;
char *f[MAXITEMS];
int *nf;
{
int ix, i, j, len, startfld, sp;

ix = 0; i = 0;
len = strlen( buf );
startfld = 1;
for( i = 0, j = 0; i < len; i++ ) {
	sp = 0;
	if( indelim == TAB && (buf[i] == '\t' || buf[i] == '\n' ) ) sp = 1;
	else if( indelim == NL && buf[i] == '\n' ) sp = 1;
	else if( indelim == WS ) sp = isspace( buf[i] );
	if( !sp && startfld ) {
		f[j++] = &buf[i];
		startfld = 0;
		}
	if( sp ) {
		buf[i] = '\0';
		startfld = 1;
		}
	}
*nf = j;
return( 0 );
}


/* ========================================== */
/* SHFUNCTIONS - TDH script access to shell commands */

TDH_shfunctions( hash, name, arg, nargs, result, typ, data, recordid )
int hash;
char *name;
char *arg[];
int nargs;
char *result;
int *typ;
char data[][DATAMAXLEN+1];
char *recordid;
{
char *f[MAXITEMS];
char fname[50];
char buf[MAXRECORDLEN];
int i, j, n, len, stat;

*typ = 0; /* numeric */


if( hash == 1006 ) { /* $shellrow() - fetch a row of results.  
 		      * Return 0 = normal, 1 = no row fetched and no more results, >1 = error. */

	/* get next result row.. */
	NEXTROW:
	if( nargs > 1 ) indelim = WS; /* more than one var given.. guess whitespace delim */
        stat = TDH_shellresultrow( buf, f, &n, SCRIPTLINELEN );
        if( stat == 0 ) {
		if( nfn > 0 ) { /* names already defined in a header.. */
			for( i = 0; i < nfn; i++ ) TDH_setvalue( fn[i], f[i], data, recordid );
			}
		else if( nargs > 0 && strcmp( arg[0], "#varvaluepair" )==0 ) { /* tag-value pair */
			strcpy( fname, f[0] );
			len = strlen( fname );
			if( len == 0 ) goto NEXTROW; /* blank name.. skip.. */
			if( fname[ len -1 ] == ':' ) fname[ len-1] = '\0';
			TDH_setvalue( fname, f[1], data, recordid );
			} 
		else if( nargs > 0 ) { /* names given as function arguments */
			for( i = 0; i < nargs; i++ ) {
				if( i >= n ) TDH_setvalue( arg[i], "", data, recordid );
				else TDH_setvalue( arg[i], f[i], data, recordid );
				}
			}
		else 	{    /* error - no name(s) specified */
			strcpy( result, "5690" );
			return( 0 );
			}
                }
        else for( i = 0; i < nfn; i++ ) TDH_setvalue( fn[i], "", data, recordid );

	/* check return status.. non-zero indicates no more rows.. */
	sprintf( result, "%d", stat );
	return( 0 );
	}

if( hash == 2569 ) { /* shellrowcount() - return number of rows presented or affected by last sql command. */
	sprintf( result, "%d", nrows );
	return( 0 );
	}

if( hash == 3084 ) { /* $shellstripchars( chars, varname1 .. varnamen ) - remove characters that could be dangerous
		      in shell commands.  Chars arg may be omitted to use a standard set of characters.  */

	int start;
	if( isalpha( arg[0][0] )) start = 0;
	else start = 1;
	for( i = start; i < nargs; i++ ) {
        	stat = TDH_getvalue( buf, arg[i], data, recordid );
		if( start == 0 ) GL_deletechars( "\"'`$\\;", buf );
        	else GL_deletechars( arg[0], buf );
        	stat = TDH_setvalue( arg[i], buf, data, recordid );
		}
	sprintf( result, "0" );
	return( 0 );
	}

if( hash == 2554 ) { /* $shellreadheader() - load field name header */
	stat = TDH_shellreadheader();
	sprintf( result, "%d", stat );
	return( 0 );
	}


if( hash == 2686 ) { /* $shellfielddelim() */
	if( arg[0][0] == 'w' ) indelim = WS;
	else if( arg[0][0] == 't' ) indelim = TAB;
	else indelim = NL;
	sprintf( result, "0" );
	return( 0 );
	}

return( err( 197, "unrecognized function", name )); /* not found */


}
