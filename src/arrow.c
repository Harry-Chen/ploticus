/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

#include "graphcore.h"
#define DELT_THETA 0.4

/* Draws an arrow from (x1,y1) inches to (x2,y2) inches with point at (x2,y2).
   Uses the current line type, thickness, etc.
   The parameter 'r' controls the length of the arrowhead in inches, and can be
   sent as 0.0 to use the default (0.2).  The 'color' parameter controls the
   color of the arrowhead (only).
*/
   
Earrow( x1, y1, x2, y2, r, color )
double x1, y1, x2, y2, r;
char *color;
{
double vx, vy, ax1, ay1, ax2, ay2, th0, th1, th2, atan();
double cos(), sin();
vx = x2 - x1;
vy = y2 - y1;

if( vx == 0.0 && y2 > y1 ) th0 = 3.141593 / 2.0; /* avoid divide by zero - scg */
else if( vx == 0.0 && y1 > y2 ) th0 = -3.141593 / 2.0; /* avoid divide by zero - scg */
else th0 = atan( vy / vx );

th1 = th0 + DELT_THETA;
th2 = th0 - DELT_THETA;

if( r <= 0.0 ) r = 0.2;


if( x2 < x1 ) {
	ax1 = x2 + (r * cos( th1 ));
	ay1 = y2 + (r * sin( th1 ));
	ax2 = x2 + (r * cos( th2 ));
	ay2 = y2 + (r * sin( th2 ));
	}
else 	{
	ax1 = x2 - (r * cos( th1 ));
	ay1 = y2 - (r * sin( th1 ));
	ax2 = x2 - (r * cos( th2 ));
	ay2 = y2 - (r * sin( th2 ));
	}

Emov( x2, y2 );
Epath( ax1, ay1 );
Epath( ax2, ay2 );
Ecolorfill( color );
Emov( x1, y1 );
Elin( x2, y2 );
}
