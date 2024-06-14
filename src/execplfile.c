/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

#include "pl.h"
#include "tdhkit.h"
#define SCRIPTLINELEN 3000
#define NONDRAWINGPROCS "page print originaldata tabulate getdata defineunits\
 rangebar catslide processdata datesettings endproc"


/* ================================================================= */
/* EXEC_PLFILE - execute one ploticus control file.
 *  ED* variables should be initialized before call.
 *  Returns 0 if ok, or a non-zero error code 
 */


exec_plfile( specfile )
char *specfile;
{
int i, j;
char buf[ SCRIPTLINELEN ];
char buf2[256];
int stat;
struct sinterpstate ss;
int first;
char procname[40];
FILE *fp;
int nt;
int lastbs;	        /* indicates that previous line ended with a backslash, indicating continuation.. */
char saveas_name[40];
char last_proctok[20]; /* either #proc or #procdef */
char clone_name[40];
int nplhold;
int buflen;


/* open spec file.. */
stat = TDH_sinterp_open( specfile, &ss );
if( stat != 0 ) {
	Eerr( 22, "Cannot open scriptfile; error in command line args", specfile );
	vermessage(); exit( 1 );
	}
if( Debug ) fprintf( Diagfp, "script file opened.. " );

Npages = 0;


first = 1;
fp = NULL;
lastbs = 0;
Dsize = 0;
Dsel = 0;
strcpy( saveas_name, "" );
strcpy( last_proctok, "" );
strcpy( procname, "" );
strcpy( Clonelist, "" );


/* data array initializations.. */
ND[0] = ND[1] = 0;
Nrecords[0] = Nrecords[1] = 0;
Nfields[0] = Nfields[1] = 0;
StartD[0] = StartD[1] = 0;

/***** proc line initializations.. */
Nobj = 0;
NPL = nplhold = 0;


/* read through script file using sinterp script interpreter.. */
while( 1 ) {
	stat = TDH_sinterp( buf, &ss, "", NULL );
	if( stat == 1226 ) {
        	err( stat, "required variable(s) missing", buf );
        	break;
        	}

        if( stat > 255 ) return( Eerr( 23, "Fatal script processing error.", "" ) );

	else if( Skipout ) { /* error that is fatal for this script */
		Skipout = 0;
		return( 1 );
		}

        else if( stat != SINTERP_MORE ) break;


	strcpy( buf2, "" );
	nt = sscanf( buf, "%s", buf2 );

	/* rewrite #endproc  ->  #proc endproc  */
	if( stricmp( buf2, "#endproc" )==0 ) {
		strcpy( buf, "#proc endproc" );
		strcpy( buf2, "#proc" );
		}

	if( ss.doingshellresult == 0 && ss.doingsqlresult == 0 && 
	    buf2[0] == '#' && buf2[1] != '#' && !GL_slmember( buf2, "#proc* #clone* #saveas* #intrailer*" ) ) {
            	while( 1 ) {
			stat = TDH_secondaryops( buf, &ss, "", NULL ); /* may call sinterp() to get lines*/
			if( stat > 255 ) return( Eerr( 24, "Fatal script processing error.", "" ) );
        		else if( stat != SINTERP_MORE ) break;
			if( ss.writefp != NULL ) fprintf( ss.writefp, "%s", buf );
			}
		if( stat == SINTERP_END_BUT_PRINT ) {
			if( ss.writefp != NULL ) fprintf( ss.writefp, "%s", buf );
			}
		continue;
		}

	if( ss.writefp != NULL ) {
		fprintf( ss.writefp, "%s", buf ); 
		continue;
		}


	/****** When we encounter #proc or #procdef for the first time, initialize some things.
         ****** When we encounter #proc or #procdef subsequent times, execute the proc that has
	 ****** just been read. */
	if( strnicmp( buf2, "#proc", 5 )==0 && !lastbs ) {  /* #proc or #procdef break */

		/* if #saveas was used, record the name.. */
		if( saveas_name[0] != '\0' )  strcpy( Objname[ Nobj ], saveas_name );

		/* get line count.. */
		Objlen[ Nobj ] = NPL - nplhold;

		/* if #proc (as opposed to #procdef), execute proc.. */
		if( !first && stricmp( last_proctok, "#proc" )==0 ) {  

			/* proc page can do the Einit, or we can do it here.. */
			if( stricmp( procname, "page" )==0 ) Initialized = 1;
			if( !Initialized && !GL_slmember( procname, NONDRAWINGPROCS )) {
				Einit( Inputfile, Device );
				Epaper( Landscape );
				Initialized = 1;

				/* we are ready to draw.. safe to say page 1.. scg 11/28/00 */
				if( stricmp( procname, "page" )!=0 ) Npages = 1;
				if( Bkcolorgiven ) {
					/* EPS color=transparent - best to do nothing.. scg 1/10/00*/
					if( Device == 'e' && strcmp( Ecurbkcolor, "transparent" )==0 ) ;
					else Eclr(); 
					}
				}


			/* execute the appropriate plotting procedure... */
			proc_call( procname );

			if( Initialized ) Eflush();  /* get output onto screen.. */
			}

		if( sscanf( buf, "%*s %s", procname ) < 1 ) {
			return( Eerr( 24, "#proc must be followed by a procedure name", "" ) );
			}

		/* if no #saveas.., discard proc lines now..*/
		if( saveas_name[0] == '\0' ) {
			for( i = nplhold; i < NPL; i++ ) free( Procline[i] ); 
		 	NPL = nplhold; 
			}
		else Nobj++;

		strcpy( last_proctok, buf2 );


		if( procname[ strlen( procname ) - 1 ] == ':' ) 
			procname[ strlen( procname ) - 1 ] = '\0';


		first = 0;

		if( stricmp( procname, "trailer" )==0 ) goto ENDOFSCRIPT;

		/* initialize to capture next proc */
		strcpy( saveas_name, "" );
		strcpy( Clonelist, "" );
		strcpy( Objname[ Nobj ], "" );
		Objstart[ Nobj ] = NPL;
		nplhold = NPL;
		}


	/****** for all other lines, get them, and add them to proc line list, looking for
	 ****** special cases such as #clone, #saveas, and #intrailer..  */

	else 	{
		if( procname[0] == '\0' ) continue;

		else 	{
			/* add lines to proc line list.. */
			/* also look for exceptions such as "#clone" */

			if( !lastbs && strnicmp( buf2, "#clone", 6 )==0 ) {
				strcpy( clone_name, "" );
				sscanf( buf, "%*s %s", clone_name );
				if( clone_name[0] == '\0' ) {
					Eerr( 27, "#clone object name is missing", procname );
					continue;
					}
				strcat( Clonelist, clone_name );
				strcat( Clonelist, " " );
				}

			else if( !lastbs && strnicmp( buf2, "#saveas", 7 )==0 ) {
				sscanf( buf, "%*s %s", saveas_name );
				}

			else if( !lastbs && strnicmp( buf2, "#intrailer", 10 )==0 ) {
				long loc;
				int seekstate;
				char tok[80];
				/* remember current location in control file, then
				   scan forward until we reach #proc trailer, copy data to
				   tmp file, then seek back to where we left off. 
				   Does not work from within an #include.  */
				loc = ftell( ss.sfp[0] );
				seekstate = 0;
				while( fgets( buf, SCRIPTLINELEN-1, ss.sfp[0] ) != NULL ) {
					sscanf( buf, "%s %s", buf2, tok );
					if( stricmp( buf2, "#proc" ) ==0 && stricmp( tok, "trailer" )==0 ) break;
					}
				while( fgets( buf, SCRIPTLINELEN-1, ss.sfp[0] ) != NULL ) {
					sscanf( buf, "%s %s", buf2, tok );
					if( strcmp( buf2, "//" )==0 ) continue;

					/* add to list */
					buflen = strlen( buf );
					buf[ buflen-1 ] = '\0'; buflen--; /* remove trailing \n */
					Procline[ NPL ] = (char *) malloc( buflen+1 );
					strcpy( Procline[ NPL ], buf );
					if( NPL >= MAXPROCLINES-1 ) 
						return( err( 28, "sorry, proc line list capacity exceeded", "" ));
					NPL++;
					}
				fseek( ss.sfp[0], loc, 0 /*SEEK_SET BSD*/ );  /* now go back.. */
				}

			else 	{
				buflen = strlen( buf );
				buf[ buflen-1 ] = '\0'; buflen--; /* remove trailing \n */
				Procline[ NPL ] = (char *) malloc(  buflen+1 );
				strcpy( Procline[ NPL ], buf );
				if( NPL >= MAXPROCLINES-1 ) 
					return( err( 28, "sorry, proc line list capacity exceeded", "" ));
				NPL++;
				if( buf[ buflen - 2 ] == '\\' ) lastbs = 1;
				else lastbs = 0;
				}
			}
		}
	}

/* last case.. */
if( procname[0] != '\0' ) {
	/* get line count.. */
	Objlen[ Nobj ] = NPL - nplhold;

	/* no need to code for proc page here; it can never be the last proc */
	if( !Initialized && !GL_slmember( procname, NONDRAWINGPROCS ) ) {
		Einit( Inputfile, Device );
		Epaper( Landscape );
		Initialized = 1;
		}
	/***** args */
	proc_call( procname );
	if( Initialized ) Eflush();  /* get output onto screen.. */
	}


ENDOFSCRIPT:
for( i = 0; i < NPL; i++ ) free( Procline[i] );
if( ss.sfp[0] != NULL ) fclose( ss.sfp[0] );

return( 0 );
}

