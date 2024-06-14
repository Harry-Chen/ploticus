/*
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


/* ================== */
/* PROCESS_ARG - process one command line argument from pl */

#include "pl.h"

process_arg( opt_in, val, valused, found )
char *opt_in; /* option */
char *val; /* argument that follows option */
int *valused; /* 1 if val was used, 0 if not */
int *found; /* 1 if option found here, 0 if not */
{
int i, j;
int vardec;
char buf[256];
int nt;
char opt[256];
int slen;


*found = 1;
if( opt_in[0] == '-' ) strcpy( opt, &opt_in[1] );
else strcpy( opt, opt_in );


vardec = 0;
for( j = 0, slen = strlen( opt ); j < slen; j++ ) {
	if( opt[j] == '=' ) {
		vardec = 1;
		break;
		}
	}

/* add to list of parameters set on command line.. */
if( !vardec ) {
	strcat( Cmdlineparms, opt );
	strcat( Cmdlineparms, " " );
	}

*valused = 1;

if( GL_slmember( opt, "x11 gif png jpeg wbmp ps bwps eps svg svgz" )) { devnamemap( &Device, opt, 1 ); *valused = 0; }

else if( strcmp( opt, "tag" )==0) { SVGshowtag( 1 ); *valused = 0; }

else if( strcmp( opt, "zlevel" )==0) { SVG_zlevel( atoi( val ) ); }

else if( strcmp( opt, "debug" )==0) { 
	Debug = 1; 
	*valused = 0; 
	if( CGIargs != NULL ) {
		/* -debug in cgi mode writes stuff to fixed temp dir files.. */
		sprintf( buf, "%s%cplcgi_diag", TMPDIR, PATH_SLASH );
		Diagfp = fopen( buf, "w" );
		if( Diagfp == NULL ) Diagfp = stderr;
		/* else fchmod( Diagfp->_file , 00644 ); */
		sprintf( buf, "%s%cplcgi_err", TMPDIR, PATH_SLASH );
		Errfp = fopen( buf, "w" );
		if( Errfp == NULL ) Errfp = stderr;
		/* else fchmod( Errfp->_file , 00644 ); */
		}
	}

else if( strcmp( opt, "tightcrop" )==0 ) { Etightbb(1); *valused = 0; }

else if( strcmp( opt, "viewer" )==0 || strcmp( opt, "v" )==0 ) {
	if( CGIargs == NULL ) strcpy( Viewer, val ); 
	}

else if( strcmp( opt, "prefab" )==0 ) {
	if( Prefabs_dir == NULL ) {
		Eerr( 4899, "PLOTICUS_PREFABS environment var not found (pathname of dir where prefab files reside)", "" );
		vermessage(); exit( 1 );
		}
	sprintf( Prefab_name, "%s.pl", val );
	sprintf( Inputfile, "%s%c%s", Prefabs_dir, PATH_SLASH, Prefab_name );
	}

else if( strncmp( opt, "crop", 4 )==0 ) {
	double cropx1, cropy1, cropx2, cropy2;
	nt = sscanf( val, "%lf,%lf,%lf,%lf", &cropx1, &cropy1, &cropx2, &cropy2 );
	if( nt != 4 ) 
		fprintf( Errfp, "pl argument error, correct usage is: -crop x1,y1,x2,y2 OR -croprel left,bottom,right,top\n" );
	else {
		if( Using_cm ) { cropx1 /= 2.54; cropy1 /= 2.54; cropx2 /= 2.54; cropy2 /= 2.54; }
		if( strcmp( opt, "croprel" )==0 ) Especifycrop( 2, cropx1, cropy1, cropx2, cropy2 ); /* relative to tight */
		else Especifycrop( 1, cropx1, cropy1, cropx2, cropy2 ); /* absolute */
		}
	}

else if( strcmp( opt, "pagesize" )==0 ) {
	nt =  sscanf( val, "%lf,%lf", &Winw, &Winh );
	if( nt != 2 ) 
		fprintf( Errfp, "pl argument error, correct usage is: -pagesize width,height (inches or cm)\n" );
	else Winsizegiven = 1;
	if( Using_cm ) { Winw /= 2.54; Winh /= 2.54; }
	}

else if( strcmp( opt, "winloc" )==0 ) {
	nt = sscanf( val, "%d,%d", &Winx, &Winy );
	if( nt != 2 ) {
		Winx = 100; Winy = 0;
		fprintf( Errfp, "pl argument error, correct usage is: -winloc x,y\n" );
		}
	}

else if( strcmp( opt, "o" )==0 ) {
	if( strlen( val ) > MAXPATH-1 ) {
		fprintf( Errfp, "pl output file name is too long\n" );
		vermessage(); exit( 1 );
		}
	strcpy( Cmdline_outfile, val );
	}

else if( strcmp( opt, "scale" )==0) {
	double sx, sy;
	nt =  sscanf( val, "%lf,%lf", &sx, &sy );
	if( nt == 1 ) Esetglobalscale( sx, sx );
	else if( nt == 2 ) Esetglobalscale( sx, sy );
	}

else if( strcmp( opt, "posteroffset" )==0) {
	double x, y;
	nt = sscanf( val, "%lf,%lf", &x, &y );
	if( nt != 2 ) fprintf( Errfp, "pl argument error, correct usage is -posteroffset x,y\n" );
	else	{
		if( Using_cm ) { x /= 2.54; y /= 2.54; }
		Esetposterofs( x, y );
		}
	}

else if( strcmp( opt, "map" )==0 ) { Clickmap = 1; *valused = 0; }   /* server side map */
else if( strcmp( opt, "csmap" )==0 ) { Clickmap = 2; *valused = 0; } /* client side map */

else if( strcmp( opt, "landscape" )==0 ) { Landscape = 1; *valused = 0; }
else if( strcmp( opt, "portrait" )==0 ) { Landscape = 0; *valused = 0; Winw = 8.5; Winh = 11.0; } 
								/* winw and winh added scg 11/14/00 */

else if( strcmp( opt, "showbad" )==0 ) { suppress_convmsg( 0 ); *valused = 0; }

else if( strcmp( opt, "font" )==0) strcpy( Estandard_font, val );
		
else if( strcmp( opt, "textsize" )==0) Estandard_textsize = atoi( val );

else if( strcmp( opt, "linewidth" )==0) Estandard_lwscale = atof( val );

else if( strcmp( opt, "color" )==0) strcpy( Estandard_color, val );

else if( strcmp( opt, "cm" )==0) { Using_cm = 1; *valused = 0; setintvar( "CM_UNITS", 1 ); }

else if( strcmp( opt, "inches" )==0) { Using_cm = 0; *valused = 0; setintvar( "CM_UNITS", 0 ); }

else if( strcmp( opt, "backcolor" )==0) {
	strcpy( Estandard_bkcolor, val );
	Ebackcolor( val );
	Bkcolorgiven = 1;
	}
/* else if( strcmp( opt, "id" )==0 ) {
 *	strcat( Flags, "I" );
 *	*valused = 0;
 *	}
 * else if( strcmp( opt, "noid" )==0 ) {
 *	strcat( Flags, "N" );
 *	*valused = 0;
 *	}
 */

else if( GL_smember( opt, "? help ver version" ) ) {
	vermessage(); exit(0);
	}

else if( strcmp( opt, "diagfile" )==0 ) {
	if( CGIargs != NULL ) return( 0 ); /* don't allow this in cgi mode.. scg 2/8/02 */
	if( GL_smember( val, "stderr STDERR" )) Diagfp = stderr; /* portability  */
	else if( GL_smember( val, "stdout STDOUT" )) Diagfp = stdout; /* portability  */
	else	{
                Diagfp = fopen( val, "w" ); /* diagnostics */
                if( Diagfp == NULL ) {
                        fprintf( Errfp, "warning, cannot open -diagfile %s, using stderr\n", val );
                        Diagfp = stderr;
                        }
                }
	}

else if( strcmp( opt, "errfile" )==0 ) {
	if( CGIargs != NULL ) return( 0 );/* don't allow this in cgi mode.. scg 2/8/02 */
	if( GL_smember( val, "stderr STDERR" )) Errfp = stderr; /* portability  */
	else if( GL_smember( val, "stdout STDOUT" )) Errfp = stdout; /* portability  */
	else	{
                Errfp = fopen( val, "w" ); /* diagnostics */
                if( Errfp == NULL ) {
                        fprintf( stderr, "warning, cannot open -errfile %s, using stderr\n", val );
                        Errfp = stderr;
                        }
                }
	}
 
else if( vardec ) {	/* its a VAR=VALUE pair */
	int ix;
	char varname[40];
       	strcpy( buf, opt );
       	ix = 0;
        GL_getseg( varname, buf, &ix, "=" );
        TDH_setvar( varname, &buf[ix] );
	*valused = 0;
        }

else 	{ *found = 0; *valused = 0; }

return( 0 );
}
