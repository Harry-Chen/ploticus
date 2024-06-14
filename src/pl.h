/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */
#ifndef PLHEAD

#define PLHEAD 1

#include "plg.h"

#define PLVERSION "2.20"  	/* see also the Copyright page, and page headers and footers */

/* =========== working limits.. ============ */
#define CPULIMIT 30		/* default max amount of cpu (seconds) - setrlimit() - may be overridden */

#define MAXD 200000  		/* default max total # of data fields - may be overridden using -maxfields */
#define MAXDROWS 10000		/* default max # data rows - may be overridden using -maxrows */
#define MAXDAT 100000 		/* default max # of members in PLV vector - may be overridden */

#define MAXPROCLINES 5000	/* max # of proc lines in current proc plus all #saved 
					procs - may be overridden using -maxproclines */

#define MAXBIGBUF  100000 	/* size of PL_bigbuf (chars) */
#define MAXNCATS 250		/* default max # of categories - may be overriden using proc categories listsize */

#define MAXOBJ 40		/* max # of #saved procs in one run */
#define MAXDS 5			/* max # of stacked datasets in memory - maintain code instances when raising */
#define MAXCLONES 5		/* max # of #clone that may be used in one proc */

#define MAXPATH 256		/* pathname max */

#define NORMAL 0

/* ============ other defines ============= */
#define dat2d(i,j)	(PLV[((i)*2)+(j)])    /* access PLV vector as an Nx2 array */
#define dat3d(i,j)      (PLV[((i)*3)+(j)])    /* access PLV vector as an NX3 array */

#define Nfields		PLD.nfields[ PLD.curds ]
#define Nrecords	PLD.nrecords[ PLD.curds ]

#ifndef Eerr
  #define Eerr(a,b,c) TDH_err(a,b,c)
#endif

#ifdef LOCALE
 #define stricmp( s, t )        stricoll( s, t )
 #define strnicmp( s, t, n )     strnicoll( s, t, n )
#else
 #define stricmp( s, t )        strcasecmp( s, t )
 #define strnicmp( s, t, n )     strncasecmp( s, t, n )
#endif

#define X 'x'
#define Y 'y'
#define PIVOTYEAR 70
#define PLHUGE 	999999999999999.0
#define NEGHUGE -999999999999999.0
#define YESANS "y"   

/* legend entry types */
#define LEGEND_COLOR 1
#define LEGEND_LINE 2
#define LEGEND_SYMBOL 4
#define LEGEND_TEXT 8


/* ============ pathname portability ============ */
#ifdef WIN32
  #define TMPDIR "c:\\temp"
  #define PATH_SLASH '\\'
#else
  #define TMPDIR "/tmp"		/* temp file directory */
  #define PATH_SLASH  '/'	/* slash as used in path name; unix uses '/'  */
#endif


/* ================ functions ======================= */
extern char *PL_da();
extern double PL_fda(), PL_conv(), PL_u();
extern char *PL_getnextattr();


/* ================ structures ====================== */

struct plstate {
	int debug;		/* indicates extra diagnostic output should be generated */
	char device;		/* c p e g or x */
	char *prefabsdir;	/* set from env var PLOTICUS_PREFABS */
	int npages;		/* page count */
	char outfile[MAXPATH];	/* output file as specified by user */
	char mapfile[MAXPATH];	/* clickmap file name */
	char cmdlineparms[300];	/* command line parms that need to override proc page settings */
	int eready;		/* indicates that Einit has been called */
	int usingcm;		/* indicates that units are centimeters */
	int skipout;		/* indicates fatal error and remainder of current script should be skipped */
	int landscape;		/* allows -landscape to be set on command line before Einit() */
	int bkcolorgiven;	/* needed so that we know whether or not to Eclr in certain places */
	int winsizegiven;	/* accomodate specification of window size on command line */
	double winw, winh;	/* accomodate specification of window size on command line */
	int winx, winy;		/* accomodate specification of window location on command line */
	int clickmap;		/* indicates whether we are doing a clickmap or not */
	char viewer[80];	/* viewer program as specified by user, eg ghostscript */
	int bignumspacer;	/* character to use in making large numbers readable (usually comma) */
	int bignumthres;	
	char tmpname[MAXPATH];	/* base name for generating temp file names */
	FILE *diagfp;		/* diagnostic output stream */
	FILE *errfp;		/* error message stream */
	char *cgiargs;		/* CGI args */
	int echolines;		/* echo evaluated script lines to 1 = stdout  2 = stderr */
	};

struct proclines {
	char **procline; 	/* array of pointers to lines of proc code */
	int maxproclines;	/* total malloc'ed size of procline array */
	int nlines;		/* next available cell when filling procline array */
	int curline;		/* next available cell when getting from procline array */

	int nobj;		/* current number of objects being stored */
	char objname[ MAXOBJ ][ 30 ];	/* list of object names for clone/saveas */
	int objstart[ MAXOBJ ];	/* which cell in procline array the object starts in */
	int objlen[ MAXOBJ ];	/* number of lines in procline array the object occupies */
	};

struct pldata {
	char **datarow; 	/* array of pointers to malloc'ed row buffers */
	int currow;		/* current number of members in datarow array */
	int maxrows;		/* total malloc'ed size of datarow array */

	char **df;		/* array of field pointers */
	int curdf;		/* next available field pointer in df array */
	int maxdf;		/* total malloc'ed size of df array; */

