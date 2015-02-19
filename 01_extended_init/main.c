/******************************************************************************\
| OpenGL 4 Example Code.													   |
| Accompanies written series "Anton's OpenGL 4 Tutorials"					   |
| Email: anton at antongerdelan dot net										   |
| First version 27 Jan 2014													   |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.				   |
| See individual libraries for separate legal notices						   |
| SDL2 port: Dr Aidan Delaney <aidan@ontologyengineering.org>				   |
|			 (c) 2015														   |
|******************************************************************************|
| Extended Initialisation. Some extra detail.								   |
\******************************************************************************/
#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <SDL2/SDL.h> // SDL helper library
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <stdarg.h> // for doing gl_log() functions that work like printf()
//#include <stdbool.h> /* for visual studio i had to comment this out and define pure-C bool :( */
#define bool int
#define true 1
#define false 0
#define GL_LOG_FILE "gl.log"

/*
 * SDL timers run in separate threads.	In the timer thread
 * push an event onto the event queue.	This event signifies
 * to call draw a frame from the thread in which the OpenGL
 * context was created.
 */
Uint32 tick(Uint32 interval, void *param) {
	SDL_Event event;
	event.type = SDL_USEREVENT;
	event.user.code = 0;
	event.user.data1 = 0;
	event.user.data2 = 0;
	SDL_PushEvent(&event);
	return interval;
}

SDL_Window* init_scene() {
	Uint32 width = 640;
	Uint32 height = 480;
	SDL_Window * window;
	SDL_GLContext glContext;

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
	window = SDL_CreateWindow("Shader Example"
							  , SDL_WINDOWPOS_CENTERED
							  , SDL_WINDOWPOS_CENTERED
							  , width
							  , height
							  , SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!window) {
		fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
		exit(1);
	}

	glContext = SDL_GL_CreateContext(window);
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

	/* tell GL to only draw onto a pixel if the shape is closer to the viewer */
	glEnable (GL_DEPTH_TEST); /* enable depth-testing */
	glDepthFunc (GL_LESS);/*depth-testing interprets a smaller value as "closer"*/

	return window;
}

/* start a new log file. put the time and date at the top */
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
	fprintf (file, "GL_LOG_FILE log. local time %s", date);
	fprintf (file, "build version: %s %s\n\n", __DATE__, __TIME__);
	fclose (file);
	return true;
}

/* add a message to the log file. arguments work the same way as printf() */
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

/* we will tell SDL to run this function whenever it finds an error */
void sdl_error_callback (int error, const char* description) {
	gl_log_err ("SDL ERROR: code %i msg: %s\n", error, description);
}

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 640;
int g_gl_height = 480;

/* we will tell GLFW to run this function whenever the window is resized */
void sdl_window_size_callback (SDL_Window* window, int width, int height) {
	g_gl_width = width;
	g_gl_height = height;
    SDL_SetWindowSize(window, width, height);
	printf ("width %i height %i\n", width, height);
	/* update any perspective matrices used here */
}

/* we can use a function like this to print some GL capabilities of our adapter
to the log file. handy if we want to debug problems on other people's computers
*/
void log_gl_params () {
	int i;
	int v[2];
	unsigned char s = 0;
	GLenum params[] = {
		GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
		GL_MAX_CUBE_MAP_TEXTURE_SIZE,
		GL_MAX_DRAW_BUFFERS,
		GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
		GL_MAX_TEXTURE_IMAGE_UNITS,
		GL_MAX_TEXTURE_SIZE,
		GL_MAX_VARYING_FLOATS,
		GL_MAX_VERTEX_ATTRIBS,
		GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
		GL_MAX_VERTEX_UNIFORM_COMPONENTS,
		GL_MAX_VIEWPORT_DIMS,
		GL_STEREO,
	};
	const char* names[] = {
		"GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
		"GL_MAX_CUBE_MAP_TEXTURE_SIZE",
		"GL_MAX_DRAW_BUFFERS",
		"GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
		"GL_MAX_TEXTURE_IMAGE_UNITS",
		"GL_MAX_TEXTURE_SIZE",
		"GL_MAX_VARYING_FLOATS",
		"GL_MAX_VERTEX_ATTRIBS",
		"GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
		"GL_MAX_VERTEX_UNIFORM_COMPONENTS",
		"GL_MAX_VIEWPORT_DIMS",
		"GL_STEREO",
	};
	gl_log ("GL Context Params:\n");
	// integers - only works if the order is 0-10 integer return types
	for (i = 0; i < 10; i++) {
		int v = 0;
		glGetIntegerv (params[i], &v);
		gl_log ("%s %i\n", names[i], v);
	}
	// others
	v[0] = v[1] = 0;
	glGetIntegerv (params[10], v);
	gl_log ("%s %i %i\n", names[10], v[0], v[1]);
	glGetBooleanv (params[11], &s);
	gl_log ("%s %i\n", names[11], (unsigned int)s);
	gl_log ("-----------------------------\n");
}

