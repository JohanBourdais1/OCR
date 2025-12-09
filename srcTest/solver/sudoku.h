#pragma once

#include <stdio.h>
#include <string.h>
#include <err.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

int isValid(int** grid, int row, int col, int num);
int solvingSudoku(int** grid);
void recreateGrid(int** grid,int oldGrid[9][9]);
SDL_Surface* doubleSizeImage(SDL_Surface* image);
int** readGridFromFile(char* path);
void writeGrid(char* path, int** grid);
int isGridValid(int** grid);
