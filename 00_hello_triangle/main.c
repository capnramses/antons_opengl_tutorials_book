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
| "Hello Triangle". Just the basics.										   |
| If you're on Mac un-comment the version number code at the beginning. It	   |
| will give you the latest, even if you say 3.2!							   |
| This uses the libraries GLEW and SDL2 to start GL. Download and compile	  |
| these first. Linking them might be a pain, but you'll need to master this.   |
|																			   |
| I wrote this so that it compiles in pedantic ISO C90, to show that it's	   |
| easily done. I usually use minimalist C++ though, for tidier-looking maths   |
| functions.																   |
\******************************************************************************/
#include <GL/glew.h> /* include GLEW and new version of GL on Windows */
#include <SDL2/SDL.h> /* SDL helper library */
#include <stdio.h>

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
							  , SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
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

int main () {
	SDL_Window* window = NULL;
	SDL_Event event;
	const GLubyte* renderer;
	const GLubyte* version;
	GLuint vao;
	GLuint vbo;
	/* geometry to use. these are 3 xyz points (9 floats total) to make a triangle
	*/
	GLfloat points[] = {
		 0.0f,	0.5f,	0.0f,
		 0.5f, -0.5f,	0.0f,
		-0.5f, -0.5f,	0.0f
	};
	/* these are the strings of code for the shaders
	the vertex shader positions each vertex point */
	const char* vertex_shader =
	"#version 130\n"
	"in vec3 vp;"
	"void main () {"
	"	gl_Position = vec4 (vp, 1.0);"
	"}";
	/* the fragment shader colours each fragment (pixel-sized area of the
	triangle) */
	const char* fragment_shader =
	"#version 130\n"
	"out vec4 frag_colour;"
	"void main () {"
	"	frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
	"}";
	/* GL shader objects for vertex and fragment shader [components] */
	GLuint vs, fs;
	/* GL shader programme object [combined, to link] */
	GLuint shader_programme;

	window = init_scene();

	/* get version info */
	renderer = glGetString (GL_RENDERER); /* get renderer string */
	version = glGetString (GL_VERSION); /* version as a string */
	printf ("Renderer: %s\n", renderer);
	printf ("OpenGL version supported %s\n", version);

	/* a vertex buffer object (VBO) is created here. this stores an array of data
	on the graphics adapter's memory. in our case - the vertex points */
	glGenBuffers (1, &vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (GLfloat), points, GL_STATIC_DRAW);

	/* the vertex array object (VAO) is a little descriptor that defines which
	data from vertex buffer objects should be used as input variables to vertex
	shaders. in our case - use our only VBO, and say 'every three floats is a 
	variable' */
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	/* here we copy the shader strings into GL shaders, and compile them. we then
	create an executable shader 'program' and attach both of the compiled shaders.
	we link this, which matches the outputs of the vertex shader to the inputs of
	the fragment shader, etc. and it is then ready to use */
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
    
	/* this loop clears the drawing surface, then draws the geometry described by
	the VAO onto the drawing surface. we 'poll events' to see if the window was
	closed, etc. finally, we 'swap the buffers' which displays our drawing surface
	onto the view area. we use a double-buffering system which means that we have
	a 'currently displayed' surface, and 'currently being drawn' surface. hence
	the 'swap' idea. in a single-buffering system we would see stuff being drawn
	one-after-the-other */

	/* Call the function "tick" every 1/60th second */
	SDL_AddTimer(1000/60, tick, NULL);

	/* The main event loop */
	while (SDL_WaitEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			SDL_Quit();
			break;
		case SDL_USEREVENT:
			/* wipe the drawing surface clear */
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glUseProgram (shader_programme);
			glBindVertexArray (vao);
			/* draw points 0-3 from the currently bound VAO with current in-use shader*/
			glDrawArrays (GL_TRIANGLES, 0, 3);
			SDL_GL_SwapWindow(window);
			break;
		default:
			break;
		}
	}
	return 0;
}
