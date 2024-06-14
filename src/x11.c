/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


/* ===========================
   X11 xlib driver (c) 1989-96 Stephen C. Grubb

   No checking for redundant calls is made here.

   Revisions:
	951109 - made font selection more robust and accessable via GRAPHCORE_XFONT env var.
	951109 - reduced # of redraws when window is moved
	960424 - improved efficiency re loading fonts
		 added color capability
		 added backing store capability

   ===========================
*/
	

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include "x11shades.h"
#define Eerr(a,b,c) TDH_err(a,b,c)

#define MAX_D_ROWS 1000		/* LIMIT */
#define DEFAULT_FONT "-adobe-courier-bold-r-normal"
#define MISC_FONT "-misc-fixed-medium-r-normal"    /* available on NDG xterminals */
#define LAST_RESORT_FONT "9x15"
#define MAXCOLORS 40
#define MAXFONTS 8

#define E_EXPOSE 1010  /* window has been exposed and contents restored from last save */
#define E_RESIZE 1011  /* window has been resized */
#define E_COLORMAP_FULL 1100 /* alert app that there are no more colors */
#define E_MONODISPLAY 1101 /* alert app that display is monochrome */

double Exsca_inv(), Eysca_inv();
static double EXWoldx = 0, EXWoldy = 0;
static int EXWwaitflag;
static Display	*EXWdisp;
static Window	EXWwin;
static int EXWscreen;
static GC EXWgc;
static char EXWdash[10][6]= {
		  {1}, {1,1}, {3,1}, {5,1}, {2,1,1,1}, {4,1,1,1}, {6,1,1,1}, 
		  {2,1,1,1,1,1}, {4,1,1,1,1,1}, {6,1,1,1,1,1} };
static int EXWndash[10] = { 1, 2, 2, 2, 4, 4, 4, 6, 6, 6 };

/* create pixmap tiles for shading.. */
static Pixmap EXW_s_00, EXW_s_10, EXW_s_20, EXW_s_30, EXW_s_40, EXW_s_50, EXW_s_60, 
	EXW_s_70, EXW_s_80, EXW_s_90, EXW_s1_0;
static Pixmap EXWshadetile;

/* polygon vertexes */
static XPoint EXWvlist[MAX_D_ROWS];
static int EXWnvtx = 0;

static int EXWwinheight, EXWwinwidth;   /* current size of window */
static int EXWwinheight0, EXWwinwidth0; /* original size of window */
static int EXWupperleftx, EXWupperlefty; /* screen pixel position of upper left of window */
static double EXWtextscale = 1.0;
static int EXWmapped; /* 1 if window mapped (visible) 0 if not */

/**** text ****/
static XFontStruct *EXWfont; /* the full name of the current font */
static Font EXWfontlist[MAXFONTS]; /* fonts for various text sizes loaded at setup time */
static XFontStruct *EXWfontstructlist[MAXFONTS]; /* info for fonts */
static char EXWfontset[120]; /* the font short name - known to be available */


/**** color ****/
struct EXWrgbp {
	unsigned short r, g, b;
	unsigned long pix;
	};
static struct EXWrgbp EXWrgblist[ MAXCOLORS ]; /* list of r,g,b colors already allocated */
static int rgb_avail = 0;  /* next available slot in above array. */
static int EXWcolordisplay;  /* 1 if color, 0 if not */
static Colormap EXWdefault_cmap; 
static int EXWdisplay_depth;
static int EXWcolormessagedone = 0; /* 1 if the "Failure to allocate color" message has
					already been displayed. */


/**** backing store ****/
static Pixmap EXWbackstore;
static int EXWbackstoreused = 0;



/* =========================== */
/* get the graphcore environment handles.. */
EXWgethandle( display, window, gc )
Display *display;
Window *window;
GC *gc;
{
display = EXWdisp;
window = &EXWwin;
gc = &EXWgc;
}

