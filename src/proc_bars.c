/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC BARS - bargraphs and histograms */

#include "pl.h"
#define MAXSTACKFIELDS 40
#define MAXCLP 200

static int do_label();
static int do_lwl();
static char stacklist[300] = ""; /* lenfields get appended to this list so that "stackfield: *"  can be used */
static int prevclust = 1; /* attempt to be smart about resetting stacklist on cluster change */


#if 1
/*
 * Michael D. Beynon (mdb) - beynon@cs.umd.edu
 * 09/23/2001 : mdb - Bug fix: wrong position of cluster bar labels.
 *                  - Bug fix: only use selected records for automatic x value
 *                  - Changes to allow clustered segment bars.
 *
 * Note: These changes allow clustered bars to not require all data on one
 *       line, and instead use select to pick the correct portion for each
 *       clusterpos.
 *
 * Todo: What would be nice is a way to automatically collect category names
 *       to avoid retyping them from the data.
 *       #proc yaxis
 *          stubs: categories
 *          ycategories_select: @2 == 1985 or @2 == 1995
 */
#endif


PLP_bars( )
{
int i;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

char buf[256];
int j;
int stat;
int align;
double adjx, adjy;
int lenfield;
int locfield;
char color[40];
char outline[256];
int do_outline;
double halfw;
double x, y, y0, xleft, xright;
char axis, baseax;
double barwidth;
int showvals;
char labeldetails[256];
int nstackf;
int stackf[MAXSTACKFIELDS];
double f;
int ncluster;
int clusterpos;
char crossover[40];
double cr;
double laby;
char backbox[40];
int labelfld;
char labelword[40];
char labelstr[40];
int lwl; /* do longwise labels */
int reverse;
int stopfld;
double taillen;
int errlofld, errhifld, reflecterr;
char selectex[256];
int result;
char legendlabel[120];
int doo;
int ii;
int reverseorder, reversespecified;
char rangelo[40], rangehi[40];
double rlo, rhi;
double clustsep;
int trunc;
int doytail, doy0tail;
int label0val;
char thinbarline[256];
int leftticfld, rightticfld, midticfld;
double ticlen;
double ytic;
char colorlist[256];
char *colorlp[MAXCLP];
int ncolorlp;
char dcolor[40];
char lblpos[40];
int taillengiven;
char numstrfmt[40];
int barwidthfield;
int hidezerobars; /* scg 11/29/00 */
double errbarmult;
int ibar;
int colorfield;
char mapurl[MAXPATH], expurl[MAXPATH];
int irow;
int segmentflag;


TDH_errprog( "pl proc bars" );



/* initialize */
axis = 'y';
lenfield = -1;
locfield = -1;
strcpy( color, "0.7" );
do_outline = 1;
strcpy( outline, "yes" );
barwidth = 0.0;
showvals = 0;
strcpy( labeldetails, "" );
nstackf = 0;
ncluster = 1;
clusterpos = 1;
strcpy( crossover, "" );
strcpy( backbox, "" );
strcpy( labelword, "" );
labelfld = -1;
lwl = 0;
stopfld = -1;
taillen = 0.0;
errlofld = errhifld = -1;
reflecterr = 0;
strcpy( selectex, "" );
strcpy( legendlabel, "" );
reverseorder = 0;
reversespecified = 0;
strcpy( rangelo, "" );
strcpy( rangehi, "" );
clustsep = 0.0;
trunc = 0;
label0val = 0;
strcpy( thinbarline, "" );
leftticfld = rightticfld = midticfld = -1;
ticlen = 0.02;
strcpy( colorlist, "" );
strcpy( lblpos, "" );
ncolorlp = 0;
taillengiven = 0;
strcpy( numstrfmt, "%g" );
barwidthfield = -1;
hidezerobars = 0;
errbarmult = 1.0;
colorfield = -1;
strcpy( mapurl, "" );
segmentflag = 0;


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];


	if( stricmp( attr, "lenfield" )==0 ) lenfield = fref( val ) -1;
	else if( stricmp( attr, "locfield" )==0 ) locfield = fref( val ) -1; 
	else if( stricmp( attr, "axis" )==0 ) axis = tolower(val[0]);
	else if( stricmp( attr, "horizontalbars" )==0 ) axis = 'x';
	else if( stricmp( attr, "color" )==0 ) strcpy( color, val );
	else if( stricmp( attr, "outline" )==0 ) strcpy( outline, lineval );
	else if( stricmp( attr, "barwidth" )==0 ) {
		barwidth = atof( val );
		if( Using_cm ) barwidth /= 2.54;
		}
	else if( strnicmp( attr, "stackfield", 10 )==0 ) {
		int ix;
		char fname[50];
		/* if( strcmp( val, "*" )==0 || strcmp( val, "all" )==0 ) strcpy( lineval, stacklist ); */
		for( ix = 0, j = 0; j < MAXSTACKFIELDS; j++ ) {
                        if( GL_smember( val, "* all" )) strcpy( fname, GL_getok( stacklist, &ix ) );
                        else strcpy( fname, GL_getok( lineval, &ix ) );
			if( fname[0] == '\0' ) break;
			stackf[j] = fref( fname );
			}
		nstackf = j;
		}
	else if( stricmp( attr, "cluster" )==0 ) {
		nt = sscanf( lineval, "%d %s %d", &clusterpos, buf, &ncluster );
		if( nt == 2 ) sscanf( lineval, "%d %d", &clusterpos, &ncluster );
		}
	else if( stricmp( attr, "clustersep" )==0 ) {
		clustsep = atof( val );
		if( Using_cm ) clustsep /= 2.54;
		}
	else if( stricmp( attr, "crossover" )==0 ) strcpy( crossover, val );
	else if( strnicmp( attr, "segmentfield", 12 )==0 ) {
		char fnames[2][50];
		/* nt = sscanf( lineval, "%d %d", &stackf[0], &stopfld ); */
		nt = sscanf( lineval, "%s %s", fnames[0], fnames[1] );
		
		if( nt == 1 ) stopfld = fref( fnames[0] );
		
		if( nt == 2 ) {
			nstackf = 1;
			stackf[0] = fref( fnames[0] );
			stopfld = fref( fnames[1] );
			}
		segmentflag = 1;
		}
	else if( strnicmp( attr, "errbarfield", 11 )==0 ) {
		/* nt = sscanf( lineval, "%d %d", &errlofld, &errhifld ); */
		char fname[2][50];
		nt = sscanf( lineval, "%s %s", fname[0], fname[1] );
		errlofld = fref( fname[0] ); 
		if( nt == 1 ) reflecterr = 1; /* use -val for lo, +val for hi */
		else 	{
			reflecterr = 0;
			errhifld = fref( fname[1] );
			}
		/* taillen = 0.2; */ /* default */ /* can't set taillen here- 
						messes up cloning of tails - scg 12/21/99 */
		/* barwidth = 0.001; */ /* force lines */
		}
	else if( strnicmp( attr, "errbarmult", 10 )==0 ) {
		errbarmult = atof( val );
		}
	else if( stricmp( attr, "tails" )==0 ) {
		taillen = atof( val );
		taillengiven = 1;
		if( Using_cm ) taillen /= 2.54;
		}
	else if( stricmp( attr, "showvalues" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) showvals = 1;
		else showvals = 0;
		}
	else if( stricmp( attr, "numbersformat" )==0 ) strcpy( numstrfmt, val );
	else if( stricmp( attr, "labelzerovalue" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) label0val = 1;
		else label0val = 0;
		}
	else if( stricmp( attr, "truncate" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) trunc = 1;
		else trunc = 0;
		}
	else if( stricmp( attr, "labeldetails" )==0 ) strcpy( labeldetails, lineval );
	else if( stricmp( attr, "backbox" )==0 ) strcpy( backbox, val );
	else if( stricmp( attr, "labelfield" )==0 ) labelfld = fref( val ) - 1;
	else if( stricmp( attr, "labelword" )==0 ) strcpy( labelword, lineval );
	else if( stricmp( attr, "thinbarline" )==0 ) strcpy( thinbarline, lineval );
	else if( stricmp( attr, "leftticfield" )==0 ) leftticfld = fref( val ) -1;
	else if( stricmp( attr, "rightticfield" )==0 ) rightticfld = fref( val ) -1;
	else if( stricmp( attr, "midticfield" )==0 ) midticfld = fref( val ) -1;
	else if( stricmp( attr, "ticlen" )==0 ) ticlen = atof( val );
	else if( stricmp( attr, "reverseorder" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) { reversespecified = 1; reverseorder = 1; }
		else reverseorder = 0;
		}
	else if( stricmp( attr, "hidezerobars" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) hidezerobars = 1;
		else hidezerobars = 0;
		}

	else if( stricmp( attr, "barsrange" )==0 ) sscanf( lineval, "%s %s", rangelo, rangehi );
	else if( stricmp( attr, "colorlist" )==0 ) strcpy( colorlist, lineval );
	else if( stricmp( attr, "colorfield" )==0 ) colorfield = fref( val ) -1;

	/* longwise label */
	else if( stricmp( attr, "longwayslabel" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) lwl = 1;
		else lwl = 0;
		}
	else if( stricmp( attr, "select" )==0 ) strcpy( selectex, lineval );
	else if( stricmp( attr, "legendlabel" )==0 ) strcpy( legendlabel, lineval );
	else if( stricmp( attr, "labelpos" )==0 ) strcpy( lblpos, val );
	else if( stricmp( attr, "barwidthfield" )==0 ) barwidthfield = fref( val ) -1;
	else if( stricmp( attr, "clickmapurl" )==0 ) strcpy( mapurl, val );

	else Eerr( 1, "attribute not recognized", attr );
	}


