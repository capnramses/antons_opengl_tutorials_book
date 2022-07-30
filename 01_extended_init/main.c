/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                          |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| Extended Initialisation. Some extra detail.                                  |
\******************************************************************************/
#include <GL/glew.h>    // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <stdarg.h> // for doing gl_log() functions that work like printf()
#include <stdio.h>
#include <time.h>

#if 1
#   include <stdbool.h> /* for visual studio i had to comment this out and define pure-C bool :( */
#else
#   define bool int
#   define true 1
#   define false 0
#endif
#define GL_LOG_FILE "gl.log"

// global to use in timer code later
static double previous_seconds;

/* start a new log file. put the time and date at the top */
bool restart_gl_log() {
  time_t now;
  char* date;
  FILE* file = fopen( GL_LOG_FILE, "w" );

  if ( !file ) {
    fprintf( stderr, "ERROR: could not open GL_LOG_FILE log file %s for writing\n", GL_LOG_FILE );
    return false;
  }
  now  = time( NULL );
  date = ctime( &now );
  fprintf( file, "GL_LOG_FILE log. local time %s", date );
  fprintf( file, "build version: %s %s\n\n", __DATE__, __TIME__ );
  fclose( file );
  return true;
}

/* add a message to the log file. arguments work the same way as printf() */
bool gl_log( const char* message, ... ) {
  va_list argptr;
  FILE* file = fopen( GL_LOG_FILE, "a" );
  if ( !file ) {
    fprintf( stderr, "ERROR: could not open GL_LOG_FILE %s file for appending\n", GL_LOG_FILE );
    return false;
  }
  va_start( argptr, message );
  vfprintf( file, message, argptr );
  va_end( argptr );
  fclose( file );
  return true;
}

/* same as gl_log except also prints to stderr */
bool gl_log_err( const char* message, ... ) {
  va_list argptr;
  FILE* file = fopen( GL_LOG_FILE, "a" );
  if ( !file ) {
    fprintf( stderr, "ERROR: could not open GL_LOG_FILE %s file for appending\n", GL_LOG_FILE );
    return false;
  }
  va_start( argptr, message );
  vfprintf( file, message, argptr );
  va_end( argptr );
  va_start( argptr, message );
  vfprintf( stderr, message, argptr );
  va_end( argptr );
  fclose( file );
  return true;
}

/* we will tell GLFW to run this function whenever it finds an error */
void glfw_error_callback( int error, const char* description ) { gl_log_err( "GLFW ERROR: code %i msg: %s\n", error, description ); }

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width  = 640;
int g_gl_height = 480;

/* we will tell GLFW to run this function whenever the framebuffer size is changed */
void glfw_framebuffer_size_callback( GLFWwindow* window, int width, int height ) {
  g_gl_width  = width;
  g_gl_height = height;
  printf( "width %i height %i\n", width, height );
  /* update any perspective matrices used here */
}

/* we can use a function like this to print some GL capabilities of our adapter
to the log file. handy if we want to debug problems on other people's computers
*/
void log_gl_params() {
  GLenum params[] = {
    GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
    GL_MAX_CUBE_MAP_TEXTURE_SIZE,
    GL_MAX_DRAW_BUFFERS,
    GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
    GL_MAX_TEXTURE_IMAGE_UNITS,
    GL_MAX_TEXTURE_SIZE,
    GL_MAX_VARYING_FLOATS,
    GL_MAX_VERTEX_ATTRIBS,
    GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
    GL_MAX_VERTEX_UNIFORM_COMPONENTS,
    GL_MAX_VIEWPORT_DIMS,
    GL_STEREO,
  };
  const char* names[] = {
    "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
    "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
    "GL_MAX_DRAW_BUFFERS",
    "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
    "GL_MAX_TEXTURE_IMAGE_UNITS",
    "GL_MAX_TEXTURE_SIZE",
    "GL_MAX_VARYING_FLOATS",
    "GL_MAX_VERTEX_ATTRIBS",
    "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
    "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
    "GL_MAX_VIEWPORT_DIMS",
    "GL_STEREO",
  };
  gl_log( "GL Context Params:\n" );
  // integers - only works if the order is 0-10 integer return types
  for ( int i = 0; i < 10; i++ ) {
    int v = 0;
    glGetIntegerv( params[i], &v );
    gl_log( "%s %i\n", names[i], v );
  }
  // others
  int v[2];
  v[0] = v[1] = 0;
  glGetIntegerv( params[10], v );
  gl_log( "%s %i %i\n", names[10], v[0], v[1] );
  unsigned char s = 0;
  glGetBooleanv( params[11], &s );
  gl_log( "%s %i\n", names[11], (unsigned int)s );
  gl_log( "-----------------------------\n" );
}

