/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC PAGE - set page-wide attributes, and do a "page" break for pp 2 and up */

#include "pl.h"

PLP_page( )
{
int i;
char buf[256];
char attr[40], val[256];
char *line, *lineval;
int nt, lvp, first;

int stat;
int align;
double adjx, adjy;
int nlines, maxlen;

int landscapemode;
char titledet[80];
int dobackground;
int dopagebox;
char outfilename[ MAXPATH ];
int pagesizegiven;
char devval[20];
double scalex, scaley;
double sx, sy;

TDH_errprog( "pl proc page" );

/* initialize */
landscapemode = Landscape; /* from command line */
strcpy( titledet, "normal" );
strcpy( outfilename, "" );
strcpy( Bigbuf, "" );
dobackground = 1;
dopagebox = 1;
if( GL_member( Device, "ges" )) 
	dopagebox = 0; /* we don't want bounding box to include entire page for gif , eps */
if( Device == 'e' ) dobackground = 0; /* added scg 9/27/99 */
pagesizegiven = 0;
strcpy( devval, "" );
scalex = scaley = 1.0;


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];


	/* if an attribute is given on command line, it overrides anything here.. */
	if( GL_slmember( attr, Cmdlineparms )) continue;
	if( stricmp( attr, "landscape" )==0 && GL_slmember( "portrait", Cmdlineparms )) continue;
	if( stricmp( attr, "outfilename" )==0 && GL_slmember( "o", Cmdlineparms )) continue;

	if( stricmp( attr, "landscape" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) landscapemode = 1;
		else landscapemode = 0;
		}
	else if( stricmp( attr, "title" )==0 ) getmultiline( "title", lineval, MAXBIGBUF, Bigbuf );
	else if( stricmp( attr, "titledetails" )==0 ) strcpy( titledet, lineval );
	else if( stricmp( attr, "color" )==0 ) strcpy( Estandard_color, val );
	else if( stricmp( attr, "scale" )==0 ) {
		nt = sscanf( val, "%lf %lf", &scalex, &scaley );
		if( nt == 1 ) scaley = scalex;
		}
	else if( stricmp( attr, "backgroundcolor" )==0 ) {
		strcpy( Estandard_bkcolor, val );
		Ebackcolor( val );
		dobackground = 1; /* added scg 9/27/99 */
		}
	else if( stricmp( attr, "linewidth" )==0 ) Estandard_lwscale = atof( val );
	else if( stricmp( attr, "textsize" )==0 ) {
		Estandard_textsize = atoi( val );
		}
	else if( stricmp( attr, "font" )==0 ) strcpy( Estandard_font, val );
	else if( stricmp( attr, "dobackground" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) dobackground = 1;
		else dobackground = 0;
		}
	else if( stricmp( attr, "dopagebox" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) dopagebox = 1;
		else dopagebox = 0;
		}
	else if( stricmp( attr, "tightcrop" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) Etightbb( 1 );
		else Etightbb( 0 );
		}
	else if( strnicmp( attr, "crop", 4 )==0 ) {
		double cropx1, cropy1, cropx2, cropy2;
		nt = sscanf( lineval, "%lf %lf %lf %lf", &cropx1, &cropy1, &cropx2, &cropy2 );
		if( nt != 4 ) Eerr( 2707, "usage: crop x1 y1 x2 y2 OR croprel left bottom right top", "" );
		else {
			if( Using_cm ) { cropx1 /= 2.54; cropy1 /= 2.54; cropx2 /= 2.54; cropy2 /= 2.54; }
			if( strcmp( attr, "croprel" )==0 ) Especifycrop( 2, cropx1, cropy1, cropx2, cropy2 ); /* relative to tight */
			else Especifycrop( 1, cropx1, cropy1, cropx2, cropy2 ); /* absolute */
			}
		}

	else if( stricmp( attr, "pagesize" )==0 ) {
		getcoords( "pagesize", lineval, &Winw, &Winh );
		pagesizegiven = 1;
		}
	else if( stricmp( attr, "outfilename" )==0 ) strcpy( outfilename, val );
	else if( stricmp( attr, "clickmapdefault" )==0 ) defaultmapurl( val );
	else if( stricmp( attr, "numbernotation" )==0 ) {
                if( stricmp( val, "us" )==0 ) Bignumspacer = ',';
                else if( stricmp( val, "euro" )==0 ) Bignumspacer = '.';
		else Bignumspacer = '\0';
		}
	else Eerr( 1, "page attribute not recognized", attr );
	}





