CC=gcc
CFLAGS=-I/usr/local/include -L/usr/local/lib -lwiringPi
CXX = g++
CXXFLAGS = 'pkg-config --cflags --libs opencv4'

linetracer: linetracer.o
	$(CC) -o linetracer linetracer.o $(CFLAGS)

linetracer.o: linetracer.c
	$(CC) -c linetracer.c $(CFLAGS)

qrrecognition: qrrecognition.cpp
	$(CXX) qrrecognition.cpp -o qrrecognition $(CXXFLAGS) 

clean:
	rm -f linetracer linetracer.o
	rm -f qrrecognition
