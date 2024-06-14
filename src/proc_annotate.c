/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC ANNOTATE - arbitrary placement of text, arrow, etc. */

#include "pl.h"

PLP_annotate()
{
int i;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

int stat;
int align;
double adjx, adjy;
double x, y;
char textdetails[256];
char fromfile[256];
int fromfilemode;
int arrowh, arrowt, arrow2h, arrow2t;
double ahx, ahy, atx, aty, ahsize;
double ah2x, ah2y, at2x, at2y;
char arrowdet[256];
int nlines, maxlen;
double boxw, boxh, ulx, uly;
char box[256];
char backcolor[40];
int verttext;
double bm;
char mapurl[MAXPATH];
double bevelsize, shadowsize;
char lowbevelcolor[40], hibevelcolor[40], shadowcolor[40];
int ioutline;


TDH_errprog( "pl proc annotate" );

/* initialize */
strcpy( Bigbuf, "" );
strcpy( textdetails, "" );
strcpy( fromfile, "" );
fromfilemode = 0;
x = 3.0;
y = 3.0;
arrowh = arrowt = 0;
arrow2h = arrow2t = 0;
strcpy( arrowdet, "" );
ahsize = 0.1;
strcpy( box, "" );
strcpy( backcolor, "" );
verttext = 0;
bm = 0.0;
strcpy( mapurl, "" );
bevelsize = 0.0;
shadowsize = 0.0;
strcpy( lowbevelcolor, "0.6" );
strcpy( hibevelcolor, "0.8" );
strcpy( shadowcolor, "black" );




/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];


	if( stricmp( attr, "location" )==0 ) {
		getcoords( "location", lineval, &x, &y );
		}
	else if( stricmp( attr, "text" )==0 ) 
		getmultiline( "text", lineval, MAXBIGBUF, Bigbuf );

	else if( stricmp( attr, "textdetails" )==0 ) strcpy( textdetails, lineval );
	else if( stricmp( attr, "fromfile" )==0 ) {
		strcpy( fromfile, lineval );
		fromfilemode = 1;
		}
	else if( stricmp( attr, "fromcommand" )==0 ) {
		strcpy( fromfile, lineval );
		fromfilemode = 2;
		}

	else if( stricmp( attr, "arrowhead" )==0 ) {
		getcoords( "arrowhead", lineval, &ahx, &ahy );
		arrowh = 1;
		}
	else if( stricmp( attr, "arrowtail" )==0 ) {
		getcoords( "arrowtail", lineval, &atx, &aty );
		arrowt = 1;
		}
	else if( stricmp( attr, "arrowhead2" )==0 ) {
		getcoords( "arrowhead2", lineval, &ah2x, &ah2y );
		arrow2h = 1;
		}
	else if( stricmp( attr, "arrowtail2" )==0 ) {
		getcoords( "arrowtail2", lineval, &at2x, &at2y );
		arrow2t = 1;
		}
	else if( stricmp( attr, "arrowdetails" )==0 ) strcpy( arrowdet, lineval );
	else if( stricmp( attr, "arrowheadsize" )==0 ) {
		ahsize = atof( val );
		if( ahsize <= 0.0 ) ahsize = 0.001; /* same as none */
		if( Using_cm ) ahsize /= 2.54;
		}
	else if( stricmp( attr, "box" )==0 ) strcpy( box, lineval );
	else if( stricmp( attr, "clickmapurl" )==0 ) strcpy( mapurl, val );
	else if( stricmp( attr, "boxmargin" )==0 ) {
		bm = atof( val );
		if( Using_cm ) bm /= 2.54;
		}
	else if( stricmp( attr, "verttext" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) verttext = 1;
		else verttext = 0;
		}
	else if( stricmp( attr, "backcolor" )==0 ) strcpy( backcolor, val );
	else if( stricmp( attr, "bevelsize" )==0 ) bevelsize = atof( val );
        else if( stricmp( attr, "shadowsize" )==0 ) shadowsize = atof( val );
        else if( stricmp( attr, "lowbevelcolor" )==0 ) strcpy( lowbevelcolor, val );
        else if( stricmp( attr, "hibevelcolor" )==0 ) strcpy( hibevelcolor, val );
        else if( stricmp( attr, "shadowcolor" )==0 ) strcpy( shadowcolor, val );

	else Eerr( 1, "attribute not recognized", attr );
	}



