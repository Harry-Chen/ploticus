/* ploticus data display engine.  Software, documentation, and examples.
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

#include "pl.h"

/* ============================= */
/* TEXTDET - parse a text details setting and execute it */
/* attributes are: size=N color=COLOR style=B|I|BI|R align=L|C|R adjust=x,y */
/* ============================= */
textdet( parmname, spec, align, adjx, adjy, sizehint, stylehint )
char *parmname, *spec;
int *align;
double *adjx, *adjy;
int sizehint;
char *stylehint;
{
char at[5][80];
int nt;
int i;
int p;
char str[80];
char font[80];
char color[80];
double tmp;

/* defaults.. */
strcpy( color, Estandard_color );
strcpy( font, stylehint );

if (Device == 's') SVGfontname( Estandard_font, font );/* for SVG font support - BT 5/11/01 */
else  EPSfontname( Estandard_font, font );

p = Estandard_textsize + sizehint;
*align = '?'; /* align and offset will be sent back to caller.. */
*adjx = 0.0;
*adjy = 0.0;

/* parse spec.. */
nt = sscanf( spec, "%s %s %s %s %s", at[0], at[1], at[2], at[3], at[4] );

if( stricmp( at[0], "yes" )==0 || 
	strnicmp( at[0], "no", 2 )==0 ) nt = 0; /* drop thru.. */

for( i = 0; i < nt; i++ ) {
	if( strnicmp( at[i], "size=", 5 )==0 ) p = atoi( &at[i][5] );
	else if( strnicmp( at[i], "style=", 6 )==0 ) {
		if( !GL_slmember( &at[i][6], "B I BI R" )) {
			Eerr( 82, "warning, text style must be B, I, BI, or R", &at[i][6] );
			strcpy( font, "R" );
			}
		else strcpy( font, &at[i][6] );

		if (Device == 's') SVGfontname( Estandard_font, font );/* for SVG font support - BT 5/11/01 */
		else EPSfontname( Estandard_font, font );
		}

	else if( strnicmp( at[i], "font=", 5 )==0 ) strcpy( font, &at[i][5] );
	else if( strnicmp( at[i], "color=", 6 )==0 ) strcpy( color, &at[i][6] );

	else if( strnicmp( at[i], "align=", 6 )==0 ) *align = at[i][6];
	else if( strnicmp( at[i], "adjust=", 7 )==0 ) {
		if( Eflip ) sscanf( &at[i][7],  "%lf,%lf", adjy, adjx );
		else sscanf( &at[i][7],  "%lf,%lf", adjx, adjy );
		}
	else 	{
		sprintf( str, "warning: %s is an unrecognized text details subattribute", at[i] );
		Eerr( 71, str, parmname );
		}
	}
if( Estandard_color[0] ) Ecolor( color ); /* 'if' added scg 7/12/01 */
Efont( font );
Etextsize( p ); 
if( !GL_member( *align, "LCR?" )) {
	Eerr( 72, "warning, align must be either L, C, or R", parmname );
	*align = '?';
	}
if( Using_cm ) { *adjx /= 2.54; *adjy /= 2.54; }
return( 0 );
}

/* ============================= */
/* LINEDET - parse a line details setting and execute it */
/* attributes are: width=W style=N color=COLOR dashscale=S */
/* ============================= */
linedet( parmname, spec, defaultwidth )
char *parmname, *spec;
double defaultwidth;
{
char at[5][80];
int nt;
int i;
int stat;
int style;
double width;
double ds;
char str[80];
char color[80];

/* defaults.. */
style = 0;
width = defaultwidth;
ds = 1.0;
strcpy( color, Estandard_color );

/* parse the spec.. */
nt = sscanf( spec, "%s %s %s %s %s", at[0], at[1], at[2], at[3], at[4] );

if( stricmp( at[0], "yes" )==0 || 
	strnicmp( at[0], "no", 2 )==0 ) nt = 0; /* drop thru.. */

for( i = 0; i < nt; i++ ) {
	if( strnicmp( at[i], "width=", 6 )==0 || strnicmp( at[i], "thick=", 6 )==0 ) {
		stat = num( &at[i][6], &width );
		if( stat != 0 || width < 0.1 ) return( Eerr( 73, "invalid line width", parmname ) );
		}
	else if( strnicmp( at[i], "style=", 6 )==0 ) style = atoi( &at[i][6] );
	else if( strnicmp( at[i], "color=", 6 )==0 ) strcpy( color, &at[i][6] );
	else if( strnicmp( at[i], "dashscale=", 10 )==0 ) ds = atof( &at[i][10] );
	else 	{
		sprintf( str, "%s is an unrecognized line details subattribute", at[i] );
		return( Eerr( 72, str, parmname ) );
		}
	}
Elinetype( style, width, ds );
Ecolor( color );
	
return( 0 );
}

