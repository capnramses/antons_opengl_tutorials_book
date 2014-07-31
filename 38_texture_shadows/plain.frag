#version 400

// vertex points in light coordinate space
in vec4 st_shadow;
// the depth map
uniform sampler2D depth_map;
uniform vec3 colour;
uniform float shad_resolution = 2048.0;
out vec4 frag_colour;

float eval_shadow (vec4 texcoods) {
	// constant that you can use to slightly tweak the depth comparison
	float epsilon = 0.003;

	float shadow = texture (depth_map, texcoods.xy).r;
	
	if (shadow + epsilon < texcoods.z) {
		return 0.25; // shadowed
	}
	return 1.0; // not shadowed
}

void main() {
	frag_colour = vec4 (colour, 1.0);
	
	vec4 shad_coord = st_shadow;
	/* we compute this in frag shader otherwise we get errors from interpolation*/
	shad_coord.xyz /= shad_coord.w;
	shad_coord.xyz += 1.0;
	shad_coord.xyz *= 0.5;
	/* this is the original sampling without a filter */
	float shadow_factor = eval_shadow (shad_coord);
	
	/* this section is a very basic filter for the harsh edges
	update the resolution uniform if you change the shadow texture size though
	*/
	/*
	vec4 sc_a = shad_coord;
	vec4 sc_b = shad_coord;
	vec4 sc_c = shad_coord;
	vec4 sc_d = shad_coord;
	sc_a.x += 1.0 / shad_resolution;
	sc_b.x -= 1.0 / shad_resolution;
	sc_c.y += 1.0 / shad_resolution;
	sc_d.y -= 1.0 / shad_resolution;
	float shadow_factor_a = eval_shadow (sc_a);
	float shadow_factor_b = eval_shadow (sc_b);
	float shadow_factor_c = eval_shadow (sc_c);
	float shadow_factor_d = eval_shadow (sc_d);
	float shadow_factor = shadow_factor_a * 0.25 + shadow_factor_b * 0.25 +
		shadow_factor_c * 0.25 + shadow_factor_d * 0.25;*/
	
	
	frag_colour = vec4 (colour * shadow_factor, 1.0);
}