double previous_seconds;
int frame_count;

/* we will use this function to update the window title with a frame rate */
void _update_fps_counter (SDL_Window* window) {
	double current_seconds;
	double elapsed_seconds;

	current_seconds = SDL_GetTicks () / 1000; //SDL_GetTicks in millis
	elapsed_seconds = current_seconds - previous_seconds;
	if (elapsed_seconds > 0.25) {
		previous_seconds = current_seconds;
		char tmp[128];
		double fps = (double)frame_count / elapsed_seconds;
		sprintf (tmp, "opengl @ fps: %.2f", fps);
		SDL_SetWindowTitle (window, tmp);
		frame_count = 0;
	}
	frame_count++;
}

int main () {
	SDL_Window* window;
	SDL_Event event;
	const GLubyte* renderer;
	const GLubyte* version;
	GLfloat points[] = {
		 0.0f,	0.5f,	0.0f,
		 0.5f, -0.5f,	0.0f,
		-0.5f, -0.5f,	0.0f
	};
	GLuint vbo;
	GLuint vao;
	const char* vertex_shader =
	"#version 130\n"
	"in vec3 vp;"
	"void main () {"
	"	gl_Position = vec4 (vp, 1.0);"
	"}";
	
	const char* fragment_shader =
	"#version 130\n"
	"out vec4 frag_colour;"
	"void main () {"
	"	frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
	"}";
	GLuint shader_programme, vs, fs;

	assert (restart_gl_log ());
	// start GL context and O/S window using the GLFW helper library
	SDL_version linked;
	SDL_GetVersion(&linked);
	gl_log ("starting SDL\n%d.%d.%d.\n", linked.major, linked.minor, linked.patch);

	window = init_scene();

	// get version info
	renderer = glGetString (GL_RENDERER); // get renderer string
	version = glGetString (GL_VERSION); // version as a string
	printf ("Renderer: %s\n", renderer);
	printf ("OpenGL version supported %s\n", version);
	gl_log ("renderer: %s\nversion: %s\n", renderer, version);
	log_gl_params ();
	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable (GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
	
	glGenBuffers (1, &vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (GLfloat), points, GL_STATIC_DRAW);
	
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	vs = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource (vs, 1, &vertex_shader, NULL);
	glCompileShader (vs);
	fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (fs, 1, &fragment_shader, NULL);
	glCompileShader (fs);
	shader_programme = glCreateProgram ();
	glAttachShader (shader_programme, fs);
	glAttachShader (shader_programme, vs);
	glLinkProgram (shader_programme);

		/* Call the function "tick" every 1/60th second */
	SDL_AddTimer(1000/60, tick, NULL);

	/* The main event loop */
	while (SDL_WaitEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			SDL_Quit();
			break;
		case SDL_USEREVENT:
			_update_fps_counter (window);
			// wipe the drawing surface clear
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport (0, 0, g_gl_width, g_gl_height);

			glUseProgram (shader_programme);
			glBindVertexArray (vao);
			// draw points 0-3 from the currently bound VAO with current in-use shader
			glDrawArrays (GL_TRIANGLES, 0, 3);

			// put the stuff we've been drawing onto the display
			SDL_GL_SwapWindow(window);
			break;
        case SDL_WINDOWEVENT:
          switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                  sdl_window_size_callback(window, event.window.data1, event.window.data2);
                break;
            default:
                break;
        }
		defult:
			break;
		}
	}
	return 0;
}