/* ========================= */
/* PROC_CALL - call the appropriate proc routine */

proc_call( procname )
char *procname;
{
int stat;
int n;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

if( Debug ) { fprintf( Diagfp, "Executing %s\n", procname ); fflush( Diagfp ); }
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	}

if( stricmp( procname, "areadef" )==0 ) {
	stat = PLP_areadef();
	if( stat != 0 ) {
		Eerr( 10, "(Fatal) areadef failed due to problems with area specification or xrange/yrange.", "" );
		vermessage(); exit(1);
		}
	}
else if( stricmp( procname, "page" )==0 ) PLP_page();
else if( stricmp( procname, "xaxis" )==0 ) PLP_axis( 'x', 0 );
else if( stricmp( procname, "yaxis" )==0 ) PLP_axis( 'y', 0 );
else if( stricmp( procname, "getdata" )==0 ) PLP_getdata();
else if( stricmp( procname, "lineplot" )==0 ) PLP_lineplot();
else if( stricmp( procname, "bars" )==0 ) PLP_bars();
else if( stricmp( procname, "rangebar" )==0 ) PLP_rangebar();
else if( stricmp( procname, "scatterplot" )==0 ) PLP_scatterplot();
else if( stricmp( procname, "annotate" )==0 ) PLP_annotate();
else if( stricmp( procname, "legend" )==0 ) PLP_legend();
else if( stricmp( procname, "tabulate" )==0 ) PLP_tabulate();
else if( stricmp( procname, "curvefit" )==0 ) PLP_curvefit();
else if( stricmp( procname, "rangesweep" )==0 ) PLP_rangesweep();
else if( stricmp( procname, "pie" )==0 ) PLP_pie();
else if( stricmp( procname, "drawcommands" )==0 ) PLP_drawcommands();
else if( stricmp( procname, "line" )==0 ) PLP_line();
else if( stricmp( procname, "rect" )==0 || stricmp( procname, "bevelrect" )==0 ) PLP_rect();
else if( stricmp( procname, "transform" )==0 || stricmp( procname, "processdata" )==0 ) PLP_processdata();
else if( stricmp( procname, "print" )==0 ) PLP_print();
else if( stricmp( procname, "legendentry" )==0 ) PLP_legendentry();
else if( stricmp( procname, "defineunits" )==0 ) PLP_defineunits();
else if( stricmp( procname, "catslide" )==0 ) PLP_catslide();
else if( stricmp( procname, "settings" )==0 || stricmp( procname, "datesettings" )==0 ) PLP_settings();
else if( stricmp( procname, "originaldata" )==0 ) {
	Dsel = 0;
	setintvar( "NRECORDS", Nrecords[Dsel] );
	setintvar( "NFIELDS", Nfields[Dsel] );
	}
