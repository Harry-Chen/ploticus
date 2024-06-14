/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC CURVEFIT - Fit a curve to the data.
   Result becomes the new data that plotting procedures access.  */

#include "pl.h"

#define MAXPTS 5000	/* max # input points for movingavg curve */
#define MAXORDER 21	/* max bspline order value (no limit for movingavg order) */
#define MAXBSP_IN 100  /* max # input points for bspline curve */

#define MOVINGAVG	'm'
#define REGRESSION	'r'
#define BSPLINE		'b'
#define SIMPLEAVG	'a'
#define INTERPOLATED	'i'

static double in[MAXPTS][2];
/* the PLV vector is used for curve points */
static int bspline(), mavg(), plainavg(), lregress();
static int dblcompare(double *f, double *g);


PLP_curvefit()
{
int i;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

int j;
int stat;
int align;
double adjx, adjy;

int order;
int xfield;
int yfield;
int npts; /* number of input points */
int nresultpoints; /* number of result points */
int curd;
int davail;
int irow;
double resolution;
int showresults;
char linedetails[256];
double linestart, linestop;
double calcstart, calcstop;
int calcrangegiven;
char curvetype[40];
char legendlabel[120];
int linerangegiven;
int statsonly;
char selectex[256];
int selectresult;
char numstr[100];

TDH_errprog( "pl proc curvefit" );

/* initialize */
order = 4;
xfield = -1;
yfield = -1;
nresultpoints = -1;
resolution = 5.0;
showresults = 0;
strcpy( linedetails, "" );
linestart = EDXlo;
linestop = EDXhi;
calcrangegiven = 0;
linerangegiven = 0;
strcpy( curvetype, "movingavg" );
statsonly = 0;
strcpy( selectex, "" );
strcpy( legendlabel, "" ); /* added scg 7/28/03 */


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( stricmp( attr, "xfield" )==0 ) xfield = fref( val ) - 1;
	else if( stricmp( attr, "yfield" )==0 ) yfield = fref( val ) - 1;
	else if( stricmp( attr, "order" )==0 ) order = atoi( val );
	else if( stricmp( attr, "resolution" )==0 ) resolution = atof( val );
	else if( stricmp( attr, "linedetails" )==0 ) strcpy( linedetails, lineval );
	else if( stricmp( attr, "legendlabel" )==0 ) strcpy( legendlabel, lineval );
	else if( stricmp( attr, "select" )==0 ) strcpy( selectex, lineval );
	else if( stricmp( attr, "linerange" )==0 ) {
		if( lineval[0] != '\0' ) linerangegiven = 1;
		getrange( lineval, &linestart, &linestop, 'x', EDXlo, EDXhi );
		}
	else if( stricmp( attr, "calcrange" )==0 ) {
		if( lineval[0] != '\0' ) {
			calcrangegiven = 1;
			getrange( lineval, &calcstart, &calcstop, 'x', EDXlo, EDXhi );
			}
		else calcrangegiven = 0;
		}
	else if( stricmp( attr, "curvetype" )==0 ) strcpy( curvetype, val );
	else if( stricmp( attr, "showresults" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) showresults = 1;
		else showresults = 0;
		}
	else if( stricmp( attr, "statsonly" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) statsonly = 1;
		else statsonly = 0;
		}
	else Eerr( 1, "curvefit attribute not recognized", attr );
	}


/* overrides and degenerate cases */
if( Nrecords < 1 ) 
 	return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );

if( yfield < 0 || yfield >= Nfields ) return( Eerr( 601, "yfield not specified or out of range", "" ) );
if( xfield < 0 || xfield >= Nfields ) return( Eerr( 601, "xfield not specified or out of range", "" ) );

if( !scalebeenset() )
         return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );
 
if( strnicmp( legendlabel, "#usefname", 9 )==0 ) getfname( yfield+1, legendlabel );


/* now do the computation work.. */
/* -------------------------- */

/* process input data.. */
npts = 0;
for( irow = 0; irow < Nrecords; irow++ ) {

	if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
                stat = do_select( selectex, irow, &selectresult );
                if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
                if( selectresult == 0 ) continue; /* reject */
                }

	in[npts][1] = fda( irow, yfield, Y );
	if( Econv_error() ) { conv_msg( irow, yfield, "yfield" ); continue; }

	if( xfield < 0 ) in[npts][0] = (int)irow;
	else 	{
		in[npts][0] = fda( irow, xfield, X );
		if( Econv_error() ) { conv_msg( irow, xfield, "xfield" ); continue; }
		}

	/* compute curve only for points within calcrange */
	if( calcrangegiven ) {
		if( in[npts][0] < calcstart ) continue;
		else if( in[npts][0] > calcstop ) break;
		}

	if( curvetype[0] == BSPLINE && npts >= MAXBSP_IN ) {
		Eerr( 2599, "max of 100 input points allowed for bspline exceeded; curve truncated", "" );
		break;
		}
	if( npts >= MAXPTS ) {
		Eerr( 2599, "Sorry, max number of input points reached; curve truncated", "" );
		break;
		}
	npts++;
	}

