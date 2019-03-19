// Supplementary demo code for Anton's OpenGL 4 Tutorials
// Copyright Anton Gerdelan <antongdl@protonmail.com> 2019
// Article: http://antongerdelan.net/opengl/shader_hot_reload.html
// This code is GLSL.

#version 410

in vec3 a_vertex_position;

void main() {
  gl_Position = vec4( a_vertex_position, 1.0 );
}
