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

void apply_conv(int size, size_t nb_out,
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

void dense_softmax(network *net) {
    for (int i = 0; i < OUTPUT_SIZE; ++i) {
        double v = net->outputValues[i];
        if (isnan(v) || isinf(v)) net->outputValues[i] = 0.0;
        if (net->outputValues[i] > 700.0) net->outputValues[i] = 700.0;
        if (net->outputValues[i] < -700.0) net->outputValues[i] = -700.0;
    }

    double max = net->outputValues[0];
    for (int i = 1; i < OUTPUT_SIZE; ++i) if (net->outputValues[i] > max) max = net->outputValues[i];

    double sum = 0.0;
    for (int i = 0; i < OUTPUT_SIZE; ++i) {
        net->outputValues[i] = exp(net->outputValues[i] - max);
        sum += net->outputValues[i];
    }
    
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

double* create_Input_inverted(char* path)
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
    
    if (image_to_array_inverted(path, &input) != 0) {
        fprintf(stderr, "Failed to convert image: %s\n", path);
        free(input);
        return NULL;
    }
    
    return input;
}

// Crop image by removing padding from all sides
SDL_Surface* crop_image(SDL_Surface* image)
{
    if (image == NULL) return NULL;
    
    int width = image->w;
    int height = image->h;
    
    // Instead of cropping, fill borders with black to remove noise
    int border = 10;  // 10 pixels border to paint white
    
    SDL_LockSurface(image);
    Uint32* pixels = (Uint32*)image->pixels;
    
    // Fill top and bottom borders
    for (int y = 0; y < border; y++) {
        for (int x = 0; x < width; x++) {
            pixels[y * width + x] = SDL_MapRGB(image->format, 255, 255, 255);  // Top
            pixels[(height - 1 - y) * width + x] = SDL_MapRGB(image->format, 255, 255, 255);  // Bottom
        }
    }
    
    // Fill left and right borders
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < border; x++) {
            pixels[y * width + x] = SDL_MapRGB(image->format, 255, 255, 255);  // Left
            pixels[y * width + (width - 1 - x)] = SDL_MapRGB(image->format, 255, 255, 255);  // Right
        }
    }
    
    SDL_UnlockSurface(image);
    
    return image;
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

int image_to_array_inverted(char* path, double** array)
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
    
    // Save original image for debug
    IMG_SavePNG(image, "debug/debug_original.png");
    
    // Crop the image to remove white borders
    SDL_Surface* cropped = crop_image(image);
    if (cropped != image) {
        SDL_FreeSurface(image);
        image = cropped;
    }
    
    // Save cropped image for debug
    IMG_SavePNG(image, "debug/debug_cropped.png");
    
    SDL_Surface* resized = SDL_CreateRGBSurface(0, 28, 28, 32, 0, 0, 0, 0);
    if (resized == NULL) {
        fprintf(stderr, "SDL_CreateRGBSurface failed: %s\n", SDL_GetError());
        SDL_FreeSurface(image);
        return -1;
    }
    
    SDL_BlitScaled(image, NULL, resized, NULL);
    SDL_FreeSurface(image);
    
    // Save resized image for debug
    IMG_SavePNG(resized, "debug/debug_resized.png");
    
    SDL_LockSurface(resized);
    
    Uint8 r, g, b;
    Uint32* pixels_rgb = resized->pixels;
    Uint8 gray_data[784];
    
    for (size_t i = 0; i < 784; i++) {
        SDL_GetRGB(pixels_rgb[i], resized->format, &r, &g, &b);
        // Convert to grayscale
        Uint8 gray = (r + g + b) / 3;
        // Invert: black becomes white, white becomes black
        gray = 255 - gray;
        
        // No thresholding - keep gradient information like MNIST
        // Just a very low threshold to remove noise floor
        gray = (gray < 30) ? 0 : gray;
        
        gray_data[i] = gray;
        // Normalize to [0, 1] for neural network
        input[i] = (double)gray / 255.0;
    }
    
    SDL_UnlockSurface(resized);
    
    // Save inverted grayscale image as png for debug
    SDL_Surface* debug_surface = SDL_CreateRGBSurface(0, 28, 28, 32, 0, 0, 0, 0);
    if (debug_surface != NULL) {
        SDL_LockSurface(debug_surface);
        Uint32* debug_pixels = debug_surface->pixels;
        for (size_t i = 0; i < 784; i++) {
            Uint8 gray = gray_data[i];
            debug_pixels[i] = SDL_MapRGB(debug_surface->format, gray, gray, gray);
        }
        SDL_UnlockSurface(debug_surface);
        char array[256];
        sprintf(array, "debug/debug_inverted%c%c.png", path[10], path[11]);
        IMG_SavePNG(debug_surface, array);
        SDL_FreeSurface(debug_surface);
    }
    
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

