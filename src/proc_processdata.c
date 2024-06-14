/* ploticus data display engine.  Software, documentation, and examples.  
 * Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
 * Covered by GPL; see the file ./Copyright for details. */

/* PROC PROCESSDATA - perform various processing to the data set */

#include "pl.h"

#define MAXFLD 80
#define MAXBREAKFLDS 5

static int rejfld[MAXFLD];
static int nrejfld;
static int kpfld[MAXFLD];
static int nkpfld;

static int stackmode = 0;
static int nr, nf;
static int curd, davail;
static FILE *outfp;
static int breakcurrow = 0;
static int eofcount = 0;

static int init(), out(), eor(), dofld();



PLP_processdata( )
{
int i;
char attr[40], val[256];
char *line, *lineval;
int nt, lvp;
int first;

char buf[256];
int stat;
int align;
double adjx, adjy;
char tmpfile[256];
char procfile[256];
char action[40];
int j;
int showresults;
int nfld;
int fld[MAXFLD];
double accum[MAXFLD];
char rformat[40];
int keepall;
char outfmt[40];
int k;
char outbuf[256];
int ds;
char selectex[256];
char tok[256];
int dispformatnum;
int resetbns;
char breakbuf[ MAXBREAKFLDS ][52];
double atof();
int outfile;

TDH_errprog( "processdata" );




/* initialize */
showresults = 0;
nfld = 0;
strcpy( rformat, "%g" );
keepall = 0;
nrejfld = 0;
nkpfld = 0;
strcpy( action, "" );
stackmode = 0;
strcpy( selectex, "" );
dispformatnum = 0;
strcpy( tmpfile, "" );
outfile = 0;

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, val, &lvp, &nt );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( stricmp( attr, "action" )==0 ) strcpy( action, val );
	else if( stricmp( attr, "fields" )==0 ) {
		int ix; ix = 0; i = 0; 
		while( 1 ) {
			strcpy( tok, GL_getok( lineval, &ix ) );
			if( tok[0] == '\0' ) break;
			fld[i] = fref( tok ) -1;
			i++;
			if( i >= MAXFLD ) break;
			}
		nfld = i;
		}
	else if( stricmp( attr, "keepfields" )==0 ) {
		int ix; ix = 0; i = 0; 
		while( 1 ) {
			strcpy( tok, GL_getok( lineval, &ix ) );
			if( tok[0] == '\0' ) break;
			kpfld[i] = fref( tok ) -1;
			i++;
			if( i >= MAXFLD ) break;
			}
		nkpfld = i;
		}
	else if( stricmp( attr, "rejectfields" )==0 ) {
		int ix; ix = 0; i = 0; 
		while( 1 ) {
			strcpy( tok, GL_getok( lineval, &ix ) );
			if( tok == '\0' ) break;
			rejfld[i] = fref( tok ) -1;
			i++;
			if( i >= MAXFLD ) break;
			}
		nrejfld = i;
		}

	else if( stricmp( attr, "resultformat" )==0 ) strcpy( rformat, lineval );
	else if( stricmp( attr, "select" )==0 ) strcpy( selectex, lineval );

	else if( stricmp( attr, "showresults" )==0 ) {
		if( strnicmp( val, YESANS, 1 ) ==0 ) showresults = 1;
		else showresults = 0;
		}
	else if( stricmp( attr, "keepall" )==0 ) {
		if( strnicmp( val, YESANS, 1 ) ==0 ) keepall = 1;
		else keepall = 0;
		}
	else if( stricmp( attr, "stack" )==0 ) {
		if( strnicmp( val, YESANS, 1 ) ==0 ) stackmode = 1;
		else stackmode = 0;
		}
	else if( stricmp( attr, "fieldnames" )==0 ) {
		definefieldnames( lineval );
		}
	else if( stricmp( attr, "outfile" )==0 ) {
		strcpy( tmpfile, val );
		outfile = 1;
		}


	else Eerr( 1, "attribute not recognized", attr );
	}