/* -------------------------- */
/* overrides and degenerate cases */
/* -------------------------- */
if( axis == 'y' ) baseax = 'x';
else baseax = 'y';

if( Nrecords[Dsel] < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );
if( !scalebeenset() )
         return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );



if( locfield > Nfields[Dsel] ) return( Eerr( 52, "locfield out of range", "" ) );
if( lenfield > Nfields[Dsel] ) return( Eerr( 52, "lenfield out of range", "" ) );
if( lenfield < 0 && stopfld < 0 ) 
	return( Eerr( 2805, "Either lenfield or segmentfields must be defined", ""));

if( stopfld >= 0 ) {
	if( nstackf > 1 )  /* but 1 is ok */
		{ Eerr( 2984, "stackfield may not be used with segments", "" ); nstackf=0; }
#if 0
	if( ncluster > 1 ) 
	  { Eerr( 2984, "clustering may not be used with segments", "" ); ncluster=0; }
#endif
	}

if( labelword[0] != '\0' ) showvals = 1;
if( showvals && labelword[0] == '\0' ) strcpy( labelword, "@N" );

if( locfield == lenfield ) Eerr( 2479, "Warning, locfield same as lenfield", "" );

for( i = 0; i < nstackf; i++ ) {
	if( (lenfield+1) == stackf[i] ) Eerr( 2479, "Warning, lenfield same as a stackfield", "" );
	if( (locfield+1) == stackf[i] ) Eerr( 2479, "Warning, locfield same as a stackfield", "" );
	}

