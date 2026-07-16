APP=map2json
ENGINE=map2json-core
VERSION=1.00
PREFIX?=/usr/local
DESTDIR?=
BINDIR=$(DESTDIR)$(PREFIX)/bin
MANDIR=$(DESTDIR)$(PREFIX)/share/man/man1
LIBEXECDIR=$(DESTDIR)$(PREFIX)/libexec/london1940

all:
	gcc -Wall -std=c11 -pedantic -O3 -o $(ENGINE) -Isrc src/*.c -lm
	cp ../tools/map2json.py $(APP)
	chmod +x $(APP)

mac:
	gcc -Wall -std=c11 -pedantic -O3 -o $(APP) -Isrc src/*.c -lm -Wl,-stack_size -Wl,0x1000000

debug:
	gcc -Wall -std=c11 -pedantic -g -o $(APP) -Isrc src/*.c -lm

source:
	tar -zcvpf ../$(APP)_$(VERSION).orig.tar.gz ../$(APP)-$(VERSION) --exclude-vcs

install:
	install -d "$(BINDIR)" "$(MANDIR)" "$(LIBEXECDIR)"
	install -m 755 $(APP) "$(BINDIR)/$(APP)"
	install -m 755 $(ENGINE) "$(LIBEXECDIR)/$(ENGINE)"
	install -m 644 ../tools/map_migrate.py "$(LIBEXECDIR)/map_migrate.py"
	install -m 755 mapoutdir "$(BINDIR)/mapoutdir"
	install -m 755 combinetiles.py "$(BINDIR)/combinetiles"
	install -m 755 combinemaps.py "$(BINDIR)/combinemaps"
	install -m 755 heightmap.py "$(BINDIR)/heightmap"
	install -m 755 allmaps "$(BINDIR)/allmaps"
	install -m 644 man/$(APP).1.gz "$(MANDIR)/$(APP).1.gz"
	install -m 644 man/combinetiles.1.gz "$(MANDIR)/combinetiles.1.gz"
	install -m 644 man/combinemaps.1.gz "$(MANDIR)/combinemaps.1.gz"
	install -m 644 man/heightmap.1.gz "$(MANDIR)/heightmap.1.gz"
	install -m 644 man/allmaps.1.gz "$(MANDIR)/allmaps.1.gz"
	install -m 644 man/mapoutdir.1.gz "$(MANDIR)/mapoutdir.1.gz"

uninstall:
	rm -f "$(BINDIR)/$(APP)" "$(BINDIR)/mapoutdir" "$(BINDIR)/combinetiles" "$(BINDIR)/combinemaps" "$(BINDIR)/heightmap" "$(BINDIR)/allmaps"
	rm -f "$(MANDIR)/$(APP).1.gz" "$(MANDIR)/combinetiles.1.gz" "$(MANDIR)/combinemaps.1.gz" "$(MANDIR)/heightmap.1.gz" "$(MANDIR)/allmaps.1.gz" "$(MANDIR)/mapoutdir.1.gz"
	rm -f "$(LIBEXECDIR)/$(ENGINE)" "$(LIBEXECDIR)/map_migrate.py"
	rm -rf "$(LIBEXECDIR)/__pycache__"
	rmdir "$(LIBEXECDIR)" 2>/dev/null || true

clean:
	rm -f src/*.plist $(APP) $(ENGINE) .#* src/.#* result.png thresholded.png road*.png water*.png sea*.png woods*.png map.json buildings*.png lake*.png sands*.png railway_lines_stage*.png railway_tunnels_stage*.png streets*.png dark_image.png *~ src/*~ src/*.c.orig src/*.h.orig src/#* largemap.json load_test.png map.png railway.png railway.json
