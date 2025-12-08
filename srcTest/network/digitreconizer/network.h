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
#define SIZE 28
#define NB_FILTER_1 8
#define NB_FILTER_2 16
#define SIZE_FILTER 3
#define MLP_SIZE 400
#define HIDDEN_SIZE 128
#define OUTPUT_SIZE 10

typedef struct network
{
    double* inputValues;
    double filter_1[NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER];
    double biais_1[NB_FILTER_1];
    double filter_2[NB_FILTER_2 * NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER];
    double biais_2[NB_FILTER_2];
    double hiddenValues[HIDDEN_SIZE];
    double outputValues[OUTPUT_SIZE];
    double input_biais[HIDDEN_SIZE];
    double input_weight[HIDDEN_SIZE][MLP_SIZE];
    double hidden_biais[OUTPUT_SIZE];
    double hidden_weight[OUTPUT_SIZE][HIDDEN_SIZE];
} network;

network* init_network();

void reLU(size_t nb_out, size_t input_size, double* out);

void apply_conv(network* net, int size, size_t nb_out,
                double* filter, double* input, double* biais, double *conv_out);
                
double* maxPool(double* input, int size, size_t nb_out);

void dense_reLU(network* net, double* input);

void dense_logits(network* net);

void dense_softmax(network* net);

void relu_backward_inplace(const double *out_forward, double *grad, int n);

void dense_backward_2(double W[][HIDDEN_SIZE], double *x, double *dout, double *dW, double *db, double *dx, size_t N, size_t M);

void dense_backward_1(double W[][MLP_SIZE], double *x, double *dout, double *dW, double *db, double *dx, size_t N, size_t M);

void maxpool2x2_backward(const double *conv_out, int H_conv, int W_conv, int C, const double *dout, double *dconv_out);

void conv2d_valid_backward(const double *in, int H_in, int W_in, int C_in,
                                  const double *dout, int H_out, int W_out, int C_out,
                                  const double *w, int Kk,
                                  double *dW, double *db, double *din);

int image_to_array(char* path,double** array);

double* create_Input(char* path);

void resize_image(char* path);

void invert_surface(SDL_Surface* surface);

Uint32 invert_pixel(Uint32 pixel_color, SDL_PixelFormat* format);

void print_result(network *net);

void save_network(char* path, network* net);

void load_network(char* path, network* net);

double** create_grid(char* path);

void write_grid(char* path, double** grid);

double** readGrid(network* net,double** grid);

void freeGrid(double** grid);

void train_network(network* net,double** test);

void free_Network(network* net);
