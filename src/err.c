/* ERR.C
 * Copyright 1998-2002 Stephen C. Grubb  (ploticus.sourceforge.net) .
 * This code is covered under the GNU General Public License (GPL);
 * see the file ./Copyright for details. */


/* error msg handler */
#include <stdio.h>

static char emode[20] = "stderr";
static char progname[80] = "";
static char errlog[256] = "";

/* ========================================== */
TDH_err_initstatic()
{
strcpy( emode, "stderr" );
strcpy( progname, "" );
strcpy( errlog, "" );
return( 0 );
}

/* ========================================== */

TDH_err( errno, msg, parm )
int errno;
char *msg, *parm;
{
char op[4], cp[4];
char *getenv(), *s;
int say_error;

strcpy( op, "" );
strcpy( cp, "" );
if( parm[0] != '\0' ) {
	strcpy( op, "(" );
	strcpy( cp, ")" );
	}

if( GL_slmember( msg, "note:* warning:*" )) say_error = 0;
else say_error = 1;

if( strcmp( emode, "stdout" )==0 || strcmp( emode, "cgi" )==0 ) {
	printf( "<br><h4>%s %s %d: %s %s%s%s</h4><br>\n", progname, (say_error)?"error":"", errno, msg, op, parm, cp );
	fflush( stdout );
	}

else 	{
	fprintf( stderr, "%s %s %d: %s %s%s%s\n", progname, (say_error)?"error":"", errno, msg, op, parm, cp );
	fflush( stderr );
	}

/* if an error log file was specified, write a line to this file .. */
if( errlog[0] != '\0' ) {
	FILE *logfp;
	int mon, day, yr, hr, min, sec;
	GL_sysdate( &mon, &day, &yr );
	GL_systime( &hr, &min, &sec );

	logfp = fopen( errlog, "a" );
	if( logfp != NULL ) {
		fprintf( logfp, "%02d/%02d/%02d %02d:%02d:%02d error %d: %s (%s)\n", 
                	yr, mon, day, hr, min, sec, errno, msg, parm );
		fclose( logfp );
		}
	}

return( errno );
}



TDH_errprog( prog )
char *prog; 
{
strcpy( progname, prog);
return( 0 );
}

TDH_geterrprog( prog )
char *prog; 
{
strcpy( prog, progname );
return( 0 );
}

TDH_errmode( mode )
char *mode;
{
strcpy( emode, mode );
return( 0 );
}

TDH_geterrmode( mode )
char *mode;
{
strcpy( mode, emode );
return( 0 );
}

TDH_errlogfile( filename )
char *filename;
{
strcpy( errlog, filename );
return( 0 );
}
