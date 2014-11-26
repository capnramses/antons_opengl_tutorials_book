#version 410

in vec3 position_eye, normal_eye;

uniform mat4 view_mat;

// fixed point light properties
vec3 light_position_world  = vec3 (0.0, 0.0, 2.0);
vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
vec3 Ld = vec3 (0.7, 0.7, 0.7); // dull white diffuse light colour
vec3 La = vec3 (0.2, 0.2, 0.2); // grey ambient colour
  
// surface reflectance
vec3 Ks = vec3 (1.0, 1.0, 1.0); // fully reflect specular light
vec3 Kd = vec3 (1.0, 0.5, 0.0); // orange diffuse surface reflectance
vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light
float specular_exponent = 1000.0; // specular 'power'

out vec4 fragment_colour; // final colour of surface

void main () {
	// ambient intensity
	vec3 Ia = La * Ka;

	/* Point Light */
	
	vec3 light_position_eye = vec3 (view_mat * vec4 (light_position_world, 1.0));
	vec3 distance_to_light_eye = light_position_eye - position_eye;
	vec3 direction_to_light_eye = normalize (distance_to_light_eye);
	

	/* Directional Light */
	
	/*vec3 light_direction_wor = normalize (vec3 (-0.5, 0.0, -1.0));
	vec3 light_direction_eye = (view_mat * vec4 (light_direction_wor, 0.0)).xyz;
	vec3 direction_to_light_eye = -light_direction_eye;*/
	
	
	/* Spotlight */
	vec3 spot_direction = normalize (vec3 (0.5, 0.0, -1.0));
	const float spot_arc = 1.0 - 5.0 / 90.0;
	float spot_dot = dot (spot_direction, -direction_to_light_eye);
	/*float spot_factor = 1.0;
	if (spot_dot < spot_arc) {
		spot_factor = 0.0;
	}*/
	
	float spot_factor = (spot_dot - spot_arc) / (1.0 - spot_arc);
	spot_factor = clamp (spot_factor, 0.0, 1.0);
	
	// diffuse intensity
	float dot_prod = dot (direction_to_light_eye, normal_eye);
	dot_prod = max (dot_prod, 0.0);
	vec3 Id = Ld * Kd * dot_prod; // final diffuse intensity
	Id *= spot_factor;
	
	// specular intensity
	vec3 surface_to_viewer_eye = normalize (-position_eye);
	
	//vec3 reflection_eye = reflect (-direction_to_light_eye, normal_eye);
	//float dot_prod_specular = dot (reflection_eye, surface_to_viewer_eye);
	//dot_prod_specular = max (dot_prod_specular, 0.0);
	//float specular_factor = pow (dot_prod_specular, specular_exponent);
	
	// blinn
	vec3 half_way_eye = normalize (surface_to_viewer_eye + direction_to_light_eye);
	float dot_prod_specular = max (dot (half_way_eye, normal_eye), 0.0);
	float specular_factor = pow (dot_prod_specular, specular_exponent);
	
	vec3 Is = Ls * Ks * specular_factor; // final specular intensity
	Is *= spot_factor;
	
	// final colour
	fragment_colour = vec4 (Is + Id + Ia, 1.0);
	//fragment_colour.rgb = spot_factor;
}
