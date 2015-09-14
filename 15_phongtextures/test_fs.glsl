/* NOTE: this shader is for GLSL 4.2.0 (OpenGL 4.2)
to convert it to an earlier version, you'll need to remove
layout (binding = x) for each texture, and instead explicitly
set glUniform1i() for each texture in C with these values */

#version 420
//#version 410

in vec3 pos_eye;
in vec3 norm_eye;
in vec2 st;


layout (binding = 0) uniform sampler2D diffuse_map;
layout (binding = 1) uniform sampler2D specular_map;
layout (binding = 2) uniform sampler2D ambient_map;
layout (binding = 3) uniform sampler2D emission_map;
/*
uniform sampler2D diffuse_map;
uniform sampler2D specular_map;
uniform sampler2D ambient_map;
uniform sampler2D emission_map;
*/
uniform mat4 view;

out vec4 frag_colour;

vec3 light_position_world = vec3 (1.0, 1.0, 10.0);
vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
vec3 Ld = vec3 (0.7, 0.7, 0.7); // dull white diffuse light colour
vec3 La = vec3 (0.2, 0.2, 0.2); // grey ambient colour
float specular_exponent = 100.0; // specular 'power'

void main() {
	vec3 light_pos_eye = (view * vec4 (light_position_world, 1.0)).xyz;

	vec3 Ka = texture (ambient_map, st).rgb;
	vec3 Ia = vec3 (0.2, 0.2, 0.2) * Ka;

	vec4 texel = texture (diffuse_map, st);
	vec3 Kd = texel.rgb;
	vec3 surface_to_light_eye = normalize (light_pos_eye - pos_eye);
	float dp = max (0.0, dot (norm_eye, surface_to_light_eye));
	vec3 Id = Kd * Ld * dp; 

	vec3 Ks = texture (specular_map, st).rgb;
	vec3 surface_to_viewer_eye = normalize (-pos_eye);
	vec3 half_way_eye = normalize (surface_to_viewer_eye + surface_to_light_eye);
	float dot_prod_specular = max (dot (half_way_eye, norm_eye), 0.0);
	float specular_factor = pow (dot_prod_specular, specular_exponent);
	vec3 Is = Ls * Ks * specular_factor; // final specular intensity

	vec3 texel_e = texture (emission_map, st).rgb;

	frag_colour = vec4 (Id + Is + Ia + texel_e, 1.0);
	//frag_colour= vec4 (Ka, 1.0);
}
