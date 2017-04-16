/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| "Hello Triangle". Just the basics.                                           |
| If you're on Mac un-comment the version number code at the beginning. It     |
| will give you the latest, even if you say 3.2!                               |
| This uses the libraries GLEW and GLFW3 to start GL. Download and compile     |
| these first. Linking them might be a pain, but you'll need to master this.   |
|                                                                              |
| I wrote this so that it compiles in pedantic ISO C90, to show that it's      |
| easily done. I usually use minimalist C++ though, for tidier-looking maths   |
| functions.                                                                   |
\******************************************************************************/
#include <GL/glew.h>		/* include GLEW and new version of GL on Windows */
#include <GLFW/glfw3.h> /* GLFW helper library */
#include <stdio.h>
#include <string.h>
/* removed to keep visual studio happy #include <stdbool.h> */
#define bool int
#define true 1
#define false 0

// Note: put APIENTRY keyword for consistency with GL 4.3 core spec
void APIENTRY debug_gl_callback( GLenum source, GLuint type, GLenum id,
																 GLsizei severity, int length,
																 const GLchar *message, void *userParam ) {
	char src_str[2048];	/* source */
	char type_str[2048]; /* type */
	char sev_str[2048];	/* severity */

	switch ( source ) {
	case 0x8246:
		strcpy( src_str, "API" );
		break;
	case 0x8247:
		strcpy( src_str, "WINDOW_SYSTEM" );
		break;
	case 0x8248:
		strcpy( src_str, "SHADER_COMPILER" );
		break;
	case 0x8249:
		strcpy( src_str, "THIRD_PARTY" );
		break;
	case 0x824A:
		strcpy( src_str, "APPLICATION" );
		break;
	case 0x824B:
		strcpy( src_str, "OTHER" );
		break;
	default:
		strcpy( src_str, "undefined" );
	}

	switch ( type ) {
	case 0x824C:
		strcpy( type_str, "ERROR" );
		break;
	case 0x824D:
		strcpy( type_str, "DEPRECATED_BEHAVIOR" );
		break;
	case 0x824E:
		strcpy( type_str, "UNDEFINED_BEHAVIOR" );
		break;
	case 0x824F:
		strcpy( type_str, "PORTABILITY" );
		break;
	case 0x8250:
		strcpy( type_str, "PERFORMANCE" );
		break;
	case 0x8251:
		strcpy( type_str, "OTHER" );
		break;
	case 0x8268:
		strcpy( type_str, "MARKER" );
		break;
	case 0x8269:
		strcpy( type_str, "PUSH_GROUP" );
		break;
	case 0x826A:
		strcpy( type_str, "POP_GROUP" );
		break;
	default:
		strcpy( type_str, "undefined" );
	}

	switch ( severity ) {
	case 0x9146:
		strcpy( sev_str, "HIGH" );
		break;
	case 0x9147:
		strcpy( sev_str, "MEDIUM" );
		break;
	case 0x9148:
		strcpy( sev_str, "LOW" );
		break;
	case 0x826B:
		strcpy( sev_str, "NOTIFICATION" );
		break;
	default:
		strcpy( sev_str, "undefined" );
	}

	// Note: removed printing of userParam -bug with type of pointer in latest v.
	fprintf( stderr,
					 "source: %s type: %s id: %u severity: %s length: %i message: %s\n",
					 src_str, type_str, (unsigned int)id, sev_str, length, message );
}

int main() {
	GLFWwindow *window = NULL;
	const GLubyte *renderer;
	const GLubyte *version;
	GLuint vao;
	GLuint vbo;
	/* geometry to use. these are 3 xyz points (9 floats total) to make a triangle
	*/
	GLfloat points[] = { 0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f };
	/* these are the strings of code for the shaders
	the vertex shader positions each vertex point */
	const char *vertex_shader = "#version 410\n"
															"in vec3 vp;"
															"void main () {"
															"	gl_Position = vec4 (vp, 1.0);"
															"}";
	/* the fragment shader colours each fragment (pixel-sized area of the
	triangle) */
	const char *fragment_shader = "#version 410\n"
																"out vec4 frag_colour;"
																"void main () {"
																"	frag_colour = vec4 (0.5, 0.0, 0.5, 1.0);"
																"}";
	/* GL shader objects for vertex and fragment shader [components] */
	GLuint vs, fs;
	/* GL shader programme object [combined, to link] */
	GLuint shader_programme;

	/* start GL context and O/S window using the GLFW helper library */
	if ( !glfwInit() ) {
		fprintf( stderr, "ERROR: could not start GLFW3\n" );
		return 1;
	}

/* We must specify 3.2 core if on Apple OS X -- other O/S can specify
 anything here. I defined 'APPLE' in the makefile for OS X */
#ifdef APPLE
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
#endif
	// added this to get a proper debug context to start
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE );

	window = glfwCreateWindow( 640, 480, "Hello Triangle", NULL, NULL );
	if ( !window ) {
		fprintf( stderr, "ERROR: could not open window with GLFW3\n" );
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent( window );
	/* start GLEW extension handler */
	glewExperimental = GL_TRUE;
	glewInit();

	// note: this will probably not be available on Apple systems
	if ( GLEW_KHR_debug ) { // or try GLEW_ARB_debug_output
		printf( "KHR_debug extension found\n" );
		glDebugMessageCallback( (GLDEBUGPROC)debug_gl_callback, NULL );
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );
		printf( "debug callback engaged\n" );
	} else {
		printf( "KHR_debug extension NOT found\n" );
	}

	renderer = glGetString( GL_RENDERER ); /* get renderer string */
	version = glGetString( GL_VERSION );	 /* version as a string */
	printf( "Renderer: %s\n", renderer );
	printf( "OpenGL version supported %s\n", version );

	glEnable( GL_DEPTH_TEST ); /* enable depth-testing */
	glDepthFunc( GL_LESS ); /*depth-testing interprets a smaller value as "closer"*/

	glGenBuffers( 1, &vbo );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glBufferData( GL_ARRAY_BUFFER, 9 * sizeof( GLfloat ), points, GL_STATIC_DRAW );

	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );
	glEnableVertexAttribArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, vbo );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );

	vs = glCreateShader( GL_VERTEX_SHADER );
	glShaderSource( vs, 1, &vertex_shader, NULL );
	glCompileShader( vs );
	fs = glCreateShader( GL_FRAGMENT_SHADER );
	glShaderSource( fs, 1, &fragment_shader, NULL );
	glCompileShader( fs );
	shader_programme = glCreateProgram();
	glAttachShader( shader_programme, fs );
	glAttachShader( shader_programme, vs );
	glLinkProgram( shader_programme );

	{ /* deliberate errors */
		/* calling invalid enum */
		glEnable( GL_LINE );
		/* value out of allowed range */
		glEnableVertexAttribArray( GL_MAX_VERTEX_ATTRIBS + 1 );
		/* invalid buffer id */
		glBindBuffer( GL_ARRAY_BUFFER, -1 );
	}

	while ( !glfwWindowShouldClose( window ) ) {
		/* wipe the drawing surface clear */
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glUseProgram( shader_programme );
		glBindVertexArray( vao );
		/* draw points 0-3 from the currently bound VAO with current in-use shader*/
		glDrawArrays( GL_TRIANGLES, 0, 3 );
		/* update other events like input handling */
		glfwPollEvents();
		/* put the stuff we've been drawing onto the display */
		glfwSwapBuffers( window );
	}

	glfwTerminate();
	return 0;
}