if( axis == 'x' && !reversespecified ) reverseorder = 1;

/* -------------------------- */
/* now do the plotting work.. */
/* -------------------------- */
if( baseax == 'y' ) Eflip = 1;

if( rangelo[0] != '\0' ) rlo = Econv( baseax, rangelo );
else rlo = Elimit( baseax, 'l', 's' );
if( rangehi[0] != '\0' ) rhi = Econv( baseax, rangehi );
else rhi = Elimit( baseax, 'h', 's' );

/* maintain stacklist */
if( clusterpos != prevclust ) {
	resetstacklist();
#if 1
	if( stopfld <= 0 ) nstackf = 0; /* needed for current bar */
#else
	nstackf = 0; /* needed for current bar */
#endif
	}
prevclust = clusterpos;
sprintf( buf, "%d ", lenfield+1 );
strcat( stacklist, buf );


if( barwidth > 0.0 ) halfw = barwidth * 0.5;
else	{
	if( ncluster <= 1 ) halfw = ( Ea( X, 1.0 ) - Ea( X, 0.0 ) ) * 0.4;
	else if( ncluster > 1 ) halfw = (( Ea( X, 1.0 ) - Ea( X, 0.0 ) ) * 0.4)/ (double)ncluster;
	if( halfw > 0.5 ) halfw = 0.1; /* sanity  - to prevent huge bars */
	}



if( outline[0] == '\0' || strnicmp( outline, "no", 2 )==0 ) do_outline = 0;
else do_outline = 1;


if( crossover[0] == '\0' ) cr = Elimit( axis, 'l', 's' );
else cr = Econv( axis, crossover );

