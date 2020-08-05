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
#include <GL/glew.h>		// include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assert.h>
#include <assimp/cimport.h>			// C importer
#include <assimp/postprocess.h> // various extra operations
#include <assimp/scene.h>				// collects data
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#define GL_LOG_FILE "gl.log"
#define VERTEX_SHADER_FILE "test_vs.glsl"
#define FRAGMENT_SHADER_FILE "test_fs.glsl"
#define MESH_FILE "monkey_with_anim_y_up.dae"
//#define MESH_FILE "Cylinder2.dae"
/* max bones allowed in a mesh */
#define MAX_BONES 32

/* keep track of window size for things like the viewport and the mouse cursor*/
int g_gl_width = 640;
int g_gl_height = 480;
GLFWwindow *g_window = NULL;

/* data structure that we will use to make a hierarchical tree for our skeleton
*/
struct Skeleton_Node;
struct Skeleton_Node {
	Skeleton_Node *children[MAX_BONES];

	/* key frames */
	vec3 *pos_keys;
	versor *rot_keys;
	vec3 *sca_keys;
	double *pos_key_times;
	double *rot_key_times;
	double *sca_key_times;
	int num_pos_keys;
	int num_rot_keys;
	int num_sca_keys;

	/* name of the bone - might be useful to remember for doing interesting stuff
	in your programme */
	char name[64];
	int num_children;
	/* if this node corresponds to one of our weight-painted bones then we give
	the index of that (the bone_ID) here, otherwrise it is set to -1 */
	int bone_index;
};

/* traverse a skeleton starting at root, and a get a pointer to a node with the
given name */
Skeleton_Node *find_node_in_skeleton( Skeleton_Node *root, const char *node_name ) {

	// validate self
	assert( root );

	// look for match
	if ( strcmp( node_name, root->name ) == 0 ) {
		return root;
	}

	// recurse to chidlren
	for ( int i = 0; i < root->num_children; i++ ) {
		Skeleton_Node *child = find_node_in_skeleton( root->children[i], node_name );
		if ( child != NULL ) {
			return child;
		}
	}

	// no children match and no self match
	return NULL;
}

/* recursive function to pull all of AssImps 'node' hierarchy out. AssImp's
tree will include everything in the scene; cameras, lights, the mesh, but also
our "Armature" which further breaks into our skeleton hierarchy. When we find a
node, we check if its name matches one of our bones' names. if so we record the
index of that bone. */
bool import_skeleton_node( aiNode *assimp_node, Skeleton_Node **skeleton_node,
													 int bone_count, char bone_names[][64] );

bool import_skeleton_node( aiNode *assimp_node, Skeleton_Node **skeleton_node,
													 int bone_count, char bone_names[][64] ) {
	// allocate memory for node
	Skeleton_Node *temp = (Skeleton_Node *)malloc( sizeof( Skeleton_Node ) );

	// get node properties out of AssImp
	strcpy( temp->name, assimp_node->mName.C_Str() );
	printf( "-node name = %s\n", temp->name );
	temp->num_children = 0;

	/* initialise key-frame info */
	temp->pos_keys = NULL;
	temp->rot_keys = NULL;
	temp->sca_keys = NULL;
	temp->pos_key_times = NULL;
	temp->rot_key_times = NULL;
	temp->sca_key_times = NULL;
	temp->num_pos_keys = 0;
	temp->num_rot_keys = 0;
	temp->num_sca_keys = 0;

	printf( "node has %i children\n", (int)assimp_node->mNumChildren );
	temp->bone_index = -1;
	for ( int i = 0; i < MAX_BONES; i++ ) {
		temp->children[i] = NULL;
	}

	// look for matching bone name
	bool has_bone = false;
	for ( int i = 0; i < bone_count; i++ ) {
		if ( strcmp( bone_names[i], temp->name ) == 0 ) {
			printf( "node uses bone %i\n", i );
			temp->bone_index = i;
			has_bone = true;
			break;
		}
	}
	if ( !has_bone ) {
		printf( "no bone found for node\n" );
	}

	bool has_useful_child = false;
	for ( int i = 0; i < (int)assimp_node->mNumChildren; i++ ) {
		if ( import_skeleton_node( assimp_node->mChildren[i],
															 &temp->children[temp->num_children], bone_count,
															 bone_names ) ) {
			has_useful_child = true;
			temp->num_children++;
		} else {
			printf( "useless child culled\n" );
		}
	}
	if ( has_useful_child || has_bone ) {
		// point parameter to our allocated node
		*skeleton_node = temp;
		return true;
	}
	// no bone or good children - cull self
	free( temp );
	temp = NULL;
	return false;
}

