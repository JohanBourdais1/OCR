#include "pre.h"

// -rotation
SDL_Surface* rotateSurface(SDL_Surface* surface, double angle)
{
    double  road_angle = angle * M_PI / 180.0;

    int w = surface->w;
    int h = surface->h;
    int new_w = (int)(fabs(w * cos(road_angle)) + fabs(h * sin(road_angle)));
    int new_h = (int)(fabs(w * sin(road_angle)) + fabs(h * cos(road_angle)));

    SDL_Surface* result  = SDL_CreateRGBSurface(0,new_w,new_h,32,0,0,0,0);


    if (result == NULL)
    {
        //SDL_Log("Erreur lors de la creation",SDL_GetError());
        return NULL;
    }
    SDL_Rect dest;
    dest.x = (new_w - w) / 2;
    dest.y = (new_h - h) / 2;
    dest.w = w;
    dest.h = h;

    SDL_FillRect(result,NULL,SDL_MapRGBA(result->format,0,0,0,0));

    if (SDL_BlitSurface(surface,NULL,result,&dest) < 0)
    {
        //SDL_Log("Erreur lors de la rotation", SDL_GetError());
        SDL_FreeSurface(result);
        return NULL;
    }
    SDL_FreeSurface(surface);
    return result;

}


SDL_Surface* rotateImage(SDL_Surface* image, double angle)
{

    angle = angle * M_PI / 180.0;  // Convert to radians

    // Calculate the size of the rotated image
    int newWidth = (int)(fabs(image->w * cos(angle)) +
                         fabs(image->h * sin(angle)));
    int newHeight = (int)(fabs(image->w * sin(angle)) +
                          fabs(image->h * cos(angle)));

    // Create a new surface for the rotated image
    SDL_Surface* rotatedImage =
        SDL_CreateRGBSurface(0, newWidth, newHeight, 32, 0, 0, 0, 0);

    // Calculate the pivot point for rotation (center of the image)
    int pivotX = image->w / 2;
    int pivotY = image->h / 2;

    // Perform the rotation by copying pixels
    for (int x = 0; x < rotatedImage->w; x++) {
        for (int y = 0; y < rotatedImage->h; y++) {
            int srcX = (int)((x - rotatedImage->w / 2) * cos(angle) -
                             (y - rotatedImage->h / 2) * sin(angle) + pivotX);
            int srcY = (int)((x - rotatedImage->w / 2) * sin(angle) +
                             (y - rotatedImage->h / 2) * cos(angle) + pivotY);

            if (srcX >= 0 && srcX < image->w && srcY >= 0 && srcY < image->h) {
                Uint32 pixel = ((Uint32*)image->pixels)[srcY * image->w + srcX];
                ((Uint32*)rotatedImage->pixels)[y * rotatedImage->w + x] =
                    pixel;
            }
        }
    }
    return rotatedImage;
}


// Loads an image in a surface.
// The format of the surface is SDL_PIXELFORMAT_RGB888.
 //
// path: Path of the image.
SDL_Surface* load_image(const char* path)
{
    SDL_Surface* pre_surface = IMG_Load(path);
    SDL_Surface* surface =
        SDL_ConvertSurfaceFormat(pre_surface,SDL_PIXELFORMAT_RGB888 , 0);
    SDL_FreeSurface(pre_surface);
    return surface;
}

// Converts a colored pixel into grayscale.
//
// pixel_color: Color of the pixel to convert in the RGB format.
// format: Format of the pixel used by the surface.
Uint32 pixel_to_grayscale(Uint32 pixel_color, SDL_PixelFormat* format)
{
    Uint8 r, g, b;
    SDL_GetRGB(pixel_color, format, &r, &g, &b);
    Uint8 average = 0.3*r + 0.59*g + 0.11*b;
    r = average;
    g = average;
    b = average;
    Uint32 color = SDL_MapRGB(format, r, g, b);
    return color;
}

Uint32 pixel_to_black_and_white(Uint32 pixel_color, SDL_PixelFormat* format, int
                                threshold)
{
    Uint8 r, g, b;
    SDL_GetRGB(pixel_color, format, &r, &g, &b);

    int luminance = (int)(0.299 * r + 0.587 * g + 0.114 * b);
    if (luminance >= threshold)
    {
        return SDL_MapRGB(format, 255, 255, 255);
    }
    else
    {
        return  SDL_MapRGB(format, 0, 0, 0);
    }
}

