/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| This demo uses the Assimp library to load a mesh from a file, and supports   |
| many formats. The library is VERY big and complex. It's much easier to write |
| a simple Wavefront .obj loader. I have code for this in other demos. However,|
| Assimp will load animated meshes, which will we need to use later, so this   |
| demo is a starting point before doing skinning animation                     |
\******************************************************************************/
#include "gl_utils.h"
#include "maths_funcs.h"
#include <GL/glew.h>    // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <assimp/cimport.h>     // C importer
#include <assimp/postprocess.h> // various extra operations
#include <assimp/scene.h>       // collects data
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define GL_LOG_FILE "gl.log"
#define VERTEX_SHADER_FILE "test_vs.glsl"
#define FRAGMENT_SHADER_FILE "test_fs.glsl"
#define MESH_FILE "monkey_with_bones_y_up.dae"
//#define MESH_FILE "monkey_with_bones.dae"
/* max bones allowed in a mesh */
#define MAX_BONES 32

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width       = 640;
int g_gl_height      = 480;
GLFWwindow* g_window = NULL;

mat4 convert_assimp_matrix( aiMatrix4x4 m ) { return mat4( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, m.a4, m.b4, m.c4, m.d4 ); }

/* load a mesh using the assimp library */
bool load_mesh( const char* file_name, GLuint* vao, int* point_count, mat4* bone_offset_mats, int* bone_count ) {
  const aiScene* scene = aiImportFile( file_name, aiProcess_Triangulate );
  if ( !scene ) {
    fprintf( stderr, "ERROR: reading mesh %s\n", file_name );
    return false;
  }
  printf( "  %i animations\n", scene->mNumAnimations );
  printf( "  %i cameras\n", scene->mNumCameras );
  printf( "  %i lights\n", scene->mNumLights );
  printf( "  %i materials\n", scene->mNumMaterials );
  printf( "  %i meshes\n", scene->mNumMeshes );
  printf( "  %i textures\n", scene->mNumTextures );

  /* get first mesh in file only */
  const aiMesh* mesh = scene->mMeshes[0];
  printf( "    %i vertices in mesh[0]\n", mesh->mNumVertices );

  /* pass back number of vertex points in mesh */
  *point_count = mesh->mNumVertices;

  /* generate a VAO, using the pass-by-reference parameter that we give to the
  function */
  glGenVertexArrays( 1, vao );
  glBindVertexArray( *vao );

  /* we really need to copy out all the data from AssImp's funny little data
  structures into pure contiguous arrays before we copy it into data buffers
  because assimp's texture coordinates are not really contiguous in memory.
  i allocate some dynamic memory to do this. */
  GLfloat* points    = NULL; // array of vertex points
  GLfloat* normals   = NULL; // array of vertex normals
  GLfloat* texcoords = NULL; // array of texture coordinates
  GLint* bone_ids    = NULL; // array of bone IDs
  if ( mesh->HasPositions() ) {
    points = (GLfloat*)malloc( *point_count * 3 * sizeof( GLfloat ) );
    for ( int i = 0; i < *point_count; i++ ) {
      const aiVector3D* vp = &( mesh->mVertices[i] );
      points[i * 3]        = (GLfloat)vp->x;
      points[i * 3 + 1]    = (GLfloat)vp->y;
      points[i * 3 + 2]    = (GLfloat)vp->z;
    }
  }
  if ( mesh->HasNormals() ) {
    normals = (GLfloat*)malloc( *point_count * 3 * sizeof( GLfloat ) );
    for ( int i = 0; i < *point_count; i++ ) {
      const aiVector3D* vn = &( mesh->mNormals[i] );
      normals[i * 3]       = (GLfloat)vn->x;
      normals[i * 3 + 1]   = (GLfloat)vn->y;
      normals[i * 3 + 2]   = (GLfloat)vn->z;
    }
  }
  if ( mesh->HasTextureCoords( 0 ) ) {
    texcoords = (GLfloat*)malloc( *point_count * 2 * sizeof( GLfloat ) );
    for ( int i = 0; i < *point_count; i++ ) {
      const aiVector3D* vt = &( mesh->mTextureCoords[0][i] );
      texcoords[i * 2]     = (GLfloat)vt->x;
      texcoords[i * 2 + 1] = (GLfloat)vt->y;
    }
  }

  /* extract bone weights */
  if ( mesh->HasBones() ) {
    *bone_count = (int)mesh->mNumBones;
    /* an array of bones names. max 256 bones, max name length 64 */
    char bone_names[256][64];

    /* here I allocate an array of per-vertex bone IDs.
    each vertex must know which bone(s) affect it
    here I simplify, and assume that only one bone can affect each vertex,
    so my array is only one-dimensional
    */
    bone_ids = (int*)malloc( *point_count * sizeof( int ) );

    for ( int b_i = 0; b_i < *bone_count; b_i++ ) {
      const aiBone* bone = mesh->mBones[b_i];

      /* get bone names */
      strcpy( bone_names[b_i], bone->mName.data );
      printf( "bone_names[%i]=%s\n", b_i, bone_names[b_i] );

      /* get [inverse] offset matrix for each bone */
      bone_offset_mats[b_i] = convert_assimp_matrix( bone->mOffsetMatrix );

      /* get bone weights
      we can just assume weight is always 1.0, because we are just using 1 bone
      per vertex. but any bone that affects a vertex will be assigned as the
      vertex' bone_id */
      int num_weights = (int)bone->mNumWeights;
      for ( int w_i = 0; w_i < num_weights; w_i++ ) {
        aiVertexWeight weight = bone->mWeights[w_i];
        int vertex_id         = (int)weight.mVertexId;
        // ignore weight if less than 0.5 factor
        if ( weight.mWeight >= 0.5f ) { bone_ids[vertex_id] = b_i; }
      }

    } // endfor
  }   // endif

  /* copy mesh data into VBOs */
  if ( mesh->HasPositions() ) {
    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, 3 * *point_count * sizeof( GLfloat ), points, GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
    glEnableVertexAttribArray( 0 );
    free( points );
  }
  if ( mesh->HasNormals() ) {
    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, 3 * *point_count * sizeof( GLfloat ), normals, GL_STATIC_DRAW );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
    glEnableVertexAttribArray( 1 );
    free( normals );
  }
  if ( mesh->HasTextureCoords( 0 ) ) {
    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, 2 * *point_count * sizeof( GLfloat ), texcoords, GL_STATIC_DRAW );
    glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 0, NULL );
    glEnableVertexAttribArray( 2 );
    free( texcoords );
  }
  if ( mesh->HasTangentsAndBitangents() ) {
    // NB: could store/print tangents here
  }
  if ( mesh->HasBones() ) {
    GLuint vbo;
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, *point_count * sizeof( GLint ), bone_ids, GL_STATIC_DRAW );
    glVertexAttribIPointer( 3, 1, GL_INT, 0, NULL );
    glEnableVertexAttribArray( 3 );
    free( bone_ids );
  }

  aiReleaseImport( scene );
  printf( "mesh loaded\n" );

  return true;
}

