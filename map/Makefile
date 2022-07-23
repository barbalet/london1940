APP=map2json
VERSION=1.00

all:
	gcc -Wall -std=c11 -pedantic -O3 -o $(APP) -Isrc src/*.c -lm

mac:
	gcc -Wall -std=c11 -pedantic -O3 -o $(APP) -Isrc src/*.c -lm -Wl,-stack_size -Wl,0x1000000

debug:
	gcc -Wall -std=c11 -pedantic -g -o $(APP) -Isrc src/*.c -lm

source:
	tar -zcvpf ../$(APP)_$(VERSION).orig.tar.gz ../$(APP)-$(VERSION) --exclude-vcs

install:
	install -m 755 $(APP) /usr/bin/$(APP)
	install -m 755 mapoutdir /usr/bin/mapoutdir
	install -m 755 combinetiles.py /usr/bin/combinetiles
	install -m 755 combinemaps.py /usr/bin/combinemaps
	install -m 755 combinemaps.py /usr/bin/heightmap
	install -m 755 allmaps /usr/bin/allmaps
	install -m 644 man/$(APP).1.gz /usr/share/man/man1/$(APP).1.gz
	install -m 644 man/combinetiles.1.gz /usr/share/man/man1/combinetiles.1.gz
	install -m 644 man/combinemaps.1.gz /usr/share/man/man1/combinemaps.1.gz
	install -m 644 man/combinemaps.1.gz /usr/share/man/man1/heightmap.1.gz
	install -m 644 man/allmaps.1.gz /usr/share/man/man1/allmaps.1.gz
	install -m 644 man/mapoutdir.1.gz /usr/share/man/man1/mapoutdir.1.gz

uninstall:
	rm -f /usr/bin/$(APP)
	rm -f /usr/bin/mapoutdir
	rm -f /usr/bin/combinetiles
	rm -f /usr/bin/combinemaps
	rm -f /usr/bin/heightmap
	rm -f /usr/bin/allmaps
	rm -f /usr/share/man/man1/$(APP).1.gz
	rm -f /usr/share/man/man1/combinetiles.1.gz
	rm -f /usr/share/man/man1/combinemaps.1.gz
	rm -f /usr/share/man/man1/heightmap.1.gz
	rm -f /usr/share/man/man1/allmaps.1.gz
	rm -f /usr/share/man/man1/mapoutdir.1.gz

clean:
	rm -f src/*.plist $(APP) .#* src/.#* result.png thresholded.png road*.png water*.png sea*.png woods*.png map.json buildings*.png lake*.png sands*.png railway_lines_stage*.png railway_tunnels_stage*.png streets*.png dark_image.png *~ src/*~ src/*.c.orig src/*.h.orig src/#* largemap.json load_test.png map.png railway.png railway.json
