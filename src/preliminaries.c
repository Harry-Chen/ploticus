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

#ifdef LOCALE
#include <locale.h>
#endif


/* =========================================== */
/* DO_PRELIMINARIES - set defaults, read config file, etc. */

PL_do_preliminaries()
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
char cgierrfile[80];


TDH_errprog( "pl" );


/* set pre-config (hard-coded) defaults.. */
PLS.debug = 0;
PLS.echolines = 0;
PLS.skipout = 0;
PLS.eready = 0;
PLS.prefabsdir = NULL;
strcpy( PLS.outfile, "" );
PLS.winx = 100; PLS.winy = 0; PLS.winw = 8.0; PLS.winh = 8.0;
PLS.winsizegiven = 0;
PLS.bkcolorgiven = 0;
PLS.clickmap = 0;
PLS.usingcm = 0;
strcpy( PLS.viewer, "" );
strcpy( PLS.mapfile, "" );
#ifndef NORLIMIT
TDH_reslimits( "cpu", CPULIMIT );
#endif
#ifdef LOCALE
 setlocale(LC_CTYPE, "" );
 setlocale(LC_COLLATE, "" );
#endif

#ifdef NOX11
PLS.device = 'e';
#else
PLS.device = 'x';    
#endif

PLD.currow = 0;
PLD.curdf = 0;
PLD.curds = -1;
PLD.maxrows = MAXDROWS;
PLD.maxdf = MAXD;
PLL.maxproclines = MAXPROCLINES;
PLVsize = MAXDAT;

PLG_set_early_defaults();

PLS.errfp = stderr; 	/* portability? */
PLS.diagfp = stderr;	/* portability? */
PLS.bignumspacer = '\0'; 	/* use standard number notation */
PLS.bignumthres = 4; 
suppress_convmsg( 1 ); /* suppress unplottable data msgs by default - user can turn on */
DT_checkdatelengths( 0 ); /* don't be strict about the length of date items */
setintvar( "CM_UNITS", 0 );
projectrootfound = 0;

strcpy( TDH_tmpdir, TMPDIR );
pathslash = PATH_SLASH;

/* set this now, but it might be updated depending on what's in config file.. */
GL_make_unique_string( uniq, 0 );
sprintf( PLS.tmpname, "%s%cplo%s", TDH_tmpdir, pathslash, uniq ); 

/* make cgierrfile (default /tmp/cplcgi_err) for cgi errors */
sprintf( cgierrfile, "%s%cplcgi_err", TDH_tmpdir, PATH_SLASH );




