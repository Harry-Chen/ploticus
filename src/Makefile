######## This Makefile will build the PL program.  To build libploticus, see Makefile_api
######## Root is not required to build ploticus.
########
######## This Makefile is READY TO GO for Linux and PNG.  For other platforms or 
######## configurations just follow the simple 3-step procedure to choose OS platform, 
######## options, etc.
########
######## Then type:  make clean; make
########
######## The executable(s) will be placed into this directory.  Move them to your desired
######## bin manually or use "make install" (which may require root)
######## 
######## After building, set environment variable PLOTICUS_PREFABS to the full pathname
######## of the ./prefabs directory.  
########
######## Unless specifically disabled herein, all ploticus builds can produce PostScript,
######## EPS, and SVG without needing any external libraries.  X11 is available by default
######## on unix, and requires xlib.  Image file output (PNG, GIF, etc) is created using
######## the GD library (see Step 2 below) which may require certain libraries.  GD code
######## is bundled with ploticus, or you can use your own copy (1.84+) if desired.
######## SWF output can be enabled if desired (see Step 3).
########
######## PLOTICUS data display engine.  Software, documentation, and examples.  
######## Copyright 1998-2004 Stephen C. Grubb  (scg@jax.org)
######## Covered by GPL; see the file ./Copyright for details. 


# defaults..
CC = gcc
PLATFORM = UNIX
RMCOM = rm -f
XOBJ = x11.o interact.o
PL = plgd18
PLPNG = 
BIN = ./
# EXE = pl


#############################################################
######## Step 1. Select an OS platform.               #######
########         All others should be commented out.  #######
#############################################################

#### For Linux uncomment the following.. #######################
XLIBS = -L/usr/X11R6/lib -lX11
INCLUDEDIRS = -I/usr/X11R6/include


#### For Solaris 2.x uncomment the following.. #################
# XLIBS = -L/usr/openwin/lib -lX11
# INCLUDEDIRS = -I/usr/openwin/include


#### For FreeBSD uncomment the following.. #####################
# XLIBS = -L/usr/X11R6/lib -lX11
# INCLUDEDIRS = -I/usr/X11R6/include -I/usr/local/include
# ADDLIBS = -L/usr/local/lib


#### For Windows/NT/MinGW uncomment the following.. ############
# PLATFORM = WIN32
# RMCOM = del
# XOBJ = 
# NOXFLAG = -DNOX11
# NORLIMFLAG = -DNORLIMIT


#### For Max OS X uncomment the following.. ######################
# CC = cc
#### with X11..
# XLIBS = -L/usr/X11R6/lib -lX11
# INCLUDEDIRS = -I/usr/X11R6/include
#### without X11..
# XLIBS =
# NOXFLAG = -DNOX11


#### For AIX 3.2.5, HP-UX 10.20 uncomment the following.. ########
# XLIBS = -L /usr/X11/lib -lX11
# INCLUDEDIRS = -I/usr/X11/include
## ..note for HP-UX it was necessary to do: ln gd_io.h gd16 by hand to get the png compiled


#### For SunOS4, Irix, other BSD uncomment the following.. #######
# XLIBS = -L/usr/lib -lX11
# INCLUDEDIRS = -I/usr/include
## ..note the following additional lines were necessary on the Irix system we used:  CC = cc    FLAGS = -o32


#### For UnixWare uncomment the following.. ######################
# XLIBS = -L/usr/X11/lib -lX11
# INCLUDEDIRS = -I/usr/X11/include
# ADDLIBS = -lnsl -lresolv -L/usr/ucblib -lucb
## ..note: the following was necessary on the UnixWare system we used: CC = cc


#### For MPE/iX uncomment the following.. ########################
# XLIBS = -L/usr/local/lib -lX11
# INCLUDEDIRS = -I/usr/local/include
# FLAGS = -D_POSIX_SOURCE
# CC = /usr/local/bin/gcc  

#### For Cygwin uncomment the following.. ########################
# PLATFORM = WIN32
# XLIBS = -L/usr/X11R6/lib -lX11
# INCLUDEDIRS = -I/usr/X11R6/include
# NORLIMFLAG = -DNORLIMIT




############################################################################
######## Step 2. GD image options                                   ########
########        Ploticus uses the GD library (www.boutell.com/gd)   ########
########	for image creation (PNG, JPEG, WBMP, pseudo-GIF).   ########
########	Several capability combinations are available.      ########
########        Choose ONE of the following...  all others should   ########
########	be commented out.                                   ########
############################################################################

