#version 410

in vec2 texture_coordinates;
uniform sampler2D basic_texture;
uniform sampler2D second_texture;
out vec4 frag_colour;

void main() {
	vec4 texel_a = texture (basic_texture, texture_coordinates);
	vec4 texel_b = texture (second_texture, texture_coordinates);
	frag_colour = mix (texel_a, texel_b, texture_coordinates.s);
}
