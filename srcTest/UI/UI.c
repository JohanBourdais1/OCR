#include "UI.h"

typedef struct {
    GtkImage *image_display;
    GtkImage *image_display2;
    GtkFrame *image_frame2;
    GtkLabel *filename_label;
    GtkLabel *status_label;
    GdkPixbuf *original_pixbuf;
    GtkAdjustment *crop_top;
    GtkAdjustment *crop_bottom;
    GtkAdjustment *crop_left;
    GtkAdjustment *crop_right;
    gchar *current_filename;
    GtkBox *images_container;
    network *net;
} UIWidgets;

static void on_file_selected(GtkFileChooserButton *button, UIWidgets *widgets)
{
    gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(button));
    
    if (filename == NULL) {
        gtk_label_set_text(widgets->status_label, "No file selected");
        gtk_label_set_text(widgets->filename_label, "");
        return;
    }

    GError *error = NULL;
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(filename, 600, 600, TRUE, &error);
    
    if (error != NULL) {
        gchar *msg = g_strdup_printf("Error loading image: %s", error->message);
        gtk_label_set_text(widgets->status_label, msg);
        g_free(msg);
        g_error_free(error);
        g_free(filename);
        return;
    }

    if (pixbuf != NULL) {
        // Store the filename
        if (widgets->current_filename != NULL) {
            g_free(widgets->current_filename);
        }
        widgets->current_filename = g_strdup(filename);
        
        if (widgets->original_pixbuf != NULL) {
            g_object_unref(widgets->original_pixbuf);
        }
        widgets->original_pixbuf = gdk_pixbuf_copy(pixbuf);
        
        gtk_image_set_from_pixbuf(widgets->image_display, pixbuf);
        gtk_label_set_text(widgets->filename_label, filename);
        gtk_label_set_text(widgets->status_label, "Image loaded successfully");
        
        // Reset crop sliders
        gtk_adjustment_set_value(widgets->crop_top, 0);
        gtk_adjustment_set_value(widgets->crop_bottom, 0);
        gtk_adjustment_set_value(widgets->crop_left, 0);
        gtk_adjustment_set_value(widgets->crop_right, 0);
        
        g_object_unref(pixbuf);
    } else {
        gtk_label_set_text(widgets->status_label, "Failed to load image");
    }

    g_free(filename);
}

static void on_crop_value_changed(GtkAdjustment *adjustment __attribute__((unused)), UIWidgets *widgets)
{
    if (widgets->original_pixbuf == NULL) {
        return;
    }

    int orig_width = gdk_pixbuf_get_width(widgets->original_pixbuf);
    int orig_height = gdk_pixbuf_get_height(widgets->original_pixbuf);
    
    int crop_top = (int)gtk_adjustment_get_value(widgets->crop_top);
    int crop_bottom = (int)gtk_adjustment_get_value(widgets->crop_bottom);
    int crop_left = (int)gtk_adjustment_get_value(widgets->crop_left);
    int crop_right = (int)gtk_adjustment_get_value(widgets->crop_right);
    
    int new_width = orig_width - crop_left - crop_right;
    int new_height = orig_height - crop_top - crop_bottom;
    
    if (new_width <= 0 || new_height <= 0) {
        gtk_label_set_text(widgets->status_label, "Invalid crop values");
        return;
    }
    
    GdkPixbuf *cropped = gdk_pixbuf_new_subpixbuf(widgets->original_pixbuf, 
                                                   crop_left, crop_top, 
                                                   new_width, new_height);
    
    if (cropped != NULL) {
        gtk_image_set_from_pixbuf(widgets->image_display, cropped);
        gchar *msg = g_strdup_printf("Crop preview: %dx%d", new_width, new_height);
        gtk_label_set_text(widgets->status_label, msg);
        g_free(msg);
        g_object_unref(cropped);
    }
}