if( fromfilemode > 0 ) file_to_buf( fromfile, fromfilemode, Bigbuf, MAXBIGBUF );

textdet( "textdetails", textdetails, &align, &adjx, &adjy, 0, "R" );
if( align == '?' ) align = 'C';

/* figure backing box */
measuretext( Bigbuf, &nlines, &maxlen );
boxw = (maxlen+2) * Ecurtextwidth;
boxh = (nlines*1.2) * Ecurtextheight;
uly = y + Ecurtextheight;
if( align == 'C' ) ulx = x - (boxw/2.0); 
else if( align == 'L' ) ulx = x;
else if( align == 'R' ) ulx = x + boxw;

if( bm != 0.0 ) {
	ulx -= bm;
	uly += bm;
	boxw += (bm*2);
	boxh += (bm*2);
	}

if( strlen( backcolor ) > 0 || (strlen( box ) > 0 && strnicmp( box, "no", 2 )!= 0 ) ) {
	if( strlen( box ) > 0 && strnicmp( box, "no", 2 )!= 0 ) {
		ioutline = 1;
		linedet( "box", box, 0.5 );
		}
	else ioutline = 0;
	Ecblock( ulx, (uly-boxh), ulx+boxw, uly, backcolor, ioutline );
	if( bevelsize > 0.0 || shadowsize > 0.0 ) 
		Ecblockdress( ulx, (uly-boxh), ulx+boxw, uly, 
       			bevelsize, lowbevelcolor, hibevelcolor, shadowsize, shadowcolor);
	}

if( Clickmap && mapurl[0] != '\0' ) mapentry( 'r', mapurl, 0, ulx, (uly-boxh), ulx+boxw, uly, 1, 0 );

/* now render the text.. */
textdet( "textdetails", textdetails, &align, &adjx, &adjy, 0, "R" ); /* need to do again */
if( align == '?' ) align = 'C';
Emov( x + adjx, y + adjy );
if( verttext ) Etextdir( 90 );
Edotext( Bigbuf, align );
if( verttext ) Etextdir( 0 );

/* now render the arrow.. use same color as text..
	if tail location not given, try to be smart about arrow placement.. */
if( arrowh ) {
	linedet( "arrowdetails", arrowdet, 0.7 );
	if( !arrowt ) calc_arrow( ulx, uly, boxw, boxh, ahx, ahy, &atx, &aty );
	Earrow( atx, aty, ahx, ahy, ahsize, Ecurcolor );
	}

/* and render 2nd arrow.. */
if( arrow2h ) {
	if( !arrow2t ) calc_arrow( ulx, uly, boxw, boxh, ah2x, ah2y, &at2x, &at2y );
	Earrow( at2x, at2y, ah2x, ah2y, ahsize, Ecurcolor );
	}

return( 0 );
}

/* ================== */
/* figure where tail of arrow should be */

calc_arrow( ulx, uly, boxw, boxh, ahx, ahy, tailx, taily )
double ulx, uly, boxw, boxh, ahx, ahy, *tailx, *taily;
{
double atx, aty;

/* ah directly above or below textbox.. straight arrow.. */
if( ahx >= ulx && ahx <= ulx+boxw ) {
	atx = ahx;
	if( ahy > uly ) aty = uly;
	else if( ahy < uly-boxh ) aty = uly-boxh;
	}

/* ah directly beside textbox.. straight arrow.. */
else if( ahy >= uly-boxh && ahy <= uly ) {
	aty = ahy;
	if( ahx < ulx ) atx = ulx;
	else if( ahx > ulx+boxw ) atx = ulx+boxw;
	} 

/* ah somewhere to left of textbox.. elbow arrow from mid-box.. */
else if( ahx < ulx ) {
	atx = ahx + ((ulx - ahx)/2.0);
	aty = uly - (boxh/2.0);
	Emov( ulx, aty ); 
	Elin( atx, aty );
	}

/* ah somewhere to right of textbox.. elbow arrow from mid-box.. */
else if( ahx > ulx+boxw ) {
	atx = ulx+boxw + ((ahx-(ulx+boxw))/2.0);
	aty = uly - (boxh/2.0);
	Emov( ulx+boxw, aty ); 
	Elin( atx, aty );
	}

else { atx = ahx; aty = ahy; } /* ? */

*tailx = atx;
*taily = aty;
return( 0 );
}
