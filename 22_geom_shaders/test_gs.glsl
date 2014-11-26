#version 410
#extension GL_EXT_geometry_shader4 : enable

layout (points) in;
// convert to points, line_strip, or triangle_strip
layout (triangle_strip, max_vertices = 4) out;


// NB: in and out pass-through vertex->fragment variables must go here if used
in vec3 colour[];
out vec3 f_colour;

void main () {
	for(int i = 0; i < gl_VerticesIn; i++) {
		// use original point as first point in triangle strip
		gl_Position = gl_in[i].gl_Position;
		// output pass-through data to go to fragment-shader (colour)
		f_colour = colour[0];
		// finalise first vertex
		EmitVertex();
		// create another point relative to the previous
		gl_Position.y += 0.4;
		f_colour = colour[0];
		EmitVertex();
		// create another point relative to the previous
		gl_Position.x += 0.2;
		gl_Position.y -= 0.4;
		f_colour = colour[0];
		EmitVertex();
		// create another point relative to the previous
		gl_Position.y += 0.4;
		f_colour = colour[0];
		EmitVertex();
	}
}
