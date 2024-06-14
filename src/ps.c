/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */



/* Postscript driver.	(c) 1989-96 Steve Grubb, Marvin Newhouse

   Checking for redundant calls is not done here; should be done by caller.

	13 Nov 95 scg	upgraded to EPSF-3.0 guidelines, including:
			bounding box and page # information; and
			using "userdict begin" and "end" to ensure
			that dictionary stack is undisturbed.

			Added trailer generation.

			Added EPstdout to ensure that stdout doesn't get
			fclosed.

	16 Apr 96 scg, mmn  Incorporated the special characters feature 
			written by Marvin Newhouse.

	24 Apr 96 scg	Added color.

	27 Sep 01 scg   Added ISO char set encoding, as contributed by 
			Johan Hedin <johan@eCare.se>  May be turned off
			by setting Latin1 = 0.

			
*/

#include <stdio.h>
#include "special_chars.h"
#define Eerr(a,b,c)  TDH_err(a,b,c)

#define YES 1
#define NO 0
#define PORTRAIT 0
#define LANDSCAPE 1
#define MARG_X 14 
#define MARG_Y 8 
#define PAGWIDTH 600;
#define stricmp(a,b) strcasecmp(a,b)

static int EPdevice;		/* 'p' = monochrome, 'c' = color, 'e' = eps (color) */
static int EPorient;		/* paper orientation (-1 = not done) */
static int EPorgx, EPorgy;	/* location of origin on page */
static int EPtheta;		/* current rotation for page orientation */
static char EPfont[50];		/* current font name */
static int EPchdir;	 	/* char direction in degrees */
static int EPcurpointsz;		/* current char size in points */
static int EPspecialpointsz;		/* current char size in pts for special chars */
static int EPstdout;		/* 1 if Epf is stdout */
static int EPpaginate = 1;
double atof();
static FILE *Epf;

static int Latin1 = 1;		/* use latin1 character encoding */

