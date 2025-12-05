#include "network/digitreconizer/network.h"
#include <stdbool.h>

#define MAX_FILE_NAME_SIZE 1024

int is_image_file(const char *filename) {
    size_t len = strlen(filename);
    if (len < 4) return 0;
    
    const char *ext = filename + len - 4;
    return (strcmp(ext, ".png") == 0 || 
            strcmp(ext, ".PNG") == 0 ||
            strcmp(ext, ".jpg") == 0 ||
            strcmp(ext, ".JPG") == 0 ||
            strcmp(ext, ".bmp") == 0 ||
            strcmp(ext, ".BMP") == 0);
}

bool check_and_print_layer(const char *name, double *arr, size_t n) {
    double maxv = -INFINITY, minv = INFINITY, sum = 0.0;
    int nancount = 0, infcount = 0;
    for (size_t i = 0; i < n; ++i) {
        double v = arr[i];
        if (isnan(v)) nancount++;
        if (isinf(v)) infcount++;
        if (v > maxv) maxv = v;
        if (v < minv) minv = v;
        sum += v;
    }
    printf("[LAYER %s] n=%zu min=%g max=%g mean=%g nan=%d inf=%d\n",
           name, n, minv, maxv, (n>0? sum / n : 0.0), nancount, infcount);
    return (nancount > 0 || infcount > 0 || fabs(maxv) > 1e6);
}

