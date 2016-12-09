all:
	g++ -std=c++0x -Wall -Wconversion -Wdouble-promotion  platform.cpp -o asteroids `sdl2-config --cflags --libs`
run: 
	./asteroids