/* ============================= */
EPSsetup( name, dev, f )    
char *name; /* arbitrary name */
char dev;  /* 'p' = monochrome   'c' = color   'e' = eps */
char *f;  /* file to put code in */
{  
char filename[256]; 

/* set globals */
if( dev != 'e' && dev != 'p' && dev != 'c' ) dev = 'p';
EPdevice = dev;
EPorient = -1;
strcpy( EPfont, "/Helvetica" );
EPtheta = 0;
EPchdir = 0;
EPcurpointsz = 10;
if( dev == 'e' ) EPpaginate = 0;


EPstdout = 0;
if( stricmp( f, "stdout" )==0 ) {
	Epf = stdout;
	EPstdout = 1;
	}
else if(  f[0] == '\0' ) {
	if( dev == 'e' ) {
		strcpy( filename, "out.eps" );
		}
	else	{
		Epf = stdout;
		EPstdout = 1;
		}
	}
else strcpy( filename, f );


if( !EPstdout ) {
	Epf = fopen( filename, "w" ); /* output file */
	if( Epf == NULL ) {
		Eerr( 12030, "Cannot open postscript output file", filename );
		exit(1);
		}
	}

/* print header */
fprintf( Epf,  "%%!PS-Adobe-3.0 EPSF-3.0\n" );
fprintf( Epf,  "%%%%Title: %s\n", name );
fprintf( Epf,  "%%%%Creator: ploticus (ploticus.sourceforge.net)\n" );
if( EPpaginate ) fprintf( Epf,  "%%%%Pages: (atend)\n" );
if( EPdevice == 'e' ) fprintf( Epf,  "%%%%BoundingBox: (atend)\n" );
fprintf( Epf,  "%%%%EndComments\n\n\n" );
fprintf( Epf,  "%%%%BeginProlog\n" );

	
/* set up macros (mainly to reduce output verbosity) */
fprintf( Epf,  "userdict begin\n" );
fprintf( Epf,  "/sset\n" );
fprintf( Epf,  "{ translate rotate } def \n" );
fprintf( Epf,  "/sclr\n" );
fprintf( Epf,  "{ rotate translate } def \n" );
fprintf( Epf,  "/mv { moveto } def\n" );
fprintf( Epf,  "/np { newpath } def\n" );
fprintf( Epf,  "/ln { lineto } def\n" );
fprintf( Epf,  "/st { stroke } def\n" ); 
fprintf( Epf,  "/sh { show } def\n" );
fprintf( Epf,  "/cent { stringwidth pop sub 2 div 0 rmoveto } def\n" );
fprintf( Epf,  "/rjust { stringwidth pop sub 0 rmoveto } def\n" );

if( Latin1 ) {
  /* Iso latin 1 encoding from gnuplot, as contributed by Johan Hedin <johan@eCare.se>  - added 9/27/01 */
  fprintf( Epf,  "/reencodeISO {\n");
  fprintf( Epf,  "dup dup findfont dup length dict begin\n");
  fprintf( Epf,  "{ 1 index /FID ne { def }{ pop pop } ifelse } forall\n");
  fprintf( Epf,  "currentdict /CharStrings known {\n");
  fprintf( Epf,  "        CharStrings /Idieresis known {\n");
  fprintf( Epf,  "                /Encoding ISOLatin1Encoding def } if\n");
  fprintf( Epf,  "} if\n");
  fprintf( Epf,  "currentdict end definefont\n");
  fprintf( Epf,  "} def\n");
  fprintf( Epf,  "/ISOLatin1Encoding [\n");
  fprintf( Epf,  "/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n");
  fprintf( Epf,  "/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n");
  fprintf( Epf,  "/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n");
  fprintf( Epf,  "/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n");
  fprintf( Epf,  "/space/exclam/quotedbl/numbersign/dollar/percent/ampersand/quoteright\n");
  fprintf( Epf,  "/parenleft/parenright/asterisk/plus/comma/minus/period/slash\n");
  fprintf( Epf,  "/zero/one/two/three/four/five/six/seven/eight/nine/colon/semicolon\n");
  fprintf( Epf,  "/less/equal/greater/question/at/A/B/C/D/E/F/G/H/I/J/K/L/M/N\n");
  fprintf( Epf,  "/O/P/Q/R/S/T/U/V/W/X/Y/Z/bracketleft/backslash/bracketright\n");
  fprintf( Epf,  "/asciicircum/underscore/quoteleft/a/b/c/d/e/f/g/h/i/j/k/l/m\n");
  fprintf( Epf,  "/n/o/p/q/r/s/t/u/v/w/x/y/z/braceleft/bar/braceright/asciitilde\n");
  fprintf( Epf,  "/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n");
  fprintf( Epf,  "/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n");
  fprintf( Epf,  "/.notdef/dotlessi/grave/acute/circumflex/tilde/macron/breve\n");
  fprintf( Epf,  "/dotaccent/dieresis/.notdef/ring/cedilla/.notdef/hungarumlaut\n");
  fprintf( Epf,  "/ogonek/caron/space/exclamdown/cent/sterling/currency/yen/brokenbar\n");
  fprintf( Epf,  "/section/dieresis/copyright/ordfeminine/guillemotleft/logicalnot\n");
  fprintf( Epf,  "/hyphen/registered/macron/degree/plusminus/twosuperior/threesuperior\n");
  fprintf( Epf,  "/acute/mu/paragraph/periodcentered/cedilla/onesuperior/ordmasculine\n");
  fprintf( Epf,  "/guillemotright/onequarter/onehalf/threequarters/questiondown\n");
  fprintf( Epf,  "/Agrave/Aacute/Acircumflex/Atilde/Adieresis/Aring/AE/Ccedilla\n");
  fprintf( Epf,  "/Egrave/Eacute/Ecircumflex/Edieresis/Igrave/Iacute/Icircumflex\n");
  fprintf( Epf,  "/Idieresis/Eth/Ntilde/Ograve/Oacute/Ocircumflex/Otilde/Odieresis\n");
  fprintf( Epf,  "/multiply/Oslash/Ugrave/Uacute/Ucircumflex/Udieresis/Yacute\n");
  fprintf( Epf,  "/Thorn/germandbls/agrave/aacute/acircumflex/atilde/adieresis\n");
  fprintf( Epf,  "/aring/ae/ccedilla/egrave/eacute/ecircumflex/edieresis/igrave\n");
  fprintf( Epf,  "/iacute/icircumflex/idieresis/eth/ntilde/ograve/oacute/ocircumflex\n");
  fprintf( Epf,  "/otilde/odieresis/divide/oslash/ugrave/uacute/ucircumflex/udieresis\n");
  fprintf( Epf,  "/yacute/thorn/ydieresis\n");
  fprintf( Epf,  "] def\n");
  }


/* load default font */
if( Latin1 ) fprintf( Epf,  "%s reencodeISO\n", EPfont ); /* scg 9/27/01 */
else fprintf( Epf,  "%s findfont\n", EPfont );

fprintf( Epf,  "%d scalefont setfont\n", (int) EPcurpointsz );
fprintf( Epf,  "%%%%EndProlog\n" ); 
}

