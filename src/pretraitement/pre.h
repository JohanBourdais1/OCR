#ifndef PRE_H
#define PRE_H


#include <math.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "../carving/carving.h"


SDL_Surface* rotateSurface(SDL_Surface* surface, double angle);

SDL_Surface* rotateImage(SDL_Surface* image, double angle);

SDL_Surface* load_image(const char* path);

Uint32 pixel_to_grayscale(Uint32 pixel_color, SDL_PixelFormat* format);

Uint32 pixel_to_black_and_white(Uint32 pixel_color, SDL_PixelFormat* format, int
                                threshold);

int threshold(SDL_Surface* surface,SDL_PixelFormat* format);

void surface_to_black_and_white(SDL_Surface* surface);

void carve2(SDL_Surface* source,SDL_Point* Points,SDL_Point[10][10]);

void surface_to_grayscale(SDL_Surface* surface);

void contrast_streching(SDL_Surface* surface);

double get_double_fm_argv(char* s);

double angle_detection(int tl_x,int tl_y,int tr_x,int tr_y,
		int bl_x,int bl_y,int br_x,int br_y);

SDL_Surface* padding2(SDL_Surface* source, int padding);

#endif
