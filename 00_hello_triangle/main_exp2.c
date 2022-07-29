/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                          |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| "Hello Triangle". Just the basics.                                           |
| If you're on Apple un-comment the version number code at the beginning. It   |
| will give you the latest, even if you say 3.2!                               |
| This uses the libraries GLEW and GLFW3 to start GL. Download and compile     |
| these first. Linking them might be a pain, but you'll need to master this.   |
\******************************************************************************/

#include <GL/glew.h>    /* include GLEW and new version of GL on Windows */
#include <GLFW/glfw3.h> /* GLFW helper library */
#include <stdio.h>

#include <string.h>
#include <stdlib.h>

#include <assert.h>

/*
Load the shader strings from text files called test.vertand test.frag(a naming convention is handy). 

Change the colour of the triangle in the fragment shader. 

Try to move the shape in the vertex
shader e.g. vec4 (vp.x, vp.y + 1.0, vp.z, 1.0); 

Try to add another triangle to the list of points and make a square shape. You will have to change several 
variables when setting up the buffer and drawing the shape. Which variables do you need to keep track of 
for each triangle? (hint: not much...). 

Try drawing with GL_LINE_STRIP or GL_LINES or GL_POINTS instead of triangles. Does it put the lines
where you expect? How big are the points by default? 

Try changing the background colour by using glClearColor ()before the rendering loop. Something grey-ish is usually fairly neutral; 0.6f, 0.6f,
0.8f, 1.0f. 

Try creating a second VAO, and drawing 2 shapes (remember to bind the second VAO before drawing again). 

Try creating a second shader programme, and draw the second shape a different colour
(remember to "use" the second shader programme before drawing again).

Gerdelan, Anton. Anton's OpenGL 4 Tutorials . Kindle Edition. */

char* read_from_file( const char* filename ) {

  FILE* text_file = fopen( filename, "rb" );
  if ( text_file == NULL) {
    fprintf( stderr, "Cannot open input file: %s. %s\n", filename, strerror( errno ) );
    exit( 2 );
  }

  fseek( text_file, 0, SEEK_END );
  long size = ftell( text_file );

  char* buffer = malloc(size + 1);
  assert( buffer );
  fseek( text_file, 0, SEEK_SET );
  size_t n = fread( buffer, 1, size, text_file );
  assert( n == size );
  buffer[size] = 0;

  fclose( text_file );
  return buffer;
}

const char* gl_error_string( GLenum err ) 
{
  switch ( err ) {
  case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
  case GL_INVALID_VALUE: return "GL_INVALID_VALUE"; 
  default: 
    break;
  }
  static char buffer[80];
  sprintf( buffer, "%d", err );
  return buffer;
}

