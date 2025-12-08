#pragma once

typedef struct network
{
    int* nblayers;
    double* input_biais;
    double* input_weight ;
    double* hidden_biais;
    double* hidden_weight;
} network;

network init_network(int nblayer, int nbneuronsfirstlayer, int nbneuronshiddenlayer , int nbneuronslastlayer);

double sigmoid(double x);

double sigmoid_derivative(double x);

double* forward_propagation(network* net ,double input[],double hidden[]);

void backpropagation(network* net, double input[], double target, double learning_rate);