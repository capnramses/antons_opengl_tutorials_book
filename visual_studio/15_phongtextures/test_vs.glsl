#version 420

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 texture_coord;

uniform mat4 view, proj;

out vec3 pos_eye;
out vec3 norm_eye; // positions and normals in eye space
out vec2 st; // texture coordinates passed through from from vertex shader

void main() {
	st = texture_coord;
	norm_eye = (view * vec4 (vertex_normal, 0.0)).xyz;;
	pos_eye = (view * vec4 (vertex_position, 1.0)).xyz;
	gl_Position = proj * vec4 (pos_eye, 1.0);
}