int threshold(SDL_Surface* surface,SDL_PixelFormat* format)
{
    Uint32* pixels = surface -> pixels;
    int hist[256] = {0};
    int len = surface->w * surface->h;
    for(int i = 0 ; i < len; i++)
    {
        Uint8 r,g,b;
        SDL_GetRGB(pixels[i], format, &r, &g, &b);
        int luminance = (int)(0.299 * r + 0.587 * g + 0.114 * b);
        hist[luminance] += 1;
    }
    int pics_numb = 0;
    int pics[256];
    for(int i = 1 ; i < 255; i++)
    {
        if (hist[i] > hist[i-1] && hist[i] > hist[i+1])
            pics[(pics_numb++)] = i;
    }
    if (pics_numb == 0)
        return 256/2;
    int sum = 0;
    for(int i = 0 ; i < pics_numb; i++)
        sum += pics[i];
    return sum / pics_numb;
}

void surface_to_grayscale(SDL_Surface* surface)
{
    Uint32* pixels = surface->pixels;
    int len = surface->w * surface->h;
    SDL_PixelFormat* format = surface->format;
    SDL_LockSurface(surface);
    for(int i = 0 ; i < len; i++)
    {
        pixels[i] =  pixel_to_grayscale(pixels[i], format);
    }
    SDL_UnlockSurface( surface);
}

void contrast_streching(SDL_Surface* surface)
{
    if (surface == NULL || surface->format->BytesPerPixel != 3) {
        // Gère les cas où la surface n'est pas valide ou n'est pas au format RGB888
        return;
    }

    Uint8 *pixels = (Uint8*)surface->pixels;
    Uint8 minPixelValue = 255;
    Uint8 maxPixelValue = 0;

    // Trouver les valeurs minimale et maximale des composantes RGB dans l'image
    for (int i = 0; i < surface->w * surface->h * 3; i += 3) {
        Uint8 pixelValue = pixels[i];
        if (pixelValue < minPixelValue) {
            minPixelValue = pixelValue;
        }
        if (pixelValue > maxPixelValue) {
            maxPixelValue = pixelValue;
        }
    }

    // Appliquer l'étirement de contraste
    for (int i = 0; i < surface->w * surface->h * 3; i += 3) {
        Uint8 pixelValue = pixels[i];
        Uint8 newValue = (Uint8)(((pixelValue - minPixelValue) * 255) / (maxPixelValue - minPixelValue));

        // Assurer que les nouvelles valeurs restent dans la plage de 0 à 255
        //newValue = (newValue > 255) ? 255 : newValue;
	//newValue = (newValue < 0) ? 0 : newValue;

        pixels[i] = newValue;
        pixels[i + 1] = newValue;
        pixels[i + 2] = newValue;
    }
}

void surface_to_black_and_white(SDL_Surface* surface)
{
    Uint32* pixels = surface->pixels;
    int len = surface->w * surface->h;
    SDL_PixelFormat* format = surface->format;
    SDL_LockSurface(surface);
    int thr =  threshold(surface, format);
    for(int i = 0 ; i < len; i++)
    {
        pixels[i] =  pixel_to_black_and_white(pixels[i], format,thr);
    }
    SDL_UnlockSurface( surface);
}

double get_double_fm_argv(char* s)
{
    double result = 0;
    while(*s != 0)
    {
        result = result * 10 + (*s - '0');
        s++;
    }
    return result;
}

double angle_detection(int tl_x,int tl_y,int tr_x,int tr_y,
			int bl_x,int bl_y,int br_x,int br_y)
{
    // x of the top
    double xt = tl_x + ((tr_x - tl_x) / 2);
    //y of the top
    double yt = tl_y + ((tr_y - tl_y) / 2);
    //x of the buttom
    double xb = bl_x + ((br_x - bl_x) / 2);
    //y of the buttom
    double yb = bl_y + ((br_y - bl_y) / 2);

    double dx = xb - xt;
    double dy = yb - yt;

    double  angle_rad = atan2(dy,dx);

    double angle_deg = angle_rad * (180.0 / M_PI);
    return angle_deg;
}