/* recursive animation using hierarchy. animate node, children inherit
animation */
void skeleton_animate( Skeleton_Node *node, double anim_time, mat4 parent_mat,
											 mat4 *bone_offset_mats, mat4 *bone_animation_mats );

void skeleton_animate( Skeleton_Node *node, double anim_time, mat4 parent_mat,
											 mat4 *bone_offset_mats, mat4 *bone_animation_mats ) {

	assert( node );

	/* the animation of a node after inheriting its parent's animation */
	mat4 our_mat = parent_mat;

	/* the animation for a particular bone at this time */
	mat4 local_anim = identity_mat4();

	mat4 node_T = identity_mat4();
	if ( node->num_pos_keys > 0 ) {
		int prev_key = 0;
		int next_key = 0;
		for ( int i = 0; i < node->num_pos_keys - 1; i++ ) {
			prev_key = i;
			next_key = i + 1;
			if ( node->pos_key_times[next_key] >= anim_time ) {
				break;
			}
		}
		float total_t = node->pos_key_times[next_key] - node->pos_key_times[prev_key];
		float t = ( anim_time - node->pos_key_times[prev_key] ) / total_t;
		vec3 vi = node->pos_keys[prev_key];
		vec3 vf = node->pos_keys[next_key];
		vec3 lerped = vi * ( 1.0f - t ) + vf * t;
		node_T = translate( identity_mat4(), lerped );
	}

	mat4 node_R = identity_mat4();
	if ( node->num_rot_keys > 0 ) {
		// find next and previous keys
		int prev_key = 0;
		int next_key = 0;
		for ( int i = 0; i < node->num_rot_keys - 1; i++ ) {
			prev_key = i;
			next_key = i + 1;
			if ( node->rot_key_times[next_key] >= anim_time ) {
				break;
			}
		}
		float total_t = node->rot_key_times[next_key] - node->rot_key_times[prev_key];
		float t = ( anim_time - node->rot_key_times[prev_key] ) / total_t;
		versor qi = node->rot_keys[prev_key];
		versor qf = node->rot_keys[next_key];
		versor slerped = slerp( qi, qf, t );
		node_R = quat_to_mat4( slerped );
	}

	local_anim = node_T * node_R;

	// if node has a weighted bone...
	int bone_i = node->bone_index;
	if ( bone_i > -1 ) {
		// ... then get offset matrices
		mat4 bone_offset = bone_offset_mats[bone_i];

		our_mat = parent_mat * local_anim;
		bone_animation_mats[bone_i] = parent_mat * local_anim * bone_offset;
	}
	for ( int i = 0; i < node->num_children; i++ ) {
		skeleton_animate( node->children[i], anim_time, our_mat, bone_offset_mats,
											bone_animation_mats );
	}
}

/* convert one of AssImp's matrices to one of mine. I ignore any rotation data
in AssImp's matrix and just use the translation part */
mat4 convert_assimp_matrix( aiMatrix4x4 m ) {
	return mat4( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
							 0.0f, m.a4, m.b4, m.c4, m.d4 );
}

