#version 400

in vec2 texture_coordinates;
uniform sampler2D basic_texture;
out vec4 frag_colour;

void main() {
	vec4 texel = texture (basic_texture, texture_coordinates);
	frag_colour = texel;
	frag_colour.rgb = pow (frag_colour.rgb, vec3 (0.45, 0.45, 0.45));
}
