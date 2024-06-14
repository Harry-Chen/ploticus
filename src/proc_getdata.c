/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC GETDATA - get some data.  Data may be specified literally, or
	gotten from a file or a command.  */

#include "pl.h"
#include "tdhkit.h"

#define MAXINBUF 3000


static int do_filter();

static int specialmode = 0, showresults = 0;
static char pathname[MAXPATH] = "";
static int nscriptrows = 0, scriptstart;


PLP_getdata( )
{
int i, j;
char buf[MAXINBUF]; /* 2/21/01 */
char attr[40], val[256]; 
char *line, *lineval;
int nt, lvp;

int stat;
int align;
double adjx, adjy;

char datafile[256];
char command[256];
char commentchar[12];
FILE *dfp, *popen();
int gotdata;
int len;
char delim_method[80];
char tok[256];
char str[256];
char selectex[256];
int dblen;
int standardinput;
int rotaterec;
int fieldnameheader;
int first;
int cclen, blen;
int literaldata;
int reqnfields;
char *pfnames;

TDH_errprog( "getdata" );



/* initialize */
strcpy( Databuf, "" );
strcpy( datafile, "" );
if( !specialmode ) strcpy( pathname, "" );
strcpy( command, "" );
strcpy( commentchar, "//" );
strcpy( delim_method, "space" );
gotdata = 0;
if( !specialmode ) showresults = 0;
strcpy( selectex, "" );
standardinput = 0;
rotaterec = 0;
strcpy( Bigbuf, "" );
fieldnameheader = 0;
literaldata = 0;
nscriptrows = 0;
reqnfields = 0;
pfnames = NULL;


/* get attributes.. */
first = 1;
if( !specialmode ) while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];


	if( stricmp( attr, "file" )==0 ) 
#ifdef WIN32
		strcpy( pathname, val ); /* to avoid invoking 'cat' .. */
#else
		strcpy( datafile, val );
#endif
		

	else if( stricmp( attr, "pathname" )==0 ) strcpy( pathname, val );
	else if( stricmp( attr, "command" )==0 ) strcpy( command, lineval );
	else if( stricmp( attr, "commentchar" )==0 ) strcpy( commentchar, val );
	else if( strnicmp( attr, "delim", 5 )==0 ) strcpy( delim_method, val );

	else if( stricmp( attr, "data" )==0 ) {
		getmultiline( "data", lineval, MAXDATABUF, Databuf );
		literaldata = 1;
		}
	else if( stricmp( attr, "filter" )==0 ) {
		if( nt == 2 ) {
			scriptstart = Curpl-1;
			strcpy( Procline[ Curpl-1 ], lineval ); /* remove 'filter:' */
			}
		else scriptstart = Curpl;
		getmultiline( "filter", lineval, MAXBIGBUF, Bigbuf ); /* use this to skip over filter script */
		nscriptrows = (Curpl - scriptstart) - 1;
		for( i = scriptstart; i < scriptstart+nscriptrows; i++ ) {
			for( j = 0; Procline[i][j] != '\0'; j++ ) {
				if( Procline[i][j] == '#' && Procline[i][j+1] == '#' ) {
					Procline[i][j] = ' ';
					break;
					}
				}
			}
		}

	else if( stricmp( attr, "showresults" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) showresults = 1;
		else showresults = 0;
		}
	else if( stricmp( attr, "standardinput" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) standardinput = 1;
		else standardinput = 0;
		}
	else if( stricmp( attr, "rotate" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) rotaterec = 1;
		else rotaterec = 0;
		}
	else if( stricmp( attr, "select" )==0 ) strcpy( selectex, lineval );
	else if( stricmp( attr, "nfields" )==0 ) reqnfields = atoi( val );
	else if( stricmp( attr, "fieldnames" )==0 ) definefieldnames( lineval );
	else if( stricmp( attr, "fieldnamerows" )==0 ) {
		getmultiline( "fieldnamerows", lineval, 4998, &Bigbuf[MAXBIGBUF-5000] );  /* share Bigbuf */
		definefieldnames( &Bigbuf[MAXBIGBUF-5000] );
		}
	else if( stricmp( attr, "pf_fieldnames" )==0 ) pfnames = lineval;

	else if( stricmp( attr, "fieldnameheader" )==0 ) {
		if( strnicmp( val, YESANS, 1 )==0 ) fieldnameheader = 1;
		else fieldnameheader = 0;
		}

	else Eerr( 1, "attribute not recognized", attr );
	}

if( specialmode ) strcpy( delim_method, "tab" );

/* now do the work.. */

if( !GL_slmember( delim_method, "s* c* t* w*" )) {
	Eerr( 5839, "Warning, invalid delim specification, using 'space'", delim_method );
	strcpy( delim_method, "space" );
	}
	
