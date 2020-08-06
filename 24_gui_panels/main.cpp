/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                          |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| 2d Panel Example                                                             |
| Working in clip space in 2d, you can very quickly build lots of widgety      |
| graphics elements. It's also very easy to work out if the mouse is hovering  |
| over one of your 2d panels                                                   |
\******************************************************************************/
#include "maths_funcs.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"  // Sean Barrett's image loader
#include <GL/glew.h>    // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <stdio.h>

int g_viewport_width  = 640;
int g_viewport_height = 480;

// virtual camera view matrix
mat4 V = identity_mat4();
// virtual camera projection matrix
mat4 P = identity_mat4();

GLuint gp_sp;        // ground plane shader programme
GLint gp_V_loc;      // view matrix location in gp_sp
GLint gp_P_loc;      // projection matrix location in gp_sp
GLuint gui_sp;       // 2d GUI panel shader programme
GLint gui_scale_loc; // scale factors for gui shader

void create_ground_plane_shaders() {
  /* here i used negative y from the buffer as the z value so that it was on
  the floor but also that the 'front' was on the top side. also note how i
  work out the texture coordinates, st, from the vertex point position */
  const char* gp_vs_str =
    "#version 410\n"
    "in vec2 vp;"
    "uniform mat4 V, P;"
    "out vec2 st;"
    "void main () {"
    "  st = (vp + 1.0) * 0.5;"
    "  gl_Position = P * V * vec4 (10.0 * vp.x, -1.0, 10.0 * -vp.y, 1.0);"
    "}";
  const char* gp_fs_str =
    "#version 410\n"
    "in vec2 st;"
    "uniform sampler2D tex;"
    "out vec4 frag_colour;"
    "void main () {"
    "  frag_colour = texture (tex, st);"
    "}";
  GLuint gp_vs = glCreateShader( GL_VERTEX_SHADER );
  glShaderSource( gp_vs, 1, &gp_vs_str, NULL );
  glCompileShader( gp_vs );
  GLuint gp_fs = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( gp_fs, 1, &gp_fs_str, NULL );
  glCompileShader( gp_fs );
  gp_sp = glCreateProgram();
  glAttachShader( gp_sp, gp_vs );
  glAttachShader( gp_sp, gp_fs );
  glLinkProgram( gp_sp );
  // get uniform locations of camera view and projection matrices
  gp_V_loc = glGetUniformLocation( gp_sp, "V" );
  assert( gp_V_loc > -1 );
  gp_P_loc = glGetUniformLocation( gp_sp, "P" );
  assert( gp_P_loc > -1 );
  // set defaults for matrices
  glUseProgram( gp_sp );
  glUniformMatrix4fv( gp_V_loc, 1, GL_FALSE, V.m );
  glUniformMatrix4fv( gp_P_loc, 1, GL_FALSE, P.m );
}

void create_gui_shaders() {
  // note that i scaled down the size to 0.5 * the viewport size here
  const char* gui_vs_str =
    "#version 410\n"
    "in vec2 vp;"
    "uniform vec2 gui_scale;"
    "out vec2 st;"
    "void main () {"
    "  st = (vp + 1.0) * 0.5;"
    "  gl_Position = vec4 (vp * gui_scale, 0.0, 1.0);"
    "}";
  const char* gui_fs_str =
    "#version 410\n"
    "in vec2 st;"
    "uniform sampler2D tex;"
    "out vec4 frag_colour;"
    "void main () {"
    "  frag_colour = texture (tex, st);"
    "}";
  GLuint gui_vs = glCreateShader( GL_VERTEX_SHADER );
  glShaderSource( gui_vs, 1, &gui_vs_str, NULL );
  glCompileShader( gui_vs );
  GLuint gui_fs = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( gui_fs, 1, &gui_fs_str, NULL );
  glCompileShader( gui_fs );
  gui_sp = glCreateProgram();
  glAttachShader( gui_sp, gui_vs );
  glAttachShader( gui_sp, gui_fs );
  glLinkProgram( gui_sp );
  gui_scale_loc = glGetUniformLocation( gui_sp, "gui_scale" );
  assert( gui_scale_loc > -1 );
}

