/******************************************************************************\
| OpenGL 4 Example Code.																											 |
| Accompanies written series "Anton's OpenGL 4 Tutorials"											|
| Email: anton at antongerdelan dot net																				|
| First version 27 Jan 2014																										|
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.								|
| See individual libraries for separate legal notices													|
|******************************************************************************|
| Doing post-processing with a secondary framebuffer													 |
\******************************************************************************/

#include "gl_utils.h"
#include "maths_funcs.h"
#include "obj_parser.h"
#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <stdio.h>

#define PICK_VS "pick.vert"
#define PICK_FS "pick.frag"
#define SPHERE_VS "sphere.vert"
#define SPHERE_FS "sphere.frag"
#define MESH_FILE "sphere.obj"

/* window global variables */
int g_gl_width = 800;
int g_gl_height = 800;
GLFWwindow *g_window = NULL;

/* global variables for the secondary framebuffer handle and attached texture */
GLuint g_fb = 0;
GLuint g_fb_tex = 0;
GLuint g_pick_sp = 0;
GLint g_pick_unique_id_loc = -1;
GLint g_pick_P_loc = -1;
GLint g_pick_V_loc = -1;
GLint g_pick_M_loc = -1;

/* sphere */
GLuint g_sphere_vao = 0;
int g_sphere_point_count = 0;

// encode an unique ID into a colour with components in range of 0.0 to 1.0
vec3 encode_id( int id ) {
	int r = id / 65536;
	int g = ( id - r * 65536 ) / 256;
	int b = ( id - r * 65536 - g * 256 );

	// convert to floats. only divide by 255, because range is 0-255
	return vec3( (float)r / 255.0, (float)g / 255.0, (float)b / 255.0 );
}

/* initialise secondary framebuffer. this will just allow us to render our main
scene to a texture instead of directly to the screen. returns false if something
went wrong in the framebuffer creation */
bool init_fb() {
	glGenFramebuffers( 1, &g_fb );
	/* create the texture that will be attached to the fb. should be the same
	dimensions as the viewport */
	glGenTextures( 1, &g_fb_tex );
	glBindTexture( GL_TEXTURE_2D, g_fb_tex );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, g_gl_width, g_gl_height, 0, GL_RGBA,
								GL_UNSIGNED_BYTE, NULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	/* attach the texture to the framebuffer */
	glBindFramebuffer( GL_FRAMEBUFFER, g_fb );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
													g_fb_tex, 0 );
	/* create a renderbuffer which allows depth-testing in the framebuffer */
	GLuint rb = 0;
	glGenRenderbuffers( 1, &rb );
	glBindRenderbuffer( GL_RENDERBUFFER, rb );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_gl_width,
												 g_gl_height );
	/* attach renderbuffer to framebuffer */
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
														 rb );
	/* tell the framebuffer to expect a colour output attachment (our texture) */
	GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, draw_bufs );

	/* validate the framebuffer - an 'incomplete' error tells us if an invalid
	image format is attached or if the glDrawBuffers information is invalid */
	GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );
	if ( GL_FRAMEBUFFER_COMPLETE != status ) {
		fprintf( stderr, "ERROR: incomplete framebuffer\n" );
		if ( GL_FRAMEBUFFER_UNDEFINED == status ) {
			fprintf( stderr, "GL_FRAMEBUFFER_UNDEFINED\n" );
		} else if ( GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT == status ) {
			fprintf( stderr, "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n" );
		} else if ( GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT == status ) {
			fprintf( stderr, "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n" );
		} else if ( GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER == status ) {
			fprintf( stderr, "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n" );
		} else if ( GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER == status ) {
			fprintf( stderr, "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n" );
		} else if ( GL_FRAMEBUFFER_UNSUPPORTED == status ) {
			fprintf( stderr, "GL_FRAMEBUFFER_UNSUPPORTED\n" );
		} else if ( GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE == status ) {
			fprintf( stderr, "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n" );
		} else if ( GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS == status ) {
			fprintf( stderr, "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n" );
		} else {
			fprintf( stderr, "unspecified error\n" );
		}
		return false;
	}

	/* re-bind the default framebuffer as a safe precaution */
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	return true;
}

void load_sphere() {
	float *points = NULL;
	float *tex_coords = NULL;
	float *normals = NULL;
	g_sphere_point_count = 0;
	assert(
		load_obj_file( MESH_FILE, points, tex_coords, normals, g_sphere_point_count ) );
	glGenVertexArrays( 1, &g_sphere_vao );
	glBindVertexArray( g_sphere_vao );
	GLuint vbo;
	glGenBuffers( 1, &vbo );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof( float ) * 3 * g_sphere_point_count, points,
								GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
	glEnableVertexAttribArray( 0 );
}