/* ========================== */
/* ========================== */
/* ========================== */
EXWsetup( name, pixels_inch, x_max, y_max, upperleftx, upperlefty )
char name[];
int pixels_inch;
double x_max, y_max;
int upperleftx, upperlefty;
{
char inbuf[20];
XSizeHints win_size;
XEvent event;
int n = 1;
int nplanes; /* # of planes of display */
unsigned long fg, bg; /* forground and background for setting pixmaps */
char *userfont;
char *getenv();
int stat;
Font fnt;
int ptsizes[8];
int i;
double foo;

ptsizes[0] = 8;
ptsizes[1] = 10;
ptsizes[2] = 12;
ptsizes[3] = 14;
ptsizes[4] = 18;
ptsizes[5] = 24;

/* get window size in pixels */
EXWwinwidth =  (int)(x_max * pixels_inch ); 
EXWwinheight = (int)(y_max * pixels_inch );

/* remember original size.. */
EXWwinwidth0 = EXWwinwidth; 
EXWwinheight0 = EXWwinheight;

/* remember position */
EXWupperleftx = upperleftx;
EXWupperlefty = upperlefty;

/* initialize parameters */
EXWfont = NULL;
EXWtextscale = 1.0;

/* open the display indicated in DISPLAY env var.. */
if(( EXWdisp = XOpenDisplay( NULL )) == NULL ){
	Eerr( 12040, "X Display not properly identified.  Set DISPLAY=", "" );
	exit( 1 );
	}

/* see if user font env variable is defined.. */
userfont = getenv( "GRAPHCORE_XFONT" );

/* window size */
EXWscreen = DefaultScreen( EXWdisp );

win_size.flags = PPosition | PSize;
win_size.x = EXWupperleftx;
win_size.y = EXWupperlefty;
win_size.width = EXWwinwidth;
win_size.height = EXWwinheight;

/* create the window */
EXWwin = XCreateSimpleWindow (
	EXWdisp,
	RootWindow(EXWdisp, EXWscreen),	/*  parent window */
	win_size.x,
	win_size.y,		/*  location	  */
	win_size.width,
	win_size.height,	/*  size          */
	5,			/*  border	  */
	BlackPixel(EXWdisp,EXWscreen),	/*  forground	  */
	WhitePixel(EXWdisp,EXWscreen)	/*  background	  */
	);

XSetWMNormalHints( EXWdisp, EXWwin, &win_size );

XStoreName( EXWdisp, EXWwin, name );


/* determine whether screen is monochrome or color.. */
EXWdisplay_depth = DefaultDepth( EXWdisp, EXWscreen );
EXWdefault_cmap = DefaultColormap( EXWdisp, EXWscreen );
if( EXWdisplay_depth > 1 ) EXWcolordisplay = 1;
else 	{
	EXWcolordisplay = 0; 
	Ehe( 0.0, 0.0, E_MONODISPLAY );
	}
	

/* create pattern grays for monochrome displays */
fg = BlackPixel(EXWdisp, EXWscreen);
bg = WhitePixel(EXWdisp, EXWscreen);
nplanes = XDisplayPlanes( EXWdisp, EXWscreen );
EXW_s_00 = XCreatePixmapFromBitmapData( EXWdisp, EXWwin, (char *)EXW_im_00, 16, 16, fg, bg, nplanes);
EXW_s_10 = XCreatePixmapFromBitmapData( EXWdisp, EXWwin, (char *)EXW_im_10, 16, 16, fg, bg, nplanes);
EXW_s_20 = XCreatePixmapFromBitmapData( EXWdisp, EXWwin, (char *)EXW_im_20, 16, 16, fg, bg, nplanes);
EXW_s_30 = XCreatePixmapFromBitmapData( EXWdisp, EXWwin, (char *)EXW_im_30, 16, 16, fg, bg, nplanes);
EXW_s_40 = XCreatePixmapFromBitmapData( EXWdisp, EXWwin, (char *)EXW_im_40, 16, 16, fg, bg, nplanes);
EXW_s_50 = XCreatePixmapFromBitmapData( EXWdisp, EXWwin, (char *)EXW_im_50, 16, 16, fg, bg, nplanes);
EXW_s_60 = XCreatePixmapFromBitmapData( EXWdisp, EXWwin, (char *)EXW_im_60, 16, 16, fg, bg, nplanes);
EXW_s_70 = XCreatePixmapFromBitmapData( EXWdisp, EXWwin, (char *)EXW_im_70, 16, 16, fg, bg, nplanes);
EXW_s_80 = XCreatePixmapFromBitmapData( EXWdisp, EXWwin, (char *)EXW_im_80, 16, 16, fg, bg, nplanes);
EXW_s_90 = XCreatePixmapFromBitmapData( EXWdisp, EXWwin, (char *)EXW_im_90, 16, 16, fg, bg, nplanes);
EXW_s1_0 = XCreatePixmapFromBitmapData( EXWdisp, EXWwin, (char *)EXW_im1_0, 16, 16, fg, bg, nplanes);


/*  create graphics context */
EXWgc = XCreateGC( EXWdisp, EXWwin, 0, NULL );


/* set foreground and background colors.. */
XSetForeground( EXWdisp, EXWgc, BlackPixel( EXWdisp, EXWscreen ));
XSetBackground( EXWdisp, EXWgc, WhitePixel( EXWdisp, EXWscreen )); /* added SCG 950907 */

/* set other attributes.. */
XSetLineAttributes( EXWdisp, EXWgc, 1, LineSolid, CapButt, JoinMiter );
XSetFillStyle( EXWdisp, EXWgc, FillSolid ); /* FillSolid is used for lines & text;
						we switch over to FillTiled for
						polygons below.. */
EXWshadetile = EXW_s_00;
XSetTile( EXWdisp, EXWgc, EXWshadetile );


/* find a font to use.. */
stat = EXWloadfont( userfont, 10, &fnt ); 		/* user's choice font */
if( stat == 0 ) strcpy( EXWfontset, userfont );
else	{
	stat = EXWloadfont( DEFAULT_FONT, 10, &fnt ); /* default font */
	if( stat == 0 ) strcpy( EXWfontset, DEFAULT_FONT );
	else	{
		stat = EXWloadfont( MISC_FONT, 10, &fnt );    /* available on ndg xterms */
		if( stat == 0 ) strcpy( EXWfontset, MISC_FONT );
		else	{
			stat = EXWloadfont( LAST_RESORT_FONT, 0, &fnt );	/* last resort.. */
			if( stat == 0 ) strcpy( EXWfontset, LAST_RESORT_FONT );
			else {
				Eerr( 12041, "Cannot load an X font.", "" );
				exit(1);
				}
			}
		}
	}

/* load a range of point sizes in the requested font.. */
for( i = 0; i < 6; i++ ) {
	stat = EXWloadfont( EXWfontset, ptsizes[i], &EXWfontlist[i] );
	if( stat != 0 ) Eerr( 12042, "X Font load error", EXWfontset );
	EXWfontstructlist[i] = EXWfont;
	}
/* set the initial default font.. */
EXWpointsize( 10, &foo );


/* create pixmap for backing store.. */
EXWbackstore = XCreatePixmap( EXWdisp, EXWwin, EXWwinwidth, EXWwinheight, EXWdisplay_depth );


/* Set up for events  */
/*  we want: (1) mouse clicks (2) Key presses (3) expose events (4) resize events */
XSelectInput (
	EXWdisp,
	EXWwin,
	ButtonPressMask | ExposureMask | KeyPressMask | StructureNotifyMask
	);


/* map the window */
XMapWindow( EXWdisp, EXWwin );
EXWmapped = 1;

/* wait for initial expose event to draw.. */
while( 1 ) {
	XNextEvent( EXWdisp, &event );
	if( event.type == Expose ) break;
	}
}
/* ============================== */
/* save window to backing store.. */
EXWsavewin( )
{
XCopyArea( EXWdisp, EXWwin, EXWbackstore, EXWgc, 0, 0, EXWwinwidth, EXWwinheight, 0, 0 );
EXWbackstoreused = 1;
return( 0 );

}
/* ============================== */
/* restore window from backing store.. */

