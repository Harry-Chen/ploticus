/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


#include "pl.h"

do_x_button( label )
char *label;
{
double x, y;
int e;
#ifndef NOX11
while( 1 ) {
	if( doingmapdebug() ) showmapfile( 'x' ); /* added scg 11/23/01 */
	Ecolor( "black" );
	Elinetype( 0, 0.5, 1.0 );
	Ecblock( 0.1, 0.1, 1.0, 0.3, "yellow", 0 );
	Emov( 0.5, 0.12 );
	Etextsize( 12 );
	Ecentext( label );
	Ecblockdress( 0.1, 0.1, 1.0, 0.3, 0.06, "0.6", "0.8", 0.0, "" );
	Esavewin();
        Egetkey( &x, &y, &e );
	if( e < 1000 || (y < 0.3 && x < 1.0 )) {
              	Ecblock( 0.1, 0.1, 1.0, 0.3, "black", 0 ); 
		Eflush();
		if( e == 'q' ) exit(1); /* scg 6/28/99 */
                break;
                }
        else fprintf( Diagfp, "Mouse click location (x,y inches): %g %g \n", x, y );
	}
#endif
}
