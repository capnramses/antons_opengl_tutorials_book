#version 400

in vec3 pos_eye;
in vec3 n_eye;
uniform samplerCube cube_texture;
uniform mat4 V; // view matrix
out vec4 frag_colour;

void main () {
	/* reflect ray around normal from eye to surface */
	vec3 incident_eye = normalize (pos_eye);
	vec3 normal = normalize (n_eye);

	float ratio = 1.0 /1.3333;
	vec3 refracted = refract (incident_eye, normal, ratio);
	refracted = vec3 (inverse (V) * vec4 (refracted, 0.0));

	frag_colour = texture (cube_texture, refracted);
}
