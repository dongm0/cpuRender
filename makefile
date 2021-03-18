SOURCE_DIR = ./src/
INCLUDE = -I /usr/include/eigen3 -I /usr/include/SDL2 -I ./src
SHARED_LIB = -lSDL2
CXX_FLAGS = -std=c++17 -O3


cpuRenderer: rasterizer.o
	g++ src/main.cpp src/rasterizer.o -o cpuRenderer.out $(INCLUDE) $(SHARED_LIB) $(CXX_FLAGS)

rasterizer.o:
	g++ src/rasterizer.cpp -c -o src/rasterizer.o $(INCLUDE) $(SHARED_LIB) $(CXX_FLAGS)

clean:
	rm -f cpuRenderer src/rasterizer.o
