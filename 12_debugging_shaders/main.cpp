/*****************************************************************************\
| OpenGL 4 Example Code.                                                      |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                     |
| Email: anton at antongerdelan dot net                                       |
| First version 27 Jan 2014                                                   |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.               |
| See individual libraries' for separate legal notices                        |
|*****************************************************************************|
| Debugging Shaders                                                           |
| this just shows how to visualise different components of shader variables as|
| colours. in this case, one component of the specular lighting equation.     |
\*****************************************************************************/
#include "maths_funcs.h"
#define STB_IMAGE_IMPLEMENTATION
#include "gl_utils.h"
#include "obj_parser.h"
#include "stb_image.h"
#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define GL_LOG_FILE "gl.log"
#define VERTEX_SHADER_FILE "test_vs.glsl"
#define FRAGMENT_SHADER_FILE "test_fs.glsl"
#define MESH_FILE "suzanne.obj"

GLfloat *g_vp = NULL; // array of vertex points
GLfloat *g_vn = NULL; // array of vertex normals
GLfloat *g_vt = NULL; // array of texture coordinates
int g_point_count = 0;

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 640;
int g_gl_height = 480;
GLFWwindow *g_window = NULL;

bool load_texture( const char *file_name, GLuint *tex ) {
	int x, y, n;
	int force_channels = 4;
	unsigned char *image_data = stbi_load( file_name, &x, &y, &n, force_channels );
	if ( !image_data ) {
		fprintf( stderr, "ERROR: could not load %s\n", file_name );
		return false;
	}
	// NPOT check
	if ( ( x & ( x - 1 ) ) != 0 || ( y & ( y - 1 ) ) != 0 ) {
		fprintf( stderr, "WARNING: texture %s is not power-of-2 dimensions\n",
						 file_name );
	}
	int width_in_bytes = x * 4;
	unsigned char *top = NULL;
	unsigned char *bottom = NULL;
	unsigned char temp = 0;
	int half_height = y / 2;

	for ( int row = 0; row < half_height; row++ ) {
		top = image_data + row * width_in_bytes;
		bottom = image_data + ( y - row - 1 ) * width_in_bytes;
		for ( int col = 0; col < width_in_bytes; col++ ) {
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}
	glGenTextures( 1, tex );
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, *tex );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE,
								image_data );
	glGenerateMipmap( GL_TEXTURE_2D );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	GLfloat max_aniso = 0.0f;
	glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso );
	// set the maximum!
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso );
	return true;
}

