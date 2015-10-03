/*****************************************************************************\
| OpenGL 4 Example Code.                                                      |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                     |
| Email: anton at antongerdelan dot net                                       |
| First version 27 Jan 2014                                                   |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.               |
| See individual libraries separate legal notices                             |
|*****************************************************************************|
| This is just a file holding some commonly-used "utility" functions to keep  |
| the main file a bit easier to read. You can might build up something like   |
| this as learn more GL. Note that you don't need much code here to do good GL|
| If you have a big object-oriented engine then maybe you can ask yourself if |
| it is really making life easier.                                            |
\*****************************************************************************/
#include "gl_utils.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#define GL_LOG_FILE "gl.log"
#define MAX_SHADER_LENGTH 262144

/*--------------------------------LOG FUNCTIONS------------------------------*/
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

/*--------------------------------GLFW3 and GLEW-----------------------------*/
bool start_gl () {
	const GLubyte* renderer;
	const GLubyte* version;

	gl_log ("starting GLFW %s", glfwGetVersionString ());

	glfwSetErrorCallback (glfw_error_callback);
	if (!glfwInit ()) {
		fprintf (stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	/* We must specify 3.2 core if on Apple OS X -- other O/S can specify
	 anything here. I defined 'APPLE' in the makefile for OS X */
#ifdef APPLE
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	/*GLFWmonitor* mon = glfwGetPrimaryMonitor ();
	const GLFWvidmode* vmode = glfwGetVideoMode (mon);
	g_window = glfwCreateWindow (
		vmode->width, vmode->height, "Extended GL Init", mon, NULL
	);*/

	g_window = glfwCreateWindow (
		g_gl_width, g_gl_height, "Extended Init.", NULL, NULL
	);
	if (!g_window) {
		fprintf (stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}
	glfwSetWindowSizeCallback (g_window, glfw_window_size_callback);
	glfwMakeContextCurrent (g_window);

	glfwWindowHint (GLFW_SAMPLES, 4);

	/* start GLEW extension handler */
	glewExperimental = GL_TRUE;
	glewInit ();

	/* get version info */
	renderer = glGetString (GL_RENDERER); /* get renderer string */
	version = glGetString (GL_VERSION); /* version as a string */
	printf ("Renderer: %s\n", renderer);
	printf ("OpenGL version supported %s\n", version);
	gl_log ("renderer: %s\nversion: %s\n", renderer, version);

	return true;
}

void glfw_error_callback (int error, const char* description) {
	fputs (description, stderr);
	gl_log_err ("%s\n", description);
}

/* a call-back function */
void glfw_window_size_callback (GLFWwindow* window, int width, int height) {
	g_gl_width = width;
	g_gl_height = height;
	printf ("width %i height %i\n", width, height);
	/* update any perspective matrices used here */
}

double previous_seconds;
int frame_count;

void _update_fps_counter (GLFWwindow* window) {
	double current_seconds;
	double elapsed_seconds;

	previous_seconds = glfwGetTime ();
	current_seconds = glfwGetTime ();
	elapsed_seconds = current_seconds - previous_seconds;
	if (elapsed_seconds > 0.25) {
		double fps;
		char tmp[128];

		previous_seconds = current_seconds;
		fps = (double)frame_count / elapsed_seconds;
		sprintf (tmp, "opengl @ fps: %.2f", fps);
		glfwSetWindowTitle (window, tmp);
		frame_count = 0;
	}
	frame_count++;
}
