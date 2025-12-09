#include "carving.h"

SDL_Surface* slider_cut(SDL_Surface* source, int x1, int x2, int y1, int y2)
{
    int nwidth = x2 - x1;
    int nheight = y2 - y1;

    SDL_Surface* cut = SDL_CreateRGBSurface(0, nwidth, nheight,
                                            32, 0, 0, 0, 0);

    SDL_Rect rect = { x1, y1, nwidth, nheight };

    SDL_BlitSurface(source, &rect, cut, NULL);

    return cut;
}

SDL_Surface* padding(SDL_Surface* source, int padding)
{
    int nwidth = source -> w + 2 * padding;
    int nheight = source -> h + 2 * padding;
    SDL_Surface* padded =
            SDL_CreateRGBSurfaceWithFormat(0, nwidth, nheight,
                                           32, SDL_PIXELFORMAT_RGBA32);

    SDL_Rect prect = { 0, 0, nwidth, nheight };
    SDL_FillRect(padded, &prect, SDL_MapRGBA(padded -> format, 0, 0, 0, 0));

    SDL_Rect srcRect = { 0, 0, source -> w, source -> h };
    SDL_Rect destRect = { padding, padding, source -> w, source -> h };
    SDL_BlitSurface(source, &srcRect, padded, &destRect);

    SDL_FreeSurface(source);

    return padded;
}

SDL_Surface* unpadding(SDL_Surface* source, int padding)
{
    int nwidth = source -> w - 2 * padding;
    int nheight = source -> h - 2 * padding;

    SDL_Surface* unpadded = SDL_CreateRGBSurface(0, nwidth, nheight,
                                                 32, 0, 0, 0, 0);

    SDL_Rect srect = { padding, padding, nwidth, nheight };

    SDL_BlitSurface(source, &srect, unpadded, NULL);

    return unpadded;
}

int finder(SDL_Surface* source, int x, int y, int outline[][11])
{
    // Constants
    int colborn = 3;
    int maxpix = 6;
    int minpix = 4;
    int rowborn = 4;

    int pixrow[11];
    for (int i = 0; i < 11; i++)
        pixrow[i] = 0;

    int active = 1;

    if (!active)
        return 1;

    for (int i = 0; i < 5; i++)
        for (int j = 0; j < 11; j++)
            outline[i][j] = 0;

    int startX = x - 5 / 2;
    int startY = y - 11 / 2;

    Uint32* pixels = (Uint32*)source -> pixels;

    // Recuperation of the black pixels around the (x, y) pixel
    // in a window of a 5x11 size
    for (int y1 = 0; y1 < 11; y1++)
        for (int x1 = 0; x1 < 5; x1++) {
            Uint32 pixel = pixels[(startY + y1) * source -> w + (startX + x1)];
            Uint8 r, g, b, a;
            SDL_GetRGBA(pixel, source -> format, &r, &g, &b, &a);

            if ((r == 0 && g == 0 && b == 0) && a != 0)
                outline[x1][y1] = 1;
        }

    outline[2][5] = 1;

    if ((outline[3][5] && outline[4][5]) || (outline[0][5] && outline[1][5]))
        return 0;

    // START UPPER
    int uptot = 0;
    int pixcol[5];

    // array reset
    for (size_t i = 0; i < 5; i++)
        pixcol[i] = 0;

    // Recuperation of the total number of black pixels in the upper part
    // Recuperation of the number of col that has a black pixel
    for (size_t i = 0; i < 5; i++) {
        int upper = 0;
        for (size_t j = 0; j < 4; j++)
            if (outline[i][j]) {
                upper++;
                pixrow[j] = 1;
            }
        uptot += upper;
        if (upper)
            pixcol[i] = 1;
    }

    // Return false if two cols which are not in a row possess a black pixel
    int current = 0;
    int nbcol = 0;
    for (size_t i = 0; i < 5; i++) {
        if (pixcol[i]) {
            if (nbcol && !current)
                return 0;
            current = 1;
            nbcol++;
        }
        else
            current = 0;
    }

    if (nbcol > colborn || uptot > maxpix || uptot < minpix)
        return 0;

    // END OF UPPER

    // array reset
    for (size_t i = 0; i < 5; i++)
        pixcol[i] = 0;

    // START LOWER
    // The running is similar to the upper part

    int lowtot = 0;

    for (size_t i = 0; i < 5; i++) {
        int lower = 0;
        for (size_t j = 5; j < 11; j++)
            if (outline[i][j]) {
                lower++;
                pixrow[j] = 1;
            }
        lowtot += lower;
        if (lower)
            pixcol[i] = 1;
    }

    current = 0;
    nbcol = 0;
    for (size_t i = 0; i < 5; i++) {
        if (pixcol[i]) {
            if (nbcol && !current)
                return 0;
            current = 1;
            nbcol++;
        }
        else
            current = 0;
    }

    if (nbcol > colborn || uptot > maxpix || uptot < minpix)
        return 0;

    // END OF LOWER

    // Recuperation of the number of rows which contain a black pixel
    int rowcount = 0;
    for (size_t i = 0; i < 11; i++)
        if (pixrow[i])
            rowcount++;

    if (rowcount < rowborn)
        return 0;

    return 1;
}

