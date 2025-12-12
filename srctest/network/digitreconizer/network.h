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
#include <termios.h>
#include <unistd.h>

#define INPUT_SIZE 784
#define SIZE 28
#define NB_FILTER_1 32   // Augmenté: 16→32 (meilleure capacité)
#define NB_FILTER_2 64   // Augmenté: 32→64 (plus de features)
#define SIZE_FILTER 3
#define MLP_SIZE 1600    // 64 × 5 × 5 = 1600
#define HIDDEN_SIZE 512  // Augmenté: 256→512 (plus de capacité dense)
#define OUTPUT_SIZE 10
#define MAX_FILE_NAME_SIZE 1024
#define L2_LAMBDA 0.0001  // Augmenté pour meilleure régularisation

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
    double input_weight[HIDDEN_SIZE * MLP_SIZE];
    double hidden_biais[OUTPUT_SIZE];
    double hidden_weight[OUTPUT_SIZE * HIDDEN_SIZE];
} network;

network* init_network();

void reLU(size_t nb_out, size_t input_size, double* out);

void apply_conv(int size, size_t nb_out,
                double* filter, double* input, double* biais, double *conv_out);
                
double* maxPool(double* input, int size, size_t nb_out);

void dense_reLU(network* net, double* input);

void dense_logits(network* net);

void dense_softmax(network* net);

void relu_backward_inplace(const double *out_forward, double *grad, int n);

void dense_backward_2(double *W_flat, double *x, double *dout, double *dW, double *db, double *dx, size_t N, size_t M);

void dense_backward_1(double *W_flat, double *x, double *dout, double *dW, double *db, double *dx, size_t N, size_t M);

void maxpool2x2_backward(const double *conv_out, int H_conv, int W_conv, int C, const double *dout, double *dconv_out);

void conv2d_valid_backward(const double *in, int H_in, int W_in, int C_in,
                                  const double *dout, int H_out, int W_out, int C_out,
                                  const double *w, int Kk,
                                  double *dW, double *db, double *din);

void apply_l2_regularization(double* weights, double* grad_weights, size_t size, double lr, double lambda);

int image_to_array(char* path, double** array);

int image_to_array_inverted(char* path, double** array);

double* create_Input(char* path);

double* create_Input_inverted(char* path);

void resize_image(char* path);

void invert_surface(SDL_Surface* surface);

Uint32 invert_pixel(Uint32 pixel_color, SDL_PixelFormat* format);

void print_result(network *net);

void save_network(char* path, network* net);

void load_network(char* path, network* net);

void free_Network(network* net);

// Fonction pour détecter les entrées clavier
int kbhit_nonblocking(void);

// Batch Normalization simple (normalise sans learnable params)
void batch_norm(double* data, size_t size, double epsilon);

void train(network *n, char *path);
int Test(network *n, char *path, int digit);
void test_on10(network *n);
int is_image_file(const char *filename);
int is_empty_cell(const char *path);
void create_grid(network *n, char *path);