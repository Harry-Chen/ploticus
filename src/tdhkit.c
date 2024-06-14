/* TDHKIT.C - read config file; global TDH vars 
 * Copyright 1998-2002 Stephen C. Grubb  (ploticus.sourceforge.net) .
 * This code is covered under the GNU General Public License (GPL);
 * see the file ./Copyright for details. */

/* compile defines:  
	TDH_NOREC = omit FDF code   
	TDH_DB = select sql database (see dbinterface.c) (undefined if no sql database)
*/

   
#include "tdhkit.h"

char TDH_scriptdir[ MAXPATH] = "./";	/* root directory for scripts */
char TDH_configfile[ MAXPATH ] = "";	/* path name of config file */
char TDH_tmpdir[ MAXPATH ] = "/tmp";	/* directory for tmp files */
char TDH_dbnull[ 40 ] = "null";		/* external code for empty data item */
int TDH_debugflag = 0;			/* may be set to 1 for extra debugging output */
char TDH_utilfieldterm = ' ';		/* default field sep for utility output */
char TDH_decpt = '.';			/* decimal point char */
char TDH_dbquote = '"';			/* quote character for use in database statements */
char TDH_dbquoteesc = '\\';		/* character to insert before embedded dbquote in db statements */
int TDH_htmlqh = 1;			/* HTML quote handling mode */

char TDH_optionstring[80] = "";		/* various options list */
char *TDH_dat = NULL;			/* points to data array for condex */
char *TDH_recid = NULL;			/* points to recordid for condex */
char TDH_progname[20] = "";
int TDH_initialized = 0;

#ifndef TDH_NOREC
char TDH_bin[ MAXPATH ] = "";			
char TDH_tmptbldir[ MAXPATH ] = "";	
char TDH_fdfpath[ MAXPATH ] =      "./";	/* directory where FDF files are kept */
#endif


#define MAXPE 2048
static char putenvstring[MAXPE+2];
static int pelen = 0;




TDH_readconfig( loc )
char *loc; /* environment var or file=filename */
{
FILE *fp;
char buf[512];
char tag[80];
char value[512];
int nt;
int stat;
char *filename, *getenv();
int slen;


if( strncmp( loc, "file=", 5 )==0 ) strcpy( TDH_configfile, &loc[5] );
else	{
	strcpy( TDH_configfile, getenv( loc ) );
	if( TDH_configfile == NULL ) return( 1 ); /* no env var specified - ok */
	if( TDH_configfile[0] == '\0' ) return( 1 ); /* env var empty - ok */
	}
fp = fopen( TDH_configfile, "r" );
if( fp == NULL ) {
	return( err( 1250, "Cannot open config file", TDH_configfile ) );
	}

strcpy( TDH_optionstring, "" );

/* get user settings.. */
while( fgets( value, 511, fp ) != NULL ) {

	TDH_value_subst( buf, value, NULL, "", NORMAL, 1 ); /* evaluate variables */

	buf[ strlen( buf ) -1 ] = '\0';
	nt = sscanf( buf, "%s %s", tag, value );
	if( nt < 1 ) continue; /* skip blank lines */
	if( tag[0] == '/' ) continue; /* skip comments */

	if( nt < 2 ) strcpy( value, "" );

	if( stricmp( tag, "scriptdir:" )==0 ) strcpy( TDH_scriptdir, value );
#ifndef TDH_NOREC
	else if( stricmp( tag, "fdfpath:" )==0 ) strcpy( TDH_fdfpath, value );
	else if( stricmp( tag, "tmpdir:" )==0 ) {
		strcpy( TDH_tmpdir, value );  
		if( TDH_tmptbldir[0] == '\0' ) strcpy( TDH_tmptbldir, value );
		}
	else if( stricmp( tag, "tdhbin:" )==0 ) sprintf( TDH_bin, "%s%c", value, PATH_SLASH );
	else if( stricmp( tag, "joinprog:" )==0 ) {
		value[ strlen( value ) - 6 ] = '\0';
		sprintf( TDH_bin, "%s%c", value, PATH_SLASH );
		}
	else if( stricmp( tag, "dbtemptabledir:" )==0 ) strcpy( TDH_tmptbldir, value );
#endif
	else if( stricmp( tag, "dbnull:" )==0 ) strcpy( TDH_dbnull, value ); 
	else if( stricmp( tag, "dbquote:" )==0 ) TDH_dbquote = value[0];
	else if( stricmp( tag, "dbquoteesq:" )==0 ) TDH_dbquoteesc = value[0];
	else if( stricmp( tag, "htmlqmode" )==0 ) {
		if( tolower(value[0]) == 'y' ) TDH_htmlqh = 1;
		else TDH_htmlqh = 0;
		}
	else if( stricmp( tag, "fieldnameheaders:" )==0 ) {
		if( tolower(value[0]) == 'y' ) strcat( TDH_optionstring, "headermode " );
		}
	else if( stricmp( tag, "tabfields:" )==0 ) {
		if( tolower(value[0]) == 'y' ) strcat( TDH_optionstring, "tabfields " );
		TDH_utilfieldterm = '\t';
		}

	else if( stricmp( tag, "varvalue:" )==0 ) {   
		int i, tt;
		char var[40], val[255];
		for( i = 0, slen = strlen( value ); i < slen; i++ ) {
			if( value[i] == '=' ) {
				value[i] = ' ';
				break;
				}
			}
		tt = sscanf( value, "%s %s", var, val );
		if( tt == 1 ) strcpy( val, "" );
		stat = TDH_setvar( var, val );
		if( stat != 0 ) return( stat );
		}
	else if( stricmp( tag, "putenv:" )==0 ) {
		/* cannot use automatic storage for putenv */
		if( pelen + strlen( value ) > MAXPE )
			err( 1251, "tdhconfig: too many putenv entries", "" );
		else	{
			strcpy( &putenvstring[ pelen ], value );
			putenv( &putenvstring[ pelen ] );
			pelen += strlen( value ) + 1;
			}
		}

	else if( stricmp( tag, "utilfieldterm:" )==0 ) {		  
		if( stricmp( value, "tab" )==0 ) TDH_utilfieldterm = '\t';
		else if( stricmp( value, "space" )==0 ) TDH_utilfieldterm = ' ';
		else TDH_utilfieldterm = value[0];
		}

	else if( stricmp( tag, "decpt:" )==0 ) TDH_decpt = value[0];

#ifndef PSQLONLY
        else if( GL_slmember( tag, "dateformat: pivotyear: months* weekdays: omitweekends: lazydates:" )) {
		int ix;
		ix = 0;
		GL_getok( buf, &ix ); /* skip tag.. */
		while( isspace(buf[ix] )) ix++;
                stat = DT_setdateparms( tag, &buf[ix] );
                if( stat != 0 ) return( err( stat, "date parm error in config file", buf ) );
                }
#endif
	}

fclose( fp ); fp = NULL;
TDH_initialized = 1;
return( 0 );
}