void train(network *n, char *path)
{
    double lr = 0.01f;
    int epochs = 200;
    DIR* directory = opendir(path);
    if (directory == NULL) {
        fprintf(stderr, "Can't open %s\n", path);
        return;
    }

    double *dconv1_w = calloc((size_t)NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER, sizeof(double));
    double *dconv1_b = calloc((size_t)NB_FILTER_1, sizeof(double));
    double *dconv2_w = calloc((size_t)NB_FILTER_2 * NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER, sizeof(double));
    double *dconv2_b = calloc((size_t)NB_FILTER_2, sizeof(double));
    double *ddense1_w = calloc((size_t)HIDDEN_SIZE * MLP_SIZE, sizeof(double));
    double *ddense1_b = calloc((size_t)HIDDEN_SIZE, sizeof(double));
    double *ddense2_w = calloc((size_t)OUTPUT_SIZE * HIDDEN_SIZE, sizeof(double));
    double *ddense2_b = calloc((size_t)OUTPUT_SIZE, sizeof(double));

    struct dirent* entry = NULL;
    int e = 0;
        while ((entry = readdir(directory)) != NULL && e < epochs) {
            char* full_name = calloc(MAX_FILE_NAME_SIZE, 1);
            snprintf(full_name, MAX_FILE_NAME_SIZE, "%s/%s", path, entry->d_name);

            if (entry->d_type == DT_DIR) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    train(n, full_name);
                }
                free(full_name);
            } else {
                if (!is_image_file(entry->d_name)) {
                    free(full_name);
                    continue;
                }
                printf("%s\n", full_name);
                n->inputValues = create_Input(full_name);
                memset(dconv1_w, 0, (size_t)NB_FILTER_1 * 1 * SIZE_FILTER * SIZE_FILTER * sizeof(double));
                memset(dconv1_b, 0, (size_t)NB_FILTER_1 * sizeof(double));
                memset(dconv2_w, 0, (size_t)NB_FILTER_2 * NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER * sizeof(double));
                memset(dconv2_b, 0, (size_t)NB_FILTER_2 * sizeof(double));
                memset(ddense1_w, 0, (size_t)HIDDEN_SIZE * MLP_SIZE * sizeof(double));
                memset(ddense1_b, 0, (size_t)HIDDEN_SIZE * sizeof(double));
                memset(ddense2_w, 0, (size_t)OUTPUT_SIZE * HIDDEN_SIZE * sizeof(double));
                memset(ddense2_b, 0, (size_t)OUTPUT_SIZE * sizeof(double));
                double *conv1_out = calloc(NB_FILTER_1 * 26 * 26, sizeof(double));
                double *conv2_out = calloc(NB_FILTER_2 * 11 * 11, sizeof(double));
                apply_conv(n, SIZE, NB_FILTER_1, n->filter_1, n->inputValues, n->biais_1, conv1_out);
                double* output_filter_1 = maxPool(conv1_out, 26, NB_FILTER_1);

                size_t conv1_n = (size_t)NB_FILTER_1 * (SIZE - 2) * (SIZE - 2);
                if (check_and_print_layer("conv1_out", conv1_out, conv1_n)) {
                    printf("⚠️ Explosion détectée après conv1 (file=%s)\n", full_name);
                    return;
                }

                apply_conv(n, 13, NB_FILTER_2, n->filter_2, output_filter_1, n->biais_2, conv2_out);
                double* output_filter_2 = maxPool(conv2_out, 11, NB_FILTER_2);

                size_t conv2_n = (size_t)NB_FILTER_2 * ((SIZE - 2) / 2 - 2) * ((SIZE - 2) / 2 - 2);
                if (check_and_print_layer("conv2_out", conv2_out, conv2_n)) {
                    printf("⚠️ Explosion détectée après conv2 (file=%s)\n", full_name);
                    return;
                }

                dense_reLU(n, output_filter_2);
                dense_softmax(n);
                char *tmp = strrchr(full_name, '/');
                if (tmp == NULL) tmp = full_name;
                else tmp++;
                int gt = tmp[0] - '0';
                int pred = 0;
                for(int i=1;i<OUTPUT_SIZE;i++) 
                {
                    if(n->outputValues[i] > n->outputValues[pred]) pred = i;
                }
                printf("%d    %d    %f    %d\n\n", gt, pred, n->outputValues[pred], e);
                double* dlogits = calloc(OUTPUT_SIZE, sizeof(double));
                for(int i=0;i<OUTPUT_SIZE;i++) dlogits[i] = n->outputValues[i];
                dlogits[gt] -= 1.0f;
                double *dx_dense1 = calloc(HIDDEN_SIZE, sizeof(double));
                dense_backward_2(n->hidden_weight, n->hiddenValues, dlogits, ddense2_w, ddense2_b, dx_dense1, HIDDEN_SIZE, OUTPUT_SIZE);

                relu_backward_inplace(n->hiddenValues, dx_dense1, HIDDEN_SIZE);

                double *dflat = calloc(MLP_SIZE, sizeof(double));
                dense_backward_1(n->input_weight, output_filter_2, dx_dense1, ddense1_w, ddense1_b, dflat, MLP_SIZE, HIDDEN_SIZE);

                free(dx_dense1);

                double *dpool2 = malloc((size_t)NB_FILTER_2 * 5 * 5 * sizeof(double));
                memcpy(dpool2, dflat, (size_t)MLP_SIZE * sizeof(double));
                free(dflat);

                double *dconv2_out = calloc((size_t)NB_FILTER_2 * 11 * 11, sizeof(double));
                maxpool2x2_backward(conv2_out, 11, 11, NB_FILTER_2, dpool2, dconv2_out);
                free(dpool2);

                relu_backward_inplace(conv2_out, dconv2_out, NB_FILTER_2*11*11);

                double *dpool1 = calloc((size_t)NB_FILTER_1 * 13 * 13, sizeof(double));
                conv2d_valid_backward(output_filter_1, 13, 13, NB_FILTER_1, dconv2_out, 11, 11, NB_FILTER_2, n->filter_2, SIZE_FILTER, dconv2_w, dconv2_b, dpool1);
                free(dconv2_out);

                double *dconv1_out = calloc((size_t)NB_FILTER_1 * 26 * 26, sizeof(double));
                maxpool2x2_backward(conv1_out, 26, 26, NB_FILTER_1, dpool1, dconv1_out);
                free(dpool1);

                relu_backward_inplace(conv1_out, dconv1_out, NB_FILTER_1*26*26);

                double *din = calloc((size_t)1 * SIZE * SIZE, sizeof(double));
                conv2d_valid_backward(n->inputValues, SIZE, SIZE, 1, dconv1_out, 26, 26, NB_FILTER_1, n->filter_1, SIZE_FILTER, dconv1_w, dconv1_b, din);

                for(size_t i=0;i<(size_t)NB_FILTER_1*1*SIZE_FILTER*SIZE_FILTER;i++) n->filter_1[i] -= lr * dconv1_w[i];
                for(size_t i=0;i<(size_t)NB_FILTER_1;i++) n->biais_1[i] -= lr * dconv1_b[i];
                for(size_t i=0;i<(size_t)NB_FILTER_2*NB_FILTER_1*SIZE_FILTER*SIZE_FILTER;i++) n->filter_2[i] -= lr * dconv2_w[i];
                for(size_t i=0;i<(size_t)NB_FILTER_2;i++) n->biais_2[i] -= lr * dconv2_b[i];
                for(size_t i=0;i<(size_t)HIDDEN_SIZE;i++) {
                    for(size_t j=0;j<(size_t)MLP_SIZE;j++) {
                        n->input_weight[i][j] -= lr * ddense1_w[i * MLP_SIZE + j];
                    }
                }
                for(size_t i=0;i<(size_t)HIDDEN_SIZE;i++) n->input_biais[i] -= lr * ddense1_b[i];
                for(size_t i = 0; i < (size_t)OUTPUT_SIZE; i++) {
                    for(size_t j = 0; j < (size_t)HIDDEN_SIZE; j++) {
                        n->hidden_weight[i][j] -= lr * ddense2_w[i * HIDDEN_SIZE + j];
                    }
                }
                for(size_t i=0;i<(size_t)OUTPUT_SIZE;i++) n->hidden_biais[i] -= lr * ddense2_b[i];

                free(dconv1_out);
                free(din);
                free(output_filter_1);
                free(output_filter_2);
                free(n->inputValues);
                free(full_name);
                free(dlogits);
                free(conv2_out);
                free(conv1_out);
                e++;
            }
        }
        free(dconv1_w);
        free(dconv1_b);
        free(dconv2_b);
        free(dconv2_w);
        free(ddense1_w);
        free(ddense1_b);
        free(ddense2_b);
        free(ddense2_w);
        closedir(directory);

    for (int i = 0; i < 10; i++)
    {
        char* full_name = calloc(MAX_FILE_NAME_SIZE, 1);
        snprintf(full_name, MAX_FILE_NAME_SIZE, "network/digitreconizer/data/%d.png", i);
        printf("%s\n", full_name);
        n->inputValues = create_Input(full_name);
        double *conv1_out = calloc(NB_FILTER_1 * (SIZE - 2) * (SIZE - 2), sizeof(double));
        double *conv2_out = calloc(NB_FILTER_2 * (SIZE - 2) / 2 * (SIZE - 2) / 2, sizeof(double));;
        apply_conv(n, SIZE, NB_FILTER_1, n->filter_1, n->inputValues, n->biais_1, conv1_out);
        double* output_filter_1 = maxPool(conv1_out, 26, NB_FILTER_1);
        apply_conv(n, 13, NB_FILTER_2, n->filter_2, output_filter_1, n->biais_2, conv2_out);
        double* output_filter_2 = maxPool(conv2_out, 11, NB_FILTER_2);
        dense_reLU(n, output_filter_2);
        dense_softmax(n);
        print_result(n);
        free(n->inputValues);
        free(output_filter_2);
        free(full_name);
        free(output_filter_1);
        free(conv2_out);
        free(conv1_out);
    }
}
/*

void train_Test(network *n, char *path)
{
    double lr = 0.01f;
    int epochs = 2;
    DIR* directory = opendir(path);
    if (directory == NULL) {
        fprintf(stderr, "Can't open %s\n", path);
        return;
    }

    double *dconv1_w = calloc((size_t)NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER, sizeof(double));
    double *dconv1_b = calloc((size_t)NB_FILTER_1, sizeof(double));
    double *dconv2_w = calloc((size_t)NB_FILTER_2 * NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER, sizeof(double));
    double *dconv2_b = calloc((size_t)NB_FILTER_2, sizeof(double));
    double *ddense1_w = calloc((size_t)HIDDEN_SIZE * MLP_SIZE, sizeof(double));
    double *ddense1_b = calloc((size_t)HIDDEN_SIZE, sizeof(double));
    double *ddense2_w = calloc((size_t)OUTPUT_SIZE * HIDDEN_SIZE, sizeof(double));
    double *ddense2_b = calloc((size_t)OUTPUT_SIZE, sizeof(double));

    struct dirent* entry = NULL;
    int e = 0;
        while ((entry = readdir(directory)) != NULL && e < epochs) {
            char* full_name = calloc(MAX_FILE_NAME_SIZE, 1);
            snprintf(full_name, MAX_FILE_NAME_SIZE, "%s/%s", path, entry->d_name);

            if (entry->d_type == DT_DIR) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    train_Test(n, full_name);
                }
                free(full_name);
            } else {
                if (!is_image_file(entry->d_name)) {
                    free(full_name);
                    continue;
                }
                printf("%s\n", "network/digitreconizer/data/2.png");
                n->inputValues = create_Input("network/digitreconizer/data/2.png");
                memset(dconv1_w, 0, (size_t)NB_FILTER_1 * 1 * SIZE_FILTER * SIZE_FILTER * sizeof(double));
                memset(dconv1_b, 0, (size_t)NB_FILTER_1 * sizeof(double));
                memset(dconv2_w, 0, (size_t)NB_FILTER_2 * NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER * sizeof(double));
                memset(dconv2_b, 0, (size_t)NB_FILTER_2 * sizeof(double));
                memset(ddense1_w, 0, (size_t)HIDDEN_SIZE * MLP_SIZE * sizeof(double));
                memset(ddense1_b, 0, (size_t)HIDDEN_SIZE * sizeof(double));
                memset(ddense2_w, 0, (size_t)OUTPUT_SIZE * HIDDEN_SIZE * sizeof(double));
                memset(ddense2_b, 0, (size_t)OUTPUT_SIZE * sizeof(double));
                double *conv1_out = calloc(NB_FILTER_1 * (SIZE - 2) * (SIZE - 2), sizeof(double));
                double *conv2_out = calloc(NB_FILTER_2 * ((SIZE - 2) / 2 - 2) * ((SIZE - 2) / 2 - 2), sizeof(double));
                double* output_filter_1 = apply_step(n, SIZE, NB_FILTER_1, n->filter_1, &conv1_out, n->inputValues, n->biais_1);
                double* output_filter_2 = apply_step(n, (SIZE - 2) / 2, NB_FILTER_2, n->filter_2, &conv2_out, output_filter_1, n->biais_2);
                dense_reLU(n, output_filter_2);
                dense_softmax(n);
                print_result(n);
                char *tmp = strrchr(full_name, '/');
                if (tmp == NULL) tmp = full_name;
                else tmp++;
                int gt = 2;
                int pred = 0;
                for(int i=1;i<OUTPUT_SIZE;i++) 
                {
                    if(n->outputValues[i] > n->outputValues[pred]) pred = i;
                }
                printf("%d    %d    %f    %d\n\n", gt, pred, n->outputValues[pred], e);
                double* dlogits = calloc(OUTPUT_SIZE, sizeof(double));
                for(int i=0;i<OUTPUT_SIZE;i++) dlogits[i] = n->outputValues[i];
                dlogits[gt] -= 1.0f;
                double *dx_dense1 = calloc(HIDDEN_SIZE, sizeof(double));
                dense_backward_2(n->hidden_weight, n->hiddenValues, dlogits, ddense2_w, ddense2_b, dx_dense1, HIDDEN_SIZE, OUTPUT_SIZE);

                relu_backward_inplace(n->hiddenValues, dx_dense1, HIDDEN_SIZE);

                double *dflat = calloc(MLP_SIZE, sizeof(double));
                dense_backward_1(n->input_weight, output_filter_2, dx_dense1, ddense1_w, ddense1_b, dflat, MLP_SIZE, HIDDEN_SIZE);

                free(dx_dense1);

                double *dpool2 = malloc((size_t)NB_FILTER_2 * 5 * 5 * sizeof(double));
                memcpy(dpool2, dflat, (size_t)MLP_SIZE * sizeof(double));
                free(dflat);

                double *dconv2_out = calloc((size_t)NB_FILTER_2 * 11 * 11, sizeof(double));
                maxpool2x2_backward(conv2_out, 11, 11, NB_FILTER_2, dpool2, dconv2_out);
                free(dpool2);

                relu_backward_inplace(conv2_out, dconv2_out, NB_FILTER_2*11*11);

                double *dpool1 = calloc((size_t)NB_FILTER_1 * 13 * 13, sizeof(double));
                conv2d_valid_backward(output_filter_1, 13, 13, NB_FILTER_1, dconv2_out, 11, 11, NB_FILTER_2, n->filter_2, SIZE_FILTER, dconv2_w, dconv2_b, dpool1);
                free(dconv2_out);

                double *dconv1_out = calloc((size_t)NB_FILTER_1 * 26 * 26, sizeof(double));
                maxpool2x2_backward(conv1_out, 26, 26, NB_FILTER_1, dpool1, dconv1_out);
                free(dpool1);

                relu_backward_inplace(conv1_out, dconv1_out, NB_FILTER_1*26*26);

                double *din = calloc((size_t)1 * SIZE * SIZE, sizeof(double));
                conv2d_valid_backward(n->inputValues, SIZE, SIZE, 1, dconv1_out, 26, 26, NB_FILTER_1, n->filter_1, SIZE_FILTER, dconv1_w, dconv1_b, din);

                for(size_t i=0;i<(size_t)NB_FILTER_1*1*SIZE_FILTER*SIZE_FILTER;i++) n->filter_1[i] -= lr * dconv1_w[i];
                for(size_t i=0;i<(size_t)NB_FILTER_1;i++) n->biais_1[i] -= lr * dconv1_b[i];
                for(size_t i=0;i<(size_t)NB_FILTER_2*NB_FILTER_1*SIZE_FILTER*SIZE_FILTER;i++) n->filter_2[i] -= lr * dconv2_w[i];
                for(size_t i=0;i<(size_t)NB_FILTER_2;i++) n->biais_2[i] -= lr * dconv2_b[i];
                for(size_t i=0;i<(size_t)HIDDEN_SIZE;i++) {
                    for(size_t j=0;j<(size_t)MLP_SIZE;j++) {
                        n->input_weight[i][j] -= lr * ddense1_w[i * MLP_SIZE + j];
                    }
                }
                for(size_t i=0;i<(size_t)HIDDEN_SIZE;i++) n->input_biais[i] -= lr * ddense1_b[i];
                for(size_t i=0;i<(size_t)OUTPUT_SIZE*HIDDEN_SIZE;i++) n->hidden_weight[i/HIDDEN_SIZE][i%HIDDEN_SIZE] -= lr * ddense2_w[i];
                for(size_t i=0;i<(size_t)OUTPUT_SIZE;i++) n->hidden_biais[i] -= lr * ddense2_b[i];

                double sum_abs_dd1 = 0.0, sum_abs_dd2 = 0.0;
                for (size_t i = 0; i < (size_t)HIDDEN_SIZE * MLP_SIZE; ++i) sum_abs_dd1 += fabs(ddense1_w[i]);
                for (size_t i = 0; i < (size_t)OUTPUT_SIZE * HIDDEN_SIZE; ++i) sum_abs_dd2 += fabs(ddense2_w[i]);
                double sum_abs_dconv1 = 0.0, sum_abs_dconv2 = 0.0;
                for (size_t i = 0; i < (size_t)NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER; ++i) sum_abs_dconv1 += fabs(dconv1_w[i]);
                for (size_t i = 0; i < (size_t)NB_FILTER_2 * NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER; ++i) sum_abs_dconv2 += fabs(dconv2_w[i]);

                fprintf(stderr, "GRADS NORMs: ddense1_w=%g ddense2_w=%g dconv1_w=%g dconv2_w=%g\n",
                        sum_abs_dd1, sum_abs_dd2, sum_abs_dconv1, sum_abs_dconv2);

                // print a few sample gradients and weights before update
                fprintf(stderr, "sample grads ddense2_w[0..4]= %g %g %g %g %g\n",
                        ddense2_w[0], ddense2_w[1], ddense2_w[2], ddense2_w[3], ddense2_w[4]);
                fprintf(stderr, "sample weights hidden_weight[0][0..4]= %g %g %g %g %g\n",
                        n->hidden_weight[0][0], n->hidden_weight[0][1], n->hidden_weight[0][2], n->hidden_weight[0][3], n->hidden_weight[0][4]);

                free(dconv1_out);
                free(din);
                free(output_filter_1);
                free(output_filter_2);
                free(n->inputValues);
                free(full_name);
                free(dlogits);
                free(conv2_out);
                free(conv1_out);
                e++;
            }
        }
        free(dconv1_w);
        free(dconv1_b);
        free(dconv2_b);
        free(dconv2_w);
        free(ddense1_w);
        free(ddense1_b);
        free(ddense2_b);
        free(ddense2_w);
        closedir(directory);
    }

*/

