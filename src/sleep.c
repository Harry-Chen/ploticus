/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* Suspend process for an interval in microseconds (us) */
/* Returns 0 if ok; otherwise an error code.. */

Eusleep( us )
long us;
{
usleep( us );
return( 0 );
}
