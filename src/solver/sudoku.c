#include "sudoku.h"


// Fonction pour vérifier si une valeur est valide dans une case de la grille
int isValid(int** grid, int row, int col, int num) 
{
    //verifie si la valeur est valide dans la case
    int i,j;
    for(i=0;i<9;i++)
    {
        if(grid[row][i]==num && i!=col)
            return 0;
    }
    for(i=0;i<9;i++)
    {
        if(grid[i][col]==num && i!=row)
            return 0;
    }
    int rowStart = row - row % 3;
    int colStart = col - col % 3;
    for(i=rowStart;i<rowStart+3;i++)
    {
        for(j=colStart;j<colStart+3;j++)
        {
            if(grid[i][j]==num && i!=row && j!=col)
                return 0;
        }
    }
    return 1;
    /*for (int i = 0; i < 9; i++) 
    {
        if (grid[row][i] == num || grid[i][col] == num) 
	{
            return 0;
        }
    }

    int startRow = row - row % 3;
    int startCol = col - col % 3;

    for (int i = 0; i < 3; i++) 
    {
        for (int j = 0; j < 3; j++) 
	{
            if (grid[i + startRow][j + startCol] == num) 
	    {
                return 0;
            }
        }
    }

    return 1;*/
}

int isGridValid(int** grid)
{
    //verifie si la grid est valide
    int i,j;
    for(i=0;i<9;i++)
    {
        for(j=0;j<9;j++)
        {
            printf("%d",grid[i][j]);
            if((grid[i][j]!=0) && isValid(grid,i,j,grid[i][j])==0)
                return 0;
        }
    }
    return 1;
	/*for(size_t i = 0;i<9;i++)
	{
		for(size_t j = 0;j<9;j++)
		{
			if((grid[i][j]!=0) && isValid(grid,i,j,grid[i][j])==0)
				return 0;
		}
	}
	return 1;*/
}

// Fonction récursive pour résoudre le Sudoku
int solvingSudoku(int** grid)
{
    //resout la grid
    int row, col;
    // Chercher une case vide
    int find = 0;
    for(row=0;row<9;row++)
    {
        for(col=0;col<9;col++)
        {
            if(grid[row][col]==0)
            {
                find = 1;
                break;
            }
        }
        if(find==1)
            break;
    }
    if(find==0)
        return 1;
    for(int num=1;num<=9;num++)
    {
        if(isValid(grid,row,col,num))
        {
            grid[row][col] = num;
            if(solvingSudoku(grid))
                return 1;
            grid[row][col] = 0;
        }
    }
    return 0;
    /*int row, col;
    // Chercher une case vide
    int find = 0;
    for (row = 0; row < 9; row++) 
    {
        for (col = 0; col < 9; col++) 
	{
            if (grid[row][col] == 0) 
	    {
                find = 1;
                break;
            }
        }
        if (find==1) 
	{
            break;
        }
    }

    // S'il n'y a pas de case vide, le Sudoku est résolu
    if (find==0) 
    {
        return 1;
    }

    // Essayer d'assigner une valeur de 1 à 9 à la case vide
    for (int num = 1; num <= 9; num++) 
    {
        if (isValid(grid, row, col, num)) 
	{
            grid[row][col] = num;
            if (solvingSudoku(grid)) 
	    {
                return 1;
            }

            // Si l'assignation ne mène pas à une solution, annuler et essayer la valeur suivante
            grid[row][col] = 0;
        }
    }

    return 0; // Aucune valeur possible n'a conduit à une solution*/
}

int** readGridFromFile(char* path)
{
    int** grid = malloc(sizeof(int*)*9);
    FILE *file = fopen(path,"r");
    if (file == NULL) 
    {
        errx(1,"Erreur : Impossible d'ouvrir le fichier.\n");
        
    }
    for (int i = 0; i < 9; i++) 
    {
        grid[i] = malloc(sizeof(int)*9);
        for (int j = 0; j < 9; j++) 
        {
            char character;
            if (fscanf(file, " %c", &character) != 1) 
            {
                fclose(file);
                errx(1,"Erreur : Le fichier n'a pas le bon format.\n");
            }
            if (character == '0') 
            {
                grid[i][j] = 0;
            } else if (character >= '1' && character <= '9') 
            {
                grid[i][j] = character - '0';
            } else 
            {
                fclose(file);
                errx(1,"Erreur : Caractère invalide dans le fichier.\n");
            }
            printf("%d", grid[i][j]);
        }
        printf("\n");
    }
    fclose(file);
    return grid;
}