/* sort points on x - added scg 11/2000 */
if( curvetype[0] != INTERPOLATED ) 
	qsort( in, npts, sizeof(double)*2, dblcompare );


if( curvetype[0] == MOVINGAVG ) { 
	mavg( in, npts, PLV, order );
	nresultpoints = npts;
	}


else if( curvetype[0] == REGRESSION ) { 
	double rlinestart, rlinestop;

	if( linerangegiven ) {
		rlinestart = linestart;
		rlinestop = linestop;
		}
	else	{
		rlinestart = in[0][0];
		rlinestop = in[npts-1][0];
		}
	lregress( in, npts, PLV, rlinestart, rlinestop );
	nresultpoints = 2;

	/* vertical line (degenerate case) suppress.. */
	if( dat2d( 0, 0 ) == dat2d( 1, 0 ) ) nresultpoints = 0;  
	
	/* clip to plotting area.. */
	stat = Elineclip( &dat2d(0,0), &dat2d(0,1), &dat2d(1,0), &dat2d(1,1), EDXlo, EDYlo, EDXhi, EDYhi );
	if( stat != 0 ) nresultpoints = 0;
	}

else if( curvetype[0] == BSPLINE ) {  
	nresultpoints = npts * resolution;
	if( nresultpoints >= PLVhalfsize ) nresultpoints = PLVhalfsize-1;

	if( order >= MAXORDER ) {
		Eerr( 2158, "Using max bspline order of 20", "" );
		order = 20;
		}
	if( npts < order ) 
		return( Eerr( 4892, "Must have at least 'order' data points", "" ) );

	/* do the computation.. */
	bspline( in, npts, PLV, nresultpoints, order );
	}

else if( curvetype[0] == INTERPOLATED ) { 
	stat = PL_smoothfit( in, npts, PLV, &nresultpoints );
	}

else 	{ 	/* average curve (basic) */
	plainavg( in, npts, PLV, order );
	nresultpoints = npts;
	}



/* draw the line.. */
linedet( "linedetails", linedetails, 1.0 );
Emovu( dat2d(0,0), dat2d(0,1) );
if( showresults ) fprintf( PLS.diagfp, "// generated curve points follow:\n%g %g\n", dat2d(0,0), dat2d(0,1) );


for( i = 1; i < nresultpoints; i++ ) {


	/* draw only within linerange.. */
	if( i > 0 && (dat2d(i,0)) > linestart && (dat2d(i-1,0)) < linestart ) {
		Emovu( dat2d(i,0), dat2d(i,1) );
		}

	else if( dat2d(i,0) < linestart ) continue;
	else if( dat2d(i,0) > linestop ) break;

	if( !statsonly ) Elinu( dat2d(i,0), dat2d(i,1) );  
	if( showresults ) fprintf( PLS.diagfp, "%g %g\n", dat2d(i,0), dat2d(i,1) );
	}


/* set YFINAL and Xfinal */
i--;
Euprint( numstr, X, dat2d(i,0), "" );
setcharvar( "XFINAL", numstr );
Euprint( numstr, Y, dat2d(i,1), "" );
setcharvar( "YFINAL", numstr );




if( legendlabel[0] != '\0' ) add_legent( LEGEND_LINE, legendlabel, "", linedetails, "", "" );

return( 0 );
}



/* =========================== */
static int mavg( in, npts, out, order )
double in[][2];  /* input points */
int npts;	 /* number of input points */
double out[][2]; /* output points (same n as input points array) */
int order;	 /* # of points to average */
{
int i, j;
int avgstart;
double accum;

for( i = 0; i < npts; i++ ) {
	avgstart = i - (order - 1);
	if( avgstart < 0 ) avgstart = 0;
	accum = 0.0;
	for( j = avgstart; j <= i; j++ ) accum += in[j][1];	
	out[i][0] = in[i][0];
	out[i][1] = accum / (double)(( i - avgstart ) + 1);
	}
return( 0 );
}

/* =========================== */
/* same as movingavg but averages in points to right as well.. */
static int plainavg( in, npts, out, order )
double in[][2];  /* input points */
int npts;	 /* number of input points */
double out[][2]; /* output points (same n as input points array) */
int order;	 /* # of points to average */
{
int i, j;
int avgstart, avgstop;
double accum;


for( i = 0; i < npts; i++ ) {
	avgstart = i - (order - 1);
	avgstop = i + (order - 1);
	if( avgstart < 0 ) avgstart = 0;
	if( avgstop >= npts ) avgstop = npts-1;
	accum = 0.0;
	for( j = avgstart; j <= avgstop; j++ ) accum += in[j][1];	
	out[i][0] = in[i][0];
	out[i][1] = accum / (double)(( avgstop - avgstart ) + 1  );
	}
return( 0 );
}