/* we will use this function to update the window title with a frame rate */
void _update_fps_counter( GLFWwindow* window ) {
  char tmp[128];

  static int frame_count;

  double current_seconds = glfwGetTime();
  double elapsed_seconds = current_seconds - previous_seconds;
  if ( elapsed_seconds > 0.25 ) {
    previous_seconds = current_seconds;

    double fps = (double)frame_count / elapsed_seconds;
    sprintf( tmp, "opengl @ fps: %.2f", fps );
    glfwSetWindowTitle( window, tmp );
    frame_count = 0;
  }
  frame_count++;
}

int main() {
  GLFWwindow* window;
  const GLubyte* renderer;
  const GLubyte* version;
  GLfloat points[] = { 0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f };
  GLuint vbo;
  GLuint vao;
  const char* vertex_shader =
    "#version 410\n"
    "in vec3 vp;"
    "void main() {"
    "  gl_Position = vec4( vp, 1.0 );"
    "}";

  const char* fragment_shader =
    "#version 410\n"
    "out vec4 frag_colour;"
    "void main() {"
    "  frag_colour = vec4( 0.5, 0.0, 0.5, 1.0 );"
    "}";
  GLuint shader_programme, vs, fs;

  restart_gl_log();
  // start GL context and O/S window using the GLFW helper library
  gl_log( "starting GLFW\n%s\n", glfwGetVersionString() );
  // register the error call-back function that we wrote, above
  glfwSetErrorCallback( glfw_error_callback );
  if ( !glfwInit() ) {
    fprintf( stderr, "ERROR: could not start GLFW3\n" );
    return 1;
  }

/* We must specify 3.2 core if on Apple OS X -- other O/S can specify
 anything here. I defined 'APPLE' in the makefile for OS X */
#ifdef APPLE
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
#endif
  // set anti-aliasing factor to make diagonal edges appear less jagged
  glfwWindowHint( GLFW_SAMPLES, 4 );

  /* we can run a full-screen window here */

  /*GLFWmonitor* mon = glfwGetPrimaryMonitor ();
  const GLFWvidmode* vmode = glfwGetVideoMode (mon);
  GLFWwindow* window = glfwCreateWindow (
    vmode->width, vmode->height, "Extended GL Init", mon, NULL
  );*/

  window = glfwCreateWindow( g_gl_width, g_gl_height, "Extended Init.", NULL, NULL );
  if ( !window ) {
    fprintf( stderr, "ERROR: could not open window with GLFW3\n" );
    glfwTerminate();
    return 1;
  }
  glfwSetFramebufferSizeCallback( window, glfw_framebuffer_size_callback );
  //
  glfwMakeContextCurrent( window );

  // start GLEW extension handler
  glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  renderer = glGetString( GL_RENDERER ); // get renderer string
  version  = glGetString( GL_VERSION );  // version as a string
  printf( "Renderer: %s\n", renderer );
  printf( "OpenGL version supported %s\n", version );
  gl_log( "renderer: %s\nversion: %s\n", renderer, version );
  log_gl_params();
  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable( GL_DEPTH_TEST ); // enable depth-testing
  glDepthFunc( GL_LESS );    // depth-testing interprets a smaller value as "closer"

  glGenBuffers( 1, &vbo );
  glBindBuffer( GL_ARRAY_BUFFER, vbo );
  glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( GLfloat ), points, GL_STATIC_DRAW );

  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );
  glEnableVertexAttribArray( 0 );
  glBindBuffer( GL_ARRAY_BUFFER, vbo );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );

  vs = glCreateShader( GL_VERTEX_SHADER );
  glShaderSource( vs, 1, &vertex_shader, NULL );
  glCompileShader( vs );
  fs = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( fs, 1, &fragment_shader, NULL );
  glCompileShader( fs );
  shader_programme = glCreateProgram();
  glAttachShader( shader_programme, fs );
  glAttachShader( shader_programme, vs );
  glLinkProgram( shader_programme );

  previous_seconds = glfwGetTime();
  while ( !glfwWindowShouldClose( window ) ) {
    _update_fps_counter( window );
    // wipe the drawing surface clear
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, g_gl_width, g_gl_height );

    glUseProgram( shader_programme );
    glBindVertexArray( vao );
    // draw points 0-3 from the currently bound VAO with current in-use shader
    glDrawArrays( GL_TRIANGLES, 0, 3 );
    // update other events like input handling
    glfwPollEvents();
    if ( GLFW_PRESS == glfwGetKey( window, GLFW_KEY_ESCAPE ) ) { glfwSetWindowShouldClose( window, 1 ); }
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers( window );
  }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
