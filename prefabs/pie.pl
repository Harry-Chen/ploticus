// ploticus data display engine.  Software, documentation, and examples.  
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details. 
// http://ploticus.sourceforge.net

//// PIE - pie graph

//// set defaults..
#setifnotgiven delim = ""
#setifnotgiven comment = ""
#setifnotgiven colors = "dullyellow"
#setifnotgiven explode = ""
#setifnotgiven lbldet = ""
#setifnotgiven title = ""
#setifnotgiven titledet = ""
#setifnotgiven select = ""
#setifnotgiven clickmapurl = ""


#include $chunk_read

#musthave values labels

//// do title..
#if @title != ""
  #proc annotate
  #if @CM_UNITS = 1
    location: 6.25 10.5
  #else
    location: 2.5 4.2
  #endif
  textdetails: @titledet
  text: @title
#endif

//// do pie graph..
#proc pie
#if @CM_UNITS = 1
  center: 6.25 6.25
  radius: 2.5
#else
  center: 2.5 2.5
  radius: 1
#endif
datafield: @values
labelfield: @labels
labelmode: line+label
colors: @colors
explode: @explode
textdetails: @lbldet
clickmapurl: @clickmapurl
