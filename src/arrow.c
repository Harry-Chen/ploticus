/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

#include "plg.h"

#define DELT_THETA 0.4

/* ARROW - draw an arrow from (x1,y1) inches to (x2,y2) inches with arrowhead at (x2,y2).
   Uses the current line type, thickness, etc.
   The parameter 'r' controls the length of the arrowhead in inches (suggested default 0.1).
   If r is 0.0 then no arrowhead will be rendered.
   w is the theta controlling the arrow head width (suggested default is 0.3)
   The 'color' parameter controls the color of the arrowhead (only).
*/
   
PLG_arrow( x1, y1, x2, y2, r, w, color )
double x1, y1, x2, y2, r, w;
char *color;
{
double vx, vy, ax1, ay1, ax2, ay2, th0, th1, th2, atan();
double cos(), sin();
vx = x2 - x1;
vy = y2 - y1;

if( r <= 0.0 || w <= 0.0 ) goto LINEONLY;

if( vx == 0.0 && y2 > y1 ) th0 = 3.141593 / 2.0; /* avoid divide by zero - scg */
else if( vx == 0.0 && y1 > y2 ) th0 = -3.141593 / 2.0; /* avoid divide by zero - scg */
else th0 = atan( vy / vx );

th1 = th0 + w;
th2 = th0 - w;



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

if( color[0] == '\0' ) Ecolorfill( Ecurcolor );
else Ecolorfill( color );

LINEONLY:
Emov( x1, y1 );
Elin( x2, y2 );
}