void writeGrid(char* path, int** grid)
{
    FILE *file_w = fopen(path, "w");
    if (file_w == NULL) 
    {
        errx(1,"Erreur : Impossible d'ouvrir le fichier d'écriture.\n");
    }
    for (int i = 0; i < 9; i++) 
    {
        for (int j = 0; j < 9; j++) 
        {
            fprintf(file_w, "%d", grid[i][j]);
        if(j==2 || j==5)
            fprintf(file_w, " ");
        }
        fprintf(file_w, "\n");
        if(i==2 || i==5)
            	fprintf(file_w, "\n");
    }
    fclose(file_w);
}

void recreateGrid(int** grid , int oldGrid[9][9])
{
    //recreate grid in an image
    SDL_Surface *img = SDL_CreateRGBSurface(0, 354, 354, 32, 0, 0, 0, 0);
    SDL_FillRect(img, NULL, SDL_MapRGB(img->format, 255, 255, 255));
    Uint32* pixel = img->pixels;
    size_t w = img->w;
    size_t h = img->h;
    SDL_PixelFormat* fmt = img->format;
    if (SDL_LockSurface(img) < 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());
    size_t a = 16;
    for (size_t k = 0; k < 4; k++)
    {
        for (size_t i = 0; i < 10; i++)
        {
            for (size_t j = w*a+16; j < w-16+a*w; j++)
            {
                pixel[(j+w*i)] = SDL_MapRGB(fmt, 0, 0, 0);
            }
        }
        a+=104;
    }
    a=5680;
    for (size_t k = 0; k < 4; k++)
    {
        for (size_t i = 0; i < 10; i++)
        {
            for (size_t j = a; j < h*w-16*w; j+=w)
            {
                pixel[j+i] = SDL_MapRGB(fmt, 0, 0, 0);
            }
        }
        a+=104;
    }
    a=54;
    for (size_t k = 0; k < 6; k++)
    {
        for (size_t i = 0; i < 5; i++)
        {
            for (size_t j = a*w+16; j < w-16+a*w; j++)
            {
                pixel[(j+w*i)] = SDL_MapRGB(fmt, 0, 0, 0);
            }
        }
        a+=32;
        if(k==1 || k==3)
            a+=40;
    }
    a=5718;
    for (size_t k = 0; k < 6; k++)
    {
        for (size_t i = 0; i < 5; i++)
        {
            for (size_t j = a; j < h*w-16*w; j+=w)
            {
                pixel[j+i] = SDL_MapRGB(fmt, 0, 0, 0);
            }
        }
        a+=32;
        if(k==1 || k==3)
            a+=40;
    }
    SDL_UnlockSurface(img);
    SDL_Rect position;
    position.y = 16;
    for (size_t i = 0; i < 9; i++)
    {
        if(i%3==0)
        {
            position.y+=10;
        }
        else
        {
            position.y+=5;
        }
        position.x = 16;
        for (size_t j = 0; j < 9; j++)
        {
            if(oldGrid[i][j]!=0)
            {
                char* path = malloc(sizeof(char)*1024);
                sprintf(path, "solver/OldNumber/%d.png", oldGrid[i][j]);
                SDL_Surface *number = IMG_Load(path);
                if (number == NULL)
                    errx(EXIT_FAILURE, "Can't load %s: %s", path, IMG_GetError());
                if(j%3==0)
                {
                    position.x+=10;
                }
                else
                {
                    position.x+=5;
                }
                position.w = 28;
                position.h = 28;
                if (SDL_BlitSurface(number, NULL, img, &position) < 0)
                    errx(EXIT_FAILURE, "%s", SDL_GetError());
                SDL_FreeSurface(number);
                free(path);
                position.x+=28;
            }
            else
            {
                char* path = malloc(sizeof(char)*1024);
                printf("%d",grid[i][j]);
                sprintf(path, "solver/NewNumber/%d.png", grid[i][j]);
                SDL_Surface *number = IMG_Load(path);
                if (number == NULL)
                    errx(EXIT_FAILURE, "Can't load %s: %s", path, IMG_GetError());
                if(j%3==0)
                {
                    position.x+=10;
                }
                else
                {
                    position.x+=5;
                }
                position.w = 28;
                position.h = 28;
                if (SDL_BlitSurface(number, NULL, img, &position) < 0)
                    errx(EXIT_FAILURE, "%s", SDL_GetError());
                SDL_FreeSurface(number);
                free(path);
                position.x+=28;
            }
        }
        position.y+=28;
    }
    SDL_Surface* image = doubleSizeImage(img);
    IMG_SavePNG(image, "solver/grid.png");
    SDL_FreeSurface(img);
    SDL_FreeSurface(image);
}

SDL_Surface* doubleSizeImage(SDL_Surface* image)
{
    SDL_Surface* newImage = SDL_CreateRGBSurface(0, image->w*2, image->h*2, 32, 0, 0, 0, 0);
    SDL_BlitScaled(image, NULL, newImage, NULL);
    return newImage;
}
