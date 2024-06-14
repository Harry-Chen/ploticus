/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


#include "pl.h"

static int Supress_convmsg = 0;
static int N_convmsg = 0;

/* ====================== */
/* DA - access D as a 2-D array; return as char *    */
char *
da( r, c )
int r, c;
{
int base;
base = StartD[Dsel];
if( r >= Nrecords[Dsel] ) { 
	fprintf( Errfp, "warning: %s data reference (rec=%d field=%d) is out of bounds\n", 
		Eprogname, r+1, c+1 ); 
	return( "" );
	}
if( c >= Nfields[Dsel] ) { 
	fprintf( Errfp, "warning: %s data reference (rec=%d field=%d) is out of bounds\n", 
		Eprogname, r+1, c+1 ); 
	return( "" );
	}
return( D[ base + ( r * Nfields[Dsel] ) + c ] );
}
/* ====================== */
/* FDA - access D as a 2-D array; return as double; 
   For non-plottables 0.0 is returned, but Econv_error may be called to see if 
   there was a conversion error */
double fda( r, c, ax )
int r, c;
char ax;
{
return( Econv( ax, da( r, c ) ) );
}

/* ====================== */
/* NUM - convert string to double, and ensure that it is proper numeric
   Result is s converted to double.
   Return is 0 on ok, 1 on non-numeric. */
num( s, result )
char s[];
double *result;
{
int i;
int nt;

nt = sscanf( s, "%lf", result );
if( nt > 0 ) return( 0 );
else	{
	*result = 0.0;
	return( 1 );
	}
}

/* ============================= */
/* GETCOORDS - extract a coordinate pair (two posex's) from val */
getcoords( parmname, val, x, y )
char *parmname, *val;
double *x, *y;
{
char px[40], py[40];
int nt;
int stat;

nt = sscanf( val, "%s %s", px, py );
if( nt < 2 ) return( Eerr( 55, "Two values expected", parmname ) );

stat = Eposex( px, X, x );
stat += Eposex( py, Y, y );
if( stat != 0 ) return( Eerr( 201, "Error on coord pair", parmname ) );
else return( 0 );
}

/* ============================= */
/* GETBOX - extract two coordinate pairs (four posex's) from val */
getbox( parmname, val, x1, y1, x2, y2 )
char *parmname, *val;
double *x1, *y1, *x2, *y2;
{
char px1[40], py1[40], px2[40], py2[40];
int nt;
int stat;

nt = sscanf( val, "%s %s %s %s", px1, py1, px2, py2 );
if( nt < 4 ) return( Eerr( 55, "Four values expected", parmname ) );
stat = Eposex( px1, X, x1 );
stat += Eposex( py1, Y, y1 );
stat += Eposex( px2, X, x2 );
stat += Eposex( py2, Y, y2 );
if( stat != 0 ) return( Eerr( 201, "Error on box specification", parmname ) );
else return( 0 );
}


/* ================== */
/* GETRANGE - get a low/high range */
getrange( lineval, lo, hi, ax, deflo, defhi )
char *lineval;
double *lo, *hi;
char ax;
double deflo, defhi;
{
char s1[80], s2[80];
int nt;
nt = sscanf( lineval, "%s %s", s1, s2 );
if( nt == 2 ) {
	*lo = Econv( ax, s1 );
	*hi = Econv( ax, s2 );
	return( 0 );
	}
else if( nt == 1 ) {
	*lo = Econv( ax, s1 );
	*hi = defhi;
	return( 0 );
	}
else if( nt <= 0 ) {
	*lo = deflo;
	*hi = defhi;
	return( 0 );
	}
}
	

/* =============================== */
/* FILE_TO_BUF - read file or execute command and place contents into buf.
   Shell expandable file name is ok.
   Returns 0 if ok, 1 if file not available */

