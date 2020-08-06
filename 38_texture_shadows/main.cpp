/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                          |
| See individual libraries' for separate legal notices                         |
|******************************************************************************|
| Shadow Mapping from Williams' Algorithm                                      |
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
#define MESH_FILE "suzanne.obj"
#define PLAIN_VS "plain.vert"
#define PLAIN_FS "plain.frag"
#define DEBUG_VS "ss_quad.vert"
#define DEBUG_FS "ss_quad.frag"
#define DEPTH_VS "depth.vert"
#define DEPTH_FS "depth.frag"
#define NUM_SPHERES 4

/* resolution of the shadow map - this is the critical performance/quality
variable  try changing this*/
int g_shadow_size = 1024;

GLuint g_ground_plane_vao;
int g_ground_plane_point_count;
GLuint g_sphere_vao;
int g_sphere_point_count;
GLuint g_ss_quad_vao;
int g_ss_quad_point_count;
/* shadow caster's view and projection matrices. in practice each light source
will be a shadow caster have a set of these. we only have one caster here */
mat4 g_caster_V;
mat4 g_caster_P;
/* the virtual camera's view and projection matrices */
mat4 g_camera_V;
mat4 g_camera_P;
/* shader used for ground and other objects */
GLuint g_plain_sp;
GLint g_plain_M_loc;        /* model matrix location */
GLint g_plain_V_loc;        /* virtual camera view matrix location */
GLint g_plain_P_loc;        /* virtual camera projection matrix location */
GLint g_plain_caster_V_loc; /* shadow caster view matrix location */
GLint g_plain_caster_P_loc; /* shadow caster projection matrix location */
GLint g_plain_colour_loc;   /* a uniform to switch colour */
GLint g_plain_shad_resolution_loc;
/* shader for debugging quad */
GLuint g_debug_sp;
/* shader to render just the depth to a texture */
GLuint g_depth_sp;
GLint g_depth_M_loc;
GLint g_depth_V_loc;
GLint g_depth_P_loc;
/* framebuffer that renders depth to a texture */
GLuint g_depth_fb;
GLuint g_depth_fb_tex;
/* unique model matrix for each sphere */
mat4 g_sphere_Ms[NUM_SPHERES];

void init_ground_plane() {
  GLuint points_vbo;
  GLfloat gp_pos[]           = { -20.0, -1.0, -20.0, -20.0, -1.0, 20.0, 20.0, -1.0, 20.0, 20.0, -1.0, 20.0, 20.0, -1.0, -20.0, -20.0, -1.0, -20.0 };
  g_ground_plane_point_count = sizeof( gp_pos ) / sizeof( GLfloat ) / 3;

  /* create VBO and VAO here */
  glGenVertexArrays( 1, &g_ground_plane_vao );
  glBindVertexArray( g_ground_plane_vao );
  glGenBuffers( 1, &points_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  glBufferData( GL_ARRAY_BUFFER, sizeof( gp_pos ), gp_pos, GL_STATIC_DRAW );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );
}

void create_shadow_caster() {
  // create a view matrix for the shadow caster
  vec3 light_pos( 7.0f, 7.0f, 0.0f );
  vec3 light_target( 0.0f, 0.0f, 0.0f );
  vec3 up_dir( normalise( vec3( -1.0f, 1.0f, 0.0f ) ) );
  g_caster_V = look_at( light_pos, light_target, up_dir );

  // create a projection matrix for the shadow caster
  float near   = 1.0f;
  float far    = 20.0f;
  float fov    = 35.0f;
  float aspect = 1.0f;
  g_caster_P   = perspective( fov, aspect, near, far );
}

