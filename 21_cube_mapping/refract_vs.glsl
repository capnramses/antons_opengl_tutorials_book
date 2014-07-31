#version 400

layout(location = 0) in vec3 vp; // positions from mesh
layout(location = 1) in vec3 vn; // normals from mesh
uniform mat4 P, V, M; // proj, view, model matrices
out vec3 pos_eye;
out vec3 n_eye;

void main () {
	pos_eye = vec3 (V * M * vec4 (vp, 1.0));
	n_eye = vec3 (V * M * vec4 (vn, 0.0));
	gl_Position = P * V * M * vec4 (vp, 1.0);
}
