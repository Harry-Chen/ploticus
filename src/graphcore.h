#ifndef GRAPHCORE
  #define GRAPHCORE 1
#include <stdio.h>
#include <math.h>


#define YES 1
#define NO 0

/* #define EAY(q)		((Eflip)?Eax(q):Eay(q))
 * #define EAX(q)		((Eflip)?Eay(q):Eax(q))
 */

/* move to x, y absolute */
/* #define Emov( x , y )		Epcode( 'M', (double)x , (double)y, "" ) */

/* line to x, y absolute */
/* #define Elin( x , y )		Epcode( 'L', (double)x , (double)y, "" )*/

/* move to x, y data */
#define Emovu( x , y )		Epcode( 'M', Eax((double) x ) , Eay((double) y ), "" )

/* line to x, y data */
#define Elinu( x , y )		Epcode( 'L', Eax((double) x ) , Eay((double) y ), "" )

/* path to x, y absolute (form a polygon to be shaded later) */
/* #define Epath( x , y )		Epcode( 'P', (double)x , (double)y, "" ) */

/* path to x, y data (form a polygon to be shaded later) */
#define Epathu( x , y )		Epcode( 'P', Eax((double) x ) , Eay((double) y ), "" ) 

/* close path properly for rectangles and polygons */
#define Eclosepath()		Epcode( 'c', 0.0, 0.0, "" )

/* fill previously defined rectangle or polygon with current color */
#define Efill( )		Epcode( 's', 0.0, 0.0, "" )

/* put text (flush-left, centered, or flush-right, relative to current position */
#define Etext( s )		Edotext( s, 'T' )
#define Ecentext( s )		Edotext( s, 'C' )
#define Erightjust( s )		Edotext( s, 'J' )

/* save / load contents of window to / from backing store */
#define Esavewin( )		Epcode( 'b', 0.0, 0.0, "" );
#define Erestorewin( )	Epcode( 'B', 0.0, 0.0, "" );

/* scaling factor for text (used when resizing a window) */
#define Escaletext( x )		Epcode( 'e', x, 0.0, "" )

/* backward comp. define */
#define Egetln( r, b, i )       Egetseg( r, b, i, "\n" )

/* Eprint() is same as Egetclick() */
#define Eprint()		Egetclick()

/* internal: eject page (printers), end-of-plot (screens) */
#define Eshow()			Epcode( 'Z', 0.0, 0.0, "" )

/* internal: wait for user input */
#define Esit()		 Epcode( 'W', 0.0, 0.0, "" )

/* cycle notifier when running asynchronously.. */
#define Easync()	 Epcode( 'w', 0.0, 0.0, "" )

/* display everything that's waiting..(x only) */
#define Eflush()		Epcode( 'U', 0.0, 0.0, "" )

/* define a circle center and radius, for the benefit of object/pick package.. */
/* x, y define the center and r the radius. */ 
/* #define Ecircledef( x, y, r )	Epcode( 'X', x, y, Eftoa( r ) ) */

/* remove window.. */
#define Ewinappear()		Epcode( 'a', 0.0, 0.0, "" )
#define Ewindisappear()		Epcode( 'd', 0.0, 0.0, "" )

/* terminate */
#define Eendoffile()		Epcode( 'Q', 0.0, 0.0, "" )

#define Eerrmsg( n, s, p )	Eerr( n, s, p )
#define Eerr( n, s, p )		TDH_err( n, s, p )

/* 
Graphics notes:
- Origin is in lower left corner of "paper", regardless of orientation of paper.
- Format of i-code will be: "a x y s\n", where a is an op code, x and y
   are coordinates in inches, and s is a variable length string (may be null).
*/

#define E_LINEAR 0
#define E_LOG 1
#define E_LOGPLUS1 2

#define E_WHITE 1
#define E_BLACK 0
extern double Ea(), Eax(), Eay(), Edx(), Edy(), atof(), sqrt(), Elimit();
extern double Econv(), Eu();
extern char *Eftoa(), *GL_getok();

#ifdef LOCALE
 #define stricmp( s, t )         stricoll( s, t )
 #define strnicmp( s, t, n )     strnicoll( s, t, n )
#else
 #define stricmp( s, t )         strcasecmp( s, t )
 #define strnicmp( s, t, n )     strncasecmp( s, t, n )
#endif


/* mouse buttons */
#define E_MOUSE_LEFT 1001
#define E_MOUSE_MIDDLE 1002
#define E_MOUSE_RIGHT 1003

#define E_EXPOSE 1010  /* window has been exposed, unable to restore contents */ 
#define E_RESIZE 1011  /* window has been resized */
#define E_MESSAGE 2000
#define E_COLORMAP_FULL 1100 /* no more colors, drop back to simple defaults */
#define E_MONODISPLAY 1101  /* monochrome display alert */

/* arrow keys */
#define E_LEFT 551
#define E_UP 552
#define E_RIGHT 553
#define E_DOWN 554
#define E_PAGEUP 555
#define E_PAGEDOWN 556

#define E_WAITFOR_WM 200000	/* in certain situations such as after remapping the window or after 
				resizing, a delay seems to be necessary before the window manager 
				responds to subsequent instructions.. This is in microseconds.  */




/* overall settings */
extern char Estandard_font[] ;
extern int Estandard_textsize;
extern double Estandard_lwscale;
extern char Estandard_color[];
extern char Estandard_bkcolor[];

/* window size.. */
extern double EWinx, EWiny;
extern double EWinx_0, EWiny_0;

/* graphics parameters.. */
extern char Edev;
extern int  Epixelsinch;
extern char Estandardfont[];
extern char Ecurfont[];
extern int Ecurtextsize;
extern int Estandardtextsize;
extern double Ecurtextheight;
extern double Ecurtextwidth;
extern int Ecurtextdirection;
extern int Ecurpaper;
extern double Estandardlinewidth;
extern double Ecurlinewidth;
extern int Ecurlinetype;
extern double Ecurpatternfactor;
extern int Ecurpen; 
extern char Ecurcolor[];
extern char Ecurbkcolor[];
extern char Ecurhilitedraw[]; 
extern char Ecurhilitebk[];  

/* event information */
extern int EEvent;
extern double EEventx, EEventy;

/* scaling.. */
extern double EXlo, EXhi, EYlo, EYhi;		/* graphic area bounds, absolute coords */
extern double EDXlo, EDXhi, EDYlo, EDYhi;	/* graphic area bounds, data coords */
extern double EScale_x, EScale_y; 		/* linear scaling factors in x and y */
extern int Escaletype_x;		/* either LINEAR or LOG */
extern int Escaletype_y;		/* either LINEAR or LOG */

/* last moveto and lineto */
extern double Ex1, Ey1, Ex2, Ey2;

extern char Eprogname[];

extern int Eflip;

extern int Eblacklines;

extern long Eflashdelay;

extern FILE *Errfp;

#endif
