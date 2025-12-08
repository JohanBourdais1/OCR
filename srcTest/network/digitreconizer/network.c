#include "network.h"

network* init_network()
{
    srand(time(NULL));
    struct network *net = calloc(1,sizeof(struct network));
    for (size_t i = 0; i < (size_t)NB_FILTER_1; i++)
    {
        net->biais_1[i] = 0;
    }
    for (size_t i = 0; i < (size_t)NB_FILTER_2; i++)
    {
        net->biais_2[i] = 0;
    }

    double scale = sqrtf(2.0f / (SIZE_FILTER * SIZE_FILTER));
    for (size_t i = 0; i < (size_t)NB_FILTER_1; i++)
    {
        for (size_t j = 0; j < (size_t)SIZE_FILTER; j++)
        {
            for (size_t k = 0; k < (size_t)SIZE_FILTER; k++)
            {
                net->filter_1[i * SIZE_FILTER * SIZE_FILTER + j * SIZE_FILTER + k] = ((double)rand() / RAND_MAX * 2.0 - 1.0) * scale;
            }
        }
    }
    scale = sqrtf(2.0f / (SIZE_FILTER * SIZE_FILTER * NB_FILTER_1));
    for (size_t i = 0; i < (size_t)NB_FILTER_2; i++)
    {
        for (size_t l = 0; l < (size_t)NB_FILTER_1; l++)
        {
            for (size_t j = 0; j < (size_t)SIZE_FILTER; j++)
            {
                for (size_t k = 0; k < (size_t)SIZE_FILTER; k++)
                {
                    net->filter_2[i * NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER + l * SIZE_FILTER * SIZE_FILTER + j * SIZE_FILTER + k]= ((double)rand() / RAND_MAX * 2.0 - 1.0) * scale;
                }
            }
        }
    }
    
    // He initialization for dense layers
    double scale_dense1 = sqrtf(2.0f / MLP_SIZE);
    for (size_t i = 0; i < (size_t)HIDDEN_SIZE * MLP_SIZE; i++)
    {
        net->input_weight[i] = ((double)rand() / RAND_MAX * 2.0 - 1.0) * scale_dense1 * 2.0;
    }
    for (size_t i = 0; i < (size_t)HIDDEN_SIZE; i++)
    {
        net->input_biais[i] = 0.0;
    }
    
    double scale_dense2 = sqrtf(2.0f / HIDDEN_SIZE);
    for (size_t i = 0; i < (size_t)OUTPUT_SIZE * HIDDEN_SIZE; i++)
    {
        net->hidden_weight[i] = ((double)rand() / RAND_MAX * 2.0 - 1.0) * scale_dense2 * 2.0;
    }
    for (size_t i = 0; i < (size_t)OUTPUT_SIZE; i++)
    {
        net->hidden_biais[i] = 0.0;
    }
    return net;
}

void reLU(size_t nb_out, size_t input_size, double* out) 
{
    for (size_t i = 0; i < nb_out; i++)
    {
        for (size_t j = 0; j < input_size; j++)
        {
            double *p = &out[i * input_size + j];
            *p = fmax(0.0, *p);
        }
    }
}

double* maxPool(double* input, int size, size_t nb_out)
{
    int out_size = size / 2;
    double* out = calloc(nb_out * out_size * out_size, sizeof(double));
    for (size_t i = 0; i < nb_out; i++)
    {
        double *in_c = &(input[i * size * size]);
        double *out_c = &(out[i * out_size * out_size]);
        for (size_t l = 0; l < (size_t)out_size; l++)
        {
            for (size_t j = 0; j < (size_t)out_size; j++)
            {
                double a = in_c[(2 * l) * size + (2 * j)];
                double b = in_c[(2 * l) * size + (2 * j + 1)];
                double c1 = in_c[(2 * l + 1) * size + (2 * j)];
                double d = in_c[(2 * l + 1) * size + (2 * j + 1)];
                double m = a > b ? a : b;
                m = m > c1 ? m : c1;
                m = m > d ? m : d;
                out_c[l * out_size + j] = m;
            }
        }
    }
    return out;
}

