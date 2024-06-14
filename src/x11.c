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

#define MAX_D_ROWS 1000		/* LIMIT: max# points in one polygon */
#define DEFAULT_FONT "-adobe-courier-bold-r-normal"
#define MISC_FONT "-misc-fixed-medium-r-normal"    /* available on NDG xterminals */
#define LAST_RESORT_FONT "9x15"
#define MAXCOLORS 40
#define MAXFONTS 8

#define E_EXPOSE 1010  /* window has been exposed and contents restored from last save */
#define E_RESIZE 1011  /* window has been resized */
#define E_COLORMAP_FULL 1100 /* alert app that there are no more colors */
#define E_MONODISPLAY 1101 /* alert app that display is monochrome */


/* no initstatic for X11 .. doesn't seem necessary now */
static double x_oldx = 0, x_oldy = 0;
static int x_waitflag;
static Display	*x_disp;
static Window	x_win;
static int x_screen;
static GC x_gc;
static char x_dash[10][6]= {
		  {1}, {1,1}, {3,1}, {5,1}, {2,1,1,1}, {4,1,1,1}, {6,1,1,1}, 
		  {2,1,1,1,1,1}, {4,1,1,1,1,1}, {6,1,1,1,1,1} };
static int x_ndash[10] = { 1, 2, 2, 2, 4, 4, 4, 6, 6, 6 };

/* create pixmap tiles for shading.. */
static Pixmap x_s_00, x_s_10, x_s_20, x_s_30, x_s_40, x_s_50, x_s_60, x_s_70, x_s_80, x_s_90, x_s1_0;
static Pixmap x_shadetile;

/* polygon vertexes */
static XPoint x_vlist[MAX_D_ROWS];
static int x_nvtx = 0;

static int x_winheight, x_winwidth;   /* current size of window */
static int x_winheight0, x_winwidth0; /* original size of window */
static int x_upperleftx, x_upperlefty; /* screen pixel position of upper left of window */
static double x_textscale = 1.0;
static int x_mapped; /* 1 if window mapped (visible) 0 if not */

/**** text ****/
static XFontStruct *x_font; /* the full name of the current font */
static Font x_fontlist[MAXFONTS]; /* fonts for various text sizes loaded at setup time */
static XFontStruct *x_fontstructlist[MAXFONTS]; /* info for fonts */
static char x_fontset[120]; /* the font short name - known to be available */


/**** color ****/
struct x_rgbp {
	unsigned short r, g, b;
	unsigned long pix;
	};
static struct x_rgbp x_rgblist[ MAXCOLORS ]; /* list of r,g,b colors already allocated */
static int rgb_avail = 0;  /* next available slot in above array. */
static int x_colordisplay;  /* 1 if color, 0 if not */
static Colormap x_default_cmap; 
static int x_display_depth;
static int x_colormessagedone = 0; /* 1 if the "Failure to allocate color" message has
					already been displayed. */

/**** backing store ****/
static Pixmap x_backstore;
static int x_backstoreused = 0;


/***** functions *****/
extern double PLG_xsca_inv(), PLG_ysca_inv();
#define Exsca( h )	PLG_xsca( h )
#define Eysca( h )	PLG_ysca( h )
#define Exsca_inv( h )	PLG_xsca_inv( h )
#define Eysca_inv( h )	PLG_ysca_inv( h )
#define Eerr(a,b,c)	TDH_err(a,b,c)



/* =========================== */
/* get the graphcore environment handles.. */
PLGX_gethandle( display, window, gc )
Display *display;
Window *window;
GC *gc;
{
display = x_disp;
window = &x_win;
gc = &x_gc;
}

/* ========================== */
/* ========================== */
/* ========================== */
PLGX_setup( name, pixels_inch, x_max, y_max, upperleftx, upperlefty )
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
x_winwidth =  (int)(x_max * pixels_inch ); 
x_winheight = (int)(y_max * pixels_inch );

/* remember original size.. */
x_winwidth0 = x_winwidth; 
x_winheight0 = x_winheight;

/* remember position */
x_upperleftx = upperleftx;
x_upperlefty = upperlefty;

/* initialize parameters */
x_font = NULL;
x_textscale = 1.0;

/* open the display indicated in DISPLAY env var.. */
if(( x_disp = XOpenDisplay( NULL )) == NULL ) return( Eerr( 12040, "X Display not properly identified.  Set DISPLAY=", "" ) );