static void on_crop_clicked(GtkButton *button __attribute__((unused)), UIWidgets *widgets)
{
    if (widgets->original_pixbuf == NULL) {
        gtk_label_set_text(widgets->status_label, "No image loaded");
        return;
    }

    int orig_width = gdk_pixbuf_get_width(widgets->original_pixbuf);
    int orig_height = gdk_pixbuf_get_height(widgets->original_pixbuf);
    
    int crop_top = (int)gtk_adjustment_get_value(widgets->crop_top);
    int crop_bottom = (int)gtk_adjustment_get_value(widgets->crop_bottom);
    int crop_left = (int)gtk_adjustment_get_value(widgets->crop_left);
    int crop_right = (int)gtk_adjustment_get_value(widgets->crop_right);
    
    int new_width = orig_width - crop_left - crop_right;
    int new_height = orig_height - crop_top - crop_bottom;
    
    if (new_width <= 0 || new_height <= 0) {
        gtk_label_set_text(widgets->status_label, "Invalid crop values");
        return;
    }
    
    GdkPixbuf *cropped = gdk_pixbuf_new_subpixbuf(widgets->original_pixbuf,
                                                   crop_left, crop_top,
                                                   new_width, new_height);
    
    if (cropped != NULL) {
        // Replace original with cropped
        g_object_unref(widgets->original_pixbuf);
        widgets->original_pixbuf = gdk_pixbuf_copy(cropped);
        
        gtk_image_set_from_pixbuf(widgets->image_display, cropped);
        
        // Reset sliders
        gtk_adjustment_set_value(widgets->crop_top, 0);
        gtk_adjustment_set_value(widgets->crop_bottom, 0);
        gtk_adjustment_set_value(widgets->crop_left, 0);
        gtk_adjustment_set_value(widgets->crop_right, 0);
        
        gchar *msg = g_strdup_printf("Image cropped to %dx%d", new_width, new_height);
        gtk_label_set_text(widgets->status_label, msg);
        g_free(msg);
        g_object_unref(cropped);
    }
}

void canny_edge(int argc, char *argv[])
{
   char *infilename = NULL;  /* Name of the input image */
   char *dirfilename = NULL; /* Name of the output gradient direction image */
   char outfilename[128];    /* Name of the output "edge" image */
   char composedfname[128];  /* Name of the output "direction" image */
   unsigned char *image;     /* The input image */
   unsigned char *edge;      /* The output edge image */
   int rows, cols;           /* The dimensions of the image. */
   float sigma,              /* Standard deviation of the gaussian kernel. */
	 tlow,               /* Fraction of the high threshold in hysteresis. */
	 thigh;              /* High hysteresis threshold control. The actual
			        threshold is the (100 * thigh) percentage point
			        in the histogram of the magnitude of the
			        gradient image that passes non-maximal
			        suppression. */

   /****************************************************************************
   * Get the command line arguments.
   ****************************************************************************/
   if(argc < 5){
   fprintf(stderr,"\n<USAGE> %s image sigma tlow thigh [writedirim]\n",argv[0]);
      fprintf(stderr,"\n      image:      An image to process. Must be in ");
      fprintf(stderr,"PGM format.\n");
      fprintf(stderr,"      sigma:      Standard deviation of the gaussian");
      fprintf(stderr," blur kernel.\n");
      fprintf(stderr,"      tlow:       Fraction (0.0-1.0) of the high ");
      fprintf(stderr,"edge strength threshold.\n");
      fprintf(stderr,"      thigh:      Fraction (0.0-1.0) of the distribution");
      fprintf(stderr," of non-zero edge\n                  strengths for ");
      fprintf(stderr,"hysteresis. The fraction is used to compute\n");
      fprintf(stderr,"                  the high edge strength threshold.\n");
      fprintf(stderr,"      writedirim: Optional argument to output ");
      fprintf(stderr,"a floating point");
      fprintf(stderr," direction image.\n\n");
      exit(1);
   }

   infilename = argv[1];
   sigma = atof(argv[2]);
   tlow = atof(argv[3]);
   thigh = atof(argv[4]);

   //sigma = calculate_global_sigma(image, rows, cols, 3);
   //calculate_thresholds(image, rows, cols, &tlow, &thigh, sigma);

   //printf("sigma = %f and tlow = %f and thigh = %f\n",sigma,tlow,thigh);

   if(argc == 6) dirfilename = infilename;
   else dirfilename = NULL;

   /****************************************************************************
   * Read in the image. This read function allocates memory for the image.
   ****************************************************************************/
   if(read_pgm_image(infilename, &image, &rows, &cols) == 0){
      fprintf(stderr, "Error reading the input image, %s.\n", infilename);
      exit(1);
   }

   /****************************************************************************
   * Perform the edge detection. All of the work takes place here.
   ****************************************************************************/
   if(dirfilename != NULL){
      sprintf(composedfname, "%s_s_%3.2f_l_%3.2f_h_%3.2f.fim", infilename,sigma, tlow, thigh);
      dirfilename = composedfname;
   }
   canny(image, rows, cols, sigma, tlow, thigh, &edge, dirfilename);

   /****************************************************************************
   * Write out the edge image to a file.
   ****************************************************************************/
   sprintf(outfilename, "ready.pgm");
   if(write_pgm_image(outfilename, edge, rows, cols, "", 255) == 0){
      fprintf(stderr, "Error writing the edge image, %s.\n", outfilename);
      exit(1);
   }
   free(image);
   free(edge);
}

