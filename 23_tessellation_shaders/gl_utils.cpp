#include "gl_utils.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

bool restart_gl_log() {
  FILE* file = fopen( GL_LOG_FILE, "w" );
  if ( !file ) {
    fprintf( stderr, "ERROR: could not open GL_LOG_FILE log file %s for writing\n", GL_LOG_FILE );
    return false;
  }
  time_t now = time( NULL );
  char* date = ctime( &now );
  fprintf( file, "GL_LOG_FILE log. local time %s\n", date );
  fclose( file );
  return true;
}

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

void glfw_error_callback( int error, const char* description ) {
  fputs( description, stderr );
  gl_log_err( "%s\n", description );
}

// a call-back function
void glfw_framebuffer_size_callback( GLFWwindow* window, int width, int height ) {
  g_gl_width  = width;
  g_gl_height = height;
  printf( "width %i height %i\n", width, height );
  /* update any perspective matrices used here */
}

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
  gl_log( "%s %u\n", names[11], (unsigned int)s );
  gl_log( "-----------------------------\n" );
}

void _update_fps_counter( GLFWwindow* window ) {
  static double previous_seconds = glfwGetTime();
  static int frame_count;
  double current_seconds = glfwGetTime();
  double elapsed_seconds = current_seconds - previous_seconds;
  if ( elapsed_seconds > 0.25 ) {
    previous_seconds = current_seconds;
    double fps       = (double)frame_count / elapsed_seconds;
    char tmp[128];
    sprintf( tmp, "opengl @ fps: %.2f", fps );
    glfwSetWindowTitle( window, tmp );
    frame_count = 0;
  }
  frame_count++;
}

void _print_shader_info_log( GLuint shader_index ) {
  int max_length    = 2048;
  int actual_length = 0;
  char log[2048];
  glGetShaderInfoLog( shader_index, max_length, &actual_length, log );
  printf( "shader info log for GL index %i:\n%s\n", shader_index, log );
}

void _print_programme_info_log( GLuint sp ) {
  int max_length    = 2048;
  int actual_length = 0;
  char log[2048];
  glGetProgramInfoLog( sp, max_length, &actual_length, log );
  printf( "program info log for GL index %i:\n%s", sp, log );
}

bool is_valid( GLuint sp ) {
  glValidateProgram( sp );
  int params = -1;
  glGetProgramiv( sp, GL_VALIDATE_STATUS, &params );
  if ( GL_TRUE != params ) {
    printf( "program %i GL_VALIDATE_STATUS = GL_FALSE\n", sp );
    _print_programme_info_log( sp );
    return false;
  }
  printf( "program %i GL_VALIDATE_STATUS = GL_TRUE\n", sp );
  return true;
}

bool parse_file_into_str( const char* file_name, char* shader_str, int max_len ) {
  shader_str[0] = '\0'; // reset string
  FILE* file    = fopen( file_name, "r" );
  if ( !file ) {
    gl_log_err( "ERROR: opening file for reading: %s\n", file_name );
    return false;
  }
  int current_len = 0;
  char line[2048];
  strcpy( line, "" ); // remember to clean up before using for first time!
  while ( !feof( file ) ) {
    if ( NULL != fgets( line, 2048, file ) ) {
      current_len += strlen( line ); // +1 for \n at end
      if ( current_len >= max_len ) { gl_log_err( "ERROR: shader length is longer than string buffer length %i\n", max_len ); }
      strcat( shader_str, line );
    }
  }
  if ( EOF == fclose( file ) ) { // probably unnecesssary validation
    gl_log_err( "ERROR: closing file from reading %s\n", file_name );
    return false;
  }
  return true;
}