/* see if user font env variable is defined.. */
userfont = getenv( "GRAPHCORE_XFONT" );

/* window size */
x_screen = DefaultScreen( x_disp );

win_size.flags = PPosition | PSize;
win_size.x = x_upperleftx;
win_size.y = x_upperlefty;
win_size.width = x_winwidth;
win_size.height = x_winheight;

/* create the window */
x_win = XCreateSimpleWindow (
	x_disp,
	RootWindow(x_disp, x_screen),	/*  parent window */
	win_size.x,
	win_size.y,		/*  location	  */
	win_size.width,
	win_size.height,	/*  size          */
	5,			/*  border	  */
	BlackPixel(x_disp,x_screen),	/*  forground	  */
	WhitePixel(x_disp,x_screen)	/*  background	  */
	);

XSetWMNormalHints( x_disp, x_win, &win_size );

XStoreName( x_disp, x_win, name );


/* determine whether screen is monochrome or color.. */
x_display_depth = DefaultDepth( x_disp, x_screen );
x_default_cmap = DefaultColormap( x_disp, x_screen );
if( x_display_depth > 1 ) x_colordisplay = 1;
else 	{
	x_colordisplay = 0; 
	PLG_he( 0.0, 0.0, E_MONODISPLAY );
	}
	

/* create pattern grays for monochrome displays */
fg = BlackPixel(x_disp, x_screen);
bg = WhitePixel(x_disp, x_screen);
nplanes = XDisplayPlanes( x_disp, x_screen );
x_s_00 = XCreatePixmapFromBitmapData( x_disp, x_win, (char *)x_im_00, 16, 16, fg, bg, nplanes);
x_s_10 = XCreatePixmapFromBitmapData( x_disp, x_win, (char *)x_im_10, 16, 16, fg, bg, nplanes);
x_s_20 = XCreatePixmapFromBitmapData( x_disp, x_win, (char *)x_im_20, 16, 16, fg, bg, nplanes);
x_s_30 = XCreatePixmapFromBitmapData( x_disp, x_win, (char *)x_im_30, 16, 16, fg, bg, nplanes);
x_s_40 = XCreatePixmapFromBitmapData( x_disp, x_win, (char *)x_im_40, 16, 16, fg, bg, nplanes);
x_s_50 = XCreatePixmapFromBitmapData( x_disp, x_win, (char *)x_im_50, 16, 16, fg, bg, nplanes);
x_s_60 = XCreatePixmapFromBitmapData( x_disp, x_win, (char *)x_im_60, 16, 16, fg, bg, nplanes);
x_s_70 = XCreatePixmapFromBitmapData( x_disp, x_win, (char *)x_im_70, 16, 16, fg, bg, nplanes);
x_s_80 = XCreatePixmapFromBitmapData( x_disp, x_win, (char *)x_im_80, 16, 16, fg, bg, nplanes);
x_s_90 = XCreatePixmapFromBitmapData( x_disp, x_win, (char *)x_im_90, 16, 16, fg, bg, nplanes);
x_s1_0 = XCreatePixmapFromBitmapData( x_disp, x_win, (char *)x_im1_0, 16, 16, fg, bg, nplanes);


/*  create graphics context */
x_gc = XCreateGC( x_disp, x_win, 0, NULL );


/* set foreground and background colors.. */
XSetForeground( x_disp, x_gc, BlackPixel( x_disp, x_screen ));
XSetBackground( x_disp, x_gc, WhitePixel( x_disp, x_screen )); /* added SCG 950907 */

/* set other attributes.. */
XSetLineAttributes( x_disp, x_gc, 1, LineSolid, CapButt, JoinMiter );
XSetFillStyle( x_disp, x_gc, FillSolid ); /* FillSolid is used for lines & text;
						we switch over to FillTiled for
						polygons below.. */
x_shadetile = x_s_00;
XSetTile( x_disp, x_gc, x_shadetile );


/* find a font to use.. */
stat = PLGX_loadfont( userfont, 10, &fnt ); 		/* user's choice font */
if( stat == 0 ) strcpy( x_fontset, userfont );
else	{
	stat = PLGX_loadfont( DEFAULT_FONT, 10, &fnt ); /* default font */
	if( stat == 0 ) strcpy( x_fontset, DEFAULT_FONT );
	else	{
		stat = PLGX_loadfont( MISC_FONT, 10, &fnt );    /* available on ndg xterms */
		if( stat == 0 ) strcpy( x_fontset, MISC_FONT );
		else	{
			stat = PLGX_loadfont( LAST_RESORT_FONT, 0, &fnt );	/* last resort.. */
			if( stat == 0 ) strcpy( x_fontset, LAST_RESORT_FONT );
			else return( Eerr( 12041, "Cannot load an X font.", "" ) );
			}
		}
	}

