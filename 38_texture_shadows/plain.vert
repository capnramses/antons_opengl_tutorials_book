#version 400

layout(location = 0) in vec3 vertex_position;
uniform mat4 M, V, P;
/* view and projection matrices from the shadow caster (light source) */
uniform mat4 caster_P, caster_V;
// point in the light's space
out vec4 st_shadow;

void main() {
	gl_Position = P * V * M * vec4 (vertex_position, 1.0);
	
	/* create a shadow map texture coordinate by working out the position in the
	from the light's viewpoint (the same way we do in the depth-writing shader */
	st_shadow = caster_P * caster_V * M * vec4 (vertex_position, 1.0);
}
