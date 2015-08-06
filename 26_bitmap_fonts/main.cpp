/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 5 Feb 2014                                                     |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries for respective legal notices                        |
|******************************************************************************|
| Bitmap Fonts example                                                         |
| Here I made a font atlas by hand in GIMP, and I've created a little function |
| called 'tweak_glyphs' that allows me to customise the spacing and height     |
| offset for individual glyphs. Ideally I would save these values to a file    |
| when I'd finished, so that I can re-use them later.                          |
| I render two strings of text here. I create an unique set of buffers and     |
| VAO for each string of text, and I work out the position and texture coords  |
| for each glyph within each string with a function called 'text_to_vbo'       |
| which maps each character to texture coordinates within the altas texture,   |
| and pulls out the corresponding glyph spacing offsets that we tweaked.       |
| I also have a uniform vec4 which lets me customise the colour of the text,   |
| which I can do easily because I coloured it in white in the image file.      |
\******************************************************************************/
#include "maths_funcs.h"
#include "stb_image.h" // Sean Barrett's image loader
#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// size of atlas. my handmade image is 16x16 glyphs
#define ATLAS_COLS 16
#define ATLAS_ROWS 16

int g_viewport_width = 800;
int g_viewport_height = 480;

GLuint sp; // shader programme
GLuint sp_text_colour_loc; // location of vec4 "text_colour" uniform

float glyph_y_offsets[256] = { 0.0f };
float glyph_widths[256] = { 0.0f };

/* After tweaking, save these values to a file */
void tweak_glyphs () {
	/* lower down glyhs by some factor */
	for (int i = 0; i < 255; i++) {
		/* default lower-case to half-size spacing */
		if (i >= 'a' && i <= 'z') {
			glyph_y_offsets[i] = 0.6f;
		} else if (i >= 'A' && i <= 'Z') {
			glyph_y_offsets[i] = 0.2f;
		}
	}
	glyph_y_offsets['b'] = 0.4f;
	glyph_y_offsets['d'] = 0.4f;
	glyph_y_offsets['f'] = 0.4f;
	glyph_y_offsets['g'] = 0.5f;
	glyph_y_offsets['h'] = 0.3f;
	glyph_y_offsets['i'] = 0.5f;
	glyph_y_offsets['j'] = 0.4f;
	glyph_y_offsets['k'] = 0.3f;
	glyph_y_offsets['l'] = 0.3f;
	glyph_y_offsets['p'] = 0.5f;
	glyph_y_offsets['q'] = 0.5f;
	glyph_y_offsets['s'] = 0.4f;
	glyph_y_offsets['t'] = 0.3f;
	glyph_y_offsets['!'] = 0.1f;
	glyph_y_offsets[','] = 1.0f;
	/* reduce spacing after glyph */
	for (int i = 0; i < 255; i++) {
		/* default lower-case to half-size spacing */
		if (i >= 'a' && i <= 'z') {
			glyph_widths[i] = 0.5f;
		} else if (i >= 'A' && i <= 'Z') {
			glyph_widths[i] = 0.7f;
		} else {
			glyph_widths[i] = 1.0f;
		}
	}
	glyph_widths[' '] = 0.4f;
	glyph_widths['!'] = 0.3f;
	glyph_widths[','] = 0.5f;
	glyph_widths['A'] = 0.8f;
	glyph_widths['B'] = 0.6f;
	glyph_widths['D'] = 0.6f;
	glyph_widths['F'] = 0.6f;
	glyph_widths['L'] = 0.8f;
	glyph_widths['M'] = 1.0f;
	glyph_widths['N'] = 0.8f;
	glyph_widths['P'] = 0.6f;
	glyph_widths['R'] = 0.6f;
	glyph_widths['S'] = 0.85f;
	glyph_widths['T'] = 0.9f;
	glyph_widths['U'] = 0.8f;
	glyph_widths['V'] = 0.8f;
	glyph_widths['W'] = 1.0f;
	glyph_widths['e'] = 0.45f;
	glyph_widths['f'] = 0.4f;
	glyph_widths['g'] = 0.45f;
	glyph_widths['h'] = 0.45f;
	glyph_widths['i'] = 0.25f;
	glyph_widths['j'] = 0.3f;
	glyph_widths['k'] = 0.45f;
	glyph_widths['l'] = 0.25f;
	glyph_widths['p'] = 0.45f;
	glyph_widths['q'] = 0.45f;
	glyph_widths['r'] = 0.45f;
	glyph_widths['t'] = 0.4f;
	glyph_widths['u'] = 0.45f;
	glyph_widths['v'] = 0.4f;
	glyph_widths['w'] = 0.7f;
	glyph_widths['y'] = 0.4f;
}

