// ploticus data display engine.  Software, documentation, and examples.
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details.
// http://ploticus.sourceforge.net

//// CHRON - do a graph over time 

//// load chron-specific parameters
#setifnotgiven y = ""
#setifnotgiven datefmt = "mmddyy"
#setifnotgiven mode = bars
#setifnotgiven barwidth = ""
#setifnotgiven color = orange
#setifnotgiven outline = no
#setifnotgiven linedet = "color=green"
#setifnotgiven step = "no"
#setifnotgiven tab = ""
#setifnotgiven tabmode = "mid"
#setifnotgiven xyears = ""
#setifnotgiven curve = ""
#setifnotgiven order = "5"
#setifnotgiven omitweekends = "no"
#setifnotgiven unittype = date
#setifnotgiven timefld = ""
#setifnotgiven fill = ""
#setifnotgiven gapmissing = ""
#if @timefld != ""
  #set unittype = datetime
#endif
#if @unittype = time
  #setifnotgiven nearest = hour
  #setifnotgiven stubfmt = HHA
#else
  #setifnotgiven nearest = day
  #setifnotgiven stubfmt = "MMMdd"
#endif
#setifnotgiven crossover = ""

//// load standard parameters..
#setifnotgiven xinc = "7"
#include $chunk_setstd

#proc datesettings
format: @datefmt
omitweekends: @omitweekends

#musthave data

//// read data file..  can't use chunk_read because this does special processing..
#proc getdata
file: @data
#if @data = stdin
  standardinput: yes
#endif
#if @delim != ""
  delim: @delim
#endif
#if @header = yes
  fieldnameheader: yes
#endif
#if @comment != ""
  commentchar: @comment
#endif
select: @select
#if @tab != ""
  #if @y != ""
    filter: ##set DT = $ref(@x)
    #if @timefld != ""
	      ##set TM = $ref(@timefld)
              ##set DT = @DT "." @TM
    #endif
	    ##set VAL = $ref(@y)
	    ##set ADJ = $dategroup( @tab, @tabmode, @@DT )
	    ##print @@ADJ @@VAL

    #set x = 1
    #set y = 2
  #else
    filter: ##set DT = $ref(@x)
    #if @timefld != ""
	      ##set TM = $ref(@timefld)
              ##set DT = @DT "." @TM
    #endif
	    ##set ADJ = $dategroup( @tab, @tabmode, @@DT )
	    ##print @@ADJ

    #set x = 1
  #endif
#endif
#endproc


#musthave x 

#if @NRECORDS = 0
  #exit
#endif

#proc print
label: Got @NRECORDS records, @NFIELDS fields per record.

#if @tab = "" && @y = ""
  #write stderr
    Error: 'y' is required unless 'tab' is being used.
  #endwrite noclose
  #exit
#endif


#if @tab != ""
  #proc processdata
  action: count
  #if @y = ""
    fields: @x
  #else 
    fields: @x @y
  #endif
  //showresults: yes
  #endproc

  #set x = 1
  #set y = 2

#endif


//// plot area
#include $chunk_area
// X range & Y range..
#if @xrange = ""
  xautorange: datafield=@x  nearest=@nearest
#else
  xrange: @xrange
#endif
#if @yrange = ""
  yautorange: datafields=@y incmult=2.0 
#elseif @yrange = "0"
  yautorange: datafields=@y incmult=2.0 lowfix=0
#else
  yrange: @yrange
#endif


//// Y axis..
#include $chunk_yaxis


//// X axis..
#include $chunk_xaxis
autoyears: @xyears


//// title..
#include $chunk_title


//// user pre-plot include..
#if @include1 != ""
  #include @include1
#endif


//// do curve fit..
#if @curve != ""
  #if @curve = "yes"
    #set curve = "color=pink width=0.5"
  #endif
  #proc curvefit
  xfield: @x
  yfield: @y
  linedetails: @curve
  order: @order
#endif


//// do bars or line..
#if @mode = bars
  #proc bars
  locfield: @x
  lenfield: @y
  color: @color
  outline: @outline
  #if @barwidth = line
    thinbarline: color=@color
  #elseif @barwidth != ""
    barwidth: @barwidth
  #endif
  #if @crossover != ""
    crossover: @crossover
  #endif
  clickmapurl: @clickmapurl

#elseif @mode = line
  #proc lineplot
  xfield: @x
  yfield: @y
  linedetails: @linedet
  stairstep: @step
  #if @fill != ""
    fill: @fill
  #endif
  #if @gapmissing != ""
    gapmissing: @gapmissing
  #endif
#endif

//// crossover line..
#if @crossover != ""
  #proc line
  linedetails: width=0.5
  points: min @crossover(s) max @crossover(s)
#endif


//// user post-plot include
#if @include2 != ""
  #include @include2
#endif