int main() {
  ( restart_gl_log() );
  ( start_gl() );
  glEnable( GL_DEPTH_TEST );          // enable depth-testing
  glDepthFunc( GL_LESS );             // depth-testing interprets a smaller value as "closer"
  glEnable( GL_CULL_FACE );           // cull face
  glCullFace( GL_BACK );              // cull back face
  glFrontFace( GL_CCW );              // set counter-clock-wise vertex order to mean the front
  glClearColor( 0.2, 0.2, 0.2, 1.0 ); // grey background to help spot mistakes
  glViewport( 0, 0, g_gl_width, g_gl_height );

  /* load the mesh using assimp */
  GLuint monkey_vao;
  mat4 monkey_bone_offset_matrices[MAX_BONES];
  int monkey_point_count = 0;
  int monkey_bone_count  = 0;
  ( load_mesh( MESH_FILE, &monkey_vao, &monkey_point_count, monkey_bone_offset_matrices, &monkey_bone_count ) );
  printf( "monkey bone count %i\n", monkey_bone_count );

  /* create a buffer of bone positions for visualising the bones */
  float bone_positions[3 * 256];
  int c = 0;
  for ( int i = 0; i < monkey_bone_count; i++ ) {
    print( monkey_bone_offset_matrices[i] );

    // get the x y z translation elements from the last column in the array
    bone_positions[c++] = -monkey_bone_offset_matrices[i].m[12];
    bone_positions[c++] = -monkey_bone_offset_matrices[i].m[13];
    bone_positions[c++] = -monkey_bone_offset_matrices[i].m[14];
  }
  GLuint bones_vao;
  glGenVertexArrays( 1, &bones_vao );
  glBindVertexArray( bones_vao );
  GLuint bones_vbo;
  glGenBuffers( 1, &bones_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, bones_vbo );
  glBufferData( GL_ARRAY_BUFFER, 3 * monkey_bone_count * sizeof( float ), bone_positions, GL_STATIC_DRAW );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
  glEnableVertexAttribArray( 0 );

  /*-------------------------------CREATE SHADERS-------------------------------*/
  GLuint shader_programme       = create_programme_from_files( VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE );
  GLuint bones_shader_programme = create_programme_from_files( "bones.vert", "bones.frag" );

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

  float cam_speed     = 5.0f;                 // 1 unit per second
  float cam_yaw_speed = 40.0f;                // 10 degrees per second
  float cam_pos[]     = { 0.0f, 0.0f, 5.0f }; // don't start at zero, or we will be too close
  float cam_yaw       = 0.0f;                 // y-rotation in degrees
  mat4 T              = translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1], -cam_pos[2] ) );
  mat4 R              = rotate_y_deg( identity_mat4(), -cam_yaw );
  mat4 view_mat       = R * T;

  /* apply a model matrix that rotates our mesh up the correct way */
  mat4 model_mat = identity_mat4();

  glUseProgram( shader_programme );
  int model_mat_location = glGetUniformLocation( shader_programme, "model" );
  glUniformMatrix4fv( model_mat_location, 1, GL_FALSE, model_mat.m );
  int view_mat_location = glGetUniformLocation( shader_programme, "view" );
  glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
  int proj_mat_location = glGetUniformLocation( shader_programme, "proj" );
  glUniformMatrix4fv( proj_mat_location, 1, GL_FALSE, proj_mat );
  int bone_matrices_locations[MAX_BONES];
  // reset all the bone matrices
  char name[64];
  for ( int i = 0; i < MAX_BONES; i++ ) {
    sprintf( name, "bone_matrices[%i]", i );
    bone_matrices_locations[i] = glGetUniformLocation( shader_programme, name );
    glUniformMatrix4fv( bone_matrices_locations[i], 1, GL_FALSE, identity_mat4().m );
  }

  glUseProgram( bones_shader_programme );
  int bones_view_mat_location = glGetUniformLocation( bones_shader_programme, "view" );
  glUniformMatrix4fv( bones_view_mat_location, 1, GL_FALSE, view_mat.m );
  int bones_proj_mat_location = glGetUniformLocation( bones_shader_programme, "proj" );
  glUniformMatrix4fv( bones_proj_mat_location, 1, GL_FALSE, proj_mat );

  float theta     = 0.0f;
  float rot_speed = 50.0f; // 50 radians per second

  while ( !glfwWindowShouldClose( g_window ) ) {
    static double previous_seconds = glfwGetTime();
    double current_seconds         = glfwGetTime();
    double elapsed_seconds         = current_seconds - previous_seconds;
    previous_seconds               = current_seconds;

    _update_fps_counter( g_window );
    // wipe the drawing surface clear
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glViewport( 0, 0, g_gl_width, g_gl_height );

    glEnable( GL_DEPTH_TEST );
    glUseProgram( shader_programme );
    glBindVertexArray( monkey_vao );
    glDrawArrays( GL_TRIANGLES, 0, monkey_point_count );

    glDisable( GL_DEPTH_TEST );
    glEnable( GL_PROGRAM_POINT_SIZE );
    glUseProgram( bones_shader_programme );
    glBindVertexArray( bones_vao );
    glDrawArrays( GL_POINTS, 0, monkey_bone_count );
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
      // cam translation
      mat4 T        = translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1], -cam_pos[2] ) );
      mat4 R        = rotate_y_deg( identity_mat4(), -cam_yaw ); //
      mat4 view_mat = R * T;
      glUseProgram( shader_programme );
      glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
      glUseProgram( bones_shader_programme );
      glUniformMatrix4fv( bones_view_mat_location, 1, GL_FALSE, view_mat.m );
    }
    mat4 ear_mat = identity_mat4();
    if ( glfwGetKey( g_window, 'Z' ) ) {
      theta += rot_speed * elapsed_seconds;
      glUseProgram( shader_programme );
      ear_mat = inverse( monkey_bone_offset_matrices[0] ) * rotate_z_deg( identity_mat4(), theta ) * monkey_bone_offset_matrices[0];
      glUniformMatrix4fv( bone_matrices_locations[0], 1, GL_FALSE, ear_mat.m );
      ear_mat = inverse( monkey_bone_offset_matrices[1] ) * rotate_z_deg( identity_mat4(), -theta ) * monkey_bone_offset_matrices[1];
      glUniformMatrix4fv( bone_matrices_locations[1], 1, GL_FALSE, ear_mat.m );
    }
    if ( glfwGetKey( g_window, 'X' ) ) {
      theta -= rot_speed * elapsed_seconds;
      glUseProgram( shader_programme );
      ear_mat = inverse( monkey_bone_offset_matrices[0] ) * rotate_z_deg( identity_mat4(), theta ) * monkey_bone_offset_matrices[0];
      glUniformMatrix4fv( bone_matrices_locations[0], 1, GL_FALSE, ear_mat.m );
      ear_mat = inverse( monkey_bone_offset_matrices[1] ) * rotate_z_deg( identity_mat4(), -theta ) * monkey_bone_offset_matrices[1];
      glUniformMatrix4fv( bone_matrices_locations[1], 1, GL_FALSE, ear_mat.m );
    }

    if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) { glfwSetWindowShouldClose( g_window, 1 ); }
    // put the stuff we've been drawing onto the display
    glfwSwapBuffers( g_window );
  }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
