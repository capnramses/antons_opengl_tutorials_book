/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries separate legal notices                              |
|******************************************************************************|
| Particle Systems                                                             |
\******************************************************************************/

#include "gl_utils.h"
#include "maths_funcs.h"
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

#define PARTICLE_COUNT 300

/* create initial attribute values for particles. return a VAO */
GLuint gen_particles() {
  float vv[PARTICLE_COUNT * 3]; // start velocities vec3
  float vt[PARTICLE_COUNT];     // start times
  float t_accum = 0.0f;         // start time
  int j         = 0;
  for ( int i = 0; i < PARTICLE_COUNT; i++ ) {
    // start times
    vt[i] = t_accum;
    t_accum += 0.01f; // spacing for start times is 0.01 seconds
    // start velocities. randomly vary x and z components
    float randx = ( (float)rand() / (float)RAND_MAX ) * 1.0f - 0.5f;
    float randz = ( (float)rand() / (float)RAND_MAX ) * 1.0f - 0.5f;
    vv[j]       = randx; // x
    vv[j + 1]   = 1.0f;  // y
    vv[j + 2]   = randz; // z
    j += 3;
  }

  GLuint velocity_vbo;
  glGenBuffers( 1, &velocity_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, velocity_vbo );
  glBufferData( GL_ARRAY_BUFFER, sizeof( vv ), vv, GL_STATIC_DRAW );

  GLuint time_vbo;
  glGenBuffers( 1, &time_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, time_vbo );
  glBufferData( GL_ARRAY_BUFFER, sizeof( vt ), vt, GL_STATIC_DRAW );

  GLuint vao;
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );
  glBindBuffer( GL_ARRAY_BUFFER, velocity_vbo );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glBindBuffer( GL_ARRAY_BUFFER, time_vbo );
  glVertexAttribPointer( 1, 1, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );
  glEnableVertexAttribArray( 1 );

  return vao;
}

int main() {
  restart_gl_log();
  // use GLFW and GLEW to start GL context. see gl_utils.cpp for details
  start_gl();

  /* create buffer of particle initial attributes and a VAO */
  GLuint vao = gen_particles();

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

  /* make up a world position for the emitter */
  vec3 emitter_world_pos( 0.0f, 0.0f, 0.0f );

  // locations of view and projection matrices
  int V_loc = glGetUniformLocation( shader_programme, "V" );
  assert( V_loc > -1 );
  int P_loc = glGetUniformLocation( shader_programme, "P" );
  assert( P_loc > -1 );
  int emitter_pos_wor_loc = glGetUniformLocation( shader_programme, "emitter_pos_wor" );
  assert( emitter_pos_wor_loc > -1 );
  int elapsed_system_time_loc = glGetUniformLocation( shader_programme, "elapsed_system_time" );
  assert( elapsed_system_time_loc > -1 );
  glUseProgram( shader_programme );
  glUniformMatrix4fv( V_loc, 1, GL_FALSE, view_mat.m );
  glUniformMatrix4fv( P_loc, 1, GL_FALSE, proj_mat );
  glUniform3f( emitter_pos_wor_loc, emitter_world_pos.v[0], emitter_world_pos.v[1], emitter_world_pos.v[2] );

  // load texture
  GLuint tex;
  if ( !load_texture( "Droplet.png", &tex ) ) {
    gl_log_err( "ERROR: loading Droplet.png texture\n" );
    return 1;
  }

  glEnable( GL_CULL_FACE );  // cull face
  glCullFace( GL_BACK );     // cull back face
  glFrontFace( GL_CCW );     // GL_CCW for counter clock-wise
  glDepthFunc( GL_LESS );    // depth-testing interprets a smaller value as "closer"
  glEnable( GL_DEPTH_TEST ); // enable depth-testing
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  glClearColor( 0.2, 0.2, 0.2, 1.0 );
  /* MUST use this is in compatibility profile. doesn't exist in core
  glEnable(GL_POINT_SPRITE);
  */

  while ( !glfwWindowShouldClose( g_window ) ) {
    static double previous_seconds = glfwGetTime();
    double current_seconds         = glfwGetTime();
    double elapsed_seconds         = current_seconds - previous_seconds;
    previous_seconds               = current_seconds;

    _update_fps_counter( g_window );
    // wipe the drawing surface clear
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, g_gl_width, g_gl_height );

    /* Render Particles. Enabling point re-sizing in vertex shader */
    glEnable( GL_PROGRAM_POINT_SIZE );
    glPointParameteri( GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT );

    glEnable( GL_BLEND );
    glDepthMask( GL_FALSE );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, tex );
    glUseProgram( shader_programme );

    /* update time in shaders */
    glUniform1f( elapsed_system_time_loc, (GLfloat)current_seconds );

    glBindVertexArray( vao );
    // draw points 0-3 from the currently bound VAO with current in-use shader
    glDrawArrays( GL_POINTS, 0, PARTICLE_COUNT );
    glDisable( GL_BLEND );
    glDepthMask( GL_TRUE );
    glDisable( GL_PROGRAM_POINT_SIZE );

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
      glUniformMatrix4fv( V_loc, 1, GL_FALSE, view_mat.m );
    }

    if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) { glfwSetWindowShouldClose( g_window, 1 ); }
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers( g_window );
  }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
