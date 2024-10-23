#ifndef UI_H
#define UI_H
#include <stdlib.h>
#include <gtk/gtk.h>
#include <err.h>
#include <unistd.h>
#include <glib.h>
#include "../solver/sudoku.h"
#include "../network/digitreconizer/network.h"
#include "../carving/carving.h"
//#include "../src/pretraitement/pre.h"

typedef struct info info;
struct info
{
    GtkImage* Image;
    GtkFileChooser* Loader;
    GtkEntry* Entry;
    GtkToggleButton* Tog;
};

typedef struct resize resize;
struct resize
{
	GtkImage* Image;
	GtkAdjustment* H;
	GtkAdjustment* L;
	GtkAdjustment* R;
	GtkAdjustment* B;
	gchar* file;
};


void file_selected(GtkFileChooser* filechooser,gpointer data);

void pretreatment(GtkButton* button,gpointer* data);

void carving(GtkButton* button,gpointer data);

void solver(GtkButton* button,gpointer data);

void risize(GtkButton* button,gpointer data);
#endif
