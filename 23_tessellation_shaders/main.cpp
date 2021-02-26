/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                          |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| Tessellation Shaders Example                                                 |
| Raise/Lower "inner" tessellation factor - Q,A keys                           |
| Raise/Lower "outer" tessellation factor - W,S keys                           |
\******************************************************************************/
#include "gl_utils.h"   // i put all the clutter and little functions here
#include <GL/glew.h>    // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width  = 640;
int g_gl_height = 480;

float inner_tess_fac = 1.0;
float outer_tess_fac = 4.0;

int main() {
  restart_gl_log();
  // start GL context and O/S window using the GLFW helper library
  gl_log( "starting GLFW %s", glfwGetVersionString() );

  glfwSetErrorCallback( glfw_error_callback );
  if ( !glfwInit() ) {
    gl_log_err( "ERROR: could not start GLFW3\n" );
    return 1;
  }

  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
  glfwWindowHint( GLFW_SAMPLES, 4 );

  /*GLFWmonitor* mon = glfwGetPrimaryMonitor ();
  const GLFWvidmode* vmode = glfwGetVideoMode (mon);
  GLFWwindow* window = glfwCreateWindow (
          vmode->width, vmode->height, "Extended GL Init", mon, NULL
  );*/

  GLFWwindow* window = glfwCreateWindow( g_gl_width, g_gl_height, "Extended Init.", NULL, NULL );
  if ( !window ) {
    gl_log_err( "ERROR: could not open window with GLFW3\n" );
    glfwTerminate();
    return 1;
  }
  glfwSetFramebufferSizeCallback( window, glfw_framebuffer_size_callback );
  glfwMakeContextCurrent( window );


  // start GLEW extension handler
  glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  const GLubyte* renderer = glGetString( GL_RENDERER ); // get renderer string
  const GLubyte* version  = glGetString( GL_VERSION );  // version as a string
  printf( "Renderer: %s\n", renderer );
  printf( "OpenGL version supported %s\n", version );
  gl_log( "renderer: %s\nversion: %s\n", renderer, version );
  log_gl_params();

  // query hardware support for tessellation
  int max_patch_vertices = 0;
  glGetIntegerv( GL_MAX_PATCH_VERTICES, &max_patch_vertices );
  printf( "Max supported patch vertices %i\n", max_patch_vertices );

  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable( GL_DEPTH_TEST ); // enable depth-testing
  glDepthFunc( GL_LESS );    // depth-testing interprets a smaller value as "closer"
  glClearColor( 0.2, 0.2, 0.2, 1.0 );

  /* i've made the mesh 2 triangles, to help illustrate the numbers used for
  patch size versus points to draw */
  GLfloat points[] = { 0.0f, 0.75f, 0.0f, 0.5f, 0.25f, 0.0f, -0.5f, 0.25f, 0.0f, 0.5f, -0.25f, 0.0f, 0.0f, -0.75f, 0.0f, -0.5f, -0.25f, 0.0f };

  GLuint points_vbo;
  glGenBuffers( 1, &points_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  glBufferData( GL_ARRAY_BUFFER, sizeof( points ), points, GL_STATIC_DRAW );

  GLuint vao;
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );

  char vertex_shader[4096];
  char tess_ctrl_shader[4096];
  char tess_eval_shader[4096];
  char fragment_shader[4096];
  ( parse_file_into_str( "test_vs.glsl", vertex_shader, 4096 ) );
  ( parse_file_into_str( "test_tcs.glsl", tess_ctrl_shader, 4096 ) );
  ( parse_file_into_str( "test_tes.glsl", tess_eval_shader, 4096 ) );
  ( parse_file_into_str( "test_fs.glsl", fragment_shader, 4096 ) );

  GLuint vs       = glCreateShader( GL_VERTEX_SHADER );
  const GLchar* p = (const GLchar*)vertex_shader;
  glShaderSource( vs, 1, &p, NULL );
  glCompileShader( vs );

  // check for compile errors
  int params = -1;
  glGetShaderiv( vs, GL_COMPILE_STATUS, &params );
  if ( GL_TRUE != params ) {
    gl_log_err( "ERROR: GL shader index %i did not compile\n", vs );
    _print_shader_info_log( vs );
    return 1; // or exit or something
  }

  // compile tessellation control shader
  GLuint tcs = glCreateShader( GL_TESS_CONTROL_SHADER );
  p          = (const GLchar*)tess_ctrl_shader;
  glShaderSource( tcs, 1, &p, NULL );
  glCompileShader( tcs );
  params = -1;
  glGetShaderiv( tcs, GL_COMPILE_STATUS, &params );
  if ( GL_TRUE != params ) {
    gl_log_err( "ERROR: GL shader index %i did not compile\n", tcs );
    _print_shader_info_log( tcs );
    return 1; // or exit or something
  }

  GLuint tes = glCreateShader( GL_TESS_EVALUATION_SHADER );
  p          = (const GLchar*)tess_eval_shader;
  glShaderSource( tes, 1, &p, NULL );
  glCompileShader( tes );
  params = -1;
  glGetShaderiv( tes, GL_COMPILE_STATUS, &params );
  if ( GL_TRUE != params ) {
    gl_log_err( "ERROR: GL shader index %i did not compile\n", tes );
    _print_shader_info_log( tes );
    return 1; // or exit or something
  }

  GLuint fs = glCreateShader( GL_FRAGMENT_SHADER );
  p         = (const GLchar*)fragment_shader;
  glShaderSource( fs, 1, &p, NULL );
  glCompileShader( fs );
  // check for compile errors
  glGetShaderiv( fs, GL_COMPILE_STATUS, &params );
  if ( GL_TRUE != params ) {
    gl_log_err( "ERROR: GL shader index %i did not compile\n", fs );
    _print_shader_info_log( fs );
    return 1; // or exit or something
  }

  GLuint shader_programme = glCreateProgram();
  glAttachShader( shader_programme, fs );
  // attach both tessellation
  glAttachShader( shader_programme, tes );
  glAttachShader( shader_programme, tcs );
  glAttachShader( shader_programme, vs );
  glLinkProgram( shader_programme );
  int outer_tess_fac_loc = glGetUniformLocation( shader_programme, "tess_fac_outer" );
  int inner_tess_fac_loc = glGetUniformLocation( shader_programme, "tess_fac_inner" );

  glGetProgramiv( shader_programme, GL_LINK_STATUS, &params );
  if ( GL_TRUE != params ) {
    gl_log_err( "ERROR: could not link shader programme GL index %i\n", shader_programme );
    _print_programme_info_log( shader_programme );
    return false;
  }
  ( is_valid( shader_programme ) );

  glEnable( GL_CULL_FACE ); // cull face
  glCullFace( GL_BACK );    // cull back face
  glFrontFace( GL_CW );     // GL_CCW for counter clock-wise
  // NB. front or back alone didn't work on OSX -- had to use F&B here
  glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  // i'm drawing a base mesh comprised of triangles (3 points per patch)
  glPatchParameteri( GL_PATCH_VERTICES, 3 );
  while ( !glfwWindowShouldClose( window ) ) {
    _update_fps_counter( window );
    // wipe the drawing surface clear
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, g_gl_width, g_gl_height );

    glUseProgram( shader_programme );
    glBindVertexArray( vao );
    // set patch parameters - I'm drawing a mesh comprising 2 triangles, so '6'
    glDrawArrays( GL_PATCHES, 0, 6 );
    // update other events like input handling
    glfwPollEvents();
    if ( GLFW_PRESS == glfwGetKey( window, GLFW_KEY_ESCAPE ) ) { glfwSetWindowShouldClose( window, 1 ); }
    // handle key controls for controlling tessellation factors; q,a,w,s
    static bool q_was_down = false;
    static bool a_was_down = false;
    static bool w_was_down = false;
    static bool s_was_down = false;
    if ( GLFW_PRESS == glfwGetKey( window, GLFW_KEY_Q ) ) {
      if ( !q_was_down ) {
        inner_tess_fac += 1.0f;
        printf( "inner tess. factor = %.1f\n", inner_tess_fac );
        q_was_down = true;
        glUniform1f( inner_tess_fac_loc, inner_tess_fac );
      }
    } else {
      q_was_down = false;
    }
    if ( GLFW_PRESS == glfwGetKey( window, GLFW_KEY_A ) ) {
      if ( !a_was_down ) {
        inner_tess_fac -= 1.0f;
        printf( "inner tess. factor = %.1f\n", inner_tess_fac );
        a_was_down = true;
        glUniform1f( inner_tess_fac_loc, inner_tess_fac );
      }
    } else {
      a_was_down = false;
    }
    if ( GLFW_PRESS == glfwGetKey( window, GLFW_KEY_W ) ) {
      if ( !w_was_down ) {
        outer_tess_fac += 1.0f;
        printf( "outer tess. factor = %.1f\n", outer_tess_fac );
        w_was_down = true;
        glUniform1f( outer_tess_fac_loc, outer_tess_fac );
      }
    } else {
      w_was_down = false;
    }
    if ( GLFW_PRESS == glfwGetKey( window, GLFW_KEY_S ) ) {
      if ( !s_was_down ) {
        outer_tess_fac -= 1.0f;
        printf( "outer tess. factor = %.1f\n", outer_tess_fac );
        s_was_down = true;
        glUniform1f( outer_tess_fac_loc, outer_tess_fac );
      }
    } else {
      s_was_down = false;
    }
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers( window );
  }

  // close GL context and any other GLFW resources
  glfwTerminate();

  return 0;
}
