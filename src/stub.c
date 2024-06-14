/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


#include "graphcore.h"
#include <ctype.h>
/* Low level elib drawing stubs.. */
/* These issue driver requests only when a parameter actually changes */

/* ======================================== */
Eclr()
{
double atof();
Epcode( 'z', 0.0, 0.0, Ecurbkcolor );
}
/* ======================================== */
/* move to x, y absolute */
Emov( x , y )
double x, y;
{
if( Eflip ) Epcode( 'M', (double)y , (double)x, "" );
else Epcode( 'M', (double)x , (double)y, "" );
}
/* ======================================== */
/* line to x, y absolute */
Elin( x , y )
double x, y;
{
if( Eflip ) Epcode( 'L', (double)y , (double)x, "" );
else Epcode( 'L', (double)x , (double)y, "" );
}
/* ======================================== */
/* path to x, y absolute (form a polygon to be shaded later) */
Epath( x, y )
double x, y;
{
if( Eflip ) Epcode( 'P', (double)y , (double)x, "" );
else Epcode( 'P', (double)x , (double)y, "" );
}
 

#ifdef CUT
/* ======================================== */
/* move to x, y data */
Emovu( x , y )
double x, y;
{
if( Eflip ) Epcode( 'M', Eax((double) y ) , Eay((double) x ), "" );
else Epcode( 'M', Eax((double) x ) , Eay((double) y ), "" );
}
/* ======================================== */
/* line to x, y data */
Elinu( x , y ) 
double x, y;
{
if( Eflip ) Epcode( 'L', Eax((double) y ) , Eay((double) x ), "" );
else Epcode( 'L', Eax((double) x ) , Eay((double) y ), "" );
}
/* path to x, y data (form a polygon to be shaded later) */
#define Epathu( x , y )         Epcode( 'P', Eax((double) x ) , Eay((double) y ), "" )
#endif

/* ======================================== */
/* handle multi-line text. */
Edotext( s, op )
char *s;
char op;
{
int i, slen;
char chunk[256];
double x, y;
x = Ex1; y = Ey1;
/* convert op */
if( tolower(op) == 'l' ) op = 'T';
else if( tolower(op) == 'c' ) op = 'C';
else if( tolower(op) == 'r' ) op = 'J';
slen = strlen( s );
for( i = 0; ;  ) {
	GL_getseg( chunk, s, &i, "\n" );
	Epcode( op, 0.0, 0.0, chunk );
	if( i >= slen ) break;
	if( Ecurtextdirection == 0 ) y -= Ecurtextheight;
	else if( Ecurtextdirection == 90 ) x += Ecurtextheight;
	else if( Ecurtextdirection == 270 ) x -= Ecurtextheight;
	if( Eflip ) Emov( y, x );
	else Emov( x, y );
	}
}

/* ======================================== */
/* EFONT - Set font to s.  If s is "" use standard font. */
Efont( s )      
char s[];
{ 
if( strlen( s ) < 1 ) {
        Epcode( 'F', 0.0, 0.0, Estandard_font ); 
	strcpy( Ecurfont, Estandard_font ); 
	}
else if( strcmp( s, Ecurfont )!= 0 ) {
	Epcode( 'F', 0.0, 0.0, s ); 
	strcpy( Ecurfont, s ); 
	} 
}
/* ======================================== */
/* ETEXTSIZE - Set textsize to x.  If x is 0 use standard textsize.
   In any case, the size is scaled by the standard text scaling factor. */