	/* data sets are managed as a stack of up to MAXDS elements */
	int curds;		/* identifies the current dataset (or stack size).  First is 0 */
	int dsfirstdf[MAXDS];	/* where a dataset begins in the df array */
	int dsfirstrow[MAXDS];	/* where a dataset begins in the datarow array.. if data set in procline array this is -1 */
	int nrecords[ MAXDS ];	/* number of records in a dataset */
	int nfields[ MAXDS ];	/* number of fields in a dataset */
	};
	


/* ================ global vars ================================= */

extern struct plstate PLS;
extern struct pldata PLD;
extern struct proclines PLL;
extern double *PLV;
extern int PLVsize, PLVhalfsize, PLVthirdsize;
extern char PL_bigbuf[];


/* ================ function redefines =========================== */
/* internally, functions usually don't use the PL_ prefix.. */
#define getmultiline(parmname,firstline,maxlen,result)	PL_getmultiline(parmname,firstline,maxlen,result)
#define getnextattr(flag,attr,val,linevalpos,nt) 	PL_getnextattr(flag,attr,val,linevalpos,nt)

#define da( r, c )					PL_da( r, c )
#define fda( r, a, ax )					PL_fda( r, a, ax )
#define num( s, result ) 				PL_num( s, result )
#define getcoords( p, v, x, y )				PL_getcoords( p, v, x, y )
#define getbox( p, v, x1, y1, x2, y2 )			PL_getbox( p, v, x1, y1, x2, y2 )
#define getrange( v, l, h, ax, dl, dh )			PL_getrange( v, l, h, ax, dl, dh )
#define file_to_buf( f, m, r, b )			PL_file_to_buf( f, m, r, b )
#define setfloatvar( v, f )				PL_setfloatvar( v, f )
#define setintvar( v, n )				PL_setintvar( v, n )
#define setcharvar( v, s )				PL_setcharvar( v, s )
#define conv_msg( r, c, a )				PL_conv_msg( r, c, a )
#define oth_mst( m )					PL_oth_msg( m )
#define suppress_convmsg( m )				PL_suppress_convmsg( m )
#define zero_convmsgcount()				PL_zero_convmsgcount()
#define report_convmsgcount()				PL_report_convmsgcount()
#define scalebeenset()					PL_scalebeenset()
#define catitem( s, d, avail )				PL_catitem( s, d, avail )
#define defaultinc( min, max, inc )			PL_defaultinc( min, max, inc )
#define rewritenums( num )				PL_rewritenums( num )
#define convertnl( s )					PL_convertnl( s )
#define measuretext( txt, nlines, maxlen )		PL_measuretext( txt, nlines, maxlen )

#define clickmap_init( nbytes, debug )			PL_clickmap_init( nbytes, debug )
#define clickmap_inprogress()				PL_clickmap_inprogress()
#define clickmap_debug()				PL_clickmap_debug()
#define clickmap_setdefaulturl( url )			PL_clickmap_setdefaulturl( url )
#define clickmap_seturlt( url )				PL_clickmap_seturlt( url )
#define clickmap_entry( t, u, p, x1, y1, x2, y2, tp, cm, tit )  PL_clickmap_entry( t, u, p, x1, y1, x2, y2, tp, cm, tit )
#define clickmap_out( tx, ty )				PL_clickmap_out( tx, ty )
#define clickmap_show( dev )				PL_clickmap_show( dev )

#define textdet( p, s, a, adjx, adjy, szh, sth )	PL_textdet( p, s, a, adjx, adjy, szh, sth )
#define linedet( p, s, d )				PL_linedet( p, s, d )
#define symdet( p, s, sc, r )				PL_symdet( p, s, sc, r )

#define devavail( dev )					PL_devavail( dev )
#define devnamemap( s, t, mode )			PL_devnamemap( s, t, mode )
#define makeoutfilename( s, o, d, p )			PL_makeoutfilename( s, o, d, p )

#define definefieldnames( list )			PL_definefieldnames( list )
#define fref( name )					PL_fref( name )
#define getfname( n, result )				PL_getfname( n, result )
#define fref_error()					PL_fref_error()
#define fref_showerr( mode )				PL_fref_showerr( mode )

#define do_select( selectex, row, result )		PL_do_select( selectex, row, result )
#define do_subst( out, in, row, mode )			PL_do_subst( out, in, row, mode )

#define	Esetunits( axis, s )				PL_setunits( axis, s )
#define Egetunits( axis, s )				PL_getunits( axis, s )
#define Egetunitsubtype( axis, result )			PL_getunitsubtype( axis, result )
#define Esetscale( axis, alo, ahi, scalelo, scalehi )	PL_setscale( axis, alo, ahi, scalelo, scalehi )
#define Econv( axis, s )				PL_conv( axis, s )
#define Econv_error()					PL_conv_error()
#define Euprint( result, axis, f, format )		PL_uprint( result, axis, f, format )
#define Eposex( val, axis, result )			PL_posex( val, axis, result )
#define Elenex( val, axis, result )			PL_lenex( val, axis, result )
#define Eevalbound( val, axis, result )			PL_evalbound( val, axis, result )
#define Esetdatesub( tok, desc )			PL_setdatesub( tok, desc )
#define Esetcatslide( axis, amount )			PL_setcatslide( axis, amount )
#define Es_inr( axis, val )				PL_s_inr( axis, val )
#define Ef_inr( axis, val )				PL_f_inr( axis, val )

#endif /* PLHEAD */
