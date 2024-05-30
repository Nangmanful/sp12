CC = gcc
CFLAGS = -I/usr/local/include -L/usr/local/lib -lwiringPi

CXX = g++
CXXFLAGS = $(shell pkg-config --cflags --libs opencv4)

all: linetracer qrrecognition

linetracer: linetracer.o
	$(CC) -o $@ $^ $(CFLAGS)

linetracer.o: linetracer.c
	$(CC) -c $< $(CFLAGS)

qrrecognition: qrrecognition.o
	$(CXX) -o $@ $^ $(CXXFLAGS)

qrrecognition.o: qrrecognition.cpp
	$(CXX) -c $< $(CXXFLAGS)

clean:
	rm -f linetracer linetracer.o
	rm -f qrrecognition qrrecognition.o
