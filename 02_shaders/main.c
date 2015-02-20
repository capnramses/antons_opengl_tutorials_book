/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| Shaders.                                                                     |
| I put some utility stuff, discussed in the previous demo into a file called  |
| gl_utils to keep this main file tidier                                       |
| I put SDL and GLEW start-up code into a "start_gl()" function. remember to  |
| uncomment the version number code if on an Apple machine                     |
\******************************************************************************/
#include "gl_utils.h"
#include <GL/glew.h> /* include GLEW and new version of GL on Windows */
#include <SDL2/SDL.h> /* SDL2 helper library */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
/* #include <stdbool.h> // for visual studio i had to comment this out and define pure-C bool :( */
#define bool int
#define true 1
#define false 0

/* keep track of window size for things like the viewport and the mouse cursor */
int g_gl_width = 640;
int g_gl_height = 480;
SDL_Window* g_window = NULL;

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

const char* GL_type_to_string (GLenum type) {
  switch (type) {
    case GL_BOOL: return "bool";
    case GL_INT: return "int";
    case GL_FLOAT: return "float";
    case GL_FLOAT_VEC2: return "vec2";
    case GL_FLOAT_VEC3: return "vec3";
    case GL_FLOAT_VEC4: return "vec4";
    case GL_FLOAT_MAT2: return "mat2";
    case GL_FLOAT_MAT3: return "mat3";
    case GL_FLOAT_MAT4: return "mat4";
    case GL_SAMPLER_2D: return "sampler2D";
    case GL_SAMPLER_3D: return "sampler3D";
    case GL_SAMPLER_CUBE: return "samplerCube";
    case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
    default: break;
  }
  return "other";
}

/* print errors in shader compilation */
void _print_shader_info_log (GLuint shader_index) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetShaderInfoLog (shader_index, max_length, &actual_length, log);
	printf ("shader info log for GL index %i:\n%s\n", shader_index, log);
}

/* print errors in shader linking */
void _print_programme_info_log (GLuint sp) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetProgramInfoLog (sp, max_length, &actual_length, log);
	printf ("program info log for GL index %i:\n%s", sp, log);
}

/* validate shader */
bool is_valid (GLuint sp) {
	int params = -1;

	glValidateProgram (sp);
	glGetProgramiv (sp, GL_VALIDATE_STATUS, &params);
	printf ("program %i GL_VALIDATE_STATUS = %i\n", sp, params);
	if (GL_TRUE != params) {
		_print_programme_info_log (sp);
		return false;
	}
	return true;
}

