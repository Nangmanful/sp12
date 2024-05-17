CC=gcc
CFLAGS=-I/usr/local/include -L/usr/local/lib -lwiringPi

linetracer: linetracer.o
	$(CC) -o linetracer linetracer.o $(CFLAGS)

linetracer.o: linetracer.c
	$(CC) -c linetracer.c $(CFLAGS)

clean:
	rm -f linetracer linetracer.o
