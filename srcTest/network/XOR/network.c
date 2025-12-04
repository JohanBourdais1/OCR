#include <stdio.h>
#include "network.h"
#include <math.h>
#include <err.h>
#include <stdlib.h>

network init_network(int nblayer ,int nbneuronsfirstlayer, int nbneuronshiddenlayer , int nbneuronslastlayer)
{
    int* nblayers = malloc(nblayer*sizeof(int));
    if(nblayers==NULL)
	    errx(1,"failed to allocate memory");
    *(nblayers)=nbneuronsfirstlayer;
    *(nblayers+1)=nbneuronshiddenlayer;
    *(nblayers+2)=nbneuronslastlayer;
    double* input_weight = malloc(nbneuronshiddenlayer*nbneuronsfirstlayer*sizeof(double));
    if(input_weight==NULL)
            errx(1,"failed to allocate memory");
    double* input_biais = malloc((nbneuronshiddenlayer)*sizeof(double));
    if(input_biais==NULL)
            errx(1,"failed to allocate memory");
    double* hidden_weight = malloc(nbneuronshiddenlayer+nbneuronslastlayer*sizeof(double));
    if(hidden_weight==NULL)
            errx(1,"failed to allocate memory");
    double* hidden_biais = malloc(nbneuronslastlayer*sizeof(double));
    if(hidden_biais==NULL)
            errx(1,"failed to allocate memory");
    for (size_t i = 0; i < (size_t)(nbneuronshiddenlayer*nbneuronsfirstlayer); i++)
    {
        *(input_weight+i)= (2.0 * rand() / RAND_MAX) - 1.0;
    }
    for (size_t i = 0; i < (size_t)nbneuronshiddenlayer; i++)
    {
        *(input_biais+i)= (2.0 * rand() / RAND_MAX) - 1.0;
    }
    for (size_t i = 0; i < (size_t)(nbneuronshiddenlayer*nbneuronslastlayer); i++)
    {
        hidden_weight[i]= (2.0 * rand() / RAND_MAX) - 1.0;
    }
    for (size_t i = 0; i < (size_t)nbneuronslastlayer; i++)
    {
        hidden_biais[i]= (2.0 * rand() / RAND_MAX) - 1.0;
    }
    struct network net = {nblayers,input_biais,input_weight,hidden_biais,hidden_weight};
    return net;
}

double sigmoid(double x) 
{
    return 1.0 / (1.0 + exp(-x));
}

double sigmoid_derivative(double x) 
{
    double sig = sigmoid(x);
    return sig * (1.0 - sig);
}

double* forward_propagation(network* net ,double input[],double hidden[])
{
    for (int i = 0; i < net->nblayers[1]; i++)
    {
        hidden[i] = 0.0;
        for (int j = 0; j < net->nblayers[0]; j++) 
        {
            hidden[i] += input[j] * net->input_weight[j+net->nblayers[0]*i];
        }
        hidden[i] += net->input_biais[i];
        hidden[i] = sigmoid(hidden[i]);
    }
    double* output=malloc(net->nblayers[2]*sizeof(double));
    for (int i = 0; i < net->nblayers[2]; i++) 
    {
        output[i]=0.0;
        for (int j = 0; j < net->nblayers[1]; j++)
        {
            output[i] += hidden[j] * net->hidden_weight[j+net->nblayers[1]*i];
        }
        output[i] += net->hidden_biais[i];
        output[i] = sigmoid(output[i]);
    }
    return output;
}

void backpropagation(network* net, double input[], double target, double learning_rate)
{
    double hidden[net->nblayers[1]];

    double* output = forward_propagation(net ,input, hidden);

    double* error = malloc(sizeof(double)*net->nblayers[2]);
    if(error==NULL)
            errx(1,"failed to allocate memory");
    for (int i = 0; i < net->nblayers[2]; i++)
    {
        *(error+i) = target-output[i];
    }
    
    double* output_delta = malloc(sizeof(double)*net->nblayers[2]);
    if(output_delta==NULL)
            errx(1,"failed to allocate memory");
    for (int i = 0; i < net->nblayers[2]; i++)
    {
        *(output_delta+i) = error[i] * sigmoid_derivative(output[i]);
    }

    for (int i = 0; i < net->nblayers[2]; i++) 
    {
        for (int j = 0; j < net->nblayers[1]; j++)
        {
            net->hidden_weight[j+net->nblayers[1]*i] += learning_rate * output_delta[i] * hidden[j];
        }
        net->hidden_biais[i] += learning_rate * output_delta[i];
    }

    double* hidden_deltas= malloc(net->nblayers[1]*sizeof(double));
    if(hidden_deltas==NULL)
            errx(1,"failed to allocate memory");
    for (int i = 0; i < net->nblayers[1]; i++) 
    {
        for (int j = 0; j < net->nblayers[2]; j++)
        {
            hidden_deltas[i] = output_delta[j] * net->hidden_weight[i+net->nblayers[1]*j] * sigmoid_derivative(hidden[i]);
        }
    }

    for (int i = 0; i < net->nblayers[1]; i++)
    {
        for (int j = 0; j < net->nblayers[0]; j++) 
        {
            net->input_weight[i*net->nblayers[0]+j]+=learning_rate*hidden_deltas[j]*input[i];
        }
    }

    for (int i = 0; i < net->nblayers[1]; i++) {
        net->input_biais[i] += learning_rate * hidden_deltas[i];
    }
}