void text_to_vbo (
	const char* str,
	float at_x,
	float at_y,
	float scale_px,
	GLuint* points_vbo,
	GLuint* texcoords_vbo,
	int* point_count
) {
	int len = strlen (str);
	
	float* points_tmp = (float*)malloc (sizeof (float) * len * 12);
	float* texcoords_tmp = (float*)malloc (sizeof (float) * len * 12);
	for (int i = 0; i < len; i++) {
		// get ascii code as integer
		int ascii_code = str[i];
		
		// work out row and column in atlas
		int atlas_col = (ascii_code - ' ') % ATLAS_COLS;
		int atlas_row = (ascii_code - ' ') / ATLAS_COLS;
		
		// work out texture coordinates in atlas
		float s = atlas_col * (1.0 / ATLAS_COLS);
		float t = (atlas_row + 1) * (1.0 / ATLAS_ROWS);
		
		// work out position of glyphtriangle_width
		float x_pos = at_x;
		float y_pos = at_y - scale_px / g_viewport_height *
			glyph_y_offsets[ascii_code];
		
		// move next glyph along to the end of this one
		if (i + 1 < len) {
			// upper-case letters move twice as far
			at_x += glyph_widths[ascii_code] * scale_px / g_viewport_width;
		}
		// add 6 points and texture coordinates to buffers for each glyph
		points_tmp[i * 12] = x_pos;
		points_tmp[i * 12 + 1] = y_pos;
		points_tmp[i * 12 + 2] = x_pos;
		points_tmp[i * 12 + 3] = y_pos - scale_px / g_viewport_height;
		points_tmp[i * 12 + 4] = x_pos + scale_px / g_viewport_width;
		points_tmp[i * 12 + 5] = y_pos - scale_px / g_viewport_height;
		
		points_tmp[i * 12 + 6] = x_pos + scale_px / g_viewport_width;
		points_tmp[i * 12 + 7] = y_pos - scale_px / g_viewport_height;
		points_tmp[i * 12 + 8] = x_pos + scale_px / g_viewport_width;
		points_tmp[i * 12 + 9] = y_pos;
		points_tmp[i * 12 + 10] = x_pos;
		points_tmp[i * 12 + 11] = y_pos;
		
		texcoords_tmp[i * 12] = s;
		texcoords_tmp[i * 12 + 1] = 1.0 - t + 1.0 / ATLAS_ROWS;
		texcoords_tmp[i * 12 + 2] = s;
		texcoords_tmp[i * 12 + 3] = 1.0 - t;
		texcoords_tmp[i * 12 + 4] = s + 1.0 / ATLAS_COLS;
		texcoords_tmp[i * 12 + 5] = 1.0 - t;
		
		texcoords_tmp[i * 12 + 6] = s + 1.0 / ATLAS_COLS;
		texcoords_tmp[i * 12 + 7] = 1.0 - t;
		texcoords_tmp[i * 12 + 8] = s + 1.0 / ATLAS_COLS;
		texcoords_tmp[i * 12 + 9] = 1.0 - t + 1.0 / ATLAS_ROWS;
		texcoords_tmp[i * 12 + 10] = s;
		texcoords_tmp[i * 12 + 11] = 1.0 - t + 1.0 / ATLAS_ROWS;
	}
	
	glBindBuffer (GL_ARRAY_BUFFER, *points_vbo);
	glBufferData (
		GL_ARRAY_BUFFER,
		len * 12 * sizeof (float),
		points_tmp,
		GL_DYNAMIC_DRAW
	);
	glBindBuffer (GL_ARRAY_BUFFER, *texcoords_vbo);
	glBufferData (
		GL_ARRAY_BUFFER,
		len * 12 * sizeof (float),
		texcoords_tmp,
		GL_DYNAMIC_DRAW
	);
	
	free (points_tmp);
	free (texcoords_tmp);
	
	*point_count = len * 6;
}

