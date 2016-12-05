all:
	g++ -std=c++0x -Wall platform.cpp -o asteroids `sdl2-config --cflags --libs`
run:
	./asteroids