Etextsize( x )          
int x;
{ 
double p;

/* scale text */
if( x == 0 ) p = Estandard_textsize;
else p = (double)x * (double)(Estandard_textsize/10.0);


if( (int)p != Ecurtextsize ) {
	Epcode( 'I', p, 0.0, "" );
	Ecurtextsize = (int)p; 
	Ecurtextheight = (p+2.0)/72.0; 
	if( p >= 14 ) Ecurtextheight = p / 72.0;
	if( Edev == 'g' && Ecurfont[0] == '\0' ) {  /* "ascii" does not use preset sizes */
		/* get exact dimensions of one of the 5 gif text sizes available .. */
		if( p <= 6 ) { Ecurtextwidth = 0.05; Ecurtextheight = 9 / 72.0; } /* 7 */
		else if( p >= 7 && p <= 9 ) 
			{ Ecurtextwidth = 0.0615384; Ecurtextheight = 12 / 72.0; } /* 10 */
		else if( p >= 10 && p <= 12 ) 
			{ Ecurtextwidth = 0.0727272; Ecurtextheight = 14 / 72.0; } /* 12 */
		else if( p >= 13 && p <= 15 ) 
			{ Ecurtextwidth = 0.08; Ecurtextheight = 17 / 72.0; } /* 15 */
		else if( p >= 15 ) { Ecurtextwidth = 0.0930232; Ecurtextheight = 20 / 72.0; }
		}
	else if( Edev != 'x' ) Ecurtextwidth = Ecurtextheight * 0.4; 
			/* note: x11 driver supplies Ecurtextwidth in 
				pcode() when text size is set */
	}
}
/* ======================================== */
Etextdir( x )
int x;
{ 
if( x != Ecurtextdirection ) {
	Epcode( 'D', (double)x , 0.0, "" ); 
	Ecurtextdirection = x ; 
	}
}
/* ======================================== */
Epaper( x )
int x;
{
/* if( Ecurpaper != x ) { deleted scg 2/29/00 trying to get multi-page landscape to work...*/
	Epcode( 'O', (double)x , 0.0, "" ); 
	Ecurpaper = x; 
/* 	} */
}
/* ======================================== */
/* ELINETYPE - Set line parameters.  If linewidth is 0 use standard linescale.
   If pattern density is 0 use standard linescale. */
Elinetype( pattern, linewidth, pat_dens )    
int pattern;
double linewidth, pat_dens;
{ 
char buf[12];

/* scale linewidth  */
if( linewidth == 0.0 ) linewidth = Estandard_lwscale;
else linewidth = linewidth * Estandard_lwscale;

if( pat_dens == 0.0 ) pat_dens = Estandard_lwscale;

if( linewidth != Ecurlinewidth || 
	pattern != Ecurlinetype || 
	pat_dens != Ecurpatternfactor ) {

	sprintf( buf, "%d", pattern );
	Epcode( 'Y', linewidth, pat_dens, buf );
	Ecurlinewidth = linewidth; 
	Ecurlinetype = pattern; 
	Ecurpatternfactor = pat_dens;
	}
}

/* ======================================== */
Enormline()             
{ 
Epcode( 'Y', Estandard_lwscale, 1.0, "0" ); 
Ecurlinewidth = Estandard_lwscale;
Ecurlinetype = 0; 
Ecurpatternfactor = 1; 
}

/* ======================================== */
/* set current color for lines and text to s.  If s is "", use
   standard color.  */
Ecolor( s )
char *s;
{
/* char color[40], fillpat[40]; */
/* strip off fillpat spec and set flag? */

if( s[0] == '\0' ) strcpy( Ecurcolor, Estandard_color );
/* else if( strcmp( Ecurcolor, s )==0 ) return( 0 ); */  /* tried but screws up w/gif driver */

/* else	{
 *	sscanf( s, "%s %s", 
 */

else strcpy( Ecurcolor, s );
	
Epcode( 'r', 0.0, 0.0, Ecurcolor );
}

/* ======================================== */
/* set background color.
   If background color is "" use standard background color. */
Ebackcolor( color )
char *color;
{
if( color[0] != '\0' )strcpy( Ecurbkcolor, color );
else strcpy( Ecurbkcolor, Estandard_bkcolor );
}

/* ======================================== */
/* fill currently defined rectangle/polygon with color c */
Ecolorfill( c )
char *c;
{
char oldcolor[30];
if( strcmp( c, Ecurcolor )==0 ) { Efill(); return(0); }
strcpy( oldcolor, Ecurcolor );
Ecolor( c );
Efill();
Ecolor( oldcolor ); /* go back to color as it existed before.. */
}
/* ======================================== */
/* (Old) do shading, within the previously defined polygon path.. the shade can be 0 to 1 */
Eshade( s )
double s;
{
char str[20];
sprintf( str, "%g", s );
Ecolorfill( str );
}
