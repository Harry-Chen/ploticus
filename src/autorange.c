/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

#include "pl.h"
#define MAXDF 50

autorange( axis, specline, minval, maxval )
char axis;
char *specline; /* spec line from proc areadef.. */
char *minval;   /* determined plot minima.. */
char *maxval;   /* determined plot maxima.. */
{
int i, j;
int df[MAXDF];
int ndf;
char nearest[80];
char buf[256];
char dfield[256];
int nt;
double min, max, fmod();
char smin[80], smax[80];
double fval;
int stat;
double margin;
int ix;
char lowfix[80], hifix[80];
char unittyp[80];
char floatformat[20];
char datepart[40], timepart[40];
double incmult;
char tok[80];
char selex[256]; /* added */
int selresult; /* added */
int combomode, first;
double hiaccum, loaccum;
int goodfound;
double submin, submax;


/* see what scaletype is being used.. */
Egetunits( axis, unittyp );
/* for linear, default to automatic determination of reasonable endpoints.. */
if( strcmp( unittyp, "linear" )==0 ) strcpy( nearest, "auto" );
/* for date, time, and other types, default to exact endpoints.. */
else strcpy( nearest, "exact" );

margin = 0.0;
strcpy( dfield, "" );
ix = 0;
strcpy( lowfix, "" );
strcpy( hifix, "" );
strcpy( floatformat, "%f" );
incmult = 1.0;
strcpy( selex, "" ); /* added */
combomode = 0;

while( 1 ) {
        strcpy( buf, GL_getok( specline, &ix ) );
        if( buf[0] == '\0' ) break;
        if( strnicmp( buf, "datafields", 10 )==0 ) strcpy( dfield, &buf[11] );
        else if( strnicmp( buf, "datafield", 9 )==0 ) strcpy( dfield, &buf[10] );
        else if( strnicmp( buf, "incmult", 7 )==0 ) incmult = atof( &buf[8] );
        else if( strnicmp( buf, "nearest", 7 )==0 ) strcpy( nearest, &buf[8] );
        else if( strnicmp( buf, "margin", 6 )==0 ) margin = atof( &buf[7] );
        else if( strnicmp( buf, "lowfix", 6 )==0 ) strcpy( lowfix, &buf[7] );
        else if( strnicmp( buf, "hifix", 5 )==0 ) strcpy( hifix, &buf[6] );
        else if( strnicmp( buf, "format", 6 )==0 ) strcpy( floatformat, &buf[7] );
        else if( strnicmp( buf, "combomode", 9 )==0 ) {
		if( stricmp( &buf[10], "stack" )==0 ) combomode = 1;
		else if( stricmp( &buf[10], "hilo" )==0 ) combomode = 2;
		else combomode = 0;
		}
        else if( strnicmp( buf, "selectrows", 10 )==0 ) {
                strcpy( selex, &buf[11] );
                strcat( selex, &specline[ix] );
                break;
                }
	else Eerr( 5702, "unrecognized autorange subattribute", buf );
	}

/* fill array of df from comma-delimited list - contributed by Paul Totten <pwtotten@nortelnetworks.com> */
ndf = 0; ix = 0; 
while( 1 ) {
	GL_getseg( tok, dfield, &ix, "," );
	if( tok[0] == '\0' ) break;
	if( ndf >= (MAXDF-1) ) break;
	df[ndf] = fref( tok ) - 1;
	if( df[ndf] < 0 || df[ndf] >= Nfields[Dsel] ) continue;
	ndf++;
	}

/* df = fref( dfield ) - 1; */

if( Nrecords[Dsel] < 1 ) return( Eerr( 17, "autorange: no data set has been read/specified w/ proc getdata", "" ) );
if( axis == '\0' ) return( Eerr( 7194, "autorange: axis attribute must be specified", "" ) );
if( ndf == 0 ) return( Eerr( 7194, "autorange: datafield omitted or invalid ", dfield ) );


/* ------------------ */
/* now do the work.. */
/* ----------------- */

/* override.. */
if( stricmp( nearest, "day" )==0 && stricmp( unittyp, "date" )==0 ) strcpy( nearest, "exact" );

/* find data min and max.. */
min = PLHUGE;
max = NEGHUGE;
for( i = 0; i < Nrecords[Dsel]; i++ ) {

        if( selex[0] != '\0' ) { /* added scg 8/1/01 */
                stat = do_select( selex, i, &selresult );
                if( selresult == 0 ) continue;
                }


	hiaccum = 0.0; loaccum = 0.0; first = 1; goodfound = 0; submin = PLHUGE; submax = NEGHUGE;
	for( j = 0; j < ndf; j++ ) {  /* for all datafields */
		fval = fda( i, df[j], axis );
		if( Econv_error() ) continue;
		goodfound = 1;
		if( !combomode ) {
			if( fval < submin ) submin = fval;
			if( fval > submax ) submax = fval;
			}
		else 	{
			hiaccum += fval;
			if( combomode == 2 ) { /* hilo */
				if( first ) { loaccum = fval; first = 0; }
				else loaccum -= fval;
				}
			else	{ /* stack */
				if( first ) { loaccum = fval; first = 0; }
				}
			}
		}
	if( !goodfound ) continue;

	if( combomode ) {
		if( loaccum < min ) min = loaccum;
		if( hiaccum > max ) max = hiaccum;
		}
	else	{
		if( submin < min ) min = submin;
		if( submax > max ) max = submax;
		}
	}



/* now convert min and max to current units and then to nearest interval.. */
if( strcmp( unittyp, "linear" )==0 ) { /* avoid using Euprint()- trouble w/ v. big or v. small #s */
	sprintf( smin, floatformat, min );
	sprintf( smax, floatformat, max );
	}
else	{
	Euprint( smin, axis, min, "" );
	Euprint( smax, axis, max, "" );
	}

/* save data min/max.. */
if( axis == 'x' ) {
	setcharvar( "DATAXMIN", smin );
	setcharvar( "DATAXMAX", smax );
	}
else 	{
	setcharvar( "DATAYMIN", smin );
	setcharvar( "DATAYMAX", smax );
	}


/* now adjust for margin.. */
min -= margin;
max += margin;


/* degenerate case.. all data the same (bad if it happens to lie on inc boundary, eg: 0) - added scg 9/21/01 */
if( min == max ) {
	min = min - 1.0;
	max = max + 1.0;
	}

/* and do the conversion with margin.. */
if( strcmp( unittyp, "linear" )==0 ) { /* avoid using Euprint()- trouble w/ v. big or v. small #s */
	sprintf( smin, floatformat, min );
	sprintf( smax, floatformat, max );
	}
else	{
	Euprint( smin, axis, min, "" );
	Euprint( smax, axis, max, "" );
	}


if( strcmp( nearest, "exact" )==0 ) {  /* exact */
	sprintf( minval, "%s", smin ); 
	sprintf( maxval, "%s", smax ); 
	}



else if( strnicmp( nearest, "month", 5 )== 0 || strnicmp( nearest, "quarter", 8 )==0 ) { 
	/* nearest month boundary / quarter-year boundary.. */
	int mon, day, yr, newmon;
	long l;
	if( !GL_smember( unittyp, "date datetime" )) 
		Eerr( 2892, "autorange 'nearest=month' or 'nearest=quarter' only valid with date or datetime scaletype", unittyp );
	/* min */
	stat = DT_jdate( smin, &l );
        DT_getmdy( &mon, &day, &yr );
	if( tolower( nearest[0] ) == 'q' ) {
		if( mon >= 10 ) mon = 10;
		else if( mon >= 7 ) mon = 7;
		else if( mon >= 4 ) mon = 4;
		else if( mon >= 1 ) mon = 1;
		}
	DT_makedate( yr, mon, 1, "", datepart );
	if( strcmp( unittyp, "datetime" )==0 ) {
		DT_maketime( 0, 0, 0.0, timepart );
		sprintf( minval, "%s.%s", datepart, timepart );
		}
	else strcpy( minval, datepart );

	/* max */
	stat = DT_jdate( smax, &l );
        DT_getmdy( &mon, &day, &yr );
	if( tolower( nearest[0] ) == 'q' ) {
		if( mon <= 3 ) mon = 4;
		else if( mon <= 6 ) mon = 7;
		else if( mon <= 9 ) mon = 10;
		else if( mon <= 12 ) mon = 13;
		}
	else mon ++;

        /* wrap around year.. */
        newmon = ((mon-1) % 12 ) +1;
        yr += ((mon-1) / 12);
        mon = newmon;
	DT_makedate( yr, mon, 1, "", datepart );
	if( strcmp( unittyp, "datetime" )==0 ) {
		DT_maketime( 0, 0, 0.0, timepart );
		sprintf( maxval, "%s.%s", datepart, timepart );
		}
	else strcpy( maxval, datepart );
	}


else if( strnicmp( nearest, "year", 4 )== 0 ) { /* nearest year boundary.. */
	int mon, day, yr;
	long l;
	if( !GL_smember( unittyp, "date datetime" )) 
		Eerr( 2892, "autorange 'nearest=year' only valid with date or datetime scaletype", unittyp );

	/* min */
	stat = DT_jdate( smin, &l );
        DT_getmdy( &mon, &day, &yr );
	DT_makedate( yr, 1, 1, "", datepart );
	if( strcmp( unittyp, "datetime" )==0 ) {
		DT_maketime( 0, 0, 0.0, timepart );
		sprintf( minval, "%s.%s", datepart, timepart );
		}
	else strcpy( minval, datepart );
		
	/* max */
	stat = DT_jdate( smax, &l );
        DT_getmdy( &mon, &day, &yr );
	DT_makedate( yr+1, 1, 1, "", datepart );
	if( strcmp( unittyp, "datetime" )==0 ) {
		DT_maketime( 0, 0, 0.0, timepart );
		sprintf( maxval, "%s.%s", datepart, timepart );
		}
	else strcpy( maxval, datepart );
	}

else if( strnicmp( nearest, "day", 3 )== 0 ) { /* nearest day boundary.. */
	int mon, day, yr;
	double days;
	if( !GL_smember( unittyp, "datetime" )) 
		Eerr( 2892, "autorange 'nearest=day' only valid with datetime scaletype", unittyp );
	/* min */
	DT_datetime2days( smin, &days );
        DT_getmdy( &mon, &day, &yr );
	DT_makedate( yr, mon, day, "", datepart );
	DT_maketime( 0, 0, 0.0, timepart );
	sprintf( minval, "%s.%s", datepart, timepart );
	/* max */
	DT_datetime2days( smax, &days );
	DT_days2datetime( days+1.0, smax ); 
	DT_datetime2days( smax, &days ); /* set next day's date for getmdy below*/
        DT_getmdy( &mon, &day, &yr );
	DT_makedate( yr, mon, day, "", datepart );
	DT_maketime( 0, 0, 0.0, timepart );
	sprintf( maxval, "%s.%s", datepart, timepart );
	}

else if( strnicmp( nearest, "hour", 4 )== 0 ) { /* nearest hour boundary.. */
	int hour, minute;
	double sec;
	if( !GL_smember( unittyp, "time datetime" )) 
		Eerr( 2892, "autorange 'nearest' is incompatible with scaletype", unittyp );

	if( strcmp( unittyp, "time" )==0 ) {
		/* min */
		DT_tomin( smin, &sec ); /* sec not used */
		DT_gethms( &hour, &minute, &sec );
		DT_maketime( hour, 0, 0.0, minval );
		/* max */
		DT_tomin( smax, &sec ); /* sec not used */
		DT_gethms( &hour, &minute, &sec );
		if( minute == 0 && sec == 0.0 )DT_maketime( hour, 0, 0.0, maxval );
		else DT_maketime( hour+1, 0, 0.0, maxval );
		}
	else if( strcmp( unittyp, "datetime" )==0 ) {
		double days;
		int mon, day, yr;

		/* min */
		DT_datetime2days( smin, &days );
		/* time part */
		DT_gethms( &hour, &minute, &sec );
		DT_maketime( hour, 0, 0.0, timepart );
		/* date part */
        	DT_getmdy( &mon, &day, &yr );
		DT_makedate( yr, mon, day, "", datepart );
		sprintf( minval, "%s.%s", datepart, timepart );

		/* max */
		DT_datetime2days( smax, &days );
		/* time part */
		DT_gethms( &hour, &minute, &sec );
		if( hour == 23 ) {
			DT_days2datetime( days+1.0, smax ); /* set next day's date for getmdy below*/
			DT_datetime2days( smax, &days ); /* set next day's date for getmdy below*/
			DT_maketime( 0, 0, 0.0, timepart );
			}
		else DT_maketime( hour+1, 0, 0.0, timepart );
		/* date part */
        	DT_getmdy( &mon, &day, &yr );
		DT_makedate( yr, mon, day, "", datepart );
		sprintf( maxval, "%s.%s", datepart, timepart );
		}
	}

else 	{  /* if( strcmp( nearest, "auto" )==0 ) {  */  /* this section added scg 7/5/00 */
	double inc, h, fmod(), a, b;

	if( strcmp( nearest, "auto" )==0 ) defaultinc( min, max, &inc );
	else inc = atof( nearest );

	h = fmod( min, inc );

	a = (min - h) - inc;
	b = a - inc;    /* include one extra inc on low end */
	if( a >= 0.0 && b < 0.0 ) b = a;  /* but don't dip below 0 */

	if( min < 0.0 ) sprintf( minval, floatformat, (min - h) - (inc * incmult) ); /* include extra inc on low end - scg 11/29/00 */
	else 	{
		a = min - h;
		b = a - (inc*(incmult-1.0)); /* include extra inc on low end - 11/29 */
		if( a >= 0.0 && b < 0.0 ) b = a;  /* but don't dip below 0  - 11/29 */
		sprintf( minval, floatformat, b );
		}

	h = fmod( max, inc );
	if( max < 0.0 ) sprintf( maxval, floatformat, (max - h) + (inc*(incmult-1.0)) ); /* include extra inc on high end - 11/29 */
	else sprintf( maxval, floatformat, (max - h) + (inc * incmult) ); /* extra inc - 11/29 */
	}
	

/* lowfix and hifix overrides.. */
if( lowfix[0] != '\0' ) strcpy( minval, lowfix );
if( hifix[0] != '\0' ) strcpy( maxval, hifix );

if( Debug )
  fprintf( Diagfp, "Autorange on %c: min=%s to max=%s\n", axis, minval, maxval);

suppress_twin_warn( 1 ); /* suppress complaints about datetime outside of window 
				until after areadef */
return( 0 );
}