// Check if a cell image is empty (mostly white pixels)
int is_empty_cell(const char *path) {
    SDL_Surface* s = IMG_Load(path);
    if (s == NULL) {
        fprintf(stderr, "IMG_Load failed for %s: %s\n", path, SDL_GetError());
        return 1; // Consider it empty if we can't load it
    }
    
    SDL_Surface *image = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGB888, 0);
    SDL_FreeSurface(s);
    if (image == NULL) {
        fprintf(stderr, "SDL_ConvertSurfaceFormat failed: %s\n", SDL_GetError());
        return 1;
    }
    
    SDL_LockSurface(image);
    Uint8 r, g, b;
    Uint32* pixels = image->pixels;
    int width = image->w;
    int height = image->h;
    
    // Define the center region (ignore borders)
    int margin_x = width / 5;   // Ignore 20% on each side
    int margin_y = height / 5;  // Ignore 20% on top and bottom
    int start_x = margin_x;
    int end_x = width - margin_x;
    int start_y = margin_y;
    int end_y = height - margin_y;
    
    int center_pixels = 0;
    int dark_pixels = 0;
    
    // Count dark pixels in the center region (where a digit would be)
    for (int y = start_y; y < end_y; y++) {
        for (int x = start_x; x < end_x; x++) {
            int index = y * width + x;
            SDL_GetRGB(pixels[index], image->format, &r, &g, &b);
            int avg = (r + g + b) / 3;
            center_pixels++;
            // Consider a pixel dark if average is < 128
            if (avg < 128) {
                dark_pixels++;
            }
        }
    }
    
    SDL_UnlockSurface(image);
    SDL_FreeSurface(image);
    
    // If less than 5% of center pixels are dark, consider the cell empty
    if (center_pixels == 0) return 1;
    double dark_ratio = (double)dark_pixels / (double)center_pixels;
    
    // Debug output
    printf("Cell %s: dark_ratio = %.2f%% (%d/%d dark pixels)\n", 
           path, dark_ratio * 100, dark_pixels, center_pixels);
    
    return (dark_ratio < 0.3) ? 1 : 0;
}

void save_network(char* path, network* net)
{
    FILE* file = fopen(path, "wb");
    if (file == NULL)
    {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier %s\n", path);
        exit(1);
    }
    
    fwrite(net->filter_1, sizeof(double), NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER, file);
    fwrite(net->biais_1, sizeof(double), NB_FILTER_1, file);
    fwrite(net->filter_2, sizeof(double), NB_FILTER_2 * NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER, file);
    fwrite(net->biais_2, sizeof(double), NB_FILTER_2, file);
    fwrite(net->input_weight, sizeof(double), HIDDEN_SIZE * MLP_SIZE, file);
    fwrite(net->input_biais, sizeof(double), HIDDEN_SIZE, file);
    fwrite(net->hidden_weight, sizeof(double), OUTPUT_SIZE * HIDDEN_SIZE, file);
    fwrite(net->hidden_biais, sizeof(double), OUTPUT_SIZE, file);
    
    fclose(file);
    printf("Network saved to %s\n", path);
}

void load_network(char* path, network* net)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier %s\n", path);
        exit(1);
    }
    
    int i = fread(net->filter_1, sizeof(double), NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER, file);
    i = fread(net->biais_1, sizeof(double), NB_FILTER_1, file);
    i = fread(net->filter_2, sizeof(double), NB_FILTER_2 * NB_FILTER_1 * SIZE_FILTER * SIZE_FILTER, file);
    i = fread(net->biais_2, sizeof(double), NB_FILTER_2, file);
    i = fread(net->input_weight, sizeof(double), HIDDEN_SIZE * MLP_SIZE, file);
    i = fread(net->input_biais, sizeof(double), HIDDEN_SIZE, file);
    i = fread(net->hidden_weight, sizeof(double), OUTPUT_SIZE * HIDDEN_SIZE, file);
    i = fread(net->hidden_biais, sizeof(double), OUTPUT_SIZE, file);

    i++;
    
    fclose(file);
    printf("Network loaded from %s\n", path);
}