/* load a range of point sizes in the requested font.. */
for( i = 0; i < 6; i++ ) {
	stat = PLGX_loadfont( x_fontset, ptsizes[i], &x_fontlist[i] );
	if( stat != 0 ) Eerr( 12042, "X Font load error", x_fontset );
	x_fontstructlist[i] = x_font;
	}
/* set the initial default font.. */
PLGX_pointsize( 10, &foo );


/* create pixmap for backing store.. */
x_backstore = XCreatePixmap( x_disp, x_win, x_winwidth, x_winheight, x_display_depth );


/* Set up for events  */
/*  we want: (1) mouse clicks (2) Key presses (3) expose events (4) resize events */
XSelectInput (
	x_disp,
	x_win,
	ButtonPressMask | ExposureMask | KeyPressMask | StructureNotifyMask
	);


/* map the window */
XMapWindow( x_disp, x_win );
x_mapped = 1;

/* wait for initial expose event to draw.. */
while( 1 ) {
	XNextEvent( x_disp, &event );
	if( event.type == Expose ) break;
	}

return( 0 );
}
/* ============================== */
/* save window to backing store.. */
PLGX_savewin( )
{
XFlush( x_disp );
XCopyArea( x_disp, x_win, x_backstore, x_gc, 0, 0, x_winwidth, x_winheight, 0, 0 );
x_backstoreused = 1;
return( 0 );

}
/* ============================== */
/* restore window from backing store.. */

PLGX_restorewin( )
{
XCopyArea( x_disp, x_backstore, x_win, x_gc, 0, 0, x_winwidth, x_winheight, 0, 0 );
return( 0 );
}

/* =============================== */
PLGX_event( event )
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
	if( ks >= 0xFFE1 && ks <= 0xFFEE ) x_waitflag = 0; 

	else	{

		eid = buf[0];


		/* arrow keys, pg up, pg dn  - added scg 9/9/98 */
		/* home=550  left = 551   up = 552   right = 553    down = 554   
		   pg up=555   pg down=556  end=557 */
		if( ks >= 0xFF50 && ks <= 0xFF58 ) {
			ks &= 0x00FF;
			eid = ks + 470;
			x_waitflag = PLG_he( Exsca_inv( x ), Eysca_inv( y ), eid );
			}
	
		/* all other keys */
		else	{
			/* eid &= 0x007F; */
			ks &= 0x00FF;
			if( charcount < 1 ) eid = ks - 96; /* map control chars to ascii 1-26 */
			x_waitflag = PLG_he( Exsca_inv( x ), Eysca_inv( y ), eid );
			}
		}
	}
else if( event.type == ButtonPress ) {
	x = event.xbutton.x;
	y = event.xbutton.y;
	if( event.xbutton.button == Button1 ) eid = 1001;
	else if( event.xbutton.button == Button2 ) eid = 1002;
	else if( event.xbutton.button == Button3 ) eid = 1003;
	x_waitflag = PLG_he( Exsca_inv( x ), Eysca_inv( y ), eid );
	}
else if( event.type == Expose ) {
	if( event.xexpose.count == 0 ) {
		PLG_he( 0.0, 0.0, E_EXPOSE );
		}
	}