else if( stricmp( procname, "symbol" )==0 ) PLP_symbol();
else if( stricmp( procname, "breakaxis" )==0 ) PLP_breakaxis();
else if( stricmp( procname, "import" )==0 ) PLP_import();
else if( stricmp( procname, "trailer" )==0 ) ; /* do nothing */
else if( stricmp( procname, "endproc" )==0 ) ; /* do nothing */
else return( Eerr( 101, "procedure name unrecognized", procname ) );

TDH_errprog( "pl" );

if( Initialized ) Eflush();
n = report_convmsgcount();
if( Debug && n > 0 ) {
	fprintf( Diagfp, "note: pl proc %s encountered %d unplottable data values\n", procname, n );
	zero_convmsgcount();
	}
return( 0 );
}

/* ================================================================= */
/* GETNEXTATTR - serve up the next proc line, or NULL if no more */

char *
getnextattr( flag, attr, val, linevalpos, nt )
int flag; /* 1 = first call for proc;  2 = getting multiline (don't get toks); any other value = nothing */
char *attr, *val;
int *linevalpos, *nt;
{
static int cloneix, stop, state;
static char *line;
int i, j, k, ix, alen;
char clone_name[40];

/* states:  0 = init   1 = getting clone  2 = getting proc  3 = done */

if( flag == 1 ) {
	state = 0;
	cloneix = 0;
	}

if( state == 3 ) {
	line = NULL;
	return( line );
	}

if( state == 0 ) {  
	RETRY:
	strcpy( clone_name, GL_getok( Clonelist, &cloneix ));
	if( clone_name[0] != '\0' ) {
		/* look up obj in list, starting with latest entry and working backward.. */
		for( j = Nobj-1; j >= 0; j-- ) if( strcmp( Objname[j], clone_name )==0 ) break;
		if( j < 0 ) {
			Eerr( 2506, "#clone object not found", clone_name );
			goto RETRY;
			}
		Curpl = Objstart[j];
		stop = Objstart[j] + Objlen[j];
		state = 1;
		}
	else 	{
		Curpl = Objstart[ Nobj ];
		stop = NPL;
		state = 2;
		}
	}

if( state == 1 || state == 2 ) {
	RETRY2:
	if( Curpl >= NPL ) return( NULL );
	if( flag == 2 ) { /* multirow */
		for( i = 0; Procline[Curpl][i] != '\0'; i++ ) if( !isspace( Procline[Curpl][i] ) ) break;
		line = &Procline[Curpl][i];
		}
	else	{  /* single row attr: val */
		line = Procline[ Curpl ];
		ix = 0;
		strcpy( attr, GL_getok( line, &ix ) );
		if( attr[0] == '\0' ) {   /* blank line.. skip */
			Curpl++;
			if( Curpl >= stop && state == 1 ) { 
				state = 0; 
				goto RETRY; 
				}
			else if( Curpl >= stop && state == 2 ) {
				state = 3; 
				return( NULL ); 
				}
			else goto RETRY2;
			}
		alen = strlen( attr );
		if( attr[ alen-1 ] == ':' ) attr[ alen-1 ] = '\0';
		if( attr[0] != '\0' ) while( line[ix] == ' ' || line[ix] == '\t' ) ix++; /* skip over ws */
		*linevalpos = ix;
		strcpy( val, GL_getok( line, &ix ) );
		if( val[0] != '\0' ) while( line[ix] == ' ' || line[ix] == '\t' ) ix++; /* skip over ws */
		if( attr[0] == '\0' ) *nt = 0;
		else if( val[0] == '\0' ) *nt = 1;
		else *nt = 2;
		}

	Curpl++;
	if( Curpl >= stop ) {
		if( state == 1 ) state = 0;
		else state = 3;
		}
	return( line );
	}
return( NULL );
}

