#version 410

in vec2 st;
uniform sampler2D depth_tex;
out vec4 frag_colour;

void main () {
	float d = texture2D (depth_tex, st).r;
	frag_colour = vec4 (d, d, d, 1.0);
}
