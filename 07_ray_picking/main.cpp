/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries separate legal notices                              |
|******************************************************************************|
| Mouse Picking with Ray Casting .                                             |
\******************************************************************************/
#include "gl_utils.h"    // common opengl functions and small utilities like logs
#include "maths_funcs.h" // my maths functions
#include "obj_parser.h"  // my little Wavefront .obj mesh loader
#include <GL/glew.h>     // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h>  // GLFW helper library
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define MESH_FILE "sphere.obj"
#define VERTEX_SHADER_FILE "test_vs.glsl"
#define FRAGMENT_SHADER_FILE "test_fs.glsl"
#define NUM_SPHERES 4

// camera matrices. it's easier if they are global
mat4 view_mat;
mat4 proj_mat;
vec3 cam_pos( 0.0f, 0.0f, 5.0f );
// a world position for each sphere in the scene
vec3 sphere_pos_wor[]     = { vec3( -2.0, 0.0, 0.0 ), vec3( 2.0, 0.0, 0.0 ), vec3( -2.0, 0.0, -2.0 ), vec3( 1.5, 1.0, -1.0 ) };
const float sphere_radius = 1.0f;
// indicates which sphere is selected
int g_selected_sphere = -1;

/* takes mouse position on screen and return ray in world coords */
vec3 get_ray_from_mouse( float mouse_x, float mouse_y ) {
  // screen space (viewport coordinates)
  float x = ( 2.0f * mouse_x ) / g_gl_window_width - 1.0f;
  float y = 1.0f - ( 2.0f * mouse_y ) / g_gl_window_height;
  float z = 1.0f;
  // normalised device space
  vec3 ray_nds = vec3( x, y, z );
  // clip space
  vec4 ray_clip = vec4( ray_nds.v[0], ray_nds.v[1], -1.0, 1.0 );
  // eye space
  vec4 ray_eye = inverse( proj_mat ) * ray_clip;
  ray_eye      = vec4( ray_eye.v[0], ray_eye.v[1], -1.0, 0.0 );
  // world space
  vec3 ray_wor = vec3( inverse( view_mat ) * ray_eye );
  // don't forget to normalise the vector at some point
  ray_wor = normalise( ray_wor );
  return ray_wor;
}

/* check if a ray and a sphere intersect. if not hit, returns false. it rejects
intersections behind the ray caster's origin, and sets intersection_distance to
the closest intersection */
bool ray_sphere( vec3 ray_origin_wor, vec3 ray_direction_wor, vec3 sphere_centre_wor, float sphere_radius, float* intersection_distance ) {
  // work out components of quadratic
  vec3 dist_to_sphere     = ray_origin_wor - sphere_centre_wor;
  float b                 = dot( ray_direction_wor, dist_to_sphere );
  float c                 = dot( dist_to_sphere, dist_to_sphere ) - sphere_radius * sphere_radius;
  float b_squared_minus_c = b * b - c;
  // check for "imaginary" answer. == ray completely misses sphere
  if ( b_squared_minus_c < 0.0f ) { return false; }
  // check for ray hitting twice (in and out of the sphere)
  if ( b_squared_minus_c > 0.0f ) {
    // get the 2 intersection distances along ray
    float t_a              = -b + sqrt( b_squared_minus_c );
    float t_b              = -b - sqrt( b_squared_minus_c );
    *intersection_distance = t_b;
    // if behind viewer, throw one or both away
    if ( t_a < 0.0 ) {
      if ( t_b < 0.0 ) { return false; }
    } else if ( t_b < 0.0 ) {
      *intersection_distance = t_a;
    }

    return true;
  }
  // check for ray hitting once (skimming the surface)
  if ( 0.0f == b_squared_minus_c ) {
    // if behind viewer, throw away
    float t = -b + sqrt( b_squared_minus_c );
    if ( t < 0.0f ) { return false; }
    *intersection_distance = t;
    return true;
  }
  // note: could also check if ray origin is inside sphere radius
  return false;
}

