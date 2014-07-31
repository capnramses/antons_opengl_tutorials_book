#version 400

in vec3 normal;
in vec3 pos_eye;
out vec4 frag_colour;

// required fog variables
const vec3 fog_colour = vec3 (0.2, 0.2, 0.2);
const float min_fog_radius = 2.0;
const float max_fog_radius = 5.0;

void main() {
	frag_colour = vec4 (normal, 1.0);
	
	// work out distance from camera to point
	float dist = length (-pos_eye);
	// get a fog factor (thickness of fog) based on the distance
	float fog_fac = (dist - min_fog_radius) / (max_fog_radius - min_fog_radius);
	// constrain the fog factor between 0 and 1
	fog_fac = clamp (fog_fac, 0.0, 1.0);

	// blend the fog colour with the lighting colour, based on the fog factor
	frag_colour.rgb = mix (frag_colour.rgb, fog_colour, fog_fac);
}
