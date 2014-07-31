#version 400

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 texture_coord;
layout(location = 3) in int bone_id;

uniform mat4 model, view, proj;
// a deformation matrix for each bone:
uniform mat4 bone_matrices[64];

out vec3 normal;
out vec2 st;
out vec3 colour;

void main() {
	colour = vec3 (0.0, 0.0, 0.0);
	if (bone_id == 0) {
		colour.r = 1.0;
	} else if (bone_id == 1) {
		colour.g = 1.0;
	} else if (bone_id == 2) {
		colour.b = 1.0;
	}

	st = texture_coord;
	normal = vertex_normal;
	gl_Position = proj * view * bone_matrices[bone_id] * vec4 (vertex_position, 1.0);
}