/* overrides and degenerate cases */
/* -------------------------- */
if( stricmp( action, "breaks" ) == 0 ) {
	setcharvar( "BREAKFIELD1", "" ); 
	setintvar( "NRECORDS", 0 ); 
	if( nfld < 1 ) 
		return( Eerr( 2879, "'breaks' action requires one or more fields", "" ));
	Dsel = 0;
	stackmode = 1;
	}

if( stricmp( action, "breakreset" )!= 0 ) {
	if( Nrecords[Dsel] < 1 ) return( Eerr( 17, "No data have been read yet w/ proc getdata", "" ) );
	}
if( GL_slmember( action, "per* tot* acc*") && nfld < 1 ) 
	return( Eerr( 2870, "one or more 'fields' must be specified with percents, totals, or accumulate", "" ) );

if( stricmp( action, "count" )==0 && ( nfld < 1 || nfld > 2 ) )
	return( Eerr( 2874, "'count' action requires one or two fields", "" ));

if( stricmp( action, "select" )== 0 && selectex[0] == '\0' ) 
	return( Eerr( 3879, "if action is 'select' a selection expression must be given", "" ));

if( stricmp( action, "select" )!= 0 && selectex[0] != '\0' )
	return( Eerr( 3880, "a selection expression may not be given unless action is select", "" ));


/* now do the work.. */
/* -------------------------- */
if( rformat[0] == 'n' ) {
	dispformatnum = 1;
	strcpy( rformat, &rformat[1] );
	}
sprintf( outfmt, "\"%s\" ", rformat );
for( i = 0; i < MAXFLD; i++ ) accum[i] = 0.0;

if( !stackmode ) {
	if( !outfile ) sprintf( tmpfile, "%s_S", Tmpname );
	outfp = fopen( tmpfile, "w" ); /* temp file, unlinked below */
	if( outfp == NULL ) return( Eerr( 2987, "Cannot open tmp file", tmpfile ) );
	}



ds = Dsel;
if( stackmode ) {
	if( ds+1 >= MAXDSEL ) 
		return( Eerr( 2784, "Data stack is full, cannot perform processing", "" ) );
 
	davail = Dsize;
	ND[ds+1] = 0;
	StartD[ds+1] = ND[ds];
	curd = StartD[ds+1];
	}
init();


/* break processing - calling script can detect when end is reached by looking at NRECORDS or BREAKFIELD1 */
if( stricmp( action, "breaks" )==0 ) { 
	int breakfound;
	char breakvarname[20];

	/* start at current row */
	if( breakcurrow >= Nrecords[ds] ) {
		eofcount++;
		if( eofcount > 10 ) {
			Eerr( 4729, "unterminated loop (processdata action=breaks)", "" );
			exit(1);
			}
		goto SKIPBREAK;
		}
	i = breakcurrow;

	/* save initial contents of break fields.. */
	/* also set DMS vars BREAKFIELD1 .. N */
	for( j = 0; j < nfld; j++ ) {
		strncpy( breakbuf[j], da( i, fld[j] ), 50 );
		breakbuf[j][50] = '\0';
		sprintf( breakvarname, "BREAKFIELD%d", j+1 );
		setcharvar( breakvarname, breakbuf[j] );
		}
	for( ; i < Nrecords[ds]; i++ ) {
		/* compare against contents of break fields.. when any differences found, break.. */
		breakfound = 0;
		for( j = 0; j < nfld; j++ ) {
			if( strncmp( breakbuf[j], da( i, fld[j] ), 50 ) != 0 ) {
				breakfound = 1;
				break;
				}
			}

		if( breakfound ) break; 

		else 	{
			for( j = 0; j < Nfields[ds]; j++ ) {  /* copy fields to new data space.. */
				if( dofld( j )) out( da(i,j) );
				}
			eor();
			}
		}
	breakcurrow = i;
	SKIPBREAK: ;
	}