EXWrestorewin( )
{
XCopyArea( EXWdisp, EXWbackstore, EXWwin, EXWgc, 0, 0, EXWwinwidth, EXWwinheight, 0, 0 );
return( 0 );
}

/* =============================== */
EXWevent( event )
XEvent event;
{
int x, y;
unsigned eid;
char s[10];
XEvent fooevent;
int index, status;
FILE *fp;
int charcount;
KeySym ks;
XComposeStatus compose; /* not used */
char buf[80];
int i;

if( event.type == KeyPress ) {
	x = event.xkey.x;
	y = event.xkey.y;

	charcount = XLookupString( (XKeyEvent *) &event, buf, 80, &ks, &compose );

	/* screen out shift, control, alt, etc. */
	if( ks >= 0xFFE1 && ks <= 0xFFEE ) EXWwaitflag = 0; 

	else	{

		eid = buf[0];


		/* arrow keys, pg up, pg dn  - added scg 9/9/98 */
		/* home=550  left = 551   up = 552   right = 553    down = 554   
		   pg up=555   pg down=556  end=557 */
		if( ks >= 0xFF50 && ks <= 0xFF58 ) {
			ks &= 0x00FF;
			eid = ks + 470;
			EXWwaitflag = Ehe( Exsca_inv( x ), Eysca_inv( y ), eid );
			}
	
		/* all other keys */
		else	{
			/* eid &= 0x007F; */
			ks &= 0x00FF;
			if( charcount < 1 ) eid = ks - 96; /* map control chars to ascii 1-26 */
			EXWwaitflag = Ehe( Exsca_inv( x ), Eysca_inv( y ), eid );
			}
		}
	}
else if( event.type == ButtonPress ) {
	x = event.xbutton.x;
	y = event.xbutton.y;
	if( event.xbutton.button == Button1 ) eid = 1001;
	else if( event.xbutton.button == Button2 ) eid = 1002;
	else if( event.xbutton.button == Button3 ) eid = 1003;
	EXWwaitflag = Ehe( Exsca_inv( x ), Eysca_inv( y ), eid );
	}
else if( event.type == Expose ) {
	if( event.xexpose.count == 0 ) {
		Ehe( 0.0, 0.0, E_EXPOSE );
		}
	}

else if( event.type == ConfigureNotify ) { 

	/* on openwin, this event is generated when window is moved, and when it is
	 * resized.  We need do nothing when it is simply moved; when it is resized
	 * we need to recalculate and redisplay everything.. */

	if( (double)event.xconfigure.width != EXWwinwidth ||
	   (double)(event.xconfigure.height) != EXWwinheight ) { /* if size changed.. */

		Pixmap tmppixmap;
		int minh, minw;

		/* see which width and which height are smallest, new size or old size */
		if( event.xconfigure.width < EXWwinwidth ) minw = event.xconfigure.width;
		else minw = EXWwinwidth;
		if( event.xconfigure.height < EXWwinheight ) minh = event.xconfigure.height;
		else minh = EXWwinheight;

		/* now set the window size */
		EXWwinwidth = event.xconfigure.width; 
		EXWwinheight = event.xconfigure.height;

		/* need to handle backing store.. */

		/* allocate a new pixmap sized to the new window size.. */
		tmppixmap = XCreatePixmap( EXWdisp, EXWwin, EXWwinwidth, 
			EXWwinheight, EXWdisplay_depth );

		/* copy from window to new backing store to get a blank area..*/
		XCopyArea( EXWdisp, EXWwin, tmppixmap, EXWgc, 0, 0, minw, minh, 0, 0 ); 

		/* copy from old backing store to new based on min size.. */
		XCopyArea( EXWdisp, EXWbackstore, tmppixmap, EXWgc, 0, 0, minw, minh, 0, 0 );

		/* free the old backing store.. */
		XFreePixmap( EXWdisp, EXWbackstore );

		/* and make EXWbackstore point to new backing store.. */
		EXWbackstore = tmppixmap;

		Ehe( (double)(event.xconfigure.width), 
			(double)(event.xconfigure.height), E_RESIZE );

		EXWsavewin( );
		}
	}
else if( event.type == MappingNotify ) { 
	XRefreshKeyboardMapping( (XMappingEvent *) &event );
	}
}
/* ============================== */
/* Code driven resize of window.
   To use this, caller must be prepared to redraw display when this routine returns.
   All parameters (except pixels_inch) may be sent as -1 = don't change */
   
