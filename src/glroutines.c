/* GLROUTINES.C - general library routines
 * Copyright 1998-2002 Stephen C. Grubb  (ploticus.sourceforge.net) .
 * This code is covered under the GNU General Public License (GPL);
 * see the file ./Copyright for details. */

/* OS-specific #defines in rand() below. */

#include <stdio.h> 
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#define stricmp( s, t ) 	strcasecmp( s, t )
#define strnicmp( s, t, n )     strncasecmp( s, t, n )

#define DATAMAXLEN 256
#define NUMBER 0
#define ALPHA 1

static int encode();
static int getdecplaces();
static int wcmp();

static char Sep = ',';  /* separator character for lists */
static int domember();
static char     Gettok_buf[256];
static char starsym = '*';
static int Maxlen = 99999;


/* ================================= */
/* thanks to Markus Hoenicka for this more portable sysdate and systime code */
/* SYSDATE - get today's date */ 

GL_sysdate( mon, day, yr )
int	*mon, *day, *yr ;
{
time_t clock;
struct tm *ltime;
time(&clock);
ltime = localtime(&clock);
*mon = ltime->tm_mon + 1;
*day = ltime->tm_mday;
*yr = ltime->tm_year;
if( (*yr) >= 100 ) (*yr) = (*yr) % 100;  /* scg y2k 11/10/98 */
return( 0 );
}

/* ================================= */
/* SYSTIME - get current time */ 
GL_systime( hour, min, sec )
int	*hour, *min, *sec ;
{
time_t clock;
struct tm *ltime;
time(&clock);
ltime = localtime(&clock);
*hour = ltime->tm_hour;
*min = ltime->tm_min;
*sec = ltime->tm_sec;
return( 0 );
}


/* ===================================================== */
/* RAND returns a "random" number between 0.0 and 1.0 */
double GL_rand()
{
double r;
static int first = 1;
if( first ) {
	srand( getpid() % 1000 );
	first = 0;
	}
r = rand() / (double)(RAND_MAX);
if( r < 0.0 || r > 1.0 ) { printf( "%f: rand return out of range\n", r ); exit(1); }
return( r );
}


/* ============================================= */
/* GETOK - copied so that buffer size could be increased.. */
 
char *GL_getok( string, index )
char    *string;
int     *index;
{
int n;
while( GL_member( string[(*index)], " \t\n" ) ) (*index)++;
for( n=0;
        n <= 255 &&
        string[*index] != ' '  &&
        string[*index] != '\t'  &&
        string[*index] != '\n'  &&
        string[*index] != '\0'  ;
                Gettok_buf[n++] = string[(*index)++] )  ;
Gettok_buf[n] = '\0' ;
return( Gettok_buf );
}

/* ===================================================================== */
/* SMEMBER - look for s in list t (white-space delimited).  Case sensitive.
   If found return 1 else 0. */

GL_smember( s, t )
char s[], t[];
{
char tok[DATAMAXLEN+1], *GL_getok();
int i;
i = 0;
while( 1 ) {
	strcpy( tok, GL_getok( t, &i ) );
	if( tok[0] == '\0' ) break;
	if( strcmp( tok, s ) == 0 ) return( 1 );
	}
return( 0 );
}
/* ===================================================================== */
/* SMEMBERI - look for s in list t (white-space delimited).  Case insensitive.
   If found return 1 else 0. */

GL_smemberi( s, t )
char s[], t[];
{
char tok[DATAMAXLEN+1], *GL_getok();
int i;
i = 0;
while( 1 ) {
	strcpy( tok, GL_getok( t, &i ) );
	if( tok[0] == '\0' ) break;
	if( stricmp( tok, s ) == 0 ) return( 1 );
	}
return( 0 );
}

/* =========================================== */
/* SLMEMBER - Return 1 if str is matches any items in list.
	List is a space-delimited list of tokens.  The tokens in the
	list may contain ? or * wildcard characters.  The match is
	Case insensitive.

	scg 3-19-97
*/
GL_slmember( str, list )
char *str;
char *list;
{
char tok[100], *GL_getok();
int i;
i = 0;
while( 1 ) {
        strcpy( tok, GL_getok( list, &i ) );
        if( tok[0] == '\0' ) break;
        if( GL_wildcmp( str, tok, strlen(tok), 0 ) == 0 ) return( 1 );
        }
return( 0 );
}