/* ============================= */
EPSclose()
{
if( EPstdout ) return( 0 ); /* we don't want to close stdout */
if( Epf != NULL )fclose( Epf );
}


/* ============================= */
EPSmoveto( x, y )
double x, y;
{

/* convert to p.s. units (1/72 inch) */
x = ( x * 72.0 ) +MARG_X; y = ( y * 72.0 ) + MARG_Y; 
fprintf( Epf,  "np\n%-5.2f %-5.2f mv\n", x, y ); 
}


/* ============================= */
EPSlineto( x, y )
double x, y;
{
/* convert to p.s. units */
x = ( x * 72 ) +MARG_X; 
y = ( y * 72 ) +MARG_Y; 
fprintf( Epf,  "%-5.2f %-5.2f ln\n", x, y );
}

/* ============================== */
EPSclosepath()
{
fprintf( Epf, "closepath\n" );
}

/* ============================== */
EPSstroke( )
{
fprintf( Epf, "st\n" );
}


/* ============================= */
EPSpath( x, y )
double x, y;
{
/* convert to p.s. units */
x = ( x * 72 ) +MARG_X; y = ( y * 72 ) + MARG_Y; 
fprintf( Epf,  "%-5.2f %-5.2f ln\n",  x, y );
}

/* ============================= */
/* set current color for text and lines */
EPScolor( color )
char *color;
{
int i, n;
double r, g, b, Ergb_to_gray();
int slen;

/* color parameter can be in any of these forms:
   "rgb(R,G,B)"  where R(ed), G(reen), and B(lue) are 0.0 (none) to 1.0 (full)
   "hsb(H,S,B)"  where H(ue), S(aturation), and B(rightness) range from 0.0 to 1.0.
   "cmyk(C,M,Y,K)"  where values are 0.0 to 1.0
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
	if( EPdevice == 'p' ) fprintf( Epf, "%g setgray\n", Ergb_to_gray( r, g, b ) );
	else fprintf( Epf, "%g %g %g setrgbcolor\n", r, g, b );
	}
else if( strncmp( color, "hsb", 3 )==0 ) {
	n = sscanf( color, "%*s %lf %lf %lf", &r, &g, &b );
	if( n != 3 ) { Eerr( 12031, "Invalid color", color ); return(1); }
	if( EPdevice == 'p' ) fprintf( Epf, "%g setgray\n", Ergb_to_gray( r, g, b ) );
	else fprintf( Epf, "%g %g %g sethsbcolor\n", r, g, b );
	}

else if( strncmp( color, "cmyk", 4 )==0 ) { /* added scg 10/26/00 */
	double c, m, y, k;
	n = sscanf( color, "%*s %lf %lf %lf %lf", &c, &m, &y, &k );
	if( n != 4 ) { Eerr( 12031, "Invalid color", color ); return(1); }
	if( EPdevice == 'p' ) fprintf( Epf, "%g setgray\n", Ergb_to_gray( r, g, b ) );
	else fprintf( Epf, "%g %g %g %g setcmykcolor\n", c, m, y, k );
	}

else if( strncmp( color, "gray", 4 )==0 || strncmp( color, "grey", 4 )==0 ) {
	n = sscanf( color, "%*s %lf", &r );
	if( n != 1 ) { Eerr( 12031, "Invalid color", color ); return(1); }
	fprintf( Epf, "%g setgray\n", r );
	}
else if( GL_goodnum( color, &i ) ) {
	fprintf( Epf, "%s setgray\n", color );
	}
else	{	/* color names */
	Ecolorname_to_rgb( color, &r, &g, &b );
	if( EPdevice == 'p' ) fprintf( Epf, "%g setgray\n", Ergb_to_gray( r, g, b ) );
	else fprintf( Epf, "%g %g %g setrgbcolor\n", r, g, b );
	}
