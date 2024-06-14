/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */
#ifndef PLHEAD

#define PLHEAD 1
#include "graphcore.h"

#ifdef WIN32
  #define TMPDIR "c:\\temp"
  #define PATH_SLASH '\\'
#else
  #define TMPDIR "/tmp"		/* temp file directory */
  #define PATH_SLASH  '/'	/* slash as used in path name; unix uses '/'  */
#endif

#define CPULIMIT 10		/* max amount of cpu time in seconds */
#define FILESIZELIMIT 5000000	/* max size of created file in bytes */

#define MAXD 200000  		/* maximum total # of data fields (*char) */
#define MAXDATABUF 1000000 	/* max capcity of data text (Databuf) (char) */
#define MAXBIGBUF  100000 	/* size of Bigbuf (char) */
#define MAXDAT 100000 		/* # members in Dat data list (double) */
#define HALFMAXDAT 50000	/* 1/2 of the above */
#define THIRDMAXDAT 33333	/* 1/3 of the above */

#define MAXNCATS 100		/* max # of categories */
#define MAXCATLEN 50		/* max len of category tag */

#define MAXPROCLINES 5000	/* max # of proc lines (current proc plus all #saved procs) */
#define MAXOBJ 40		/* max # of #saved procs in one run */

#define MAXPATH 256		/* pathname max */
#define MAXDSEL 5		/* max # of stacked datasets in memory */
#define MAXCLONES 5		/* max # of #clone that may be used in one proc */



#define dat2d(i,j)	(Dat[((i)*2)+(j)])    /* access Dat as an Nx2 array */
#define dat3d(i,j)      (Dat[((i)*3)+(j)])    /* access Dat as an  NX3  array */



#ifdef LOCALE
 #define stricmp( s, t )        stricoll( s, t )
 #define strnicmp( s, t, n )     strnicoll( s, t, n )
#else
 #define stricmp( s, t )        strcasecmp( s, t )
 #define strnicmp( s, t, n )     strncasecmp( s, t, n )
#endif

#define X 'x'
#define Y 'y'

extern char *da();
extern double fda(), avg();
extern char *getnextattr();

/* legend entry types */
#define LEGEND_COLOR 1
#define LEGEND_LINE 2
#define LEGEND_SYMBOL 4
#define LEGEND_TEXT 8

#define PIVOTYEAR 70

#define PLHUGE 	999999999999999.0
#define NEGHUGE -999999999999999.0

#define YESANS "y"   

#ifndef Eerr
  #define Eerr(a,b,c) TDH_err(a,b,c)
#endif


/* -------------------------------------------------------------- */
/* vars */


extern int Dataobj[];
extern int Nfields[];
extern int Nrecords[];
extern int ND[];
extern int StartD[];
extern int Dsel;
extern int Dsize;

extern char *D[];
extern char Databuf[];
extern char Bigbuf[];
extern double Dat[];
extern char Cats[2][MAXNCATS][MAXCATLEN+1];
extern int Ncats[];
extern int Catcompmethod;

extern char Tmpname[];
extern int Npages;
extern char Device;
extern char Cmdline_outfile[];

extern FILE *Diagfp; 
extern FILE *Errfp; 
extern int Debug;

extern int Bkcolorgiven;
extern int Landscape;
extern int Initialized;

extern int Winsizegiven;
extern double Winw, Winh;
extern int Winx, Winy;


extern int Using_cm; 

extern char Viewer[];

extern char Cmdlineparms[];

extern char Flags[];

extern char Inputfile[];

/* extern int Png; */

extern int Skipout;

extern char Bignumspacer;
extern int Bignumspacer_thres;

extern char *CGIargs;

extern char Prefab_name[];

/* proc line storage.. */
extern char *Procline[];
extern char Objname[][30];
extern int Objstart[], Objlen[], Nobj;
extern int NPL;
extern int Curpl;

extern char Clonelist[];

extern int Clickmap;

extern char *Prefabs_dir;

#endif /* PLHEAD */