/* breakreset */
else if( stricmp( action, "breakreset" )==0 ) {
	breakcurrow = 0;
	eofcount = 0;
	return( 0 );
	}


/* reverse */
else if( strnicmp( action, "rev", 3 )==0 ) { 
	for( i = Nrecords[ds]-1; i >= 0; i-- ) {
		for( j = 0; j < Nfields[ds]; j++ ) {
			if( dofld( j )) {
				out( da(i,j) );
				/* fprintf( fp, "\"%s\" ", da(i,j) ); */
				}
			}
		eor();
		/* fprintf( fp, "\n" ); */
		}
	}


/* rotate */
else if( strnicmp( action, "rot", 3 )==0 ) {     
	for( j = 0; j < Nfields[ds]; j++ ) {
		if( dofld( j )) {
			for( i = 0; i < Nrecords[ds]; i++ ) {
				/* fprintf( fp, "\"%s\" ", da(i,j) ); */
				out( da( i,j) );
				}
			/* fprintf( fp, "\n" ); */
			eor();
			}
		}
	}


/* percents */
else if( strnicmp( action, "per", 3 )==0 ) {    
	/* find all totals.. */
	for( i = 0; i < nfld; i++ ) {
		for( j = 0; j < Nrecords[ds]; j++ ) accum[i] += atof( da( j, fld[i] ) );
		}

	for( i = 0; i < Nrecords[ds]; i++ ) {
		for( j = 0; j < Nfields[ds]; j++ ) {
			if( dofld( j )) {
				/* see if j is a 'hot' field */
				for( k = 0; k < nfld; k++ ) if( j == fld[k] ) break;
				if( k != nfld ) {
				    if( keepall ) {
					/* fprintf( fp, "\"%s\" ", da( i, j ) ); */
					out( da( i, j ) );
					}	
				    sprintf( outbuf, outfmt, (atof(da( i, j )) / accum[k]) * 100.0 );
				    out( outbuf );
				    }
				else 	{
					out( da( i, j ) );
					/* fprintf( fp, "\"%s\" ", da( i, j ) ); */
					}
				}
			}
		/* fprintf( fp, "\n" ); */
		eor();
		}
	}


/* accumulate */
else if( strnicmp( action, "acc", 3 )==0 ) {     
	for( i = 0; i < Nrecords[ds]; i++ ) {
		for( j = 0; j < Nfields[ds]; j++ ) {
			/* see if j is a 'hot' field */
			if( dofld( j )) {
				for( k = 0; k < nfld; k++ ) if( j == fld[k] ) break;
				if( k != nfld ) {
				    accum[k] += atof( da( i, j ) );
				    if( keepall ) {
					/* fprintf( fp, "\"%s\" ", da( i, j ) ); */
					out( da( i, j ) );
					}
				    /* fprintf( fp, outfmt, accum[k] ); */
				    out( da( i, j ) );
				    }
				else 	{
					out( da( i, j ) );
					/* fprintf( fp, "\"%s\" ", da( i, j ) ); */
					}
				}
			}
		/* fprintf( fp, "\n" ); */
		eor();
		}
	}

/* count - may be used with one or two fields.
   If one field, result has these fields: 1) field contents 2) count
   If two fields, result has these fields: 1) field contents 2) sum of field 2 */
else if( strnicmp( action, "cou", 3 )==0 ) {    
	char curcon[120];
	double count;
	count = 0.0;
	strcpy( curcon, "" );
	for( i = 0; i < Nrecords[ds]; i++ ) {
		if( strcmp( da( i, fld[0] ), curcon )!=0 ) {
			if( i == 0 ) strcpy( curcon, da( i, fld[0] ) );
			else	{
				out( curcon );
				sprintf( buf, "%g", count );
				out( buf );
				eor();
				strcpy( curcon, da( i, fld[0] ) );
				count = 0.0;
				}
			}
		/* bug - counts off by one - moved from above the if .. scg 1/25/02 */
		if( nfld == 1 ) count = count + 1.0;
		else if( nfld == 2 ) count = count + atof( da( i, fld[1] ) );
		}
	out( curcon );
	sprintf( buf, "%g", count );
	out( buf );
	eor();
	}
			

