
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>

#include "shader.h"

int oglError()
{
	/* prints out opengl error codes if they have been generated */
    GLenum glErr;
    int retCode = 0;
    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        printf("glError 0x%x: %s\n", glErr, gluErrorString(glErr));
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}

void shaderError(GLuint shader)
{
	/* prints out detailed compile errors for the given shader (fragment or vertex) */
#ifdef SHOW_SHADER_ERRORS
    int infologLength = 0;
    int charsWritten  = 0;
    GLchar *infoLog;
    oglError();

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infologLength);
    oglError();
    if (infologLength > 1)
    {
        infoLog = (GLchar *)malloc(infologLength);
        glGetShaderInfoLog(shader, infologLength, &charsWritten, infoLog);
        printf("Shader InfoLog:%s\n", infoLog);
        free(infoLog);
    }
    oglError();
#endif
}

void programError(GLuint program)
{
	/* prints out detailed compile errors for given program */
#ifdef SHOW_SHADER_ERRORS
    int infologLength = 0;
    int charsWritten  = 0;
    GLchar *infoLog;
    oglError();
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);
	oglError();
    if (infologLength > 1)
    {
        infoLog = (GLchar *)malloc(infologLength);
        glGetProgramInfoLog(program, infologLength, &charsWritten, infoLog);
        printf("Program InfoLog:%s\n", infoLog);
        free(infoLog);
    }
    oglError();
#endif
}

char* readFile(const char* filename)
{
	/* reads and returns the entire contents of a file "filename" */
	/* NOTE: remember to free this memory using free() after use! */
	int size;
	char* data;
	if (!filename)
		return NULL;
	FILE* file = fopen(filename, "rb");
	if (!file)
		return NULL;
	fseek(file, 0, 2);
	size = ftell(file);
	fseek(file, 0, 0);
	data = (char*)malloc(size+1);
	fread(data, sizeof(char), size, file);
	fclose(file);
	data[size] = '\0';
	return data;
}

GLuint loadShaderPart(const char* file, GLenum type)
{
	char* src;
	GLuint shader;
	
	if (!file) return 0; /* return 0 if no filename given */
	
	/* read shader source */
	src = readFile(file);
	
	/* return 0 if no source file did not read */
	if (!src)
	{
		free(src);
		printf("ERROR: Shader file %s do not exist\n", file);
		return 0;
	}
	
	/* create the shader object */
	shader = glCreateShader(type);
	
	/* pass the source to OpenGL */
	glShaderSource(shader, 1, (const GLchar**)&src, NULL);
	
	/* compile them and check for compile errors */
	glCompileShader(shader);
	shaderError(shader);
	
	/* cleanup and return */
	free(src);
	
	return shader;
}

GLuint loadShader(const char* vertexFile, const char* fragmentFile)
{
    GLuint vert, frag, program;
	if (oglError() != GL_NO_ERROR) /* just in case an error occurred beforehand */
		printf("NOTE: GL error set before call to loadShader()\n");
	
	/* load vertex and fragment shaders */
	vert = loadShaderPart(vertexFile, GL_VERTEX_SHADER);
	frag = loadShaderPart(fragmentFile, GL_FRAGMENT_SHADER);
	
	if (!vert && !frag)
		return 0;
	
	/* create the shader program object */
	program = glCreateProgram();
	
	/* attach the shader objects to the program */
	if (vert) glAttachShader(program, vert);
	if (frag) glAttachShader(program, frag);
	
	/* link the attached shader objects to create the shader executable */
	glLinkProgram(program);
	
	/* check for errors */
	programError(program);
	
	/* now it's compiled we don't need the intermediates */
	if (vert) glDeleteShader(vert);
	if (frag) glDeleteShader(frag);
	
	/* return our shader program */
    oglError();
	return program;
}

void deleteShader(GLuint program)
{
	if (program > 0)
		glDeleteProgram(program);
}

