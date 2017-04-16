/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| This demo uses the Assimp library to load a mesh from a file, and supports   |
| many formats. The library is VERY big and complex. It's much easier to write |
| a simple Wavefront .obj loader. I have code for this in other demos. However,|
| Assimp will load animated meshes, which will we need to use later, so this   |
| demo is a starting point before doing skinning animation                     |
\******************************************************************************/
#include "gl_utils.h"
#include "maths_funcs.h"
#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <assimp/cimport.h>			// C importer
#include <assimp/postprocess.h> // various extra operations
#include <assimp/scene.h>				// collects data
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define GL_LOG_FILE "gl.log"
#define VERTEX_SHADER_FILE "test_vs.glsl"
#define FRAGMENT_SHADER_FILE "test_fs.glsl"
#define MESH_FILE "monkey2.obj"

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 640;
int g_gl_height = 480;
GLFWwindow *g_window = NULL;

/* load a mesh using the assimp library */
bool load_mesh( const char *file_name, GLuint *vao, int *point_count ) {
	const aiScene *scene = aiImportFile( file_name, aiProcess_Triangulate );
	if ( !scene ) {
		fprintf( stderr, "ERROR: reading mesh %s\n", file_name );
		return false;
	}
	printf( "  %i animations\n", scene->mNumAnimations );
	printf( "  %i cameras\n", scene->mNumCameras );
	printf( "  %i lights\n", scene->mNumLights );
	printf( "  %i materials\n", scene->mNumMaterials );
	printf( "  %i meshes\n", scene->mNumMeshes );
	printf( "  %i textures\n", scene->mNumTextures );

	/* get first mesh in file only */
	const aiMesh *mesh = scene->mMeshes[0];
	printf( "    %i vertices in mesh[0]\n", mesh->mNumVertices );

	/* pass back number of vertex points in mesh */
	*point_count = mesh->mNumVertices;

	/* generate a VAO, using the pass-by-reference parameter that we give to the
	function */
	glGenVertexArrays( 1, vao );
	glBindVertexArray( *vao );

	/* we really need to copy out all the data from AssImp's funny little data
	structures into pure contiguous arrays before we copy it into data buffers
	because assimp's texture coordinates are not really contiguous in memory.
	i allocate some dynamic memory to do this. */
	GLfloat *points = NULL;		 // array of vertex points
	GLfloat *normals = NULL;	 // array of vertex normals
	GLfloat *texcoords = NULL; // array of texture coordinates
	if ( mesh->HasPositions() ) {
		points = (GLfloat *)malloc( *point_count * 3 * sizeof( GLfloat ) );
		for ( int i = 0; i < *point_count; i++ ) {
			const aiVector3D *vp = &( mesh->mVertices[i] );
			points[i * 3] = (GLfloat)vp->x;
			points[i * 3 + 1] = (GLfloat)vp->y;
			points[i * 3 + 2] = (GLfloat)vp->z;
		}
	}
	if ( mesh->HasNormals() ) {
		normals = (GLfloat *)malloc( *point_count * 3 * sizeof( GLfloat ) );
		for ( int i = 0; i < *point_count; i++ ) {
			const aiVector3D *vn = &( mesh->mNormals[i] );
			normals[i * 3] = (GLfloat)vn->x;
			normals[i * 3 + 1] = (GLfloat)vn->y;
			normals[i * 3 + 2] = (GLfloat)vn->z;
		}
	}
	if ( mesh->HasTextureCoords( 0 ) ) {
		texcoords = (GLfloat *)malloc( *point_count * 2 * sizeof( GLfloat ) );
		for ( int i = 0; i < *point_count; i++ ) {
			const aiVector3D *vt = &( mesh->mTextureCoords[0][i] );
			texcoords[i * 2] = (GLfloat)vt->x;
			texcoords[i * 2 + 1] = (GLfloat)vt->y;
		}
	}

	/* copy mesh data into VBOs */
	if ( mesh->HasPositions() ) {
		GLuint vbo;
		glGenBuffers( 1, &vbo );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glBufferData( GL_ARRAY_BUFFER, 3 * *point_count * sizeof( GLfloat ), points,
									GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		free( points );
	}
	if ( mesh->HasNormals() ) {
		GLuint vbo;
		glGenBuffers( 1, &vbo );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glBufferData( GL_ARRAY_BUFFER, 3 * *point_count * sizeof( GLfloat ), normals,
									GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
		free( normals );
	}
	if ( mesh->HasTextureCoords( 0 ) ) {
		GLuint vbo;
		glGenBuffers( 1, &vbo );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glBufferData( GL_ARRAY_BUFFER, 2 * *point_count * sizeof( GLfloat ), texcoords,
									GL_STATIC_DRAW );
		glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 2 );
		free( texcoords );
	}
	if ( mesh->HasTangentsAndBitangents() ) {
		// NB: could store/print tangents here
	}

	aiReleaseImport( scene );
	printf( "mesh loaded\n" );

	return true;
}

int main() {
	restart_gl_log();
	start_gl();
	glEnable( GL_DEPTH_TEST ); // enable depth-testing
	glDepthFunc( GL_LESS );		 // depth-testing interprets a smaller value as "closer"
	glEnable( GL_CULL_FACE );	// cull face
	glCullFace( GL_BACK );		 // cull back face
	glFrontFace( GL_CCW ); // set counter-clock-wise vertex order to mean the front
	glClearColor( 0.2, 0.2, 0.2, 1.0 ); // grey background to help spot mistakes
	glViewport( 0, 0, g_gl_width, g_gl_height );

	/* load the mesh using assimp */
	GLuint monkey_vao;
	int monkey_point_count = 0;
	( load_mesh( MESH_FILE, &monkey_vao, &monkey_point_count ) );

	/*-------------------------------CREATE
	 * SHADERS-------------------------------*/
	GLuint shader_programme =
		create_programme_from_files( VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE );

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
	float cam_pos[] = { 0.0f, 0.0f,
											5.0f }; // don't start at zero, or we will be too close
	float cam_yaw = 0.0f;				// y-rotation in degrees
	mat4 T =
		translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1], -cam_pos[2] ) );
	mat4 R = rotate_y_deg( identity_mat4(), -cam_yaw );
	mat4 view_mat = R * T;

	int view_mat_location = glGetUniformLocation( shader_programme, "view" );
	glUseProgram( shader_programme );
	glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
	int proj_mat_location = glGetUniformLocation( shader_programme, "proj" );
	glUseProgram( shader_programme );
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
		glBindVertexArray( monkey_vao );
		glDrawArrays( GL_TRIANGLES, 0, monkey_point_count );
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
