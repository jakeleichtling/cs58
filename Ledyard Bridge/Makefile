CFLAGS = -Wall

LedyardBridge:	LedyardBridge.o
		gcc $(CFLAGS) -o LedyardBridge LedyardBridge.o

LedyardBridge.o:	LedyardBridge.c
			gcc $(CFLAGS) -c LedyardBridge.c

gdb:	LedyardBridge.c
	gcc $(CFLAGS) -g -c LedyardBridge.c
	gcc $(CFLAGS) -g -o LedyardBridge LedyardBridge.o

clean:
	rm -f LedyardBridge
	rm -f LedyardBridge.o
	rm -f *~