/* ================== */
/* SYMDET - parse a point symbol detail spec and build a symbol code  */
/* shape=A style=S radius=R linewidth=W symcode= giffile=F gifscale=sx,sy */
/* =================== */
symdet( parmname, spec, symcode, radius )
char *parmname, *spec;
char *symcode; /* new symbol code copied into here */
double *radius;
{
char at[5][80];
int nt;
int i;
int stat;
int shape;
char style;
double linewidth;
char fillcolor[80];
char linecolor[80];
int symcodedone;
char str[MAXPATH];
int doing_img;
double sx, sy;

/* defaults.. */
shape = 6;
style = 'a';
linewidth = 0.5; /* nice & thin */
/* strcpy( fillcolor, Estandard_color ); */
strcpy( fillcolor, "" );
strcpy( linecolor, Estandard_color );
*radius = 0.05;
symcodedone = 0;
doing_img = 0;
sx = 1.0; sy = 1.0;

/* parse the spec.. */
nt = sscanf( spec, "%s %s %s %s %s", at[0], at[1], at[2], at[3], at[4] );

if( stricmp( at[0], "yes" )==0 || 
	strnicmp( at[0], "no", 2 )==0 ) nt = 0; /* drop thru.. */


for( i = 0; i < nt; i++ ) {
	if( strnicmp( at[i], "linewidth=", 10 )==0 ) {
		stat = num( &at[i][10], &linewidth );
		if( stat != 0 || linewidth < 0.1 ) return( Eerr( 73, "invalid line width", parmname ) );
		}
	else if( strnicmp( at[i], "shape=", 6 )==0 ) {
		if( atoi( &at[i][6] ) > 0 ) shape = atoi( &at[i][6] );
		else if( strnicmp( &at[i][6], "tri", 3 )==0 ) shape = 1;
		else if( strnicmp( &at[i][6], "downtri", 7 )==0 ) shape = 2;
		else if( strnicmp( &at[i][6], "di", 2 )==0 ) shape = 3;
		else if( strnicmp( &at[i][6], "sq", 2 )==0 ) shape = 4;
		else if( strnicmp( &at[i][6], "pe", 2 )==0 ) shape = 5;
		else if( strnicmp( &at[i][6], "ci", 2 )==0 ) shape = 6;
		else if( strnicmp( &at[i][6], "lefttri", 7 )==0 ) shape = 8;
		else if( strnicmp( &at[i][6], "righttri", 8 )==0 ) shape = 7;
		else if( strnicmp( &at[i][6], "niceci", 6 )==0 ) shape = 9;
		}
	else if( strnicmp( at[i], "style=", 6 )==0 ) {
		if( strlen( &at[i][6] ) == 1 ) style = at[i][6];
		else if( strnicmp( &at[i][6], "out", 3 )==0 ) style = 'a';
		else if( strnicmp( &at[i][6], "spo", 3 )==0 ) style = 's';
		else if( strnicmp( &at[i][6], "fil", 3 )==0 ) {
			style = 'n';
			if( fillcolor[0] == '\0' ) strcpy( fillcolor, Estandard_color );
			}
		}
	
	else if( strnicmp( at[i], "fillcolor=", 10 )==0 ) strcpy( fillcolor, &at[i][10] );
	else if( strnicmp( at[i], "linecolor=", 10 )==0 ) strcpy( linecolor, &at[i][10] );
	else if( strnicmp( at[i], "radius=", 7 )==0 ) {
		*radius = atof( &at[i][7] );
		if( Using_cm ) *radius /= 2.54; 
		}
	else if( strnicmp( at[i], "sym", 3 ) == 0 ) { /* return a specified sym code as is.. */
		strcpy( symcode, at[i] );
		symcodedone = 1;
		}
	else if( strnicmp( at[i], "pngfile", 7 ) ==0 || strnicmp( at[i], "imgfile", 7 )==0 ) {    
		strcpy( str, &at[i][8] );
		doing_img = 1;
		}
	else if( strnicmp( at[i], "pngscale", 8 ) == 0 || strnicmp( at[i], "imgscale", 8 )==0 ) {
		nt = sscanf( &at[i][9], "%lf,%lf", &sx, &sy );
		if( nt == 1 ) sy = sx;
		}
	else 	{
		sprintf( str, "%s is an unrecognized symboldetails subattribute", at[i] );
		return( Eerr( 72, str, parmname ) );
		}
	}

if( doing_img ) {
	if( sx < 0.001 ) sx = 1.0;
	if( sy < 0.001 ) sy = 1.0;
	stat = Eimload( str, sx, sy );
	if( stat != 0 ) Eerr( 5893, "warning, error on image load", str );
	strcpy( symcode, "gif" );
	return( 0 );
	}
if( !symcodedone ) sprintf( symcode, "sym%d%c%s", shape, style, fillcolor );
Elinetype( 0, linewidth, 1.0 );
Ecolor( linecolor );
return( 0 );
}


/* ================= */
double avg( g, h )
double g, h;
{
return( (g + h) / 2.0 );
}