/* load a mesh using the assimp library */
bool load_mesh( const char *file_name, GLuint *vao, int *point_count,
								mat4 *bone_offset_mats, int *bone_count, Skeleton_Node **root_node,
								double *anim_duration ) {
	const aiScene *scene = aiImportFile( file_name, aiProcess_Triangulate );
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
	const aiMesh *mesh = scene->mMeshes[0];
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
	GLfloat *points = NULL;		 // array of vertex points
	GLfloat *normals = NULL;	 // array of vertex normals
	GLfloat *texcoords = NULL; // array of texture coordinates
	GLint *bone_ids = NULL;		 // array of bone IDs
	if ( mesh->HasPositions() ) {
		points = (GLfloat *)malloc( *point_count * 3 * sizeof( GLfloat ) );
		for ( int i = 0; i < *point_count; i++ ) {
			const aiVector3D *vp = &( mesh->mVertices[i] );
			points[i * 3] = (GLfloat)vp->x;
			points[i * 3 + 1] = (GLfloat)vp->y;
			points[i * 3 + 2] = (GLfloat)vp->z;
		}
	}
	if ( mesh->HasNormals() ) {
		normals = (GLfloat *)malloc( *point_count * 3 * sizeof( GLfloat ) );
		for ( int i = 0; i < *point_count; i++ ) {
			const aiVector3D *vn = &( mesh->mNormals[i] );
			normals[i * 3] = (GLfloat)vn->x;
			normals[i * 3 + 1] = (GLfloat)vn->y;
			normals[i * 3 + 2] = (GLfloat)vn->z;
		}
	}
	if ( mesh->HasTextureCoords( 0 ) ) {
		texcoords = (GLfloat *)malloc( *point_count * 2 * sizeof( GLfloat ) );
		for ( int i = 0; i < *point_count; i++ ) {
			const aiVector3D *vt = &( mesh->mTextureCoords[0][i] );
			texcoords[i * 2] = (GLfloat)vt->x;
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
		bone_ids = (int *)malloc( *point_count * sizeof( int ) );

		for ( int b_i = 0; b_i < *bone_count; b_i++ ) {
			const aiBone *bone = mesh->mBones[b_i];

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
				int vertex_id = (int)weight.mVertexId;
				// ignore weight if less than 0.5 factor
				if ( weight.mWeight >= 0.5f ) {
					bone_ids[vertex_id] = b_i;
				}
			}

		} // endfor

		/* get the skeleton hierarchy from a separate AssImp data structure */

		// there should always be a 'root node', even if no skeleton exists
		aiNode *assimp_node = scene->mRootNode;

		if ( !import_skeleton_node( assimp_node, root_node, *bone_count,
																bone_names ) ) {
			fprintf( stderr, "ERROR: could not import node tree from mesh\n" );
		} // endif

		/* get the first animation out and into keys */
		if ( scene->mNumAnimations > 0 ) {
			// get just the first animation
			aiAnimation *anim = scene->mAnimations[0];
			printf( "animation name: %s\n", anim->mName.C_Str() );
			printf( "animation has %i node channels\n", anim->mNumChannels );
			printf( "animation has %i mesh channels\n", anim->mNumMeshChannels );
			printf( "animation duration %f\n", anim->mDuration );
			printf( "ticks per second %f\n", anim->mTicksPerSecond );

			*anim_duration = anim->mDuration;
			printf( "anim duration is %f\n", anim->mDuration );

			// get the node channels
			for ( int i = 0; i < (int)anim->mNumChannels; i++ ) {
				aiNodeAnim *chan = anim->mChannels[i];
				// find the matching node in our skeleton by name
				Skeleton_Node *sn =
					find_node_in_skeleton( *root_node, chan->mNodeName.C_Str() );
				if ( !sn ) {
					fprintf( stderr, "WARNING: did not find node named %s in skeleton."
													 "animation broken.\n",
									 chan->mNodeName.C_Str() );
					continue;
				}

				sn->num_pos_keys = chan->mNumPositionKeys;
				sn->num_rot_keys = chan->mNumRotationKeys;
				sn->num_sca_keys = chan->mNumScalingKeys;

				// allocate memory
				sn->pos_keys = (vec3 *)malloc( sizeof( vec3 ) * sn->num_pos_keys );
				sn->rot_keys = (versor *)malloc( sizeof( versor ) * sn->num_rot_keys );
				sn->sca_keys = (vec3 *)malloc( sizeof( vec3 ) * sn->num_sca_keys );
				sn->pos_key_times = (double *)malloc( sizeof( double ) * sn->num_pos_keys );
				sn->rot_key_times = (double *)malloc( sizeof( double ) * sn->num_rot_keys );
				sn->sca_key_times = (double *)malloc( sizeof( double ) * sn->num_sca_keys );

				// add position keys to node
				for ( int i = 0; i < sn->num_pos_keys; i++ ) {
					aiVectorKey key = chan->mPositionKeys[i];
					sn->pos_keys[i].v[0] = key.mValue.x;
					sn->pos_keys[i].v[1] = key.mValue.y;
					sn->pos_keys[i].v[2] = key.mValue.z;
					// TODO -- forgot this
					sn->pos_key_times[i] = key.mTime;
				}
				// add rotation keys to node
				for ( int i = 0; i < sn->num_rot_keys; i++ ) {
					aiQuatKey key = chan->mRotationKeys[i];
					sn->rot_keys[i].q[0] = key.mValue.w;
					sn->rot_keys[i].q[1] = key.mValue.x;
					sn->rot_keys[i].q[2] = key.mValue.y;
					sn->rot_keys[i].q[3] = key.mValue.z;
					sn->rot_key_times[i] = key.mTime;
				}
				// add scaling keys to node
				for ( int i = 0; i < sn->num_sca_keys; i++ ) {
					aiVectorKey key = chan->mScalingKeys[i];
					sn->sca_keys[i].v[0] = key.mValue.x;
					sn->sca_keys[i].v[1] = key.mValue.y;
					sn->sca_keys[i].v[2] = key.mValue.z;
					sn->sca_key_times[i] = key.mTime;
				} // endfor
			}		// endfor mNumChannels
		} else {
			fprintf( stderr, "WARNING: no animations found in mesh file\n" );
		} // endif mNumAnimations > 0

	} // endif hasbones

	/* copy mesh data into VBOs */
	if ( mesh->HasPositions() ) {
		GLuint vbo;
		glGenBuffers( 1, &vbo );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glBufferData( GL_ARRAY_BUFFER, 3 * *point_count * sizeof( GLfloat ), points,
									GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		free( points );
	}
	if ( mesh->HasNormals() ) {
		GLuint vbo;
		glGenBuffers( 1, &vbo );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glBufferData( GL_ARRAY_BUFFER, 3 * *point_count * sizeof( GLfloat ), normals,
									GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
		free( normals );
	}
	if ( mesh->HasTextureCoords( 0 ) ) {
		GLuint vbo;
		glGenBuffers( 1, &vbo );
		glBindBuffer( GL_ARRAY_BUFFER, vbo );
		glBufferData( GL_ARRAY_BUFFER, 2 * *point_count * sizeof( GLfloat ), texcoords,
									GL_STATIC_DRAW );
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
		glBufferData( GL_ARRAY_BUFFER, *point_count * sizeof( GLint ), bone_ids,
									GL_STATIC_DRAW );
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
	glEnable( GL_DEPTH_TEST ); // enable depth-testing
	glDepthFunc( GL_LESS );		 // depth-testing interprets a smaller value as "closer"
	glEnable( GL_CULL_FACE );	// cull face
	glCullFace( GL_BACK );		 // cull back face
	glFrontFace( GL_CCW ); // set counter-clock-wise vertex order to mean the front
	glClearColor( 0.2, 0.2, 0.2, 1.0 ); // grey background to help spot mistakes
	glViewport( 0, 0, g_gl_width, g_gl_height );

	/* load the mesh using assimp */
	GLuint monkey_vao;
	mat4 monkey_bone_offset_matrices[MAX_BONES];
	mat4 monkey_bone_animation_mats[MAX_BONES];
	for ( int i = 0; i < MAX_BONES; i++ ) {
		monkey_bone_animation_mats[i] = identity_mat4();
		monkey_bone_offset_matrices[i] = identity_mat4();
		monkey_bone_animation_mats[i] = identity_mat4();
	}
	int monkey_point_count = 0;
	int monkey_bone_count = 0;
	Skeleton_Node *monkey_root_node = NULL;
	double monkey_anim_duration = 0.0;
	( load_mesh( MESH_FILE, &monkey_vao, &monkey_point_count,
										 monkey_bone_offset_matrices, &monkey_bone_count,
										 &monkey_root_node, &monkey_anim_duration ) );
	printf( "monkey bone count %i\n", monkey_bone_count );

	/* create a buffer of bone positions for visualising the bones */
	float bone_positions[3 * 256];
	int c = 0;
	for ( int i = 0; i < monkey_bone_count; i++ ) {
		// print (monkey_bone_offset_matrices[i]);

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
	glBufferData( GL_ARRAY_BUFFER, 3 * monkey_bone_count * sizeof( float ),
								bone_positions, GL_STATIC_DRAW );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
	glEnableVertexAttribArray( 0 );

	/*-------------------------------CREATE
	 * SHADERS-------------------------------*/
	GLuint shader_programme =
		create_programme_from_files( VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE );
	GLuint bones_shader_programme =
		create_programme_from_files( "bones.vert", "bones.frag" );

#define ONE_DEG_IN_RAD ( 2.0 * M_PI ) / 360.0 // 0.017444444
	// input variables
	float near = 0.1f;									// clipping plane
	float far = 100.0f;									// clipping plane
	float fov = 67.0f * ONE_DEG_IN_RAD; // convert 67 degrees to radians
	float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
	// matrix components
	float inverse_range = 1.0f / tan( fov * 0.5f );
	float Sx = inverse_range / aspect;
	float Sy = inverse_range;
	float Sz = -( far + near ) / ( far - near );
	float Pz = -( 2.0f * far * near ) / ( far - near );
	GLfloat proj_mat[] = { Sx,	 0.0f, 0.0f, 0.0f,	0.0f, Sy,		0.0f, 0.0f,
												 0.0f, 0.0f, Sz,	 -1.0f, 0.0f, 0.0f, Pz,		0.0f };

	float cam_speed = 5.0f;			 // 1 unit per second
	float cam_yaw_speed = 40.0f; // 10 degrees per second
	float cam_pos[] = { 0.0f, 0.0f,
											5.0f }; // don't start at zero, or we will be too close
	float cam_yaw = 0.0f;				// y-rotation in degrees
	mat4 T =
		translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1], -cam_pos[2] ) );
	mat4 R = rotate_y_deg( identity_mat4(), -cam_yaw );
	mat4 view_mat = R * T;

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
		glUniformMatrix4fv( bone_matrices_locations[i], 1, GL_FALSE,
												identity_mat4().m );
	}

	glUseProgram( bones_shader_programme );
	int bones_view_mat_location =
		glGetUniformLocation( bones_shader_programme, "view" );
	glUniformMatrix4fv( bones_view_mat_location, 1, GL_FALSE, view_mat.m );
	int bones_proj_mat_location =
		glGetUniformLocation( bones_shader_programme, "proj" );
	glUniformMatrix4fv( bones_proj_mat_location, 1, GL_FALSE, proj_mat );

	double anim_time = 0.0;

	while ( !glfwWindowShouldClose( g_window ) ) {
		static double previous_seconds = glfwGetTime();
		double current_seconds = glfwGetTime();
		double elapsed_seconds = current_seconds - previous_seconds;
		previous_seconds = current_seconds;

		/* update animation timer and loop */
		anim_time += elapsed_seconds * 0.5;
		if ( anim_time >= monkey_anim_duration ) {
			anim_time = monkey_anim_duration - anim_time;
		}

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
			mat4 T = translate( identity_mat4(), vec3( -cam_pos[0], -cam_pos[1],
																								 -cam_pos[2] ) ); // cam translation
			mat4 R = rotate_y_deg( identity_mat4(), -cam_yaw );					//
			mat4 view_mat = R * T;
			glUseProgram( shader_programme );
			glUniformMatrix4fv( view_mat_location, 1, GL_FALSE, view_mat.m );
			glUseProgram( bones_shader_programme );
			glUniformMatrix4fv( bones_view_mat_location, 1, GL_FALSE, view_mat.m );
		}
		skeleton_animate( monkey_root_node, anim_time, identity_mat4(),
											monkey_bone_offset_matrices, monkey_bone_animation_mats );
		glUseProgram( shader_programme );
		glUniformMatrix4fv( bone_matrices_locations[0], monkey_bone_count, GL_FALSE,
												monkey_bone_animation_mats[0].m );

		if ( GLFW_PRESS == glfwGetKey( g_window, GLFW_KEY_ESCAPE ) ) {
			glfwSetWindowShouldClose( g_window, 1 );
		}
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers( g_window );
	}

	// close GL context and any other GLFW resources
	glfwTerminate();
	return 0;
}