void create_shaders () {
	/* here i used negative y from the buffer as the z value so that it was on
	the floor but also that the 'front' was on the top side. also note how i
	work out the texture coordinates, st, from the vertex point position */
	const char* vs_str =
	"#version 410\n"
	"layout (location = 0) in vec2 vp;"
	"layout (location = 1) in vec2 vt;"
	"out vec2 st;"
	"void main () {"
	"  st = vt;"
	"  gl_Position = vec4 (vp, 0.0, 1.0);"
	"}";
	const char* fs_str =
	"#version 410\n"
	"in vec2 st;"
	"uniform sampler2D tex;"
	"uniform vec4 text_colour;"
	"out vec4 frag_colour;"
	"void main () {"
	"  frag_colour = texture (tex, st) * text_colour;"
	"}";
	GLuint vs = glCreateShader (GL_VERTEX_SHADER);
	glShaderSource (vs, 1, &vs_str, NULL);
	glCompileShader (vs);
	int params = -1;
	glGetShaderiv (vs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr, "ERROR: GL shader index %i did not compile\n", vs);
	}
	GLuint fs = glCreateShader (GL_FRAGMENT_SHADER);
	glShaderSource (fs, 1, &fs_str, NULL);
	glCompileShader (fs);
	glGetShaderiv (fs, GL_COMPILE_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (stderr, "ERROR: GL shader index %i did not compile\n", fs);
	}
	sp = glCreateProgram ();
	glAttachShader (sp, vs);
	glAttachShader (sp, fs);
	glLinkProgram (sp);
	glGetProgramiv (sp, GL_LINK_STATUS, &params);
	if (GL_TRUE != params) {
		fprintf (
			stderr,
			"ERROR: could not link shader programme GL index %i\n",
			sp
		);
	}
	sp_text_colour_loc = glGetUniformLocation (sp, "text_colour");
}

