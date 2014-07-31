

#include "obj_parser.h"
#include "gl_utils.h"
#include "maths_funcs.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>

#define FIRST_PASS_VS "first_pass.vert"
#define FIRST_PASS_FS "first_pass.frag"
#define SECOND_PASS_VS "second_pass.vert"
#define SECOND_PASS_FS "second_pass.frag"
#define SPHERE_FILE "sphere.obj"
#define PLANE_FILE "plane.obj"
#define NUM_LIGHTS 64

int g_gl_width = 800;
int g_gl_height = 800;
GLFWwindow* g_window;

/* the framebuffer used for write-to-G-buffer */
GLuint g_fb;
GLuint g_fb_tex_p; /* G-buffer hog_L_ding positions in a texture */
GLuint g_fb_tex_n; /* G-buffer hog_L_ding normag_L_s in a texture */
/* 3d sphere representing light coverage area */
GLuint g_sphere_vao;
int g_sphere_point_count;
/* 3d rectangle that will have light cast onto it */
GLuint g_plane_vao;
int g_plane_point_count;
/* shader for collecting the G-buffer */
GLuint g_first_pass_sp;
GLint g_first_pass_P_loc; /* Projection matrix uniform location */
GLint g_first_pass_V_loc; /* View matrix uniform location */
GLint g_first_pass_M_loc; /* Model matrix uniform location */
/* shader for rendering the deferred pass */
GLuint g_second_pass_sp;
GLint g_second_pass_P_loc = -1;
GLint g_second_pass_V_loc = -1;
GLint g_second_pass_M_loc = -1; /* Model matrix uniform location */
GLint g_second_pass_L_p_loc = -1; /* light position uniform location */
GLint g_second_pass_L_d_loc = -1; /* light diffuse colour uniform location */
GLint g_second_pass_L_s_loc = -1; /* light specular colour uniform location */
GLint g_second_pass_p_tex_loc = -1; /* positions texture uniform location */
GLint g_second_pass_n_tex_loc = -1; /* normals texture uniform location */

/* objects to be lit. model matrices */
mat4 g_plane_M;

/* light world positions and model matrices */
vec3 g_L_p[NUM_LIGHTS];
mat4 g_L_M[NUM_LIGHTS];
/* light colours */
vec3 g_L_d[NUM_LIGHTS];
vec3 g_L_s[NUM_LIGHTS];

/* virtual camera projection and view matrices */
mat4 g_P;
mat4 g_V;


/* this function creates the first-pass framebuffer that will write to textures
comprising the g-buffer, which contain normag_L_s and positions for each pixel on
the framebuffer. we will attach a renderbuffer which allows us to depth sort
and write the front-most geometry data into each texture's texel */
bool init_fb () {
	glGenFramebuffers (1, &g_fb);
	glGenTextures (1, &g_fb_tex_p);
	glBindTexture (GL_TEXTURE_2D, g_fb_tex_p);
	/* note 16-bit float RGB format used for positions */
	glTexImage2D (
		GL_TEXTURE_2D,
		0,
		GL_RGB32F,
		g_gl_width,
		g_gl_height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		NULL
	);
	/* no bi-linear filtering required because texture same size as viewport */
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	/* generate output texture to hog_L_d normag_L_s */
	glGenTextures (1, &g_fb_tex_n);
	glBindTexture (GL_TEXTURE_2D, g_fb_tex_n);
	/* you coug_L_d get away with less than 16-bit precision here for normag_L_s */
	glTexImage2D (
		GL_TEXTURE_2D,
		0,
		GL_RGB16F,
		g_gl_width,
		g_gl_height,
		0,
		GL_RGB,
		GL_UNSIGNED_BYTE,
		NULL
	);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	/* depth texture instead of a renderbuffer */
	GLuint depth_tex;
	glGenTextures (1, &depth_tex);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, depth_tex);
	glTexImage2D (
		GL_TEXTURE_2D,
		0,
		GL_DEPTH_COMPONENT32F,
		g_gl_width,
		g_gl_height,
		0,
		GL_DEPTH_COMPONENT,
		GL_UNSIGNED_BYTE,
		NULL
	);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// attach depth texture to framebuffer
	glFramebufferTexture2D (
		GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0);

	/* attach textures to framebuffer. the attachment numbers 0 and 1 don't
	automatically corresponed to frament shader output locations 0 and 1, so we
	specify that afterwards */
	glBindFramebuffer (GL_FRAMEBUFFER, g_fb);
	glFramebufferTexture2D (
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_fb_tex_p, 0
	);
	glFramebufferTexture2D (
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_fb_tex_n, 0
	);
	/* the first item in this array matches fragment shader output location 0 */
	GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers (2, draw_bufs);

	/* validate g_fb and return false on error */
	GLenum status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
	if (GL_FRAMEBUFFER_COMPLETE != status) {
		fprintf (stderr, "ERROR: incomplete framebuffer\n");
		return false;
	}
	return true;
}

