/* TDHKIT.H
 * Copyright 1998-2002 Stephen C. Grubb  (ploticus.sourceforge.net) .
 * This code is covered under the GNU General Public License (GPL);
 * see the file ./Copyright for details. */

#ifndef TDHKIT
  #define TDHKIT 1

/* general includes and defines.. */
#include <stdio.h> 

char *GL_getok();

/* Data array size */
#define MAXITEMS 80	/* max number of fields per record */
#define DATAMAXLEN 256  /* max length of one field */

/* Variables list */
#define MAXVAR 120	/* max number of variables */
#define VARMAXLEN 256	/* max length of variable contents */

/* Variable name size */
#define NAMEMAXLEN 50	/* max length of variable name */

/* Data type max size */
#define DTMAXLEN 40 	

/* Other maximums */
#define MAXRECORDLEN 3000  /* max length of an input data record */

#define SCRIPTLINELEN 3000 /* max length of a scripter text line, 
			      both before variable evaluation, and after. 
			      4/6/01 expanded from 1024 to accomodate sql result rows */

#define VARSUBLINELEN 1024 /* max length of a text line that will have
				variable substitution applied, before and after. */
#ifdef PLOTICUS
#define MAXPATH 256
#else
#define MAXPATH 128
#endif

#ifndef PATH_SLASH
#ifdef WIN32
#define PATH_SLASH '\\'
#else
#define PATH_SLASH '/'
#endif
#endif

#define MAXSORTFIELDS 20

/* value_subst modes */
#define NORMAL 0
#define FOR_CONDEX 1

/* indent mode */
#define NO_INDENT 1000


/* ======== sinterp state ======== */
#define IFNESTMAX 20
#define INCNESTMAX 5
#define LOOPNESTMAX 20
#define SINTERP_END -1
#define SINTERP_END_BUT_PRINT -2
#define SINTERP_MORE -5
struct sinterpstate {
	int ifnest;			/* current 'if' nest level */
	char condmet[IFNESTMAX];	/* flags for condition met, one per nest level */
	char disp[IFNESTMAX];		/* flags for whether to display based on #if, 1 per nest level */

	int incnest;			/* current 'include' nest level */
	FILE *sfp[INCNESTMAX];		/* array of file pointers, one per nest level */
	int incifnest[INCNESTMAX];	/* save current ifnest to restore to in case of #return */
	int incloopnest[INCNESTMAX];	/* save current loopnest to restore to in case of #return */

	int loopnest;			/* current 'for' or 'loop' nest level */
	long forloc[ LOOPNESTMAX ];	/* seek offset for top of loop, one per nest level */
	int forcount[ LOOPNESTMAX ];	/* loop counter value, one per nest level */
	int forlistpos[ LOOPNESTMAX ];	/* loop, current position in list, one per nest level */
	int loopifnest[ LOOPNESTMAX ];	/* save current ifnest to restore to in case of #break or #continue */
	char listdelim;			/* character to be used as list delimiter */
	int nitems;			/* n data array slots filled */
	int evalvars;			/* 1 = evaluate vars  0 = don't */
	int doingshellresult;		/* >0 = in midst of getting shell command result, 0 = not */
	int doingsqlresult;		/* >0 = in midst of getting sql result, 0 = not */
	int doingdataout;		/* >0 = in midst of a #dataout op, tells next item(-1)   0 = not */
	int doingtext;			/* >0 = in midst of getting text, 0 = not */
	FILE *writefp;			/* fp for use during a #write   */
	int dbc;			/* db connection for sql dump */
	char **memrows;			/* in-memory script rows (optional) */
	int nmemrows;			/* number of in-memory script rows */
	int mrow;			/* current in-memory row */
	} ;

/* ======== select parms ======== */
struct selectparms {
	int nrows;		/* number of rows of results (before DISTINCT & LIMIT).  
				   If 0, rows[] array should not be directly accessed (SQ_rows call is ok) */
	int firstrow;		/* first row (LIMIT) */
	int lastrow;        	/* last row (LIMIT) */
	char *itemlist[MAXITEMS]; /* names of requested items */
	int fldpos[MAXITEMS];   /* positions of requested items */
	int nitems;		/* number of members in above lists */
	FILE *outfp;		/* destination for SELECT INTO results */
	char undflag[MAXITEMS];	/* 0 = remove underscore   1 = leave underscore alone */
	char numflag[MAXITEMS];	/* 0 = alpha   1 = treat as numeric in sort, etc. */
	int distinct;		/* 1 = select distinct  */
	int nfdf;		/* number of fields in raw records */
	int irow;		/* current row */
	int ngbflds;		/* number of 'group by' fields */
	int gbflds[MAXSORTFIELDS]; /* 'group by' fields */
	char aggflag[MAXITEMS];	/* flags for aggregate processing, one per itemlist member */
	int nagg;		/* number of fields to which aggregate processing will be applied */
	int aggcount[MAXITEMS];	/* counts */
	double aggaccum[MAXITEMS]; /* accumulators */
	char *aggbuf;		/* will point to malloc'ed storage for char reps of aggregate values */
	int finalrowcount;	/* row count after all rows gotten */
	} ;

/* ==== macros ==== */
#define stricmp( s, t ) 	strcasecmp( s, t )
#define strnicmp( s, t, n )     strncasecmp( s, t, n )
#define err(a,b,c) 		TDH_err(a,b,c)

/* ==== fseek defines ==== */
#ifndef SEEK_SET
  #define SEEK_SET 0
  #define SEEK_CUR 1
  #define SEEK_END 2
#endif

/* ==== vars ==== */
extern char TDH_scriptdir[];
extern char TDH_tmpdir[];
extern char TDH_dbnull[];
extern int TDH_debugflag;
extern char TDH_utilfieldterm;
extern char TDH_decpt;
extern char TDH_dbquote;
extern char TDH_dbquoteesc;
extern int TDH_htmlqh;
extern char TDH_optionstring[];
extern char *TDH_dat;
extern char *TDH_recid;
extern char TDH_progname[];
extern int TDH_initialized;
#ifndef TDH_NOREC
extern char TDH_fdfpath[];
extern char TDH_bin[];
extern char TDH_tmptbldir[];
#endif


#endif
