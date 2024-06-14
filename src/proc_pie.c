/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC PIE  - render a pie graph */

/* 7/12/01 - scg - now renders each entire slice as a single polygon */

#include "pl.h"
#define TORAD 0.0174532
#define MAXSLICE 40


PLP_pie()
{
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

char buf[256];
int j;
int stat;
int align;
double adjx, adjy;
int df;
double cx, cy, radius;
double theta;
char color[MAXSLICE][40];
int lblfld;
double total;
double ux, uy, stop;
double uux, uuy;
int ncolors;
double starttheta;
double sin(), cos();
double fval;
double x, y;
double oldx, oldy;
int nexpl;
double expl[MAXSLICE];
char outlinedetails[256];
char lablinedetails[256];
char labelmode[50];
int lbltextgiven;
int ibb;
char label[256];
char textdetails[256];
double stheta;
double lblfarout;
char pctstr[30];
char pctfmt[30];
int colorfield;
int nlines, maxlen;
double boxwid, boxhi;
char mapurl[MAXPATH];
char expurl[MAXPATH];
double labx, laby;
int irow;


TDH_errprog( "pie" );




/* initialize */
theta = 0.0;
cx = cy = -1.0;
radius = -1.0;
lblfld = -1;
strcpy( labelmode, "legend" );
strcpy( Bigbuf, "" );
ncolors = 0;
starttheta = 90.0 * TORAD;
total = 0.0;
nexpl = 0;
strcpy( outlinedetails, "" );
strcpy( lablinedetails, "" );
/* strcpy( labelmode, "labelonly" ); */
lbltextgiven = 0;
strcpy( textdetails, "" );
lblfarout = 0.0;
strcpy( pctfmt, "%.1f" );
colorfield = -1;
strcpy( mapurl, "" );

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( stricmp( attr, "datafield" )==0 ) df = fref( val ) -1 ;
	else if( stricmp( attr, "center" )==0 ) getcoords( "center", lineval, &cx, &cy );
	else if( stricmp( attr, "radius" )==0 ) Elenex( val, X, &radius ); /* use X units.. */
	else if( stricmp( attr, "firstslice" )==0 ) starttheta = ((360-atof( val )) * TORAD ) + 90.0 * TORAD;
	else if( stricmp( attr, "total" )==0 ) total = atof( val );
	else if( stricmp( attr, "clickmapurl" )==0 ) strcpy( mapurl, val );
	else if( stricmp( attr, "colors" )==0 ) {
		int i, ix;
		for( i = 0, ix = 0; i < MAXSLICE; i++ ) {
			strcpy( color[i], GL_getok( lineval, &ix ) );
			if( color[i][0] == '\0' ) break;
			}
		ncolors = i;
		}
	else if( stricmp( attr, "labels" )==0 ) {
		getmultiline( "labels", lineval, MAXBIGBUF, Bigbuf );
		lbltextgiven = 1;
		}
	else if( stricmp( attr, "labelfield" )==0 ) lblfld = fref( val ) - 1;
	else if( stricmp( attr, "colorfield" )==0 ) colorfield = fref( val ) - 1;
	else if( stricmp( attr, "outlinedetails" )==0 ) strcpy( outlinedetails, lineval );
	else if( stricmp( attr, "lablinedetails" )==0 ) strcpy( lablinedetails, lineval );
	else if( stricmp( attr, "textdetails" )==0 ) strcpy( textdetails, lineval );
	else if( stricmp( attr, "labelmode" )==0 ) strcpy( labelmode, val );
	else if( stricmp( attr, "labelfarout" )==0 ) lblfarout = atof( val );
	else if( stricmp( attr, "pctformat" )==0 ) strcpy( pctfmt, val );
	else if( stricmp( attr, "explode" )==0 ) {
		int i, ix;
		for( i = 0, ix = 0; i < MAXSLICE; i++ ) {
			strcpy( buf, GL_getok( lineval, &ix ));
			if( buf[0] == '\0' ) break;
			else expl[i] = atof( buf );
			}
		nexpl = i;
		}
	else Eerr( 1, "attribute not recognized", attr );
	}



