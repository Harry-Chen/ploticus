/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


#include "plg.h"
#include "pl.h"


struct plstate PLS;
struct pldata PLD;
struct proclines PLL;
double *PLV;
int PLVsize, PLVhalfsize, PLVthirdsize;
char PL_bigbuf[ MAXBIGBUF ];	/* general purpose large buffer - don't rely 
				on content integrety across procs */

