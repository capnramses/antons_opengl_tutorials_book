/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| Doing post-processing with a secondary framebuffer                           |
\******************************************************************************/

#include "gl_utils.h"
#include "maths_funcs.h"
#include "obj_parser.h"
#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <stdio.h>

#define POST_VS "post.vert"
#define POST_FS "post.frag"
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
/* global variable for the screen-space quad that we will draw */
GLuint g_ss_quad_vao = 0;

/* sphere */
GLuint g_sphere_vao = 0;
int g_sphere_point_count = 0;

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

/* create 2 triangles that cover the whole screen. after rendering the scene to
a texture we will texture the quad with it. this will look like a normal scene
rendering, but means that we can perform image filters after all the rendering
is done */
void init_ss_quad() {
	// x,y vertex positions
	GLfloat ss_quad_pos[] = { -1.0, -1.0, 1.0,	-1.0, 1.0,	1.0,
														1.0,	1.0,	-1.0, 1.0,	-1.0, -1.0 };
	// create VBOs and VAO in the usual way
	glGenVertexArrays( 1, &g_ss_quad_vao );
	glBindVertexArray( g_ss_quad_vao );
	GLuint vbo;
	glGenBuffers( 1, &vbo );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, sizeof( ss_quad_pos ), ss_quad_pos,
								GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, NULL );
	glEnableVertexAttribArray( 0 );
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

int main() {
	assert( restart_gl_log() );
	assert( start_gl() );
	/* set up framebuffer with texture attachment */
	assert( init_fb() );
	init_ss_quad();
	/* load the post-processing effect shaders */
	GLuint post_sp = create_programme_from_files( POST_VS, POST_FS );
	/* load a mesh to draw in the main scene */
	load_sphere();
	GLuint sphere_sp = create_programme_from_files( SPHERE_VS, SPHERE_FS );
	GLint sphere_P_loc = glGetUniformLocation( sphere_sp, "P" );
	GLint sphere_V_loc = glGetUniformLocation( sphere_sp, "V" );
	assert( sphere_P_loc > -1 );
	assert( sphere_V_loc > -1 );
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
		glFlush();
		glFinish();
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, 0 );
		glBindFramebuffer( GL_FRAMEBUFFER, g_fb );
		/* clear the framebuffer's colour and depth buffers */
		glClearColor( 0.2, 0.2, 0.2, 1.0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		// render an obj or something
		glUseProgram( sphere_sp );
		glBindVertexArray( g_sphere_vao );
		glDrawArrays( GL_TRIANGLES, 0, g_sphere_point_count );

		/* bind default framebuffer for post-processing effects. sample texture
		from previous pass */

		glFlush();
		glFinish();
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		// clear the framebuffer's colour and depth buffers
		//		glClearColor (0.0, 0.0, 0.0, 1.0);
		//		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// use our post-processing shader for the screen-space quad
		glUseProgram( post_sp );
		// bind the quad's VAO
		glBindVertexArray( g_ss_quad_vao );
		// activate the first texture slot and put texture from previous pass in it
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, g_fb_tex );
		// draw the quad that covers the screen area
		glDrawArrays( GL_TRIANGLES, 0, 6 );

		// flip drawn framebuffer onto the display
		glfwSwapBuffers( g_window );
		glfwPollEvents();
		if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) {
			glfwSetWindowShouldClose( g_window, 1 );
		}
	}
	return 0;
}
