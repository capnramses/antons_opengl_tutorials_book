#version 400

// number of CPs in patch
layout (vertices = 3) out;

// from VS (use empty modifier [] so we can say anything)
in vec3 controlpoint_wor[];

// to evluation shader. will be used to guide positioning of generated points
out vec3 evaluationpoint_wor[];

uniform float tess_fac_inner = 1.0; // controlled by keyboard buttons
uniform float tess_fac_outer = 4.0; // controlled by keyboard buttons

void main () {
	evaluationpoint_wor[gl_InvocationID] = controlpoint_wor[gl_InvocationID];

	// Calculate the tessellation levels
	gl_TessLevelInner[0] = tess_fac_inner; // number of nested primitives to generate
	gl_TessLevelOuter[0] = tess_fac_outer; // times to subdivide first side
	gl_TessLevelOuter[1] = tess_fac_outer; // times to subdivide second side
	gl_TessLevelOuter[2] = tess_fac_outer; // times to subdivide third side
}