bool load_texture (const char* file_name, GLuint* tex) {
	int x, y, n;
	int force_channels = 4;
	unsigned char* image_data = stbi_load (file_name, &x, &y, &n, force_channels);
	if (!image_data) {
		fprintf (stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}
	// NPOT check
	if ((x & (x - 1)) != 0 || (y & (y - 1)) != 0) {
		fprintf (
			stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name
		);
	}
	int width_in_bytes = x * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = y / 2;

	for (int row = 0; row < half_height; row++) {
		top = image_data + row * width_in_bytes;
		bottom = image_data + (y - row - 1) * width_in_bytes;
		for (int col = 0; col < width_in_bytes; col++) {
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}
	glGenTextures (1, tex);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, *tex);
	glTexImage2D (
		GL_TEXTURE_2D,
		0,
		GL_RGBA,
		x,
		y,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		image_data
	);
	glGenerateMipmap (GL_TEXTURE_2D);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	GLfloat max_aniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
	// set the maximum!
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
	return true;
}

/* we will tell GLFW to run this function whenever the window is resized */
void glfw_window_size_callback (GLFWwindow* window, int width, int height) {
	g_viewport_width = width;
	g_viewport_height = height;
	/* update any perspective matrices used here */
}

int main () {
	// start GL context with helper libraries
	assert (glfwInit ());
	
	/* We must specify 3.2 core if on Apple OS X -- other O/S can specify
	 anything here. I defined 'APPLE' in the makefile for OS X */
#ifdef APPLE
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	GLFWwindow* window = glfwCreateWindow (
		g_viewport_width, g_viewport_height, "Bitmap Fonts", NULL, NULL);
	glfwSetWindowSizeCallback (window, glfw_window_size_callback);
	glfwMakeContextCurrent (window);
	glewExperimental = GL_TRUE;
	glewInit ();
	const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString (GL_VERSION); // version as a string
	printf ("Renderer: %s\n", renderer);
	printf ("OpenGL version supported %s\n", version);
	
	/* load font meta-data (spacings for each glyph) */
	tweak_glyphs ();
	
	/* set a string of text for lower-case letters */
	GLuint first_string_vp_vbo, first_string_vt_vbo, first_string_vao;
	glGenBuffers (1, &first_string_vp_vbo);
	glGenBuffers (1, &first_string_vt_vbo);
	float x_pos = -0.75f;
	float y_pos = 0.2f;
	float pixel_scale = 64.0f;
	const char first_str[] = "abcdefghijklmnopqrstuvwxyz";
	int first_string_points = 0;
	text_to_vbo (
		first_str,
		x_pos,
		y_pos,
		pixel_scale,
		&first_string_vp_vbo,
		&first_string_vt_vbo,
		&first_string_points
	);
	glGenVertexArrays (1, &first_string_vao);
	glBindVertexArray (first_string_vao);
	glBindBuffer (GL_ARRAY_BUFFER, first_string_vp_vbo);
	glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, first_string_vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	
	/* second string of text for capital letters */
	GLuint second_string_vp_vbo, second_string_vt_vbo, second_string_vao;
	glGenBuffers (1, &second_string_vp_vbo);
	glGenBuffers (1, &second_string_vt_vbo);
	x_pos = -1.0f;
	y_pos = 1.0f;
	pixel_scale = 64.0f;
	const char second_str[] = "The human torch was denied a bank loan!";
	int second_string_points = 0;
	text_to_vbo (
		second_str,
		x_pos,
		y_pos,
		pixel_scale,
		&second_string_vp_vbo,
		&second_string_vt_vbo,
		&second_string_points
	);
	glGenVertexArrays (1, &second_string_vao);
	glBindVertexArray (second_string_vao);
	glBindBuffer (GL_ARRAY_BUFFER, second_string_vp_vbo);
	glVertexAttribPointer (0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (0);
	glBindBuffer (GL_ARRAY_BUFFER, second_string_vt_vbo);
	glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	
	create_shaders ();
	
	// textures
	GLuint tex;
	assert (load_texture ("handmade2.png", &tex));
	
	// rendering defaults
	//glDepthFunc (GL_LESS); // set depth function
	//glEnable (GL_DEPTH_TEST);
	glCullFace (GL_BACK); // cull back face
	glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
	glEnable (GL_CULL_FACE); // cull face
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // partial transparency
	glClearColor (0.2, 0.2, 0.6, 1.0);
	glViewport (0, 0, g_viewport_width, g_viewport_height);
	
	// start main rendering loop
	while (!glfwWindowShouldClose (window)) {
		// wipe the drawing surface clear
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		// draw
		glActiveTexture (GL_TEXTURE0);
		glBindTexture (GL_TEXTURE_2D, tex);
		glUseProgram (sp);
		
		/* Draw text with no depth test and alpha blending */
		glDisable (GL_DEPTH_TEST);
		glEnable (GL_BLEND);
		
		glBindVertexArray (first_string_vao);
		glUniform4f (sp_text_colour_loc, 1.0, 0.0, 1.0, 1.0);
		glDrawArrays (GL_TRIANGLES, 0, first_string_points);
		
		glBindVertexArray (second_string_vao);
		glUniform4f (sp_text_colour_loc, 1.0, 1.0, 0.0, 1.0);
		glDrawArrays (GL_TRIANGLES, 0, second_string_points);
		
		// update other events like input handling 
		glfwPollEvents ();
		if (GLFW_PRESS == glfwGetKey (window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose (window, 1);
		}
		glfwSwapBuffers (window);
	}
	// done
	 glfwTerminate();
	return 0;
}
