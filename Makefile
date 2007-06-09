##
## Top level Makefile for ploticus and libploticus
##

DESTDIR=
BIN    =$(DESTDIR)/usr/bin
LIB    =$(DESTDIR)/usr/lib
INC    =$(DESTDIR)/usr/include

all:
	cd src; $(MAKE); $(MAKE) clean; $(MAKE) -f Makefile_api; $(MAKE) clean -f Makefile_api; \
	$(MAKE) -f Makefile_api_so; $(MAKE) clean -f Makefile_api_so;

clean:
	(cd src; $(MAKE) distclean; $(MAKE) distclean -f Makefile_api; $(MAKE) distclean -f Makefile_api_so);
	(cd pltestsuite; $(MAKE) distclean);
	rm -f configure-stamp build-stamp
	rm -f pltestsuite/out

install:
	install -d -m 755 $(BIN)
	install -m 755 src/plgd18 $(BIN)/ploticus
	install -d -m 755 $(DESTDIR)/usr/share/man/man1
	install -m 644 man/man1/pl.1 $(DESTDIR)/usr/share/man/man1/ploticus.1
#	install -d -m 755 $(DESTDIR)/usr/share/ploticus
#	install -m 644 prefabs/* $(DESTDIR)/usr/share/ploticus/
#	install -d -m 755 $(DESTDIR)/usr/share/doc/ploticus/examples
#	install -m 644 pltestsuite/*  $(DESTDIR)/usr/share/doc/ploticus/examples
#	install -m 755 pltestsuite/run* $(DESTDIR)/usr/share/doc/ploticus/examples
#	install -m 644 pltestsuite/run_script_test.bat $(DESTDIR)/usr/share/doc/ploticus/examples
#	install -m 755 pltestsuite/testpf_* $(DESTDIR)/usr/share/doc/ploticus/examples
	install -d -m 755 $(LIB)
	install -m 644 src/libploticus.a $(LIB)
	install -m 644 src/libploticus.so.0.0.0 $(LIB)
	install -d -m 755 $(INC)
	install -m 644 src/libploticus.h $(INC)
