CFLAGS = -Wall

PhotoAlbum:	PhotoAlbum.o
		gcc $(CFLAGS) -o PhotoAlbum PhotoAlbum.o

PhotoAlbum.o:	PhotoAlbum.c
		gcc $(CFLAGS) -c PhotoAlbum.c

gdb:	PhotoAlbum.c
	gcc $(CFLAGS) -g -c PhotoAlbum.c
	gcc $(CFLAGS) -g -o PhotoAlbum PhotoAlbum.o

clean:
		rm -f PhotoAlbum
		rm -f PhotoAlbum.o

clean_all:
		rm -f PhotoAlbum
		rm -f PhotoAlbum.o
		rm -f *.jpg
		rm -f index.html
		rm -f *~