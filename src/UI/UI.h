#ifndef UI_H
#define UI_H
#include <gtk/gtk.h>
#include <err.h>
#include <unistd.h>
#include <glib.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <err.h>
#include "../pretraitement/pre.h"
#include "../canny/canny_edge.h"
#include "../carving/carving.h"
#include "../solver/sudoku.h"
#include "../network/digitreconizer/network.h"

void mainUI();

#endif