/* parse colorlist if any */
if( colorlist[0] != '\0' ) {
	int ix, ixx; char tok[40];
	/* initialize all pointers to default color.. */
	strcpy( dcolor, color );
	for( i = 0; i < MAXCLP; i++ ) colorlp[i] = dcolor;
	ix = 0; ixx = 0;
	i = 0;
	while( 1 ) {
		strcpy( tok, GL_getok( colorlist, &ix ) ); 
		if( tok[0] == '\0' ) break;
		if( atoi( tok ) > 0 && atoi( tok ) < MAXCLP ) {
			colorlp[ atoi(tok) - 1 ] = &colorlist[ix];
			GL_getok( colorlist, &ix );
			}
		else if( i < MAXCLP ) {
			colorlp[ i ] = &colorlist[ ixx ];
			i++;
			}
		ixx = ix;
		}
	}

linedet( "outline", outline, 0.5 );

if( thinbarline[0] != '\0' && strnicmp( thinbarline, "no", 2 ) != 0 ) 
	linedet( "thinbarline", thinbarline, 0.3 );

if( errlofld >= 0 && !taillengiven ) taillen = 0.2; /* set a default taillen for errorbars */


/* now start looping through current data set.. */
ibar = -1;
for( irow = 0; irow < Nrecords[Dsel]; irow++ ) {

	if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
                stat = do_select( selectex, irow, &result );
                if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
                if( result == 0 ) continue; /* reject */
                }
	ibar++;

	if( lenfield >= 0 ) {
		y = fda( irow, lenfield, axis );
		if( Econv_error() ) { conv_msg( irow, lenfield, "lenfield" ); continue; }
		}
	if( locfield < 0 ) {
#if 1 /* count only selected data */	 
		if( reverseorder ) x = (Elimit( baseax, 'h', 's' ) - 1.0) - (double)ibar;
		else x = (double)ibar+1;
		if( x > Elimit( baseax, 'h', 's' ) ) {
			fprintf( Errfp, "bars warning, skipping bar# %d, loc is out of range\n", ibar+1 );
			continue; /* out of range hi */
			}
#else
		if( reverseorder ) x = (Elimit( baseax, 'h', 's' ) - 1.0) - (double)irow;
		else x = (double)irow+1;
		if( x > Elimit( baseax, 'h', 's' ) ) {
			fprintf( Errfp, "bars warning, skipping bar# %d, loc is out of range\n", irow+1 );
			continue; /* out of range hi */
			}
#endif
		}	
	else 	{
		x = fda( irow, locfield, baseax );
		if( Econv_error() ) { conv_msg( irow, locfield, "locfield" ); continue; }
		if( x < rlo ) continue;  /* out of range low */
		if( x > rhi ) continue;  /* out of range high */
		}

	/* y0 = Elimit( axis, 'l', 's' ); */
	if( nstackf > 0 && !segmentflag ) y0 = 0.0; /* added scg 11/27/01 */
	else y0 = cr;

	/* if barwidthfield was specified, set bar width now.. */
	if( barwidthfield >= 0 ) {
		double bw;
		bw = fda( irow, barwidthfield, axis );
		halfw = (Ea( X, bw ) - Ea( X, 0.0 )) * 0.5;
		if( Econv_error() ) { 
			conv_msg( irow, barwidthfield, "barwidthfield" ); 
			halfw = 0.05; 
			}
		}

	if( nstackf > 0 ) /* stacking */
		for( j = 0; j < nstackf; j++ ) {
			f = fda( irow, stackf[j]-1, axis );
			if( stopfld > 0 ) f -= cr; /* normalize? */
			y0 += f;
			y += f;
			}


	if( ncluster > 1 ) {   /* clustering - move sideways a little.. */
		xleft = Ea( X, x ) - ((halfw+clustsep) * (double)(ncluster));
		xleft += clustsep;
		if( baseax == Y ) xleft += ((halfw+clustsep) * (ncluster-clusterpos)*2.0);
		else xleft += ((halfw+clustsep) * (clusterpos-1)*2.0);
		xright = xleft + (halfw*2.0);
		}
	else	{
		xleft = Ea( X, x) - halfw;
		xright = Ea( X, x) + halfw;
		}

	if( stopfld > 0 ) { /* set y from stopfld; bar start is done via stacking, above */
		y = fda( irow, stopfld-1, axis );
		}
	if( errlofld > 0 ) { /* set y and y0 as offsets from original y */
		y0 = y - (fda( irow, errlofld-1, axis ) * errbarmult);
		if( reflecterr ) y += (fda( irow, errlofld-1, axis ) * errbarmult);
		else y += (fda( irow, errhifld+1, axis ) * errbarmult);
		}

	/* catch units errors for stopfld and errflds.. */
	if( stopfld > 0  && Econv_error() ) {conv_msg( irow, stopfld, "segmentfields" );continue;}
	if( errlofld > 0 && Econv_error() ){conv_msg( irow, stopfld, "errbarfields" );continue;} 

	/* null-length bars.. skip out.. scg 11/29/00 */
	if( hidezerobars && y == y0 ) continue;

	/* truncate to plotting area.. scg 5/12/99 */
	doytail = 1; doy0tail = 1;
	if( trunc ) {

		if( y0 <= Elimit( axis, 'l', 's' ) && y < Elimit( axis, 'l', 's' ) ) {
			fprintf( Errfp, "warning, bar completely out of %c plotting area\n", axis );
			continue; /* skip entirely */
			}
		if( y0 >= Elimit( axis, 'h', 's' ) && y > Elimit( axis, 'h', 's' ) ) {
			fprintf( Errfp, "warning, bar completely out of %c plotting area\n", axis );
			continue; /* skip entirely */
			}

		if( !Ef_inr( axis, y0 ) ) {
			if( y0 < y ) y0 = Elimit( axis, 'l', 's' );
			else y0 = Elimit( axis, 'h', 's' );
			doy0tail = 0;
			}
		if( !Ef_inr( axis, y ) ) {
			if( y0 < y ) y = Elimit( axis, 'h', 's' );
			else y = Elimit( axis, 'l', 's' );
			doytail = 0;
			}
		}

	/* if colorfield used, get color.. */
	if( colorfield >= 0 ) {
		strcpy( color, "" );
		get_legent( da( irow, colorfield ), color, NULL, NULL );
		}

	/* if colorlist used, get color.. */
	if( colorlist[0] != '\0' && ibar < MAXCLP ) sscanf( colorlp[ibar], "%s", color ); 

	/* now do the bar.. */
	if( strcmp( color, Ecurbkcolor )==0 ) doo = 1; /* if bar color is same as background, force outline.. */
	else doo = do_outline;

	/* allow @field substitutions into url */
	if( Clickmap && mapurl[0] != '\0' ) do_subst( expurl, mapurl, irow );

	/* if thinbarline specified, or if doing error bars, render bar as a line */
	if( ( thinbarline[0] != '\0' && strnicmp( thinbarline, "no", 2 )!= 0 ) || errlofld >= 0 ) { 
		Emov( xleft+halfw, Ea( Y, y0 ) ); 
		Elin( xleft+halfw, Ea( Y, y ) ); 
		}

  	/* otherwise, render bar as a rectangle */
  	else 	{
		Ecblock( xleft, Ea( Y, y0 ), xright, Ea( Y, y ), color, doo ); 
		if( Clickmap && mapurl[0] != '\0' ) {
			if( Eflip ) mapentry( 'r', expurl, 0, Ea( Y, y0 ), xleft, Ea( Y, y ), xright, 0, 0 );
			else mapentry( 'r', expurl, 0, xleft, Ea( Y, y0 ), xright, Ea( Y, y ), 0, 0 );
			}
		}
  
  	/* do tics if requested */  /* don't do if trunc && outside area - scg 11/21/00 */ 
	/* Bug fix - ticks not being drawn at bars when truncating was not switched on.
         * Supplied by Michael Rausch (mr@netadair.de) date: 04 Jun 2001 */
  	if( leftticfld >= 0 ) { 
  		ytic = fda( irow, leftticfld, axis );
  		if( !Econv_error() && ( !trunc || Ef_inr( axis, ytic ) ) ) { 
  			Emov( (xleft+halfw), Ea(Y,ytic) ); 
  			Elin( (xleft+halfw)-ticlen, Ea(Y,ytic) ); 
  			}
  		}
  	if( rightticfld >= 0 ) { 
  		ytic = fda( irow, rightticfld, axis );
  		if( !Econv_error() && ( !trunc || Ef_inr( axis, ytic ) ) ) {
  			Emov( (xleft+halfw), Ea(Y,ytic) ); 
  			Elin( (xleft+halfw)+ticlen, Ea(Y,ytic) ); 
  			}
  		}
  	if( midticfld >= 0 ) { 
  		ytic = fda( irow, midticfld, axis );
  		if( !Econv_error() && ( !trunc || Ef_inr( axis, ytic ) ) ) { 
  			Emov( (xleft+halfw)-(ticlen/2.0), Ea(Y,ytic) ); 
  			Elin( (xleft+halfw)+(ticlen/2.0), Ea(Y,ytic) ); 
  			}
		}


	/* do tails if requested */
	if( taillen > 0.0 ) {
		double g, h;
		g = xleft + ((xright-xleft)  / 2.0);
		h = taillen / 2.0;
		if( doytail ) { Emov( g-h, Ea(Y,y) ); Elin( g+h, Ea(Y,y) ); }
		if( doy0tail ) { Emov( g-h, Ea(Y,y0) ); Elin( g+h, Ea(Y,y0) ); }
		}

	}