bool load_texture( const char* file_name, GLuint* tex ) {
  int x, y, n;
  int force_channels        = 4;
  unsigned char* image_data = stbi_load( file_name, &x, &y, &n, force_channels );
  if ( !image_data ) {
    fprintf( stderr, "ERROR: could not load %s\n", file_name );
    return false;
  }
  // NPOT check
  if ( ( x & ( x - 1 ) ) != 0 || ( y & ( y - 1 ) ) != 0 ) { fprintf( stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name ); }
  int width_in_bytes    = x * 4;
  unsigned char* top    = NULL;
  unsigned char* bottom = NULL;
  unsigned char temp    = 0;
  int half_height       = y / 2;

  for ( int row = 0; row < half_height; row++ ) {
    top    = image_data + row * width_in_bytes;
    bottom = image_data + ( y - row - 1 ) * width_in_bytes;
    for ( int col = 0; col < width_in_bytes; col++ ) {
      temp    = *top;
      *top    = *bottom;
      *bottom = temp;
      top++;
      bottom++;
    }
  }
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

/* we will tell GLFW to run this function whenever the window is resized */
void glfw_framebuffer_size_callback( GLFWwindow* window, int width, int height ) {
  g_viewport_width  = width;
  g_viewport_height = height;
  /* update any perspective matrices used here */
  P = perspective( 67.0f, (float)g_viewport_width / (float)g_viewport_height, 0.1f, 100.0f );
  glViewport( 0, 0, g_viewport_width, g_viewport_height );
}

int main() {
  // start GL context with helper libraries
  ( glfwInit() );

  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

  GLFWwindow* window = glfwCreateWindow( g_viewport_width, g_viewport_height, "GUI Panels", NULL, NULL );
  glfwSetFramebufferSizeCallback( window, glfw_framebuffer_size_callback );
  glfwMakeContextCurrent( window );
  glewExperimental = GL_TRUE;
  glewInit();
  const GLubyte* renderer = glGetString( GL_RENDERER ); // get renderer string
  const GLubyte* version  = glGetString( GL_VERSION );  // version as a string
  printf( "Renderer: %s\n", renderer );
  printf( "OpenGL version supported %s\n", version );

  // create a 2d panel. from 2 triangles = 6 xy coords.
  float points[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f };
  // for ground plane we can just re-use panel points but y is now z
  GLuint vp_vbo, vao;
  glGenBuffers( 1, &vp_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, vp_vbo );
  glBufferData( GL_ARRAY_BUFFER, sizeof( points ), points, GL_STATIC_DRAW );
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );
  // note: vertex buffer is already bound
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );

  // create a 3d camera to move in 3d so that we can tell that the panel is 2d
  // keep track of some useful vectors that can be used for keyboard movement
  vec4 fwd( 0.0f, 0.0f, -1.0f, 0.0f );
  vec4 rgt( 1.0f, 0.0f, 0.0f, 0.0f );
  vec4 up( 0.0f, 1.0f, 0.0f, 0.0f );
  vec3 cam_pos( 0.0f, 1.0f, 5.0f );
  mat4 T_inv = translate( identity_mat4(), cam_pos );
  // point slightly downwards to see the plane
  versor quaternion = quat_from_axis_deg( 0.0f, 1.0f, 0.0f, 0.0f );
  mat4 R_inv        = quat_to_mat4( quaternion );
  // combine the inverse rotation and transformation to make a view matrix
  V = inverse( R_inv ) * inverse( T_inv );
  // projection matrix
  P                             = perspective( 67.0f, (float)g_viewport_width / (float)g_viewport_height, 0.1f, 100.0f );
  const float cam_speed         = 3.0f;  // 1 unit per second
  const float cam_heading_speed = 50.0f; // 30 degrees per second

  create_ground_plane_shaders();
  create_gui_shaders();

  // textures for ground plane and gui
  GLuint gp_tex, gui_tex;
  ( load_texture( "tile2-diamonds256x256.png", &gp_tex ) );
  ( load_texture( "skulluvmap.png", &gui_tex ) );

  // rendering defaults
  glDepthFunc( GL_LESS );   // set depth function but don't enable yet
  glEnable( GL_CULL_FACE ); // cull face
  glCullFace( GL_BACK );    // cull back face
  glFrontFace( GL_CCW );    // GL_CCW for counter clock-wise

  // absolute panel dimensions in pixels
  const float panel_width  = 256.0f;
  const float panel_height = 256.0f;

  glViewport( 0, 0, g_viewport_width, g_viewport_height );

  // start main rendering loop
  while ( !glfwWindowShouldClose( window ) ) {
    // update timers
    static double previous_seconds = glfwGetTime();
    double current_seconds         = glfwGetTime();
    double elapsed_seconds         = current_seconds - previous_seconds;
    previous_seconds               = current_seconds;

    bool cam_moved = false;
    vec3 move( 0.0, 0.0, 0.0 );

    // wipe the drawing surface clear
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // draw ground plane. note: depth test is enabled here
    glEnable( GL_DEPTH_TEST );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, gp_tex );
    glUseProgram( gp_sp );
    glBindVertexArray( vao );
    glDrawArrays( GL_TRIANGLES, 0, 6 );

    // draw GUI panel. note: depth test is disabled here and drawn AFTER scene
    glDisable( GL_DEPTH_TEST );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, gui_tex );
    glUseProgram( gui_sp );
    // resize panel to size in pixels
    float x_scale = panel_width / g_viewport_width;
    float y_scale = panel_height / g_viewport_height;
    glUniform2f( gui_scale_loc, x_scale, y_scale );
    glBindVertexArray( vao );
    glDrawArrays( GL_TRIANGLES, 0, 6 );

    // update other events like input handling
    glfwPollEvents();
    if ( GLFW_PRESS == glfwGetKey( window, GLFW_KEY_ESCAPE ) ) { glfwSetWindowShouldClose( window, 1 ); }
    float cam_yaw   = 0.0f; // y-rotation in degrees
    float cam_pitch = 0.0f;
    float cam_roll  = 0.0;
    if ( glfwGetKey( window, GLFW_KEY_A ) ) {
      move.v[0] -= cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( window, GLFW_KEY_D ) ) {
      move.v[0] += cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( window, GLFW_KEY_Q ) ) {
      move.v[1] += cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( window, GLFW_KEY_E ) ) {
      move.v[1] -= cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( window, GLFW_KEY_W ) ) {
      move.v[2] -= cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( window, GLFW_KEY_S ) ) {
      move.v[2] += cam_speed * elapsed_seconds;
      cam_moved = true;
    }
    if ( glfwGetKey( window, GLFW_KEY_LEFT ) ) {
      cam_yaw += cam_heading_speed * elapsed_seconds;
      cam_moved    = true;
      versor q_yaw = quat_from_axis_deg( cam_yaw, up.v[0], up.v[1], up.v[2] );
      quaternion   = q_yaw * quaternion;
    }
    if ( glfwGetKey( window, GLFW_KEY_RIGHT ) ) {
      cam_yaw -= cam_heading_speed * elapsed_seconds;
      cam_moved    = true;
      versor q_yaw = quat_from_axis_deg( cam_yaw, up.v[0], up.v[1], up.v[2] );
      quaternion   = q_yaw * quaternion;
    }
    if ( glfwGetKey( window, GLFW_KEY_UP ) ) {
      cam_pitch += cam_heading_speed * elapsed_seconds;
      cam_moved      = true;
      versor q_pitch = quat_from_axis_deg( cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2] );
      quaternion     = q_pitch * quaternion;
    }
    if ( glfwGetKey( window, GLFW_KEY_DOWN ) ) {
      cam_pitch -= cam_heading_speed * elapsed_seconds;
      cam_moved      = true;
      versor q_pitch = quat_from_axis_deg( cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2] );
      quaternion     = q_pitch * quaternion;
    }
    if ( glfwGetKey( window, GLFW_KEY_Z ) ) {
      cam_roll -= cam_heading_speed * elapsed_seconds;
      cam_moved     = true;
      versor q_roll = quat_from_axis_deg( cam_roll, fwd.v[0], fwd.v[1], fwd.v[2] );
      quaternion    = q_roll * quaternion;
    }
    if ( glfwGetKey( window, GLFW_KEY_C ) ) {
      cam_roll += cam_heading_speed * elapsed_seconds;
      cam_moved     = true;
      versor q_roll = quat_from_axis_deg( cam_roll, fwd.v[0], fwd.v[1], fwd.v[2] );
      quaternion    = q_roll * quaternion;
    }
    // update view matrix
    if ( cam_moved ) {
      // re-calculate local axes so can move fwd in dir cam is pointing
      R_inv = quat_to_mat4( quaternion );
      fwd   = R_inv * vec4( 0.0, 0.0, -1.0, 0.0 );
      rgt   = R_inv * vec4( 1.0, 0.0, 0.0, 0.0 );
      up    = R_inv * vec4( 0.0, 1.0, 0.0, 0.0 );

      cam_pos = cam_pos + vec3( fwd ) * -move.v[2];
      cam_pos = cam_pos + vec3( up ) * move.v[1];
      cam_pos = cam_pos + vec3( rgt ) * move.v[0];
      T_inv   = translate( identity_mat4(), cam_pos );

      V = inverse( R_inv ) * inverse( T_inv );
      glUseProgram( gp_sp );
      glUniformMatrix4fv( gp_V_loc, 1, GL_FALSE, V.m );
    }
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers( window );
  }
  // done
  glfwTerminate();
  return 0;
}