// Convert image to grayscale PGM format
static int convert_to_pgm(const char *input_file, const char *output_file)
{
    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file(input_file, NULL);
    if (pixbuf == NULL) {
        fprintf(stderr, "Failed to load image: %s\n", input_file);
        return 0;
    }

    int width = gdk_pixbuf_get_width(pixbuf);
    int height = gdk_pixbuf_get_height(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);
    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    // Create grayscale image
    unsigned char *gray_image = (unsigned char *)malloc(width * height);
    if (gray_image == NULL) {
        g_object_unref(pixbuf);
        return 0;
    }

    // Convert to grayscale
    for (int y = 0; y < height; y++) {
        guchar *row = pixels + y * rowstride;
        for (int x = 0; x < width; x++) {
            guchar r = row[x * n_channels];
            guchar g = row[x * n_channels + 1];
            guchar b = row[x * n_channels + 2];
            // Standard grayscale conversion
            gray_image[y * width + x] = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
        }
    }

    // Write PGM file
    FILE *fp = fopen(output_file, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to create output file: %s\n", output_file);
        free(gray_image);
        g_object_unref(pixbuf);
        return 0;
    }

    fprintf(fp, "P5\n");
    fprintf(fp, "%d %d\n", width, height);
    fprintf(fp, "255\n");
    fwrite(gray_image, 1, width * height, fp);
    fclose(fp);

    free(gray_image);
    g_object_unref(pixbuf);
    return 1;
}

