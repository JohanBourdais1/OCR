#include "network/digitreconizer/network.h"
#include <stdbool.h>

int main(int argc, char *argv[])
{
    network* n = init_network();
    
    if (argc > 1 && strcmp(argv[1], "--load") == 0) {
        printf("Loading existing network...\n");
        load_network("network/digitreconizer/network_trained.dat", n);
        
        printf("\n=== Testing on sample images ===\n");
        Test(n, "network/digitreconizer/data/mnist_png/test", 0);
        //test_on10(n);
    } else if (argc > 1 && strcmp(argv[1], "--save") == 0) {
        printf("Starting fresh training...\n");
        train(n, "network/digitreconizer/data/mnist_png/train");
        printf("\n=== Saving trained network ===\n");
        save_network("network/digitreconizer/network_trained.dat", n);
        printf("\n=== Testing on sample images ===\n");
        Test(n, "network/digitreconizer/data/mnist_png/test", 0);
        //test_on10(n);
    }
    else {
        printf("Starting fresh training...\n");
        train(n, "network/digitreconizer/data/mnist_png/train");
        printf("\n=== Testing on sample images ===\n");
        Test(n, "network/digitreconizer/data/mnist_png/test", 0);
        //test_on10(n);
    }
    
    free_Network(n);
    return 0;
}