/* some floating objects to cast shadows */
void init_spheres() {
  GLfloat* vp = NULL; // array of vertex points
  GLfloat* vn = NULL; // array of vertex normals
  GLfloat* vt = NULL; // array of texture coordinates
  ( load_obj_file( MESH_FILE, vp, vt, vn, g_sphere_point_count ) );

  glGenVertexArrays( 1, &g_sphere_vao );
  glBindVertexArray( g_sphere_vao );

  GLuint points_vbo = 0;
  if ( NULL != vp ) {
    glGenBuffers( 1, &points_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
    glBufferData( GL_ARRAY_BUFFER, 3 * g_sphere_point_count * sizeof( GLfloat ), vp, GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
    glEnableVertexAttribArray( 0 );
  }
}

/* create a 2d screen space 'quad' of triangles to show the depth map in a
little box */
void init_ss_quad() {
  // create geometry and vao for screen-space quad
  GLfloat ss_quad_pos[] = { -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0 };

  // create VBOs, VAO, and set attribute locations
  g_ss_quad_point_count = sizeof( ss_quad_pos ) / sizeof( GLfloat ) / 2;

  /* create VBO and VAO here */
  glGenVertexArrays( 1, &g_ss_quad_vao );
  glBindVertexArray( g_ss_quad_vao );
  GLuint points_vbo = 0;
  glGenBuffers( 1, &points_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, points_vbo );
  glBufferData( GL_ARRAY_BUFFER, sizeof( ss_quad_pos ), ss_quad_pos, GL_STATIC_DRAW );
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );
}

/* setup framebuffer that renders depth to a texture */
void init_shadow_fb() {
  // create framebuffer
  glGenFramebuffers( 1, &g_depth_fb );
  glBindFramebuffer( GL_FRAMEBUFFER, g_depth_fb );

  // create texture for framebuffer
  glGenTextures( 1, &g_depth_fb_tex );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, g_depth_fb_tex );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, g_shadow_size, g_shadow_size, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL );
  // bi-linear filtering might help, but might make it less accurate too
  // glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

  // clamp to edge. clamp to border may reduce artifacts outside light frustum
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  // attach depth texture to framebuffer
  glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_depth_fb_tex, 0 );

  // tell framebuffer not to use any colour drawing outputs
  GLenum draw_bufs[] = { GL_NONE };
  glDrawBuffers( 1, draw_bufs );

  // this *should* avoid a GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER error that
  // comes on mac - invalid framebuffer due to having only a depth buffer and
  // no colour buffer specified.
  glReadBuffer( GL_NONE );

  // bind default framebuffer again
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

/* do a rendering pass writing just the depth to a texture */
void render_shadow_casting() {
  /* similar to epsilon offset */
  /*
    glEnable (GL_POLYGON_OFFSET_FILL);
    glPolygonOffset (-1,-1); */

  // bind framebuffer that renders to texture instead of screen
  glBindFramebuffer( GL_FRAMEBUFFER, g_depth_fb );
  // set the viewport to the size of the shadow map
  glViewport( 0, 0, g_shadow_size, g_shadow_size );
  // clear the shadow map to black (or white)
  glClearColor( 0.0, 0.0, 0.0, 1.0 );
  // no need to clear the colour buffer
  glClear( GL_DEPTH_BUFFER_BIT );
  // bind out shadow-casting shader from the previous section
  glUseProgram( g_depth_sp );
  // send in the view and projection matrices from the light
  glUniformMatrix4fv( g_depth_V_loc, 1, GL_FALSE, g_caster_V.m );
  glUniformMatrix4fv( g_depth_P_loc, 1, GL_FALSE, g_caster_P.m );
  // model matrix does nothing for the monkey - make it an identity matrix

  // bind the sphere vao and draw them
  glBindVertexArray( g_sphere_vao );
  for ( int i = 0; i < NUM_SPHERES; i++ ) {
    glUniformMatrix4fv( g_depth_M_loc, 1, GL_FALSE, g_sphere_Ms[i].m );
    glDrawArrays( GL_TRIANGLES, 0, g_sphere_point_count );
  }
  // bind the default framebuffer again
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );

  glDisable( GL_POLYGON_OFFSET_FILL );
}

// a world position for each sphere in the scene
vec3 sphere_pos_wor[] = { vec3( -2.0, 0.0, 0.0 ), vec3( 2.0, 0.0, 0.0 ), vec3( -2.0, 0.0, -2.0 ), vec3( 1.5, 1.0, -1.0 ) };