if( standardinput || strcmp( datafile, "-" ) ==0 ) { /* a file of "-" means read from stdin */
	dfp = stdin;
	goto PT1;
	}

if( literaldata ) {
	if( selectex[0] != '\0' ) Eerr( 5792, "Warning, select ignored (it cannot be used with supplied data)", "" );
	if( nscriptrows > 0 ) Eerr( 5792, "Warning, filter ignored (it cannot be used with supplied data)", "" );
	if( fieldnameheader ) {
		int nf;
		i = 0;
		GL_getchunk( buf, Databuf, &i, "\n" );
		nf = definefieldnames( buf );
		if( reqnfields == 0 ) reqnfields = nf;
		strcpy( Databuf, &Databuf[ strlen( buf ) ] ); /* inefficient; user should use fieldnames attr */
		}
	gotdata = 1;
	goto HAVE_DATABUF;
	}

if( datafile[0] != '\0' ) sprintf( command, "cat %s", datafile );
else if( pathname[0] != '\0' ) strcpy( command, "////file" );
	
if( command[0] != '\0' ) {
	if( strcmp( command, "////file" )==0 ) {
		dfp = fopen( pathname, "r" );
		if( dfp == NULL ) {
			Skipout = 1;
			return( Eerr( 401, "Cannot open data file", pathname ) );
			}
		}
	else	{
		dfp = popen( command, "r" );
		if( dfp == NULL ) {
			Skipout = 1;
			return( Eerr( 401, "Cannot open", command ) );
			}
		}

	PT1:
	strcpy( Databuf, "" );
	len = 0;
	gotdata = 1;
	first = 1;
	cclen = strlen( commentchar );
	while( fgets( buf, MAXINBUF-1, dfp ) != NULL ) { 
		nt = sscanf( buf, "%s", tok );
		if( nt < 1 ) continue;
		if( strncmp( tok, commentchar, cclen )==0 ) continue;
		blen = strlen( buf );


		/* #set var = value  - added scg 11/13/00 */
		if( strcmp( tok, "#set" )==0 ) {
			int ix, oldix;
			char varname[40];
			ix = 0;
			GL_getchunk( tok, buf, &ix, " 	" ); /* #set */
			GL_getchunk( varname, buf, &ix, " 	=" ); /* varname */
			oldix = ix;
			GL_getchunk( tok, buf, &ix, " 	" ); /* optional = */
			if( strcmp( tok, "=" ) != 0 ) ix = oldix;
			buf[ blen - 1 ] = '\0'; /* strip off trailing newline - scg 7/27/01 */
			blen--;
			if( buf[ix+1] == '"' ) TDH_setvar( varname, &buf[ix+2] );
			else TDH_setvar( varname, &buf[ix+1] );
			continue;
			}

		/* field name header.. */
		if( first && fieldnameheader ) {
			definefieldnames( buf );
			first = 0;
			continue;
			}


		/* select */
		if( selectex[0] != '\0' ) { 
			stat = do_filter( buf, selectex, delim_method, 1 ); /*doesn't modify buf*/
			if( ! stat ) continue;
			}

		/* optional filter data processing.. */
                if( nscriptrows > 0  ) {
                        do_filter( buf, "", delim_method, 0 ); /* modifies buf */
                        TDH_getvar( "_PRINTDATA", str );
                        if( strcmp( str, "0" )== 0 ) continue; /* user is supressing print of data*/
			blen = strlen( buf );
			if( buf[ blen -1 ] != '\n' ) strcpy( &buf[ blen-1 ], "\n" );
			/* if( showresults ) fprintf( Diagfp, "%s", buf ); */
                        }

		dblen = len;
		len += blen;
		if( len >= MAXDATABUF ) {
			/* this is pretty much fatal.. */
			Skipout = 1;
			return( Eerr( 406, "Sorry, data set too large, capacity exceeded.", "" ) );
			}
		strcpy( &Databuf[dblen], buf );
		}
	}

if( GL_smember( datafile, "- stdin STDIN" )); /* don't want to close STDIN */
else if( pathname[0] != '\0' ) fclose( dfp );
else pclose( dfp );


/* set a clean slate.. */
HAVE_DATABUF:
Dsize = strlen( Databuf ); 
Dsel = 0;
 
ND[0] = 0; ND[1] = 0;
Nrecords[0] = 0; Nrecords[1] = 0;
Nfields[0] = 0; Nfields[1] = 0;
StartD[0] = 0; StartD[1] = 0;


if( !gotdata ) {
	Skipout = 1;
	return( Eerr( 407, "No data, file, or command was specified", "" ) );
	}

if( reqnfields > 0 ) Nfields[Dsel] = reqnfields;

