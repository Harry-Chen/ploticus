/* ERR.C
 * Copyright 1998-2002 Stephen C. Grubb  (ploticus.sourceforge.net) .
 * This code is covered under the GNU General Public License (GPL);
 * see the file ./Copyright for details. */


/* error msg handler */
#include <stdio.h>

static char emode[20] = "stderr";
static char progname[20] = "";


TDH_err( errno, msg, parm )
int errno;
char *msg, *parm;
{
char op[4], cp[4];
char *getenv(), *s;

/* allow error mode to be set from environment */
s = getenv( "TDHKIT_ERRMODE" );
if( s != NULL ) {
	strcpy( emode, s );
 	}

strcpy( op, "" );
strcpy( cp, "" );
if( parm[0] != '\0' ) {
	strcpy( op, "(" );
	strcpy( cp, ")" );
	}


if( strcmp( emode, "cgi" )==0 ) 
	printf( "<br><h4>%s error %d: %s %s%s%s</h4><br>\n", progname, errno, msg, op, parm, cp );

else fprintf( stderr, "%s error %d: %s %s%s%s\n", progname, errno, msg, op, parm, cp );

return( errno );
}



TDH_errprog( prog )
char *prog; 
{
strcpy( progname, prog);
return( 0 );
}
