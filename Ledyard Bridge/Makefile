CFLAGS = -Wall -pthread
LedyardBridge:	LedyardBridge.o TestDriver.o
		gcc $(CFLAGS) -o LedyardBridge LedyardBridge.o TestDriver.o

LedyardBridge.o:	LedyardBridge.c
			gcc $(CFLAGS) -c LedyardBridge.c

TestDriver.o:	TestDriver.c
		gcc $(CFLAGS) -c TestDriver.c

gdb:	LedyardBridge.c
	gcc $(CFLAGS) -g -c LedyardBridge.c
	gcc $(CFLAGS) -g -c TestDriver.c
	gcc $(CFLAGS) -g -o LedyardBridge LedyardBridge.o TestDriver.o

clean:
	rm -f LedyardBridge
	rm -f *.o
	rm -f *~