#### For PNG (only) uncomment the following.. (libpng and zlib required) ###########
# ploticus: plpng 
# PLPNG = pl 
# GD16LIBS = -lpng -lz
# GD16H = 
# ZFLAG = -DWZ


#### For pseudo-GIF (only) uncomment the following..  (no libs required) ###########
# ploticus: pl 


#### For both pseudo-GIF & PNG uncomment the following..  ##########################
# ploticus: pl plpng 
# GD16LIBS = -lpng -lz
# GD16H = 
# EXE = pl plpng


#### For JPEG, WBMP, and PNG (without FreeType2), using your copy of GD 1.84+,  #####
####  uncomment the following.. (gd 1.84+, libpng, zlib, libjpeg required)      #####
# ploticus: plgd18 
# GD18LIBS = -lgd -lpng -lz -ljpeg 
# GD18H = 
# GDFREETYPE = 
# ZFLAG = -DWZ


#### For JPEG, WBMP, PNG, and FreeType2, using your copy of GD 1.84+,          ######
#### uncomment the following..  (gd 1.84+, libpng, zlib, libjpeg, libfreetype  ######
#### are all required).  Set GDFONTPATH to directory containing .ttf files     ######
ploticus: plgd18 
GD18LIBS = -lgd -lpng -lz -ljpeg -lfreetype
GD18H = 
GDFREETYPE = -DGDFREETYPE
ZFLAG = -DWZ


#### To disable all GD image support uncomment the following..  ######################
# ploticus: plnogd 
# NOGDFLAG = -DNOGD



################################################################
######## Step 3.  Some other things you might       ############
########          want control over..               ############
################################################################

#### If you plan to do a "make install", where do you want the executable(s) to be moved to?
BIN = /usr/local/bin


#### To set a hard-coded directory for prefabs files, uncomment & edit the following.. 
#### If done, users won't be required to have PLOTICUS_PREFABS environment variable defined.
#### This option not available for win32
PREFABS_DIR = "/usr/share/ploticus"


#### For LOCALE support (non-roman alphabets & collation), uncomment the following..
LOCALEOBJ = localef.o
LOCALE_FLAG = -DLOCALE


#### SVG output is always available by default.
#### If you want compressed SVG (.svgz), and you selected "GIF only", "GIF & PNG", or "disable GD" above, 
#### uncomment the following.  You need to have zlib.  If necessary change -lz to location of zlib.
ZLIB = -lz
ZFLAG = -DWZ


#### To completely remove X11 output capability from any of the above unix builds, uncomment the following..
# NOXFLAG = -DNOX11
# XLIBS = 
# XOBJ =


#### To completely remove PostScript / EPS output capability, uncomment the following..
# NOPSFLAG = -DNOPS 


#### To completely remove SVG output capability, uncomment the following..
# NOSVGFLAG = -DNOSVG 


#### If you want SWF (flash) output capability
#### uncomment the following.  You need to have ming library.  If necessary change -lming to location of ming.
#MING = -lming
# ..and if your ming.h is in a nonstandard place add a line like this:
# MINGH = -I/home/fred/lib
#### Otherwise, for no SWF capability, uncomment the following line
NOSWFFLAG = -DNOSWF 


#### To completely remove safety limits on cpu time uncomment the following..
# NORLIMFLAG = -DNORLIMIT


###############################################################################
######## Done.  Now save this file and type:  make clean; make      ###########
########                                                            ###########
###############################################################################

######## FYI... additional Makefile statement examples:
######## eg: GD16LIBS = /home/scg/lib/libpng.a /home/scg/lib/libz.a
######## eg: GD16H = -I/home/scg/lib
######## eg: GD18LIBS = -L /home/scg/lib -lgd -lpng -lz -ljpeg -lfreetype
######## eg: GD18H = -I/home/scg/lib/freetype-2.0.4/include
######## eg: GD18H = -I/home/scg/lib/gd-1.8.4 -I/home/scg/lib/jpeg-6b -I/home/scg/lib -I/home/scg/lib/freetype-2.0.4/include
######## eg: MINGH = -I/home/scg/lib

###############################################################################
###############################################################################
###############################################################################
###############################################################################