void apply_conv(network* net, int size, size_t nb_out,
                double* filter, double* input, double* biais, double *conv_out)
{
    int out_size = size - 2;

    for (size_t i = 0; i < nb_out; i++)
    {
        double *kf = &(filter[i * SIZE_FILTER * SIZE_FILTER]);
        double b = biais[i];
        double* out_f = &(conv_out[i * out_size * out_size]);

        for (int l = 0; l < out_size; l++)
        {
            for (int j = 0; j < out_size; j++)
            {
                double s = b;
                for (int k = 0; k < SIZE_FILTER; k++)
                    for (int h = 0; h < SIZE_FILTER; h++)
                        s += kf[k * SIZE_FILTER + h] * input[(l + k) * size + (j + h)];

                out_f[l * out_size + j] = s;
            }
        }
    }

    reLU(nb_out, out_size * out_size, conv_out);
}

void dense_reLU(network* net, double* input)
{
    for (size_t i = 0; i < (size_t)HIDDEN_SIZE; i++)
    {
        double s = net->input_biais[i];
        for (size_t j = 0; j < (size_t)MLP_SIZE; j++)
        {
            s += net->input_weight[i * MLP_SIZE + j] * input[j];
        }
        net->hiddenValues[i] = s > 0 ? s : 0;
    }
}

// Second dense layer: hiddenValues -> logits (before softmax)
void dense_logits(network* net)
{
    for (size_t i = 0; i < (size_t)OUTPUT_SIZE; i++)
    {
        double s = net->hidden_biais[i];
        for (size_t j = 0; j < (size_t)HIDDEN_SIZE; j++)
        {
            s += net->hidden_weight[i * HIDDEN_SIZE + j] * net->hiddenValues[j];
        }
        net->outputValues[i] = s;
    }
}

// Apply softmax to logits
void dense_softmax(network *net) {
    // First clip and handle NaN/Inf
    for (int i = 0; i < OUTPUT_SIZE; ++i) {
        double v = net->outputValues[i];
        if (isnan(v) || isinf(v)) net->outputValues[i] = 0.0;
        // Clip extreme values before exp
        if (net->outputValues[i] > 700.0) net->outputValues[i] = 700.0;
        if (net->outputValues[i] < -700.0) net->outputValues[i] = -700.0;
    }

    // Subtract max for numerical stability
    double max = net->outputValues[0];
    for (int i = 1; i < OUTPUT_SIZE; ++i) if (net->outputValues[i] > max) max = net->outputValues[i];

    // Compute softmax
    double sum = 0.0;
    for (int i = 0; i < OUTPUT_SIZE; ++i) {
        net->outputValues[i] = exp(net->outputValues[i] - max);
        sum += net->outputValues[i];
    }
    
    // Normalize
    if (sum < 1e-12) sum = 1e-12;
    for (int i = 0; i < OUTPUT_SIZE; ++i) net->outputValues[i] /= sum;
}

void relu_backward_inplace(const double *out_forward, double *grad, int n){
    for(int i=0;i<n;i++) if(out_forward[i] <= 0) grad[i] = 0;
}

void dense_backward_2(double *W_flat, double *x, double *dout, double *dW, double *db, double *dx, size_t N, size_t M){
    // N = nombre d'entrées (HIDDEN_SIZE = 128)
    // M = nombre de sorties (OUTPUT_SIZE = 10)
    // Forward: y[i] = sum_j(W[i,j] * x[j]) + b[i]
    // W est (M x N) stocké linéairement
    
    // Gradient des biais et poids
    for(size_t i=0;i<M;i++){
        db[i] += dout[i];
        for(size_t j=0;j<N;j++){
            dW[i*N + j] += dout[i] * x[j];
        }
    }
    
    // Gradient des entrées
    // W est linéaire: W[i,j] = W_flat[i*N + j]
    for(size_t j=0;j<N;j++){
        double s = 0.0f;
        for(size_t i=0;i<M;i++) s += W_flat[i*N + j] * dout[i];
        dx[j] += s;
    }
}

