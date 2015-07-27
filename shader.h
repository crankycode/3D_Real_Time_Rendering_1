
#ifndef SHADER_H
#define SHADER_H

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>

#define SHOW_SHADER_ERRORS

/*
 * A note about the OpenGL terminology:
 * The GL "program" is the fully compiled shader program
 * where as the "shader" is a vertex or fragment shader
 * that is compiled into a "program". Here, loadShader
 * actually returns the compiled OpenGL "program".
 */

/* Loads a GLSL shader from the specified files. Returns 0 on error */
GLuint loadShader(const char* vertexFile, const char* fragmentFile);

/* deletes the loaded shader referenced by "program" */
void deleteShader(GLuint program);

#endif