// Convert PGM image to PNG format
static int convert_pgm_to_png(const char *pgm_file, const char *png_file)
{
    FILE *fp = fopen(pgm_file, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open PGM file: %s\n", pgm_file);
        return 0;
    }

    // Read PGM header with comment support
    char magic[3];
    int width, height, maxval;
    char line[256];
    
    // Read magic number
    if (fgets(line, sizeof(line), fp) == NULL) {
        fprintf(stderr, "Failed to read magic number\n");
        fclose(fp);
        return 0;
    }
    sscanf(line, "%2s", magic);
    
    if (strcmp(magic, "P5") != 0 && strcmp(magic, "P2") != 0) {
        fprintf(stderr, "Invalid PGM format (expected P5 or P2, got %s)\n", magic);
        fclose(fp);
        return 0;
    }

    // Read width and height (skip comments)
    do {
        if (fgets(line, sizeof(line), fp) == NULL) {
            fprintf(stderr, "Failed to read dimensions\n");
            fclose(fp);
            return 0;
        }
    } while (line[0] == '#');
    sscanf(line, "%d %d", &width, &height);

    // Read maxval (skip comments)
    do {
        if (fgets(line, sizeof(line), fp) == NULL) {
            fprintf(stderr, "Failed to read maxval\n");
            fclose(fp);
            return 0;
        }
    } while (line[0] == '#');
    sscanf(line, "%d", &maxval);

    // Read image data
    unsigned char *gray_data = (unsigned char *)malloc(width * height);
    if (gray_data == NULL) {
        fclose(fp);
        return 0;
    }

    if (fread(gray_data, 1, width * height, fp) != (size_t)(width * height)) {
        fprintf(stderr, "Failed to read PGM data (expected %d bytes)\n", width * height);
        free(gray_data);
        fclose(fp);
        return 0;
    }
    fclose(fp);

    // Create GdkPixbuf from grayscale data
    GdkPixbuf *pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
    if (pixbuf == NULL) {
        free(gray_data);
        return 0;
    }

    guchar *pixels = gdk_pixbuf_get_pixels(pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride(pixbuf);

    // Convert grayscale to RGB
    for (int y = 0; y < height; y++) {
        guchar *row = pixels + y * rowstride;
        for (int x = 0; x < width; x++) {
            unsigned char gray = gray_data[y * width + x];
            row[x * 3] = gray;      // R
            row[x * 3 + 1] = gray;  // G
            row[x * 3 + 2] = gray;  // B
        }
    }

    // Save as PNG
    GError *error = NULL;
    gboolean success = gdk_pixbuf_save(pixbuf, png_file, "png", &error, NULL);
    
    if (!success) {
        if (error != NULL) {
            fprintf(stderr, "Failed to save PNG: %s\n", error->message);
            g_error_free(error);
        }
        g_object_unref(pixbuf);
        free(gray_data);
        return 0;
    }

    g_object_unref(pixbuf);
    free(gray_data);
    return 1;
}


static void on_pretreatment_clicked(GtkButton *button __attribute__((unused)), UIWidgets *widgets)
{
    if (widgets->current_filename == NULL) {
        gtk_label_set_text(widgets->status_label, "No image loaded");
        return;
    }

    // Create second image frame only if it doesn't exist
    if (widgets->image_display2 == NULL) {
        // Second image frame
        widgets->image_frame2 = GTK_FRAME(gtk_frame_new("Ready Canny"));
        gtk_frame_set_label_align(widgets->image_frame2, 0.5, 0.5);
        
        GtkBox *image_box2 = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
        gtk_container_add(GTK_CONTAINER(widgets->image_frame2), GTK_WIDGET(image_box2));

        // Second image display
        widgets->image_display2 = GTK_IMAGE(gtk_image_new_from_icon_name("image-missing", GTK_ICON_SIZE_DIALOG));
        gtk_image_set_pixel_size(widgets->image_display2, 400);
        gtk_box_pack_start(image_box2, GTK_WIDGET(widgets->image_display2), TRUE, TRUE, 10);

        gtk_box_pack_start(widgets->images_container, GTK_WIDGET(widgets->image_frame2), TRUE, TRUE, 0);
        
        // Show the new widgets
        gtk_widget_show_all(GTK_WIDGET(widgets->image_frame2));
    }
    
    gtk_label_set_text(widgets->status_label, "Applying pretreatment...");
    
    // Call pretreatment with the filename
    // TODO: Add your pretreatment logic here
    // You have access to the filename via: widgets->current_filename
    gchar *msg = g_strdup_printf("Pretreatment on: %s", widgets->current_filename);
    gtk_label_set_text(widgets->status_label, msg);
    g_free(msg);

    
   double angle = 0.0;
    // - Initialize the SDL.
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());
    // - Create a surface from the colored image.
    SDL_Surface* pre_surface = IMG_Load(widgets->current_filename);
    SDL_Surface* surface = SDL_ConvertSurfaceFormat(pre_surface, SDL_PIXELFORMAT_RGB888, 0);
    SDL_FreeSurface(pre_surface);
    
    // Convert image to PGM without using system()
    if (!convert_to_pgm(widgets->current_filename, "ready.pgm")) {
        gtk_label_set_text(widgets->status_label, "Failed to convert image to PGM");
        SDL_FreeSurface(surface);
        SDL_Quit();
        return;
    }
    
    printf("Image converted to ready.pgm\n");
	canny_edge(5,(char*[]){"./main", "ready.pgm","1.0","0.95","0.475"});
    
    // Convert PGM to PNG without using system()
    if (!convert_pgm_to_png("ready.pgm", "carving/ready_canny.png")) {
        gtk_label_set_text(widgets->status_label, "Failed to convert PGM to PNG");
        SDL_Quit();
        return;
    }
    
    printf("PGM converted to PNG\n");
    // - rotation
	SDL_Surface* source = IMG_Load("carving/ready_canny.png");
	SDL_Point Points[4];
	SDL_Point intersection[10][10];
	carve2(source,Points,intersection);
	SDL_Point tl = Points[0];
	SDL_Point tr = Points[1];
	SDL_Point bl = Points[2];
	SDL_Point br = Points[3];
	angle = 90.0 - angle_detection(tl.x,tl.y,tr.x,tr.y,bl.x,bl.y,br.x,br.y);

    // - grayscal
    surface_to_grayscale(surface);
    // - cantrast
    contrast_streching(surface);

    SDL_Surface* rotated_surface =
    rotateImage(surface, 360 - fmod(angle,360.0));
    surface_to_black_and_white(surface);
    IMG_SavePNG(rotated_surface,"carving/ready.png");
    // - Free the surface.
    SDL_FreeSurface(surface);
    SDL_FreeSurface(rotated_surface);
    // - Destroy thif (SDL_Init(SDL_INIT_VIDEO) != 0)
    SDL_Quit();
    
    // Load and display the two resulting images
    GError *error = NULL;
    GdkPixbuf *pixbuf1 = gdk_pixbuf_new_from_file_at_scale("carving/ready.png", 400, 400, TRUE, &error);
    if (pixbuf1 != NULL) {
        gtk_image_set_from_pixbuf(widgets->image_display, pixbuf1);
        g_object_unref(pixbuf1);
    }
    
    error = NULL;
    GdkPixbuf *pixbuf2 = gdk_pixbuf_new_from_file_at_scale("carving/ready_canny.png", 400, 400, TRUE, &error);
    if (pixbuf2 != NULL) {
        gtk_image_set_from_pixbuf(widgets->image_display2, pixbuf2);
        g_object_unref(pixbuf2);
    }
    
    gtk_label_set_text(widgets->status_label, "Pretreatment completed - Images displayed");
}

