comp = clang++-5.0

all:
	${comp} -std=c++11 -O3 -Wall -Wdouble-promotion platform.cpp -o asteroids `sdl2-config --cflags --libs`
run: 
	./asteroids
