#version 410

in vec3 colour; // i made this up in the tess. evaluation shader
out vec4 fragcolour;

void main () {
	fragcolour = vec4 (colour, 1.0);
}
