#version 410

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 vt; // per-vertex texture co-ords

uniform mat4 view, proj;

out vec2 texture_coordinates;

void main() {
	texture_coordinates = vt;
	gl_Position = proj * view * vec4 (vertex_position, 1.0);
}