int main() {
  GLFWwindow* window = NULL;
  const GLubyte* renderer;
  const GLubyte* version;
  GLuint vao;
  GLuint vbo;

  /* geometry to use. these are 3 xyz points to make a triangle(s) */
  GLfloat points[] = { 
#if 0
      0.0f, 0.5f, 0.0f, 
      0.5f, -0.5f, 0.0f,
      -0.5f, -0.5f, 0.0f,
#else
      0.0f, 0.5f, 0.0f, 
      0.5f, -0.5f, 0.0f,
      -0.5f, -0.5f, 0.0f,

      1.0f, 1.5f, 0.0f,
      1.5f, 0.5f, 0.0f,
      0.5f, 0.5f, 0.0f,
#endif
  };

  /* Load the shader strings from text files called test.vertand test.frag( a naming convention is handy ).*/

  /* these are the strings of code for the shaders
  the vertex shader positions each vertex point */
  const char* vertex_shader = 
    "#version 410\n"
    "in vec3 vp;"
    "void main () {"
    "  gl_Position = vec4(vp, 1.7);"
    "}";

  /* the fragment shader colours each fragment (pixel-sized area of the
  triangle) */
  const char* fragment_shader = 
    "#version 410\n"
    "out vec4 frag_colour;"
    "void main () {"
    "  frag_colour = vec4(0.5, 0.0, 0.0, 1.0);"
    "}";

  /* GL shader objects for vertex and fragment shader [components] */
  GLuint vert_shader, frag_shader;
  /* GL shader programme object [combined, to link] */
  GLuint shader_programme;

  /* start GL context and O/S window using the GLFW helper library */
  if ( !glfwInit() ) {
    fprintf( stderr, "ERROR: could not start GLFW3\n" );
    return 1;
  }

  /* Version 4.1 Core is a good default that should run on just about everything. Adjust later to suit project requirements. */
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

  window = glfwCreateWindow( 800, 600, "Hello Triangle (Experiments: #2)", NULL, NULL );
  if ( !window ) {
    fprintf( stderr, "ERROR: could not open window with GLFW3\n" );
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent( window );

  /* start GLEW extension handler */
  glewExperimental = GL_TRUE;
  glewInit();

  /* get version info */
  renderer = glGetString( GL_RENDERER ); /* get renderer string */
  version  = glGetString( GL_VERSION );  /* version as a string */
  printf( "Renderer: %s\n", renderer );
  printf( "OpenGL version supported %s\n", version );

  /* tell GL to only draw onto a pixel if the shape is closer to the viewer
  than anything already drawn at that pixel */
  glEnable( GL_DEPTH_TEST ); /* enable depth-testing */
  /* with LESS depth-testing interprets a smaller depth value as meaning "closer" */
  glDepthFunc( GL_LESS );

  /* a vertex buffer object (VBO) is created here. this stores an array of
  data on the graphics adapter's memory. in our case - the vertex points */
  glGenBuffers( 1, &vbo );
  glBindBuffer( GL_ARRAY_BUFFER, vbo );
  glBufferData( GL_ARRAY_BUFFER, sizeof( points ), points, GL_STATIC_DRAW );

  /* the vertex array object (VAO) is a little descriptor that defines which
  data from vertex buffer objects should be used as input variables to vertex
  shaders. in our case - use our only VBO, and say 'every three floats is a
  variable' */
  glGenVertexArrays( 1, &vao );
  glBindVertexArray( vao );
  /* "attribute #0 should be enabled when this vao is bound" */
  glEnableVertexAttribArray( 0 );
  /* this VBO is already bound, but it's a good habit to explicitly specify which
  VBO's data the following vertex attribute pointer refers to */
  glBindBuffer( GL_ARRAY_BUFFER, vbo );
  /* "attribute #0 is created from every 3 variables in the above buffer, of type
  float (i.e. make me vec3s)" */
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );

  /* here we copy the shader strings into GL shaders, and compile them. we
  then create an executable shader 'program' and attach both of the compiled
      shaders. we link this, which matches the outputs of the vertex shader to
  the inputs of the fragment shader, etc. and it is then ready to use */
  vert_shader = glCreateShader( GL_VERTEX_SHADER );
  glShaderSource( vert_shader, 1, &vertex_shader, NULL );
  glCompileShader( vert_shader );
  frag_shader = glCreateShader( GL_FRAGMENT_SHADER );
  glShaderSource( frag_shader, 1, &fragment_shader, NULL );
  glCompileShader( frag_shader );
  shader_programme = glCreateProgram();
  glAttachShader( shader_programme, frag_shader );
  glAttachShader( shader_programme, vert_shader );
  glLinkProgram( shader_programme );

  /*  this loop clears the drawing surface, then draws the geometry described
      by the VAO onto the drawing surface. we 'poll events' to see if the window
  was closed, etc. finally, we 'swap the buffers' which displays our drawing
      surface onto the view area. we use a double-buffering system which means
      that we have a 'currently displayed' surface, and 'currently being drawn'
      surface. hence the 'swap' idea. in a single-buffering system we would see
      stuff being drawn one-after-the-other */
  while ( !glfwWindowShouldClose( window ) ) {
    /* experimenting... */
    glClearColor( 0.6f, 0.6f, 0.6f, 1.0f );
    /* wipe the drawing surface clear */
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glUseProgram( shader_programme );
    glBindVertexArray( vao );
    /* draw points 0-4 from the currently bound VAO with current in-use shader */

#if 1
    glDrawArrays( GL_LINE_STRIP, 0, 6 );
#else
    glDrawArrays( GL_TRIANGLES, 0, 6 );
#endif
    int err;
    if ( (err = glGetError()) ) { 
        printf( "GL ERROR: %d: %s\n", err, gl_error_string( err ) );
        break;
    }

    /* update other events like input handling */
    glfwPollEvents();
    /* put the stuff we've been drawing onto the display */
    glfwSwapBuffers( window );
  }

  /* close GL context and any other GLFW resources */
  glfwTerminate();
  return 0;
}
