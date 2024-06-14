/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

#include "pl.h"
#define MAXENTRIES 300
#define MAXURLTEXT 10000

static char mapfile[MAXPATH];
static int imap;
static char *urls[MAXENTRIES];
static struct {
	char typ;
	int pmode;
	int x1; int y1; int x2; int y2;
	} box[MAXENTRIES];
static char *urltext;
static int itext;
static char defaulturl[MAXPATH] = "";
static char tpurl[MAXPATH] = ""; /* a url template */
static int mapstatus = 0; /* 1 if we are in the process of doing a map; 0 otherwise */
static int debugmode = 0; /* 1 if we are in debug mode */
static int first = 1;
static int intersect = 0;

/* ========================= */
/* MAPFILENAME - set the filename for the mapfile */
mapfilename( m, nbytes, debug )
char *m;
int nbytes; /* # bytes to allocate for url pool */
int debug; /* if 1, display map regions within image for debugging */
{
strcpy( mapfile, m );
strcpy( defaulturl, "" );
if( first ) urltext = (char *) malloc( nbytes );
first = 0;
debugmode = debug;
imap = 0;
itext = 0;
mapstatus = 1;
intersect = 0;
return( 0 );
}

/* ========================= */
/* DOINGMAP - return 1 if we are doing a clickmap, 0 otherwise */
doingmap()
{
return( mapstatus );
}
/* ========================= */
/* DOINGMAPDEBUG - return 1 if we are in debug mode, 0 otherwise */
doingmapdebug()
{
return( debugmode );
}

/* ========================= */
/* DEFAULTMAPURL - set the "default" url */
defaultmapurl( url )
char *url;
{
strcpy( defaulturl, url );
return( 0 );
}

/* ========================== */
/* TPMAPURL - set the url template */
tpmapurl( url )
char *url;
{
strcpy( tpurl, url );
return( 0 );
}


/* ========================= */
/* MAPENTRY - add one map entry */
/* typs: 'r' = rectangle, lower left x,y and upper right x,y;   'p' = point */
mapentry( typ, url, pmode, x1, y1, x2, y2, textpad, clipmode )
char typ;
char *url;
int pmode; /* processing mode; 0 = none, 1 = sub into tpurl as x, 2 = sub into tpurl as y, 
		3 = intersect w/another then sub into tpurl as x, 4 = intersect then sub as y  */
double x1, y1, x2, y2;
int textpad; /* add extra padding to text */
int clipmode;  /* 0 = no clip;  1 = clip X to plotting area  2 = clip Y to plotting area */
{
int i;
double r, sx, sy;

if( imap >= MAXENTRIES-1 ) return( Eerr( 2706, "mapfile: cannot add entry, capacity exceeded", url ) );
if( !mapstatus ) return( Eerr( 2707, "mapfile: -map must be specified on command line", "" ) );

urls[ imap ] = &urltext[ itext ];
strcpy( &urltext[ itext ], url );

for( i = 0; urltext[itext+i] != '\0'; i++ ) if( GL_member( urltext[itext+i], " \n" )) urltext[itext+i] = '_';
GL_substitute( "\\n", "_", &urltext[ itext ] ); /* convert newline rep to underscores.. */

i =  itext + strlen( &urltext[ itext ] ) +1;
if( i > MAXURLTEXT ) return( Eerr( 2705, "mapfile: cannot add entry, text capacity exceeded", url ) );
itext = i;

if( pmode > 2 ) intersect = 1;
box[ imap ].pmode = pmode;
box[ imap ].typ = typ;
if( typ == 'p' ) { x2 = x1; y2 = y1; }

/* adjust for textpad, clip, and global scaling, if any.. */
if( textpad ) { x1 -= 0.15; x2 += 0.1; y1 -= 0.04; }
if( clipmode == 1 ) { if( x1 < EXlo ) x1 = EXlo; if( x2 >  EXhi ) x2 = EXhi; }
if( clipmode == 2 ) { if( y1 < EYlo ) y1 = EYlo; if( y2 >  EYhi ) y2 = EYhi; }
Egetglobalscale( &sx, &sy );
x1 *= sx; y1 *= sy; x2 *= sx; y2 *= sy;

box[ imap ].x1 = Exsca( x1 ); box[ imap ].y1 = Eysca( y2 );
box[ imap ].x2 = Exsca( x2 ); box[ imap ].y2 = Eysca( y1 );

imap++;
return( 0 );
}

/* ========================= */
/* WRITEMAPFILE - write out the map file */
/* Note: Clickmap == 1 for server-side,  2 for client-side */