if( showvals || labelfld >= 0 ) { /* print values.. a lot of code replicated from above loop.. */
	textdet( "labeldetails", labeldetails, &align, &adjx, &adjy, -3, "R" );
	if( adjy == 0.0 ) adjy = 0.02; /* so label is a little above end of bar */
	if( align == '?' ) align = 'C';
#if 1
	ibar = -1;
#endif
	for( i = 0; i < Nrecords[Dsel]; i++ ) {
		if( selectex[0] != '\0' ) { /* added 8/23/01 - process against selection condition if any.. */
                	stat = do_select( selectex, i, &result );
                	if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
                	if( result == 0 ) continue; /* reject */
                	}
#if 1
		ibar++;
#endif

		if( lenfield >= 0 ) {
			y = fda( i, lenfield, axis );
			if( Econv_error() ) continue; /* don't bother to label bad values */
			if( !label0val && GL_close_to( y, cr, 0.01 )) continue; /* don't label 0 */
			}
		if( locfield < 0 ) {
#if 1 /* count only selected data */	 
		        if( reverseorder ) x = (Elimit( baseax, 'h', 's' ) - 1.0) - (double)ibar;
			else x = (double)ibar+1;
#else
		        if( reverseorder ) x = (Elimit( baseax, 'h', 's' ) - 1.0) - (double)i;
			else x = (double)i+1;
#endif
			}	
		else 	{
			x = fda( i, locfield, baseax );
			if( x < rlo ) continue; /* out of range low */
			if( x > rhi ) continue; /* out of range high */
			}

		if( labelfld >= 0 ) strcpy( labelstr, da( i, labelfld ) );
		else strcpy( labelstr, labelword );
		Euprint( buf, axis, y, numstrfmt );
		GL_varsub( labelstr, "@N", buf );

		f = cr; /* needed in case we want to center long text label along len of bar */
		if( nstackf > 0 ) /* stacking affects label placement */
			for( j = 0; j < nstackf; j++ ) {
				f = fda( i, stackf[j]-1, axis );
				/* (f is used later on below to center longwise labels) */
				if( stopfld > 0 ) y += (f - cr); /* normalize? */
				else y += f;
				}

		if( stopfld >= 0 ) { /* set y from stopfld; bar start is done via stacking, above */
			y = fda( i, stopfld-1, axis );
			if( Econv_error() ) continue;
			}

		if( errlofld > 0 ) { /* set y and y0 as offsets from original y */
			y0 = y - fda( i, errlofld+1, axis );
			if( reflecterr ) y += fda( i, errlofld-1, axis );
			else y += fda( i, errhifld+1, axis );
			if( Econv_error() ) continue;
			}

		/* truncate to plotting area.. scg 5/12/99 */
		if( trunc && !Ef_inr( axis, y ) ) {
			if( y > Elimit( axis, 'h', 's' ) )  y = Elimit( axis, 'h', 's' );
			else laby = y = Elimit( axis, 'l', 's' );
			}

		if( y < cr ) { 
			laby = Ea( Y, y ) + (adjy*(-1.0));
			if( !Eflip ) laby -= Ecurtextheight;
			reverse = 1;
			}
		else 	{
			laby = (Ea( Y, y )+adjy);
			reverse = 0;
			}


		/* if explicit label position given, use it.. */
		if( lblpos[0] != '\0' ) Eposex( lblpos, axis, &laby );


		if( ncluster > 1 ) { /* if clusters, move sideways a little bit.. */
#if 1
			x = Ea( X, x ) - ((halfw+clustsep) * (double)(ncluster));
			x += clustsep;
		        if( baseax == Y ) x += ((halfw+clustsep) * (ncluster-clusterpos)*2.0);
			else x += ((halfw+clustsep) * (clusterpos-1)*2.0);
			x += halfw;
#else
			x = Ea( X, x ) - (halfw * (double)(ncluster));
			x += (halfw * (clusterpos-1)*2.0);
			x += halfw;
#endif
			if( lwl ) do_lwl( labelstr, x+adjx, Ea( Y, y )+adjy, Ea(Y,f), 
				align, reverse );
			else do_label( labelstr, x+adjx, laby, align, backbox, reverse );
			}
		else 	{
			if( lwl ) {
				do_lwl( labelstr, Ea(X,x)+adjx, Ea( Y, y )+adjy, Ea(Y,f), 
					align, reverse );
				}
			else do_label( labelstr, Ea(X,x)+adjx, laby, align, backbox, reverse );
			}
		}
	}