/* EXWresizewin( int pixels_inch, int x, int y, double x_max, double y_max ) */
EXWresizewin( pixels_inch, x, y, x_max, y_max )
int pixels_inch, x, y;
double x_max, y_max;
{
XSizeHints win_size;

/* get window size in pixels, and consider it 'original' */
if( x_max > 0 ) { EXWwinwidth =  (int)(x_max * pixels_inch ); EXWwinwidth0 = EXWwinwidth; }
if( y_max > 0 ) {EXWwinheight = (int)(y_max * pixels_inch ); EXWwinheight0 = EXWwinheight; }

if( x > 0 ) EXWupperleftx = x;
if( y > 0 ) EXWupperlefty = y;

win_size.x = EXWupperleftx;
win_size.y = EXWupperlefty;
win_size.width = EXWwinwidth;
win_size.height = EXWwinheight;

/* resize the window */
XMoveResizeWindow ( EXWdisp,
	EXWwin,
	win_size.x,
	win_size.y,	
	win_size.width,	
	win_size.height	);

/* free and reallocate backing store at new size.. */
XFreePixmap( EXWdisp, EXWbackstore );
EXWbackstore = XCreatePixmap( EXWdisp, EXWwin, EXWwinwidth, EXWwinheight, EXWdisplay_depth );


/* XSetWMNormalHints( EXWdisp, EXWwin, &win_size ); */
XFlush( EXWdisp ); /* scg 2-13-96 */
return( 0 );
}

