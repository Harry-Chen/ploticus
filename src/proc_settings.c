/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC SETTINGS - date, unit, notation settings */

#include "pl.h"

PLP_settings()
{
int i;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

int stat;
int align;
double adjx, adjy;


TDH_errprog( "settings" );

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( stricmp( attr, "format" )==0 || stricmp( attr, "dateformat" )==0 ) DT_setdatefmt( val );

	else if( stricmp( attr, "units" )==0 ) {
		if( tolower( val[0] ) == 'c' ) { Using_cm = 1; setintvar( "CM_UNITS", 1 ); }
		else { Using_cm = 0; setintvar( "CM_UNITS", 0 ); }
		}

#ifndef WIN32
	else if( stricmp( attr, "uid" )==0 ) setuid( atoi( val ) );
        else if( stricmp( attr, "gid" )==0 ) setgid( atoi( val ) );
#endif
#ifndef NORLIMIT
	else if( stricmp( attr, "cpulimit" )==0 ) reslimits( "cpu", atoi( val ) );
        else if( stricmp( attr, "filesizelimit" )==0 ) reslimits( "filesize", atoi( val ) );
#endif

	else if( stricmp( attr, "numbernotation" )==0 ) {
		if( stricmp( val, "us" )==0 ) Bignumspacer = ',';
		else if( stricmp( val, "euro" )==0 ) Bignumspacer = '.';
		else Bignumspacer = '\0';
		}
	else if( stricmp( attr, "numberspacerthreshold" )==0 ) Bignumspacer_thres = atoi( val ); /* scg 2/28/02 */

	else if( GL_slmember( attr, "pivotyear months* weekdays omitweekends lazydates" )) {
                stat = DT_setdateparms( attr, lineval );
		}

	else Eerr( 1, "attribute not recognized", attr );
	}

return( 0 );
}
