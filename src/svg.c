/* SVG Driver for Ploticus - Copyright 2001 Bill Traill (bill@traill.demon.co.uk).
 * Covered by GPL; see the file ./Copyright for details. */

/*
   Checking for redundant calls is not done here; should be done by caller.

   special characters not delt with
   need to add an option to output .svgz

	04Nov01 bt	Created svg driver based on existing postscript driver ps.c

	12Nov01 bt	Changed to relative addressing within a path. 
			Added grouping of styles and a default style in <g> elements
			Added some shorthand in the entity header
			Accuracy of some of the coords improved
			Added stoke to SVGfill to get rid of occasional ghost lines
*/

#include <stdio.h>
#define Eerr(a,b,c)  TDH_err(a,b,c)

#define YES 1
#define NO 0
#define MARG_X 0 
#define MARG_Y 0 
#define stricmp(a,b) strcasecmp(a,b)

extern int Debug;	/* scg */
extern char Tmpname[];	/* scg */

static int SVGstdout;		/* 1 if SVGpf is stdout */
static FILE *SVGpf;

static double SVGx_size;		/* width of the drawing area */
static double SVGy_size;		/* height of the drawing area */
static int  SVGpath_in_prog =0;		/* flag to indicate if an svg path is in progress */
static char SVGcur_color[1024];
static char SVGdash_style[1024];
static double SVGline_width=1;

static char SVGfont_name[100];		/* current font name */
static int  SVGchdir;	 		/* char direction in degrees */
static int  SVGcurrpointsz;		/* current char size in points */
static char SVGfont_weight[100];
static char SVGfont_style[100];
static char SVGalign[100];
static long SVGbbofs; 		   /* byte offset of viewbox statement as returned by ftell() - scg */
static int SVGpixs_inch; 	   /* number of pixels per inch - scg */
static int SVGdotag = 0;  	   /* if 1, write a suitable html <embed> tag to stdout - scg */
static char SVGfilename[256] = ""; /* output file name (was local) - scg */

static char SVGstyle[1024] = "";	/* line,font styles etc */
static char SVGnew_style[1024] = "";
static int  SVGstyle_in_prog = 0;	/* flag to indicate if a current <g> for attributes is open */

static double SVGlast_x;
static double SVGlast_y;

static char *SVGdef_fill = "fill:#000000;";  	/* char * added - scg */
static char *SVGdef_stroke = "stroke:#000000;"; /* char * added - scg */
static char *SVGdef_font = "&ff;Helvetica;";    /* char * added - scg */

static int SVGcompress = 0;			/* 1 = compress output; 0 = don't */
static char SVGcompressmode[10] = "wb9";		
static char SVGtmpfilename[256] = "";

/* ============================= */
static SVGset_style() 
{
	char fontw[200] = "";
	char fonts[200] = "";
	char align[200] = "";
	char font[200] = "";
	char fill[200] = "";
	char stroke[200] = "";

	if (SVGfont_weight[0] != '\0') {
		/* font-weight */
		sprintf(fontw,"&fw;%s;",SVGfont_weight);
	}
	if (SVGfont_style[0] != '\0') {
		/* font-style */
		sprintf(fonts,"&fst;%s;",SVGfont_style);
	}

	sprintf(font,"&ff;%s;",SVGfont_name);
	if (!strcmp(font,SVGdef_font)) strcpy(font,"");
	sprintf(fill,"fill:%s;",SVGcur_color);
	if (!strcmp(fill,SVGdef_fill)) strcpy(fill,"");
	sprintf(stroke,"stroke:%s;",SVGcur_color);
	if (!strcmp(stroke,SVGdef_stroke)) strcpy(stroke,"");


	if (!strcmp(SVGalign, "start")) strcpy(align,"&as;");
	if (!strcmp(SVGalign, "middle")) strcpy(align,"&am;");
	if (!strcmp(SVGalign, "end")) strcpy(align,"&ae;");

	sprintf (SVGnew_style,"style=\"%s%s%s&sw;%3.1f;%s%s&fs;%d;%s\"",
		fill,stroke,font,SVGline_width,fontw,fonts,SVGcurrpointsz,align);
}
/* ============================= */

static SVGprint_style()
{
	if (strcmp(SVGnew_style,SVGstyle) ) {
		if (SVGstyle_in_prog) {
			fprintf( SVGpf, "</g>");
		}
		fprintf( SVGpf, "<g %s>\n",SVGnew_style);
		strcpy(SVGstyle,SVGnew_style);
		SVGstyle_in_prog = 1;
	}
}