/* ====================================================================== */
/* MEMBER - returns char position if character c is a member of string s, 
		0 otherwise. Char positions start with 1 for this purpose.  */
GL_member( c, s )
char c, s[];
{
int i, len;
for( i = 0, len = strlen( s ); i < len; i++ ) if( s[i] == c ) return( i+1 );
return( 0 );
}


/* ===================================================================== */
/* GOODNUM - checks a token to see if it is a legal number.  Returns 1 = yes  0 = no.
	'prec' is returned.. and is the precision of the number (position of decimal point).

	Number may contain unary + or -, and one decimal point.
	Leading and trailing whitespace are discarded before determining
	whether the case is a valid number or not.
*/

GL_goodnum( str, prec )
char *str;
int *prec;
{
int i, start, len, p, bad;

bad = 0; *prec = -1;
len = strlen( str );

/* find limit of trailing whitespace.. */
for( i = len-1; i >= 0; i-- ) if( !isspace( str[i] ) ) break;
len = i+1;

/* skip over leading whitespace.. */
for( i = 0; i < len; i++ ) if( !isspace( str[i] ) ) break;
start = i;

/* screen out degenerate cases.. */
if( len < 1 ) return( 0 ); /* degenerate case "" */
if( len-start == 1 && ( str[start] == '.' || str[start] == '+' || str[start] == '-' ) )
	return( 0 );       /* degenerate case; ".", "+", "-" */

/* check invididual characters.. */
for( p = start; p < len; p++ ) { 
	if( str[p] == '.' ) { 
		if( *prec == -1 ) *prec = p; 
		else bad=1; 
		}
	else if( p == start && ( str[p] == '-' || str[p] == '+' ) );
	else if( ! isdigit( str[p]) ) bad=1;
	}

/* return.. */
if( bad ) return( 0 );
else return( 1 );
}

/* =========================================== */
/* GETSEG - Get fields, which are delimited by any member of sepstring.
   Similar to GL_getchunk(); however
   Whereas GL_getchunk() skips over adjacent separators,
   this routine delimits on EACH separator character encountered,
   Also, separator character is ignored if preceded in inbuf by a backslash(\). 

   Returns 1 when end-of-line is reached and no token is being returned.

   SCG 12-2-96
*/


GL_getseg( rtn, inbuf, i, sep )
char rtn[];
char inbuf[];
int *i;
char *sep;
{
int n;
int escaping;
int eol;

n = 0;
rtn[0] = '\0';
escaping = 0;
eol = 0;
while( 1 ){
	if( inbuf[*i] == '\0' ) { 
		if( n == 0 ) eol = 1; 
		break; 
		}
	else if( GL_member( inbuf[*i], sep ) && !escaping ) { (*i)++; break; }
	else if( inbuf[*i] == '\\' && GL_member( inbuf[(*i)+1], sep ) ) { 
		escaping = 1; 
		(*i)++; 
		continue; 
		}
	else rtn[n++] = inbuf[(*i)++];
	if( n >= 511 ) break; /* 512 max */
	escaping = 0;
	}
rtn[n] = '\0' ;
return( eol );
}

/* ==================================================================== */
/* GETCHUNK - Get tokens, which are separated by any member of sepstring */

GL_getchunk( rtn, line, i, sepstring )
char rtn[];
char line[];
int *i;
char sepstring[];
{

int n;

while( GL_member( line[(*i)], sepstring ) ) (*i)++; 
n = 0;
rtn[0] = '\0';
while( 1 ){
	if( GL_member( line[*i], sepstring ) || line[*i] == '\0' ) break;
	else rtn[n++] = line[(*i)++];
	if( n >= (Maxlen-1) ) break;
	}
rtn[n] = '\0' ;
}

/* ==================================================================== */
/* SETMAXLEN - set maximum token length for GETSEG (future: others) */

