#version 400

in vec2 vp;
out vec2 st;

void main () {
	gl_Position = vec4 (vp * 0.333 + 0.667, 0.0, 1.0);
	st = (vp + 1.0) * 0.5;
}
