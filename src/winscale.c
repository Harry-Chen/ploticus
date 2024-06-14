/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


static double xscale, yscale;
static int window_height, window_width;

/* ====================== set up user coords */
Esetwinscale( width, height, x_max, y_max )
int width, height;
double x_max, y_max;
{
xscale = (double)(width) / (x_max);
yscale = (double)(height) / (y_max);
window_height = height;
window_width = width;
}
/* ==================== scale in x to window size */
Exsca( f )
double f;
{
int iout;
iout = (int) (f * xscale);
return( iout );
}

/* ==================== inverse of xsca */
double Exsca_inv( i )
int i;
{
double out;
out = (double) i / xscale;
return( out );
}

/* ==================== scale in y to window size */
Eysca( f )
double f;
{
int iout;
iout = window_height - (int) (f * yscale);
return( iout );
}

/* ===================== inverse of ysca */
double Eysca_inv( i )
int i;
{
double out;
out = (double)(window_height-i) / yscale;
return( out );
}