GL_setmaxlen( maxlen )
int maxlen;
{
if( maxlen == 0 ) Maxlen = 99999;
else Maxlen = maxlen;
return( 0 );
}


/* ===================================================================== */
/* WILDCMP - compare two strings s1 and s2.  S2 may contain 
   wildcards (* and ?).

   Function returns 0 on a match;  < 0 if s1 < s2;   > 0 if s1 > s2
   Prints an error message and returns -999 on error.

   * wildcard limited to the following uses: *ppp; ppp*; pp*pp; *ppp*
   ? can be used anywhere.

   Double asterisks at beginning and end are also handled (means the
   same as single asterisk).

   scg 3-4-96 (written elsewhere)

 */


GL_wildcmp( char *s1, char *s2, int len, int casecare )
/* s1  = data value
   s2  = query value which can contain wildcards - not null terminated.
   len = length of s2
   casecare = 0 for case-insensitive, 1 for case-sensitive
 */
{
int i, nwc, wcp, stat, stat2;


if( len == 0 ) return( strlen( s1 ) );
else if( s2[0] == starsym ) {
	if( len == 1 ) return( 0 ); /* everything matches */
	}
else if( s2[0] == '?' ) ; /* can't tell yet */
else if( tolower( s1[0] ) < tolower( s2[0] ) ) return( -1 ); /* way off */
else if( tolower( s1[0] ) > tolower( s2[0] ) ) return( 1 );  /* way off */

/* strip off extraneous * at beginning and end.. */
if( s2[0] == starsym && s2[1] == starsym ) { s2 = &s2[1]; len--; }
if( s2[len-1] == starsym && s2[len-2] == starsym ) len--;

/* see if any wild cards were used.. */
nwc = 0;
for( i = 0; i < len; i++ ) if( s2[i] == starsym ) { nwc++; wcp = i; }

if( nwc < 1 ) {  /* straight match */
	if( strlen( s1 ) > len ) return( wcmp( s1, s2, strlen( s1 ), casecare));
	else return( wcmp( s1, s2, len, casecare ) ); 
	}

else if( nwc == 1 ) {                /* wildcard match */
	/* find beginning of what we need to compare */
	i = strlen( s1 ) - (len - (wcp+1) );

	/* case 1: wc at end.. */
	if( wcp == len-1 ) {
		return( wcmp( s1, s2, len-1, casecare ) );
		}

	/* case 2: wc at beginning.. */
	if( wcp == 0 ) {
		return( wcmp( &s1[i], &s2[ 1 ], len-1, casecare ) );
		}

	/* case 3: wc in middle.. */
	else	{
		int frontlen, backlen;

		frontlen = wcp;

		/* do front compare.. */
		stat = wcmp( s1, s2, frontlen, casecare ); 
		if( stat != 0 ) return( stat );

		backlen = strlen( s2 ) - (frontlen + 1);
		if( strlen( s1 )  < frontlen + backlen ) return( 1 );  /* fail if s1 too short */

		/* do back compare.. */
		stat = wcmp( &s1[ strlen( s1 ) - backlen ], &s2[ strlen( s2 ) - backlen ], backlen, casecare );
		return( stat );

		}
	}

else if( nwc == 2 ) {
	int stop;
	/* case 4: wc at beginning and end.. */
	if( wcp != (len-1) ) goto ERR;
	else if( s2[0] != starsym ) goto ERR;
	stop = ( strlen( s1 ) - len ) + 2;
	for( i = 0; i <= stop; i++ ) {
		if( wcmp( &s1[i], &s2[1], len-2, casecare ) == 0 ) return( 0 );
		}

	return( -1 );
	}
else 	{
	ERR:
	fprintf( stderr, "Wild card match error (%s vs %s).\n", s1, s2 );
	return( -999 );
	}
}

/* WCMP - compare two strings.  S2 may contain ? wildcards which matches any
       single character.  Len is the # of characters to check.
 */
