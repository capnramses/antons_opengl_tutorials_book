/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries separate legal notices                              |
|******************************************************************************|
| Texture Mapping                                                              |
| * I used Sean Barrett's stb_image library to load an image file into memory  |
| * I made a load_texture() function to copy this into a GL texture            |
\******************************************************************************/

#include "gl_utils.h"
#include "maths_funcs.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"  // Sean Barrett's image loader - http://nothings.org/
#include <GL/glew.h>    // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define GL_LOG_FILE "gl.log"

int g_gl_width       = 640;
int g_gl_height      = 480;
GLFWwindow* g_window = NULL;

bool load_texture( const char* file_name, GLuint* tex ) {
  int x, y, n;
  int force_channels = 4;
  // the following function call flips the image
  // needs to be called before each stbi_load(...);
  stbi_set_flip_vertically_on_load( true );
  unsigned char* image_data = stbi_load( file_name, &x, &y, &n, force_channels );
  if ( !image_data ) {
    fprintf( stderr, "ERROR: could not load %s\n", file_name );
    return false;
  }
  // NPOT check
  if ( ( x & ( x - 1 ) ) != 0 || ( y & ( y - 1 ) ) != 0 ) { fprintf( stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name ); }

  glGenTextures( 1, tex );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, *tex );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data );
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
  restart_gl_log();
  // use GLFW and GLEW to start GL context. see gl_utils.cpp for details
  start_gl();

  // tell GL to only draw onto a pixel if the shape is closer to the viewer
  glEnable( GL_DEPTH_TEST ); // enable depth-testing
  glDepthFunc( GL_LESS );    // depth-testing interprets a smaller value as "closer"

  /* OTHER STUFF GOES HERE NEXT */
  GLfloat points[] = { -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.5f, 0.5f, 0.0f, 0.5f, 0.5f, 0.0f, -0.5f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f };

  // 2^16 = 65536
  GLfloat texcoords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f };

  GLuint points_vbo;
  glGenBuffers( 1, &points_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  glBufferData( GL_ARRAY_BUFFER, 18 * sizeof( GLfloat ), points, GL_STATIC_DRAW );

  GLuint texcoords_vbo;
  glGenBuffers( 1, &texcoords_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, texcoords_vbo );
  glBufferData( GL_ARRAY_BUFFER, 12 * sizeof( GLfloat ), texcoords, GL_STATIC_DRAW );

  GLuint vao;
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glBindBuffer( GL_ARRAY_BUFFER, texcoords_vbo );
  glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL ); // normalise!
  glEnableVertexAttribArray( 0 );
  glEnableVertexAttribArray( 1 );

  GLuint shader_programme = create_programme_from_files( "test_vs.glsl", "test_fs.glsl" );

#define ONE_DEG_IN_RAD ( 2.0 * M_PI ) / 360.0 // 0.017444444
  // input variables
  float near   = 0.1f;                                   // clipping plane
  float far    = 100.0f;                                 // clipping plane
  float fov    = 67.0f * ONE_DEG_IN_RAD;                 // convert 67 degrees to radians
  float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
  // matrix components
  float inverse_range = 1.0f / tan( fov * 0.5f );
  float Sx            = inverse_range / aspect;
  float Sy            = inverse_range;
  float Sz            = -( far + near ) / ( far - near );
  float Pz            = -( 2.0f * far * near ) / ( far - near );
  GLfloat proj_mat[]  = { Sx, 0.0f, 0.0f, 0.0f, 0.0f, Sy, 0.0f, 0.0f, 0.0f, 0.0f, Sz, -1.0f, 0.0f, 0.0f, Pz, 0.0f };

  float cam_speed     = 1.0f;                 // 1 unit per second
  float cam_yaw_speed = 10.0f;                // 10 degrees per second
  float cam_pos[]     = { 0.0f, 0.0f, 2.0f }; // don't start at zero, or we will be too close
  float cam_yaw       = 0.0f;                 // y-rotation in degrees
  mat4 T              = translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1], -cam_pos[2] ) );
  mat4 R              = rotate_y_deg( identity_mat4(), -cam_yaw );
  mat4 view_mat       = R * T;

  int view_mat_location = glGetUniformLocation( shader_programme, "view" );
  glUseProgram( shader_programme );
  glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
  int proj_mat_location = glGetUniformLocation( shader_programme, "proj" );
  glUseProgram( shader_programme );
  glUniformMatrix4fv( proj_mat_location, 1, GL_FALSE, proj_mat );

  // load texture
  GLuint tex;
  ( load_texture( "skulluvmap.png", &tex ) );

  glEnable( GL_CULL_FACE ); // cull face
  glCullFace( GL_BACK );    // cull back face
  glFrontFace( GL_CCW );    // GL_CCW for counter clock-wise

  while ( !glfwWindowShouldClose( g_window ) ) {
    static double previous_seconds = glfwGetTime();
    double current_seconds         = glfwGetTime();
    double elapsed_seconds         = current_seconds - previous_seconds;
    previous_seconds               = current_seconds;

    _update_fps_counter( g_window );
    // wipe the drawing surface clear
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, g_gl_width, g_gl_height );

    glUseProgram( shader_programme );
    glBindVertexArray( vao );
    // draw points 0-3 from the currently bound VAO with current in-use shader
    glDrawArrays( GL_TRIANGLES, 0, 6 );
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
      mat4 T        = translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1],
                                             -cam_pos[2] ) ); // cam translation
      mat4 R        = rotate_y_deg( identity_mat4(), -cam_yaw );     //
      mat4 view_mat = R * T;
      glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
    }

    if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) { glfwSetWindowShouldClose( g_window, 1 ); }
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers( g_window );
  }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
