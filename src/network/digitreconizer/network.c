#include "network.h"


network* init_network()
{
    srand(time(NULL));
    struct network *net = calloc(1,sizeof(struct network));
    for (size_t i = 0; i < (size_t)INPUT_SIZE; i++)
    {
        for (size_t j = 0; j < (size_t)HIDDEN_SIZE; j++)
        {
            net->input_weight[i][j]= (double)rand() / RAND_MAX * 2.0 - 1.0;
        }
    }
    for (size_t i = 0; i < (size_t)HIDDEN_SIZE; i++)
    {
        net->input_biais[i]= (double)rand() / RAND_MAX * 2.0 - 1.0;
    }
    for (size_t i = 0; i < (size_t)HIDDEN_SIZE; i++)
    {
        for (size_t j = 0; j < (size_t)OUTPUT_SIZE; j++)
        {
            net->hidden_weight[i][j]= (double)rand() / RAND_MAX * 2.0 - 1.0;
        }
    }
    for (size_t i = 0; i < (size_t)OUTPUT_SIZE; i++)
    {
        net->hidden_biais[i]= (double)rand() / RAND_MAX * 2.0 - 1.0;
    }
    return net;
}

void  input_network(network* net, double input[784])
{
    for (size_t j = 0; j < 784; j++)
    {
        net->inputValues[j] = input[j];
    }
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

void forward_propagation(network* net)
{
    for (size_t i = 0; i < HIDDEN_SIZE; i++)
    {
        net->hiddenValues[i] = 0.0;
        for (size_t j = 0; j < INPUT_SIZE; j++) 
        {
            net->hiddenValues[i] += net->inputValues[j] * net->input_weight[j][i];
        }
        net->hiddenValues[i] += net->input_biais[i];
        net->hiddenValues[i] = sigmoid(net->hiddenValues[i]);
    }
    for (size_t i = 0; i < OUTPUT_SIZE; i++) 
    {
        net->outputValues[i]=0.0;
        for (size_t j = 0; j < HIDDEN_SIZE; j++)
        {
            net->outputValues[i] += net->hiddenValues[j] * net->hidden_weight[j][i];
        }
        net->outputValues[i] += net->hidden_biais[i];
        net->outputValues[i] = sigmoid(net->outputValues[i]);
    }
}

void backpropagation(network* net, double* target)
{
    double errorRate = 0.0;
    double errorTemp = 0.0;
    forward_propagation(net);
    double output_delta[OUTPUT_SIZE];
    for (size_t i = 0; i < OUTPUT_SIZE; i++)
    {
        errorTemp = target[i] - net->outputValues[i];
        output_delta[i] = errorTemp * sigmoid_derivative(net->outputValues[i]);
        errorRate += errorTemp * errorTemp;
    }
    double hidden_deltas[HIDDEN_SIZE];
    for (size_t i = 0; i < HIDDEN_SIZE; i++) 
    {
        errorTemp = 0.0;
        for (size_t j = 0; j < OUTPUT_SIZE; j++)
        {
            errorTemp += output_delta[j] * net->hidden_weight[i][j];
        }
        hidden_deltas[i] = errorTemp * sigmoid_derivative(net->hiddenValues[i]);
    }
    for (size_t i = 0; i < OUTPUT_SIZE; i++) 
    {
        for (size_t j = 0; j < HIDDEN_SIZE; j++)
        {
            net->hidden_weight[j][i] += LEARNING_RATE * output_delta[i] * net->hiddenValues[j];
            net->hidden_biais[i] += LEARNING_RATE * output_delta[i]* net->hiddenValues[j];
        }
    }
    for (size_t i = 0; i < HIDDEN_SIZE; i++)
    {
        for (size_t j = 0; j < INPUT_SIZE; j++) 
        {
            net->input_weight[j][i] += LEARNING_RATE * hidden_deltas[i] * net->inputValues[j];
            net->input_biais[i] += LEARNING_RATE * hidden_deltas[i]* net->inputValues[j];
        }
    }
}

double** create_test(char* path)
{
    double** input = malloc(50*sizeof(double*));
    for (size_t i = 0; i < 50; i++)
    {
        input[i] = malloc(784*sizeof(double));
    }
    for (size_t i = 0; i < 10; i++)
    {
        for (size_t j = 0; j < 5; j++)
        {
            char* path_image = malloc(100*sizeof(char));
            sprintf(path_image,"%s/%ld/%ld_%ld.png",path,i,i,j);
            image_to_array(path_image,input[i*5+j]);
            free(path_image);
        }
    }
    return input;
}

void resize_image(char* path)
{
    SDL_Surface* s = IMG_Load(path);
    //printf("%s\n",path);
    if (s == NULL)
    {
        printf("Erreur lors du chargement de l'image\n");
        exit(1);
    }
    SDL_Surface *image = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGB888, 0);
    SDL_FreeSurface(s);
    if (image == NULL)
    {
        printf("Erreur lors du chargement de l'image\n");
        exit(1);
    }
    SDL_Surface* resized = SDL_CreateRGBSurface(0, 28, 28, 32, 0, 0, 0, 0);
    if (resized == NULL)
    {
        printf("Erreur lors de l'allocation de la mémoire\n");
        exit(1);
    }
    SDL_BlitScaled(image, NULL, resized, NULL);
    SDL_SaveBMP(resized,path);
    SDL_FreeSurface(image);
    SDL_FreeSurface(resized);
}

void image_to_array(char* path, double* array)
{
    resize_image(path);
    SDL_Surface* s = IMG_Load(path);
    if (s == NULL)
    {
        printf("Erreur lors du chargement de l'image\n");
        exit(1);
    }
    SDL_Surface *image = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGB888, 0);
    SDL_FreeSurface(s);
    if (image == NULL)
    {
        printf("Erreur lors du chargement de l'image\n");
        exit(1);
    }
    SDL_LockSurface(image);
    Uint8 r, g, b;
    Uint32* pixels = image->pixels;
    for (size_t i = 0; i < 784; i++)
    {
        SDL_GetRGB(pixels[i], image->format, &r, &g, &b);
        array[i] = (double)(r+g+b)/3/255;
    }
    SDL_UnlockSurface(image);
    SDL_FreeSurface(image);
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

void freeSudoku(double** grid)
{
    for (size_t i = 0; i < 9; i++)
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

void freeNetwork(network* net)
{
   free(net);
}
