#include <stdlib.h>
#include <gtk/gtk.h>
#include "UI.h"


void file_selected(GtkFileChooser* filechooser,gpointer data)
{
	system("make clean");
	system("make all");
	struct resize* pre = (struct resize*) data;
	gchar* file = gtk_file_chooser_get_filename(filechooser);
	SDL_Surface* s = IMG_Load(file);
	SDL_Surface *sr = SDL_ConvertSurfaceFormat(s, SDL_PIXELFORMAT_RGB888, 0);
    SDL_FreeSurface(s);
    if (sr == NULL)
    {
        printf("Erreur lors du chargement de l'image\n");
        exit(1);
    }
    SDL_Surface* resized = SDL_CreateRGBSurface(0, sr->w*0.55, sr->h*0.55, 32, 0, 0, 0, 0);
    if (resized == NULL)
    {
        printf("Erreur lors de l'allocation de la mémoire\n");
        exit(1);
    }
    SDL_BlitScaled(sr, NULL, resized, NULL);
    SDL_SaveBMP(resized,"../assets/image.png");
    SDL_FreeSurface(sr);
    SDL_FreeSurface(resized);
	GtkImage *image = pre->Image;
	gtk_image_set_from_file(image,"../assets/image.png");
	g_free(file);
}

void pretreatment(GtkButton* button,gpointer* data)
{
	struct info* pre = (struct info*) data;
	const gchar* value = gtk_entry_get_text(pre->Entry);
	char  arg[1024];
	if(value == NULL)
		sprintf(arg,"./pre pretraitement/grid_resize.png");
	else
		sprintf(arg,"./pre pretraitement/grid_resize.png %s",value);
	int statue = system(arg);
	if(statue == -1)
	{
		errx(1,"Pretreatment error");
	}
	GtkImage* Image = pre->Image;
	gtk_image_set_from_file(Image,"carving/ready.png");
}

void carving(GtkButton* button,gpointer data)
{
	int statue = system("./carvin");
	if(statue == -1)
	{
		errx(1,"carving error");
	}
	GtkImage* Image = data;
	gtk_image_set_from_file(Image,"carving/border.png");
}

void solver(GtkButton* button,gpointer data)
{
	struct info* pre = (struct info*) data;
	if(gtk_toggle_button_get_active(pre->Tog))
	{
		const gchar* value = gtk_entry_get_text(pre->Entry);
		char* val = g_strdup_printf("%s",value);
		FILE* file = fopen("solver/grid","w");
		if(file == NULL)
		{
			errx(1,"impossible d'ouvrir le fichier");
		}
		if(fprintf(file,"%s",val)==-1)
		{
			errx(1,"error printf");
		}
		fclose(file);
		int statu = system("./solve2");
		if(statu == -1)
			errx(1,"manual solver error");
	}
	else
	{
		system("./solve solve");
	}
	GtkImage *image = pre->Image;
	gtk_image_set_from_file(image,"solver/grid.png");
}

void risize(GtkButton* button,gpointer data)
{
	GValue value = {0,};
	g_value_init(&value,G_TYPE_STRING);
	struct resize* size = (struct resize*) data;
	int width = 0;
	int height = 0;
	g_object_get_property(G_OBJECT(size->Image),"file",&value);
	size->file = g_value_get_string(&value);
	SDL_Surface* source = IMG_Load(size->file);
	SDL_Surface* s = SDL_ConvertSurfaceFormat(source,SDL_PIXELFORMAT_RGB888,0);
	height = s->h;
	width = s->w;
	double l = (double)gtk_adjustment_get_value(size->L);
	double r = (double)gtk_adjustment_get_value(size->R);
	double h = (double)gtk_adjustment_get_value(size->H);
	double b = (double)gtk_adjustment_get_value(size->B);
	SDL_Surface* res =slider_cut(s,(l/100)*width,(((100-r)/100)*width),(h/100)*height,
			((100-b)/100)*height);
	SDL_SaveBMP(res,"pretraitement/grid_resize.png");
	GtkImage* image = size->Image;
	gtk_image_set_from_file(image,"pretraitement/grid_resize.png");
	size->file = "pretraitement/grid_resize.png";
	SDL_FreeSurface(source);
	SDL_FreeSurface(res);
}