/* load the ground plane */
bool load_plane () {
	float* points = NULL;
	float* tex_coords = NULL;
	float* normals = NULL;
	if (!load_obj_file (
		PLANE_FILE,
		points,
		tex_coords,
		normals,
		g_plane_point_count
	)) {
		fprintf (stderr, "ERROR loading plane mesh %s\n", PLANE_FILE);
		return false;
	}
	// create vao
	glGenVertexArrays (1, &g_plane_vao);
	glBindVertexArray (g_plane_vao);
	
	GLuint points_vbo;
	glGenBuffers (1, &points_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, points_vbo);
	glBufferData (
		GL_ARRAY_BUFFER,
		g_plane_point_count * 3 * sizeof (GLfloat),
		points,
		GL_STATIC_DRAW
	);
	
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (0);
	
	GLuint normals_vbo;
	glGenBuffers (1, &normals_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, normals_vbo);
	glBufferData (
		GL_ARRAY_BUFFER,
		g_plane_point_count * 3 * sizeof (GLfloat),
		normals,
		GL_STATIC_DRAW
	);
	
	glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (1);
	return true;
}

/* load spherical mesh used for coverage area of each light */
bool load_sphere () {
	float* sphere_points = NULL;
	float* sphere_tex_coords = NULL;
	float* sphere_normals = NULL;
	if (!load_obj_file (
		SPHERE_FILE,
		sphere_points,
		sphere_tex_coords,
		sphere_normals,
		g_sphere_point_count
	)) {
		fprintf (stderr, "ERROR loading sphere mesh %s\n", SPHERE_FILE);
		return false;
	}
	// create vao
	glGenVertexArrays (1, &g_sphere_vao);
	glBindVertexArray (g_sphere_vao);
	
	GLuint points_vbo;
	glGenBuffers (1, &points_vbo);
	glBindBuffer (GL_ARRAY_BUFFER, points_vbo);
	glBufferData (
		GL_ARRAY_BUFFER,
		g_sphere_point_count * 3 * sizeof (GLfloat),
		sphere_points,
		GL_STATIC_DRAW
	);
	
	glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray (0);
	return true;
}

/* the first pass draws
* pixel positions
* pixel normals
* pixel depths
to 3 attached textures. */
void draw_first_pass () {
	glBindFramebuffer (GL_FRAMEBUFFER, g_fb);
	glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glDisable (GL_BLEND);
	glEnable (GL_DEPTH_TEST);
	glDepthMask (GL_TRUE);
	
	glUseProgram (g_first_pass_sp);
	glBindVertexArray (g_plane_vao);
	
	/* virtual camera matrices */
	glUniformMatrix4fv (g_first_pass_P_loc, 1, GL_FALSE, g_P.m);
	glUniformMatrix4fv (g_first_pass_V_loc, 1, GL_FALSE, g_V.m);
	
	glUniformMatrix4fv (g_first_pass_M_loc, 1, GL_FALSE, g_plane_M.m);
	glDrawArrays (GL_TRIANGLES, 0, g_plane_point_count);
}

/* the second pass
* retrieves pixel positions, normals, and depths
*/
void draw_second_pass () {
	glBindFramebuffer (GL_FRAMEBUFFER, 0);
	/* clear to any colour */
	glClearColor (0.2, 0.2, 0.2, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT);
	
	glEnable (GL_BLEND); // --- could reject background frags!
	glBlendEquation (GL_FUNC_ADD);
	glBlendFunc (GL_ONE, GL_ONE); // addition each time
	glDisable (GL_DEPTH_TEST);
	glDepthMask (GL_FALSE);
	
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, g_fb_tex_p);
	glActiveTexture (GL_TEXTURE1);
	glBindTexture (GL_TEXTURE_2D, g_fb_tex_n);
	
	glUseProgram (g_second_pass_sp);
	glBindVertexArray (g_sphere_vao);
	
	/* virtual camera matrices */
	glUniformMatrix4fv (g_second_pass_P_loc, 1, GL_FALSE, g_P.m);
	glUniformMatrix4fv (g_second_pass_V_loc, 1, GL_FALSE, g_V.m);
	
	for (int i = 0; i < NUM_LIGHTS; i++) {
		 /* world position */
		glUniform3f (g_second_pass_L_p_loc, g_L_p[i].v[0], g_L_p[i].v[1], g_L_p[i].v[2]);
		/* diffuse colour */
		glUniform3f (g_second_pass_L_d_loc, g_L_d[i].v[0], g_L_d[i].v[1], g_L_d[i].v[2]);
		/* specular colour */
		glUniform3f (g_second_pass_L_s_loc, g_L_s[i].v[0], g_L_s[i].v[1], g_L_s[i].v[2]);
		
		glUniformMatrix4fv (g_second_pass_M_loc, 1, GL_FALSE, g_L_M[i].m);
		glDrawArrays (GL_TRIANGLES, 0, g_sphere_point_count);
	}
}

