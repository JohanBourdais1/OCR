#include "solver/sudoku.h"
#include "network/digitreconizer/network.h"
#include "carving/carving.h"
#include "UI/UI.h"
#include "pretraitement/pre.h"
#include "canny/canny_edge.h"

void solve(int argc,char *argv[]);
void solve2();
void carvin();
void pretraitement(int argc,char *argv[]);
void ui(int argc, char* argv[]);
void canny_edge(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    if (strcmp(argv[0],"./solve")==0)
    {
        solve(argc,argv);
    }
    else if (strcmp(argv[0],"./solve2")==0)
    {
        solve2();
    }
    else if (strcmp(argv[0],"./pre")==0)
    {
        pretraitement(argc,argv);
    }
    else if(strcmp(argv[0],"./carvin")==0)
    {
	carvin();
    }
    else if(strcmp(argv[0],"./ui")==0)
    {
	ui(argc,argv);
    }
    else if(strcmp(argv[0],"./canny_edge")==0)
    {
	canny_edge(argc,argv);
    }
    return 0;
}

void solve(int argc,char *argv[])
{
    srand(time(NULL));
    if (argc==2 && strcmp(argv[1],"train")==0)
    {
        struct network* net = init_network();
        load_network("network/digitreconizer/network1.txt",net);
        double** test = create_test("network/digitreconizer/testdata1");
        train_network(net, test);
        save_network("network/digitreconizer/network1.txt",net);
	    freeNetwork(net);
        for (size_t i = 0; i < 50; i++)
        {
            free(test[i]);
        }
    free(test);
    }
    else if (argc==2 && strcmp(argv[1],"solve")==0)
    {
        struct network* net = init_network();
        load_network("network/digitreconizer/network1.txt",net);
        double** grid = create_grid("network/digitreconizer/grid");
        double** gridres = readGrid(net,grid);
        write_grid("solver/grid",gridres);
        freeGrid(grid);
        freeSudoku(gridres);
        int** sudoku = readGridFromFile("solver/grid");
	freeNetwork(net);
	if(isGridValid(sudoku)==0)
	{
		errno = -1;
		errx(1,"grille pas resolvable");
	}
        int oldGrid[9][9];
        for(int i=0;i<9;i++)
            for(int j=0;j<9;j++)
                oldGrid[i][j]=sudoku[i][j];

        if (solvingSudoku(sudoku)==1) 
        {
            writeGrid("solver/grid.result",sudoku);
            recreateGrid(sudoku,oldGrid);
        }
        else
	{
	    errno = -1;
            errx(1,"la grille n'est pas resolvable");
	}
        freeSudoku((double**)sudoku);
        //system("rm -rf network/digitreconizer/grid");
    }
    else
    {
        errx(1,"argument invalide");
    }
}

void carvin()
{
    // Module initialisation
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

}


void pretraitement(int argc,char *argv[])
{
    printf("argc = %d\n",argc);
    for (int i = 0 ; i <argc; i++)
    {
	printf("argv = %s\n",*(argv + i));
    }
    // Checks the number of arguments.
    if (argc > 3)
        errx(EXIT_FAILURE, "Usage: use 2 arguments");
    double angle = 0.0;
    // - Initialize the SDL.
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
        errx(EXIT_FAILURE, "%s", SDL_GetError());
    // - Create a surface from the colored image.
    SDL_Surface* surface = load_image(argv[1]);
    // - Convert the surface into black_and_white.
    //surface_to_black_and_white(surface);
    // - cannny
    char  convert[1024];
    int r = sprintf(convert,"convert %s ready.pgm",argv[1]);
    if(r==-1)
	 errx(1,"failed to convert to pgm");
    system(convert);
    system("./canny_edge ready.pgm 1.0 0.95 0.475");
    system("convert ready.pgm carving/ready_canny.png");
    // - rotation
     if(argc == 2)
    {
	    SDL_Surface* source = IMG_Load("carving/ready_canny.png");
		SDL_Point Points[4];
		SDL_Point intersection[10][10];
		carve2(source,Points,intersection);
		SDL_Point tl = Points[0];
		SDL_Point tr = Points[1];
		SDL_Point bl = Points[2];
		SDL_Point br = Points[3];
		angle = 90.0 - angle_detection(tl.x,tl.y,tr.x,tr.y,bl.x,bl.y,br.x,br.y);
    }
    else
    {
		char *ptr;
		double ret;
		ret = strtod(argv[2],&ptr);
		angle =ret;
    }
    // - grayscal
    surface_to_grayscale( surface);
    // - cantrast
    contrast_streching( surface);

    SDL_Surface* rotated_surface =
    rotateImage(surface, 360 - fmod(angle,360.0));
    surface_to_black_and_white(surface);
    IMG_SavePNG(rotated_surface,"carving/ready.png");
    // - Free the surface.
    SDL_FreeSurface(surface);
    SDL_FreeSurface(rotated_surface);
    // - Destroy thif (SDL_Init(SDL_INIT_VIDEO) != 0)
    SDL_Quit();
}