void test_on10(network *n)
{
    for (int i = 0; i < 10; i++)
    {
        char* full_name = calloc(MAX_FILE_NAME_SIZE, 1);
        snprintf(full_name, MAX_FILE_NAME_SIZE, "../data/mnist_png/%d.png", i);
        printf("%s\n", full_name);
        n->inputValues = create_Input(full_name);
        double *conv1_out = calloc(NB_FILTER_1 * (SIZE - 2) * (SIZE - 2), sizeof(double));
        double *conv2_out = calloc(NB_FILTER_2 * (SIZE - 2) / 2 * (SIZE - 2) / 2, sizeof(double));;
        apply_conv(SIZE, NB_FILTER_1, n->filter_1, n->inputValues, n->biais_1, conv1_out);
        double* output_filter_1 = maxPool(conv1_out, 26, NB_FILTER_1);
        apply_conv(13, NB_FILTER_2, n->filter_2, output_filter_1, n->biais_2, conv2_out);
        double* output_filter_2 = maxPool(conv2_out, 11, NB_FILTER_2);
        dense_reLU(n, output_filter_2);
        
        dense_logits(n);
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

int Test(network *n, char *path, int digit)
{
    DIR* directory = opendir(path);
    if (directory == NULL) {
        fprintf(stderr, "Can't open %s\n", path);
        return -1;
    }

    struct dirent* entry = NULL;
    int res = 0;
        while ((entry = readdir(directory)) != NULL) {
            char* full_name = calloc(MAX_FILE_NAME_SIZE, 1);
            snprintf(full_name, MAX_FILE_NAME_SIZE, "%s/%s", path, entry->d_name);

            if (entry->d_type == DT_DIR) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    int result = Test(n, full_name, entry->d_name[0] - '0');
                    switch (entry->d_name[0] - '0')
                    {
                        case 0:
                            printf("%d    prediction = %d/%d\n", entry->d_name[0] - '0', result, 1000);
                            break;
                        case 1:
                            printf("%d    prediction = %d/%d\n", entry->d_name[0] - '0', result, 1000);
                            break;
                        case 2:
                            printf("%d    prediction = %d/%d\n", entry->d_name[0] - '0', result, 1000);
                            break;
                        case 3:
                            printf("%d    prediction = %d/%d\n", entry->d_name[0] - '0', result, 1000);
                            break;
                        case 4:
                            printf("%d    prediction = %d/%d\n", entry->d_name[0] - '0', result, 1000);
                            break;
                        case 5:
                            printf("%d    prediction = %d/%d\n", entry->d_name[0] - '0', result, 1000);
                            break;
                        case 6:
                            printf("%d    prediction = %d/%d\n", entry->d_name[0] - '0', result, 1000);
                            break;
                        case 7:
                            printf("%d    prediction = %d/%d\n", entry->d_name[0] - '0', result, 1000);
                            break;
                        case 8:
                            printf("%d    prediction = %d/%d\n", entry->d_name[0] - '0', result, 1000);
                            break;
                        case 9:
                            printf("%d    prediction = %d/%d\n", entry->d_name[0] - '0', result, 1000);
                            break;
                    }
                    
                }
                free(full_name);
            } else {
                if (!is_image_file(entry->d_name)) {
                    free(full_name);
                    continue;
                }
                n->inputValues = create_Input(full_name);
                double *conv1_out = calloc(NB_FILTER_1 * 26 * 26, sizeof(double));
                double *conv2_out = calloc(NB_FILTER_2 * 11 * 11, sizeof(double));
                apply_conv(SIZE, NB_FILTER_1, n->filter_1, n->inputValues, n->biais_1, conv1_out);
                double* output_filter_1 = maxPool(conv1_out, 26, NB_FILTER_1);
                apply_conv(13, NB_FILTER_2, n->filter_2, output_filter_1, n->biais_2, conv2_out);
                double* output_filter_2 = maxPool(conv2_out, 11, NB_FILTER_2);
                dense_reLU(n, output_filter_2);
                dense_logits(n);
                dense_softmax(n);
                int pred = 0;
                for(int i=1;i<OUTPUT_SIZE;i++) 
                {
                    if(n->outputValues[i] > n->outputValues[pred]) pred = i;
                }
                if (digit == pred)
                    res++;
                free(output_filter_2);
                free(full_name);
                free(output_filter_1);
                free(conv2_out);
                free(conv1_out);
                free(n->inputValues);
            }
        }
        closedir(directory);
    return res;
}

// L2 regularization update
void apply_l2_regularization(double* weights, double* grad_weights, size_t size, double lr, double lambda)
{
    for (size_t i = 0; i < size; i++) {
        weights[i] -= lr * (grad_weights[i] + lambda * weights[i]);
    }
}

