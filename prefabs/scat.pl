// ploticus data display engine.  Software, documentation, and examples.  
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details. 
// http://ploticus.sourceforge.net

//// SCAT - do a scatterplot of two variables with pearson coefficient and regression line

//// set scat-specific defaults..
#if @CM_UNITS = 1
  #setifnotgiven rectangle = "2.5 2.5 10 10"
  #setifnotgiven ptsize = "0.1"
  #set LBLDIST = 1.38
  #set REGLBL = 1.5
  #set PTLBL = 0.18
  #set TAILS = 0.05
#else
  #setifnotgiven rectangle = "1 1 4 4"
  #setifnotgiven ptsize = "0.04"
  #set LBLDIST = 0.55
  #set REGLBL = 0.6
  #set PTLBL = 0.07
  #set TAILS = 0.02
#endif
#setifnotgiven ptsym = "circle"
#setifnotgiven ptcolor = "blue"
#setifnotgiven corrcolor = "green"
#setifnotgiven xerr = ""
#setifnotgiven errcolor = "gray(0.7)"
#setifnotgiven idcolor = "orange"

//// load standard vars..
#include $chunk_setstd

//// read data..
#include $chunk_read

//// required vars..
#musthave x y

//// set up plotting area..
#include $chunk_area
#if @xrange != ""
  xrange: @xrange
#elseif @cats = yes
  xscaletype: categories
  xcategories: @x
#else
  xautorange: datafield=@x incmult=2.0
#endif
#if @yrange != ""
  yrange: @yrange
#else
  yautorange: datafield=@y incmult=2.0
#endif

//// x axis
#include $chunk_xaxis
stubcull: yes

//// y axis
#include $chunk_yaxis
stubcull: yes
labeldistance: @LBLDIST

//// title
#include $chunk_title

//// user pre-plot include
#if @include1 != ""
  #include @include1
#endif


//// do regression line and correlation..
#if @corr = yes
  #proc curvefit
  curvetype: regression
  xfield: @x
  yfield: @y
  linedetails: color=@corrcolor width=0.5

  #proc annotate
  location: max min-@REGLBL
  textdetails: align=R size=8 color=@corrcolor
  text: r = @CORRELATION
	@REGRESSION_LINE
#endif


//// do scatterplot..
#if @id != ""
  #proc scatterplot
  xfield: @x
  yfield: @y
  labelfield: @id
  textdetails: size=6 color=@idcolor adjust=0,@PTLBL
#endif



//// do error bars..
#if @err != ""
 #proc bars
  locfield: @x
  lenfield: @y
  errbarfield: @err
  thinbarline: color=@errcolor width=0.5
  tails: @TAILS
  truncate: yes
#endif
  
#if @xerr != ""
  #proc bars
  locfield: @y
  lenfield: @x
  horizontalbars: yes
  errbarfield: @xerr
  thinbarline: color=@errcolor width=0.5
  tails: @TAILS
  truncate: yes
#endif


//// do the data points last so they are on top of the rest of the stuff..
#proc scatterplot
xfield: @x
yfield: @y
symbol: shape=@ptshape style=filled radius=@ptsize fillcolor=@ptcolor
clickmapurl: @clickmapurl

//// user post-plot include..
#if @include2 != ""
  #include @include2
#endif