if( legendlabel[0] != '\0' ) {
	if( errlofld > 0 || ( thinbarline[0] != '\0' && strnicmp( thinbarline, "no", 2 )!= 0) ) 
		add_legent( LEGEND_LINE, legendlabel, "", thinbarline, "", "" );

	else add_legent( LEGEND_COLOR, legendlabel, "", color, "", "" );
	}

if( baseax == 'y' ) Eflip = 0;
return( 0 );
}


/* ====================== */
static int
do_label( s, x, y, align, backbox, reverse )
char *s;
double x, y;
char align;
char *backbox;
int reverse;
{
double halfbox;

halfbox = ((strlen( s ) * Ecurtextwidth) / 2.0) + 0.01;

convertnl( s ); /* added scg 3/8 */

if( Eflip ) {
	if( reverse && align == 'L' ) {
		align = 'R';
		/* no backing necessary */
		Emov( x-0.02, y );
		}
	else if( align != 'R' ) {
		align = 'L'; 
		/* no backing box necessary, past end of bar */
		Emov( x-0.02, y+0.03 ); 
		}
	else if( reverse && align == 'R' ) {
		align = 'L';
		if( backbox[0] != '\0' )
			Ecblock( x-0.1, y+0.03,
				x+(Ecurtextheight*0.8), y+(halfbox*2)+0.03, backbox, 0 );
		Emov( x-0.02, y+0.03 );
		}
	else 	{ /* R align */
		if(  backbox[0] != '\0' )
			Ecblock( x-0.1, (y-(halfbox*2))-0.03, 
				x+(Ecurtextheight*0.8), y-0.03, backbox, 0 );
		Emov( x-0.02, y-0.03 ); 
		}
	}

else 	{
	if( backbox[0] != '\0' ) 
		Ecblock( x-halfbox, y-0.01, x+halfbox, y+(Ecurtextheight*0.8), backbox, 0 );
	Emov( x, y );
	}

Edotext( s, align );

return( 0 );
}

/* ============================ */
static int
do_lwl( s, x, y, y0, align, reverse )
char *s;
double x, y, y0;
char align;
int reverse;
{
double y1, y2;
int nlines, maxlen;


if( y0 < y ) { y1 = y; y2 = y0; }
else { y1 = y0; y2 = y; }

convertnl( s );
measuretext( s, &nlines, &maxlen );
if( Eflip ) {
	x -= Ecurtextheight*0.4;
	x += (((nlines-1)*0.5)*Ecurtextheight);
	}
else 	{
	x += Ecurtextheight*0.4;
	x -= (((nlines-1)*0.5)*Ecurtextheight);
	}

if( reverse ) {
	if( align == 'L' ) align = 'R';
	else if( align == 'R' ) align = 'L';
	}

if( align == 'C' ) Emov( x, y2+((y1-y2)/2.0) );
else if( align == 'L' ) Emov( x, y2 );
else Emov( x, y1 );
if( !Eflip ) Etextdir(90);
Edotext( s, align );
if( !Eflip ) Etextdir(0);
}

/* ================================ */
/* RESETSTACKLIST - called when a new areadef is done, or 
   when beginning a new member-of-cluster */

resetstacklist()
{
strcpy( stacklist, "" );
return( 0 );
}