void dense_backward_1(double *W_flat, double *x, double *dout, double *dW, double *db, double *dx, size_t N, size_t M){
    // N = nombre d'entrées (MLP_SIZE = 400)
    // M = nombre de sorties (HIDDEN_SIZE = 128)
    // Forward: y[i] = sum_j(W[i,j] * x[j]) + b[i]
    // W est (M x N) stocké linéairement
    
    // Gradient des biais et poids
    for(size_t i=0;i<M;i++){
        db[i] += dout[i];
        for(size_t j=0;j<N;j++){
            dW[i*N + j] += dout[i] * x[j];
        }
    }
    
    // Gradient des entrées
    // W est linéaire: W[i,j] = W_flat[i*N + j]
    for(size_t j=0;j<N;j++){
        double s = 0.0f;
        for(size_t i=0;i<M;i++) s += W_flat[i*N + j] * dout[i];
        dx[j] += s;
    }
}

void maxpool2x2_backward(const double *conv_out, int H_conv, int W_conv, int C, const double *dout, double *dconv_out){
    int H_p = H_conv/2, W_p = W_conv/2;
    for(int i=0;i<C*H_conv*W_conv;i++) dconv_out[i] = 0.0f;
    for(int c=0;c<C;c++){
        for(int x=0;x<H_p;x++){
            for(int y=0;y<W_p;y++){
                int gx = 2*x, gy = 2*y;
                double a = conv_out[c*H_conv*W_conv + (gx+0)*W_conv + (gy+0)];
                double b = conv_out[c*H_conv*W_conv + (gx+0)*W_conv + (gy+1)];
                double c1 = conv_out[c*H_conv*W_conv + (gx+1)*W_conv + (gy+0)];
                double d = conv_out[c*H_conv*W_conv + (gx+1)*W_conv + (gy+1)];
                double m = a; int mi=0,mj=0;
                if(b>m){ m=b; mi=0; mj=1; }
                if(c1>m){ m=c1; mi=1; mj=0; }
                if(d>m){ m=d; mi=1; mj=1; }
                dconv_out[c*H_conv*W_conv + (gx+mi)*W_conv + (gy+mj)] += dout[c*H_p*W_p + x*W_p + y];
            }
        }
    }
}

void conv2d_valid_backward(const double *in, int H_in, int W_in, int C_in,
                                  const double *dout, int H_out, int W_out, int C_out,
                                  const double *w, int Kk,
                                  double *dW, double *db, double *din)
{
    for(int f=0; f<C_out; f++){
        double s=0;
        const double *dout_f = dout + (size_t)f * H_out * W_out;
        for(int x=0;x<H_out;x++) for(int y=0;y<W_out;y++) s += dout_f[x*W_out + y];
        db[f] += s;
    }
    for(int f=0; f<C_out; f++){
        double *dWf = dW + (size_t)f * C_in * Kk * Kk;
        const double *dout_f = dout + (size_t)f * H_out * W_out;
        for(int c=0;c<C_in;c++){
            const double *in_c = in + (size_t)c * H_in * W_in;
            double *dWfc = dWf + (size_t)c * Kk * Kk;
            for(int i=0;i<Kk;i++){
                for(int j=0;j<Kk;j++){
                    double sum = 0.0f;
                    for(int x=0;x<H_out;x++){
                        for(int y=0;y<W_out;y++){
                            sum += in_c[(x+i)*W_in + (y+j)] * dout_f[x*W_out + y];
                        }
                    }
                    dWfc[i*Kk + j] += sum;
                }
            }
        }
    }

    for(int c=0;c<C_in;c++){
        double *din_c = din + (size_t)c * H_in * W_in;
        for(int x=0;x<H_in;x++) for(int y=0;y<W_in;y++) din_c[x*W_in + y] += 0.0f;
    }
    for(int f=0; f<C_out; f++){
        const double *wf = w + (size_t)f * C_in * Kk * Kk;
        const double *dout_f = dout + (size_t)f * H_out * W_out;
        for(int x=0;x<H_out;x++){
            for(int y=0;y<W_out;y++){
                double dout_val = dout_f[x*W_out + y];
                for(int c=0;c<C_in;c++){
                    const double *wf_c = wf + (size_t)c * Kk * Kk;
                    double *din_c = din + (size_t)c * H_in * W_in;
                    for(int i=0;i<Kk;i++){
                        for(int j=0;j<Kk;j++){
                            din_c[(x+i)*W_in + (y+j)] += wf_c[i*Kk + j] * dout_val;
                        }
                    }
                }
            }
        }
    }
}

