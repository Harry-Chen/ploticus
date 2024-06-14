
set PLOTICUS_PREFABS=c:\unzipped\plsrc203win32\prefabs

pl -gif -prefab dist fld=1  data=data6  curve=yes  binsize=0.05 barwidth=0.08  ygrid=yes -o dist1.gif

pl -gif -prefab dist  fld=1  data=data8  cats=yes  yrange=0  stubvert=yes  barwidth=0.05  ylbl="# Hits"   order=rev  -o dist2.gif

pl  -gif -prefab pie  values=3  labels=1  data=data9  delim=tab  title="Student enrollment" -o pie1.gif 

pl  -gif -prefab chron  data=data14  x=1  y=2  datefmt=yy/mm/dd  xinc="1 month" stubfmt=M  xyears=yyyy  yrange="0 25"  barwidth=line  color=red  title="# hits per day"  omitweekends=yes -o chron1.gif

pl -svg -prefab dist fld=1  data=data6  curve=yes  binsize=0.05 barwidth=0.08  ygrid=yes -o dist1.svg

pl -svg -prefab dist  fld=1  data=data8  cats=yes  yrange=0  stubvert=yes  barwidth=0.05  ylbl="# Hits"   order=rev  -o dist2.svg

pl  -svg -prefab pie  values=3  labels=1  data=data9  delim=tab  title="Student enrollment" -o pie1.svg

pl  -svg -prefab chron  data=data14  x=1  y=2  datefmt=yy/mm/dd  xinc="1 month" stubfmt=M  xyears=yyyy  yrange="0 25"  barwidth=line  color=red  title="# hits per day"  omitweekends=yes -o chron1.svg
