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
		if( tolower( val[0] ) == 'c' ) { PLS.usingcm = 1; setintvar( "CM_UNITS", 1 ); }
		else { PLS.usingcm = 0; setintvar( "CM_UNITS", 0 ); }
		}

#ifndef WIN32
	else if( stricmp( attr, "uid" )==0 ) setuid( atoi( val ) );
        else if( stricmp( attr, "gid" )==0 ) setgid( atoi( val ) );
#endif
#ifndef NORLIMIT
	else if( stricmp( attr, "cpulimit" )==0 ) TDH_reslimits( "cpu", atoi( val ) );
#endif

	else if( stricmp( attr, "numbernotation" )==0 ) {
		if( stricmp( val, "us" )==0 ) PLS.bignumspacer = ',';
		else if( stricmp( val, "euro" )==0 ) PLS.bignumspacer = '.';
		else PLS.bignumspacer = '\0';
		}
	else if( stricmp( attr, "numberspacerthreshold" )==0 ) PLS.bignumthres = atoi( val ); /* scg 2/28/02 */

	else if( GL_slmember( attr, "pivotyear months* weekdays omitweekends lazydates" )) {
                stat = DT_setdateparms( attr, lineval );
		}

	else if( stricmp( attr, "font" )==0 ) strcpy( Estandard_font, lineval );

#ifndef NOSVG
	else if( stricmp( attr, "xml_encoding" )==0 ) PLGS_setxmlparms( "encoding", val );
        else if( stricmp( attr, "xml_declaration" )==0 ) {
                if( stricmp( val, YESANS )==0 ) PLGS_setxmlparms( "xmldecl", "1" );
                else PLGS_setxmlparms( "xmldecl", "0" );
                }
	else if( stricmp( attr, "svg_tagparms" )==0 ) PLGS_setxmlparms( "svgparms", lineval );
#endif

	else if( stricmp( attr, "dtsep" )==0 ) DT_setdtsep( val[0] );

	else Eerr( 1, "attribute not recognized", attr );
	}

return( 0 );
}