else if( event.type == ConfigureNotify ) { 

	/* on openwin, this event is generated when window is moved, and when it is
	 * resized.  We need do nothing when it is simply moved; when it is resized
	 * we need to recalculate and redisplay everything.. */

	if( (double)event.xconfigure.width != x_winwidth ||
	   (double)(event.xconfigure.height) != x_winheight ) { /* if size changed.. */

		Pixmap tmppixmap;
		int minh, minw;

		/* see which width and which height are smallest, new size or old size */
		if( event.xconfigure.width < x_winwidth ) minw = event.xconfigure.width;
		else minw = x_winwidth;
		if( event.xconfigure.height < x_winheight ) minh = event.xconfigure.height;
		else minh = x_winheight;

		/* now set the window size */
		x_winwidth = event.xconfigure.width; 
		x_winheight = event.xconfigure.height;

		/* need to handle backing store.. */

		/* allocate a new pixmap sized to the new window size.. */
		tmppixmap = XCreatePixmap( x_disp, x_win, x_winwidth, 
			x_winheight, x_display_depth );

		/* copy from window to new backing store to get a blank area..*/
		XCopyArea( x_disp, x_win, tmppixmap, x_gc, 0, 0, minw, minh, 0, 0 ); 

		/* copy from old backing store to new based on min size.. */
		XCopyArea( x_disp, x_backstore, tmppixmap, x_gc, 0, 0, minw, minh, 0, 0 );

		/* free the old backing store.. */
		XFreePixmap( x_disp, x_backstore );

		/* and make x_backstore point to new backing store.. */
		x_backstore = tmppixmap;

		PLG_he( (double)(event.xconfigure.width), 
			(double)(event.xconfigure.height), E_RESIZE );

		PLGX_savewin( );
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
   
PLGX_resizewin( pixels_inch, x, y, x_max, y_max )
int pixels_inch, x, y;
double x_max, y_max;
{
XSizeHints win_size;

/* get window size in pixels, and consider it 'original' */
if( x_max > 0 ) { x_winwidth =  (int)(x_max * pixels_inch ); x_winwidth0 = x_winwidth; }
if( y_max > 0 ) { x_winheight = (int)(y_max * pixels_inch ); x_winheight0 = x_winheight; }

if( x > 0 ) x_upperleftx = x; 
if( y > 0 ) x_upperlefty = y;

win_size.x = x_upperleftx; win_size.y = x_upperlefty; 
win_size.width = x_winwidth; win_size.height = x_winheight;

/* resize the window */
XMoveResizeWindow ( x_disp, x_win, win_size.x, win_size.y, win_size.width, win_size.height );

/* free and reallocate backing store at new size.. */
XFreePixmap( x_disp, x_backstore );
x_backstore = XCreatePixmap( x_disp, x_win, x_winwidth, x_winheight, x_display_depth );


/* XSetWMNormalHints( x_disp, x_win, &win_size ); */
XFlush( x_disp ); /* scg 2-13-96 */
return( 0 );
}

/* =============================== */
PLGX_wait()
{
XEvent event;
x_waitflag = 0;
while( ! x_waitflag ) {
	XNextEvent( x_disp, &event );
	PLGX_event( event );
        }
}

/* ==================== dot */
PLGX_dot( x, y )
double x, y;
{
XDrawPoint( x_disp, x_win, x_gc, Exsca( x ), Eysca( y ) );
}

/* ====================  line to */
PLGX_lineto(x,y)
double x, y;
{
int a, b, c, d;

a = Exsca( x_oldx ); b = Eysca( x_oldy ); c = Exsca( x ); d = Eysca( y );
XDrawLine( x_disp, x_win, x_gc, a, b, c, d );
x_oldx=x;
x_oldy=y;
}

/* =====================  moveto */
PLGX_moveto(x,y)
double x, y;
{
x_oldx = x;
x_oldy = y;
}

/* ===================== raw line - no conversion on coords */
PLGX_rawline( a, b, c, d )
int a, b, c, d;
{
XDrawLine( x_disp, x_win, x_gc, a, b, c, d );
return( 0 );
}

/* ===================== set point size of type */
/* pointsize */
PLGX_pointsize( p, aw )
int p;
double *aw; /* width of one character -- returned */
{
int stat;
Font fnt;
int i;

/* scale based upon pixels/inch */
p = (int)(p * x_textscale);

/* use p to get index into font list.. */
if( p <= 6 )  i = 0;
else if( p == 7 || p == 8 )  i = 1;
else if( p == 9 || p == 10 )  i = 2;
else if( p >= 11 && p <= 14 ) i = 3;
else if( p >= 15 && p <= 18 ) i = 4;
else i = 5;

fnt = x_fontlist[i]; /* access font from pre-allocated list */
x_font = x_fontstructlist[i]; /* and get font info */

/* get the approximate text width.. */
*aw = Exsca_inv( XTextWidth( x_font, "A", 1 ) ); 

/* set the font.. */
XSetFont( x_disp, x_gc, fnt );
return( 0 );
}

/* ======================== */
/* x_ LOADFONT - given short font name f, see if an X font is available for
   the requested size, or something close.. */
/* When this routine returns successfully, Font is returned in fnt, and
   XFontStruct is placed in x_font. */

PLGX_loadfont( f, size, fnt )
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
	x_font = XLoadQueryFont( x_disp, fontname );
	if( x_font == NULL ) return( 1 );
	goto LOAD_IT;
	}

for( i = 0; i < 10; i++ ) { /* try 10 times with varying sizes until we get a hit.. */
	p = size + ( ofslist[i] - 'E' );
	sprintf( fontname, "%s--%d-*-*-*-*-*-*-*", f, p );
	x_font = XLoadQueryFont( x_disp, fontname );
	if( x_font != NULL ) break;
	}
if( i == 10 ) return( 1 ); /* not found */

LOAD_IT:
*fnt = XLoadFont( x_disp, fontname );
return( 0 );
}

