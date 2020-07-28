/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| Vertex Buffer Objects and interpolation                                      |
\******************************************************************************/
#include "gl_utils.h"   // utility stuff discussed in previous tutorials is here
#include <GL/glew.h>    // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define GL_LOG_FILE "gl.log"

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width       = 640;
int g_gl_height      = 480;
GLFWwindow* g_window = NULL;

int main() {
  restart_gl_log();
  // all the start-up code for GLFW and GLEW is called here
  start_gl();

  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable( GL_DEPTH_TEST ); // enable depth-testing
  glDepthFunc( GL_LESS );    // depth-testing interprets a smaller value as "closer"

  /* OTHER STUFF GOES HERE NEXT */
  GLfloat points[] = { 0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f };

  GLfloat colours[] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

  GLuint points_vbo;
  glGenBuffers( 1, &points_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( GLfloat ), points, GL_STATIC_DRAW );

  /* create a second VBO, containing the array of colours.
  note that we could also put them both into a single vertex buffer. in this
  case we would use the pointer and stride parameters of glVertexAttribPointer()
  to describe the different data layouts */
  GLuint colours_vbo;
  glGenBuffers( 1, &colours_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, colours_vbo );
  glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( GLfloat ), colours, GL_STATIC_DRAW );

  /* create the VAO.
  we bind each VBO in turn, and call glVertexAttribPointer() to indicate where
  the memory should be fetched for vertex shader input variables 0, and 1,
  respectively. we also have to explicitly enable both 'attribute' variables.
  'attribute' is the older name for vertex shader 'in' variables. */
  GLuint vao;
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glBindBuffer( GL_ARRAY_BUFFER, colours_vbo );
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );
  glEnableVertexAttribArray( 1 );

  char vertex_shader[1024 * 256];
  char fragment_shader[1024 * 256];
  parse_file_into_str( "test_vs.glsl", vertex_shader, 1024 * 256 );
  parse_file_into_str( "test_fs.glsl", fragment_shader, 1024 * 256 );

  GLuint vs       = glCreateShader( GL_VERTEX_SHADER );
  const GLchar* p = (const GLchar*)vertex_shader;
  glShaderSource( vs, 1, &p, NULL );
  glCompileShader( vs );

  // check for compile errors
  int params = -1;
  glGetShaderiv( vs, GL_COMPILE_STATUS, &params );
  if ( GL_TRUE != params ) {
    fprintf( stderr, "ERROR: GL shader index %i did not compile\n", vs );
    print_shader_info_log( vs );
    return 1; // or exit or something
  }

  GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
  p         = (const GLchar*)fragment_shader;
  glShaderSource( fs, 1, &p, NULL );
  glCompileShader( fs );

  // check for compile errors
  glGetShaderiv( fs, GL_COMPILE_STATUS, &params );
  if ( GL_TRUE != params ) {
    fprintf( stderr, "ERROR: GL shader index %i did not compile\n", fs );
    print_shader_info_log( fs );
    return 1; // or exit or something
  }

  GLuint shader_programme = glCreateProgram();
  glAttachShader( shader_programme, fs );
  glAttachShader( shader_programme, vs );
  glLinkProgram( shader_programme );

  glGetProgramiv( shader_programme, GL_LINK_STATUS, &params );
  if ( GL_TRUE != params ) {
    fprintf( stderr, "ERROR: could not link shader programme GL index %i\n", shader_programme );
    print_programme_info_log( shader_programme );
    return false;
  }

  glEnable( GL_CULL_FACE ); // cull face
  glCullFace( GL_BACK );    // cull back face
  glFrontFace( GL_CW );     // GL_CCW for counter clock-wise

  while ( !glfwWindowShouldClose( g_window ) ) {
    _update_fps_counter( g_window );
    // wipe the drawing surface clear
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, g_gl_width, g_gl_height );

    glUseProgram( shader_programme );
    glBindVertexArray( vao );
    // draw points 0-3 from the currently bound VAO with current in-use shader
    glDrawArrays( GL_TRIANGLES, 0, 3 );
    // update other events like input handling
    glfwPollEvents();
    if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) { glfwSetWindowShouldClose( g_window, 1 ); }
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers( g_window );
  }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
