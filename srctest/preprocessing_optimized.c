#include "network.h"

// Nouvelle fonction de crop intelligent (au lieu de 10px fixe)
SDL_Surface* crop_image_adaptive(SDL_Surface* image)
{
    int width = image->w;
    int height = image->h;
    
    SDL_LockSurface(image);
    Uint32* pixels = image->pixels;
    SDL_PixelFormat* fmt = image->format;
    
    // Trouver les limites du texte
    int top = 0, bottom = height - 1;
    int left = 0, right = width - 1;
    
    // Trouver la première ligne non-vide (top)
    for (int y = 0; y < height; y++) {
        int has_content = 0;
        for (int x = 0; x < width; x++) {
            Uint8 r, g, b;
            SDL_GetRGB(pixels[y * width + x], fmt, &r, &g, &b);
            // Si le pixel n'est pas blanc
            if (r < 200 || g < 200 || b < 200) {
                has_content = 1;
                break;
            }
        }
        if (has_content) {
            top = y;
            break;
        }
    }
    
    // Trouver la dernière ligne non-vide (bottom)
    for (int y = height - 1; y >= 0; y--) {
        int has_content = 0;
        for (int x = 0; x < width; x++) {
            Uint8 r, g, b;
            SDL_GetRGB(pixels[y * width + x], fmt, &r, &g, &b);
            if (r < 200 || g < 200 || b < 200) {
                has_content = 1;
                break;
            }
        }
        if (has_content) {
            bottom = y;
            break;
        }
    }
    
    // Trouver la colonne gauche non-vide
    for (int x = 0; x < width; x++) {
        int has_content = 0;
        for (int y = 0; y < height; y++) {
            Uint8 r, g, b;
            SDL_GetRGB(pixels[y * width + x], fmt, &r, &g, &b);
            if (r < 200 || g < 200 || b < 200) {
                has_content = 1;
                break;
            }
        }
        if (has_content) {
            left = x;
            break;
        }
    }
    
    // Trouver la colonne droite non-vide
    for (int x = width - 1; x >= 0; x--) {
        int has_content = 0;
        for (int y = 0; y < height; y++) {
            Uint8 r, g, b;
            SDL_GetRGB(pixels[y * width + x], fmt, &r, &g, &b);
            if (r < 200 || g < 200 || b < 200) {
                has_content = 1;
                break;
            }
        }
        if (has_content) {
            right = x;
            break;
        }
    }
    
    SDL_UnlockSurface(image);
    
    // Ajouter un petit padding (3-5 px)
    int padding = 3;
    top = (top - padding > 0) ? top - padding : 0;
    left = (left - padding > 0) ? left - padding : 0;
    bottom = (bottom + padding < height) ? bottom + padding : height - 1;
    right = (right + padding < width) ? right + padding : width - 1;
    
    int crop_width = right - left + 1;
    int crop_height = bottom - top + 1;
    
    SDL_Rect crop_rect = {left, top, crop_width, crop_height};
    SDL_Surface* cropped = SDL_CreateRGBSurface(0, crop_width, crop_height, 32, 0, 0, 0, 0);
    SDL_BlitSurface(image, &crop_rect, cropped, NULL);
    
    return cropped;
}

// Fonction d'image_to_array améliorée avec meilleure normalisation
int image_to_array_improved(char* path, double** array)
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
    
    // Crop adaptatif au lieu du 10px fixe
    SDL_Surface* cropped = crop_image_adaptive(image);
    SDL_FreeSurface(image);
    image = cropped;
    
    // Redimensionner à 28x28
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
    
    // Convertir en grayscale avec meilleure normalisation
    double min_val = 255.0, max_val = 0.0;
    double gray_values[784];
    
    // Premier pass: convertir et trouver min/max
    for (size_t i = 0; i < 784; i++) {
        SDL_GetRGB(pixels[i], resized->format, &r, &g, &b);
        double gray = (double)(r + g + b) / 3.0;
        gray_values[i] = gray;
        
        if (gray < min_val) min_val = gray;
        if (gray > max_val) max_val = gray;
    }
    
    // Deuxième pass: normaliser [0, 1] avec meilleur contrast
    if (max_val > min_val) {
        for (size_t i = 0; i < 784; i++) {
            input[i] = (gray_values[i] - min_val) / (max_val - min_val);
        }
    } else {
        // Image uniforme
        for (size_t i = 0; i < 784; i++) {
            input[i] = gray_values[i] / 255.0;
        }
    }
    
    SDL_UnlockSurface(resized);
    SDL_FreeSurface(resized);
    
    return 0;
}

// Diagnostic: vérifier les statistiques du dataset
void diagnose_dataset_stats(char* path)
{
    printf("\n=== DATASET DIAGNOSTICS ===\n");
    
    DIR* dir = opendir(path);
    if (dir == NULL) {
        printf("Error opening path: %s\n", path);
        return;
    }
    
    int class_counts[10] = {0};
    int total_images = 0;
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR && entry->d_name[0] >= '0' && entry->d_name[0] <= '9') {
            int digit = entry->d_name[0] - '0';
            
            char subpath[MAX_FILE_NAME_SIZE];
            snprintf(subpath, MAX_FILE_NAME_SIZE, "%s/%s", path, entry->d_name);
            
            DIR* subdir = opendir(subpath);
            if (subdir != NULL) {
                struct dirent* subentry;
                while ((subentry = readdir(subdir)) != NULL) {
                    if (is_image_file(subentry->d_name)) {
                        class_counts[digit]++;
                        total_images++;
                    }
                }
                closedir(subdir);
            }
        }
    }
    closedir(dir);
    
    printf("Total images: %d\n", total_images);
    printf("Class distribution:\n");
    
    double expected = total_images / 10.0;
    for (int i = 0; i < 10; i++) {
        double ratio = 100.0 * class_counts[i] / total_images;
        printf("  Digit %d: %6d images (%.1f%%) %s\n", 
               i, class_counts[i], ratio,
               fabs(ratio - 10.0) > 5.0 ? "⚠️  IMBALANCED" : "✓");
    }
}

// Afficher les statistiques du réseau
void print_network_stats(network *n)
{
    printf("\n=== NETWORK STATS ===\n");
    printf("Conv1: %d filters, 3x3 kernel\n", NB_FILTER_1);
    printf("Conv2: %d filters, 3x3 kernel\n", NB_FILTER_2);
    printf("Dense1: %d hidden units\n", HIDDEN_SIZE);
    printf("Dense2: %d outputs\n", OUTPUT_SIZE);
    printf("Total parameters: ~%.1f million\n", 
           (NB_FILTER_1*9 + NB_FILTER_2*NB_FILTER_1*9 + HIDDEN_SIZE*MLP_SIZE + OUTPUT_SIZE*HIDDEN_SIZE) / 1e6);
}