/* =============================== */
EXWwait()
{
XEvent event;
EXWwaitflag = 0;
while( ! EXWwaitflag ) {
	XNextEvent( EXWdisp, &event );
	EXWevent( event );
        }
}

/* ==================== dot */
EXWdot( x, y )
double x, y;
{
XDrawPoint( EXWdisp, EXWwin, EXWgc, Exsca( x ), Eysca( y ) );
}

/* ====================  line to */
EXWlineto(x,y)
double x, y;
{
int a, b, c, d;


a = Exsca( EXWoldx ); b = Eysca( EXWoldy ); c = Exsca( x ); d = Eysca( y );
XDrawLine( EXWdisp, EXWwin, EXWgc, a, b, c, d );
EXWoldx=x;
EXWoldy=y;
}

/* =====================  moveto */
EXWmoveto(x,y)
double x, y;
{
EXWoldx = x;
EXWoldy = y;
}

/* ===================== raw line - no conversion on coords */
EXWrawline( a, b, c, d )
int a, b, c, d;
{
XDrawLine( EXWdisp, EXWwin, EXWgc, a, b, c, d );
return( 0 );
}

/* ===================== set point size of type */
/* XWpointsize */
EXWpointsize( p, aw )
int p;
double *aw; /* width of one character -- returned */
{
int stat;
Font fnt;
int i;

/* scale based upon pixels/inch */
p = (int)(p * EXWtextscale);

/* use p to get index into font list.. */
if( p <= 6 )  i = 0;
else if( p == 7 || p == 8 )  i = 1;
else if( p == 9 || p == 10 )  i = 2;
else if( p >= 11 && p <= 14 ) i = 3;
else if( p >= 15 && p <= 18 ) i = 4;
else i = 5;

fnt = EXWfontlist[i]; /* access font from pre-allocated list */
EXWfont = EXWfontstructlist[i]; /* and get font info */

/* get the approximate text width.. */
*aw = Exsca_inv( XTextWidth( EXWfont, "A", 1 ) ); 

/* set the font.. */
XSetFont( EXWdisp, EXWgc, fnt );
return( 0 );
}