/* parse data and always put in D beginning at position 0.. */
stat = parsedata( Databuf, delim_method, commentchar, D, MAXD, 
	&Nrecords[Dsel], &Nfields[Dsel], &ND[Dsel] );

 

setintvar( "NRECORDS", Nrecords[Dsel] );
setintvar( "NFIELDS", Nfields[Dsel] );

if( stat != 0 ) {
	/* this is pretty much fatal.. */
	Skipout = 1; 
	return( Eerr( stat, "Parsing error on input data.", "" ));
	}

if( ND[Dsel] < 1 ) {
	/* Skipout = 1; */  /* trying this.. scg 11/24/00 */
	setintvar( "NRECORDS", 0 );
	return( Eerr( 408, "No data were read", "" ) );
	}

if( Debug ) fprintf( Diagfp, "// getdata has read %d records with %d fields per record.\n", 
	Nrecords[Dsel], Nfields[Dsel] );

if( showresults ) {
	fprintf( Diagfp, "// Data rows and fields (square brackets added):\n" );
 	for( i = 0; i < Nrecords[Dsel]; i++ ) {
 		for( j = 0; j < Nfields[Dsel]; j++ ) fprintf( Diagfp, "[%s]", da(i,j) );
		fprintf( Diagfp, "\n" );
 		}
	}

/* if filter was used and pfnames given, assign post-filter field names now.. */
if( nscriptrows > 0 && pfnames != NULL ) definefieldnames( pfnames );


if( rotaterec ) {   /* swap nrecords and nfields */
	int tmp;
	if( Nrecords[Dsel] != 1 ) Eerr( 5798, "rotate cannot be done if more than 1 row of data", "" );
	else	{
		tmp = Nrecords[Dsel];
		Nrecords[Dsel] = Nfields[Dsel];
		Nfields[Dsel] = tmp;
		}
	}

return( 0 );
}
/* ======================================== */

static int
do_filter( buf, scriptname, delim, mode )
char *buf;
char *scriptname;
char *delim;
int mode; /* 0 = filter    1 = select */
{
int stat;
char recordid[80]; 
char data[MAXITEMS][DATAMAXLEN+1];
char *df[MAXITEMS];
char str[MAXINBUF], str2[MAXINBUF]; /* size increased from 255  scg 6/27/01 */
int nfields, nrecords, nd;
int i;
char commentchar[12];
struct sinterpstate ss;

strcpy( recordid, "" ); /* not used */
strcpy( commentchar, "//" ); /* not used? */

/* split up buf into fields.. */
strcpy( str, buf );
nfields = 0;
parsedata( str, delim, commentchar, df, MAXITEMS, &nrecords, &nfields, &nd ); 

if( mode == 1 ) { /* condex processing.. */
	strcpy( str2, str );
	stat = plvalue_subst( str2, scriptname, df, FOR_CONDEX );
	if( stat > 1 ) Eerr( 2208, "value_subst error", scriptname );
	stat = TDH_condex( str2, 0 );
	return( stat );
	}

/* for sinterp we need to copy data into array.. */
for( i = 0; i < MAXITEMS; i++ ) strcpy( data[i], "" ); /* null out data array.. added scg 11/15/00 */
for( i = 0; i < nfields; i++ ) strcpy( data[i], df[i] );



stat = TDH_sinterp_openmem( &Procline[ scriptstart ], nscriptrows, &ss ); /* scriptstart & nscriptlines set above.. */
if( stat != 0 ) return( Eerr( stat, "filter script error", "" ) );

/* do filter processing.. */
TDH_setvar( "_PRINTDATA", "" );
strcpy( buf, "" );
while( 1 ) {
	stat = TDH_sinterp( str, &ss, recordid, data );
        if( stat > 255 ) return( Eerr( 169, "filter script error.. quitting..", "" ) );
        else if( stat != SINTERP_MORE ) break;

	strcat( buf, str ); /* strcat ok */
	}
TDH_getvar( "_PRINTDATA", str );
if( strcmp( str, "" )== 0 ) return( 0 ); /* don't print data; return results in buf */

/* if user opted to "print data", assemble fields back into one string.. */
strcpy( buf, "" );
for( i = 0; i < nfields; i++ ) {
	sprintf( str, "\"%s\" ", data[i] );
	strcat( buf, str );
	}
return( 0 );
}


/* ===================================== */
/* GETDATA_SPECIALMODE - allow getdata to read in a data file without accessing 
	attribute list.  Used by proc processdata.  TAB delimitation is always
	used when operating in special mode */
getdata_specialmode( mode, dfn, showr )
int mode;
char *dfn;
int showr;
{
specialmode = mode;
strcpy( pathname, dfn );
showresults = showr;
return( 0 );
}

