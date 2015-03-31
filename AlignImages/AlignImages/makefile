all: clean first second
	
first:
	g++ -Wall -pthread -std=c++11 -O2 -I. -c main.cpp -o main.o
second:
	g++ -Wall -pthread -std=c++11 -O2 main.o -o HUVRStitch /usr/lib/libexiv2.so -lm -Wl,--rpath -Wl,/usr/local/lib


clean:
	rm *.o