double* create_Input(char* path)
{
    double* input = malloc(784 * sizeof(double));
    if (input == NULL) {
        fprintf(stderr, "Memory allocation failed for input\n");
        return NULL;
    }
    
    FILE *test = fopen(path, "r");
    if (test == NULL) {
        fprintf(stderr, "File not found: %s\n", path);
        free(input);
        return NULL;
    }
    fclose(test);
    
    if (image_to_array(path, &input) != 0) {
        fprintf(stderr, "Failed to convert image: %s\n", path);
        free(input);
        return NULL;
    }
    
    return input;
}

int image_to_array(char* path, double** array)
{
    double* input = *array;
    
    SDL_Surface* s = IMG_Load(path);
    if (s == NULL) {
        fprintf(stderr, "IMG_Load failed for %s: %s\n", path, SDL_GetError());
        return -1;
    }
    
    SDL_Surface *image = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGB888, 0);
    SDL_FreeSurface(s);
    if (image == NULL) {
        fprintf(stderr, "SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError());
        return -1;
    }
    
    SDL_Surface* resized = SDL_CreateRGBSurface(0, 28, 28, 32, 0, 0, 0, 0);
    if (resized == NULL) {
        fprintf(stderr, "SDL_CreateRGBSurface failed: %s\n", SDL_GetError());
        SDL_FreeSurface(image);
        return -1;
    }
    
    SDL_BlitScaled(image, NULL, resized, NULL);
    SDL_FreeSurface(image);
    
    SDL_LockSurface(resized);
    Uint8 r, g, b;
    Uint32* pixels = resized->pixels;
    for (size_t i = 0; i < 784; i++) {
        SDL_GetRGB(pixels[i], resized->format, &r, &g, &b);
        input[i] = (double)(r + g + b) / 3.0 / 255.0;
    }
    SDL_UnlockSurface(resized);
    SDL_FreeSurface(resized);
    
    return 0;
}

