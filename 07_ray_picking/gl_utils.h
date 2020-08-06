/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Dr Anton Gerdelan, Trinity College Dublin, Ireland.                          |
| See individual libraries' separate legal notices                             |
|******************************************************************************|
| This is just a file holding some commonly-used "utility" functions to keep   |
| the main file a bit easier to read. You can might build up something like    |
| this as you learn more GL. Note that you don't need much code here to do     |
| good GL. If you have a big object-oriented engine then maybe you can ask     |
| yourself if  it is really making life easier.                                |
\******************************************************************************/
#ifndef _GL_UTILS_H_
#define _GL_UTILS_H_

#include <GL/glew.h>    // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <stdarg.h>     // used by log functions to have variable number of args

/*------------------------------GLOBAL VARIABLES------------------------------*/
extern int g_gl_window_width;
extern int g_gl_window_height;
extern int g_gl_framebuffer_width;
extern int g_gl_framebuffer_height;
// flag raised when window size changes so app can update camera. app should then set this flag to false
extern bool g_frambuffer_changed;
extern GLFWwindow* g_window;
/*--------------------------------LOG FUNCTIONS-------------------------------*/
bool restart_gl_log();
bool gl_log( const char* message, ... );
/* same as gl_log except also prints to stderr */
bool gl_log_err( const char* message, ... );
/*--------------------------------GLFW3 and GLEW------------------------------*/
bool start_gl();
void _update_fps_counter( GLFWwindow* window );
/*-----------------------------------SHADERS----------------------------------*/
bool parse_file_into_str( const char* file_name, char* shader_str, int max_len );
void print_shader_info_log( GLuint shader_index );
bool create_shader( const char* file_name, GLuint* shader, GLenum type );
bool is_programme_valid( GLuint sp );
bool create_programme( GLuint vert, GLuint frag, GLuint* programme );
/* just use this func to create most shaders; give it vertex and frag files */
GLuint create_programme_from_files( const char* vert_file_name, const char* frag_file_name );
#endif
