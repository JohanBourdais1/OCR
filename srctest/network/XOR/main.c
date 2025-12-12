#include <stdio.h>
#include "network.h"
#include <math.h>
#include <stdlib.h>

int main() 
{
    struct network net = init_network(3,2,3,1);
    
    double training_data[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    double target_data[4] = {0, 1, 1, 0};
    double hidden[3]; 
    double learning_rate = 1;

    int epochs = 100000;
    for (int epoch = 0; epoch < epochs; epoch++) 
    {
        for (int i = 0; i < 4; i++) 
        {
            backpropagation(&net,training_data[i], target_data[i],learning_rate);
        }
    }

    for (int i = 0; i < 4; i++) 
    {
        double* output = forward_propagation(&net,training_data[i],hidden);
        printf("Input: %d %d, Output: %f, Target: %f\n", (int)training_data[i][0], (int)training_data[i][1], output[0], target_data[i]);
    }
    return 0;
}
