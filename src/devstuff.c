/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


#include "pl.h"

/* ========================== */
/* DEVAVAIL - returns 1 if specified device driver or resource is available
	in this build, or 0 if not. */
devavail( dev )
char *dev;
{
if( dev[0] == 'x' ) {  /* x11 */
#ifdef NOX11
	return( 0 );
#endif
	return( 1 );
	}

else if( strcmp( dev, "gif" )==0 ) { 
#if GD16 || GD18 || NOGD
	return( 0 );
#endif
	return( 1 );
	}

else if( strcmp( dev, "png" )==0 ) {
#if GD13 || NOGD
	return( 0 );
#endif
	return( 1 );
	}

else if( strcmp( dev, "jpeg" )==0 ) {
#if GD18
	return( 1 );
#endif
	return( 0 );
	}

else if( strcmp( dev, "wbmp" )==0 ) {
#if GD18
	return( 1 );
#endif
	return( 0 );
	}

else if( GL_smember( dev, "ps eps svg" )) return( 1 );

else if( strcmp( dev, "svgz" )==0 ) {
#if GD16 || GD18 || WZ
       	return( 1 );
#else
	return( 0 );
#endif
	}
return( 0 );
}

/* ========================== */
/* DEVSTRING - creates a string listing the output format options available with this build. */
devstring( s )
char *s;
{
/* Added svg option - BT 05/11/01 */
strcpy( s, "Available: ps eps svg " );
#ifdef WZ
       	strcat( s, "svgz " );
#endif
#ifndef NOX11
 strcat( s, "x11 " );
#endif
#ifdef GD13
 strcat( s, "gif (png) " );
#endif
#ifdef GD16
 strcat( s, "png " );
#endif
#ifdef GD18
 strcat( s, "png jpeg wbmp " );
#endif
#ifdef GDFREETYPE
 strcat( s, "FreeType2 " );
#endif
return( 0 );
}

/* ========================== */
/* DEVNAMEMAP - translate between internal and external 
	representation of output device code */
devnamemap( s, t, mode )
char *s; /* internal (Device) */
char *t; /* external (command line opt[1] or DEVICE */
int mode; /* 1 = set s given t    2 = set t given s */
{
char c;
char imfmt[20];

if( mode == 1 ) {
	c = tolower(t[0]);

	/* old monochrome ps */
	if( strcmp( t, "bwps" )==0 ) *s = 'p';

	/* postscript */
	else if( strcmp( t, "ps" )==0 ) *s = 'c';

	/* eps */
	else if( strcmp( t, "eps" )==0 ) *s = 'e';

	/* svg  - BT 05/11/01 */
	else if( strcmp( t, "svg" )==0 ) *s = 's';
	else if( strcmp( t, "svgz" )==0 ) {
		*s = 's';
		SVG_z( 1 );
		}

	/* x11 */
	else if( strcmp( t, "x11" )==0 ) {
		if( !devavail( "x" ) ) {
			fprintf( Errfp, "X11 not supported in this build.\n" );
			exit(1);
			}
		*s = 'x';
		}

#ifndef NOGD
	/* GD image formats */
	else if( GL_smember( t, "gif png jpeg wbmp" ) ) { 
		*s = 'g'; 
		if( !devavail( t )) {
			fprintf( Errfp, "%s format not supported in this build.\n", t );
			exit(1);
			}
		/* Png = 1;  */
		EG_setimfmt( t );
		}
#endif


	else Eerr( 8026, "unrecognized device code", t );
	return( 0 );
	}
else if( mode == 2 ) {
	if( *s == 'p' ) strcpy( t, "bwps" );
	else if( *s == 'c' ) strcpy( t, "ps" );
	else if( *s == 'e' ) strcpy( t, "eps" );
#ifndef NOGD
	else if( *s == 'g' ) EG_getimfmt( t );
#endif
	else if( *s == 's' ) SVG_fmt( t ); /* svg  - BT 05/11/01 */
	else if( *s == 'x' ) strcpy( t, "x11" );
	else Eerr( 8025, "unrecognized internal device rep", "" );
	return( 0 );
	}
else Eerr( 8027, "invalid devnamemap mode", "" );
}

/* ============================== */
/* MAKEOUTFILENAME - given script file name, make a default output file
   for when no output file is specified by user.

   Returns 0 if ok; 1 if usage error 
*/
makeoutfilename( scriptfn, outfn, dev, page )
char *scriptfn; /* script name (or if -o and page > 1, may be the output file name given on command line) */
char *outfn;
char dev;
int page;
{
int len, j;
char *ext;
char imfmt[20];

len = strlen( scriptfn );

for( j = len-1; j >= 0; j-- ) if( scriptfn[j] == '.' ) break;
if( j < 0 ) ext = "";
else ext = &scriptfn[ j ];

/* svg added - BT 05/11/01 */
if( GL_smember( ext, ".p .pl .plo .pls .htm .html .gif .png .eps .ps .jpg .jpeg .bmp .svg .svgz" )) {
	strcpy( outfn, scriptfn );
	len -= strlen( ext );
	}
else strcpy( outfn, "out" );

if( dev == 'e' ) strcpy( imfmt, "eps" );
#ifndef NOGD
else if( dev == 'g' ) EG_getimfmt( imfmt );
#endif
else if( dev == 's' ) SVG_fmt( imfmt ); /* svg  - BT 05/11/01 */
else if( dev == 'm' ) strcpy( imfmt, "map" ); /* for click map name */

if( page > 1 ) sprintf( &outfn[ len ], ".p%d.%s", page, imfmt );
else sprintf( &outfn[ len ], ".%s", imfmt );
return( 0 );
}
