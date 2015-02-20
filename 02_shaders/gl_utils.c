/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries separate legal notices                              |
|******************************************************************************|
| This is just a file holding some commonly-used "utility" functions to keep   |
| the main file a bit easier to read. You can might build up something like    |
| this as learn more GL. Note that you don't need much code here to do good GL.|
| If you have a big object-oriented engine then maybe you can ask yourself if  |
| it is really making life easier.                                             |
\******************************************************************************/
#include "gl_utils.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#define GL_LOG_FILE "gl.log"
#define MAX_SHADER_LENGTH 262144

/*--------------------------------LOG FUNCTIONS-------------------------------*/
bool restart_gl_log () {
	time_t now;
	char* date;
	FILE* file = fopen (GL_LOG_FILE, "w");

	if (!file) {
		fprintf (
			stderr,
			"ERROR: could not open GL_LOG_FILE log file %s for writing\n",
			GL_LOG_FILE
		);
		return false;
	}
	now = time (NULL);
	date = ctime (&now);
	fprintf (file, "GL_LOG_FILE log. local time %s\n", date);
	fclose (file);
	return true;
}

bool gl_log (const char* message, ...) {
	va_list argptr;
	FILE* file = fopen (GL_LOG_FILE, "a");
	if (!file) {
		fprintf (
			stderr,
			"ERROR: could not open GL_LOG_FILE %s file for appending\n",
			GL_LOG_FILE
		);
		return false;
	}
	va_start (argptr, message);
	vfprintf (file, message, argptr);
	va_end (argptr);
	fclose (file);
	return true;
}

/* same as gl_log except also prints to stderr */
bool gl_log_err (const char* message, ...) {
	va_list argptr;
	FILE* file = fopen (GL_LOG_FILE, "a");
	if (!file) {
		fprintf (
			stderr,
			"ERROR: could not open GL_LOG_FILE %s file for appending\n",
			GL_LOG_FILE
		);
		return false;
	}
	va_start (argptr, message);
	vfprintf (file, message, argptr);
	va_end (argptr);
	va_start (argptr, message);
	vfprintf (stderr, message, argptr);
	va_end (argptr);
	fclose (file);
	return true;
}

/*--------------------------------GLFW3 and GLEW------------------------------*/
bool start_gl () {
	SDL_GLContext glContext;
    const GLubyte* renderer;
    const GLubyte* version;

	/* Glew will later ensure that OpenGL 3 *is* supported on this machine */
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	/* Do double buffering in GL */
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  /* Initialise SDL - when using C/C++ it's common to have to
	 initialise libraries by calling a function within them. */
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER)<0) {
		  fprintf(stderr, "Failed to initialise SDL: %s\n", SDL_GetError());
		  exit(1);
	}

	/* When we close a window quit the SDL application */
	atexit(SDL_Quit);

	/* Create a new window with an OpenGL surface */
	g_window = SDL_CreateWindow("02 Shader Example"
							  , SDL_WINDOWPOS_CENTERED
							  , SDL_WINDOWPOS_CENTERED
							  , g_gl_width
							  , g_gl_height
							  , SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!g_window) {
		fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
		exit(1);
	}

	glContext = SDL_GL_CreateContext(g_window);
	if (!glContext) {
		fprintf(stderr, "Failed to create OpenGL context: %s\n", SDL_GetError());
		exit(1);
	}

	/* Initialise GLEW - an easy way to ensure OpenGl 3.0+
	   The *must* be done after we have set the video mode - otherwise we have no
	   OpenGL context to test. */
	glewExperimental = GL_TRUE;
	glewInit();
	if (!glewIsSupported("GL_VERSION_3_0")) {
		fprintf(stderr, "OpenGL 3.0 not available");
		exit(1);
	}

    /* get version info */
    renderer = glGetString (GL_RENDERER); /* get renderer string */
    version = glGetString (GL_VERSION); /* version as a string */
    printf ("Renderer: %s\n", renderer);
    printf ("OpenGL version supported %s\n", version);
    gl_log ("renderer: %s\nversion: %s\n", renderer, version);

	/* tell GL to only draw onto a pixel if the shape is closer to the viewer */
	glEnable (GL_DEPTH_TEST); /* enable depth-testing */
	glDepthFunc (GL_LESS);/*depth-testing interprets a smaller value as "closer"*/

	return true;
}

void sdl_error_callback (int error, const char* description) {
	fputs (description, stderr);
	gl_log_err ("%s\n", description);
}

/* a call-back function */
void sdl_window_size_callback (SDL_Window* window, int width, int height) {
	g_gl_width = width;
	g_gl_height = height;
	SDL_SetWindowSize(window, width, height);
	printf ("width %i height %i\n", width, height);
	/* update any perspective matrices used here */
}

double previous_seconds;
int frame_count;

void _update_fps_counter (SDL_Window* window) {
	double current_seconds;
	double elapsed_seconds;

	current_seconds = SDL_GetTicks () / 1000; /* SDL_GetTicks in millis */
	elapsed_seconds = current_seconds - previous_seconds;
	if (elapsed_seconds > 0.25) {
		char tmp[128];
		double fps = (double)frame_count / elapsed_seconds;
		previous_seconds = current_seconds;
		sprintf (tmp, "opengl @ fps: %.2f", fps);
		SDL_SetWindowTitle (window, tmp);
		frame_count = 0;
	}
	frame_count++;
}