return( 0 );
}


/* ============================== */
/* fill current path with current color */
EPSfill( )
{
fprintf( Epf,  "closepath\nfill\n" );
/* fprintf( Epf, "fill\n" ); */
}

/* ============================== */
EPSpaper( i )
int i;
{

if( EPdevice == 'e' ) return( 1 ); /* paper orientation has no meaning with EPS */

if( i == 1 ) { /* handle landscape mode-- it's put into portrait mode before beginning each page */
	EPorgx = PAGWIDTH; 
	EPorgy = 0; 
	EPtheta = 90; 
	fprintf( Epf,  "%d %d %d sset\n", EPtheta, EPorgx, EPorgy );
	} 
EPorient = (int) i;
}


/* ================================= */
EPStext( com, x, y, s, w )
char com;
double x, y;
char s[];
double w;
{
char str[400];
int i, j, k, slen;

x = (x*72)+MARG_X;  y = (y*72)+MARG_Y; w *= 72;

/* if text direction is not normal do a rotate then move.. */
if( EPchdir != 0 ) fprintf( Epf,  "%d %-5.2f %-5.2f sset 0 0 mv\n", EPchdir, x, y ); 
/* normal direction-- do a move.. */
else fprintf( Epf,  "%-5.2f %-5.2f mv ", x, y );

if( GL_member( com, "CJ" )) {
	strip_ws( s );

	/* escape out parens and substitute special char requests with 'm' for centering */
	for( i = 0, j = 0, slen = strlen( s ); i < slen; i++ ) {
		if( s[i] == '(' || s[i] == ')' ) { str[j++] = '\\'; str[j++] = s[i]; }
		else if( s[i] == '\\' && s[i+1] == '(' ) { str[j++] = 'm'; i += 3; }
	        else if( s[i] == '\\' && s[i+1] == 's' ) { str[j++] = 'm'; i += 3; }
	        else if( s[i] == '\\' ) { str[j++] = s[i]; str[j++] = s[i]; i += 3; }
		else str[j++] = s[i];
		}
	str[j] = '\0';
	
	/* centered text */
	if( com == 'C' ) fprintf( Epf,  "%-5.2f (%s) cent ", w, str ); 
	else if( com == 'J' ) fprintf( Epf,  "%-5.2f (%s) rjust ", w, str );
	}

EPspecialpointsz = EPcurpointsz;

/* print the string */
fprintf( Epf, "\n(" );
for( i = 0, slen = strlen( s ); i < slen; i++ ) {
	if( s[i] == '(' || s[i] == ')' ) { fprintf( Epf, "\\%c", s[i]); }
	else if( s[i] == '\\' && s[i+1] == 's' ) { EPSset_specialsz( &s[i+2] ); i += 3; }
	else if( s[i] == '\\' && s[i+1] == '(' ) { EPSprint_special( &s[i+2] ); i += 3; }
	else if( s[i] == '\\' ) fprintf( Epf, "\\%c", s[i] ); /* scg 1-6-97 */
	else { fprintf( Epf, "%c", s[i] ); }
	}
fprintf( Epf, ") sh\n" );

if( EPchdir != 0 ) fprintf( Epf,  "%-5.2f %-5.2f %d sclr\n", -x, -y, -EPchdir ); /* restore */
}


/* ================================= */
EPSpointsize( p )
int p;
{
EPcurpointsz = p;
if( Latin1 ) fprintf( Epf,  "%s reencodeISO\n", EPfont ); /* scg 09/27/01 */
else fprintf( Epf,  "%s findfont\n", EPfont ); 

fprintf( Epf,  "%d scalefont\nsetfont\n", p );
}


/* ================================== */
EPSfont( f )
char f[];
{
if( strcmp( f, EPfont ) != 0 ) {
	strcpy( EPfont, f );

	if( Latin1 ) fprintf( Epf,  "%s reencodeISO\n", EPfont ); /* scg 9/27/01 */
	else fprintf( Epf,  "%s findfont\n", EPfont ); 

	fprintf( Epf,  "%d scalefont setfont\n", EPcurpointsz );
	}
}

/* ================================== */
EPSchardir( t )
int t;
{
EPchdir = t;
}


/* ================================== */
EPSlinetype( s, x, y )
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

