##
## Top level Makefile for ploticus and libploticus
##

DESTDIR=
BIN    =$(DESTDIR)/usr/bin
LIB    =$(DESTDIR)/usr/lib
INC    =$(DESTDIR)/usr/include

all:
	cd src; $(MAKE); $(MAKE) libploticus-static; $(MAKE) clean; \
	$(MAKE) FPIC=-fPIC libploticus-so; $(MAKE) clean;

clean:
	(cd src; $(MAKE) distclean);
	(cd pltestsuite; $(MAKE) distclean);
	rm -f configure-stamp build-stamp
	rm -f pltestsuite/out

install:
	install -d -m 755 $(BIN)
	install -m 755 src/pl $(BIN)/ploticus
	install -d -m 755 $(DESTDIR)/usr/share/man/man1
	install -m 644 man/man1/pl.1 $(DESTDIR)/usr/share/man/man1/ploticus.1
	install -d -m 755 $(LIB)
	install -m 644 src/libploticus.a $(LIB)
	install -m 644 src/libploticus.so.0.0.0 $(LIB)
	install -d -m 755 $(INC)
	install -m 644 src/libploticus.h $(INC)
