#version 410

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec3 vertex_normal;
layout(location = 2) in vec2 texture_coord;

uniform mat4 view, proj;
uniform float time;

out vec3 normal;
out vec3 pos_eye;

void main() {
	pos_eye = (view * vec4 (vertex_position, 1.0)).xyz;
	pos_eye.z += sin (time);
	normal = vertex_normal;
	gl_Position = proj * vec4 (pos_eye, 1.0);
}
