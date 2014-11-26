#version 410

layout(location = 0) in vec3 vp; // positions from mesh
layout(location = 1) in vec3 vn; // normals from mesh
uniform mat4 M; // model matrix

/* virtual camera uniforms */
layout (std140) uniform cam_block {
	mat4 P;
	mat4 V;
};

out vec3 pos_eye;
out vec3 n_eye;

void main () {
	pos_eye = vec3 (V * M * vec4 (vp, 1.0));
	n_eye = vec3 (V * M * vec4 (vn, 0.0));
	gl_Position = P * V * M * vec4 (vp, 1.0);
}