file_to_buf( filename, mode, result, buflen )
char *filename;
int mode; /* 1 = file   2 = command */
char *result;
int buflen;
{
FILE *fp, *popen();
char buf[1000];

if( mode == 1 ) fp = fopen( filename, "r" );
else fp = popen( filename, "r" );
if( fp == NULL ) return( 1 );
strcpy( result, "" );
while( fgets( buf, 999, fp ) != NULL ) {
	if( strlen( result ) + strlen( buf ) >= buflen ) {
		Eerr( 7254, "warning: truncated, capacity reached", filename );
		return( 2 );
		}
	strcat( result, buf );
	}
if( mode == 1 ) fclose( fp );
else pclose( fp );
return( 0 );
}

/* ========================= */
/* SETFLOATVAR - set a DMS var to a double value */
setfloatvar( varname, f )
char *varname;
double f;
{
char buf[80];
int stat;
sprintf( buf, "%g", f );
stat = TDH_setvar( varname, buf );
if( stat != 0 ) return( Eerr( 3890, "Error on setting variable", varname ) );
return( 0 );
}
/* ========================= */
/* SETINTVAR - set a DMS var to an integer value */
setintvar( varname, n )
char *varname;
int n;
{
char buf[80];
int stat;
sprintf( buf, "%d", n );
stat = TDH_setvar( varname, buf );
if( stat != 0 ) return( Eerr( 3892, "Error on setting variable", varname ) );
return( 0 );
}

/* ========================== */
/* SETCHARVAR - set a DMS var to a char string value */
setcharvar( varname, s )
char *varname;
char *s;
{
int stat;
stat = TDH_setvar( varname, s );
if( stat != 0 ) return( Eerr( 3894, "Error on setting variable", varname ) );
return( 0 );
}

/* =========================== */
/* CONV_MSG - print a message to Errfp for a data item of invalid type */
conv_msg( row, col, aname )
int row, col;
char *aname;
{
N_convmsg++;
if( Supress_convmsg ) return( 0 );
fprintf( Errfp, "%s warning, %s skipping unplottable '%s' (rec=%d field=%d)\n",
	Eprogname, aname, da(row,col), row+1, col+1 );
return( 0 );
}

/* =========================== */
/* OTH_MSG - print a message to Errfp for a data item of invalid type */
oth_msg( msg )
char *msg;
{
N_convmsg++;
if( Supress_convmsg ) return( 0 );
fprintf( Errfp, "%s %s\n", Eprogname, msg );
return( 0 );
}

/* =========================== */

/* =========================== */
/* SUPPRESS_CONVMSG - suppress invalid type messages */
suppress_convmsg( mode )
int mode;
{
Supress_convmsg = mode;
return( 0 );
}

/* ============================= */
/* ZERO_CONVMSGCOUNT - zero out the conv msg counter */
zero_convmsgcount()
{
N_convmsg = 0;
return( 0 );
}
/* ============================= */
/* REPORT_CONVMSGCOUNT - report on what the conv msg count is.. */
report_convmsgcount()
{
return( N_convmsg );
}



/* ========================== */
/* SCALEBEENSET - return 1 if scaling has been set, 0 if not */
scalebeenset()
{
if( EDXhi - EDXlo > 0.0000001 ) return( 1 );
else return( 0 );
}

/* ===================== */
/* CATITEM - add a new data item */
catitem( s, d, avail )
char *s;
int *d;
int *avail;
{
D[ (*d)++ ] = &Databuf[ *avail ];
strcpy( &Databuf[ (*avail) ], s );
(*avail) += (strlen( s ) + 1);
return( 0 );
}

/* ======================== */
/* DEFAULTINC - given a min and a max, estimate a reasonable default inc */
static double threshvals[40] = { 0.000001, 0.00001, 0.0001, 0.001, 0.002, 0.005,
	0.01, 0.02, 0.05, 
	0.1, 0.2, 0.5, 
	1.0, 2.0, 5.0, 
	10.0, 20.0, 50.0, 
	100.0, 200.0, 500.0, 
	1000.0, 2000.0, 5000.0, 
	10000.0, 20000.0, 50000.0, 
	100000.0, 200000.0, 500000.0, 
	1000000.0, 2000000.0, 5000000.0, 
	10000000.0, 20000000.0, 50000000.0 };