int main () {
	/* initialise GL context and window */
	assert (restart_gl_log ());
	assert (start_gl ());
	/* initialise framebuffer and G-buffer */
	assert (init_fb ());
	/* load pre-pass shaders that write to the g-buffer */
	g_first_pass_sp = create_programme_from_files (
		FIRST_PASS_VS, FIRST_PASS_FS);
	g_first_pass_P_loc = glGetUniformLocation (g_first_pass_sp, "P");
	g_first_pass_V_loc = glGetUniformLocation (g_first_pass_sp, "V");
	g_first_pass_M_loc = glGetUniformLocation (g_first_pass_sp, "M");
	/* load screen-space pass shaders that read from the g-buffer */
	g_second_pass_sp = create_programme_from_files (
		SECOND_PASS_VS, SECOND_PASS_FS);
	g_second_pass_P_loc = glGetUniformLocation (g_second_pass_sp, "P");
	g_second_pass_V_loc = glGetUniformLocation (g_second_pass_sp, "V");
	g_second_pass_M_loc = glGetUniformLocation (g_second_pass_sp, "M");
	g_second_pass_L_p_loc = glGetUniformLocation (g_second_pass_sp, "lp");
	g_second_pass_L_d_loc = glGetUniformLocation (g_second_pass_sp, "ld");
	g_second_pass_L_s_loc = glGetUniformLocation (g_second_pass_sp, "ls");
	g_second_pass_p_tex_loc = glGetUniformLocation (g_second_pass_sp, "p_tex");
	g_second_pass_n_tex_loc = glGetUniformLocation (g_second_pass_sp, "n_tex");
	glUseProgram (g_second_pass_sp);
	glUniform1i (g_second_pass_p_tex_loc, 0);
	glUniform1i (g_second_pass_n_tex_loc, 1);
	
	/* object positions and matrices */
	assert (load_plane ());
	g_plane_M = scale (identity_mat4 (), vec3 (200.0f, 1.0f, 200.0f));
	g_plane_M = translate (g_plane_M, vec3 (0.0f, -2.0f, 0.0f));
	
	/* load sphere mesh */
	assert (load_sphere ());
	/* light positions and matrices */
	for (int i = 0; i < NUM_LIGHTS; i++) {
		float x = -sinf ((float)i * 0.5f) * 25.0f; // between +- 10 x
		float y = 2.0f;
		float z = (float)-i * 2.0f + 10.0; // 1 light every 2 meters away on z
		g_L_p[i] = vec3 (x, y, z);
	}
	
	float light_radius = 10.0f;
	int redi = 0;
	int bluei = 1;
	int greeni = 2;
	for (int i = 0; i < NUM_LIGHTS; i++) {
		g_L_M[i] = scale (identity_mat4 (),
			vec3 (light_radius, light_radius, light_radius));
		g_L_M[i] = translate (g_L_M[i], g_L_p[i]);
		/* cycle different colours for each of the lights */
		g_L_d[i] = vec3 (
			(float)((redi + 1) / 3),
			(float)((greeni + 1) / 3),
			(float)((bluei + 1) / 3)
		);
		g_L_s[i] = vec3 (1.0, 1.0, 1.0);
		redi = (redi + 1) % 3;
		bluei = (bluei + 1) % 3;
		greeni = (greeni + 1) % 3;
	}
	
	/* set up virtual camera */
	float aspect = (float)g_gl_width / (float)g_gl_height;
	float near = 0.1f;
	float far = 1000.0f;
	float fovy = 67.0f;
	g_P = perspective (fovy, aspect, near, far);
	vec3 up (0.0f, 1.0f, 0.0f);
	vec3 targ_pos (0.0f, 0.0f, 0.0f);
	vec3 cam_pos (0.0f, 30.0f, 30.0f);
	g_V = look_at (cam_pos, targ_pos, up);
	
	glViewport (0, 0, g_gl_width, g_gl_height);
	glEnable (GL_CULL_FACE); // cull face
	glCullFace (GL_BACK); // cull back face
	glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
	while (!glfwWindowShouldClose (g_window)) {
		_update_fps_counter (g_window);
		draw_first_pass ();
		draw_second_pass ();
		
		glfwSwapBuffers (g_window);
		glfwPollEvents ();
		if (GLFW_PRESS == glfwGetKey (g_window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose (g_window, 1);
		}
	}
	
	/* close GL context and any other GLFW resources */
	glfwTerminate();
	return 0;
}
