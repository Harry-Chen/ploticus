/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */


#include "graphcore.h"
#include "pl.h"


int Nfields[MAXDSEL];
int Nrecords[MAXDSEL];
int ND[MAXDSEL];
int StartD[MAXDSEL];
int Dsel = 0;
int Dsize; 	/* size of 1st data set */

char *D[ MAXD ]; /* list of pointers to data items */
char Databuf[ MAXDATABUF ]; /* data actually resides here */
char Bigbuf[ MAXBIGBUF ];   /* other general buffer */
double Dat[MAXDAT];	/* data list for intermediate use - typically sorting */
char Cats[2][MAXNCATS][MAXCATLEN+1];	/* categories */
int Ncats[2] = { 0, 0 }; 	/* number of category entries */
int Catcompmethod;

char Tmpname[ MAXPATH ];
int Npages;
char Device; 	/* c p e g or x */
char Cmdline_outfile[ MAXPATH ];

FILE *Diagfp;  /* diagnostics written to this file/stream */
FILE *Errfp;  /* error messages written to this file/stream */
int Debug = 0; 

int Bkcolorgiven = 0; /* needed so that we know whether or not to Eclr in certain places */
int Landscape = 0; /* allows -landscape to be set on command line before Einit() */
int Initialized = 0;  /* needed to tell in various places whether Einit has been called */

int Winsizegiven = 0;
double Winw, Winh;
int Winx, Winy;
 

int Using_cm = 0; /* 1 if absolute units are to be given in cm */

char Viewer[80]; /* view command */

char Cmdlineparms[200]; /* list of parameters specified on command line - these override
				specifications made in proc page even tho they come first */

char Flags[20]; /* Misc flags */

char Inputfile[ MAXPATH ]; /* Pl script file */

int Skipout = 0; /* flag to indicate fatal error and remainder 
		of current script should be skipped */


char Bignumspacer; /* character to use in making large numbers readable (usually comma) */
int Bignumspacer_thres = 4;

char *CGIargs = NULL; /* REQUEST_URI env var; non-null indicates CGI mode */

char Prefab_name[80] = ""; 

char *Prefabs_dir = NULL;

/* proc line storage.. */
char *Procline[MAXPROCLINES];
char Objname[MAXOBJ][30];
int Objstart[MAXOBJ], Objlen[MAXOBJ], Nobj;
int NPL;
int Curpl;

char Clonelist[200];

int Clickmap = 0;

plhead()
{
return( 0 );
}
