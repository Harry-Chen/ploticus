/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC LEGEND - Render an automatic legend. Plotting procs supply legend entries */

#include "pl.h"

#define MAXLEGENT 80            /* max # of legend entries */
#define MAXLEGTEXT 2000         /* max amount of legend text, including labels
                                        and details attributes */
 
static int NLE = 0;
static int LEavail = 0;
static int LEtype[MAXLEGENT];
static int LEparm1[MAXLEGENT];
static int LEparm2[MAXLEGENT];
static int LEparm3[MAXLEGENT];
static int LElabel[MAXLEGENT];
static int LEtag[MAXLEGENT];
static char Ltext[MAXLEGTEXT];

/* ================================ */
PLP_legend_initstatic()
{
NLE = 0;
LEavail = 0;
return(0);
}

/* ================================ */
PLP_legend()
{
int i;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

int stat;
int align;
double adjx, adjy;

char buf[256];
double x, y;
char format;
int outline;
int nlines, maxlen;
double yy;
char textdetails[256];
int reverseorder;
int j;
double seglen;
double hsep, msep;
int do_outline;
int colortext;
int specifyorder;
int ix, ixx, k;
double swatchsize;
int noclear;
char holdstdcolor[40];
char color[40];
int buflen;
char url[MAXPATH];
char *s;


TDH_errprog( "pl proc legend" );

/* initialize */
x = -9999.0;
y = -9999.0;
format = 'm';
strcpy( textdetails, "" );
reverseorder = 0;
seglen = 0.5;
msep = 0.0;
hsep = 1.2;
do_outline = 0;
colortext = 0;
strcpy( PL_bigbuf, "" );
specifyorder = 0;
ix = 0;
swatchsize = 0.1;
noclear = 0;


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( stricmp( attr, "location" )==0 ) {
		/* note: location x = beginning of LABEL; y = top */
		if( lineval[0] != '\0' ) getcoords( "location", lineval, &x, &y );
		}
	else if( stricmp( attr, "textdetails" )==0 ) strcpy( textdetails, lineval );
	else if( stricmp( attr, "seglen" )==0 ) seglen = atof( val );
	else if( stricmp( attr, "sep" )==0 ) {
		hsep = atof( val );
		msep = atof( val );
		}
	else if( stricmp( attr, "colortext" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) colortext = 1;
		else colortext = 0;
		}
		
	else if( stricmp( attr, "outlinecolors" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) do_outline = 1;
		else do_outline = 0;
		}
	else if( stricmp( attr, "noclear" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) noclear = 1;
		else noclear = 0;
		}
	else if( stricmp( attr, "format" )==0 ) {
		if(  val[0] != '\0' ) format = tolower(val[0]);
		else format = 'm';
		}
	else if( stricmp( attr, "specifyorder" )==0 ) {
		getmultiline( "specifyorder", lineval, MAXBIGBUF, PL_bigbuf );
		specifyorder = 1;
		}

	else if( stricmp( attr, "swatchsize" )==0 ) {
		if( val[0] != '\0' ) {
			swatchsize = atof( val );
			if( PLS.usingcm ) swatchsize /= 2.54;
			}
		else swatchsize = 0.1;
		}

	else if( stricmp( attr, "reverseorder" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) reverseorder = 1;
		else reverseorder = 0;
		}
	else if( stricmp( attr, "reset" )==0 ) {
		NLE = 0;
		return( 0 );
		}

	else Eerr( 1, "attribute not recognized", attr );
	}


if( NLE < 1 ) return( Eerr( 5839, "Cannot do legend, no legend entries given yet", "" ) );


/* now do the plotting work.. */

/* default location */
if( x < -9000.0 ) { 
	if( scalebeenset() ) { x = EXhi - 1.5; y = EYhi - 0.1; }
	else { x = 6.0; y = 2.0; }
	}

ix = 0;
buflen = strlen( PL_bigbuf );

