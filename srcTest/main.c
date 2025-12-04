#include "network/digitreconizer/network.h"

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

void train(network *n, char *path)
{
    double lr = 0.001f;
    int epochs = 1000;
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
        while ((entry = readdir(directory)) != NULL) {
            char* full_name = calloc(MAX_FILE_NAME_SIZE, 1);
            snprintf(full_name, MAX_FILE_NAME_SIZE, "%s/%s", path, entry->d_name);

            if (entry->d_type == DT_DIR) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    train(n, full_name);
                }
            } else {
                if (!is_image_file(entry->d_name)) {
                    continue;
                }
                printf("%s\n", full_name);
                n->inputValues = create_Input(full_name);
                double* output_filter_1 = apply_step(n, SIZE, INPUT_SIZE, n->filter_1);
                double* output_filter_2 = apply_step(n, (SIZE - 2) / 2, ((SIZE - 2) / 2) * ((SIZE - 2) / 2), n->filter_2);
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
                printf("%d    %f\n\n", pred, n->outputValues[pred]);
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
                maxpool2x2_backward(output_filter_2, 11, 11, NB_FILTER_2, dpool2, dconv2_out);
                free(dpool2);

                relu_backward_inplace(output_filter_2, dconv2_out, NB_FILTER_2*11*11);

                double *dpool1 = calloc((size_t)NB_FILTER_1 * 13 * 13, sizeof(double));
                conv2d_valid_backward(output_filter_1, 13, 13, NB_FILTER_1, dconv2_out, 11, 11, NB_FILTER_2, n->filter_2, SIZE_FILTER, dconv2_w, dconv2_b, dpool1);
                free(dconv2_out);

                double *dconv1_out = calloc((size_t)NB_FILTER_1 * 26 * 26, sizeof(double));
                maxpool2x2_backward(output_filter_1, 26, 26, NB_FILTER_1, dpool1, dconv1_out);
                free(dpool1);

                relu_backward_inplace(output_filter_1, dconv1_out, NB_FILTER_1*26*26);

                double *din = calloc((size_t)1 * SIZE * SIZE, sizeof(double));
                conv2d_valid_backward(n->inputValues, SIZE, SIZE, 1, dconv1_out, 26, 26, NB_FILTER_1, n->filter_1, SIZE_FILTER, dconv1_w, dconv1_b, din);

                for(size_t i=0;i<(size_t)NB_FILTER_1*1*SIZE_FILTER*SIZE_FILTER;i++) n->filter_1[i] -= lr * dconv1_w[i];
                for(size_t i=0;i<(size_t)NB_FILTER_1;i++) n->biais_1[i] -= lr * dconv1_b[i];
                for(size_t i=0;i<(size_t)NB_FILTER_2*NB_FILTER_1*SIZE_FILTER*SIZE_FILTER;i++) n->filter_2[i] -= lr * dconv2_w[i];
                for(size_t i=0;i<(size_t)NB_FILTER_2;i++) n->biais_2[i] -= lr * dconv2_b[i];
                for(size_t i=0;i<(size_t)HIDDEN_SIZE*MLP_SIZE;i++) n->input_weight[i/MLP_SIZE][i%MLP_SIZE] -= lr * ddense1_w[i];
                for(size_t i=0;i<(size_t)HIDDEN_SIZE;i++) n->input_biais[i] -= lr * ddense1_b[i];
                for(size_t i=0;i<(size_t)OUTPUT_SIZE*HIDDEN_SIZE;i++) n->hidden_weight[i/HIDDEN_SIZE][i%HIDDEN_SIZE] -= lr * ddense2_w[i];
                for(size_t i=0;i<(size_t)OUTPUT_SIZE;i++) n->hidden_biais[i] -= lr * ddense2_b[i];

                free(dconv1_out);
                free(din);
                free(output_filter_1);
                free(output_filter_2);
                free(n->inputValues);
                free(full_name);
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



int main()
{
    network* n = init_network();
    train(n, "network/digitreconizer/data/mnist_png/train");
    n->inputValues = create_Input("network/digitreconizer/data/6.png");
    free(apply_step(n, SIZE, INPUT_SIZE, n->filter_1));
    double* output_filter_2 = apply_step(n, (SIZE - 2) / 2, ((SIZE - 2) / 2) * ((SIZE - 2) / 2), n->filter_2);
    dense_reLU(n, output_filter_2);
    dense_softmax(n);
    print_result(n);
    free(n->inputValues);
    free_Network(n);
    free(output_filter_2);
    return 0;
}