static int
wcmp( char *s1, char *s2, int len, int casecare )
{
int i;
for( i = 0; i < len; i++ ) {
	if( ! casecare ) {
		if( tolower(s1[i]) < tolower(s2[i]) && s2[i] != '?' ) 
			return( -1 );
		else if( tolower(s1[i]) > tolower(s2[i]) && s2[i] != '?' ) 
			return( 1 );
		}
	else	{
		if( s1[i] < s2[i] && s2[i] != '?' ) return( -1 );
		else if( s1[i] > s2[i] && s2[i] != '?' ) return( 1 );
		}
	}
return( 0 );
}

/* WILDCHAR - set the wildcard symbol to be used instead of '*' */
GL_wildchar( c )
char c;
{
starsym = c;
return( 0 );
}

/* ===================================================================== */
/* FUZZYMATCH - Do a 'fuzzy' match.  See if s1 is similar to s2.

   This routine may alter s1 and s2.

   A wildcard character '*' at the beginning or end of s2 indicates
   that a match can occur anywhere within s1 (eg. smith* would match
   smithington and *smith would match harrowsmith; *smith* would match
   both).  If there are no asterisks, lengths
   of the two strings must be similar.

   Degree changes the 'looseness' of the match.  
   5 = strict, 4 = medium-strict, 3 = medium 2= loose, 1 = very loose.
   
   Returns 1 on a match, 0 not. 

   SCG '94,'95
 */

GL_fuzzymatch( s1, s2, len2, degree )
char *s1; /* data value */
char *s2; /* query value */
int len2; /* length of s2 */
int degree; /* dgree of tightness */
{
int i, j, k, len1, pts;
int bestpts, score;
char c;
int openfront, openend, goal;

/* exact match.. no need to do more.. */
if( strcmp( s2, s1 ) ==0 ) return( 1 );

len1 = strlen( s1 );

/* see what type of match, open or closed */
if( s2[0] == '*' ) { openfront = 1; s2 = &s2[1]; len2--; }
else openfront = 0;
if( s2[ len2-1 ] == '*' ) { openend = 1; len2--; }
else openend = 0;

/* set goal */
if( len2 < 4 )goal = len2 * degree;
else if( len2 < 10 ) goal = (len2 -2) * degree;
else goal = (len2 -3) * degree;


/* reject disparate lengths if doing a closed match */
if( !openfront && !openend && len1 - len2 > 3 ) return( 0 );

/* truncate end of s1 if openended match */
if( openend && !openfront && len1 > len2+2 ) len1 = len2+2;

/* chop of front of s1 if openfronted match */
if( openfront && !openend && len1 > len2+2 ) s1 = &s1[ len1-(len2+2) ];

/* printf( "[%d-%d]", openfront, openend ); */

/* go thru s2.. */
score = 0;
for( i = 0; i < len2; i++ ) {
	c = s2[i];

	/* find each occurrence of c in s1.. */
	bestpts = 0;
	pts = 0;
	for( j = 0; j < len1; j++ ) {

		if( s1[j] == c ) {

			/* check adjacent characters, count matches.. */
			pts = 1;

			if( i + j == 0 ) pts+=2; /* both 1st char */
			
			else /* go left.. */
			  for( k = 1; k <= 4; k++ ) {
				if( j-k < 0 || i-k < 0 ) break;
				else if( s1[j-k] == s2[i-k] ) pts++;
				else break;
				}
			if( i == len2-1 && j == len1-1 )
				pts+=2; /* both last char */

			else /* go right.. */
			  for( k = 1; k <= 4; k++ ) {
				if( j+k > len1-1 || i+k > len2-1 ) break;
				else if( s1[j+k] == s2[i+k] ) pts++;
				else break;
				}
			}
		if( pts > bestpts )bestpts = pts;
		}
	if( bestpts > 6 ) score += 6;
	else score += bestpts;
	/* printf( "[%c:%d]", c, bestpts ); */
	if( score > goal ) return( 1 ); /* optimize */
	}
/* printf( "<%s v. %s : %d>\n", s2, s1, score ); */
if( score > goal ) return( 1 );
else return( 0 );
}


/* ============================================= */
/* EXPAND_TABS Takes a string parameter 'in' and expands tabs into spaces, placing the
      result into parameter 'out'.  
 */