int main()
{
    network* n = init_network();
    train(n, "network/digitreconizer/data/mnist_png/train");
    free_Network(n);
    return 0;
}

/*
void diagnostic_test() {
    printf("=== DIAGNOSTIC TEST ===\n");
    
    // Charger 3 images différentes
    for (int i = 0; i < 3; i++) {
        char path[256];
        snprintf(path, 256, "network/digitreconizer/data/%d.png", i);
        
        double *input = create_Input(path);
        if (input == NULL) {
            printf("Failed to load %s\n", path);
            continue;
        }
        
        // Calculer quelques statistiques sur l'input
        double sum = 0.0, min = 1.0, max = 0.0;
        for (int j = 0; j < 784; j++) {
            sum += input[j];
            if (input[j] < min) min = input[j];
            if (input[j] > max) max = input[j];
        }
        
        printf("Image %d: mean=%.4f min=%.4f max=%.4f | First 10 pixels: ", 
               i, sum/784, min, max);
        for (int j = 0; j < 10; j++) {
            printf("%.2f ", input[j]);
        }
        printf("\n");
        
        free(input);
    }
    
    // Test du réseau
    printf("\n=== NETWORK TEST ===\n");
    network *n = init_network();
    
    // Afficher quelques poids initiaux
    printf("Sample filter_1 weights: ");
    for (int i = 0; i < 10; i++) {
        printf("%.4f ", n->filter_1[i]);
    }
    printf("\n");
    
    printf("Sample input_weight[0]: ");
    for (int i = 0; i < 10; i++) {
        printf("%.4f ", n->input_weight[0][i]);
    }
    printf("\n");
    
    // Tester forward pass avec 2 images
    for (int img = 0; img < 2; img++) {
        char path[256];
        snprintf(path, 256, "network/digitreconizer/data/%d.png", img);
        n->inputValues = create_Input(path);
        
        double *conv1_out = calloc(NB_FILTER_1 * (SIZE - 2) * (SIZE - 2), sizeof(double));
        double *conv2_out = calloc(NB_FILTER_2 * ((SIZE - 2) / 2 - 2) * ((SIZE - 2) / 2 - 2), sizeof(double));
        
        double* output_filter_1 = apply_step(n, SIZE, NB_FILTER_1, n->filter_1, &conv1_out, n->inputValues, n->biais_1);
        
        // Vérifier conv1_out AVANT maxpool
        double sum_conv1 = 0.0;
        for (int i = 0; i < NB_FILTER_1 * 26 * 26; i++) {
            sum_conv1 += conv1_out[i];
        }
        printf("\nImage %d: conv1_out sum=%.4f | output_filter_1[0..5]: ", img, sum_conv1);
        for (int i = 0; i < 40; i++) {
            printf("%.3f ", output_filter_1[i]);
        }
        
        double* output_filter_2 = apply_step(n, (SIZE - 2) / 2, NB_FILTER_2, n->filter_2, &conv2_out, output_filter_1, n->biais_2);
        dense_reLU(n, output_filter_2);
        
        printf("| hiddenValues[0..5]: ");
        for (int i = 0; i < 6; i++) {
            printf("%.3f ", n->hiddenValues[i]);
        }
        
        dense_softmax(n);
        printf("| outputs: ");
        for (int i = 0; i < 10; i++) {
            printf("%.3f ", n->outputValues[i]);
        }
        printf("\n");
        
        free(n->inputValues);
        free(output_filter_1);
        free(output_filter_2);
        free(conv1_out);
        free(conv2_out);
    }
    
    free_Network(n);
}

int main() {
    diagnostic_test();
    return 0;
}*/