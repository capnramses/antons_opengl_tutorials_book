/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries' for separate legal notices                         |
|******************************************************************************|
| Virtual Camera with Quaternions.                                             |
| quaternion code is at the top of this file. i made these using only arrays   |
| of floats so that it was maths-library independent. you might prefer to      |
| write a quaternion class. i have one in my maths library - have a look at    |
| the code there.                                                              |
|                                                                              |
| controls:                                                                    |
| pitch = up,down arrow keys                                                   |
| yaw = left,right arrow keys                                                  |
| roll = z,c keys                                                              |
| move forward/back = w,s keys                                                 |
| move left/right = a,d keys                                                   |
|                                                                              |
| I wrote a little Wavefront .obj loader to load a mesh from a file            |
| It's in obj_parser.h and .cpp                                                |
\******************************************************************************/
#include "gl_utils.h"    // common opengl functions and small utilities like logs
#include "maths_funcs.h" // my maths functions
#include "obj_parser.h"  // my little Wavefront .obj mesh loader
#include <GL/glew.h>     // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h>  // GLFW helper library
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define MESH_FILE "sphere.obj"
#define VERTEX_SHADER_FILE "test_vs.glsl"
#define FRAGMENT_SHADER_FILE "test_fs.glsl"
#define NUM_SPHERES 4

/* create a unit quaternion q from an angle in degrees a, and an axis x,y,z */
void create_versor( float* q, float degrees, float x, float y, float z ) {
  float rad = ONE_DEG_IN_RAD * degrees;
  q[0]      = cosf( rad / 2.0f );
  q[1]      = sinf( rad / 2.0f ) * x;
  q[2]      = sinf( rad / 2.0f ) * y;
  q[3]      = sinf( rad / 2.0f ) * z;
}

/* convert a unit quaternion q to a 4x4 matrix m */
void quat_to_mat4( float* m, const float* q ) {
  float w = q[0];
  float x = q[1];
  float y = q[2];
  float z = q[3];
  m[0]    = 1.0f - 2.0f * y * y - 2.0f * z * z;
  m[1]    = 2.0f * x * y + 2.0f * w * z;
  m[2]    = 2.0f * x * z - 2.0f * w * y;
  m[3]    = 0.0f;
  m[4]    = 2.0f * x * y - 2.0f * w * z;
  m[5]    = 1.0f - 2.0f * x * x - 2.0f * z * z;
  m[6]    = 2.0f * y * z + 2.0f * w * x;
  m[7]    = 0.0f;
  m[8]    = 2.0f * x * z + 2.0f * w * y;
  m[9]    = 2.0f * y * z - 2.0f * w * x;
  m[10]   = 1.0f - 2.0f * x * x - 2.0f * y * y;
  m[11]   = 0.0f;
  m[12]   = 0.0f;
  m[13]   = 0.0f;
  m[14]   = 0.0f;
  m[15]   = 1.0f;
}

/* normalise a quaternion in case it got a bit mangled */
void normalise_quat( float* q ) {
  // norm(q) = q / magnitude (q)
  // magnitude (q) = sqrt (w*w + x*x...)
  // only compute sqrt if interior sum != 1.0
  float sum = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];
  // NB: floats have min 6 digits of precision
  const float thresh = 0.0001f;
  if ( fabs( 1.0f - sum ) < thresh ) { return; }
  float mag = sqrt( sum );
  for ( int i = 0; i < 4; i++ ) { q[i] = q[i] / mag; }
}

/* multiply quaternions to get another one. result=R*S */
void mult_quat_quat( float* result, const float* r, const float* s ) {
  float w   = s[0] * r[0] - s[1] * r[1] - s[2] * r[2] - s[3] * r[3];
  float x   = s[0] * r[1] + s[1] * r[0] - s[2] * r[3] + s[3] * r[2];
  float y   = s[0] * r[2] + s[1] * r[3] + s[2] * r[0] - s[3] * r[1];
  float z   = s[0] * r[3] - s[1] * r[2] + s[2] * r[1] + s[3] * r[0];
  result[0] = w;
  result[1] = x;
  result[2] = y;
  result[3] = z;
  // re-normalise in case of mangling
  normalise_quat( result );
}

// camera matrices. it's easier if they are global
mat4 view_mat;
mat4 proj_mat;
vec3 cam_pos( 0.0f, 0.0f, 5.0f );
// a world position for each sphere in the scene
vec3 sphere_pos_wor[] = { vec3( -2.0, 0.0, 0.0 ), vec3( 2.0, 0.0, 0.0 ), vec3( -2.0, 0.0, -2.0 ), vec3( 1.5, 1.0, -1.0 ) };

