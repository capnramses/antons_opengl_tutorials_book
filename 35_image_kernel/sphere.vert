#version 410

in vec3 vp;
uniform mat4 P, V;

void main () {
	gl_Position = P * V * vec4 (vp, 1.0);
}
