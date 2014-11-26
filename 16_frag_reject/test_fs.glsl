#version 410

in vec2 texture_coordinates;
uniform sampler2D basic_texture;
out vec4 frag_colour;

void main() {
	vec4 texel = texture (basic_texture, texture_coordinates);
	frag_colour = texel;
	if (texel.r > 0.2 && texel.g + texel.b < 1.0) {
		discard;
	}
}