/* ==================== x_scaletext - when window is resized, to adjust text size */
PLGX_scaletext( f )
double f;
{
x_textscale = f;
}

/* ==================== x_display left adjusted text starting at current position */
PLGX_text( s, aw )
char s[];
double *aw; /* actual string width in inches (sent back) */
{
/* XSetFillStyle( x_disp, x_gc, FillSolid ); */  /* added SCG 950907 */
XDrawString( x_disp, x_win, x_gc, Exsca( x_oldx ), Eysca( x_oldy ), s, strlen( s ) );
/* XSetFillStyle( x_disp, x_gc, FillTiled ); */  /* added SCG 950907 */
*aw = Exsca_inv( XTextWidth( x_font, s, strlen( s ) ) );
x_oldx += ( *aw );
}

/* ==================== x_display centered text */
PLGX_centext( s, w, x, aw )
char s[];
double w;
double *x; /* actual X starting point in inches (sent back) */
double *aw; /* actual string width in inches (sent back) */
{
double width;

GL_strip_ws( s );
width = Exsca_inv( XTextWidth( x_font, s, strlen( s ) ) );
PLGX_moveto( x_oldx+((w-width)/2.0), x_oldy );
PLGX_text( s, aw );
*x = Exsca_inv((int)( x_oldx+((w-width)/2.0) ) );
}

/* ==================== x_display right-justified text */
PLGX_rightjust( s, w, x, aw )
char s[];
double w;
double *x; /* actual X starting point in inches (sent back) */
double *aw; /* actual string width in inches (sent back) */
{
double width;

GL_strip_ws( s );
width = Exsca_inv( XTextWidth( x_font, s, strlen( s ) ) );
PLGX_moveto( x_oldx+(w-width), x_oldy );
PLGX_text( s, aw );
*x = Exsca_inv((int)(x_oldx+(w-width) ));
}

/* ========= set line thickness and dash pattern attribs ============= */

PLGX_linetype( t, w, mag )
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
	XSetLineAttributes( x_disp, x_gc, linewidth, LineSolid, CapButt, JoinMiter );
	}
else	{
	for( i = 0; i < x_ndash[ style % 10 ]; i++ ) {
		j = (int) ( x_dash[ style %10 ][i] * mag );
		if( j < 1 ) j = 1;
		if( j > 127 ) j = 127; /* keep in char range */
		dashlist[i] = (char) j;
		}

	XSetDashes( x_disp, x_gc, 0, dashlist, x_ndash[ atoi(t)%10 ] );
	XSetLineAttributes( x_disp, x_gc, linewidth, LineOnOffDash, CapButt, JoinMiter );
	}
}

/* ==================== add to "fill path" */
PLGX_path( x, y )
double x, y;
{
if( x_nvtx == 0 ) { x_vlist[0].x = Exsca( x_oldx ); x_vlist[0].y = Eysca( x_oldy ); x_nvtx++; }
if( x_nvtx >= MAX_D_ROWS ) {
	Eerr( 12045, "X polygon: Too many points ", "" );
	return( 1 );
	}
x_vlist[ x_nvtx ].x = (short) (Exsca(x));
x_vlist[ x_nvtx ].y = (short) (Eysca(y));
x_nvtx++;
}


