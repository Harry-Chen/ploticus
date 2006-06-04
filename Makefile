all:  src/pl

DESTDIR=
BIN    =$(DESTDIR)/usr/bin

src/pl:
	cd src; make

clean:
	rm -f ./src/*.o ./src/plgd18
	rm -Rf debian/ploticus
	rm -f configure-stamp build-stamp
	rm -f pltestsuite/out

install:  src/pl
	install -m 755 src/plgd18 $(BIN)/ploticus
	install -m 644 man/man1/pl.1 $(DESTDIR)/usr/share/man/man1/ploticus.1
	install -m 644 prefabs/* $(DESTDIR)/usr/share/ploticus/
	install -d -m 755 pltestsuite $(DESTDIR)/usr/share/doc/ploticus/examples
	install -m 644 pltestsuite/*  $(DESTDIR)/usr/share/doc/ploticus/examples
	install -m 755 pltestsuite/run* $(DESTDIR)/usr/share/doc/ploticus/examples
	install -m 644 pltestsuite/run_script_test.bat $(DESTDIR)/usr/share/doc/ploticus/examples
	install -m 644 pltestsuite/run_prefabs_test.bat $(DESTDIR)/usr/share/doc/ploticus/examples
	install -m 755 pltestsuite/testpf_* $(DESTDIR)/usr/share/doc/ploticus/examples