void solve2()
{
    int** sudoku = readGridFromFile("solver/grid");
    if(isGridValid(sudoku)==0)
	{
		errno = -1;
		errx(1,"grille pas resolvable");
	}
    int oldGrid[9][9];
    for(int i=0;i<9;i++)
        for(int j=0;j<9;j++)
            oldGrid[i][j]=sudoku[i][j];

    if (solvingSudoku(sudoku)==1) 
    {
        for(int i=0;i<9;i++)
            for(int j=0;j<9;j++)
                printf("%d",sudoku[i][j]);
        writeGrid("grid.result",sudoku);
        recreateGrid(sudoku,oldGrid);
    }
    else
        errx(1,"la grille n'est pas resolvable");
    freeSudoku((double**)sudoku);
    //system("rm -rf network/digitreconizer/grid");
}
void ui(int argc, char* argv[])
{
	//variables
	GtkWindow* window;
	GtkButton* ExitButton1;
	GtkButton* ExitButton2;
	GtkButton* All;
	GtkButton* ResizeButton;
	GtkButton* SolveButton;
	GtkButton* CarvingButton;
	GtkToggleButton* HandWrite;
	GtkStack* Stack;
	GtkStackSwitcher* Switcher;
	GtkImage*Image;
	GtkFileChooser* Loader;
	GtkAdjustment* AdjHigh;
	GtkAdjustment* AdjBottom;
	GtkAdjustment* AdjLeft;
	GtkAdjustment* AdjRight;
	GtkEntry* Manual_buffer;
	GtkEntry* GridEntry;
	struct info pre_info;
	struct info Solver_info;
	//GdkPixbuf* Imagepixbuf;
	struct resize resize;
	//GValue value = {0,};
	//g_value_init(&value,G_TYPE_STRING);

	//init gtk
	gtk_init(&argc,&argv);

	// Constructs a GtkBuilder instance.
	GtkBuilder* builder = gtk_builder_new ();

	// Loads the UI description.
	// (Exits if an error occurs.)
	GError* error = NULL;
	if (gtk_builder_add_from_file(builder, "UI/Principal_page.glade", &error) == 0)
	{
		g_printerr("Error loading file: %s\n", error->message);
		g_clear_error(&error);
		return ;
	}

	//init the window
	window = GTK_WINDOW(gtk_builder_get_object(builder,"Principal_window"));

	//Init the stack
	Stack = GTK_STACK(gtk_builder_get_object(builder, "Stack"));

	//Init EXIT buttons
	ExitButton1 = GTK_BUTTON(gtk_builder_get_object(builder, "Quit_button"));
	ExitButton2 = GTK_BUTTON(gtk_builder_get_object(builder, "Exit_button"));

	//Init the switcher
	Switcher = GTK_STACK_SWITCHER(gtk_builder_get_object(builder, "Switcher"));
	gtk_stack_switcher_set_stack (Switcher, Stack);

	//Init the image  and the loader
	Image = GTK_IMAGE(gtk_builder_get_object(builder, "Image"));
	resize.Image=Image;
	//g_object_get_property(G_OBJECT(Image),"file",&value);
	Loader = GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "Load_button"));
	//resize.file = g_value_get_string(&value);
	//Init the sizer
	ResizeButton = GTK_BUTTON(gtk_builder_get_object(builder, "ResizeButton"));
	AdjHigh = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adjustment1"));
	AdjLeft = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adjustment2"));
	AdjBottom = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adjustment3"));
	AdjRight = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "adjustment4"));
	resize.H = AdjHigh;
	resize.L = AdjLeft;
	resize.R = AdjRight;
	resize.B = AdjBottom;
	//Init traitement function
	All = GTK_BUTTON(gtk_builder_get_object(builder, "All_button"));
	Manual_buffer = GTK_ENTRY(gtk_builder_get_object(builder,
	"Manual_buffer"));
	pre_info.Entry = Manual_buffer;
	//pre_info.Loader = Loader;
	pre_info.Image = Image;

	//Init carving
	CarvingButton = GTK_BUTTON(gtk_builder_get_object(builder,
				"CarvingButton"));

	//Init solver function
	SolveButton = GTK_BUTTON(gtk_builder_get_object(builder, "SolverButton"));
	GridEntry = GTK_ENTRY(gtk_builder_get_object(builder, "GridEntry"));
	Solver_info.Loader = NULL;
	Solver_info.Entry = GridEntry;
	Solver_info.Image = Image;
	HandWrite = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "Hand_write_grid"));
	Solver_info.Tog = HandWrite;

	//connect signals
	g_signal_connect(G_OBJECT(GTK_WIDGET(window)), "destroy",
			G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(ExitButton1), "clicked",G_CALLBACK(
				gtk_main_quit),NULL);
	g_signal_connect(G_OBJECT(ExitButton2), "clicked",G_CALLBACK(
				gtk_main_quit),NULL);
	g_signal_connect(G_OBJECT(GTK_WIDGET(Loader)),"file-set",
			G_CALLBACK(file_selected),&resize);
	resize.file = gtk_file_chooser_get_filename(Loader);
	g_signal_connect(G_OBJECT(GTK_WIDGET(All)),"clicked",
			G_CALLBACK(pretreatment),(gpointer*)&pre_info);
	g_signal_connect(G_OBJECT(GTK_WIDGET(CarvingButton)),"clicked",
			G_CALLBACK(carving),Image);
	g_signal_connect(G_OBJECT(GTK_WIDGET(SolveButton)),"clicked",
			G_CALLBACK(solver),(gpointer*)&Solver_info);
	g_signal_connect(G_OBJECT(GTK_WIDGET(ResizeButton)),"clicked",
			G_CALLBACK(risize),(gpointer*)&resize);
	//Show the window
	gtk_widget_show_all (GTK_WIDGET(window));
	gtk_main();
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
   if(VERBOSE) printf("Reading the image %s.\n", infilename);
   if(read_pgm_image(infilename, &image, &rows, &cols) == 0){
      fprintf(stderr, "Error reading the input image, %s.\n", infilename);
      exit(1);
   }

   /****************************************************************************
   * Perform the edge detection. All of the work takes place here.
   ****************************************************************************/
   if(VERBOSE) printf("Starting Canny edge detection.\n");
   if(dirfilename != NULL){
      sprintf(composedfname, "%s_s_%3.2f_l_%3.2f_h_%3.2f.fim", infilename,sigma, tlow, thigh);
      dirfilename = composedfname;
   }
   canny(image, rows, cols, sigma, tlow, thigh, &edge, dirfilename);

   /****************************************************************************
   * Write out the edge image to a file.
   ****************************************************************************/
   sprintf(outfilename, "ready.pgm");
   if(VERBOSE) printf("Writing the edge iname in the file %s.\n", outfilename);
   if(write_pgm_image(outfilename, edge, rows, cols, "", 255) == 0){
      fprintf(stderr, "Error writing the edge image, %s.\n", outfilename);
      exit(1);
   }
   free(image);
   free(edge);
}
