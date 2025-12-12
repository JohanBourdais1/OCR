#ifndef CARVING
#define CARVING

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <unistd.h>
#include <sys/stat.h>

SDL_Surface* slider_cut(SDL_Surface* source, int x1, int x2, int y1, int y2);

SDL_Surface* padding(SDL_Surface* source, int padding);

SDL_Surface* unpadding(SDL_Surface* source, int padding);

int finder(SDL_Surface* source, int x, int y, int outline[][11]);

SDL_Surface* carve(SDL_Surface* source, SDL_Point intersections[][10]);

SDL_Surface* image(SDL_Surface* source, SDL_Point tl, SDL_Point tr,
                   SDL_Point bl, SDL_Point br);

void cut(SDL_Surface* source, SDL_Point intersections[][10]);

#endif