/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* DATETIME - datetime routines */
#include <stdio.h>


static double Dtwinbegin = 0.0, Dtwinend = 1440.0, Dtwinsize = 1440.0; /* default: 24 hours */
static int Suppress_twin_warn = 0;


/* ================================== */
/* SETDATETIMEFMT - set the format for the date and time 
   	components of datetime */
DT_setdatetimefmt( fmt, window )
char *fmt;
char *window; /* Specifies a 'window' of hours to be shown for each day.
		 Notation nn-mm where nn is hour to begin with and mm is 
		hour to end with (0-24) */
{
char datepart[20], timepart[20];
char beginpart[20], endpart[20];
int i;

i = 0;
GL_getchunk( datepart, fmt, &i, "." );
GL_getchunk( timepart, fmt, &i, "." );
DT_setdatefmt( datepart );
DT_settimefmt( timepart );

if( window[0] != '\0' ) { /* parse 'window' specification */
	i = 0;
	GL_getchunk( beginpart, window, &i, "-" );
	GL_getchunk( endpart, window, &i, "-" );
	Dtwinbegin = atoi( beginpart ) * 60.0;
	Dtwinend = atoi( endpart ) * 60.0;
	Dtwinsize = (Dtwinend - Dtwinbegin);
	}
if( window[0] == '\0' || Dtwinsize == 0.0 ) {
	Dtwinbegin = 0.0;
	Dtwinend = 1440.0;
	Dtwinsize = 1440.0;
	}

return( 0 );
}


/* ================================== */
/* DATETIME2DAYS - convert a datetime to julian date with 
	decimal portion representing the time component.

   Returns 0 if ok, 1 on error */
DT_datetime2days( dt, days )
char *dt;
double *days;
{
char datepart[20], timepart[20];
long jul;
int i, stat;
double mins;

i = 0;
GL_getchunk( datepart, dt, &i, "." );
GL_getchunk( timepart, dt, &i, "." );
stat = DT_jdate( datepart, &jul );
if( stat != 0 ) return( 1 );

stat = DT_tomin( timepart, &mins );
if( stat != 0 ) return( 1 );

if( mins < Dtwinbegin || mins > Dtwinend ) {
	if( !Suppress_twin_warn ) 
	   TDH_err( 4279, "warning: time is outside of window range- displaying it at window boundary", dt );
	if( mins < Dtwinbegin ) mins = Dtwinbegin;
	if( mins > Dtwinend ) mins = Dtwinend;
	}

mins = (mins-Dtwinbegin) / Dtwinsize; /* normalize to  0.0 - 1.0 */
*days = jul+mins;
return( 0 );
}
/* ================================== */
/* DAYS2DATETIME - convert decimal julian date to datetime */
DT_days2datetime( days, dt )
double days;
char *dt;
{
char s[30], t[30];
double datepart, timepart, f, floor(), fmod();

datepart = floor( days );
timepart = days - datepart;

timepart = (timepart * Dtwinsize) + Dtwinbegin; /* convert from 0.0 - 1.0 to #min past midnite */
timepart = floor( timepart + 0.5 ); /* remove any rounding error introduced by above */

DT_fromjul( (long)datepart, s );
DT_frommin( timepart, t );

sprintf( dt, "%s.%s", s, t );
return( 0 );
}
/* ================================== */
/* FORMATDATETIME - convert a datetime from current format 
	to a new format for display.  Either the date or
	time component may be omitted for display purposes.
*/
DT_formatdatetime( dt, format, newdt )
char *dt, *format, *newdt;
{
char datepart[20], timepart[20];
char datefmt[20], timefmt[20];
char s[30], t[30];
int i;

i = 0;
strcpy( datefmt, "" );
strcpy( timefmt, "" );
if( format[0] != '.' && tolower(format[0]) != 'h' ) GL_getchunk( datefmt, format, &i, "." );
GL_getchunk( timefmt, format, &i, "." );

i = 0;
GL_getchunk( datepart, dt, &i, "." );
GL_getchunk( timepart, dt, &i, "." );

if( datefmt[0] != '\0' ) {
	DT_formatdate( datepart, datefmt, s );
	}
else strcpy( s, "" );
if( timefmt[0] != '\0' ) {
	if( datefmt[0] != '\0' )strcpy( t, "." );	
	else strcpy( t, "" );
	DT_formattime( timepart, timefmt, &t[strlen(t)] );
	}
else strcpy( t, "" );

sprintf( newdt, "%s%s", s, t );

return( 0 );
}

/* ================================= */
/* SUPPRESS_TWIN_WARN - allow warning message 'value outside datetime
	window' to be suppressed */
suppress_twin_warn( mode )
int mode;
{
Suppress_twin_warn = mode;
return( 0 );
}


#ifdef TESTING
/* ================================== */
main()
{
double min;
char result[80];
char result2[80];

DT_tomin( "now", &min );
DT_frommin( min, result );
DT_formattime( result, "hh:mmA", result2 );
printf( "%s\n", result2 );
}
#endif


