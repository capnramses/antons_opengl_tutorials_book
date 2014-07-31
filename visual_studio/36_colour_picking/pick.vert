#version 400
in vec3 vp;

uniform mat4 P, V, M;

void main () {
  gl_Position = P * V * M * vec4 (vp, 1.0);
}
