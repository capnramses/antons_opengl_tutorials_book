#version 400

// vertex positions input attribute
in vec2 vp;

// texture coordinates to be interpolated to fragment shaders
out vec2 st;

void main () {
	// interpolate texture coordinates
	st = (vp + 1.0) * 0.5;
	// transform vertex position to clip space (camera view and perspective)
	gl_Position = vec4 (vp, 0.0, 1.0);
}