/* ======================== */
/* EXW LOADFONT - given short font name f, see if an X font is available for
   the requested size, or something close.. */
/* When this routine returns successfully, Font is returned in fnt, and
   XFontStruct is placed in EXWfont. */
/* EXWloadfont( char *f, int size ) */
EXWloadfont( f, size, fnt )
char *f;
int size;
Font *fnt;
{
char fontname[100];
char ofslist[12];
int i, p;
/* Font fnt; */

strcpy( ofslist, "EFDGCHBIAJ" ); /* used to vary the size.. */

if( f == NULL ) return( 1 );
if( f[0] == '\0' ) return( 1 );
if( strcmp( f, LAST_RESORT_FONT ) == 0 ) {
	strcpy( fontname, f );
	EXWfont = XLoadQueryFont( EXWdisp, fontname );
	if( EXWfont == NULL ) return( 1 );
	goto LOAD_IT;
	}

for( i = 0; i < 10; i++ ) { /* try 10 times with varying sizes until we get a hit.. */
	p = size + ( ofslist[i] - 'E' );
	sprintf( fontname, "%s--%d-*-*-*-*-*-*-*", f, p );
	EXWfont = XLoadQueryFont( EXWdisp, fontname );
	if( EXWfont != NULL ) break;
	}
if( i == 10 ) return( 1 ); /* not found */

LOAD_IT:
*fnt = XLoadFont( EXWdisp, fontname );
return( 0 );
}

/* ==================== EXWscaletext - when window is resized, to adjust text size */
EXWscaletext( f )
double f;
{
EXWtextscale = f;
}

/* ==================== EXWdisplay left adjusted text starting at current position */
EXWtext( s, aw )
char s[];
double *aw; /* actual string width in inches (sent back) */
{
/* XSetFillStyle( EXWdisp, EXWgc, FillSolid ); */  /* added SCG 950907 */
XDrawString( EXWdisp, EXWwin, EXWgc, Exsca( EXWoldx ), Eysca( EXWoldy ), s, strlen( s ) );
/* XSetFillStyle( EXWdisp, EXWgc, FillTiled ); */  /* added SCG 950907 */
*aw = Exsca_inv( XTextWidth( EXWfont, s, strlen( s ) ) );
EXWoldx += ( *aw );
}

/* ==================== EXWdisplay centered text */
EXWcentext( s, w, x, aw )
char s[];
double w;
double *x; /* actual X starting point in inches (sent back) */
double *aw; /* actual string width in inches (sent back) */
{
double width;

strip_ws( s );
width = Exsca_inv( XTextWidth( EXWfont, s, strlen( s ) ) );
EXWmoveto( EXWoldx+((w-width)/2.0), EXWoldy );
EXWtext( s, aw );
*x = Exsca_inv((int)( EXWoldx+((w-width)/2.0) ) );
}

/* ==================== EXWdisplay right-justified text */
EXWrightjust( s, w, x, aw )
char s[];
double w;
double *x; /* actual X starting point in inches (sent back) */
double *aw; /* actual string width in inches (sent back) */
{
double width;

strip_ws( s );
width = Exsca_inv( XTextWidth( EXWfont, s, strlen( s ) ) );
EXWmoveto( EXWoldx+(w-width), EXWoldy );
EXWtext( s, aw );
*x = Exsca_inv((int)(EXWoldx+(w-width) ));
}

/* ========= set line thickness and dash pattern attribs ============= */