/* reads and process config file, if any.. */
if( PLS.cgiargs != NULL ) {
	/* determine name of config file.. (can't use PLOTICUS_CONFIG in CGI context) */
	char *cgiprogname;
	strcpy( buf, "file=" );
	strcat( buf, CONFIGFILE );
	if( strlen( buf ) == 5 ) {   /* CONFIGFILE not set.. retrieve prog name from CGI environment and 
					 build config file name from that..  */
		cgiprogname = getenv( "SCRIPT_FILENAME" );
		if( cgiprogname == NULL ) {
			PLS.errfp = fopen( cgierrfile, "w" );
			if( PLS.errfp != NULL ) {
				fprintf( PLS.errfp, "cgi var SCRIPT_FILENAME not found.\n" );
#ifdef UNIX
				fchmod( fileno( PLS.errfp ), 00666 );
#endif
				}
			return( 1 );
			}
		strcat( buf, cgiprogname );
		j = strlen( buf ) -4;
		if( strcmp( &buf[ j ], ".cgi" )==0 ) buf[ j ] = '\0';
		else if( strcmp( &buf[ j ], ".exe" )==0 ) buf[ j ] = '\0';
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
	if( PLS.cgiargs != NULL ) goto BAD_CGI_CONFIG;
	else goto SKIPCONFIG;  /* no config file given.. */
	}


stat = TDH_readconfig( configfile );
/* no point in checking return stat.. */


/* do this again because TDH_tmpdir might have been updated.. */
GL_make_unique_string( uniq, 0 );
sprintf( PLS.tmpname, "%s%cplo%s", TDH_tmpdir, pathslash, uniq ); 

/* now read it again to get pl-specific items.. */

fp = fopen( &configfile[5], "r" );
if( fp == NULL ) {
	if( PLS.cgiargs != NULL ) {
		BAD_CGI_CONFIG:
		PLS.errfp = fopen( cgierrfile, "w" );
		if( PLS.errfp != NULL ) {
			fprintf( PLS.errfp, "cgi mode: cannot open config file (%s).\n", &configfile[5] );
#ifdef UNIX
			fchmod( fileno( PLS.errfp ), 00666 );
#endif
			}
		return( 1 );
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
		if( tolower( value[0] ) == 'c' ) { PLS.usingcm = 1; setintvar( "CM_UNITS", 1 ); }
		else { PLS.usingcm = 0; setintvar( "CM_UNITS", 0 ); }
		}

	else if( strcmp( tag, "projectroot:" )==0 ) {
		stat = chdir( value );
		if( stat != 0 ) goto CGI_BAD_CHDIR;
		projectrootfound = 1;
		}

#ifndef NORLIMIT
	else if( stricmp( tag, "cpulimit:" )==0 ) TDH_reslimits( "cpu", atoi( value ) );
#endif

	else if( strcmp( tag, "option:" )==0 ) {
		value[0] = '\0';
		sscanf( buf, "%*s %s %s", tag, value );
		PL_process_arg( tag, value, &valused, &found );	
		if( !found ) Eerr( 2784, "invalid option in config file", tag );
		}

	else if( strcmp( tag, "numbernotation:" )==0 ) {
		if( stricmp( value, "us" )==0 ) PLS.bignumspacer = ',';
		else if( stricmp( value, "euro" )==0 ) PLS.bignumspacer = '.';
		else PLS.bignumspacer = '\0';
		}
	else if( strcmp( tag, "numberspacerthreshold:" )==0 ) PLS.bignumthres = atoi( value ); /* scg 2/28/02 */

#ifndef NOSVG
	else if( stricmp( tag, "xml_encoding:" )==0 ) PLGS_setxmlparms( "encoding", value );
        else if( stricmp( tag, "xml_declaration:" )==0 ) {
                if( stricmp( value, YESANS )==0 ) PLGS_setxmlparms( "xmldecl", "1" );
                else PLGS_setxmlparms( "xmldecl", "0" );
                }
        else if( stricmp( tag, "svg_tagparms:" )==0 ) {
		int ix;
		ix = 0;
		GL_getok( buf, &ix );
		PLGS_setxmlparms( "svgparms", &buf[ix] );
		}
#endif

	else if( stricmp( tag, "dtsep" )==0 ) DT_setdtsep( value[0] ); 
		
	/* can't do this because there might be general tdh tags.. */
	/* else 	{ 
	 *	Eerr( 2749, "Unrecognized tag in config file", tag );
	 *	continue;
	 *	}
	 */
	}

fclose( fp );

SKIPCONFIG:
if( PLS.cgiargs != NULL && !projectrootfound ) {
	CGI_BAD_CHDIR:
	PLS.errfp = fopen( cgierrfile, "w" );
	if( PLS.errfp != NULL ) {
		fprintf( PLS.errfp, "cgi mode: no projectroot in config file, or could not cd to projectroot dir.\n" );
#ifdef UNIX
		fchmod( fileno( PLS.errfp ), 00666 );
#endif
		}
	return( 1 );
	}

/* get prefabs directory name if available.. */
/* this must come after config file is read, because in cgi mode PLOTICUS_PREFABS is set via config file. */
PLS.prefabsdir = getenv( "PLOTICUS_PREFABS" );
/* maybe PREFABS_DIR was set in the Makefile... */
#ifdef UNIX
if( PLS.prefabsdir == NULL ) PLS.prefabsdir = PREFABS_DIR ;
else if( PLS.prefabsdir[0] == '\0' ) PLS.prefabsdir = PREFABS_DIR ;
if( PLS.prefabsdir[0] == '\0' ) PLS.prefabsdir = NULL;
#endif
if( PLS.prefabsdir != NULL ) {
	setsid( PLS.prefabsdir ); /* set special include directory (#include $foo) */
        /* note: prefabsdir must reference static storage, either via getenv() or constant */
	}


return( 0 );
}


/* ====================================== */
/* BEGIN - initializations that are done AFTER config file and args are looked at.. */

PL_begin()
{
char buf[128];

/* graphcore initializations.. */
Esetsize( PLS.winw, PLS.winh, PLS.winx, PLS.winy );
EDXlo = 0.0; EDXhi = 0.0; EDYlo = 0.0; EDYhi = 0.0;
PLS.eready = 0;

#ifndef NOSVG
if( PLS.device == 's' ) PLGS_setparms( PLS.debug, PLS.tmpname, PLS.clickmap ); 
#endif
#ifndef NOSWF
if( PLS.device == 'f' ) PLGF_setparms( PLS.debug, PLS.tmpname, Estandard_font );  /* pass user selected -font if any */
#endif


/* initialize the data structures.. */
PL_init_mem();

if( PLS.debug ) {
        fprintf( PLS.diagfp, "Version: pl %s\n", PLVERSION );
        if( PLS.cgiargs != NULL ) fprintf( PLS.diagfp, "operating in CGI mode\n" );
        Epcodedebug( 1, PLS.diagfp ); /* tell pcode.c to output diagnostics too */
        }

/* set PLVERSION variable.. */
sprintf( buf, "ploticus %s http://ploticus.sourceforge.net (GPL)", PLVERSION );
TDH_setvar( "PLVERSION", buf );

if( PLS.clickmap ) {  /* .map filename */
	if( PLS.mapfile[0] == '\0' ) {
		if( PLS.outfile[0] != '\0' ) makeoutfilename( PLS.outfile, PLS.mapfile, 'm', 1);
		else strcpy( PLS.mapfile, "unnamed.map" );
		}
	PL_clickmap_init();
	}

return( 0 );
}

