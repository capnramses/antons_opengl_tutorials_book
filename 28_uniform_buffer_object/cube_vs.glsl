#version 400

in vec3 vp;
//uniform mat4 P, V;
uniform mat4 cam_R;
out vec3 texcoords;

/* virtual camera uniforms */
layout (std140) uniform cam_block {
	mat4 P;
	mat4 V; // not used by cube - it has it's own view matrix cam_R
};

void main () {
	texcoords = vp;
	gl_Position = P * cam_R * vec4 (vp, 1.0);
}