GL_expand_tabs( out, in )
char in[], out[];
{
int i, j, k, l, len;

out[0] = '\0';
k = 0;
for( i = 0, len = strlen( in ); i < len; i++ ) {
	if( in[i] == '\t' ) {
		j =  8 - ( k % 8 ); /* 1 to 8 spaces needed */
		for( l = 0; l < j; l++ ) out[k++] = ' ';
		}
	else out[k++] = in[i];
	}
out[k] = '\0';
}


/* ============================================= */
/* MAKE_UNIQUE_STRING - generate an identifier using date, time, and pid */

GL_make_unique_string( s, i )
char *s;
int i;  /* may be sent as an integer.. if 0 getpid() will be used.. */
{
int mon, day, yr, hr, min, sec, pid, a, b, c;
GL_sysdate( &mon, &day, &yr );
GL_systime( &hr, &min, &sec );
s[0] = encode( yr % 100 );
s[1] = encode( mon );
s[2] = encode( day );
s[3] = encode( hr );
s[4] = encode( min );
s[5] = encode( sec );
if( i == 0 ) pid = getpid();
else pid = i;
s[6] = encode( pid % 62 );
pid = pid / 62;
s[7] = encode( pid % 62 );
s[8] = encode( pid / 62 );
s[9] = '\0';

return( 0 );
}

/* encode - derive a character representation of a number */
static int encode( a )
int a;
{
if( a >= 0 && a <= 9 ) return( a + '0' );
else if( a > 35 ) return( (a-36) + 'A' ); /* A-Z    26 letters + 9 = 35 */
else if( a > 9 ) return( (a-10) + 'a' ); /* a-z */
else return( '0' );
}

/* ============================================= */
/* ADDMEMBER - append a new member to the end of a comma-delimited list */
GL_addmember( newmem, list )
char *newmem;
char *list;
{
if( list[0] == '\0' ) strcpy( list, newmem );
else	{
	strcat( list, "," );
	strcat( list, newmem );
	}
return( 0 );
}

/* ============================================= */
/* CONTAINS - if string s contains any of chars in clist, return position (1=first)
     of first occurance in list.  0 if not found at all.
     example: contains( "\"*'", "'hello'" )  -> 1
 */
GL_contains( clist, s )
char *clist, *s;
{

int i, len;

for( i = 0, len = strlen( s ); i < len; i++ ) {
	if( GL_member( s[i], clist )) return( i+1 );
	}
return( 0 );
}
/* ============================================= */
/* SUBST - change all occurances of s1 to s2, in t.

   Max length of t is 255.
   Returns 0 if successful, 1 if no occurance of s1 found. */

GL_substitute( s1, s2, t )
char *s1, *s2, *t;
{
char buf[255];
int i, j, len1, buflen, found;

len1 = strlen( s1 );
if( len1 < 1 ) return( 1 );
strcpy( buf, t );
buflen = strlen( buf );
if( buflen < 1 ) return( 1 );

found = 0;
j = 0;
for( i = 0; i < buflen; i++ ) {
/* 	printf( "[%s|%s|%d]", &buf[i], s1, len1 ); */

	if( strncmp( &buf[i], s1, len1 )==0 ) {
		strcpy( &t[j], s2 );
		j += strlen( s2 );
		i += (len1 - 1);
		found = 1;
		}
	else t[j++] = buf[i];
	}
t[j] = '\0';
if( found ) return( 0 );
else return( 1 );
}

/* ============================================= */
/* CHANGECHARS - go through string s and if any characters in clist found, change
	the character to newchar */
GL_changechars( clist, s, newchar )
char *clist, *s, *newchar;
{

int i, len;

for( i = 0, len = strlen( s ); i < len; i++ ) {
	if( GL_member( s[i], clist )) s[i] = newchar[0];
	}
return( 0 );
}

/* ============================================= */
/* DELETECHARS - go through string s and if any characters in clist found, delete
	the character. */