int main() {
  /*--------------------------------START OPENGL--------------------------------*/
  ( restart_gl_log() );
  /* start GL context and O/S window using the GLFW helper library */
  ( start_gl() );
  /*---------------------CREATE FRAMEBUFFER TO CAPTURE DEPTH--------------------*/
  init_shadow_fb();
  /*------------------------------CREATE GEOMETRY-------------------------------*/
  init_ground_plane(); /* a floor that i'll make green */
  init_spheres();      /* some floating spheres to cast shadows */
  init_ss_quad();      /* on-screen square for debugging the depth map */

  /*-------------------------------CREATE SHADERS-------------------------------*/
  g_plain_sp                  = create_programme_from_files( PLAIN_VS, PLAIN_FS );
  g_plain_M_loc               = glGetUniformLocation( g_plain_sp, "M" );
  g_plain_V_loc               = glGetUniformLocation( g_plain_sp, "V" );
  g_plain_P_loc               = glGetUniformLocation( g_plain_sp, "P" );
  g_plain_caster_V_loc        = glGetUniformLocation( g_plain_sp, "caster_V" );
  g_plain_caster_P_loc        = glGetUniformLocation( g_plain_sp, "caster_P" );
  g_plain_colour_loc          = glGetUniformLocation( g_plain_sp, "colour" );
  g_plain_shad_resolution_loc = glGetUniformLocation( g_plain_sp, "shad_resolution" );

  g_debug_sp = create_programme_from_files( DEBUG_VS, DEBUG_FS );

  g_depth_sp    = create_programme_from_files( DEPTH_VS, DEPTH_FS );
  g_depth_M_loc = glGetUniformLocation( g_depth_sp, "M" );
  g_depth_V_loc = glGetUniformLocation( g_depth_sp, "V" );
  g_depth_P_loc = glGetUniformLocation( g_depth_sp, "P" );
  /*-------------------------------CREATE CAMERAS-------------------------------*/
  create_shadow_caster();

  float near   = 0.1f;                                   // clipping plane
  float far    = 100.0f;                                 // clipping plane
  float fov    = 67.0f;                                  // convert 67 degrees to radians
  float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
  g_camera_P   = perspective( fov, aspect, near, far );

  float cam_speed         = 5.0f;   // 1 unit per second
  float cam_heading_speed = 100.0f; // 10 degrees per second
  vec3 cam_pos( 0.0f, 1.0f, 5.0f ); // don't start at zero, or we will be too close
  versor quaternion = quat_from_axis_deg( 0.0f, 0.0f, 1.0f, 0.0f );
  vec4 fwd( 0.0f, 0.0f, -1.0f, 0.0f );
  vec4 rgt( 1.0f, 0.0f, 0.0f, 0.0f );
  vec4 up( 0.0f, 1.0f, 0.0f, 0.0f );
  mat4 T     = translate( identity_mat4(), cam_pos );
  mat4 R     = quat_to_mat4( quaternion );
  g_camera_V = inverse( R ) * inverse( T );

  /*---------------------------SET RENDERING DEFAULTS---------------------------*/
  glUseProgram( g_plain_sp );
  glUniformMatrix4fv( g_plain_V_loc, 1, GL_FALSE, g_camera_V.m );
  glUniformMatrix4fv( g_plain_P_loc, 1, GL_FALSE, g_camera_P.m );
  glUniformMatrix4fv( g_plain_caster_V_loc, 1, GL_FALSE, g_caster_V.m );
  glUniformMatrix4fv( g_plain_caster_P_loc, 1, GL_FALSE, g_caster_P.m );
  glUniform1f( g_plain_shad_resolution_loc, (GLfloat)g_shadow_size );
  for ( int i = 0; i < NUM_SPHERES; i++ ) { g_sphere_Ms[i] = translate( identity_mat4(), sphere_pos_wor[i] ); }

  glEnable( GL_CULL_FACE );  // cull face
  glEnable( GL_DEPTH_TEST ); // enable depth-testing
  glDepthFunc( GL_LESS );    // depth-testing interprets a smaller value as "closer"
  /*-------------------------------RENDERING LOOP-------------------------------*/
  while ( !glfwWindowShouldClose( g_window ) ) {
    // update timers
    static double previous_seconds = glfwGetTime();
    double current_seconds         = glfwGetTime();
    double elapsed_seconds         = current_seconds - previous_seconds;
    previous_seconds               = current_seconds;
    _update_fps_counter( g_window );

    /*------------------------DEPTH WRITING RENDERING PASS--------------------------
    should do one of these per visible light source or shadow caster. we just have
    one caster that writes to a single texture
    ------------------------------------------------------------------------------*/
    /* back-face rendering only to remove self-shadowing issues. usually helps
    in this demo it made it worse */
    // glCullFace (GL_FRONT);
    render_shadow_casting();
    /*------------------------DEPTH READING RENDERING PASS--------------------------
    normal rendering here, but we can sample the depth map and use the shadow
    caster's view and projection matrices to work out what part of the depth map
    should cover the rendered parts of our scene. note that i reset the culling
    information, clear colour, and viewport dimensions here, because these are
    changed in the shadow casting pass */
    glCullFace( GL_BACK );              // cull back face
    glFrontFace( GL_CCW );              // set counter-clock-wise vertex order to mean the front
    glClearColor( 0.2, 0.2, 0.2, 1.0 ); // grey background to help spot mistakes
    glViewport( 0, 0, g_gl_width, g_gl_height );
    // wipe the drawing surface clear
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glUseProgram( g_plain_sp );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, g_depth_fb_tex );

    /* ground plane (receives shadows) */
    glUniform3f( g_plain_colour_loc, 0.0, 1.0, 0.0 ); /* green */
    glBindVertexArray( g_ground_plane_vao );
    glUniformMatrix4fv( g_plain_M_loc, 1, GL_FALSE, identity_mat4().m );
    glDrawArrays( GL_TRIANGLES, 0, g_ground_plane_point_count );

    /* spheres (cast and receive shadows) */
    glUniform3f( g_plain_colour_loc, 1.0, 0.0, 0.0 ); /* red */
    glBindVertexArray( g_sphere_vao );
    for ( int i = 0; i < NUM_SPHERES; i++ ) {
      glUniformMatrix4fv( g_plain_M_loc, 1, GL_FALSE, g_sphere_Ms[i].m );
      glDrawArrays( GL_TRIANGLES, 0, g_sphere_point_count );
    }
    // update other events like input handling
    glfwPollEvents();

    // control keys
    bool cam_moved  = false;
    float cam_yaw   = 0.0f; // y-rotation in degrees
    float cam_pitch = 0.0f; // y-rotation in degrees
    float cam_roll  = 0.0f; // y-rotation in degrees
    vec3 move( 0.0, 0.0, 0.0 );
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
    // rotations are else-if to prevent roll when pitch and yawing
    if ( glfwGetKey( g_window, GLFW_KEY_LEFT ) ) {
      cam_yaw += cam_heading_speed * elapsed_seconds;
      cam_moved = true;
      // create a quaternion representing change in heading (the yaw)
      versor q_yaw = quat_from_axis_deg( cam_yaw, 0.0f, 1.0f, 0.0f );
      // add yaw rotation to the camera's current orientation
      quaternion = q_yaw * quaternion;
    } else if ( glfwGetKey( g_window, GLFW_KEY_RIGHT ) ) {
      cam_yaw -= cam_heading_speed * elapsed_seconds;
      cam_moved    = true;
      versor q_yaw = quat_from_axis_deg( cam_yaw, 0.0f, 1.0f, 0.0f );
      quaternion   = q_yaw * quaternion;
    } else if ( glfwGetKey( g_window, GLFW_KEY_UP ) ) {
      cam_pitch += cam_heading_speed * elapsed_seconds;
      cam_moved      = true;
      versor q_pitch = quat_from_axis_deg( cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2] );
      quaternion     = q_pitch * quaternion;
    } else if ( glfwGetKey( g_window, GLFW_KEY_DOWN ) ) {
      cam_pitch -= cam_heading_speed * elapsed_seconds;
      cam_moved      = true;
      versor q_pitch = quat_from_axis_deg( cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2] );
      quaternion     = q_pitch * quaternion;
    } else if ( glfwGetKey( g_window, GLFW_KEY_Z ) ) {
      cam_roll -= cam_heading_speed * elapsed_seconds;
      cam_moved     = true;
      versor q_roll = quat_from_axis_deg( cam_roll, fwd.v[0], fwd.v[1], fwd.v[2] );
      quaternion    = q_roll * quaternion;
    } else if ( glfwGetKey( g_window, GLFW_KEY_C ) ) {
      cam_roll += cam_heading_speed * elapsed_seconds;
      cam_moved     = true;
      versor q_roll = quat_from_axis_deg( cam_roll, fwd.v[0], fwd.v[1], fwd.v[2] );
      quaternion    = q_roll * quaternion;
    }
    // update view matrix
    if ( cam_moved ) {
      // re-calculate local axes so can move fwd in dir cam is pointing
      R   = quat_to_mat4( quaternion );
      fwd = R * vec4( 0.0, 0.0, -1.0, 0.0 );
      rgt = R * vec4( 1.0, 0.0, 0.0, 0.0 );
      up  = R * vec4( 0.0, 1.0, 0.0, 0.0 );

      cam_pos    = cam_pos + vec3( fwd ) * -move.v[2];
      cam_pos    = cam_pos + vec3( 0.0f, 1.0f, 0.0f ) * move.v[1];
      cam_pos    = cam_pos + vec3( rgt ) * move.v[0];
      mat4 T     = translate( identity_mat4(), cam_pos );
      g_camera_V = inverse( R ) * inverse( T );
      glUniformMatrix4fv( g_plain_V_loc, 1, GL_FALSE, g_camera_V.m );
    }
    /* switch between looking from virtual camera and shadow caster matrices */
    if ( glfwGetKey( g_window, GLFW_KEY_SPACE ) ) {
      glUniformMatrix4fv( g_plain_V_loc, 1, GL_FALSE, g_caster_V.m );
      glUniformMatrix4fv( g_plain_P_loc, 1, GL_FALSE, g_caster_P.m );
    } else {
      glUniformMatrix4fv( g_plain_V_loc, 1, GL_FALSE, g_camera_V.m );
      glUniformMatrix4fv( g_plain_P_loc, 1, GL_FALSE, g_camera_P.m );
    }

    /* draw ss quad */
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, g_depth_fb_tex );
    glUseProgram( g_debug_sp );
    glBindVertexArray( g_ss_quad_vao );
    glDrawArrays( GL_TRIANGLES, 0, g_ss_quad_point_count );

    if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) { glfwSetWindowShouldClose( g_window, 1 ); }
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers( g_window );
  }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