static void on_grid_cutting_clicked(GtkButton *button __attribute__((unused)), UIWidgets *widgets)
{
    gtk_label_set_text(widgets->status_label, "Cutting grid...");
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());

    if (IMG_Init(IMG_INIT_PNG) == 0)
        errx(EXIT_FAILURE, "/!\\ IMG_Init() failure");

    if (system("rm -rf network/digitreconizer/grid") != 0)
        errx(EXIT_FAILURE, "/!\\ system(\"rm -rf\") failure");

    // Image loading
    SDL_Surface* ready;
    if ((ready = IMG_Load("carving/ready.png")) == NULL)
        errx(EXIT_FAILURE, "/!\\ IMG_Load() failure");
    SDL_Surface* canny;
    if ((canny = IMG_Load("carving/ready_canny.png")) == NULL)
    	errx(EXIT_FAILURE, "/!\\ IMG_Load() failure");

    // Subsection points of the 9x9 grid
    SDL_Point intersections[10][10];

    // Detection of the grid and recuperation of the intersection points
    // with the image modified by the canny algorithm
    carve(canny, intersections);

    // Creations of the tiles of the grid in "grid/".
    cut(ready, intersections);

    SDL_FreeSurface(canny);
    SDL_FreeSurface(ready);

    IMG_Quit();
    SDL_Quit();
    
    // Display border.png in the second image frame
    if (widgets->image_display2 != NULL) {
        GError *error = NULL;
        GdkPixbuf *border_pixbuf = gdk_pixbuf_new_from_file_at_scale("border.png", 400, 400, TRUE, &error);
        if (border_pixbuf != NULL) {
            gtk_image_set_from_pixbuf(widgets->image_display2, border_pixbuf);
            g_object_unref(border_pixbuf);
            gtk_label_set_text(widgets->status_label, "Grid cutting completed - Border displayed");
        } else {
            gtk_label_set_text(widgets->status_label, "Grid cutting completed - Border image not found");
        }
    }
}

static void on_solve_clicked(GtkButton *button __attribute__((unused)), UIWidgets *widgets)
{
    gtk_label_set_text(widgets->status_label, "Solving sudoku...");
    create_grid(widgets->net, "grid/");
    int **grid = readGridFromFile("sudoku_grid.txt");
    if (grid == NULL) {
        gtk_label_set_text(widgets->status_label, "Failed to read sudoku grid");
        return;
    }
    int oldGrid[9][9];
        for(int i=0;i<9;i++)
            for(int j=0;j<9;j++)
                oldGrid[i][j]=grid[i][j];
    if(isGridValid(grid)==0)
    {
        gtk_label_set_text(widgets->status_label, "The Sudoku grid is invalid");
        return;
    }
    if (solvingSudoku(grid)) {
        writeGrid("solver/sudoku_solution.txt", grid);
        gtk_label_set_text(widgets->status_label, "Sudoku solved! Solution saved to sudoku_solution.txt");
    } else {
        gtk_label_set_text(widgets->status_label, "No solution exists for the given Sudoku");
    }
    recreateGrid(grid,oldGrid);
    
    // Display the solved grid image
    GdkPixbuf *solved_pixbuf = gdk_pixbuf_new_from_file("solver/grid.png", NULL);
    if (solved_pixbuf != NULL) {
        GdkPixbuf *scaled_pixbuf = gdk_pixbuf_scale_simple(solved_pixbuf, 400, 400, GDK_INTERP_BILINEAR);
        gtk_image_set_from_pixbuf(widgets->image_display2, scaled_pixbuf);
        g_object_unref(scaled_pixbuf);
        g_object_unref(solved_pixbuf);
    }
}