/* print absolutely everything about a shader - only useful if you get really
stuck wondering why a shader isn't working properly */
void print_all (GLuint sp) {
	int params = -1;
	int i;

	printf ("--------------------\nshader programme %i info:\n", sp);
	glGetProgramiv (sp, GL_LINK_STATUS, &params);
	printf ("GL_LINK_STATUS = %i\n", params);
	
	glGetProgramiv (sp, GL_ATTACHED_SHADERS, &params);
	printf ("GL_ATTACHED_SHADERS = %i\n", params);
	
	glGetProgramiv (sp, GL_ACTIVE_ATTRIBUTES, &params);
	printf ("GL_ACTIVE_ATTRIBUTES = %i\n", params);
	
	for (i = 0; i < params; i++) {
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveAttrib (sp, i, max_length, &actual_length, &size, &type, name);
		if (size > 1) {
			int j;
			for (j = 0; j < size; j++) {
				char long_name[64];
				int location;

				sprintf (long_name, "%s[%i]", name, j);
				location = glGetAttribLocation (sp, long_name);
				printf ("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string (type), long_name, location);
			}
		} else {
			int location = glGetAttribLocation (sp, name);
			printf ("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string (type), name, location);
		}
	}
	
	glGetProgramiv (sp, GL_ACTIVE_UNIFORMS, &params);
	printf ("GL_ACTIVE_UNIFORMS = %i\n", params);
	for (i = 0; i < params; i++) {
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveUniform (sp, i, max_length, &actual_length, &size, &type, name);
		if (size > 1) {
			int j;
			for (j = 0; j < size; j++) {
				char long_name[64];
				int location;

				sprintf (long_name, "%s[%i]", name, j);
				location = glGetUniformLocation (sp, long_name);
				printf ("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string (type), long_name, location);
			}
		} else {
			int location = glGetUniformLocation (sp, name);
			printf ("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string (type), name, location);
		}
	}
	
	_print_programme_info_log (sp);
}

/* copy a shader from a plain text file into a character array */
bool parse_file_into_str (
	const char* file_name, char* shader_str, int max_len
) {
	FILE* file = fopen (file_name , "r");
	int current_len = 0;
	char line[2048];

	shader_str[0] = '\0'; /* reset string */
	if (!file) {
		gl_log_err ("ERROR: opening file for reading: %s\n", file_name);
		return false;
	}
	strcpy (line, ""); /* remember to clean up before using for first time! */
	while (!feof (file)) {
		if (NULL != fgets (line, 2048, file)) {
			current_len += strlen (line); /* +1 for \n at end */
			if (current_len >= max_len) {
				gl_log_err (
					"ERROR: shader length is longer than string buffer length %i\n",
					max_len
				);
			}
			strcat (shader_str, line);
		}
	}
	if (EOF == fclose (file)) { /* probably unnecesssary validation */
		gl_log_err ("ERROR: closing file from reading %s\n", file_name);
		return false;
	}
	return true;
}


int main () {
	SDL_Event event;
	GLfloat points[] = {
		 0.0f,	0.5f,	0.0f,
		 0.5f, -0.5f,	0.0f,
		-0.5f, -0.5f,	0.0f
	};
	GLuint vbo;
	GLuint vao;
	char vertex_shader[1024 * 256];
	char fragment_shader[1024 * 256];
	GLuint vs, fs, shader_programme;
	const GLchar* p;
	int params = -1;
	GLint colour_loc;

	assert (restart_gl_log ());
	assert (start_gl ());
	
	/* tell GL to only draw onto a pixel if the shape is closer to the viewer */
	glEnable (GL_DEPTH_TEST); /* enable depth-testing */
	glDepthFunc (GL_LESS); /* depth-testing interprets a smaller value as "closer" */
	
	
	glGenBuffers (1, &vbo);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glBufferData (GL_ARRAY_BUFFER, 9 * sizeof (GLfloat), points, GL_STATIC_DRAW);
	
	glGenVertexArrays (1, &vao);
	glBindVertexArray (vao);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	
	/* load shaders from files here */
	
	assert (parse_file_into_str ("test_vs.glsl", vertex_shader, 1024 * 256));
	assert (parse_file_into_str ("test_fs.glsl", fragment_shader, 1024 * 256));
	
	vs = glCreateShader (GL_VERTEX_SHADER);
	p = (const GLchar*)vertex_shader;
	glShaderSource (vs, 1, &p, NULL);
	glCompileShader (vs);
	
	/* check for shader compile errors - very important! */
	
	glGetShaderiv (vs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr, "ERROR: GL shader index %i did not compile\n", vs);
		_print_shader_info_log (vs);
		return 1; /* or exit or something */
	}
	
	fs = glCreateShader (GL_FRAGMENT_SHADER);
	p = (const GLchar*)fragment_shader;
	glShaderSource (fs, 1, &p, NULL);
	glCompileShader (fs);
	
	/* check for compile errors */
	glGetShaderiv (fs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr, "ERROR: GL shader index %i did not compile\n", fs);
		_print_shader_info_log (fs);
		return 1; /* or exit or something */
	}
	
	shader_programme = glCreateProgram ();
	glAttachShader (shader_programme, fs);
	glAttachShader (shader_programme, vs);
	glLinkProgram (shader_programme);
	
	/* check for shader linking errors - very important! */
	glGetProgramiv (shader_programme, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (
			stderr,
			"ERROR: could not link shader programme GL index %i\n",
			shader_programme
		);
		_print_programme_info_log (shader_programme);
		return 1;
	}
	print_all (shader_programme);
	assert (is_valid (shader_programme));
	
	colour_loc = glGetUniformLocation (shader_programme, "inputColour");
	assert (colour_loc > -1);
	glUseProgram (shader_programme);
	glUniform4f (colour_loc, 1.0f, 0.0f, 0.0f, 1.0f);

	/* Call the function "tick" every 1/60th second */
	SDL_AddTimer(1000/60, tick, NULL);

	/* The main event loop */
	while (SDL_WaitEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			SDL_Quit();
			break;
		case SDL_USEREVENT:
			_update_fps_counter (g_window);
			/* wipe the drawing surface clear */
			glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glViewport (0, 0, g_gl_width, g_gl_height);

			glUseProgram (shader_programme);
			glBindVertexArray (vao);
			/* draw points 0-3 from the currently bound VAO with current in-use shader */
			glDrawArrays (GL_TRIANGLES, 0, 3);

			/* put the stuff we've been drawing onto the display */
			SDL_GL_SwapWindow(g_window);
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					sdl_window_size_callback(g_window, event.window.data1, event.window.data2);
					break;
				default:
					break;
			}
		default:
			break;
		}
	}

	return 0;
}