fprintf( Epf,  "%3.1f setlinewidth\n", x );
if(  s[0] == '\0' || strcmp( s, "0" )==0 ) fprintf( Epf,  "[] 0 setdash\n" );
else 	{
	if( strlen( s ) > 1 ) { 
		ltype = 0; 
		sscanf( s, "%d %d %d %d %d %d", &dash[0][0], &dash[0][1], &dash[0][2], 
			&dash[0][3], &dash[0][4], &dash[0][5] );
		}
	else ltype = atoi( s );
	fprintf( Epf,  "[" );
	for( i = 0; i < 6; i++ ) 
		if( dash[ ltype ][ i ] > 0 ) fprintf( Epf,  "%3.1f ", dash[ ltype ][ i ] * y );
	fprintf( Epf,  "] 0 setdash\n" );
	}
}
	

/* =================================== */

EPSshow()
{
if( EPorient == 1 )fprintf( Epf,  "%d %d %d sclr\n", -EPorgx, -EPorgy, -EPtheta ); /* restore rotation */
EPorient = -1; 
fprintf( Epf, "showpage\n" );
}

/* =================================== */

EPSnewpage( p )
int p;
{
if( EPpaginate )fprintf( Epf, "\n\n\n%%%%Page: %d %d\n", p, p );
}

/* =================================== */

/* EPStrailer( int pp, double x1, double y1, double x2, double y2 ) */
EPStrailer( pp, x1, y1, x2, y2 )
int pp; 
double x1, y1, x2, y2;
{
/* if( x1 < 0.0 ) x1 = 0.0;
 * if( y1 < 0.0 ) y1 = 0.0;
 * if( x2 < 0.0 ) x2 = 0.0;
 * if( y2 < 0.0 ) y2 = 0.0;
 */ /* negative values in bounding box should be ok - scg 1/22/01 */

fprintf( Epf, "end\n" );
fprintf( Epf, "\n\n\n%%%%Trailer\n" );
if( EPpaginate )fprintf( Epf, "%%%%Pages: %d\n", pp );
if( EPdevice == 'e' ) {
	fprintf( Epf, "%%%%BoundingBox: %5.2f %5.2f %5.2f %5.2f\n",
  	    ( x1 * 72 ) + MARG_X, ( y1 * 72 ) + MARG_Y, ( x2 * 72 ) + MARG_X, ( y2 * 72 ) + MARG_Y );
	}

fprintf( Epf, "%%%%EOF\n" );
if( EPstdout ) return( 0 ); /* we don't want to close stdout */
if( Epf != NULL )fclose( Epf );
return( 0 );
}

/* ================================= */
/* ================================= */
/* ================================= */
/* ================================= */
/* ================================= */
/* ================================= */
/* ================================= */

EPSset_specialsz( s )
char s[];
{
int i;
char tmp[5];
/*
 * Check for two characters to set the point size for special chars
 * This size will remain in effect for all special chars in this string
 * or until another \s is encountered
 */
if( GL_member(s[0],"+-0123456789") && GL_member(s[1],"0123456789") )
	{
	tmp[0] = s[0]; tmp[1] = s[1]; tmp[2] = '\0';
	if( GL_member(s[0], "+-") ) EPspecialpointsz = EPcurpointsz + atoi( tmp );
	else EPspecialpointsz = atoi( tmp );
	}
else 	{
	char sbuf[8];
	sprintf( sbuf, "\\s%c%c", s[0], s[1] );
	Eerr( 12033, "Invalid \\s operator", sbuf );
	}
}

/* ================================= */
/*
 * Lookup and generate PostScript for printing special chars
 *	using the  troff  conventions as input where they exist.
 * e.g. Input: \(bu	Outputs a bullet
 *      Input: \(dg	Outputs a dagger
 *      Input: \(<=	Outputs a less than or equal sign
 * MMN 072292
 */