/* ============================== */
/* LREGRESS - linear regression curve */
static int lregress( in, npts, out, start, stop )
double in[][2];
int npts;	 /* number of input points */
double out[][2]; /* output points (n=2 since it's a straight line) */
double start, stop; /* X values - result line start and stop */
{
int i;
double sumx, sumxx, sumy, sumyy, sumxy;
double b, m;
double numer, denom, denomleft, denomright;
char buf[128], tok[128];
double sqrt();
char *GL_autoroundf();

sumx = sumxx = sumy = sumyy = sumxy = 0.0;
for( i = 0; i < npts; i++ ) {
	sumx += in[i][0];
	sumxx += (in[i][0] * in[i][0]);
	sumy += in[i][1];
	sumyy += (in[i][1] * in[i][1]);
	sumxy += (in[i][0] * in[i][1]);
	}
/* compute x intercept (b) */
numer = (sumy * sumxx) - (sumx * sumxy);
denom = ( (double)npts * sumxx ) - (sumx * sumx);
b = numer / denom; 

/* compute slope (m) */
numer = ((double)npts * sumxy) - (sumx * sumy);
denom = ((double)npts * sumxx) - (sumx * sumx);
if( denom == 0.0 ) m = 0.0;  /* ? */
else m = numer / denom;

out[0][0] = start;
out[0][1] = (m * start) + b;
out[1][0] = stop;
out[1][1] = (m * stop) + b;

strcpy( tok, GL_autoroundf( m, 0 ) );
sprintf( buf, "Y = %s + %sX", GL_autoroundf(b,0), tok );
TDH_setvar( "REGRESSION_LINE", buf );

/* compute r (pearson correlation coeff) */
/* numer = ((double) nvalues * sumxy) - (sumx * sumy ); */
denomleft = ((double) npts * sumxx)  - (sumx * sumx);
denomright = ((double) npts * sumyy)  - (sumy * sumy);
denom = sqrt( denomleft * denomright );

if( denom == 0.0 ) strcpy( buf, "(none)" );
else sprintf( buf, "%s", GL_autoroundf( (numer/denom), 0 ) );
TDH_setvar( "CORRELATION", buf );

return( 0 );
}

/* ======================== */
/* BSPLINE curve */
static int
bspline( in, npts, out, ncv, order )
double in[][2];  /* input points */
int npts;	 /* number of input points */
double out[][2]; /* output points */
int ncv;         /* number of output points to generate */
int order;	 /* order of the curve  (2..n_in) */
{
int i, j, k, l, m, n;
int nknot; /* size of knot vector */
double t; /* parameter */
double N[MAXBSP_IN][MAXORDER]; /* weighting function */ 
double knot[MAXBSP_IN+MAXORDER]; /* knot vector */
double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
int n_out;



/* generate the knot vector */
/* ------------------------ */
for( i = 0; i < MAXBSP_IN; i++ ) knot[i] = 0.0; /* init */
n = npts - 1;
nknot = n + 1 + order;
for( i = 0; i < order; i++ ) knot[i] = 0;
j = 1;
for( ; i < nknot-order; i++ ) knot[i] = j++;
for( ; i < nknot; i++ ) knot[i] = j;
/* printf( "Knot= [ " ); 
 *  for( i = 0; i < nknot; i++ ) printf( "%g ", knot[i] ); 
 * printf( "]\n" ); 
 */


t = 0.0;
n_out = 0;
for( j = 0; j < ncv; j++ ) { /* for each point to be generated.. */

    /* do the N(?,1) set.. */
    for( i = 0; i <= n+1; i++ ) {
	if( knot[i] <= t && t < knot[i+1] ) N[i][1] = 1.0;
	else N[i][1] = 0.0;
	}

    /* do middle N.. */
    for( k = 2; k <= order; k++ ) {
	for( i = 0; i <= npts; i++ ) {
	    tmp1 = ((t - knot[i])*N[i][k-1]);
	    tmp2 = (knot[i+k-1] - knot[i]);
	    tmp3 = ((knot[i+k] - t)*N[i+1][k-1]);
	    tmp4 = (knot[i+k] - knot[i+1]);
	    if( tmp2 == 0.0 ) { tmp5 = 0.0; }
	    else { tmp5 = tmp1 / tmp2; }
	    if( tmp4 == 0.0 ) { tmp6 = 0.0; }
	    else { tmp6 = tmp3 / tmp4; }
	    N[i][k] = tmp5 + tmp6;
	    }
	}

    if( j == ncv-1 ) N[n][order] = 1.0; /* for last point */

    tmp1 = tmp2 = tmp3 = 0;
    for( i = 0; i < npts; i++ ) {
	tmp1 += (in[i][0]*N[i][order]);
	tmp2 += (in[i][1]*N[i][order]);
	tmp3 += 0.0; /* (in[i][2]*N[i][order]); */
	}

    /* put curve into D2 */
    out[n_out][0] = tmp1;
    out[n_out][1] = tmp2;
    /* out[n_out][2] = tmp3; */
    n_out++;
    t += ( knot[nknot-1] / (double)(ncv - 1) );
    }

return( 1 );
}

/* ============================= */
static int
dblcompare( f, g )
double *f, *g;
{
if( *f > *g ) return( 1 );
if( *f < *g ) return( -1 );
return( 0 );
}

