all:  src/pl

DESTDIR=
BIN    =$(DESTDIR)/usr/bin

src/pl: 
	cd src; make

clean:
	rm -f ./src/*.o ./src/pl
	rm -Rf debian/ploticus
	rm -f configure.stamp build.stamp

install:  src/pl
	install -m 755 src/pl $(BIN)/ploticus
	install -m 644 man/man1/pl.1 $(DESTDIR)/usr/share/man/man1/ploticus.1
	install -m 644 prefabs/* $(DESTDIR)/usr/share/ploticus/