static void on_clear_clicked(GtkButton *button __attribute__((unused)), UIWidgets *widgets)
{
    gtk_image_set_from_icon_name(widgets->image_display, "image-missing", GTK_ICON_SIZE_DIALOG);
    gtk_label_set_text(widgets->filename_label, "No file selected");
    gtk_label_set_text(widgets->status_label, "Image cleared");
    
    if (widgets->original_pixbuf != NULL) {
        g_object_unref(widgets->original_pixbuf);
        widgets->original_pixbuf = NULL;
    }
    
    if (widgets->current_filename != NULL) {
        g_free(widgets->current_filename);
        widgets->current_filename = NULL;
    }
    
    // Remove the second image frame if it exists
    if (widgets->image_frame2 != NULL) {
        gtk_widget_destroy(GTK_WIDGET(widgets->image_frame2));
        widgets->image_frame2 = NULL;
        widgets->image_display2 = NULL;
    }
}

static void on_quit_clicked(GtkButton *button __attribute__((unused)), gpointer data __attribute__((unused)))
{
    gtk_main_quit();
}

static void on_window_destroy(GtkWidget *widget __attribute__((unused)), gpointer data)
{
    UIWidgets *widgets = (UIWidgets *)data;
    if (widgets->original_pixbuf != NULL) {
        g_object_unref(widgets->original_pixbuf);
    }
    if (widgets->current_filename != NULL) {
        g_free(widgets->current_filename);
    }
    g_free(widgets);
    gtk_main_quit();
}