/* ============================= */
SVGsetup( name, dev, f, pixs_inch, Ux, Uy, Upleftx, Uplefty )
char *name; /* arbitrary name */
char dev;  /* 'p' = monochrome   'c' = color   'e' = eps */
char *f;  /* file to put code in */
int pixs_inch;
double Ux;
double Uy;
int Upleftx;
int Uplefty;
{  


/* set globals */
if( dev != 's' ) dev = 's';
strcpy( SVGfont_name, "Helvetica" );
strcpy( SVGfont_weight, "" );
strcpy( SVGfont_style, "" );
strcpy( SVGalign, "start" );
SVGchdir = 0;
SVGcurrpointsz = 10;

SVGpixs_inch = pixs_inch; /* scg */


/* determine if we need to write to tmp file, and open appropriate file for write.. */
SVGstdout = 0;
if( stricmp( f, "stdout" )==0 || f[0] == '\0' ) SVGstdout = 1;
else strcpy( SVGfilename, f );

if( SVGstdout || SVGcompress ) {
	sprintf( SVGtmpfilename, "%s_V", Tmpname );
	SVGpf = fopen( SVGtmpfilename, "w" ); /* output file */
	if( SVGpf == NULL ) Eerr( 12031, "Cannot open tmp output file", SVGtmpfilename );
	}
else 	{
	SVGpf = fopen( SVGfilename, "w" );
	if( SVGpf == NULL ) Eerr( 12031, "Cannot open output file", SVGfilename );
	}
if( SVGpf == NULL ) exit( 1 );


SVGx_size = Ux * pixs_inch;
SVGy_size = Uy * pixs_inch;
/* print header */

fprintf( SVGpf, "<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>\n");
fprintf( SVGpf, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20001102//EN\"\n");
fprintf( SVGpf, "\"http://www.w3.org/TR/2000/CR-SVG-20001102/DTD/svg-20001102.dtd\" [\n");
fprintf( SVGpf, "<!ENTITY ff \"font-family:\">\n"); 
fprintf( SVGpf, "<!ENTITY fs \"font-size:\">\n"); 
fprintf( SVGpf, "<!ENTITY fw \"font-weight:\">\n"); 
fprintf( SVGpf, "<!ENTITY fst \"font-style:\">\n"); 
fprintf( SVGpf, "<!ENTITY sw \"stroke-width:\">\n"); 
fprintf( SVGpf, "<!ENTITY as \"text-anchor:start;\">\n"); 
fprintf( SVGpf, "<!ENTITY am \"text-anchor:middle;\">\n"); 
fprintf( SVGpf, "<!ENTITY ae \"text-anchor:end;\">\n"); 
fprintf( SVGpf, "]>\n");
fprintf( SVGpf, "<!-- Generated by ploticus (http://ploticus.sourceforge.net/)\n");
fprintf( SVGpf, "Title: %s\n",name);
fprintf( SVGpf, "SVG Driver by B.Traill\n");
fprintf( SVGpf, "-->\n");

SVGbbofs = ftell( SVGpf ); /* remember location of the viewBox line so we can update it later.. -scg */

/* these two statements will be overridden at eof when bounding box is known.. -scg  */
fprintf( SVGpf, "<svg viewBox=\"0 0  %-5.2f %-5.2f\">\n", SVGx_size,SVGy_size); 
fprintf( SVGpf, "<g tranform=\"translate(0,0)\"                 \n" ); /* scg - padding */
if (Debug) fprintf( SVGpf, "<!-- Name:%s dev:%c f:%s pixs_inch:%d Ux:%f Uy:%f Upleftx:%f  Uplefty:%f -->\n",name, dev, f, pixs_inch, Ux, Uy, Upleftx, Uplefty );
/* print out default style */
fprintf( SVGpf, "<g style=\"%s%s%s\">\n",SVGdef_fill,SVGdef_stroke,SVGdef_font);
strcpy(SVGstyle,"");
strcpy(SVGnew_style,"");
SVGstyle_in_prog = 0;

}


/* ============================= */
SVGmoveto( x, y )
double x, y;
{
double x1,y1;

x = ( x * SVGpixs_inch ) +MARG_X; 
y = SVGy_size - (( y * SVGpixs_inch ) +MARG_Y); 

if (!SVGpath_in_prog) {
	SVGset_style();
	SVGprint_style();
	fprintf( SVGpf, "<path d=\"");
	SVGpath_in_prog =1;
	fprintf( SVGpf, "M%.6g% .6g", x, y );
} else { 
	x1 = x - SVGlast_x;
	y1 = y - SVGlast_y;
	fprintf( SVGpf, "m%.5g% .5g", x1, y1 );
}
SVGlast_x = x;
SVGlast_y = y;

}


/* ============================= */
SVGlineto( x, y )
double x, y;
{
double x1,y1;
x = ( x * SVGpixs_inch ) +MARG_X; 
y = SVGy_size - (( y * SVGpixs_inch ) +MARG_Y); 

if (!SVGpath_in_prog) {
	SVGset_style();
	SVGprint_style();
	fprintf( SVGpf, "<path d=\"");
	SVGpath_in_prog =1;
	fprintf( SVGpf, "L%.6g% .6g", x, y ); 
} else { 
	x1 = x - SVGlast_x;
	y1 = y - SVGlast_y;
	fprintf( SVGpf, "l%.5g% .5g", x1, y1 );
}
SVGlast_x = x;
SVGlast_y = y;
}

/* ============================== */
SVGclosepath()
{
if (Debug) fprintf( SVGpf,  "<!--closepath-->\n"); 
}

/* ============================== */
SVGstroke( )
{
/* fprintf( SVGpf,  "<!--stroke -->\n"); */
char dash[50] = "";

if  (SVGdash_style[0] != '\0') {
	sprintf(dash," stroke-dasharray=\"%s\"",SVGdash_style);
}

if (SVGpath_in_prog) {
	fprintf( SVGpf, "\" fill=\"none\"%s/>\n",dash); 
} 
SVGpath_in_prog =0;
}


/* ============================= */
SVGpath( x, y )
double x, y;
{

double x1,y1;
/* fprintf( SVGpf,  "<!--path %-5.2f %-5.2f-->\n", x, y );  */


x = ( x * SVGpixs_inch ) +MARG_X; 
y = SVGy_size - (( y * SVGpixs_inch ) +MARG_Y); 

if (!SVGpath_in_prog) {
	SVGset_style();
	SVGprint_style();
	fprintf( SVGpf, "<path d=\"");
	SVGpath_in_prog =1;
	fprintf( SVGpf, "L%.6g% .6g", x, y );
} else { 
	x1 = x - SVGlast_x;
	y1 = y - SVGlast_y;
	fprintf( SVGpf, "l%.5g% .5g", x1, y1 );
}
SVGlast_x = x;
SVGlast_y = y;


}

/* ============================= */
/* set current color for text and lines */
SVGcolor( color )
char *color;
{
int i, n;
double r, g, b, Ergb_to_gray();
int red,green,blue;
int slen;

/* color parameter can be in any of these forms:
   "rgb(R,G,B)"  where R(ed), G(reen), and B(lue) are 0.0 (none) to 1.0 (full)
   "gray(S)"	 where S is 0.0 (black) to 1.0 (white)
   "S"		 same as above
    or, a color name such as "blue" (see color.c)
*/ 

for( i = 0, slen = strlen( color ); i < slen; i++ ) {
	if( GL_member( color[i], "(),/:|-" ) ) color[i] = ' ';
	else color[i] = tolower( color[i] );
	}
if( strncmp( color, "rgb", 3 )==0 ) {
	n = sscanf( color, "%*s %lf %lf %lf", &r, &g, &b );
	if( n != 3 ) { Eerr( 12031, "Invalid color", color ); return(1); }
	r *= 255;
	g *= 255;
	b *= 255;
	red = (int)r;	
	green = (int)g;	
	blue = (int)b;	
	 sprintf( SVGcur_color, "#%02x%02x%02x", red, green, blue);
	}
else if( strncmp( color, "gray", 4 )==0 || strncmp( color, "grey", 4 )==0 ) {
	int gray;
	n = sscanf( color, "%*s %lf", &r );
	if( n != 1 ) { Eerr( 12031, "Invalid color", color ); return(1); }
	gray = r * 256;
	sprintf( SVGcur_color, "#%02x%02x%02x", gray, gray, gray);
	}
else if( GL_goodnum( color, &i ) ) {
	float no;
	int gray;
	sscanf(color,"%f",&no);

	gray = no * 256;

	 sprintf( SVGcur_color, "#%02x%02x%02x", gray, gray, gray);
	}
else	{	/* color names */
	Ecolorname_to_rgb( color, &r, &g, &b );

	r *= 255;
	g *= 255;
	b *= 255;
	red = (int)r;	
	green = (int)g;	
	blue = (int)b;	
	sprintf( SVGcur_color, "#%02x%02x%02x", red, green, blue);
	}


return( 0 );
}


/* ============================== */
/* fill current path with current color */
SVGfill( )
{

	if (SVGpath_in_prog) {
		fprintf( SVGpf, "z\" fill=\"%s\" stroke=\"%s\"/>\n",SVGcur_color,SVGcur_color);
	} 

	SVGpath_in_prog =0;

}

/* ============================== */
SVGpaper( i )
int i;
{
	if (Debug) fprintf( SVGpf,  "<!--paper-->\n"); 
	return( 1 ); /* paper orientation has no meaning with SVG */
}
/* ================================= */

esc_txt_svg(esc_txt, s)
char *esc_txt;
char *s;
{
	while (*s) {
		switch (*s) {
			case '&' : strcat(esc_txt,"&amp;"); break;
			case '<' : strcat(esc_txt,"&lt;"); break;
			case '>' : strcat(esc_txt,"&gt;"); break;
			default	: strncat(esc_txt,s,1);
		}
		++s;
	}
}

/* ================================= */
SVGtext( com, x, y, s, w )
char com;
double x, y;
char s[];
double w;
{
	char transform[200];
	char esc_txt[4096];

	x = (x*SVGpixs_inch)+MARG_X;  y = SVGy_size - ((y*SVGpixs_inch)+MARG_Y); w *= SVGpixs_inch;

	/* set the text alignment */
	if (com == 'T') strcpy(SVGalign, "start");
	else if (com == 'C') strcpy(SVGalign, "middle");
	else if (com == 'J') strcpy(SVGalign, "end");

	*esc_txt = '\0';
	esc_txt_svg(esc_txt, s);

	if (SVGchdir) {
		sprintf(transform, " transform=\"rotate(-%d,%.2f,%.2f)\" ",SVGchdir,x,y);
	} else {
		strcpy(transform,"");
	}

	SVGset_style();
	SVGprint_style();

	fprintf(SVGpf, "<text x=\"%.2f\" y=\"%.2f\" stroke=\"none\"%s>%s</text>\n",
			x,y,transform,esc_txt);

}


/* ================================= */
SVGpointsize( p )
int p;
{
/*	fprintf( SVGpf,  "<!-- pointsize %d -->\n",p);  */
	SVGcurrpointsz = p;
}


/* ================================== */
SVGfont( f )
char f[];
{
/*	fprintf( SVGpf,  "<!-- font %s -->\n",f);  */
	if (f[0] == '/') {
		strcpy( SVGfont_name, ++f );
	} else {
		strcpy( SVGfont_name, f );
	}
}

/* ================================== */
SVGchardir( t )
int t;
{
	/*	fprintf( SVGpf,  "<!-- chardir %d -->\n",t);  */
	SVGchdir = t;
}


/* ================================== */
SVGlinetype( s, x, y )
char s[];
double x, y;
{
/* X = line width;  Y = dash pattern magnification (0.1 to 10)
 *  S indicates dash pattern.  If S is "0", an unbroken (normal) line is produced.
 *  If S is "1" through "8", a preset dash pattern is used.  Otherwise, S is
 *  assumed to hold the dash pattern string "[ n1 n2 n3.. ]".	
 */
static int dash[10][6]= { {0,0,0,0,0,0}, {1,1}, {3,1}, {5,1}, {2,1,1,1}, {4,1,1,1}, {6,1,1,1}, 
			  {2,1,1,1,1,1}, {4,1,1,1,1,1}, {6,1,1,1,1,1} };
int ltype, i;

if (Debug) fprintf( SVGpf,  "<!--linetype:%s  x:%f y:%f -->\n",s,x,y); 
strcpy(SVGdash_style,"");
SVGline_width = x ;

if(  s[0] == '\0' || strcmp( s, "0" )==0 ) strcpy(SVGdash_style,"");
else 	{
	char *p= SVGdash_style;

	if( strlen( s ) > 1 ) { 
		ltype = 0; 
		sscanf( s, "%d %d %d %d %d %d", &dash[0][0], &dash[0][1], &dash[0][2], 
			&dash[0][3], &dash[0][4], &dash[0][5] );
		}
	else ltype = atoi( s );

	/* p += sprintf(p,"stroke-dasharray=\""); */
	for( i = 0; i < 6; i++ ) {
		if( dash[ ltype ][ i ] > 0 ) {
			p += sprintf( p,"%3.1f,", dash[ ltype ][ i ] * y );
		}
	}
	p--;
	*p = '\0';
	}
}
	

/* =================================== */

SVGshow()
{
	if (Debug) fprintf( SVGpf,  "<!--show-->\n"); 
}

/* =================================== */

SVGnewpage( p )
int p;
{
	if (Debug) fprintf( SVGpf,  "<!--newpage-->\n"); 
}

/* =================================== */

/* EPStrailer( int pp, double x1, double y1, double x2, double y2 ) */
SVGtrailer( pp, x1, y1, x2, y2 )
int pp; 
double x1, y1, x2, y2;
{
char *buf;
FILE *outfp;
int i;

if (SVGstyle_in_prog) {
	fprintf( SVGpf, "</g>");
}

fprintf( SVGpf, "</g>" );   /* close default style */
fprintf( SVGpf, "</g>" );   /* close translation transform -scg */
fprintf( SVGpf, "</svg>\n" );

/* now go back and update viewbox - scg */
fseek( SVGpf, SVGbbofs, SEEK_SET );	
fprintf( SVGpf, "<svg viewBox=\"0 0 %-5.2f %-5.2f\">\n", (x2-x1) * SVGpixs_inch, (y2-y1) * SVGpixs_inch );
fprintf( SVGpf, "<g transform=\"translate(%-5.2f,%-5.2f)\" >", 
	x1*SVGpixs_inch *-1.0, 
	(SVGy_size - (y2 * SVGpixs_inch)) *-1.0 );
if( SVGdotag ) {
	printf( "<embed src=\"%s\" name=\"SVGEmbed\" width=\"%-5.2f\" height=\"%-5.2f\" \n",
		SVGfilename, (x2-x1)* SVGpixs_inch, (y2-y1) * SVGpixs_inch );
	printf( "type=\"image/svg-xml\" pluginspage=\"http://www.adobe.com/svg/viewer/install/\"> \n" );
	}

fclose( SVGpf );

/* if temp file used, do read/write.. -scg */
if( SVGtmpfilename[0] != '\0' ) {
	SVGpf = fopen( SVGtmpfilename, "r" );
	if( SVGpf == NULL ) { Eerr( 2487, "cannot reopen temp file", SVGtmpfilename ); exit( 1 ); }
#ifdef WZ
	if( SVGcompress ) {
		if( SVGstdout ) outfp = (FILE *) gzdopen( 1, SVGcompressmode );  /* stdout = 1 */
		else outfp = (FILE *) gzopen( SVGfilename, SVGcompressmode );
		if( outfp == NULL ) { Eerr( 2488, "cannot open output file", SVGfilename ); exit( 1 ); }
		} 
#endif
	buf = SVGstyle; /* reuse */
	while( fgets( buf, 999, SVGpf ) != NULL ) {
#ifdef WZ
		if( SVGcompress ) gzprintf( outfp, "%s", buf ); 
		else 
#endif
			printf( "%s", buf );
		}
	fclose( SVGpf );
	unlink( SVGtmpfilename );
#ifdef WZ
	if( SVGcompress && !SVGstdout ) {
		gzclose( outfp );
		chmod( SVGfilename, 00644 );
		}
#endif
	}
else chmod( SVGfilename, 00644 );


	
return( 0 );
}


/* ================================= */
/* ========== */
/* SVGfontname - given a base name (such as Helvetica) and a modifier
   such as I (italic) B (bold) or BI (bold italic), build the 
	font style and weight strings */

SVGfontname( basename, name )
char *basename; 
char *name; /* in: B, I, or BI.  out: still the font name but statics now hold the font style and weight */
{
	int i, slen;

	for( i = 0, slen = strlen( name ); i < slen; i++ ) name[i] = tolower( name[i] );

	strcpy (SVGfont_weight, "");
	strcpy (SVGfont_style, "");

	if( strcmp( name, "b" )==0 ) {
		strcpy (SVGfont_weight, "bold");
	}

	if( strcmp( name, "i" )==0 ) {
		strcpy (SVGfont_style, "italic");
	}

	if( strcmp( name, "bi" )==0 ) {
		strcpy (SVGfont_weight, "bold");
		strcpy (SVGfont_style, "italic");
	}
	if (basename[0] == '/') {
		strcpy( name, basename++ );
	} else {
		strcpy( name, basename );
	}

	return( 0 );
}

/* ================================ */
/* SHOWTAG - set flag to write a suitable html <embed> tag to stdout */
SVGshowtag( mode )
int mode;
{
SVGdotag = mode;
return( 0 );
}

/* ================================= */
SVG_z( mode )
int mode;
{
#ifdef WZ
SVGcompress = mode;
#else
Eerr( 4275, "svgz not available in this build; making uncompressed svg", "" );
#endif
return( 0 );
}

/* ================================== */
SVG_zlevel( level )
int level;
{
if( level < 0 || level > 9 ) level = 9;
sprintf( SVGcompressmode, "wb%d", level );
return( 0 );
}

/* ================================== */
SVG_fmt( tag )
char *tag;
{
if( SVGcompress ) strcpy( tag, "svgz" );
else strcpy( tag, "svg" );
return( 0 );
}