EPSprint_special( s )
char s[];
{
int i;
char sbuf[8];

/* Check for two letter abbreviations which corresppond to a standard font encoding */
for( i = 0; strcmp( EPspecial_std[i][0],"@@") != 0 ; i++ ) {
	if( strncmp( EPspecial_std[i][0], s, 2 ) != 0 ) continue;

	/* end the current string and save the current font on the stack */
	fprintf( Epf, ") sh\ncurrentfont\n");

	/* Reset the font to the currentfont at the special point size  */
	if( Latin1 == 1 ) fprintf( Epf, "%s reencodeISO %d scalefont setfont", EPfont, EPspecialpointsz ); 
	else /* fprintf( Epf, "%s findfont %d scalefont setfont", EPfont, EPspecialpointsz ); */

	/* Show the character and reset the font from what's left on the stack */
	fprintf( Epf, " (\\%s) sh\nsetfont\n(", EPspecial_std[i][1]);
	return ( 1 );
	}
/* If not found ... */
/* Check for symbols in the symbol font encoding vector */
for( i = 0; strcmp(EPsymbol[i][0],"@@") != 0 ; i++ ) {
	if( strncmp(EPsymbol[i][0], s, 2 ) != 0 ) continue;

	/* end the current string and save the current font on the stack */
	fprintf( Epf, ") sh\ncurrentfont\n");

	/* Set the font to Symbol */
	fprintf( Epf, "/Symbol findfont %d scalefont setfont", EPspecialpointsz ); /* don't reencode Symbol font */

	/* Show the character and reset the font from what's left on the stack */
	fprintf( Epf, " (\\%s) sh\nsetfont\n(", EPsymbol[i][1]);
	return ( 1 );
	}
/* symbol abbrev. not found */
sprintf( sbuf, "\\(%c%c", s[0], s[1] );
Eerr( 12035, "warning: special symbol not found", sbuf );
fprintf( Epf, "??" );
return( -1 );
}
/* ========== */
/* EPSfontname - given a base name (such as /Helvetica) and a modifier
   such as I (italic) B (bold) or BI (bold italic), build the postscript
   font name and copy it into name. */

EPSfontname( basename, name )
char *basename; 
char *name; /* in: B, I, or BI.  out: full postscript font name */
{
int i, slen;

for( i = 0, slen = strlen( name ); i < slen; i++ ) name[i] = tolower( name[i] );

if( strcmp( name, "" )== 0 || strcmp( name, "r" )==0 ) {
	strcpy( name, basename );
	return( 0 );
	}

if( strcmp( name, "b" )==0 ) {
	if( strcmp( basename, "/Helvetica" )==0 ) strcpy( name, "/Helvetica-Bold" );
	else if( strcmp( basename, "/Helvetica-Oblique" )==0 ) 
		strcpy( name, "/Helvetica-BoldOblique" );
	else if( strcmp( basename, "/Times-Roman" )==0 ) strcpy( name, "/Times-Bold" );
	else if( strcmp( basename, "/Times-Italic" )==0 ) strcpy( name, "/Times-BoldItalic" );
	else if( strcmp( basename, "/Courier" )==0 ) strcpy( name, "/Courier-Bold" );
	else if( strcmp( basename, "/Courier-Oblique" )==0 ) strcpy( name, "/Courier-BoldOblique" );
	else if( strcmp( basename, "/Palatino-Roman" )==0 ) strcpy( name, "/Palatino-Bold" );
	else sprintf( name, "%s-Bold", basename );
	}
if( strcmp( name, "i" )==0 ) {
	if( strcmp( basename, "/Helvetica" )==0 ) strcpy( name, "/Helvetica-Oblique" );
	else if( strcmp( basename, "/Times-Roman" )==0 ) strcpy( name, "/Times-Italic" );
	else if( strcmp( basename, "/Times-Bold" )==0 ) strcpy( name, "/Times-BoldItalic" );
	else if( strcmp( basename, "/Courier" )==0 ) strcpy( name, "/Courier-Oblique" );
	else if( strcmp( basename, "/Courier-Bold" )==0 ) strcpy( name, "/Courier-BoldOblique" );
	else sprintf( name, "%s-Italic", basename );
	}
if( strcmp( name, "bi" )==0 ) {
	if( strncmp( basename, "/Helvetica", 10 )==0 ) strcpy( name, "/Helvetica-BoldOblique" );
	else if( strncmp( basename, "/Times", 6 )==0 ) strcpy( name, "/Times-BoldItalic" );
	else if( strncmp( basename, "/Courier", 8 )==0 ) strcpy( name, "/Courier-BoldOblique" );
	else sprintf( name, "%s-BoldItalic", basename );
	}
return( 0 );
}