SDL_Surface* carve(SDL_Surface* source, SDL_Point intersections[][10])
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
    SDL_Point tl, tr, bl, br;

    // Storage array where all the pixels around at a distance of 5
    // of the one the algorithm selects will be saved
    // in order to determine if it's a pixel which belong to a vertical line.
    // -> "1" is for a black pixel
    // -> "0" is for a white pixel
    int outline[5][11];

    // Recuperation of the x, y coordinates of the lower left border
    // of the sudoku grid.

    // Recuperation of the angle of the left border
    SDL_Point mlp;
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

    SDL_Point ulp;
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
    SDL_Point mrp;
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

    SDL_Point urp;
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
    SDL_Surface* vectt = SDL_CreateRGBSurface(0, width, height,
                                              32, 0, 0, 0, 0);

    vectt = unpadding(vectt, padd);

    SDL_Renderer* renderer = SDL_CreateSoftwareRenderer(vectt);

    tl.x -= padd - 2;
    tl.y -= padd - 2;
    tr.x -= padd - 2;
    tr.y -= padd - 2;
    bl.x -= padd - 2;
    bl.y -= padd - 2;
    br.x -= padd - 2;
    br.y -= padd - 2;

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // rouge
    SDL_RenderDrawLine(renderer, tl.x, tl.y, bl.x, bl.y);
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // vert
    SDL_RenderDrawLine(renderer, tl.x, tl.y, tr.x, tr.y);
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // bleu
    SDL_RenderDrawLine(renderer, bl.x, bl.y, br.x, br.y);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // blanc
    SDL_RenderDrawLine(renderer, br.x, br.y, tr.x, tr.y);

    SDL_SetRenderDrawColor(renderer, 51, 204, 51, 255); // vert pomme

    SDL_Point topsub[10];
    for (int i = 0; i < 10; i++) {
        float t = i / 9.0;
        topsub[i].x = tl.x + (int)(t * (tr.x - tl.x));
        topsub[i].y = tl.y + (int)(t * (tr.y - tl.y));
    }

    SDL_Point botsub[10];
    for (int i = 0; i < 10; i++) {
        float t = i / 9.0;
        botsub[i].x = bl.x + (int)(t * (br.x - bl.x));
        botsub[i].y = bl.y + (int)(t * (br.y - bl.y));
    }

    SDL_Point leftsub[10];
    for (int i = 0; i < 10; i++) {
        float t = i / 9.0;
        leftsub[i].x = tl.x + (int)(t * (bl.x - tl.x));
        leftsub[i].y = tl.y + (int)(t * (bl.y - tl.y));
    }

    SDL_Point rightsub[10];
    for (int i = 0; i < 10; i++) {
        float t = i / 9.0;
        rightsub[i].x = tr.x + (int)(t * (br.x - tr.x));
        rightsub[i].y = tr.y + (int)(t * (br.y - tr.y));
    }

    for (int i = 0; i < 10; i++) {
        SDL_RenderDrawLine(renderer, topsub[i].x, topsub[i].y,
                           botsub[i].x, botsub[i].y);
        SDL_RenderDrawLine(renderer, leftsub[i].x, leftsub[i].y,
                           rightsub[i].x, rightsub[i].y);
    }

    // Structure of the grid
    IMG_SavePNG(vectt, "border.png");

    // Intersection points (i : line, j : column)
    for (int j = 0; j < 10; j++) {
        for (int i = 0; i < 10; i++) {
            intersections[i][j].x = tl.x + (int)((i / 9.0) *
                    (bl.x - tl.x)) + (int)((j / 9.0) * (tr.x - tl.x));
            intersections[i][j].y = tl.y + (int)((i / 9.0) *
                    (bl.y - tl.y)) + (int)((j / 9.0) * (tr.y - tl.y));
        }
    }
    
    for (size_t row = 0; row < 10; row++)
        for (size_t col = 0; col < 10; col++)
            printf("[%zu][%zu] = %i, %i\n", row, col,
                   intersections[row][col].x, intersections[row][col].y);

    SDL_FreeSurface(vectt);

    return source;
}

SDL_Surface* image(SDL_Surface* source, SDL_Point tl, SDL_Point tr,
                   SDL_Point bl, SDL_Point br)
{
    int minX = SDL_min(tl.x, SDL_min(tr.x, SDL_min(bl.x, br.x)));
    int minY = SDL_min(tl.y, SDL_min(tr.y, SDL_min(bl.y, br.y)));
    int maxX = SDL_max(tl.x, SDL_max(tr.x, SDL_max(bl.x, br.x)));
    int maxY = SDL_max(tl.y, SDL_max(tr.y, SDL_max(bl.y, br.y)));

    SDL_Surface* tile = SDL_CreateRGBSurface(0, maxX - minX, maxY - minY,
                                             32, 0, 0, 0, 0);

    SDL_Rect srect = { minX, minY, maxX - minX, maxY - minY };
    SDL_Rect drect = { 0, 0, maxX - minX, maxY - minY };
    SDL_BlitSurface(source, &srect, tile, &drect);

    return tile;
}

void cut(SDL_Surface* source, SDL_Point intersections[][10])
{
    // Directory creation
    if (mkdir("grid", 0777) == -1)
        //errx(EXIT_FAILURE, "Error while creating the main directory.");
        printf("Directory grid/ already exists.\n");

    char filepath[] = "grid/tile_00.png";

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            SDL_Surface* tile = image(source, intersections[i][j],
                                      intersections[i][j+1],
                                      intersections[i+1][j],
                                      intersections[i+1][j+1]);

            IMG_SavePNG(tile, filepath);
            filepath[11]++;

            SDL_FreeSurface(tile);
        }
        filepath[11] = '0';
        filepath[10]++;
    }
}