/* this function is called when the mouse buttons are clicked or un-clicked */
void glfw_mouse_click_callback( GLFWwindow* window, int button, int action, int mods ) {
  // Note: could query if window has lost focus here
  if ( GLFW_PRESS == action ) {
    double xpos, ypos;
    glfwGetCursorPos( g_window, &xpos, &ypos );
    // work out ray
    vec3 ray_wor = get_ray_from_mouse( (float)xpos, (float)ypos );
    // check ray against all spheres in scene
    int closest_sphere_clicked = -1;
    float closest_intersection = 0.0f;
    for ( int i = 0; i < NUM_SPHERES; i++ ) {
      float t_dist = 0.0f;
      if ( ray_sphere( cam_pos, ray_wor, sphere_pos_wor[i], sphere_radius, &t_dist ) ) {
        // if more than one sphere is in path of ray, only use the closest one
        if ( -1 == closest_sphere_clicked || t_dist < closest_intersection ) {
          closest_sphere_clicked = i;
          closest_intersection   = t_dist;
        }
      }
    } // endfor
    g_selected_sphere = closest_sphere_clicked;
    printf( "sphere %i was clicked\n", closest_sphere_clicked );
  }
}

void update_perspective() {
  // input variables
  float near   = 0.1f;                                                           // clipping plane
  float far    = 100.0f;                                                         // clipping plane
  float fovy   = 67.0f;                                                          // 67 degrees
  float aspect = (float)g_gl_framebuffer_width / (float)g_gl_framebuffer_height; // aspect ratio
  proj_mat     = perspective( fovy, aspect, near, far );
}

