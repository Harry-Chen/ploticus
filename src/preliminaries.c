/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


#include "pl.h"
#include "tdhkit.h"
#ifndef PREFABS_DIR
#define PREFABS_DIR ""
#endif


#ifndef CONFIGFILE
#define CONFIGFILE ""
#endif

do_preliminaries()
{
char buf[512];
FILE *fp;
char *filename, *getenv();
char tag[80];
char value[512];
int valused, found;
int nt, stat, j;
int projectrootfound;
char pathslash;
char uniq[80];
char configfile[MAXPATH];



Errfp = stderr; 	/* portability? */
Diagfp = stderr;	/* portability? */
Bignumspacer = '\0'; 	/* use standard number notation */
Bignumspacer_thres = 4; 
suppress_convmsg( 1 ); /* suppress unplottable data msgs by default - user can turn on */
DT_checkdatelengths( 0 ); /* don't be strict about the length of date items */
setintvar( "CM_UNITS", 0 );
projectrootfound = 0;

strcpy( TDH_tmpdir, TMPDIR );
pathslash = PATH_SLASH;

/* set this now, but it might be updated depending on what's in config file.. */
GL_make_unique_string( uniq, 0 );
sprintf( Tmpname, "%s%cplo%s", TDH_tmpdir, pathslash, uniq ); 


/* reads and process config file, if any.. */
if( CGIargs != NULL ) {
	/* determine name of config file.. (can't use PLOTICUS_CONFIG in CGI context) */
	char *cgiprogname;
	strcpy( buf, "file=" );
	strcat( buf, CONFIGFILE );
	if( strlen( buf ) == 5 ) {   /* CONFIGFILE not set.. retrieve prog name from CGI environment and 
					 build config file name from that..  */
		cgiprogname = getenv( "SCRIPT_FILENAME" );
		if( cgiprogname == NULL ) {
			Eerr( 15063, "SCRIPT_FILENAME not found", "" );
			vermessage(); exit( 1 );
			}
		strcat( buf, cgiprogname );
		j = strlen( buf ) -4;
		if( strcmp( &buf[ j ], ".cgi" )==0 ) buf[ j ] = '\0';
		strcat( buf, ".cnf" );
		}
	strcpy( configfile, buf );
	}
else	{
	/* command line usage.. check PLOTICUS_CONFIG.. */
	filename = getenv( "PLOTICUS_CONFIG" );
	if( filename == NULL ) goto SKIPCONFIG;
	sprintf( configfile, "file=%s", filename );
	}

if( strlen( configfile ) == 5 ) {
	if( CGIargs != NULL ) {
		Eerr( 15061, "required ploticus config file not found", &configfile[5] );
		vermessage(); exit( 1 );
		}
	else goto SKIPCONFIG;  /* no config file given.. */
	}


stat = TDH_readconfig( configfile );
/* no point in checking return stat.. */


/* do this again because TDH_tmpdir might have been updated.. */
GL_make_unique_string( uniq, 0 );
sprintf( Tmpname, "%s%cplo%s", TDH_tmpdir, pathslash, uniq ); 

/* now read it again to get pl-specific items.. */

fp = fopen( &configfile[5], "r" );
if( fp == NULL ) {
	if( CGIargs != NULL ) {
		Eerr( 15061, "required ploticus config file not found", &configfile[5] );
		vermessage(); exit( 1 );
		}
	else Eerr( 15060, "Cannot open ploticus config file", &configfile[5] );
	return( 0 );
	}

/* get user settings.. */
while( fgets( buf, 511, fp ) != NULL ) {
	buf[ strlen( buf ) -1 ] = '\0';
	strcpy( value, "" );
	nt = sscanf( buf, "%s %s", tag, value );
	if( nt < 1 ) continue; /* skip blank lines */
	if( GL_member( tag[0], "#/" )) continue; /* skip comments of various kinds */


	if( strcmp( tag, "units:" )==0 ) {
		if( tolower( value[0] ) == 'c' ) { Using_cm = 1; setintvar( "CM_UNITS", 1 ); }
		else { Using_cm = 0; setintvar( "CM_UNITS", 0 ); }
		}

	else if( strcmp( tag, "projectroot:" )==0 ) {
		stat = chdir( value );
		if( stat != 0 ) {
			Eerr( 2704, "chdir projectroot failed", value );
			vermessage(); exit( 1 );
			}
		projectrootfound = 1;
		}

#ifndef WIN32
	else if( stricmp( tag, "uid:" )==0 ) setuid( atoi( value ) );
        else if( stricmp( tag, "gid:" )==0 ) setgid( atoi( value ) );
#endif
#ifndef NORLIMIT
	else if( stricmp( tag, "cpulimit:" )==0 ) reslimits( "cpu", atoi( value ) );
        else if( stricmp( tag, "filesizelimit:" )==0 ) reslimits( "filesize", atoi( value ) );
#endif

	else if( strcmp( tag, "option:" )==0 ) {
		value[0] = '\0';
		sscanf( buf, "%*s %s %s", tag, value );
		process_arg( tag, value, &valused, &found );	
		if( !found ) Eerr( 2784, "invalid option in config file", tag );
		}

	else if( strcmp( tag, "numbernotation:" )==0 ) {
		if( stricmp( value, "us" )==0 ) Bignumspacer = ',';
		else if( stricmp( value, "euro" )==0 ) Bignumspacer = '.';
		else Bignumspacer = '\0';
		}
	else if( strcmp( tag, "numberspacerthreshold:" )==0 ) Bignumspacer_thres = atoi( value ); /* scg 2/28/02 */
	else if( strcmp( tag, "pathslash:" )==0 ) sscanf( buf, "%*s %c", &pathslash );
		
	/* can't do this because there might be general tdh tags.. */
	/* else 	{ 
	 *	Eerr( 2749, "Unrecognized tag in config file", tag );
	 *	continue;
	 *	}
	 */
	}

fclose( fp );

SKIPCONFIG:
if( CGIargs != NULL && !projectrootfound ) {
	Eerr( 2705, "config file must set projectroot", "" );
	vermessage(); exit( 1 );
	}

/* get prefabs directory name if available.. */
/* this must come after config file is read, because in cgi mode PLOTICUS_PREFABS is set via config file. */
Prefabs_dir = getenv( "PLOTICUS_PREFABS" );
/* maybe PREFABS_DIR was set in the Makefile... */
#ifdef UNIX
if( Prefabs_dir == NULL ) Prefabs_dir = PREFABS_DIR ;
else if( Prefabs_dir[0] == '\0' ) Prefabs_dir = PREFABS_DIR ;
if( Prefabs_dir[0] == '\0' ) Prefabs_dir = NULL;
#endif
if( Prefabs_dir != NULL ) {
	setsid( Prefabs_dir ); /* set special include directory (#include $foo) */
        /* note: Prefabs_dir must reference static storage, either via getenv() or constant */
	}


return( 0 );
}

/* ========================= */
Ehandle_events( x, y, e )
double x, y;
int e;
{
if( e == E_EXPOSE || e == E_RESIZE ) Erestorewin();
return( 0 );
}
