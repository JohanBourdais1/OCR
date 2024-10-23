#pragma once
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

#define INPUT_SIZE 784
#define HIDDEN_SIZE 450
#define OUTPUT_SIZE 10
#define LEARNING_RATE 0.01


typedef struct network
{
    double inputValues[INPUT_SIZE];
    double hiddenValues[HIDDEN_SIZE];
    double outputValues[OUTPUT_SIZE];
    double input_biais[HIDDEN_SIZE];
    double input_weight[INPUT_SIZE][HIDDEN_SIZE];
    double hidden_biais[OUTPUT_SIZE];
    double hidden_weight[HIDDEN_SIZE][OUTPUT_SIZE];
} network;

network* init_network();

void input_network(network* net, double input[784]);

double sigmoid(double x);

double sigmoid_derivative(double x);

void forward_propagation(network* net);

void backpropagation(network* net, double* target);

void image_to_array(char* path,double* array);

double** create_test(char* path);

void resize_image(char* path);

void invert_surface(SDL_Surface* surface);

Uint32 invert_pixel(Uint32 pixel_color, SDL_PixelFormat* format);

void save_network(char* path, network* net);

void load_network(char* path, network* net);

double** create_grid(char* path);

void write_grid(char* path, double** grid);

double** readGrid(network* net,double** grid);

void freeGrid(double** grid);

void freeSudoku(double** grid);

void train_network(network* net,double** test);

void freeNetwork(network* net);