int main() {
  /*--------------------------------START OPENGL--------------------------------*/
  restart_gl_log();
  // start GL context and O/S window using the GLFW helper library
  start_gl();
  /*------------------------------CREATE GEOMETRY-------------------------------*/
  GLfloat* vp     = NULL; // array of vertex points
  GLfloat* vn     = NULL; // array of vertex normals
  GLfloat* vt     = NULL; // array of texture coordinates
  int point_count = 0;
  load_obj_file( MESH_FILE, vp, vt, vn, point_count );

  GLuint vao;
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );

  GLuint points_vbo;
  if ( NULL != vp ) {
    glGenBuffers( 1, &points_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
    glBufferData( GL_ARRAY_BUFFER, 3 * point_count * sizeof( GLfloat ), vp, GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
    glEnableVertexAttribArray( 0 );
  }

  /*-------------------------------CREATE SHADERS-------------------------------*/
  GLuint shader_programme = create_programme_from_files( VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE );
  int model_mat_location  = glGetUniformLocation( shader_programme, "model" );
  int view_mat_location   = glGetUniformLocation( shader_programme, "view" );
  int proj_mat_location   = glGetUniformLocation( shader_programme, "proj" );

/*-------------------------------CREATE CAMERA--------------------------------*/
#define ONE_DEG_IN_RAD ( 2.0 * M_PI ) / 360.0 // 0.017444444
  // input variables
  float near   = 0.1f;                                   // clipping plane
  float far    = 100.0f;                                 // clipping plane
  float fovy   = 67.0f;                                  // 67 degrees
  float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
  proj_mat     = perspective( fovy, aspect, near, far );

  float cam_speed         = 5.0f;   // 1 unit per second
  float cam_heading_speed = 100.0f; // 30 degrees per second
  float cam_heading       = 0.0f;   // y-rotation in degrees
  mat4 T                  = translate( identity_mat4(), vec3( -cam_pos.v[0], -cam_pos.v[1], -cam_pos.v[2] ) );
  // rotation matrix from my maths library. just holds 16 floats
  mat4 R;
  // make a quaternion representing negated initial camera orientation
  float quaternion[4];
  create_versor( quaternion, -cam_heading, 0.0f, 1.0f, 0.0f );
  // convert the quaternion to a rotation matrix (just an array of 16 floats)
  quat_to_mat4( R.m, quaternion );
  // combine the inverse rotation and transformation to make a view matrix
  view_mat = R * T;
  // keep track of some useful vectors that can be used for keyboard movement
  vec4 fwd( 0.0f, 0.0f, -1.0f, 0.0f );
  vec4 rgt( 1.0f, 0.0f, 0.0f, 0.0f );
  vec4 up( 0.0f, 1.0f, 0.0f, 0.0f );

  /*---------------------------SET RENDERING
   * DEFAULTS---------------------------*/
  glUseProgram( shader_programme );
  glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
  glUniformMatrix4fv( proj_mat_location, 1, GL_FALSE, proj_mat.m );
  // unique model matrix for each sphere
  mat4 model_mats[NUM_SPHERES];
  for ( int i = 0; i < NUM_SPHERES; i++ ) { model_mats[i] = translate( identity_mat4(), sphere_pos_wor[i] ); }

  glEnable( GL_DEPTH_TEST );          // enable depth-testing
  glDepthFunc( GL_LESS );             // depth-testing interprets a smaller value as "closer"
  glEnable( GL_CULL_FACE );           // cull face
  glCullFace( GL_BACK );              // cull back face
  glFrontFace( GL_CCW );              // set counter-clock-wise vertex order to mean the front
  glClearColor( 0.2, 0.2, 0.2, 1.0 ); // grey background to help spot mistakes
  glViewport( 0, 0, g_gl_width, g_gl_height );

  /*-------------------------------RENDERING
   * LOOP-------------------------------*/
  while ( !glfwWindowShouldClose( g_window ) ) {
    // update timers
    static double previous_seconds = glfwGetTime();
    double current_seconds         = glfwGetTime();
    double elapsed_seconds         = current_seconds - previous_seconds;
    previous_seconds               = current_seconds;
    _update_fps_counter( g_window );

    // wipe the drawing surface clear
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( shader_programme );
    for ( int i = 0; i < NUM_SPHERES; i++ ) {
      glUniformMatrix4fv( model_mat_location, 1, GL_FALSE, model_mats[i].m );
      glDrawArrays( GL_TRIANGLES, 0, point_count );
    }
    // update other events like input handling
    glfwPollEvents();

    // control keys
    bool cam_moved = false;
    vec3 move( 0.0, 0.0, 0.0 );
    float cam_yaw   = 0.0f; // y-rotation in degrees
    float cam_pitch = 0.0f;
    float cam_roll  = 0.0;
    if ( glfwGetKey( g_window, GLFW_KEY_A ) ) {
      move.v[0] -= cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_D ) ) {
      move.v[0] += cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_Q ) ) {
      move.v[1] += cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_E ) ) {
      move.v[1] -= cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_W ) ) {
      move.v[2] -= cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_S ) ) {
      move.v[2] += cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_LEFT ) ) {
      cam_yaw += cam_heading_speed * elapsed_seconds;
      cam_moved = true;

      // create a quaternion representing change in heading (the yaw)
      float q_yaw[4];
      create_versor( q_yaw, cam_yaw, up.v[0], up.v[1], up.v[2] );
      // add yaw rotation to the camera's current orientation
      mult_quat_quat( quaternion, q_yaw, quaternion );

      // recalc axes to suit new orientation
      quat_to_mat4( R.m, quaternion );
      fwd = R * vec4( 0.0, 0.0, -1.0, 0.0 );
      rgt = R * vec4( 1.0, 0.0, 0.0, 0.0 );
      up  = R * vec4( 0.0, 1.0, 0.0, 0.0 );
    }
    if ( glfwGetKey( g_window, GLFW_KEY_RIGHT ) ) {
      cam_yaw -= cam_heading_speed * elapsed_seconds;
      cam_moved = true;
      float q_yaw[4];
      create_versor( q_yaw, cam_yaw, up.v[0], up.v[1], up.v[2] );
      mult_quat_quat( quaternion, q_yaw, quaternion );

      // recalc axes to suit new orientation
      quat_to_mat4( R.m, quaternion );
      fwd = R * vec4( 0.0, 0.0, -1.0, 0.0 );
      rgt = R * vec4( 1.0, 0.0, 0.0, 0.0 );
      up  = R * vec4( 0.0, 1.0, 0.0, 0.0 );
    }
    if ( glfwGetKey( g_window, GLFW_KEY_UP ) ) {
      cam_pitch += cam_heading_speed * elapsed_seconds;
      cam_moved = true;
      float q_pitch[4];
      create_versor( q_pitch, cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2] );
      mult_quat_quat( quaternion, q_pitch, quaternion );

      // recalc axes to suit new orientation
      quat_to_mat4( R.m, quaternion );
      fwd = R * vec4( 0.0, 0.0, -1.0, 0.0 );
      rgt = R * vec4( 1.0, 0.0, 0.0, 0.0 );
      up  = R * vec4( 0.0, 1.0, 0.0, 0.0 );
    }
    if ( glfwGetKey( g_window, GLFW_KEY_DOWN ) ) {
      cam_pitch -= cam_heading_speed * elapsed_seconds;
      cam_moved = true;
      float q_pitch[4];
      create_versor( q_pitch, cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2] );
      mult_quat_quat( quaternion, q_pitch, quaternion );

      // recalc axes to suit new orientation
      quat_to_mat4( R.m, quaternion );
      fwd = R * vec4( 0.0, 0.0, -1.0, 0.0 );
      rgt = R * vec4( 1.0, 0.0, 0.0, 0.0 );
      up  = R * vec4( 0.0, 1.0, 0.0, 0.0 );
    }
    if ( glfwGetKey( g_window, GLFW_KEY_Z ) ) {
      cam_roll -= cam_heading_speed * elapsed_seconds;
      cam_moved = true;
      float q_roll[4];
      create_versor( q_roll, cam_roll, fwd.v[0], fwd.v[1], fwd.v[2] );
      mult_quat_quat( quaternion, q_roll, quaternion );

      // recalc axes to suit new orientation
      quat_to_mat4( R.m, quaternion );
      fwd = R * vec4( 0.0, 0.0, -1.0, 0.0 );
      rgt = R * vec4( 1.0, 0.0, 0.0, 0.0 );
      up  = R * vec4( 0.0, 1.0, 0.0, 0.0 );
    }
    if ( glfwGetKey( g_window, GLFW_KEY_C ) ) {
      cam_roll += cam_heading_speed * elapsed_seconds;
      cam_moved = true;
      float q_roll[4];
      create_versor( q_roll, cam_roll, fwd.v[0], fwd.v[1], fwd.v[2] );
      mult_quat_quat( quaternion, q_roll, quaternion );

      // recalc axes to suit new orientation
      quat_to_mat4( R.m, quaternion );
      fwd = R * vec4( 0.0, 0.0, -1.0, 0.0 );
      rgt = R * vec4( 1.0, 0.0, 0.0, 0.0 );
      up  = R * vec4( 0.0, 1.0, 0.0, 0.0 );
    }
    // update view matrix
    if ( cam_moved ) {
      quat_to_mat4( R.m, quaternion );

      // checking for fp errors
      //	printf ("dot fwd . up %f\n", dot (fwd, up));
      //	printf ("dot rgt . up %f\n", dot (rgt, up));
      //	printf ("dot fwd . rgt\n %f", dot (fwd, rgt));

      cam_pos = cam_pos + vec3( fwd ) * -move.v[2];
      cam_pos = cam_pos + vec3( up ) * move.v[1];
      cam_pos = cam_pos + vec3( rgt ) * move.v[0];
      mat4 T  = translate( identity_mat4(), vec3( cam_pos ) );

      view_mat = inverse( R ) * inverse( T );
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