GL_deletechars( clist, s )
char *clist, *s;
{

int i;

for( i = 0; ; ) {
	if( s[i] == '\0' ) break;
	if( GL_member( s[i], clist )) {
		strcpy( &s[i], &s[i+1] );
		continue;
		}
	else i++;
	}
return( 0 );
}
/* ======================================================================== */
/* SUBSTRING - 
   GL_substring( result, str, fromchar, nchar )
   char *result;   // substring is copied into this variable
   char *str;	// the original string
   int fromchar;   // starting point from which to take the substring
   int nchar;	// length of the substring

   In all cases the first char is 1, not 0.

   Two ways it can operate:
    If <fromchar> is greater than 0, the result will be the portion of <string>
	beginning at position <fromchar>, for a length of <nchar>, or until
	the end is reached.
    If <fromchar> is less than 0, and <nchar> is greater than 0:
	we will begin counting from the end of <string>, 
	leftward.  for (abs)<fromchar> characters.  Then, we will take the 
	substring beginning from that character 
	for a length of <nchar>, or until the end is reached.
	
    Examples: substring( result, "02001.fv02", 7, 4 )    -- result would be "fv02"
              substring( result, "02001.fv02", -4, 99 )   -- result would be "fv02"
*/

GL_substring( result, str, fromchar, nchar )
char *result;
char *str;
int fromchar;
int nchar;
{
int i, j;
int len;

len = strlen( str );
if( fromchar > 0 ) fromchar -= 1;
else if( fromchar < 0 ) fromchar += len;

for( i = fromchar, j = 0; i < fromchar + nchar; i++ ) { 
	if( i > len-1 ) break;
	result[j++] = str[i];
	}
result[j] = '\0';
return( 0 );
}

/* ===================================================================== */
/* VARSUB - given string s, find every occurance 
   of symbol (case-sensitive) and change it to value.  
    -Copies result into s.  
     -Returns number of times a substitution was made, 0 if none.
     -This routine is not sophisticated about delimiting the symbol;
      e.g. if s contains $NUMBER and varsub() is looking for $NUM it will find it.
*/
GL_varsub( s, symbol, value )
char *s, *symbol, *value;
{
int i, j, len, found;
int slen;
char rtnbuf[256];

len = strlen( symbol );
slen = strlen( s );
found = 0;
for( i = 0, j = 0; i < slen; i++, j++ ) {
	if( strncmp( &s[i], symbol, len )==0 ) {
		strcpy( &rtnbuf[j], value );
		j = strlen( rtnbuf ) - 1;
		i+= (len-1);
		found++;
		}
	else rtnbuf[j] = s[i]; 
	}
rtnbuf[j] = '\0';
strcpy( s, rtnbuf );
return( found );
}



/* ============================================= */
/* AUTOROUND - round the decimal portion a number reasonably based on its magnitude.
   val is the value, represented as a string.
   decoffset controls the precision of the rounded result as follows:
	If decoffset = 0 then nothing happens.  
	If decoffset = 1 then rounding will go to 1 additional decimal place.  
	decoffset = -1 then rounding will go to one less decimal place than normal.

   The rounded result is copied back into val.

   If val is non-numeric or a whole number then it is left unchanged.
*/
GL_autoround( val, decoffset )
char *val;
int decoffset; 
{
int precision, decplaces, stat;
char roundingfmt[50];
double g, atof();

stat = GL_goodnum( val, &precision );
if( stat && precision > 0 ) {
	g = atof( val );
	decplaces = getdecplaces( g );
	if( decplaces > -99 ) {
		if( decplaces < 0 ) decplaces = 0;
		sprintf( roundingfmt, "%%.%df", decplaces + decoffset );
		sprintf( val, roundingfmt, g );
		}
	}
return( 0 );
}

/* ============================================= */
/* AUTOROUNDF - variant of autoround(), takes val as a double, return value is character rep..*/
char *
GL_autoroundf( val, decoffset )
double val;
int decoffset;
{
int precision, decplaces, stat;
char roundingfmt[50];
static char result[50];

sprintf( result, "%g", val ); /* fallback */
decplaces = getdecplaces( val );
if( decplaces > -99 ) {
	if( decplaces < 0 ) decplaces = 0;
	sprintf( roundingfmt, "%%.%df", decplaces + decoffset );
	sprintf( result, roundingfmt, val );
	}
return( result );
}