/* ================================================================= */
/* GETMULTILINE - get a multi-line text item from script file.
   End is marked by an empty line.
   Leading white space is removed from all lines.
   Example
	title:  Number of days difference 
	 	\
	        Subgroup: 1B

	frame: yes
*/
   
getmultiline( parmname, firstline, maxlen, result )
char *parmname, *firstline;
int maxlen;
char *result;
{
char *line, buf[256];
int nt, i, len, foo, rlen, lonebs;

rlen = 0;

/* strip leading white space from first line.. */
for( i = 0, len = strlen( firstline ); i < len; i++ ) 
	if( firstline[i] != ' ' && firstline[i] != '\t' ) break;
if( i < len ) {
	sprintf( result, "%s\n", &firstline[i] );
	rlen = ( len - i ) + 1;
	}

/* now get remaining lines.. */
while( 1 ) {
	line = getnextattr( 2, NULL, NULL, &foo, &foo );

	if( line == NULL ) break;
	if( line[0] == '\0' ) break;  /* first blank line terminates */

	len = strlen( line );
	if( rlen + len > maxlen ) return( Eerr( 60, "Warning, multiline text truncated", parmname ) );
	
	/* detect lines having just a backslash.. */
	lonebs = 0;
	if( line[0] == '\\' ) {
		nt = sscanf( &line[1], "%s", buf );
		if( nt < 1 ) lonebs = 1;
		}

	if( lonebs ) {
		strcpy( &result[ rlen ], "\n" ); rlen += 1;
		}
	else	{
		sprintf( &result[ rlen ], "%s\n", line ); rlen += (len+1);
		}
	}
return( 0 );
}
