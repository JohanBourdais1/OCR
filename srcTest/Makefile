# Makefile

# Compiler
CC = gcc

# ASAN configuration
export ASAN_OPTIONS=suppressions=asan_suppressions.txt:allocator_may_return_null=1:halt_on_error=0

# Compiler flags
CFLAGS = -Wall -Wextra -O0 -g `pkg-config --cflags gtk+-3.0` -fsanitize=address -fno-omit-frame-pointer
LDFLAGS = `pkg-config --libs gtk+-3.0`
LDLIBS = -lSDL2 -lSDL2_image -lm -fsanitize=address

# Source files
SOURCES = main.c network/digitreconizer/network.c UI/UI.c pretraitement/pre.c canny/canny_edge.c canny/hysteresis.c canny/pgm_io.c carving/carving.c solver/sudoku.c

# Object files
OBJECTS = $(SOURCES:.c=.o)

# Executable
EXECUTABLE = ${SOURCES:.c=}

# Build target
main: $(OBJECTS)
	$(CC) $^ $(LDFLAGS) $(LDLIBS) -o main

# Compile source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target

clean:
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
	rm -f main
	rm -f network/digitreconizer/network1.txt
	rm -rf grid/
	rm -f border.png
	rm -f ../assets/ready.pgm
	rm -f sudoku_grid.txt
	rm -f solver/sudoku_solution.txt