/* overrides and degenerate cases */
/* -------------------------- */

if( Nrecords[Dsel] < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );
if( df < 0 || df >= Nfields[Dsel] ) return( Eerr( 2840, "invalid datafield", "" ) );
if( cx < 0.0 || cy < 0.0 ) return( Eerr( 2841, "invalid center", "" ) );
if( radius < 0.0 || radius > 5.0 ) return( Eerr( 2842, "invalid radius", "" ) );


if( lblfarout < 0.001 && strnicmp( labelmode, "label", 5 )==0 ) lblfarout = 0.67;
if( lblfarout < 0.001 && strnicmp( labelmode, "line", 4 )==0 ) lblfarout = 1.3;




/* now do the plotting work.. */
/* -------------------------- */

/* compute total.. */
if( total <= 0.0 ) {
	total = 0.0;
	for( irow = 0; irow < Nrecords[Dsel]; irow++ ) {
		total += atof( da( irow, df ) );
		}
	}

ibb = 0;


for( j = 0; j < 2; j++ ) { /* first time - colors; 2nd time, lines */
	theta = starttheta + 6.28319;
	if( j == 1 ) {
		/* set line details for outline.. */
		linedet( "outlinedetails", outlinedetails, 0.5 );
		}
	for( irow = 0; irow < Nrecords[Dsel]; irow++ ) {
		/* take val as % of total and convert to rads */
		fval = (atof( da( irow, df ) ) / total ) * 6.28319; 
		stop = theta - fval;

		/* find (ux,uy), the point at the wedge of the slice, normalized to 0,0 center.. */
		if( nexpl <= 0 ) { /* don't explode any slices */
			ux = 0.0; 
			uy = 0.0;
			}
		else if( irow >= nexpl ) { /* explode slice according to last explode value */
			ux = (expl[nexpl-1]*radius) * cos( theta+(fval/-2.0) );
			uy = (expl[nexpl-1]*radius) * sin( theta+(fval/-2.0) );
			}
		else if( irow < nexpl ) { /* explode slice according to explode value [i] */
			ux = (expl[irow]*radius) * cos( theta+(fval/-2.0) );
			uy = (expl[irow]*radius) * sin( theta+(fval/-2.0) );
			}
			
		first = 1;
		stheta = theta;
		if( j == 1 && strnicmp( outlinedetails, "no", 2 )==0 ) goto DOLAB;
		for( ; theta > stop; theta -= 0.03 ) {
			if( theta - stop < 0.03 ) theta = stop;
			x = cx + (radius * cos( theta ));
			y = cy + (radius * sin( theta ));
			if( j == 0 ) {
				if( first ) { 
					first = 0; oldx = x; oldy = y; 
					Emov( cx+ux, cy+uy );
					Epath( x+ux, y+uy );
					continue; 
					}

				Epath( x+ux, y+uy ); 
				oldx = x; oldy = y;
				}
			else if( j == 1 ) {
				if( first ) { Emov( cx+ux, cy+uy ); Elin( x+ux, y+uy ); first = 0; }
				Elin( x+ux, y+uy );
				}
			}
		if( j == 1 ) Elin( cx+ux, cy+uy ); 

		if( j == 0 ) {
			Epath( cx+ux, cy+uy );

			if( colorfield >=0 ) {  
				char color1[80];
                		strcpy( color1, "" );
                		get_legent( da( irow, colorfield ), color1, NULL, NULL );
				Ecolorfill( color1 );
                		}
			else if( stricmp( color[0], "auto" )==0 ) {
				char color1[40];
				Eicolor( irow, color1 );
				Ecolorfill( color1 );
				}
			else if( irow < ncolors ) Ecolorfill( color[irow] );
			else if( ncolors > 0 ) Ecolorfill( color[ncolors-1] );
			else Ecolorfill( "0.8" );
			}

		/* labeling */
		DOLAB:
		if( j == 1 && labelmode[0] != '\0' ) {
			strcpy( label, "" );
			if( lblfld >= 0 ) strcpy( label, da( irow, lblfld ) );
			else if( lbltextgiven ) GL_getseg( label, Bigbuf, &ibb, "\n" );

			sprintf( pctstr, pctfmt, (atof( da( irow, df ) ) / total)*100.0 );

			GL_varsub( label, "@PCT", pctstr );
			convertnl( label );

			/* allow @field substitutions into url */
			if( Clickmap && mapurl[0] != '\0' ) do_subst( expurl, mapurl, irow );

			if( strnicmp( labelmode, "legend", 6 )==0 ) {
				add_legent( LEGEND_COLOR, label, "", color[irow], "", "" );
				}
			else if( strnicmp( labelmode, "labelonly", 5 ) ==0 ) {
				double htheta;
				htheta = stop + ((stheta - stop) / 2.0 );
				x = cx + ( (radius * lblfarout) * cos( htheta ) );
				y = cy + ( (radius * lblfarout) * sin( htheta ) );
				textdet( "textdetails", textdetails, &align, &adjx, &adjy, -2,"R");
				labx = x+ux;
				laby = y+uy;
				Emov( labx, laby );
				Ecentext( label );
				if( Clickmap && mapurl[0] != '\0' ) {
					measuretext( label, &nlines, &maxlen );
					boxwid = (maxlen * Ecurtextwidth) / 2.0;
					boxhi = nlines * Ecurtextheight;
					laby += Ecurtextheight;
					mapentry( 'r', expurl, 0, labx-boxwid, laby-boxhi, labx+boxwid, laby, 1, 0 );
					}
				linedet( "linedetails", outlinedetails, 0.5 ); /* restore */
				}
			else if( strnicmp( labelmode, "line+label", 4 ) ==0 ) {
				double htheta, px, py, w, z;

				/* w = radius * lblfarout;
				 * if( w < (1.1 * radius) ) z = lblfarout;
				 */
				w = radius * lblfarout;
				if( w < (1.1 * radius) ) z = lblfarout;
				else z = 1.1;

				htheta = stop + ((stheta - stop) / 2.0 );
				px = cx + ( (radius * 0.9 ) * cos( htheta ) );
				py = cy + ( (radius * 0.9 ) * sin( htheta ) );
		
				x = cx + ( (radius * z ) * cos( htheta ) );
				y = cy + ( (radius * z ) * sin( htheta ) );

				linedet( "lablinedetails", lablinedetails, 0.5 ); 
				Emov( px+ux, py+uy );
				Elin( x+ux, y+uy );
				if( x+ux < cx ) {
					Elin( (cx+ux)-w, y+uy );
					textdet( "textdetails", textdetails, &align, &adjx, &adjy, -2,"R");
					labx = ((cx+ux)-w)-0.1;
					laby = y+uy;
					Emov( labx, laby );
					Erightjust( label );
					if( Clickmap && mapurl[0] != '\0' ) {
						measuretext( label, &nlines, &maxlen );
						boxwid = maxlen * Ecurtextwidth;
						boxhi = nlines * Ecurtextheight;
						laby += Ecurtextheight;
						mapentry( 'r', expurl, 0, labx-boxwid, laby-boxhi, labx, laby, 1, 0 );
						}
					}
				else 	{
					Elin( (cx+ux)+w, y+uy );
					textdet( "textdetails", textdetails, &align, &adjx, &adjy, -2,"R");
					labx = cx+ux+w+0.1;
					laby = y+uy;
					Emov( labx, laby );
					Etext( label );
					if( Clickmap && mapurl[0] != '\0' ) {
						measuretext( label, &nlines, &maxlen );
						boxwid = maxlen * Ecurtextwidth;
						boxhi = nlines * Ecurtextheight;
						laby += Ecurtextheight;
						mapentry( 'r', expurl, 0, labx, laby-boxhi, labx+boxwid, laby, 1, 0 );
						}
					}
				linedet( "outlinedetails", outlinedetails, 0.5 ); /* restore */
				}
			}


		theta = stop;
		}
	}

return( 0 );
}
