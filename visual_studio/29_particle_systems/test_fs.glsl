/* shader to render simple particle system points */
#version 410 core

in float opacity;
uniform sampler2D tex; // optional. enable point-sprite coords to use
out vec4 frag_colour;

const vec4 particle_colour = vec4 (0.4, 0.4, 0.8, 0.8); 

void main () {
	// using point texture coordinates which are pre-defined over the point
	vec2 texcoord = vec2 (gl_PointCoord.s, 1.0 - gl_PointCoord.t);
	vec4 texel = texture (tex, texcoord);
	frag_colour.a = opacity * texel.a;
	frag_colour.rgb = particle_colour.rgb * texel.rgb;
}