bool debug_colours = false;
void draw_picker_colours( mat4 P, mat4 V, mat4 M[3] ) {
	if ( debug_colours ) {
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	} else {
		glBindFramebuffer( GL_FRAMEBUFFER, g_fb );
	}
	glViewport( 0, 0, g_gl_width, g_gl_height );
	glClearColor( 0.0, 0.0, 0.0, 1.0 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glUseProgram( g_pick_sp );
	// set colour
	vec3 id = encode_id( 255 );
	glUniform3f( g_pick_unique_id_loc, id.v[0], id.v[1], id.v[2] );
	glUniformMatrix4fv( g_pick_P_loc, 1, GL_FALSE, P.m );
	glUniformMatrix4fv( g_pick_V_loc, 1, GL_FALSE, V.m );
	glUniformMatrix4fv( g_pick_M_loc, 1, GL_FALSE, M[0].m );

	glBindVertexArray( g_sphere_vao );
	glDrawArrays( GL_TRIANGLES, 0, g_sphere_point_count );

	// 2nd sphere
	id = encode_id( 65280 );
	glUniformMatrix4fv( g_pick_M_loc, 1, GL_FALSE, M[1].m );
	glUniform3f( g_pick_unique_id_loc, id.v[0], id.v[1], id.v[2] );
	glDrawArrays( GL_TRIANGLES, 0, g_sphere_point_count );

	// 3rd sphere
	id = encode_id( 16711680 );
	glUniformMatrix4fv( g_pick_M_loc, 1, GL_FALSE, M[2].m );
	glUniform3f( g_pick_unique_id_loc, id.v[0], id.v[1], id.v[2] );
	glDrawArrays( GL_TRIANGLES, 0, g_sphere_point_count );

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

int decode_id( int r, int g, int b ) { return b + g * 256 + r * 256 * 256; }

int main() {
	( restart_gl_log() );
	( start_gl() );
	/* load a mesh to draw in the main scene */
	load_sphere();
	/* set up framebuffer with texture attachment */
	( init_fb() );
	/* load the picking shaders */
	g_pick_sp = create_programme_from_files( PICK_VS, PICK_FS );
	g_pick_unique_id_loc = glGetUniformLocation( g_pick_sp, "unique_id" );
	g_pick_P_loc = glGetUniformLocation( g_pick_sp, "P" );
	g_pick_V_loc = glGetUniformLocation( g_pick_sp, "V" );
	g_pick_M_loc = glGetUniformLocation( g_pick_sp, "M" );
	assert( g_pick_P_loc > -1 );
	assert( g_pick_V_loc > -1 );
	assert( g_pick_M_loc > -1 );
	GLuint sphere_sp = create_programme_from_files( SPHERE_VS, SPHERE_FS );
	GLint sphere_P_loc = glGetUniformLocation( sphere_sp, "P" );
	GLint sphere_V_loc = glGetUniformLocation( sphere_sp, "V" );
	GLint sphere_M_loc = glGetUniformLocation( sphere_sp, "M" );
	assert( sphere_P_loc > -1 );
	assert( sphere_V_loc > -1 );
	assert( sphere_M_loc > -1 );
	/* set up view and projection matrices for sphere shader */
	mat4 P =
		perspective( 67.0f, (float)g_gl_width / (float)g_gl_height, 0.1f, 100.0f );
	mat4 V = look_at( vec3( 0.0f, 0.0f, 5.0f ), vec3( 0.0f, 0.0f, 0.0f ),
										vec3( 0.0f, 1.0f, 0.0f ) );
	glUseProgram( sphere_sp );
	glUniformMatrix4fv( sphere_P_loc, 1, GL_FALSE, P.m );
	glUniformMatrix4fv( sphere_V_loc, 1, GL_FALSE, V.m );

	glViewport( 0, 0, g_gl_width, g_gl_height );

	while ( !glfwWindowShouldClose( g_window ) ) {
		_update_fps_counter( g_window );

		/* bind the 'render to a texture' framebuffer for main scene */
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		/* clear the framebuffer's colour and depth buffers */
		glClearColor( 0.2, 0.2, 0.2, 1.0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		// render an obj or something
		glUseProgram( sphere_sp );
		glBindVertexArray( g_sphere_vao );

		// model matrices for all 3 spheres
		mat4 Ms[3];
		// first sphere
		Ms[0] = identity_mat4();
		glUniformMatrix4fv( sphere_M_loc, 1, GL_FALSE, Ms[0].m );
		glDrawArrays( GL_TRIANGLES, 0, g_sphere_point_count );
		// 2nd sphere
		Ms[1] = translate( identity_mat4(), vec3( 1.0, -1.0, -4.0 ) );
		glUniformMatrix4fv( sphere_M_loc, 1, GL_FALSE, Ms[1].m );
		glDrawArrays( GL_TRIANGLES, 0, g_sphere_point_count );
		// 3rd sphere
		Ms[2] = translate( identity_mat4(), vec3( -0.50, 2.0, -2.0 ) );
		glUniformMatrix4fv( sphere_M_loc, 1, GL_FALSE, Ms[2].m );
		glDrawArrays( GL_TRIANGLES, 0, g_sphere_point_count );

		/* bind framebuffer for pick */
		draw_picker_colours( P, V, Ms );

		// flip drawn framebuffer onto the display
		glfwSwapBuffers( g_window );
		glfwPollEvents();
		if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) {
			glfwSetWindowShouldClose( g_window, 1 );
		}
		debug_colours = glfwGetKey( g_window, GLFW_KEY_SPACE );
		if ( glfwGetMouseButton( g_window, 0 ) ) {
			glBindFramebuffer( GL_FRAMEBUFFER, g_fb );
			double xpos, ypos;
			glfwGetCursorPos( g_window, &xpos, &ypos );
			int mx = (int)xpos;
			int my = (int)ypos;
			unsigned char data[4] = { 0, 0, 0, 0 };
			glReadPixels( mx, g_gl_height - my, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &data );
			int id = decode_id( (int)data[0], (int)data[1], (int)data[2] );
			int mid = -1;
			if ( id == 255 ) {
				mid = 0;
			}
			if ( id == 65280 ) {
				mid = 1;
			}
			if ( id == 16711680 ) {
				mid = 2;
			}
			printf( "%i,%i,%i means -> id was %i, and monkey number is %i\n", data[0],
							data[1], data[2], id, mid );
			glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		}
	}
	return 0;
}