void mainUI(network *net)
{
    GtkWindow *window;
    GtkBox *main_box;
    GtkBox *content_box;
    GtkBox *sidebar_box;
    GtkBox *button_box;
    GtkBox *image_box;
    GtkFileChooserButton *file_button;
    GtkButton *solve_button;
    GtkButton *clear_button;
    GtkButton *quit_button;
    GtkButton *pretreatment_button;
    GtkButton *grid_cutting_button;
    GtkButton *crop_button;
    GtkImage *image_display;
    GtkLabel *filename_label;
    GtkLabel *status_label;
    GtkFrame *image_frame;
    GtkFrame *status_frame;

    UIWidgets *widgets = g_malloc(sizeof(UIWidgets));
    widgets->original_pixbuf = NULL;
    widgets->current_filename = NULL;
    widgets->image_display2 = NULL;
    widgets->image_frame2 = NULL;
    widgets->net = net;

    gtk_init(0, NULL);

    // Create main window
    window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    gtk_window_set_title(window, "Sudoku OCR Solver");
    gtk_window_set_default_size(window, 1500, 1000);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    // Main vertical box
    main_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 10));
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(main_box));

    // Title label
    GtkLabel *title = GTK_LABEL(gtk_label_new(NULL));
    gtk_label_set_markup(title, "<b><big>Sudoku Solver</big></b>");
    gtk_box_pack_start(main_box, GTK_WIDGET(title), FALSE, FALSE, 0);

    // File chooser button
    file_button = GTK_FILE_CHOOSER_BUTTON(gtk_file_chooser_button_new(
        "Select Sudoku Image",
        GTK_FILE_CHOOSER_ACTION_OPEN
    ));
    
    // Add file filters
    GtkFileFilter *filter_images = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_images, "Image Files");
    gtk_file_filter_add_pattern(filter_images, "*.png");
    gtk_file_filter_add_pattern(filter_images, "*.jpg");
    gtk_file_filter_add_pattern(filter_images, "*.jpeg");
    gtk_file_filter_add_pattern(filter_images, "*.bmp");
    gtk_file_filter_add_pattern(filter_images, "*.gif");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_button), filter_images);

    GtkFileFilter *filter_all = gtk_file_filter_new();
    gtk_file_filter_set_name(filter_all, "All Files");
    gtk_file_filter_add_pattern(filter_all, "*");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_button), filter_all);

    gtk_box_pack_start(main_box, GTK_WIDGET(file_button), FALSE, FALSE, 0);

    // Filename label
    filename_label = GTK_LABEL(gtk_label_new("No file selected"));
    gtk_label_set_line_wrap(filename_label, TRUE);
    gtk_box_pack_start(main_box, GTK_WIDGET(filename_label), FALSE, FALSE, 0);

    // Content box (horizontal: sidebar + image)
    content_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10));
    gtk_box_pack_start(main_box, GTK_WIDGET(content_box), TRUE, TRUE, 0);

    // Sidebar (left)
    sidebar_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 10));
    gtk_box_set_homogeneous(sidebar_box, FALSE);
    gtk_box_pack_start(content_box, GTK_WIDGET(sidebar_box), FALSE, FALSE, 0);

    // Crop button in sidebar
    crop_button = GTK_BUTTON(gtk_button_new_with_label("Apply Crop"));
    gtk_widget_set_size_request(GTK_WIDGET(crop_button), 120, -1);
    gtk_box_pack_start(sidebar_box, GTK_WIDGET(crop_button), FALSE, FALSE, 0);

    // Crop controls
    GtkLabel *crop_label = GTK_LABEL(gtk_label_new("Crop Values:"));
    gtk_box_pack_start(sidebar_box, GTK_WIDGET(crop_label), FALSE, FALSE, 0);

    // Top crop
    GtkLabel *top_label = GTK_LABEL(gtk_label_new("Top:"));
    gtk_box_pack_start(sidebar_box, GTK_WIDGET(top_label), FALSE, FALSE, 0);
    GtkAdjustment *crop_top_adj = gtk_adjustment_new(0, 0, 300, 5, 10, 0);
    GtkScale *crop_top_scale = GTK_SCALE(gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, crop_top_adj));
    gtk_scale_set_draw_value(crop_top_scale, TRUE);
    gtk_scale_set_value_pos(crop_top_scale, GTK_POS_RIGHT);
    gtk_box_pack_start(sidebar_box, GTK_WIDGET(crop_top_scale), FALSE, FALSE, 0);
    widgets->crop_top = crop_top_adj;

    // Bottom crop
    GtkLabel *bottom_label = GTK_LABEL(gtk_label_new("Bottom:"));
    gtk_box_pack_start(sidebar_box, GTK_WIDGET(bottom_label), FALSE, FALSE, 0);
    GtkAdjustment *crop_bottom_adj = gtk_adjustment_new(0, 0, 300, 5, 10, 0);
    GtkScale *crop_bottom_scale = GTK_SCALE(gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, crop_bottom_adj));
    gtk_scale_set_draw_value(crop_bottom_scale, TRUE);
    gtk_scale_set_value_pos(crop_bottom_scale, GTK_POS_RIGHT);
    gtk_box_pack_start(sidebar_box, GTK_WIDGET(crop_bottom_scale), FALSE, FALSE, 0);
    widgets->crop_bottom = crop_bottom_adj;

    // Left crop
    GtkLabel *left_label = GTK_LABEL(gtk_label_new("Left:"));
    gtk_box_pack_start(sidebar_box, GTK_WIDGET(left_label), FALSE, FALSE, 0);
    GtkAdjustment *crop_left_adj = gtk_adjustment_new(0, 0, 300, 5, 10, 0);
    GtkScale *crop_left_scale = GTK_SCALE(gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, crop_left_adj));
    gtk_scale_set_draw_value(crop_left_scale, TRUE);
    gtk_scale_set_value_pos(crop_left_scale, GTK_POS_RIGHT);
    gtk_box_pack_start(sidebar_box, GTK_WIDGET(crop_left_scale), FALSE, FALSE, 0);
    widgets->crop_left = crop_left_adj;

    // Right crop
    GtkLabel *right_label = GTK_LABEL(gtk_label_new("Right:"));
    gtk_box_pack_start(sidebar_box, GTK_WIDGET(right_label), FALSE, FALSE, 0);
    GtkAdjustment *crop_right_adj = gtk_adjustment_new(0, 0, 300, 5, 10, 0);
    GtkScale *crop_right_scale = GTK_SCALE(gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, crop_right_adj));
    gtk_scale_set_draw_value(crop_right_scale, TRUE);
    gtk_scale_set_value_pos(crop_right_scale, GTK_POS_RIGHT);
    gtk_box_pack_start(sidebar_box, GTK_WIDGET(crop_right_scale), FALSE, FALSE, 0);
    widgets->crop_right = crop_right_adj;

    // Add stretch space
    gtk_box_pack_start(sidebar_box, gtk_box_new(GTK_ORIENTATION_VERTICAL, 0), TRUE, TRUE, 0);

    // Image frames container (horizontal box for two images)
    GtkBox *images_container = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10));
    gtk_box_pack_start(content_box, GTK_WIDGET(images_container), TRUE, TRUE, 0);

    // First image frame
    image_frame = GTK_FRAME(gtk_frame_new("Original/Ready"));
    gtk_frame_set_label_align(image_frame, 0.45, 0.5);
    
    image_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL, 0));
    gtk_container_add(GTK_CONTAINER(image_frame), GTK_WIDGET(image_box));

    // Image display
    image_display = GTK_IMAGE(gtk_image_new_from_icon_name("image-missing", GTK_ICON_SIZE_DIALOG));
    gtk_image_set_pixel_size(image_display, 400);
    gtk_box_pack_start(image_box, GTK_WIDGET(image_display), TRUE, TRUE, 10);

    gtk_box_pack_start(images_container, GTK_WIDGET(image_frame), TRUE, TRUE, 0);

    // Status frame
    status_frame = GTK_FRAME(gtk_frame_new("Status"));
    gtk_frame_set_label_align(status_frame, 0.5, 0.5);
    
    status_label = GTK_LABEL(gtk_label_new("Ready"));
    gtk_label_set_line_wrap(status_label, TRUE);
    gtk_container_add(GTK_CONTAINER(status_frame), GTK_WIDGET(status_label));
    
    gtk_box_pack_start(main_box, GTK_WIDGET(status_frame), FALSE, FALSE, 0);

    // Button box
    button_box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10));
    gtk_box_set_homogeneous(button_box, TRUE);

    pretreatment_button = GTK_BUTTON(gtk_button_new_with_label("Pretreatment"));
    gtk_box_pack_start(button_box, GTK_WIDGET(pretreatment_button), TRUE, TRUE, 0);

    grid_cutting_button = GTK_BUTTON(gtk_button_new_with_label("Grid Cutting"));
    gtk_box_pack_start(button_box, GTK_WIDGET(grid_cutting_button), TRUE, TRUE, 0);

    solve_button = GTK_BUTTON(gtk_button_new_with_label("Solve Sudoku"));
    gtk_box_pack_start(button_box, GTK_WIDGET(solve_button), TRUE, TRUE, 0);

    clear_button = GTK_BUTTON(gtk_button_new_with_label("Clear Image"));
    gtk_box_pack_start(button_box, GTK_WIDGET(clear_button), TRUE, TRUE, 0);

    quit_button = GTK_BUTTON(gtk_button_new_with_label("Quit"));
    gtk_box_pack_start(button_box, GTK_WIDGET(quit_button), TRUE, TRUE, 0);

    gtk_box_pack_start(main_box, GTK_WIDGET(button_box), FALSE, FALSE, 0);

    // Store widgets in struct
    widgets->image_display = image_display;
    widgets->filename_label = filename_label;
    widgets->status_label = status_label;
    widgets->images_container = images_container;

    // Connect signals
    g_signal_connect(file_button, "file-set",
                     G_CALLBACK(on_file_selected), widgets);
    
    g_signal_connect(crop_top_adj, "value-changed",
                     G_CALLBACK(on_crop_value_changed), widgets);
    g_signal_connect(crop_bottom_adj, "value-changed",
                     G_CALLBACK(on_crop_value_changed), widgets);
    g_signal_connect(crop_left_adj, "value-changed",
                     G_CALLBACK(on_crop_value_changed), widgets);
    g_signal_connect(crop_right_adj, "value-changed",
                     G_CALLBACK(on_crop_value_changed), widgets);
    
    g_signal_connect(crop_button, "clicked",
                     G_CALLBACK(on_crop_clicked), widgets);
    
    g_signal_connect(pretreatment_button, "clicked",
                     G_CALLBACK(on_pretreatment_clicked), widgets);
    
    g_signal_connect(grid_cutting_button, "clicked",
                     G_CALLBACK(on_grid_cutting_clicked), widgets);
    
    g_signal_connect(solve_button, "clicked",
                     G_CALLBACK(on_solve_clicked), widgets);
    
    g_signal_connect(clear_button, "clicked",
                     G_CALLBACK(on_clear_clicked), widgets);
    
    g_signal_connect(quit_button, "clicked",
                     G_CALLBACK(on_quit_clicked), NULL);
    
    g_signal_connect(window, "destroy",
                     G_CALLBACK(on_window_destroy), widgets);

    gtk_widget_show_all(GTK_WIDGET(window));
    gtk_main();
}
