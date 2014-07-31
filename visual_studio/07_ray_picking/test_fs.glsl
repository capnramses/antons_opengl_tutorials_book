#version 400

in float dist;
uniform float blue = 0.0;
out vec4 frag_colour;

void main() {
	frag_colour = vec4 (1.0, 0.0, blue, 1.0);
	// use z position to shader darker to help perception of distance
	frag_colour.xyz *= dist;
}