int main() {
  /*--------------------------------START OPENGL--------------------------------*/
  restart_gl_log();
  // start GL context and O/S window using the GLFW helper library
  start_gl();
  // set a function to be called when the mouse is clicked
  glfwSetMouseButtonCallback( g_window, glfw_mouse_click_callback );
  /*------------------------------CREATE GEOMETRY-------------------------------*/
  GLfloat* vp       = NULL; // array of vertex points
  GLfloat* vn       = NULL; // array of vertex normals
  GLfloat* vt       = NULL; // array of texture coordinates
  int g_point_count = 0;
  if ( !load_obj_file( MESH_FILE, vp, vt, vn, g_point_count ) ) {
    gl_log_err( "ERROR: loading mesh file\n" );
    return 1;
  }

  GLuint vao;
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );

  GLuint points_vbo;
  if ( NULL != vp ) {
    glGenBuffers( 1, &points_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
    glBufferData( GL_ARRAY_BUFFER, 3 * g_point_count * sizeof( GLfloat ), vp, GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
    glEnableVertexAttribArray( 0 );
  }

  /*-------------------------------CREATE SHADERS-------------------------------*/
  GLuint shader_programme = create_programme_from_files( VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE );
  int model_mat_location  = glGetUniformLocation( shader_programme, "model" );
  int view_mat_location   = glGetUniformLocation( shader_programme, "view" );
  int proj_mat_location   = glGetUniformLocation( shader_programme, "proj" );
  int blue_location       = glGetUniformLocation( shader_programme, "blue" );

  /*-------------------------------CREATE CAMERA--------------------------------*/
  update_perspective();

  float cam_speed         = 3.0f;  // 1 unit per second
  float cam_heading_speed = 50.0f; // 30 degrees per second
  float cam_heading       = 0.0f;  // y-rotation in degrees
  mat4 T                  = translate( identity_mat4(), vec3( -cam_pos.v[0], -cam_pos.v[1], -cam_pos.v[2] ) );
  mat4 R                  = rotate_y_deg( identity_mat4(), -cam_heading );
  versor q                = quat_from_axis_deg( -cam_heading, 0.0f, 1.0f, 0.0f );
  view_mat                = R * T;
  // keep track of some useful vectors that can be used for keyboard movement
  vec4 fwd( 0.0f, 0.0f, -1.0f, 0.0f );
  vec4 rgt( 1.0f, 0.0f, 0.0f, 0.0f );
  vec4 up( 0.0f, 1.0f, 0.0f, 0.0f );

  /*---------------------------SET RENDERING DEFAULTS---------------------------*/
  // unique model matrix for each sphere
  mat4 model_mats[NUM_SPHERES];
  for ( int i = 0; i < NUM_SPHERES; i++ ) { model_mats[i] = translate( identity_mat4(), sphere_pos_wor[i] ); }

  glEnable( GL_DEPTH_TEST );          // enable depth-testing
  glDepthFunc( GL_LESS );             // depth-testing interprets a smaller value as "closer"
  glEnable( GL_CULL_FACE );           // cull face
  glCullFace( GL_BACK );              // cull back face
  glFrontFace( GL_CCW );              // set counter-clock-wise vertex order to mean the front
  glClearColor( 0.2, 0.2, 0.2, 1.0 ); // grey background to help spot mistakes

  /*-------------------------------RENDERING LOOP-------------------------------*/
  while ( !glfwWindowShouldClose( g_window ) ) {
    // update timers
    static double previous_seconds = glfwGetTime();
    double current_seconds         = glfwGetTime();
    double elapsed_seconds         = current_seconds - previous_seconds;
    previous_seconds               = current_seconds;
    _update_fps_counter( g_window );

    // update viewport every frame in case window resized
    glViewport( 0, 0, g_gl_framebuffer_width, g_gl_framebuffer_height );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( shader_programme );
    glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
    glUniformMatrix4fv( proj_mat_location, 1, GL_FALSE, proj_mat.m );
    glBindVertexArray( vao );

    for ( int i = 0; i < NUM_SPHERES; i++ ) {
      if ( g_selected_sphere == i ) {
        glUniform1f( blue_location, 1.0f );
      } else {
        glUniform1f( blue_location, 0.0f );
      }
      glUniformMatrix4fv( model_mat_location, 1, GL_FALSE, model_mats[i].m );
      glDrawArrays( GL_TRIANGLES, 0, g_point_count );
    }
    // update other events like input handling
    glfwPollEvents();

    if ( g_frambuffer_changed ) {
      update_perspective();
      g_frambuffer_changed = false;
    }

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
      cam_moved    = true;
      versor q_yaw = quat_from_axis_deg( cam_yaw, up.v[0], up.v[1], up.v[2] );
      q            = q_yaw * q;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_RIGHT ) ) {
      cam_yaw -= cam_heading_speed * elapsed_seconds;
      cam_moved    = true;
      versor q_yaw = quat_from_axis_deg( cam_yaw, up.v[0], up.v[1], up.v[2] );
      q            = q_yaw * q;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_UP ) ) {
      cam_pitch += cam_heading_speed * elapsed_seconds;
      cam_moved      = true;
      versor q_pitch = quat_from_axis_deg( cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2] );
      q              = q_pitch * q;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_DOWN ) ) {
      cam_pitch -= cam_heading_speed * elapsed_seconds;
      cam_moved      = true;
      versor q_pitch = quat_from_axis_deg( cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2] );
      q              = q_pitch * q;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_Z ) ) {
      cam_roll -= cam_heading_speed * elapsed_seconds;
      cam_moved     = true;
      versor q_roll = quat_from_axis_deg( cam_roll, fwd.v[0], fwd.v[1], fwd.v[2] );
      q             = q_roll * q;
    }
    if ( glfwGetKey( g_window, GLFW_KEY_C ) ) {
      cam_roll += cam_heading_speed * elapsed_seconds;
      cam_moved     = true;
      versor q_roll = quat_from_axis_deg( cam_roll, fwd.v[0], fwd.v[1], fwd.v[2] );
      q             = q_roll * q;
    }
    // update view matrix
    if ( cam_moved ) {
      // re-calculate local axes so can move fwd in dir cam is pointing
      R   = quat_to_mat4( q );
      fwd = R * vec4( 0.0, 0.0, -1.0, 0.0 );
      rgt = R * vec4( 1.0, 0.0, 0.0, 0.0 );
      up  = R * vec4( 0.0, 1.0, 0.0, 0.0 );

      cam_pos = cam_pos + vec3( fwd ) * -move.v[2];
      cam_pos = cam_pos + vec3( up ) * move.v[1];
      cam_pos = cam_pos + vec3( rgt ) * move.v[0];
      mat4 T  = translate( identity_mat4(), vec3( cam_pos ) );

      view_mat = inverse( R ) * inverse( T );
    }

    if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) { glfwSetWindowShouldClose( g_window, 1 ); }
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers( g_window );
  }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