/* ==================== fill prev. defined polygon path with current color */
PLGX_fill( )
{
if( !x_colordisplay ) {
	XSetTile( x_disp, x_gc, x_shadetile );
	XSetFillStyle( x_disp, x_gc, FillTiled );   /* added SCG 11-10-95 */
	}

XFillPolygon( x_disp, x_win, x_gc, x_vlist, x_nvtx, Complex, CoordModeOrigin );

/* set fillstyle back to solid and set tile back to black.. */
if( !x_colordisplay ) {
	XSetFillStyle( x_disp, x_gc, FillSolid );   /* added SCG 11-10-95 */
	XSetTile( x_disp, x_gc, x_s_00 );
	}
/* reset vertex counter */
x_nvtx = 0;
}

/* ==================== set drawing color */
PLGX_color( color )
char *color;
{
int i, n;
double r, g, b, avg, atof(), PLG_rgb_to_gray();
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
else PLG_colorname_to_rgb( color, &r, &g, &b );
	

if( ! x_colordisplay ) {
	avg = PLG_rgb_to_gray( r, g, b );
	
	/* select a tile for current shading.. */
	if( avg == 1.0 ) x_shadetile = x_s1_0;  /* white */
	else if( avg <= .15 ) x_shadetile = x_s_00; /* black */
	else if( avg > .95 ) x_shadetile = x_s_90; /* greys.. */
	else if( avg > .90 ) x_shadetile = x_s_80;
	else if( avg > .85 ) x_shadetile = x_s_70;
	else if( avg > .75 ) x_shadetile = x_s_60;
	else if( avg > .65 ) x_shadetile = x_s_50;
	else if( avg > .55 ) x_shadetile = x_s_40;
	else if( avg > .45 ) x_shadetile = x_s_30;
	else if( avg > .35 ) x_shadetile = x_s_20;
	else if( avg > .15 ) x_shadetile = x_s_10;
	else x_shadetile = (Pixmap ) 0;
	
	/* set color for lines and text.. */
	if( r >= 1.0 && g >= 1.0 && b >= 1.0 ) {  /* white */
		XSetForeground( x_disp, x_gc, WhitePixel( x_disp, x_screen ));
		XSetBackground( x_disp, x_gc, BlackPixel( x_disp, x_screen )); /* 950907 */
		}
	else {  /* black */
		XSetForeground( x_disp, x_gc, BlackPixel( x_disp, x_screen ));
		XSetBackground( x_disp, x_gc, WhitePixel( x_disp, x_screen )); /* 950907 */
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
		if( sr == x_rgblist[i].r && sg == x_rgblist[i].g && 
			sb == x_rgblist[i].b  ) break;
		}
	if( i == rgb_avail ) { 	/* it wasn't found, add it to the list.. */
		if( rgb_avail >= MAXCOLORS ) {
			i = rgb_avail = 0;
			
			}
		colordef.red = sr;
		colordef.green = sg;
		colordef.blue = sb;
		status = XAllocColor( x_disp, x_default_cmap, &colordef );
		if( status == 0 ) {
			if( !x_colormessagedone ) Eerr( 12050, "warning, X colormap full; colors may be inaccurate", "" );
			x_colormessagedone = 1;
			XSetForeground( x_disp, x_gc, colordef.pixel );
			return( 1 );
			}
		XSetForeground( x_disp, x_gc, colordef.pixel );
		x_rgblist[i].r = sr; /* colordef.red; */ 
		x_rgblist[i].g = sg; /* colordef.green; */
		x_rgblist[i].b = sb; /* colordef.blue; */ 
		x_rgblist[i].pix = colordef.pixel;
		rgb_avail++;
		return( 1 );
		}
	else 	{
		XSetForeground( x_disp, x_gc, x_rgblist[i].pix ); 
		return( 1 );
		}
	}
}

/* ==================== asynchronous */
PLGX_async()
{
XEvent event;
while( 1 ) {
	if( XCheckMaskEvent( x_disp, ButtonPressMask | ExposureMask | 
		KeyPressMask | StructureNotifyMask , &event ) != True ) return(1);
	PLGX_event( event );
        }
}
/* ==================== */

PLGX_flush()
{
XFlush( x_disp );
}

/* ===================== */

PLGX_appear()
{
XEvent event;

if( x_mapped ) return( 0 );

/* map the window */
XMapWindow( x_disp, x_win );
x_mapped = 1;

/* redraw everything.. */
XFlush( x_disp );
}
/* ====================== */

PLGX_disappear()
{
if( ! x_mapped ) return( 0 );

/* unmap the window.. */
XUnmapWindow( x_disp, x_win );
x_mapped = 0;
XFlush( x_disp );
}