textdet( "textdetails", textdetails, &align, &adjx, &adjy, -2, "R" );
y -= Ecurtextheight; 
for( i = 0; i < NLE; i++ ) {
	/* fprintf( stderr, "%d|%s|%s|%s\n", LEtype[i],
	 *	&Ltext[LElabel[i]],
	 *	&Ltext[LEparm1[i]],
	 *	&Ltext[LEparm2[i]] ); fflush ( stderr );
 	 */
	if( specifyorder ) {
		/* get next line in orderspec.. */
		NEXTORDERLINE:
		GL_getchunk( buf, PL_bigbuf, &ix, "\n" );
		if( ix >= buflen ) break;

		/* now search for matching entry.. */
		for( k = 0; k < NLE; k++ ) {
			if( strnicmp( buf, &Ltext[LElabel[k]], strlen(buf) )==0) { 
				j = k;
				break;
				}
			}
		if( k == NLE ) {
			/* following changed, scg 2/27/02 */
			/* Eerr( 2894, "No legend entry matched", buf ); */ 
			goto NEXTORDERLINE;
			/* continue; */
			}
		}
	else if( reverseorder ) j = ((NLE-1)-i);
	else j = i;
		
	yy = y+(Ecurtextheight*0.4);

	/* draw swatch(es), depending on type */
	if( LEtype[j] == LEGEND_COLOR ) {
		sscanf( &Ltext[LEparm1[j]], "%s", color );
		if( strcmp( color, Ecurbkcolor ) ==0 ) outline = 1;
		else outline = do_outline;
		if( outline ) { Elinetype( 0, 0.5, 1.0 ); Ecolor( Estandard_color ); }
		Ecblock( x-(swatchsize+0.1), y, x-0.1, y+swatchsize, color, outline );
		}
	else if( LEtype[j] == LEGEND_LINE ) {
		linedet( &Ltext[LElabel[j]], &Ltext[LEparm1[j]], 1.0 );
		Emov( x-(seglen+0.1), yy );
		Elin( x-0.1, yy );
		}
	else if( LEtype[j] == LEGEND_SYMBOL ) {
		double radius;
		char symcode[40];
		symdet( "symbol", &Ltext[LEparm1[j]], symcode, &radius );
		Emark( x-0.17, yy, symcode, radius );
		}
	else if( LEtype[j] == LEGEND_TEXT ) {
		/* parm1 is text, parm2 is textdetails */
		textdet( &Ltext[LElabel[j]], &Ltext[LEparm2[j]], &align, &adjx, &adjy, -2, "R" );
		Emov( x-0.1, y );
		Erightjust( &Ltext[LEparm1[j]] ); 
		}
	else if( LEtype[j] == LEGEND_LINE + LEGEND_SYMBOL ) {
		/* parm1 is linedetails, parm2 is symboldetails */
		double radius;
		char symcode[40];
		linedet( &Ltext[LElabel[j]], &Ltext[LEparm1[j]], 1.0 );
		Emov( x-(seglen+0.1), yy );
		Elin( x-0.1, yy );
		symdet( "symbol", &Ltext[LEparm2[j]], symcode, &radius );
		Emark( x-(seglen), yy, symcode, radius );
		Emark( x-0.1, yy, symcode, radius );
		}
	else if( LEtype[j] == LEGEND_TEXT + LEGEND_SYMBOL ) {
		/* parm1 is text, parm2 is textdetails, parm3 is symboldetails */
		double radius;
		char symcode[40];
		symdet( "symbol", &Ltext[LEparm3[j]], symcode, &radius );
		Emark( x-0.17, yy, symcode, radius );
		textdet( &Ltext[LElabel[j]], &Ltext[LEparm2[j]], &align, &adjx, &adjy, -2, "R" );
		Emov( x-0.17, y+(Ecurtextheight*0.2) );
		Ecentext( &Ltext[LEparm1[j]] ); 
		}


	s = &Ltext[ LElabel[ j ]];

	/* check for embedded url.. */
	url[0] = '\0';
	if( strnicmp( s, "url:", 4 )==0 ) {
		ixx = 0;
		strcpy( url, GL_getok( &s[4], &ixx ) );
		s = &Ltext[ LElabel[j] + ixx + 4 + 1 ];
		}
	

	/* get #lines and maxlen of label */
	measuretext( s, &nlines, &maxlen );

	/* render label */
	if( colortext ) {
		if( LEtype[j] == LEGEND_COLOR ) Ecolor( color );
		strcpy( holdstdcolor, Estandard_color );
		strcpy( Estandard_color, "" ); /* this prevents textdet() from changing the color 7/12/01 */
		}
	textdet( "textdetails", textdetails, &align, &adjx, &adjy, -2, "R" );
	Emov( x + adjx, y + adjy );
	Etext( s );
	if( colortext ) strcpy( Estandard_color, holdstdcolor );

	if( PLS.clickmap && url[0] ) {
		clickmap_entry( 'r', url, 0, x+adjx, y+adjy, 
			x+adjx+(maxlen*Ecurtextwidth), y+adjy+(nlines*Ecurtextheight), 1, 0, "" );
		}

	/* position for next line.. */
	if( format == 'm' ) {
		if( msep > 0.0 ) y -= msep;
		else y -= ((Ecurtextheight * nlines) + 0.03 );
		}
	else if( format == 's' ) {
		if( hsep > 0.0 ) x += ((Ecurtextwidth * maxlen ) + hsep);
		else x += ((Ecurtextwidth * maxlen ) + 1.2);
		}
	}

