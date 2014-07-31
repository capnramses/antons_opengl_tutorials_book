#version 400

in vec3 p_eye;
in vec3 n_eye;

/* note that we should force the location number here by adding layout location
keywords */
layout (location = 0) out vec3 def_p;
layout (location = 1) out vec3 def_n;

out vec4 f;

void main () {
	//f = vec4 (depth,depth,depth, 1.0);
	def_p = p_eye;
	def_n = n_eye;
}
