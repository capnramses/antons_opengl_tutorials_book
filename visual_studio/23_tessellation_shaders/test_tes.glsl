#version 400

// triangles, quads, or isolines
layout (triangles, equal_spacing, ccw) in;
in vec3 evaluationpoint_wor[];

// could use a displacement map here

out vec3 colour;

// gl_TessCoord is location within the patch
// (barycentric for triangles, UV for quads)

void main () {
	vec3 p0 = gl_TessCoord.x * evaluationpoint_wor[0]; // x is one corner
	vec3 p1 = gl_TessCoord.y * evaluationpoint_wor[1]; // y is the 2nd corner
	vec3 p2 = gl_TessCoord.z * evaluationpoint_wor[2]; // z is the 3rd corner (ignore when using quads)
// this is wrong!	vec3 pos = normalize (p0 + p1 + p2);
	vec3 pos = p0 + p1 + p2; // this is right!
	gl_Position = vec4 (pos, 1.0); // use view and projection matrices here
	
	// use coords as colours in the frag shader
	colour.rbg = gl_TessCoord.xyz;
}
