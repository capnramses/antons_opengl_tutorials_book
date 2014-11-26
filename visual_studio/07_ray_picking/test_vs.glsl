#version 410

layout(location = 0) in vec3 vertex_position;
uniform mat4 model, view, proj;
// use z position to shader darker to help perception of distance
out float dist;

void main() {
	gl_Position = proj * view * model * vec4 (vertex_position, 1.0);
	dist = vertex_position.z;//1.0 - (-pos_eye.z / 10.0);
}