#### for basic gd build..
GD13 = grgd13.o devstuff13.o gd13.o 
GD16 = grgd16.o devstuff16.o gd16.o gd_io.o gd_io_dp.o gd_io_file.o gd_png.o 
GD = gdfontg.o gdfontl.o gdfontmb.o gdfonts.o gdfontt.o


LIBS = $(XLIBS) -lm $(ADDLIBS) 

CFLAGS = -DPLOTICUS -D$(PLATFORM) -DTDH_NOREC $(ZFLAG) $(FLAGS) $(LOCALE_FLAG) \
  $(NOGDFLAG) $(NOXFLAG) $(NOPSFLAG) $(NOSVGFLAG) $(NOSWFFLAG) $(INCLUDEDIRS) \
  $(MINGH) $(NORLIMFLAG) -DPREFABS_DIR=\"$(PREFABS_DIR)\"

OBJ = proc_annotate.o proc_areadef.o proc_axis.o proc_bars.o proc_rect.o proc_breakaxis.o \
  proc_categories.o proc_curvefit.o proc_settings.o proc_defineunits.o proc_drawcommands.o \
  proc_getdata.o proc_import.o proc_legend.o proc_legendentry.o proc_line.o proc_lineplot.o \
  proc_page.o proc_pie.o proc_print.o proc_rangebar.o proc_rangesweep.o proc_usedata.o \
  proc_scatterplot.o proc_vector.o proc_symbol.o proc_processdata.o proc_tabulate.o clickmap.o\
  plhead.o execscriptfile.o execline.o lib.o details.o parse.o units.o preliminaries.o \
  process_arg.o plvalue_subst.o cats.o fieldnames.o autorange.o select.o memstuff.o \
  color.o plg.o init.o arrow.o mark.o block.o pcode.o ps.o svg.o swf.o stub.o winscale.o lineclip.o smoothfit.o \
  condex.o dates.o dbinterface.o err.o functions.o glroutines.o reslimits.o secondaryops.o  \
  shell.o sinterp.o tdhkit.o times.o value.o value_subst.o variable.o custom.o datetime.o \
  $(LOCALEOBJ)

pl: pl.o $(OBJ) $(XOBJ) gd00files gd13files
	$(CC) pl.o $(FLAGS) $(OBJ) $(GD13) $(GD) $(XOBJ) $(LIBS) $(ZLIB) $(MING) -o $(PL)

plpng: pl.o $(OBJ) $(XOBJ) gd00files gd16files
	$(CC) pl.o $(FLAGS) $(OBJ) $(GD16) $(GD) $(XOBJ) $(GD16LIBS) $(LIBS) $(MING) -o $(PLPNG)

gd00files: $(GD)
	$(CC) -I./gd13 -I./ -c gdfontg.c gdfontl.c gdfontmb.c gdfonts.c gdfontt.c
	echo "done with gd00files" > gd00files

gd13files: 
	$(CC) -c grgd.c -DGD13 -I./gd13 -o grgd13.o
	$(CC) -c $(CFLAGS) devstuff.c -DGD13 -o devstuff13.o
	$(CC) -c gd13.c -I./gd13 
	echo "done with gd13files" > gd13files

gd16files: 
	$(CC) -c grgd.c -DGD16 -I./ -I./gd16 -o grgd16.o
	$(CC) -c $(CFLAGS) devstuff.c -DGD16 -o devstuff16.o
	$(CC) -c -I./gd16 -I./ $(GD16H) gd16.c gd_io.c gd_io_dp.c gd_io_file.c gd_png.c
	echo "done with gd16files" > gd16files

# GD18: 
plgd18: pl.o $(OBJ) $(XOBJ) gd18files
	$(CC) $(FLAGS) pl.o grgd.o devstuff.o $(OBJ) $(XOBJ) $(GD18LIBS) $(LIBS) $(MING) -o $(PL)

gd18files:
	$(CC) -c grgd.c devstuff.c -DGD18 $(GDFREETYPE) $(GD18H) $(CFLAGS)



# NOGD: 
plnogd: pl.o $(OBJ) $(XOBJ) nogdfiles
	$(CC) $(FLAGS) pl.o devstuff.o $(OBJ) $(XOBJ) $(LIBS) $(ZLIB) $(MING) -o $(PL)

nogdfiles:
	$(CC) -c devstuff.c $(CFLAGS)



install:
	mv $(EXE) $(BIN)

clean:
	$(RMCOM) *.o gd??files 