EXWlinetype( t, w, mag )
char t[];
double w, mag;
{
int i, j, k;
int linewidth;
char dashlist[12];
int style;

if( t[0] == '\0' ) return( 0 );
linewidth = (w*1.6) + 0.5;
style = atoi( t );
if( style < 1 || style > 9 ) {
	XSetLineAttributes( EXWdisp, EXWgc, linewidth, LineSolid, CapButt, JoinMiter );
	}
else	{
	for( i = 0; i < EXWndash[ style % 10 ]; i++ ) {
		j = (int) ( EXWdash[ style %10 ][i] * mag );
		if( j < 1 ) j = 1;
		if( j > 127 ) j = 127; /* keep in char range */
		dashlist[i] = (char) j;
		}

	XSetDashes( EXWdisp, EXWgc, 0, dashlist, EXWndash[ atoi(t)%10 ] );
	XSetLineAttributes( EXWdisp, EXWgc, linewidth, LineOnOffDash, CapButt, JoinMiter );
	}
}

/* ==================== add to "fill path" */
EXWpath( x, y )
double x, y;
{
if( EXWnvtx == 0 ) { EXWvlist[0].x = Exsca( EXWoldx ); EXWvlist[0].y = Eysca( EXWoldy ); EXWnvtx++; }
if( EXWnvtx >= MAX_D_ROWS ) {
	Eerr( 12045, "X polygon: Too many points ", "" );
	return( 1 );
	}
EXWvlist[ EXWnvtx ].x = (short) (Exsca(x));
EXWvlist[ EXWnvtx ].y = (short) (Eysca(y));
EXWnvtx++;
}


/* ==================== fill prev. defined polygon path with current color */
EXWfill( )
{
if( !EXWcolordisplay ) {
	XSetTile( EXWdisp, EXWgc, EXWshadetile );
	XSetFillStyle( EXWdisp, EXWgc, FillTiled );   /* added SCG 11-10-95 */
	}

XFillPolygon( EXWdisp, EXWwin, EXWgc, EXWvlist, EXWnvtx, Complex, CoordModeOrigin );

/* set fillstyle back to solid and set tile back to black.. */
if( !EXWcolordisplay ) {
	XSetFillStyle( EXWdisp, EXWgc, FillSolid );   /* added SCG 11-10-95 */
	XSetTile( EXWdisp, EXWgc, EXW_s_00 );
	}
/* reset vertex counter */
EXWnvtx = 0;
}

