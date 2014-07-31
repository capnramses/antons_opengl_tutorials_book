/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 5 Feb 2014                                                     |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries for respective legal notices                        |
|******************************************************************************|
| Bitmap Fonts example                                                         |
| Modified previous font viewer to read the new generated font, loading meta   |
| data from a file                                                             |
\******************************************************************************/
#include "maths_funcs.h"
#include "stb_image.h" // Sean Barrett's image loader
#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define ATLAS_IMAGE "freemono.png"
#define ATLAS_META "freemono.meta"
// size of atlas. my handmade image is 16x16 glyphs
#define ATLAS_COLS 16
#define ATLAS_ROWS 16

int g_viewport_width = 800;
int g_viewport_height = 480;

GLuint sp; // shader programme
GLuint sp_text_colour_loc; // location of vec4 "text_colour" uniform

float glyph_y_offsets[256] = { 0.0f };
float glyph_widths[256] = { 0.0f };

/* load meta data file for font. we really only need the ascii value, width,
height, and y_offset, but if you want to be super precise you have the other
adjustment values there too */
bool load_meta_data (const char* meta_file) {
	FILE* fp = fopen (meta_file, "r");
	if (!fp) {
		fprintf (stderr, "ERROR: could not open file %s\n", meta_file);
		return false;
	}
	char line [128];
	int ascii_code = -1;
	float prop_xMin = 0.0f;
	float prop_width = 0.0f;
	float prop_yMin = 0.0f;
	float prop_height = 0.0f;
	float prop_y_offset = 0.0f;
	// get header line first
	fgets (line, 128, fp);
	// loop through and get each glyph's info
	while (EOF != fscanf (
		fp, "%i %f %f %f %f %f\n",
		&ascii_code,
		&prop_xMin,
		&prop_width,
		&prop_yMin,
		&prop_height,
		&prop_y_offset
	)) {
		glyph_widths[ascii_code] = prop_width;
		glyph_y_offsets[ascii_code] = 1.0 - prop_height - prop_y_offset;
	}
	fclose (fp);
	return true;
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
	"#version 400\n"
	"layout (location = 0) in vec2 vp;"
	"layout (location = 1) in vec2 vt;"
	"out vec2 st;"
	"void main () {"
	"  st = vt;"
	"  gl_Position = vec4 (vp, 0.0, 1.0);"
	"}";
	const char* fs_str =
	"#version 400\n"
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
	// uncomment these lines if on Apple OS X
	/*glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/
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
	assert (load_meta_data (ATLAS_META));
	
	/* set a string of text for lower-case letters */
	GLuint first_string_vp_vbo, first_string_vt_vbo, first_string_vao;
	glGenBuffers (1, &first_string_vp_vbo);
	glGenBuffers (1, &first_string_vt_vbo);
	float x_pos = -0.75f;
	float y_pos = 0.2f;
	float pixel_scale = 190.0f;
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
	pixel_scale = 70.0f;
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
	assert (load_texture (ATLAS_IMAGE, &tex));
	
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
