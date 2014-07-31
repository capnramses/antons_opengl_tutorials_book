#version 400

in vec3 vp;
uniform mat4 proj, view;

void main () {
	gl_PointSize = 10.0;
	gl_Position = proj * view * vec4 (vp, 1.0);
}