writemapfile( tx, ty )
int tx, ty; /* translate vector */
{
int i, j;
FILE *fp;
char buf[255];
int ox1, oy1, ox2, oy2;

if( imap < 1 ) return( Eerr( 2795, "Warning, no map regions were assigned", mapfile ) );

fp = fopen( mapfile, "w" );
if( fp == NULL ) return( Eerr( 2705, "Cannot open mapfile", mapfile ));

if( debugmode ) fprintf( stderr, "writing clickmap file %s, coords translated by %d,%d\n", mapfile, tx, ty );

if( Clickmap == 1 && defaulturl[0] != '\0' ) fprintf( fp, "default %s\n", defaulturl );

/* first do any intersections.. */
if( intersect ) for( i = imap-1; i >= 0; i-- ) {
	if( box[i].pmode == 3 ) {
		/* find all '4' entries.. write out an entry for each one found.. */
		/* assume all '4' entries overlap all '3' entries.. */
		for( j = imap-1; j >= 0; j-- ) {
			if( box[j].pmode == 4 ) {
				strcpy( buf, tpurl );
				GL_varsub( buf, "@XVAL", urls[ i ] );
				GL_varsub( buf, "@YVAL", urls[ j ] );
				/* take greatest x1 */
				if( box[ i ].x1 > box[j].x1 ) ox1 = box[i].x1;
				else ox1 = box[j].x1;
				/* take smallest x2 */
				if( box[ i ].x2 < box[j].x2 ) ox2 = box[i].x2;
				else ox2 = box[j].x2;
				/* take greatest y1 */
				if( box[ i ].y1 > box[j].y1 ) oy1 = box[i].y1;
				else oy1 = box[j].y1;
				/* take smallest y2 */
				if( box[ i ].y2 < box[j].y2 ) oy2 = box[i].y2;
				else oy2 = box[j].y2;

				if( Clickmap == 1 ) 
					fprintf( fp, "rect %s	%d,%d	%d,%d\n", buf, ox1-tx, oy1-ty, ox2-tx, oy2-ty );
				else if( Clickmap == 2 ) 
					fprintf( fp, "<area shape=\"rect\" href=\"%s\" coords=\"%d,%d,%d,%d\" >\n", 
						buf, ox1-tx, oy1-ty, ox2-tx, oy2-ty );
				}
			continue;
			}
		}
	}

/* now do the rest.. */
for( i = imap-1; i >= 0; i-- ) {
	/* if( box[ i ].typ == 'p' ) {
	 *	fprintf( fp, "point %s	%d,%d\n", urls[ i ], box[ i ].x1, box[ i ].y1 ); 
	 *	continue; 
	 *	}	
	 */

	if( box[ i ].pmode > 0 ) {
		strcpy( buf, tpurl );
		if( box[ i ].pmode == 1 || box[i].pmode == 3 ) {
			GL_varsub( buf, "@XVAL", urls[ i ] );
			GL_varsub( buf, "@YVAL", "" );
			if( Clickmap == 1 ) fprintf( fp, "rect %s	", buf );
			else if( Clickmap == 2 ) fprintf( fp, "<area shape=\"rect\" href=\"%s\" ", buf );
			}

		else if( box[ i ].pmode == 2 || box[i].pmode == 4 ) {
			GL_varsub( buf, "@YVAL", urls[ i ] );
			GL_varsub( buf, "@XVAL", "" );
			if( Clickmap == 1 ) fprintf( fp, "rect %s	", buf );
			else if( Clickmap == 2 ) fprintf( fp, "<area shape=\"rect\" href=\"%s\" ", buf );
			}
		}
	else 	{
		if( Clickmap == 1 ) fprintf( fp, "rect %s	", urls[i] );
		else if( Clickmap == 2 ) fprintf( fp, "<area shape=\"rect\" href=\"%s\" ", urls[i] );
		}

	if( Clickmap == 1 ) fprintf( fp, "%d,%d	%d,%d\n", 
				box[ i ].x1 - tx, box[ i ].y1 - ty, box[ i ].x2 - tx, box[ i ].y2 - ty );
	else if( Clickmap == 2 ) fprintf( fp, "coords=\"%d,%d,%d,%d\" >\n",
				box[ i ].x1 - tx, box[ i ].y1 - ty, box[ i ].x2 - tx, box[ i ].y2 - ty );
	}

if( Clickmap == 2 ) {
	if( defaulturl[0] != '\0' ) fprintf( fp, "<area shape=\"default\" href=\"%s\">\n", defaulturl );
	/* fprintf( fp, "</map>\n" ); */
	}

fclose( fp );
#ifndef WIN32
chmod( mapfile, 00644 );
#endif
return( 0 );
}

/* =========================== */
/* SHOWMAPFILE - display map regions for debugging */
showmapfile( dev )
char dev;
{
int i;
if( imap < 1 ) return( Eerr( 2937, "Warning, no map regions were assigned", mapfile ) );
if( dev == 'x' ) {
#ifndef NOX11
	EXWcolor( "brightgreen" );
	EXWlinetype( "0", 0.5, 1.0 );
	for( i = 0; i < imap; i++ ) {
		if( box[i].typ == 'p' ) { box[i].x2 += 1; box[i].y2 += 1; }
		EXWrawline( box[i].x1, box[i].y1, box[i].x2, box[i].y1 );
		EXWrawline( box[i].x2, box[i].y1, box[i].x2, box[i].y2 );
		EXWrawline( box[i].x2, box[i].y2, box[i].x1, box[i].y2 );
		EXWrawline( box[i].x1, box[i].y2, box[i].x1, box[i].y1 );
		}
#endif
	;
	}

else if( dev == 'g' ) {
#ifndef NOGD
	EGcolor( "brightgreen" );
	EGlinetype( "1", 0.5, 1.0 );
	for( i = 0; i < imap; i++ ) {
		if( box[i].typ == 'p' ) { box[i].x2 += 1; box[i].y2 += 1; }
		EGrawline( box[i].x1, box[i].y1, box[i].x2, box[i].y1 );
		EGrawline( box[i].x2, box[i].y1, box[i].x2, box[i].y2 );
		EGrawline( box[i].x2, box[i].y2, box[i].x1, box[i].y2 );
		EGrawline( box[i].x1, box[i].y2, box[i].x1, box[i].y1 );
		}
#endif
	;
	}
return( 0 );
}