static int getdecplaces( val )
double val;
{
int decplaces;
double g, fabs();

g = fabs( val );
decplaces = -99;
if( g >= 1000 ) decplaces = 0; 
else if( g >= 100 ) decplaces = 0; 
else if( g >= 10 ) decplaces = 1;
else if( g >= 1.0 ) decplaces = 2;
else if( g >= 0.1 ) decplaces = 3;
else if( g >= 0.01 ) decplaces = 4;
else if( g >= 0.001 ) decplaces = 5;
else if( g >= 0.0001 ) decplaces = 6;

return( decplaces );
}

/* ======================== */
/* FMOD  */
double fmod( a, b )
double a, b;
{
double x, y;

x = a / b;
y = (int) (x) * b;
return( a - y );
}


/* ======================================================================== */
/* RANGER - take a range specification of integers and return an enumeration of all members.
 *	    Examples: "3-8" would return in list array: 3,4,5,6,7,8
 *		      "4,5,7-9,12-last" would return (for a 15 member list): 4,5,7,8,9,12,13,14,15
 *		      "4,5 7-9 12-last" would be equivalent to the above example.
 *		      "1-last" would return (for an 8 member list): 1,2,3,4,5,6,7,8
 *
 * 	    There may be no embedded spaces within the dash construct.
 */

GL_ranger( spec, list, n )
char *spec;
int *list;  /* array */
int *n;  /* in: size of list (max number of members)
	    out: number of members in list that have been filled */
{
int i, ix, p, j, lo, hi;
char tok[80], histr[80];

/* parse spec.. */
ix = 0;
i = 0;
while( 1 ) {
	/* split up on commas or spaces */
	GL_getchunk( tok, spec, &ix, ", " );
	if( tok[0] == '\0'  ) break;

	if( GL_goodnum( tok, &p ) ) {
		list[i] = atoi( tok );
		i++;
		}
	else	{
		sscanf( tok, "%d-%s", &lo, histr );
		if( stricmp( histr, "last" )==0 ) hi = *n;
		else hi = atoi( histr );
		if( hi < lo ) {
			fprintf( stderr, "bad range specification: %s\n", tok );
			return( 1 );
			}
		for( j = lo; j <= hi; j++ ) {
			list[i] = j;
			i++;
			}
		}
	}
*n = i;
return( 0 );
}


/* ============================== */
/* CLOSE_TO - test two floating point numbers to see if
        they are within a small tolerance. */

GL_close_to( a, b, tol )
double a, b;
double tol;
{
if( a == b ) return( 1 );
else if( a > b && a - b < tol ) return( 1 );
else if( b > a && b - a < tol ) return( 1 );
else return( 0 );
}


/* ================================= */
/* CHECKSUM FUNCTIONS  */

/* CHECKDIG - returns the check digit for the number in aptr with length l.
 	NOTE: when the check digit is computed to be 10 an `x' is returned.  

   This algorithm protects against interchanged digits.
*/
 
#define TEN	'x'

char GL_checkdig(aptr,l)
char *aptr;
int l;
{
int i,odds,evens,checkdigit;
	
odds = evens = 0;
i = 0;

while (i < l) {
	if (i % 2 == 0) { evens += (*(aptr+i) - '0'); }
	else { odds += (*(aptr+i) - '0'); }
	i++;
	}
checkdigit = (odds + (3 * evens)) % 11;
	
/* if the checkdigit is 10 then return the character to be used in place of ten */
if ( checkdigit == 10 ) return ( TEN );
else return(checkdigit+'0'); 
}


