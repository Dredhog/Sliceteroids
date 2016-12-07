all:
	g++ -std=c++0x -Wall *.cpp -o asteroids `sdl2-config --cflags --libs`
run:
	./asteroids