/* -------------------------- */
/* Page break logic.. */
/* -------------------------- */
if( Npages == 0 ) {

	/* following 3 lines moved here from above - also replicated below.  scg 10/31/00 */
	if( scalex != 1.0 || scaley != 1.0 ) Esetglobalscale( scalex, scaley );
	Egetglobalscale( &sx, &sy );
	if( pagesizegiven ) Esetsize( Winw * sx, Winh * sy, Winx, Winy );
	else if( landscapemode && !Winsizegiven ) Esetsize( 11.0, 8.5, Winx, Winy ); /* landscape */

	/* initialize and give specified output file name .. */
	if( outfilename[0] != '\0' ) Esetoutfilename( outfilename );
	Einit( Inputfile, Device );

	/* set paper orientation */
	if( landscapemode ) Epaper( 1 );
	}


else if( Npages > 0 ) {

	if( GL_member( Device, "ges" )) {

		/* finish up current page before moving on to next one.. */
		Eshow();
		Eendoffile();

		/* now set file name for next page.. */
		if( outfilename[0] != '\0' ) Esetoutfilename( outfilename );
		else	{
			makeoutfilename( Cmdline_outfile, buf, Device, Npages+1 );
			Esetoutfilename( buf );
			}

		if( Clickmap ) {
			/* initialize a new click map file.. */
			char mapfile[ MAXPATH ];
			makeoutfilename( Cmdline_outfile, mapfile, 'm', Npages+1);
			mapfilename( mapfile, 10000, Debug );
			}


		/* perhaps set global scaling and/or page size for next page.. */
		/* following 3 lines copied here from above - scg 10/31/00 */
		if( scalex != 1.0 || scaley != 1.0 ) Esetglobalscale( scalex, scaley );
		Egetglobalscale( &sx, &sy ); 
		if( pagesizegiven ) Esetsize( Winw * sx, Winh * sy, Winx, Winy );
		else if( landscapemode && !Winsizegiven ) Esetsize( 11.0, 8.5, Winx, Winy ); /* landscape */

		/* initialize next page.. */
		Einit( Inputfile, Device );
		}

	else if ( Device == 'x' ) do_x_button( "More.." );

	else if ( GL_member( Device, "pc" ) ) {
		Eprint();
		if( landscapemode ) Epaper( 1 ); /* added scg 2/29/00 */
		Elinetype( 0, 0.6, 1.0 );   /* added scg 9/20/99 */
		}
	}
Npages++;


/* -------------------------- */
/* now do other work.. */
/* -------------------------- */


/* do background.. */
/* if( dopagebox ) Ecblock( 0.0, 0.0, EWinx, EWiny, Ecurbkcolor, 0 ); */ /* does update bb */
if( dopagebox ) Ecblock( 0.0, 0.0, Winw, Winh, Ecurbkcolor, 0 ); /* does update bb */
else if( dobackground ) {
	/* EPS color=transparent - best to do nothing.. */
        if( Device == 'e' && strcmp( Ecurbkcolor, "transparent" )==0 ) ;

        else Eclr(); /* doesn't update bb */
	}

if( Bigbuf[0] != '\0' ) {
	textdet( "titledetails", titledet, &align, &adjx, &adjy, 3, "B" );
	if( align == '?' ) align = 'C';
	measuretext( Bigbuf, &nlines, &maxlen );
	if( align == 'L' ) {
		/* Emov( 1.0 + adjx, (EWiny-0.8) + adjy ); */ 	/* these changes due to winsize being influenced by -scale */
		Emov( 1.0 + adjx, (Winh-0.8) + adjy );
		}
	else if ( align == 'C' ) {
        	/* Emov( (EWinx / 2.0 ) + adjx, (EWiny-0.8) + adjy ); */
        	Emov( (Winw / 2.0 ) + adjx, (Winh-0.8) + adjy );
		}
	else if( align == 'R' ) {
		/* Emov( (EWinx-1.0) + adjx, (EWiny-0.8) + adjy ); */
		Emov( (Winw-1.0) + adjx, (Winh-0.8) + adjy );
		}
	Edotext( Bigbuf, align );
	}

return( 0 );
}