/* ------------ */
/* script entry point for checksum functions */
GL_checksum_functions( hash, name, arg, nargs, result, typ )
int hash;
char *name;
char *arg[];
int nargs;
char *result;
int *typ;
{
char *s;

if( hash == 2182 ) { /* $checksumvalid(s) -  validate s which is an integer containing a trailing check digit, 
   			using the algorithm in $ps/clibx/checkdig  */
	int i;
	i = GL_checkdig( arg[0], strlen( arg[0] ) -1 );
	if( i == arg[0][ strlen( arg[0] ) -1 ] ) sprintf( result, "1" );
	else sprintf( result, "0" );
	*typ = NUMBER;
	return( 0 );
	}

if( hash == 2412 ) { /* $checksumencode(i) - result is i with checksum digit appended. */
	long i, atol();
	char tmp[20];
	int cs;
	i = atol( arg[0] );
	sprintf( tmp, "%ld", i );
	cs = GL_checkdig( tmp, strlen( tmp ) );
	sprintf( result, "%s%c", tmp, cs );
	*typ = ALPHA;
	return( 0 );
	}
	
if( hash == 2155 ) { /* $checksumnext(s) take s which is a number including trailing checksum 
   			digit, increment number and recompute new checksum digit. */
	int cs;
	long i, atol();
	char tmp[20];
	s = arg[0];
	s[ strlen( s ) - 1 ] = '\0'; /* strip off existing checksum digit */
	i = atol( s );
	i++;
	sprintf( tmp, "%ld", i );
	cs = GL_checkdig( tmp, strlen( tmp ) );
	sprintf( result, "%s%c", tmp, cs );
	*typ = ALPHA;
	return( 0 );
	}

fprintf( stderr, "unrecognized function: %s\n", name ); /* not found */
return( 195 );
}


/* ============================= */
/* COMMONMEMBERS - compare two commalists and return number of members
   that are in common.. */

GL_commonmembers( list1, list2, mode )
char *list1;
char *list2;
int mode; 	/* 0 = return a count; 1 = quit when one found */
{
int i, j, ii, ij, count;
int len1, len2;
char tok1[DATAMAXLEN+1], tok2[DATAMAXLEN+1];

count = 0;
len1 = strlen( list1 );
len2 = strlen( list2 );
for( i = 0, ii = 0; ; i++ ) {
	if( ii >= len1 ) break;
	GL_getseg( tok1, list1, &ii, "," );
	for( j = 0, ij = 0; ; j++ ) {
		if( ij >= len2 ) break;
		GL_getseg( tok2, list2, &ij, "," );
		if( stricmp( tok1, tok2 )==0 ) {
			if( mode == 1 ) return( 1 );
			count++;
			}
		}
	}
return( count );
}

/* ==================================== */
/* LISTMEMBER - see if s is in list (comma-delimited); 
   if so return 1 and list position (first=1) and string position (first=0) */
GL_listmember( s, list, memnum, pos )
char *s;
char *list;
int *memnum;
int *pos;
{
int ix, i, lastix, len;
char tok[256];

*memnum = 0;
ix = 0;
len = strlen( list );
lastix = 0;
for( i = 1, ix = 0; ; i++ ) {
	if( ix >= len ) break;
	lastix = ix;
	GL_getseg( tok, list, &ix, "," );
	if( strcmp( tok, s )==0 ) {
		*memnum = i;	
		*pos = lastix;
		return( 1 );
		}
	}
return( 0 );
}

/* ===================================== */
/* GETCGIARG - get next arg from CGI REQUEST_URI string (escape constructs are converted) */
GL_getcgiarg( arg, uri, pos, maxlen )
char *arg, *uri;
int *pos; /* current position */
int maxlen; /* max size of string, including terminator */
{
int i, j;
char hex[10];
unsigned int val;

if( *pos == 0 ) {   /* scan to '?'.. */
	for( i = 0; ; i++ ) {
		if( uri[i] == '?' || uri[i] == '\0' ) {
			strncpy( arg, uri, i );
			arg[i] = '\0';
			if( uri[i] == '\0' ) *pos = i;
			else *pos = i+1;
			return( 0 );
			}
		}
	}
else	{
	for( i = *pos, j = 0; j < maxlen; i++ ) {
		if( uri[i] == '&' || uri[i] == '\0' || j >= maxlen ) {
			arg[j] = '\0';
			if( uri[i] == '\0' ) *pos = i;
			else *pos = i+1;
			return( 0 );
			}
		else if( uri[i] == '%' && isxdigit( uri[i+1] ) && isxdigit( uri[i+2] ) ) {
			sprintf( hex, "%c%c", uri[i+1], uri[i+2] );
        		sscanf( hex, "%x", &val );
			arg[j++] = (char) val;
			i += 2;
			}
		else arg[j++] = uri[i];
		}
	}
return( 0 );
}