defaultinc( min, max, inc )
double min, max, *inc;
{
double diff, h, fabs();
int i;

diff = max - min;
diff = fabs( diff );
h = diff / 10.0;  /* we want to have about 10 tics on an axis.. */

for( i = 0; ; i++ ) {
	if( h < threshvals[i] ) break;
	if( threshvals[i] > 49999999.0 ) break;
	}

*inc = threshvals[i];
return( 0 );
}


/* ======================== */
/* REWRITENUMS - rewrite numbers, supplying a spacer (comma in US)
	every 3 zeros in large numbers.  If spacer is dot (.), European
	mode is presumed, decimal point is written as a comma, and
	large numbers are spaced using dot.   

 	Parameter num is modified.
*/

rewritenums( num )
char *num;
{
int i, j, k, decplace, len;
char s[40], tmp[40];

if( Bignumspacer == '\0' ) return( 0 ); /* do nothing */

sscanf( num, "%s", s ); /* strip off any leading spaces */

/* find any dec pt; convert it; put result into 'tmp'.. */
k = -1;
decplace = -1;
for( i = 0, len = strlen( s ); i < len; i++ ) {
	/* remember the char where number begins.. */
	if( k < 0 && ( isdigit( s[i] ) || s[i] == '.' || s[i] == '-' ) ) k = i;
	if( s[i] == '.' ) {
		decplace = i - k;
		if( Bignumspacer == '.' ) tmp[i] = ',';
		else tmp[i] = '.';
		}
	else tmp[i] = s[i];
	}
if( decplace < 0 ) decplace = i - k;
tmp[i] = '\0';

if( decplace > Bignumspacer_thres ) {   /* add spacers.. */
	/* process tmp; put result back in 's'.. */
	for( i = -1, j = 0, k = 0; i < decplace; k++ ) {
		/* just copy over any leading alpha portion.. */
		if( i < 0 && ( ! isdigit( tmp[k] ) && tmp[k] != '.' && tmp[k] != '-' ) ) {
			s[j++] = tmp[k];
			continue; /* scg 2/28/02 */
			}

		i++;
		if( i > 0 && decplace-i > 0 && (decplace-i) % 3 == 0 ) {  /* insert 000 separator.. */
			s[j++] = Bignumspacer;
			s[j++] = tmp[k];
			}
		else s[j++] = tmp[k];
		}
	s[j] = '\0';
	strcat( s, &tmp[k] ); /* append decimal point and rightward */
	}
else strcpy( s, tmp );

strcpy( num, s );
return( 0 );
} 

/* ============================= */
/* CONVERTNL - change all occurrances of "\n" (backslash followed by n) to a newline character */
convertnl( str )
char *str;
{
int i, j, len;
for( i = 0, j = 0, len = strlen( str ); i < len; i++ ) {
	if( str[i] == '\\' && str[i+1] == 'n' ) {
		str[j] = '\n';
		i++;
		}
	else str[j] = str[i];
	j++;
	}
str[j] = '\0';
return( 0 );
}
/* ============================== */
/* MEASURETEXT - count the number of lines present in txt and find the length of longest line. */
measuretext( txt, nlines, maxlen )
char *txt;
int *nlines, *maxlen;
{
int i, len, tlen;
char line[256];

i = 0;
*nlines = 0;
*maxlen = 0;
tlen = strlen( txt );
while( 1 ) {
	if( i >= tlen ) break;
	GL_getseg( line, txt, &i, "\n" );
	len = strlen( line );
	if( len > *maxlen ) *maxlen = len;
	(*nlines)++;
	}
return( 0 );
}

/* =================================== */
/* STRIP_WS strip white-space off of front and end of string s */
strip_ws( s )
char s[];
{
int i, j, len;

/* don't do anything if first and last characters are non-space.. */
if( !isspace( s[0] ) && !isspace( s[ strlen( s ) - 1 ] ) ) return( 0 );
 

/* find last significant char and put a null after it */
for( j = strlen( s ) -1; j >= 0; j-- )
	if( !GL_member( s[j], " \t\n" )) break;
s[j+1] = '\0';
/* find 1st significant char at position i */
for( i = 0, len = strlen( s ); i < len; i++ ) 
	if( !GL_member( s[i], " \t\n" )) break; 
strcpy( s, &s[i] );
}