/* now reset so a new legend can be accumulated.. */
if( !noclear ) NLE = 0;
return( 0 );
}


/* ===================================== */
/* ADD_LEGENT - used by procs to add a legend entry */

add_legent( typ, label, tag, parm1, parm2, parm3 )
int typ; 
char *label, *tag, *parm1, *parm2, *parm3;
{
char *errmsg;

errmsg = "Sorry, too much legend content";

convertnl( label );

LEtype[ NLE ] = typ;
LElabel[ NLE ] = LEavail;
if( LEavail + strlen( label ) >= MAXLEGTEXT ) 
	return( Eerr( 8478, errmsg, "" ) );
strcpy( &Ltext[ LEavail ], label );
LEavail += (strlen( label ) + 1);

LEtag[ NLE ] = LEavail;
if( LEavail + strlen( tag ) >= MAXLEGTEXT ) 
	return( Eerr( 8478, errmsg, "" ) );
strcpy( &Ltext[ LEavail ], tag );
LEavail += (strlen( tag ) + 1);

if( LEavail + strlen( parm1 ) >= MAXLEGTEXT ) 
	return( Eerr( 8478, errmsg, "" ) );
LEparm1[ NLE ] = LEavail;
strcpy( &Ltext[ LEavail ], parm1 );
LEavail += (strlen( parm1 ) + 1);

if( LEavail + strlen( parm2 ) >= MAXLEGTEXT ) 
	return( Eerr( 8478, errmsg, "" ) );
LEparm2[ NLE ] = LEavail;
strcpy( &Ltext[ LEavail ], parm2 );
LEavail += (strlen( parm2 ) + 1);

if( LEavail + strlen( parm3 ) >= MAXLEGTEXT ) 
	return( Eerr( 8478, errmsg, "" ) );
LEparm3[ NLE ] = LEavail;
strcpy( &Ltext[ LEavail ], parm3 );
LEavail += (strlen( parm3 ) + 1);


NLE++;
return( 0 );
}

/* ========================================= */
/* GET_LEGENT - get a legend entry based on tag */

get_legent( tag, parm1, parm2, parm3 )
char *tag, *parm1, *parm2, *parm3;
{
int i;
for( i = 0; i < NLE; i++ ) if( strcmp( tag, &Ltext[ LEtag[i] ] )==0 ) break;
if( i == NLE ) return( 1 ); /* tag not found.. */

if( parm1 != NULL ) strcpy( parm1, &Ltext[ LEparm1[i] ] );
if( parm2 != NULL ) strcpy( parm2, &Ltext[ LEparm2[i] ] );
if( parm3 != NULL ) strcpy( parm3, &Ltext[ LEparm3[i] ] );
return( 0 );
}
/* ========================================= */
/* GET_LEGENT_RG - get a legend entry based numeric comparison with tag */

get_legent_rg( val, parm1, parm2, parm3 )
double val;
char *parm1, *parm2, *parm3;
{
int i;
double atof();

for( i = 0; i < NLE; i++ ) if( val >= atof( &Ltext[ LEtag[i] ] ) ) break;
	
if( i == NLE ) return( 1 ); /* tag not found.. */

if( parm1 != NULL ) strcpy( parm1, &Ltext[ LEparm1[i] ] );
if( parm2 != NULL ) strcpy( parm2, &Ltext[ LEparm2[i] ] );
if( parm3 != NULL ) strcpy( parm3, &Ltext[ LEparm3[i] ] );
return( 0 );
}
