/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


/* PL - ploticus main module */

#include "pl.h"

#ifdef LOCALE
#include <locale.h>
#endif

vermessage()
{
char outputformats[80];
devstring( outputformats );

fprintf( Diagfp, "\n\
Usage: pl -prefab prefabname [parameters]     ..or.. pl scriptfile [options] \n\
%s\n\n", outputformats );

fprintf( Diagfp, "ploticus 2.03 " );
#ifdef UNIX
fprintf( Diagfp, "(unix)" );
#endif
#ifdef WIN32
fprintf( Diagfp, "(win32)" );
#endif
fprintf( Diagfp, " data display software (c) 1998-2002 Stephen C. Grubb\n\
Home page is ploticus.sourceforge.net   Please see the Copyright file for\n\
additional credits and information.\n\
\n\
This program is free software; you can redistribute it and/or modify it\n\
under the terms of the GNU General Public License as published by the\n\
Free Software Foundation; either version 2 of the License, or (at your\n\
option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful, but \n\
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY \n\
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License\n\
for more details.\n" );
exit( 1 );
}

main( argc, argv )
int argc;
char **argv;
{
int i, j;
char buf[256];
int use_stdin;
int vardec;
int nt;
FILE *fp;
int stat;
int found;
int valused;
int stoparg;
int ci, cii;
char *arg, *nextarg, *getenv();


#ifdef LOCALE
 setlocale(LC_CTYPE, "" );
 setlocale(LC_COLLATE, "" );
#endif


/* set pre-profile defaults.. */
TDH_errprog( "pl" );
Winx = 100; Winy = 0;
Winw = 8.0; Winh = 8.0;
Debug = 0;
Winsizegiven = 0;
Bkcolorgiven = 0;
use_stdin = 0;
strcpy( Inputfile, "" );
strcpy( Cmdline_outfile, "" );
strcpy( Cmdlineparms, "" );
strcpy( Flags, "" );
strcpy( Prefab_name, "" );
#ifndef NORLIMIT
reslimits( "cpu", CPULIMIT );
reslimits( "filesize", FILESIZELIMIT );
#endif


/* default device.. */
#ifdef NOX11
Device = 'e';
#else
Device = 'x';    
#endif
strcpy( Viewer, "" );

if( argc == 1 ) CGIargs = getenv( "REQUEST_URI" );
else CGIargs = NULL;


/* read and process profile if any.. */
strcpy( Cmdlineparms, "" );
do_preliminaries();


/* cgi mode.. */
if( CGIargs != NULL ) {
	/* begin parsing REQUEST_URI.. */
	stoparg = 80; 
	ci = 0;
	GL_getcgiarg( Bigbuf, CGIargs, &ci, 252 ); /* skip over first part.. */
	/* set some useful defaults.. */
	strcpy( Cmdline_outfile, "stdout" );
	Device = 's';
#ifndef NOGD
	Device = 'g';
	/* try to be smart about picking default output format.. */
	if( devavail( "gif" )) EG_setimfmt( "gif" );
	else if( devavail( "jpeg" )) EG_setimfmt( "jpeg" );
	else if( devavail( "png" )) EG_setimfmt( "png" );
#endif
	}
else stoparg = argc;


/* process command line arguments.. */
for( i = 1; i < stoparg; i++ ) {

	if( CGIargs != NULL ) {
		/* parse next 2 args from REQUEST_URI.. */
		GL_getcgiarg( Bigbuf, CGIargs, &ci, 252 );
		if( Bigbuf[0] == '\0' ) break;
		arg = Bigbuf;
		cii = ci;
		GL_getcgiarg( Databuf, CGIargs, &cii, 252 );
		nextarg = Databuf;
		}
	else	{
		arg = argv[i];
		if( i+1 < argc ) nextarg = argv[i+1];
		else nextarg = "";
		}

	if( strcmp( arg, "-stdin" )==0 && CGIargs == NULL ) use_stdin = 1;

	else if( strcmp( arg, "-png" )==0 && !devavail( "png" ) && CGIargs == NULL ) { 
		char *p[100];
		if( strlen( argv[0] ) >= 5 ) {
			if( strcmp( &argv[0][strlen(argv[0])-5], "plpng" )==0 ) {
				Eerr( 5279, "png not available in plpng", "" );
				vermessage(); exit(1);
				}
			}
		p[0] = "plpng";
		for( i = 1; i < argc; i++ ) p[i] = argv[i];
		p[argc] = NULL;
		execvp( "plpng", argv );
		fprintf( Errfp, "PNG not supported in this build (plpng not found).\n" );
                vermessage(); exit(1);
		}

	else 	{
		process_arg( arg, nextarg, &valused, &found );
		if( !found && arg[0] == '-' ) 
			Eerr( 4892, "warning, unrecognized argument", arg );
		else if( !found ) {
			if( strlen( arg ) > MAXPATH-10 ) { /* allow extra for output file suffix add */
				fprintf( Errfp, "pl: script file name too long" );
				vermessage(); exit( 1 );
				}
			strcpy( Inputfile, arg  );  
			}
		i += valused;
		if( CGIargs != NULL && valused ) ci = cii; /* jump ahead */
		}
	}

if( Debug ) {
	fprintf( Diagfp, "pl debug mode\n" );
	Epcodedebug( 1, Diagfp );
	}


if( CGIargs != NULL ) {
	char dd, imagetype[20];
	strcpy( Cmdline_outfile, "stdout" );
	/* check for loopy script file names.. */
	if( Inputfile[0] == '/' && Prefab_name[0] == '\0' ) {   /* changed scg 2/6/02 */
		Eerr( 2740, "cgi mode: script file name may not begin with '/'", Inputfile );
		vermessage(); exit(1);
		}
	if( GL_slmember( Inputfile, "*..* .*" ) ) {
		Eerr( 2740, "cgi mode: script file name may not begin with '.' or contain '..'", Inputfile );
		vermessage(); exit(1);
		}
	/* output the HTTP content-type header.. */
	devnamemap( &Device, imagetype, 2 );
	if( Device == 's' ) printf( "Content-type: image/%s-xml\n\n", imagetype );
	else printf( "Content-type: image/%s\n\n", imagetype );
	}



/* copy stdin to a tmp file.. */
if( use_stdin ) {
	sprintf( Inputfile, "%s_I", Tmpname );
	fp = fopen( Inputfile, "w" ); /* temp file, unlinked below */
	if( fp == NULL ) {
		Eerr( 102, "Cannot open tmp file for stdin script\n", Inputfile );
		vermessage(); exit(1);
		}
	while( fgets( buf, 255, stdin ) != NULL ) fprintf( fp, "%s", buf );
	fclose( fp );
	}
	
if( Inputfile[0] == '\0' ) {
	Eerr( 20, "No -prefab or scriptfile specified on command line", "" );
	vermessage( );
	}


/* set up.. */
Esetsize( Winw, Winh, Winx, Winy );

/* DEVICE variable.. */
if( TDH_getvar( "DEVICE", buf ) != 0 ) { /* DEVICE not given on command line, set DEVICE */
	devnamemap( &Device, buf, 2 );
	TDH_setvar( "DEVICE", buf );
	}
else	{ /* DEVICE given on command line, set Device from DEVICE */
	TDH_getvar( "DEVICE", buf );
	devnamemap( &Device, buf, 1 );
	}
if( Debug ) fprintf( Diagfp, "Device code is %c\n", Device );

/* set default output file names for gif and eps.. and svg.. */
if( Cmdline_outfile[0] == '\0' ) {
    	if( GL_member( Device, "egs" ) ) {
		if( Prefab_name[0] != '\0' ) makeoutfilename( Prefab_name, Cmdline_outfile, Device, 1);
		else makeoutfilename( Inputfile, Cmdline_outfile, Device, 1 );
		}
	}

/* set clickmap name */
if( Clickmap ) {
	char mapfile[ MAXPATH ];
	if( Cmdline_outfile[0] != '\0' ) makeoutfilename( Cmdline_outfile, mapfile, 'm', 1);
	else if( Prefab_name[0] != '\0' ) makeoutfilename( Prefab_name, mapfile, 'm', 1);
	else makeoutfilename( Inputfile, mapfile, 'm', 1 );
	mapfilename( mapfile, 10000, Debug );
	}

/* if a viewcommand is specified and an outfile has not been and the device 
   is paginated postscript, we need to set the output file to out.ps */
if( Viewer[0] != '\0' && Cmdline_outfile[0] == '\0' ) {
    	if( Device == 'c' || Device == 'p' ) strcpy( Cmdline_outfile, "out.ps" );
	}


if( Cmdline_outfile[0] != '\0' && 
    strcmp( Cmdline_outfile, "-" )!= 0 ) 
	Esetoutfilename( Cmdline_outfile );


/* pl initializations.. */
EDXlo = 0.0; EDXhi = 0.0; EDYlo = 0.0; EDYhi = 0.0;
Initialized = 0;


/* EXECUTE THE SCRIPT FILE to produce the plot.. */

if( Debug ) fprintf( Diagfp, "Script: %s\n", Inputfile );
stat = exec_plfile( Inputfile );
if( stat != 0 ) { vermessage(); exit( stat ); }


/* finish up (x11: button, etc.) */
if( Initialized && Device == 'x' ) do_x_button( "End." );


if( Initialized ) {
	Eendoffile();
	}

if( use_stdin ) unlink( Inputfile );

if( Viewer[0] != '\0' && Device != 'x' && CGIargs == NULL ) {
	Egetoutfilename( buf );
	if( stricmp( buf, "stdout" )==0 ) fprintf( Diagfp, "Cannot use -o stdout with -viewer\n" );
	else if( buf[0] != '\0' ) {
		strcat( Viewer, " " );
		strcat( Viewer, buf );
		fprintf( Diagfp, "Executing '%s' to view results..\n", Viewer );
		system( Viewer );
		}
	}

exit( 0 );
}

