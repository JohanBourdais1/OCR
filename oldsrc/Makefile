# Makefile

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -O2 -g  -lSDL2 -lSDL2_image -lm  `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -w

# Source files
SOURCES = main.c solver/sudoku.c carving/carving.c network/digitreconizer/network.c UI/UI.c canny/canny_edge.c canny/hysteresis.c canny/pgm_io.c pretraitement/pre.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Executable
EXECUTABLE = ${SOURCES:.c=}

solve: main.o solver/sudoku.o network/digitreconizer/network.o carving/carving.o  UI/UI.o canny/canny_edge.o canny/hysteresis.o canny/pgm_io.o pretraitement/pre.o
	$(CC) $(CFLAGS)  main.o solver/sudoku.o network/digitreconizer/network.o carving/carving.o UI/UI.o canny/canny_edge.o canny/hysteresis.o canny/pgm_io.o pretraitement/pre.o -o solve

carvin: main.o carving/carving.o solver/sudoku.o network/digitreconizer/network.o UI/UI.o pretraitement/pre.o
	$(CC) $(CFLAGS) main.o solver/sudoku.o network/digitreconizer/network.o carving/carving.o  UI/UI.o canny/canny_edge.o canny/hysteresis.o canny/pgm_io.o pretraitement/pre.o -o carvin

solve2: main.o carving/carving.o  solver/sudoku.o network/digitreconizer/network.o UI/UI.o pretraitement/pre.o
	$(CC) $(CFLAGS) main.o solver/sudoku.o network/digitreconizer/network.o carving/carving.o  UI/UI.o canny/canny_edge.o canny/hysteresis.o canny/pgm_io.o pretraitement/pre.o -o solve2

ui: main.o carving/carving.o  solver/sudoku.o network/digitreconizer/network.o UI/UI.o pretraitement/pre.o
	$(CC) $(CFLAGS) main.o solver/sudoku.o network/digitreconizer/network.o carving/carving.o UI/UI.o canny/canny_edge.o canny/hysteresis.o canny/pgm_io.o pretraitement/pre.o -o ui

canny_edge: main.o carving/carving.o solver/sudoku.o network/digitreconizer/network.o UI/UI.o pretraitement/pre.o
	$(CC) $(CFLAGS) main.o solver/sudoku.o network/digitreconizer/network.o carving/carving.o UI/UI.o canny/canny_edge.o canny/hysteresis.o canny/pgm_io.o pretraitement/pre.o -o canny_edge

pre: main.o carving/carving.o solver/sudoku.o network/digitreconizer/network.o UI/UI.o pretraitement/pre.o
	$(CC) $(CFLAGS) main.o solver/sudoku.o network/digitreconizer/network.o carving/carving.o UI/UI.o canny/canny_edge.o canny/hysteresis.o canny/pgm_io.o pretraitement/pre.o -o pre


# Default target
all: solve carvin solve2 ui canny_edge pre

# Build target
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXECUTABLE)

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJECTS)
	rm -f solver/grid.png
	rm -f solver/*.result
	rm -f pretraitement/grid_resize.png
	rm -f solver/grid
	rm -rf network/digitreconizer/grid
	rm -f solve
	rm -f solve2
	rm -f carvin
	rm -f carving/border.png
	rm -f canny/canny_edge
	rm -f carving/ready.png
	rm -f carving/ready_*
	rm -f carving/carr.png
	rm -f *.pgm
	rm -f pre
	rm -f canny_edge

cleanall:
	rm -f $(OBJECTS)
	rm -f solver/grid.png
	rm -f solver/*.result
	rm -f carving/carr.png
	rm -f solver/grid
	rm -rf network/digitreconizer/grid
	rm -f solve
	rm -f solve2
	rm -f carvin
	rm -f carving/border.png
	rm -f canny/canny_edge
	rm -f pretraitement/grid_resize.png
	rm -f carving/ready.png
	rm -f carving/ready_*
	rm -f *.pgm
	rm -f pre
	rm -f canny_edge
	rm -f ui
	rm -f UI/*~
	rm -f *~
	rm -f UI/UI.o

