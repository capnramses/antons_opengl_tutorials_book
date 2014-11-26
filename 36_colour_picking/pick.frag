#version 410
uniform vec3 unique_id;
out vec4 frag_colour;

void main () {
  frag_colour = vec4 (unique_id, 1.0);
}
