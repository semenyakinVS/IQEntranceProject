////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef IQ_OPTION_ENTRANCE_TEST_GLUTILS_H
#define IQ_OPTION_ENTRANCE_TEST_GLUTILS_H

////////////////////////////////////////////////////////////////////////////////////////////////////
#include <GLES2/gl2.h>
#include <stdlib.h>

#define RENDER_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////
//Program
GLuint shaderCompileAndSend(const char *inSource, GLenum inType);
void deleteShader(GLuint inShaderID);

GLuint createAndLinkShaderProgram(GLuint inVertexShaderID, GLuint inFragmentShaderID);
void deleteShaderProgram(GLuint inShaderProgramID);

//Data
//TODO: Provide error tracking for VBO creation
GLuint createVBO(const void *inBuffer, int inSize, GLenum inTarget, GLenum inUsage);
void deleteVBO(GLuint inVBOID);

//Debug
//TODO: Move this code to GL utils
#ifdef RENDER_DEBUG

//NB: Function return const literal strings. Don't remove them!
const char *getGLErrorFlagString(GLenum inErrorFlag);
void processGLErrors(const char *inMessage = "undefined");
void printGLInfo(bool inPrintExtensions = false);
#endif //RENDER_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //IQ_OPTION_ENTRANCE_TEST_GLUTILS_H