void invert_surface(SDL_Surface* surface)
{
    Uint32* pixels = surface->pixels;
    size_t len = surface->w * surface->h;
    SDL_PixelFormat* format = surface->format;

    if (SDL_LockSurface(surface) < 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    for (size_t i = 0; i < len; i++) {
        pixels[i] = invert_pixel(pixels[i], format);
    }

    SDL_UnlockSurface(surface);
}

// Invert the colors of a pixel
Uint32 invert_pixel(Uint32 pixel_color, SDL_PixelFormat* format)
{
    Uint8 r, g, b;
    SDL_GetRGB(pixel_color, format, &r, &g, &b);
    return SDL_MapRGB(format, 255 - r, 255 - g, 255 - b);
}

void print_result(network *net)
{
    for (size_t i = 0; i < 10; i++) {
        printf("%ld : %f\n", i, net->outputValues[i]);
    }
    printf("\n");
}
/*
void save_network(char* path,network* net)
{
    FILE* file = fopen(path,"wb");
    if (file == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier\n");
        exit(1);
    }
    fwrite(net,sizeof(network),1,file);
    fclose(file);
}

void load_network(char* path, network* net)
{
    FILE* file = fopen(path,"rb");
    if (file == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier\n");
        exit(1);
    }
    int a = fread(net,sizeof(network),1,file);
    if (a != 1)
    {
        printf("Erreur lors de la lecture du fichier\n");
        exit(1);
    }
    fclose(file);
}

double** create_grid(char* path)
{
    double** input = calloc(81,sizeof(double*));
    for (size_t i = 0; i < 9; i++)
    {
        for (size_t j = 0; j < 9; j++)
        {
            input[i*9+j] = malloc(784*sizeof(double));
            char* path_image = malloc(100*sizeof(char));
            sprintf(path_image,"%s/tile_%ld%ld.png",path,i,j);
            image_to_array(path_image,input[i*9+j]);
            free(path_image);
        }
    }
    return input;
}

void write_grid(char* path, double** grid)
{
    FILE* file = fopen(path,"w");
    if (file == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier\n");
        exit(1);
    }
    for (size_t i = 0; i < 9; i++)
    {
        for (size_t j = 0; j < 9; j++)
        {
            if (grid[i][j] == 0)
            {
                fprintf(file,".");
            }
            else
            {
                fprintf(file,"%ld",(size_t)grid[i][j]);
            }
            if (j == 2 || j == 5)
            {
                fprintf(file,"  ");
            }
            
        }
        fprintf(file,"\n");
        if (i == 2 || i == 5)
        {
            fprintf(file,"\n");
        }
    }
    fclose(file);
}

double** readGrid(network* net,double** grid)
{
    double** gridres = malloc(9*sizeof(double*));
    for (size_t i = 0; i < 9; i++)
    {
        gridres[i] = malloc(9*sizeof(double));
    }
    for (size_t i = 0; i < 81; i++)
    {
        input_network(net,grid[i]);
        forward_propagation(net);
        double max = 0;
        size_t index = 0;
        for (size_t k = 0; k < 10; k++)
        {
            //printf("%f ",net.outputValues[k]);
            if (net->outputValues[k] > max)
            {
                max = net->outputValues[k];
                index = k;
            }
        }
        //printf("\n");
        gridres[i/9][i%9] = index;
    }
    return gridres;
}

void freeGrid(double** grid)
{
    for (size_t i = 0; i < 81; i++)
    {
        free(grid[i]);
    }
    free(grid);
}

void train_network(network* net, double** test)
{
    DIR* rep = opendir("network/digitreconizer/traindata1");
    if (rep == NULL)
    {
        printf("Erreur lors de l'ouverture du dossier\n");
        exit(1);
    }
    double acc = 0;
    int epoch = 0;
    while (acc < 1)
    {
        struct dirent* ent;
        while ((ent = readdir(rep)) != NULL)
        {
            if (ent->d_name[0] != '.')
            {
                char* path_image = malloc(1024*sizeof(char));
                sprintf(path_image,"network/digitreconizer/traindata1/%s",ent->d_name);
                double target[10] = {0};
                target[ent->d_name[0]-'0'] = 1;
                double input[784];
                image_to_array(path_image,input);
                input_network(net,input);
                backpropagation(net,target);
                free(path_image);
            }
        }
        rewinddir(rep);
        
        printf("Epoch : %d : ",epoch);
        double nb_true = 0;
        for(size_t k = 0; k < 10; k++) 
        {
            for (int i = 0; i < 5; i++)
            {
                input_network(net,test[i+k*5]);
                forward_propagation(net);
                double max = 0;
                for (size_t j = 0; j < 10; j++)
                {
                    if (net->outputValues[j] > max)
                    {
                        max = net->outputValues[j];
                    }
                }
                if(max == net->outputValues[k])
                {
                    nb_true++;
                }
            }
        }
        acc = nb_true/50;
        printf("Accuracy : %f \n", acc);
        epoch++;
    }
    closedir(rep);
}
*/
void free_Network(network* net)
{
    free(net);
}