/* total */
else if( strnicmp( action, "tot", 3 )==0 )  {
	for( i = 0; i < nfld; i++ ) {
		for( j = 0; j < Nrecords[ds]; j++ ) accum[i] += atof( da( j, fld[i] ) );
		}
	}



else	{
	/* just write out fields */
	for( i = 0; i < Nrecords[ds]; i++ ) {
		if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
			int result;
               		stat = do_select( selectex, i, &result );
                	if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
                	if( result == 0 ) continue; /* reject */
                	}
		for( j = 0; j < Nfields[ds]; j++ ) {
			if( dofld( j ) ) {
				/* fprintf( fp, "\"%s\" ", da( i, j ) ); */
				out( da( i, j ) );
				}
			}
		/* fprintf( fp, "\n" ); */
		eor();
		}
	}

if( !stackmode ) fclose( outfp );


if( GL_slmember( action, "per* acc* tot*" )) {
	/* make a comma-delimited list of totals for TOTALS */
	strcpy( buf, "" );
	for( i = 0; i < nfld; i++ ) {
		char out[40];
		sprintf( out, rformat, accum[i] );
		if( dispformatnum ) {  /* rewrite using numbernotation */
			resetbns = 0;
			if( Bignumspacer == '\0' ) {
				Bignumspacer = ',';
				resetbns = 1;
				}
			rewritenums( out ); /* rewrite w/spacing, decimal pt options*/
			if( resetbns ) Bignumspacer = '\0';
			}
		strcat( buf, out );
		strcat( buf, "," );
		}
	buf[ strlen( buf ) -1 ] = '\0'; /* last comma */
	setcharvar( "TOTALS", buf );

	/* if doing totals, exit here */
	if( strnicmp( action, "tot", 3 )==0 ) return( 0 );
	}

if( stackmode ) {
	Dsel++;
	Nrecords[Dsel] = nr;
	ND[Dsel] = curd - StartD[ds];
	setintvar( "NRECORDS", Nrecords[Dsel] );
	setintvar( "NFIELDS", Nfields[Dsel] );
	if( showresults ) {
		for( i = 0; i < Nrecords[Dsel]; i++ ) {
			for( j = 0; j < Nfields[Dsel]; j++ ) {
				fprintf( Diagfp, "|%s| ", da( i, j ) );
				}
			fprintf( Diagfp, "\n" );
			}
		}
	}

if( !stackmode ) {
	/* have proc getdata read in the temp file.. */
	getdata_specialmode( 1, tmpfile, showresults );
	PLP_getdata();
	getdata_specialmode( 0, "", 0 );
	if( !outfile ) unlink( tmpfile );
	}

return( 0 );
}

/* ================ */
/* DOFLD - return 1 or 0 depending on whether field has been listed
   in keepfields or rejectfields */
static int
dofld( fld )
int fld;
{
int i;
if( nrejfld > 0 ) {
	for( i = 0; i < nrejfld; i++ ) if( fld == rejfld[i] ) break;
	if( i != nrejfld ) return( 0 ); /* no */
	}
if( nkpfld > 0 ) {
	for( i = 0; i < nkpfld; i++ ) if( fld == kpfld[i] ) break;
	if( i != nkpfld ) return( 1 ); /* yes */
	else return( 0 ); /* no */
	}
return( 1 );
}

/* ================= */
/* OUTPUT mgmt routines */
init()
{
nr = 0;
nf = 0;
}

out( s )
char *s;
{
if( !stackmode ) fprintf( outfp, "%s	", s );
else 	{
	catitem( s, &curd, &davail );
	nf++;
	}
}

eor()
{
if( !stackmode ) fprintf( outfp, "\n" );
else	{
	Nfields[Dsel+1] = nf;
	nf = 0;
	nr++;
	}
}