void create_grid(network *n, char *path)
{
    DIR* directory = opendir(path);
    if (directory == NULL) {
        fprintf(stderr, "Can't open %s\n", path);
        return;
    }
    int grid[9][9] = {0};
    struct dirent* entry = NULL;
        while ((entry = readdir(directory)) != NULL) {
            char* full_name = calloc(MAX_FILE_NAME_SIZE, 1);
            snprintf(full_name, MAX_FILE_NAME_SIZE, "%s%s", path, entry->d_name);

            if (entry->d_type == DT_DIR) {
                free(full_name);
            } else {
                if (!is_image_file(entry->d_name)) {
                    free(full_name);
                    continue;
                }
                
                n->inputValues = create_Input_inverted(full_name);
                double *conv1_out = calloc(NB_FILTER_1 * 26 * 26, sizeof(double));
                double *conv2_out = calloc(NB_FILTER_2 * 11 * 11, sizeof(double));
                apply_conv(SIZE, NB_FILTER_1, n->filter_1, n->inputValues, n->biais_1, conv1_out);
                double* output_filter_1 = maxPool(conv1_out, 26, NB_FILTER_1);
                apply_conv(13, NB_FILTER_2, n->filter_2, output_filter_1, n->biais_2, conv2_out);
                double* output_filter_2 = maxPool(conv2_out, 11, NB_FILTER_2);
                dense_reLU(n, output_filter_2);
                dense_logits(n);
                dense_softmax(n);
                int pred = 0;
                for(int i=1;i<OUTPUT_SIZE;i++) 
                {
                    if(n->outputValues[i] > n->outputValues[pred]) pred = i;
                }
                printf("Predicted digit for cell [%c][%c]: %f\n", entry->d_name[5], entry->d_name[6], n->outputValues[pred]);
                grid[entry->d_name[5]-'0'][entry->d_name[6]-'0'] = pred;
                printf("Digit %d detected at [%c][%c]\n", pred, entry->d_name[5], entry->d_name[6]);
                free(output_filter_2);
                free(full_name);
                free(output_filter_1);
                free(conv2_out);
                free(conv1_out);
                free(n->inputValues);
            }
        }
        closedir(directory);

        FILE *output_file = fopen("sudoku_grid.txt", "w");
        if (output_file == NULL) {
            fprintf(stderr, "Error opening output file\n");
            return;
        }
        for (int i = 0; i < 9; i++) {
            if (i != 0 && i % 3 == 0) {
                fprintf(output_file, "\n");
            }
            for (int j = 0; j < 9; j++) {
                if (j != 0 && j % 3 == 0) {
                    fprintf(output_file, " ");
                }
                fprintf(output_file, "%d", grid[i][j]);
            }
            fprintf(output_file, "\n");
        }
        fclose(output_file);
}

void train(network *n, char *path)
{
    double lr = 0.0006f;  // Slightly reduced from 0.0007
    int max_iterations = 100000;
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
        while ((entry = readdir(directory)) != NULL && e < max_iterations) {
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
                apply_conv(SIZE, NB_FILTER_1, n->filter_1, n->inputValues, n->biais_1, conv1_out);
                double* output_filter_1 = maxPool(conv1_out, 26, NB_FILTER_1);

                apply_conv(13, NB_FILTER_2, n->filter_2, output_filter_1, n->biais_2, conv2_out);
                double* output_filter_2 = maxPool(conv2_out, 11, NB_FILTER_2);

                dense_reLU(n, output_filter_2);
                
                dense_logits(n);
                
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

                double current_lr = lr;
                if (e > 30000) current_lr = lr * 0.5;
                if (e > 40000) current_lr = lr * 0.2;
                if (e > 70000) current_lr = lr * 0.1;

                // Update weights with L2 regularization
                apply_l2_regularization(n->filter_1, dconv1_w, (size_t)NB_FILTER_1*1*SIZE_FILTER*SIZE_FILTER, current_lr, L2_LAMBDA);
                for(size_t i=0;i<(size_t)NB_FILTER_1;i++) n->biais_1[i] -= current_lr * dconv1_b[i];
                apply_l2_regularization(n->filter_2, dconv2_w, (size_t)NB_FILTER_2*NB_FILTER_1*SIZE_FILTER*SIZE_FILTER, current_lr, L2_LAMBDA);
                for(size_t i=0;i<(size_t)NB_FILTER_2;i++) n->biais_2[i] -= current_lr * dconv2_b[i];
                apply_l2_regularization(n->input_weight, ddense1_w, (size_t)HIDDEN_SIZE * MLP_SIZE, current_lr, L2_LAMBDA);
                for(size_t i=0;i<(size_t)HIDDEN_SIZE;i++) n->input_biais[i] -= current_lr * ddense1_b[i];
                apply_l2_regularization(n->hidden_weight, ddense2_w, (size_t)OUTPUT_SIZE * HIDDEN_SIZE, current_lr, L2_LAMBDA);
                for(size_t i=0;i<(size_t)OUTPUT_SIZE;i++) n->hidden_biais[i] -= current_lr * ddense2_b[i];

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

void free_Network(network* net)
{
    free(net);
}
