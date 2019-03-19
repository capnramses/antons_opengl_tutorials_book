// Supplementary demo code for Anton's OpenGL 4 Tutorials
// Copyright Anton Gerdelan <antongdl@protonmail.com> 2019
// Article: http://antongerdelan.net/opengl/shader_hot_reload.html
// This code is C99.
// To compile on Windows with gcc:
// gcc -std=c99 -Wall -DGLEW_STATIC .\main.c ..\common\GL\glew.c  ..\common\win64_gcc\libglfw3.a -I ..\common\ -I ..\common\include -lOpenGL32 -lgdi32 -lws2_32

#include <GL/glew.h>    // or use another OpenGL header/function pointer wrangler eg glad
#include <GLFW/glfw3.h> // or use another OpenGL context/window creator eg SDL2
#include <stdbool.h>    // only required for C99
#include <assert.h>
#include <stdio.h>
#include <string.h>

// OpenGL context+window using GLFW
#define WINDOW_TITLE "shader hot reload demo"
GLFWwindow* window;
// set a max limit on shader length to avoid dynamic memory allocation
#define MAX_SHADER_SZ 100000

// geometry to display
GLuint triangle_vao;   // mesh/attribute descriptor handle
GLuint triangle_vbo;   // handle to OpenGL copy of buffer
GLuint shader_program; // our shader program that we will reload

// starts OpenGL context + window. returns false on error
bool start_opengl() {
  { // start context + window with GLFW
    if ( !glfwInit() ) {
      fprintf( stderr, "ERROR: could not start GLFW3\n" );
      return false;
    }
    // OpenGL 4.1 is now ~ the safest default version for compatibility across all platforms
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    window = glfwCreateWindow( 800, 600, WINDOW_TITLE, NULL, NULL );
    if ( !window ) {
      fprintf( stderr, "ERROR: could not open window with GLFW3\n" );
      glfwTerminate();
      return false;
    }
    glfwMakeContextCurrent( window );
  }
  { // hook up OpenGL function pointers with GLEW
    glewExperimental = GL_TRUE;
    GLenum err       = glewInit();
    if ( GLEW_OK != err ) {
      fprintf( stderr, "ERROR: %s\n", glewGetErrorString( err ) );
      return false;
    }
    printf( "Status: Using GLEW %s\n", glewGetString( GLEW_VERSION ) );
  }
  { // print which version of OpenGL started
    const GLubyte* renderer = glGetString( GL_RENDERER );
    const GLubyte* version  = glGetString( GL_VERSION );
    printf( "Renderer: %s\n", renderer );
    printf( "OpenGL version supported %s\n", version );
  }
  return true;
}

// stops OpenGL context + window. call any time you exit the program, after start_opengl() was called, to tidy up
void stop_opengl() {
  glfwTerminate();
  printf( "GLFW window closed\n" );
}

void create_geometry() {
  float points[] = { 0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f };
  glGenVertexArrays( 1, &triangle_vao );
  glGenBuffers( 1, &triangle_vbo );
  glBindVertexArray( triangle_vao );
  glBindBuffer( GL_ARRAY_BUFFER, triangle_vbo );
  {
    glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( float ), points, GL_STATIC_DRAW ); // copy points into bound OpenGL buffer
    glEnableVertexAttribArray( 0 );                                               // enable input variables from bound VAO for shaders
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );                   // describe buffer's data layout. stored in VAO
  }
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );
}

// function creates a shader program from a vertex and fragment shader, stored in provided strings
// vertex_shader_str - a null-terminated string of text containing a vertex shader
// fragment_shader_str - a null-terminated string of text containing a fragment shader
// returns a new, valid shader program handle, or 0 if there was a problem finding/loading/compiling/linking
// asserts on NULL parameters
GLuint create_shader_program_from_strings( const char* vertex_shader_str, const char* fragment_shader_str ) {
  assert( vertex_shader_str && fragment_shader_str );

  GLuint shader_program         = glCreateProgram();
  GLuint vertex_shader_handle   = glCreateShader( GL_VERTEX_SHADER );
  GLuint fragment_shader_handle = glCreateShader( GL_FRAGMENT_SHADER );
  { // compile shader and check for errors
    glShaderSource( vertex_shader_handle, 1, &vertex_shader_str, NULL );
    glCompileShader( vertex_shader_handle );
    int params = -1;
    glGetShaderiv( vertex_shader_handle, GL_COMPILE_STATUS, &params );
    if ( GL_TRUE != params ) {
      fprintf( stderr, "ERROR: vertex shader index %u did not compile\n", vertex_shader_handle );
      const int max_length = 2048;
      int actual_length    = 0;
      char slog[2048];
      glGetShaderInfoLog( vertex_shader_handle, max_length, &actual_length, slog );
      fprintf( stderr, "shader info log for GL index %u:\n%s\n", vertex_shader_handle, slog );

      glDeleteShader( vertex_shader_handle );
      glDeleteShader( fragment_shader_handle );
      glDeleteProgram( shader_program );
      return 0;
    }
  }

  { // compile shader and check for errors
    glShaderSource( fragment_shader_handle, 1, &fragment_shader_str, NULL );
    glCompileShader( fragment_shader_handle );
    int params = -1;
    glGetShaderiv( fragment_shader_handle, GL_COMPILE_STATUS, &params );
    if ( GL_TRUE != params ) {
      fprintf( stderr, "ERROR: fragment shader index %u did not compile\n", fragment_shader_handle );
      const int max_length = 2048;
      int actual_length    = 0;
      char slog[2048];
      glGetShaderInfoLog( fragment_shader_handle, max_length, &actual_length, slog );
      fprintf( stderr, "shader info log for GL index %u:\n%s\n", fragment_shader_handle, slog );

      glDeleteShader( vertex_shader_handle );
      glDeleteShader( fragment_shader_handle );
      glDeleteProgram( shader_program );
      return 0;
    }
  }

  glAttachShader( shader_program, fragment_shader_handle );
  glAttachShader( shader_program, vertex_shader_handle );
  { // link program and check for errors
    glLinkProgram( shader_program );
    glDeleteShader( vertex_shader_handle );
    glDeleteShader( fragment_shader_handle );
    int params = -1;
    glGetProgramiv( shader_program, GL_LINK_STATUS, &params );
    if ( GL_TRUE != params ) {
      fprintf( stderr, "ERROR: could not link shader program GL index %u\n", shader_program );
      const int max_length = 2048;
      int actual_length    = 0;
      char plog[2048];
      glGetProgramInfoLog( shader_program, max_length, &actual_length, plog );
      fprintf( stderr, "program info log for GL index %u:\n%s", shader_program, plog );

      glDeleteProgram( shader_program );
      return 0;
    }
  }

  return shader_program;
}

