// ploticus data display engine.  Software, documentation, and examples.  
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details. 
// http://ploticus.sourceforge.net

//// STACK - do a stacked bar graph 


//// load stack-specific parms..
#setifnotgiven barwidth = ""
#setifnotgiven xnumeric = ""
#setifnotgiven y2 = ""
#setifnotgiven y3 = ""
#setifnotgiven y4 = ""
#setifnotgiven color = orange
#setifnotgiven color2 = "powderblue"
#setifnotgiven color3 = "dullyellow"
#setifnotgiven color4 = "drabgreen"
#setifnotgiven name = ""
#setifnotgiven name2 = ""
#setifnotgiven name3 = ""
#setifnotgiven name4 = ""
#setifnotgiven outline = no
#if @CM_UNITS = 1
  #setifnotgiven legend = "min+1.25 max+1.25"
#else
  #setifnotgiven legend = "min+0.5 max+0.5"
#endif


//// load standard parms..
#include $chunk_setstd


//// read data..
#include $chunk_read


//// plot area..
#include $chunk_area
#if @xnumeric = yes
  xautorange: datafield=@x incmult=2.0
#else
  xscaletype: categories
  xcategories: datafield=@x
#endif
#if @yrange = ""
  yautorange: datafields=@y,@y2,@y3,@y4 combomode=stack incmult=2.0 
#elseif @yrange = 0
  yautorange: datafields=@y,@y2,@y3,@y4 combomode=stack incmult=2.0 lowfix=0
#else
  yrange: @yrange
#endif


//// x axis..
#include $chunk_xaxis
stubcull: yes
#if @xnumeric = yes
  stubs: inc @xinc
#else
  stubs: usecategories
#endif


//// y axis..
#include $chunk_yaxis
stubcull: yes


//// title..
#include $chunk_title


//// user pre-plot include..
#if @include1 != ""
  #include @include1
#endif


//// do 1st level bars..
#proc bars
locfield: @x
lenfield: @y
color: @color
outline: @outline
#if @barwidth != ""
  barwidth: @barwidth
#endif
legendlabel: @name

//// 2nd level bars..
#if @y2 != ""
  #proc bars
  locfield: @x
  lenfield: @y2
  stackfields: *
  color: @color2
  outline: @outline
  #if @barwidth != ""
    barwidth: @barwidth
  #endif
  legendlabel: @name2
#endif

//// 3rd level bars..
#if @y3 != ""
  #proc bars
  locfield: @x
  lenfield: @y3
  stackfields: *
  color: @color3
  outline: @outline
  #if @barwidth != ""
    barwidth: @barwidth
  #endif
  legendlabel: @name3
#endif

//// 4th level bars..
#if @y4 != ""
  #proc bars
  locfield: @x
  lenfield: @y4
  stackfields: *
  color: @color4
  outline: @outline
  #if @barwidth != ""
    barwidth: @barwidth
  #endif
  legendlabel: @name4
#endif

//// legend..
#if @name != ""
#proc legend
location: @legend
#endif

//// user post-plot include..
#if @include2 != ""
  #include @include2
#endif