/* ==================== set drawing color */
EXWcolor( color )
char *color;
{
int i, n;
double r, g, b, avg, atof(), Ergb_to_gray();
int slen;

/* color parameter can be in any of these forms:
   "rgb(R,G,B)"  where R(ed), G(reen), and B(lue) are 0.0 (none) to 1.0 (full)
   "hsb(H,S,B)"  where H(ue), S(aturation), and B(rightness) range from 0.0 to 1.0.
   "gray(S)"	 where S is 0.0 (black) to 1.0 (white)
   "S"		 same as above
*/ 
for( i = 0, slen = strlen( color ); i < slen; i++ ) {
	if( GL_member( color[i], "(),/:|-" ) ) color[i] = ' ';
	}

/* for now, convert to gray using the first parameter.. */
if( strncmp( color, "rgb", 3 )==0 ) {
	n = sscanf( color, "%*s %lf %lf %lf", &r, &g, &b );
	if( n != 3 ) { Eerr( 24780, "Invalid color", color ); return(1); }
	}
else if( strncmp( color, "gray", 4 )==0 || strncmp( color, "grey", 4 )==0 ) {
	n = sscanf( color, "%*s %lf", &r );
	if( n != 1 ) { Eerr( 24780, "Invalid color", color ); return(1); }
	g = b = r;
	}
else if( GL_goodnum( color, &i ) ) {
	r = atof( color );
	g = b = r;
	}
else Ecolorname_to_rgb( color, &r, &g, &b );
	

if( ! EXWcolordisplay ) {
	avg = Ergb_to_gray( r, g, b );
	
	/* select a tile for current shading.. */
	if( avg == 1.0 ) EXWshadetile = EXW_s1_0;  /* white */
	else if( avg <= .15 ) EXWshadetile = EXW_s_00; /* black */
	else if( avg > .95 ) EXWshadetile = EXW_s_90; /* greys.. */
	else if( avg > .90 ) EXWshadetile = EXW_s_80;
	else if( avg > .85 ) EXWshadetile = EXW_s_70;
	else if( avg > .75 ) EXWshadetile = EXW_s_60;
	else if( avg > .65 ) EXWshadetile = EXW_s_50;
	else if( avg > .55 ) EXWshadetile = EXW_s_40;
	else if( avg > .45 ) EXWshadetile = EXW_s_30;
	else if( avg > .35 ) EXWshadetile = EXW_s_20;
	else if( avg > .15 ) EXWshadetile = EXW_s_10;
	else EXWshadetile = (Pixmap ) 0;
	
	/* set color for lines and text.. */
	if( r >= 1.0 && g >= 1.0 && b >= 1.0 ) {  /* white */
		XSetForeground( EXWdisp, EXWgc, WhitePixel( EXWdisp, EXWscreen ));
		XSetBackground( EXWdisp, EXWgc, BlackPixel( EXWdisp, EXWscreen )); /* 950907 */
		}
	else {  /* black */
		XSetForeground( EXWdisp, EXWgc, BlackPixel( EXWdisp, EXWscreen ));
		XSetBackground( EXWdisp, EXWgc, WhitePixel( EXWdisp, EXWscreen )); /* 950907 */
		}
	}
else	{
	unsigned short sr, sg, sb;
	XColor colordef;
	int status;

	/* convert double r, g, b to unsigned short.. 1.0 to 65535 */
	sr = (unsigned short)(r * 65535.0);
	sg = (unsigned short)(g * 65535.0);
	sb = (unsigned short)(b * 65535.0);

	/* see if this color is in list of colors allocated so far.. */
	for( i = 0; i < rgb_avail; i++ ) {
		if( sr == EXWrgblist[i].r && sg == EXWrgblist[i].g && 
			sb == EXWrgblist[i].b  ) break;
		}
	if( i == rgb_avail ) { 	/* it wasn't found, add it to the list.. */
		if( rgb_avail >= MAXCOLORS ) {
			i = rgb_avail = 0;
			
			}
		colordef.red = sr;
		colordef.green = sg;
		colordef.blue = sb;
		status = XAllocColor( EXWdisp, EXWdefault_cmap, &colordef );
		if( status == 0 ) {
			if( !EXWcolormessagedone ) {
				Eerr( 12050, "warning, X colormap full; colors may be inaccurate", "" );
				/* Ehe( 0.0, 0.0, E_COLORMAP_FULL ); */ /* scg 11/22/99 */
				}
			EXWcolormessagedone = 1;
			XSetForeground( EXWdisp, EXWgc, colordef.pixel );
			return( 1 );
			}
		XSetForeground( EXWdisp, EXWgc, colordef.pixel );
		EXWrgblist[i].r = sr; /* colordef.red; */ 
		EXWrgblist[i].g = sg; /* colordef.green; */
		EXWrgblist[i].b = sb; /* colordef.blue; */ 
		EXWrgblist[i].pix = colordef.pixel;
		rgb_avail++;
		return( 1 );
		}
	else 	{
		XSetForeground( EXWdisp, EXWgc, EXWrgblist[i].pix ); 
		return( 1 );
		}
	}
}

/* ==================== asynchronous */
EXWasync()
{
XEvent event;
while( 1 ) {
	if( XCheckMaskEvent( EXWdisp, ButtonPressMask | ExposureMask | 
		KeyPressMask | StructureNotifyMask , &event ) != True ) return(1);
	EXWevent( event );
        }
}
/* ==================== */

EXWflush()
{
XFlush( EXWdisp );
}

/* ===================== */

EXWappear()
{
XEvent event;

if( EXWmapped ) return( 0 );

/* map the window */
XMapWindow( EXWdisp, EXWwin );
EXWmapped = 1;

/* redraw everything.. */
/* Egenall();   scg Nov 95 */
XFlush( EXWdisp );
}
/* ====================== */

EXWdisappear()
{
if( ! EXWmapped ) return( 0 );

/* unmap the window.. */
XUnmapWindow( EXWdisp, EXWwin );
EXWmapped = 0;
XFlush( EXWdisp );
}
