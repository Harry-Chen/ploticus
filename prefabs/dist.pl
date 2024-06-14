// ploticus data display engine.  Software, documentation, and examples.
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details.
// http://ploticus.sourceforge.net


//// DIST - frequency distribution

//// load dist-specific parameters..
#setifnotgiven binsize = ""
#setifnotgiven color = "pink"
#setifnotgiven barwidth = ""
#setifnotgiven savetbl = ""
#setifnotgiven cats = ""
#setifnotgiven order = "natural"

//// load standard parameters..
#include $chunk_setstd


//// read data...
#include $chunk_read

#musthave fld

//// do title..
#include $chunk_title


//// user pre-plot include
#if @include1 != ""
  #include @include1
#endif


//// set up plotting area..
#include $chunk_area
#if @cats = yes
  xrange: 0 100
#elseif @xrange = ""
  xautorange: datafield=@fld
#else
  xrange: @xrange
#endif
yrange: 0 100   // to be revised after we run the distribution..


// for categories, stubs must be done AFTER the distribution is run below..
#if @cats != yes
  #include $chunk_xaxis
  stubs: inc @xinc
  stubcull: yes
  stubdetails: size=8
#endif

#if @binsize = "" && @cats != yes
  #endproc 
  #set binsize = $arith( @XINC/2 ) 
#endif

//// tabulate the distribution of values, e.g. how many strains fell into each bin
#proc tabulate
datafield1: @fld
#if @cats != yes
  doranges1: yes
  rangespec1: @XMIN @binsize
  showrange: avg
#else
  order1: @order
#endif
#if @savetbl != ""
  savetable: @savetbl
#endif

//// now that we have the distribution, recompute the plotting area with a auto Y range
#include $chunk_area
#if @yrange = ""
  yautorange: datafield=2
#elseif @yrange = 0
  yautorange: datafield=2 lowfix=0
#else
  yrange: @yrange
#endif
#if @cats = yes
  xscaletype: categories
  xcategories: datafield=1
#else
  xrange: @XMIN @XMAX
#endif

#include $chunk_yaxis

#if @cats = yes
  #proc xaxis
  stubs: usecategories
  stubdetails: size=8
  #if @stubvert = yes
    stubvert: yes
  #endif
#endif



//// do the background curve..
#if @curve = yes && @NRECORDS > 6
  #proc curvefit
  curvetype: bspline
  xfield: 1
  yfield: 2
  order: 5
  linedetails: color=gray(0.5) width=0.5
#endif


//// do the bars..
#proc bars
locfield: 1
lenfield: 2
color: @color
#if @barwidth != ""
  barwidth: @barwidth
#endif
outline: no
hidezerobars: yes
clickmapurl: @clickmapurl

//// user post-plot include
#if @include2 != ""
  #include @include2
#endif