SDL_Surface* padding2(SDL_Surface* source, int padding)
{
    int nwidth = source->w + 2 * padding;
    int nheight = source->h + 2 * padding;
    SDL_Surface* padded = SDL_CreateRGBSurfaceWithFormat(0, nwidth, nheight, 32, SDL_PIXELFORMAT_RGBA32);

    SDL_Rect prect = {0, 0, nwidth, nheight};
    SDL_FillRect(padded, &prect, SDL_MapRGBA(padded->format, 0, 0, 0, 0));

    SDL_Rect srcRect = {0, 0, source->w, source->h};
    SDL_Rect destRect = {padding, padding, source->w, source->h};
    SDL_BlitSurface(source, &srcRect, padded, &destRect);

    SDL_FreeSurface(source);

    return padded;
}

void carve2(SDL_Surface* source,SDL_Point* Points,SDL_Point intersections[10][10])
{

    int padd = 250;
    int absval = 2;

    source = unpadding(source, 2);
    source = padding(source, padd);
    int width, height;
    width = source -> w;
    height = source -> h;

    Uint32* pixels = (Uint32*)source -> pixels;

    // all these abbreviations stand for :
    // top left x, top left y, top right x, top right y,
    // bottom left x, bottom left y, bottom right x, bottom right y
    // top left x, top left y, top right x, top right y, bottom left x, bottom left y, bottom right x, bottom right y
    SDL_Point tl = {0,0};
    SDL_Point tr = {0,0};
    SDL_Point bl = {0,0};
    SDL_Point br = {0,0};

    // Storage array where all the pixels around at a distance of 5
    // of the one the algorithm selects will be saved
    // in order to determine if it's a pixel which belong to a vertical line.
    // -> "1" is for a black pixel
    // -> "0" is for a white pixel
    int outline[5][11];

    // Recuperation of the x, y coordinates of the lower left border
    // of the sudoku grid.

    // Recuperation of the angle of the left border
    SDL_Point mlp = {0,0};
    for (int x = 0; x < width; x++) {
        Uint32 pixel = pixels[(height / 2) * width + x];
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

        if (r == 0 && g == 0 && b == 0 && a != 0) {
            printf("first coordinates are : %i, %i\n", x, (height / 2));
            if (finder(source, x, (height / 2), outline)) {
                mlp.x = x;
                printf("finder true\n");
                break;
            }
            printf("finder false\n");
            continue;
        }
    }

    SDL_Point ulp = {0,0};
    for (int x = 0; x < width; x++) {
        Uint32 pixel = pixels[(height / 2 - 10) * width + x];
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

        if (r == 0 && g == 0 && b == 0 && a != 0) {
            printf("second coordinates are : %i, %i\n", x, (height / 2 - 10));
            if (finder(source, x, (height / 2 - 10), outline)) {
                ulp.x = x;
                printf("finder true\n");
                break;
            }
            printf("finder false\n");
            continue;
        }
    }

    if (mlp.x > ulp.x) {
        tl.x = mlp.x;
        tl.y = height / 2;
        // Upper left corner
        for (int y = height / 2; y > 0; y--) {
            for (int x = 0; x < width; x++) {
                Uint32 pixel = pixels[y * width + x];
                Uint8 r, g, b, a;
                SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

                if ((r == 0 && g == 0 && b == 0) && a != 0 &&
                abs(tl.x - x) < absval)
                    if (finder(source, x, y, outline)) {
                        tl.x = x;
                        tl.y = y;
                        break;
                    }
            }
        }

        bl.x = mlp.x;
        bl.y = height / 2;
        // Lower left corner
        for (int y = height / 2; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Uint32 pixel = pixels[y * width + x];
                Uint8 r, g, b, a;
                SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

                if (r == 0 && g == 0 && b == 0 && a != 0 &&
                abs(bl.x - x) < absval)
                    if (finder(source, x, y, outline)) {
                        bl.x = x;
                        bl.y = y;
                        break;
                    }
            }
        }
    }
    if (mlp.x <= ulp.x) {
        tl.x = mlp.x;
        tl.y = height / 2;
        // Upper left corner
        for (int y = height / 2; y > 0; y--) {
            for (int x = 0; x < width; x++) {
                Uint32 pixel = pixels[y * width + x];
                Uint8 r, g, b, a;
                SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

                if (r == 0 && g == 0 && b == 0 && a != 0 &&
                abs(x - tl.x) < absval)
                    if (finder(source, x, y, outline)) {
                        tl.x = x;
                        tl.y = y;
                        break;
                    }
            }
        }

        bl.x = mlp.x;
        bl.y = height / 2;
        // Lower left corner
        for (int y = height / 2; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Uint32 pixel = pixels[y * width + x];
                Uint8 r, g, b, a;
                SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

                if (r == 0 && g == 0 && b == 0 && a != 0 &&
                abs(bl.x - x) < absval)
                    if (finder(source, x, y, outline)) {
                        bl.x = x;
                        bl.y = y;
                        break;
                    }
            }
        }
    }

    // Recuperation of the angle of the right border
    SDL_Point mrp = {0,0};
    for (int x = width; x > 0; x--) {
        Uint32 pixel = pixels[(height / 2) * width + x];
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

        if (r == 0 && g == 0 && b == 0 && a != 0) {
            if (finder(source, x, (height / 2), outline)) {
                mrp.x = x;
                break;
            }
            continue;
        }
    }

    SDL_Point urp = {0,0};
    for (int x = width; x > 0; x--) {
        Uint32 pixel = pixels[(height / 2 - 10) * width + x];
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

        if (r == 0 && g == 0 && b == 0 && a != 0) {
            if (finder(source, x, (height / 2 - 10), outline)) {
                urp.x = x;
                break;
            }
            continue;
        }
    }

    if (mrp.x >= urp.x) {
        tr.x = mrp.x;
        tr.y = height / 2;
        // Upper right corner
        for (int y = height / 2; y > 0; y--) {
            for (int x = width; x > 0; x--) {
                Uint32 pixel = pixels[y * width + x];
                Uint8 r, g, b, a;
                SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

                if (r == 0 && g == 0 && b == 0 && a != 0 &&
                abs(tr.x - x) < absval)
                    if (finder(source, x, y, outline)) {
                        tr.x = x;
                        tr.y = y;
                        break;
                    }
            }
        }

        br.x = mrp.x;
        br.y = height / 2;
        // Lower right corner
        for (int y = height / 2; y < height; y++) {
            for (int x = width; x > 0; x--) {
                Uint32 pixel = pixels[y * width + x];
                Uint8 r, g, b, a;
                SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

                if ((r == 0 && g == 0 && b == 0) && a != 0 &&
                abs(br.x - x) < absval)
                    if (finder(source, x, y, outline)) {
                        br.x = x;
                        br.y = y;
                        break;
                    }
            }
        }
    }
    if (mrp.x < urp.x) {
        tr.x = mrp.x;
        tr.y = height / 2;

        // Upper right corner
        for (int y = height / 2; y > 0; y--) {
            for (int x = width; x > 0; x--) {
                Uint32 pixel = pixels[y * width + x];
                Uint8 r, g, b, a;
                SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

                if (r == 0 && g == 0 && b == 0 && a != 0 &&
                abs(tr.x - x) < absval)
                    if (finder(source, x, y, outline)) {
                        tr.x = x;
                        tr.y = y;
                        break;
                    }
            }
        }

        br.x = mrp.x;
        br.y = height / 2;
        // Lower right corner
        for (int y = height / 2; y < height; y++) {
            for (int x = width; x > 0; x--) {
                Uint32 pixel = pixels[y * width + x];
                Uint8 r, g, b, a;
                SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

                if (r == 0 && g == 0 && b == 0 && a != 0 &&
                abs(br.x - x) < absval)
                    if (finder(source, x, y, outline)) {
                        br.x = x;
                        br.y = y;
                        break;
                    }
            }
        }
    }

    // Segments drawing
    

    tl.x -= padd;
    tl.y -= padd;
    tr.x -= padd;
    tr.y -= padd;
    bl.x -= padd;
    bl.y -= padd;
    br.x -= padd;
    br.y -= padd;

    Points[0] = tl;
    Points[1] = tr;
    Points[2] = bl;
    Points[3] = br;
    printf("tl.x = %d et tl.y = %d tr.x = %d tr.y = %d,bl.x = %d bl.y = %d , br.x = %d,br.y = %d \n"
           ,tl.x,tl.y,tr.x,tr.y,bl.x,bl.y,br.x,br.y);

    SDL_FreeSurface(source);
}