int main() {
	/*--------------------------------START OPENGL---------------------------*/
	restart_gl_log();
	// start GL context and O/S window using the GLFW helper library
	start_gl();
	glEnable( GL_DEPTH_TEST ); // enable depth-testing
	// depth-testing interprets a smaller value as "closer"
	glDepthFunc( GL_LESS );
	glEnable( GL_CULL_FACE ); // cull face
	glCullFace( GL_BACK );		// cull back face
	// set counter-clock-wise vertex order to mean the front
	glFrontFace( GL_CCW );
	glClearColor( 0.2, 0.2, 0.2, 1.0 ); // grey background to help spot mistakes
	glViewport( 0, 0, g_gl_width, g_gl_height );

	/*------------------------------CREATE
	 * GEOMETRY------------------------------*/
	GLfloat *vp = NULL; // array of vertex points
	GLfloat *vn = NULL; // array of vertex normals
	GLfloat *vt = NULL; // array of texture coordinates
	int g_point_count = 0;
	assert( load_obj_file( MESH_FILE, vp, vt, vn, g_point_count ) );

	GLuint vao;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	GLuint points_vbo;
	if ( NULL != vp ) {
		glGenBuffers( 1, &points_vbo );
		glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
		glBufferData( GL_ARRAY_BUFFER, 3 * g_point_count * sizeof( GLfloat ), vp,
									GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
	}
	GLuint normals_vbo;
	if ( NULL != vn ) {
		glGenBuffers( 1, &normals_vbo );
		glBindBuffer( GL_ARRAY_BUFFER, normals_vbo );
		glBufferData( GL_ARRAY_BUFFER, 3 * g_point_count * sizeof( GLfloat ), vn,
									GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
	}
	GLuint texcoords_vbo;
	if ( NULL != vp ) {
		glGenBuffers( 1, &texcoords_vbo );
		glBindBuffer( GL_ARRAY_BUFFER, texcoords_vbo );
		glBufferData( GL_ARRAY_BUFFER, 2 * g_point_count * sizeof( GLfloat ), vp,
									GL_STATIC_DRAW );
		glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 2 );
	}

	/*-------------------------------CREATE
	 * SHADERS------------------------------*/
	GLuint shader_programme =
		create_programme_from_files( VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE );
	int view_mat_location = glGetUniformLocation( shader_programme, "view" );
	int proj_mat_location = glGetUniformLocation( shader_programme, "proj" );

	/* if converting to GLSL 410 do this to replace GLSL texture bindings:
	GLint diffuse_map_loc, specular_map_loc, ambient_map_loc, emission_map_loc;
	diffuse_map_loc = glGetUniformLocation (shader_programme, "diffuse_map");
	specular_map_loc = glGetUniformLocation (shader_programme, "specular_map");
	ambient_map_loc = glGetUniformLocation (shader_programme, "ambient_map");
	emission_map_loc = glGetUniformLocation (shader_programme, "emission_map");
	assert (diffuse_map_loc > -1);
	assert (specular_map_loc > -1);
	assert (ambient_map_loc > -1);
	assert (emission_map_loc > -1);
	glUseProgram (shader_programme);
	glUniform1i (diffuse_map_loc, 0);
	glUniform1i (specular_map_loc, 1);
	glUniform1i (ambient_map_loc, 2);
	glUniform1i (emission_map_loc, 3);
	*/

	// load texture
	GLuint tex_diff, tex_spec, tex_amb, tex_emiss;
	glActiveTexture( GL_TEXTURE0 );
	assert( load_texture( "boulder_diff.png", &tex_diff ) );
	glActiveTexture( GL_TEXTURE1 );
	assert( load_texture( "boulder_spec.png", &tex_spec ) );
	glActiveTexture( GL_TEXTURE2 );
	assert( load_texture( "ao.png", &tex_amb ) );
	glActiveTexture( GL_TEXTURE3 );
	assert( load_texture( "tileable9b_emiss.png", &tex_emiss ) );

#define ONE_DEG_IN_RAD ( 2.0 * M_PI ) / 360.0 // 0.017444444
	// input variables
	float near = 0.1f;									// clipping plane
	float far = 100.0f;									// clipping plane
	float fov = 67.0f * ONE_DEG_IN_RAD; // convert 67 degrees to radians
	float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
	// matrix components
	float range = tan( fov * 0.5f ) * near;
	float Sx = ( 2.0f * near ) / ( range * aspect + range * aspect );
	float Sy = near / range;
	float Sz = -( far + near ) / ( far - near );
	float Pz = -( 2.0f * far * near ) / ( far - near );
	GLfloat proj_mat[] = { Sx,	 0.0f, 0.0f, 0.0f,	0.0f, Sy,		0.0f, 0.0f,
												 0.0f, 0.0f, Sz,	 -1.0f, 0.0f, 0.0f, Pz,		0.0f };

	float cam_speed = 1.0f;			 // 1 unit per second
	float cam_yaw_speed = 10.0f; // 10 degrees per second
															 // don't start at zero, or we will be too close
	float cam_pos[] = { 0.0f, 0.0f, 5.0f };
	float cam_yaw = 0.0f; // y-rotation in degrees
	mat4 T =
		translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1], -cam_pos[2] ) );
	mat4 R = rotate_y_deg( identity_mat4(), -cam_yaw );
	mat4 view_mat = R * T;

	glUseProgram( shader_programme );
	glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
	glUniformMatrix4fv( proj_mat_location, 1, GL_FALSE, proj_mat );

	while ( !glfwWindowShouldClose( g_window ) ) {
		static double previous_seconds = glfwGetTime();
		double current_seconds = glfwGetTime();
		double elapsed_seconds = current_seconds - previous_seconds;
		previous_seconds = current_seconds;

		_update_fps_counter( g_window );
		// wipe the drawing surface clear
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glViewport( 0, 0, g_gl_width, g_gl_height );

		glUseProgram( shader_programme );
		glBindVertexArray( vao );
		// draw points 0-3 from the currently bound VAO with current shader
		glDrawArrays( GL_TRIANGLES, 0, g_point_count );
		// update other events like input handling
		glfwPollEvents();

		// control keys
		bool cam_moved = false;
		if ( glfwGetKey( g_window, GLFW_KEY_A ) ) {
			cam_pos[0] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_D ) ) {
			cam_pos[0] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_PAGE_UP ) ) {
			cam_pos[1] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_PAGE_DOWN ) ) {
			cam_pos[1] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_W ) ) {
			cam_pos[2] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_S ) ) {
			cam_pos[2] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_LEFT ) ) {
			cam_yaw += cam_yaw_speed * elapsed_seconds;
			cam_moved = true;
		}
		if ( glfwGetKey( g_window, GLFW_KEY_RIGHT ) ) {
			cam_yaw -= cam_yaw_speed * elapsed_seconds;
			cam_moved = true;
		}
		// update view matrix
		if ( cam_moved ) {
			mat4 T = translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1],
																								 -cam_pos[2] ) ); // cam translation
			mat4 R = rotate_y_deg( identity_mat4(), -cam_yaw );					//
			mat4 view_mat = R * T;
			glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
		}

		if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) {
			glfwSetWindowShouldClose( g_window, 1 );
		}
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers( g_window );
	}

	// close GL context and any other GLFW resources
	glfwTerminate();
	return 0;
}