GLuint create_shader_program_from_files( const char* vertex_shader_filename, const char* fragment_shader_filename ) {
  assert( vertex_shader_filename && fragment_shader_filename );

  printf( "loading shader from files `%s` and `%s`\n", vertex_shader_filename, fragment_shader_filename );

  char vs_shader_str[MAX_SHADER_SZ];
  char fs_shader_str[MAX_SHADER_SZ];
  vs_shader_str[0] = fs_shader_str[0] = '\0';
  {
    FILE* fp = fopen( vertex_shader_filename, "r" );
    if ( !fp ) {
      fprintf( stderr, "ERROR: could not open vertex shader file `%s`\n", vertex_shader_filename );
      return 0;
    }
    char line[1024];
    line[0]                = '\0';
    int num_bytes_appended = 0;
    while ( fgets( line, 1024, fp ) ) {
      int len = strlen( line );
      if ( ( len + num_bytes_appended ) > ( MAX_SHADER_SZ - 1 ) ) {
        fprintf( stderr, "ERROR: shader is too long for buffer\n" );
        return 0;
      }
      strncat( vs_shader_str, line, MAX_SHADER_SZ - num_bytes_appended - 1 );
      num_bytes_appended += len;
    }
    fclose( fp );
  }
  {
    FILE* fp = fopen( fragment_shader_filename, "r" );
    if ( !fp ) {
      fprintf( stderr, "ERROR: could not open fragment shader file `%s`\n", fragment_shader_filename );
      return 0;
    }
    char line[1024];
    line[0]                = '\0';
    int num_bytes_appended = 0;
    while ( fgets( line, 1024, fp ) ) {
      int len = strlen( line );
      if ( ( len + num_bytes_appended ) > ( MAX_SHADER_SZ - 1 ) ) {
        fprintf( stderr, "ERROR: shader is too long for buffer\n" );
        return 0;
      }
      strncat( fs_shader_str, line, MAX_SHADER_SZ - num_bytes_appended - 1 );
      num_bytes_appended += len;
    }
    fclose( fp );
  }

  return create_shader_program_from_strings( vs_shader_str, fs_shader_str );
}

void reload_shader_program_from_files( GLuint* program, const char* vertex_shader_filename, const char* fragment_shader_filename ) {
  assert( program && vertex_shader_filename && fragment_shader_filename );

  GLuint reloaded_program = create_shader_program_from_files( vertex_shader_filename, fragment_shader_filename );

  if ( reloaded_program ) {
    glDeleteProgram( *program );
    *program = reloaded_program;
  }
}

int main() {
  if ( !start_opengl() ) { return 1; }

  create_geometry();

  shader_program = create_shader_program_from_files( "myshader.vert", "myshader.frag" );
  if ( !shader_program ) {
    stop_opengl();
    return 1;
  }

  glEnable( GL_DEPTH_TEST );
  glDepthFunc( GL_LESS );
  // drawing loop
  while ( !glfwWindowShouldClose( window ) ) {
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // wipe the drawing surface clear

    { // draw our triangle
      glUseProgram( shader_program );
      glBindVertexArray( triangle_vao );

      glDrawArrays( GL_TRIANGLES, 0, 3 );

      glBindVertexArray( 0 );
      glUseProgram( 0 );
    }

    glfwPollEvents();          // update other events like input handling. Wait events only updates on demand. Poll events updates as fast as possible frame.
    glfwSwapBuffers( window ); // put the stuff we've been drawing onto the display

    if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) ) { glfwSetWindowShouldClose( window, 1 ); }

    static bool reload_key_pressed = false;
    bool down                      = glfwGetKey( window, GLFW_KEY_R );
    if ( down && !reload_key_pressed ) {
      reload_key_pressed = true;
    } else if ( !down && reload_key_pressed ) {
      reload_key_pressed = false;
      reload_shader_program_from_files( &shader_program, "myshader.vert", "myshader.frag" );
    }
  } // endwhile

  stop_opengl();
  return 0;
}
