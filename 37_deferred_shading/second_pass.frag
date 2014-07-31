#version 400

uniform mat4 V;
uniform sampler2D p_tex;
uniform sampler2D n_tex;
uniform vec3 ls;
uniform vec3 ld;
uniform vec3 lp;

out vec4 frag_colour;

vec3 kd = vec3 (0.9, 0.9, 0.9);
vec3 ks = vec3 (0.5, 0.5, 0.5);
float specular_exponent = 200.0;

vec3 phong (in vec3 op_eye, in vec3 n_eye) {
	vec3 lp_eye = (V * vec4 (lp, 1.0)).xyz;
	vec3 dist_to_light_eye = lp_eye - op_eye;
	vec3 direction_to_light_eye = normalize (dist_to_light_eye);

	// standard diffuse light
	float dot_prod = max (dot (direction_to_light_eye,  n_eye), 0.0);
	
	vec3 Id = ld * kd * dot_prod; // final diffuse intensity

	// standard specular light
	vec3 reflection_eye = reflect (-direction_to_light_eye, n_eye);
	vec3 surface_to_viewer_eye = normalize (-op_eye);
	float dot_prod_specular = dot (reflection_eye, surface_to_viewer_eye);
	dot_prod_specular = max (dot_prod_specular, 0.0);
	float specular_factor = pow (dot_prod_specular, specular_exponent);
	vec3 Is = ls * ks * specular_factor; // final specular intensity
	
	float dist_2d = max (0.0, 1.0 - distance (lp_eye, op_eye) / 10.0);
	float atten_factor =  dist_2d;
	
//	return vec3(dist_2d,dist_2d,dist_2d);
	
	return (Id + Is) * atten_factor;
}

void main () {
	frag_colour.a = 1.0;
	
	vec2 st;
	st.s = gl_FragCoord.x / 800.0;
	st.t = gl_FragCoord.y / 800.0;
	vec4 p_texel = texture (p_tex, st);
	
	// skip background
	if (p_texel.z > -0.0001) {
		discard;
	}
	
	vec4 n_texel = texture (n_tex, st);
	
	frag_colour.rgb = phong (p_texel.rgb, normalize (n_texel.rgb));
	
	//	frag_colour.rgb = p_texel.rgb;
}
