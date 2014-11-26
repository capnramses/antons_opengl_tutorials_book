#version 410

in vec3 vp_loc;
out vec3 controlpoint_wor;

void main() {
	controlpoint_wor = vp_loc; // control points out == vertex points in
}

