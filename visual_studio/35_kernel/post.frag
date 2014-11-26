#version 410

// texture coordinates from vertex shaders
in vec2 st;

// texture sampler
uniform sampler2D tex;

// size of 1 pixel in texture coordinates
uniform vec2 pixel_scale = vec2 (1.0 / 800.0, 1.0 / 800.0);

// output fragment colour RGBA
out vec4 frag_colour;

// Gaussian kernel weights
#define KERNEL_SIZE 25
float kernel_weights[] = float[]( 
	0.00048031, 0.00500493, 0.01093176, 0.00500493, 0.00048031,
	0.00500493, 0.05215252, 0.11391157, 0.05215252, 0.00500493,
	0.01093176, 0.11391157, 0.24880573, 0.11391157, 0.01093176,
	0.00500493, 0.05215252, 0.11391157, 0.05215252, 0.00500493,
	0.00048031, 0.00500493, 0.01093176, 0.00500493, 0.00048031
);
float weights_factor = 1.01238;

void main () {
	vec2 offset[] = vec2[](
		vec2 (-pixel_scale.s * 2.0, -pixel_scale.t * 2.0),
		vec2 (-pixel_scale.s, -pixel_scale.t * 2.0),
		vec2 (0.0, -pixel_scale.t * 2.0),
		vec2 (pixel_scale.s, -pixel_scale.t * 2.0),
		vec2 (pixel_scale.s * 2.0, -pixel_scale.t * 2.0),
	
		vec2 (-pixel_scale.s * 2.0, -pixel_scale.t),
		vec2 (-pixel_scale.s, -pixel_scale.t),
		vec2 (0.0, -pixel_scale.t),
		vec2 (pixel_scale.s, -pixel_scale.t),
		vec2 (pixel_scale.s * 2.0, -pixel_scale.t),
	
		vec2 (-pixel_scale.s * 2.0, 0.0),
		vec2 (-pixel_scale.s, 0.0),
		vec2 (0.0, 0.0),
		vec2 (pixel_scale.s, 0.0),
		vec2 (pixel_scale.s * 2.0, 0.0),
	
		vec2 (-pixel_scale.s * 2.0, pixel_scale.t),
		vec2 (-pixel_scale.s, pixel_scale.t),
		vec2 (0.0, pixel_scale.t),
		vec2 (pixel_scale.s, pixel_scale.t),
		vec2 (pixel_scale.s * 2.0, pixel_scale.t),
	
		vec2 (-pixel_scale.s * 2.0, pixel_scale.t * 2.0),
		vec2 (-pixel_scale.s, pixel_scale.t * 2.0),
		vec2 (0.0, pixel_scale.t * 2.0),
		vec2 (pixel_scale.s, pixel_scale.t * 2.0),
		vec2 (pixel_scale.s * 2.0, pixel_scale.t * 2.0)
	);

	vec3 colour;
	// only blur rhs for comparison
	if (st.s >= 0.5) {
		for (int i = 0; i < KERNEL_SIZE; i++) {
			colour += texture (tex, st + offset[i]).rgb * kernel_weights[i] * weights_factor;
		}
	} else {
		colour = texture(tex, st).rgb;
	}
	frag_colour = vec4 (colour, 1.0);
}
