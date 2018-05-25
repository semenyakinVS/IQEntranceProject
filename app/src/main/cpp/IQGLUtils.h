////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef IQ_OPTION_ENTRANCE_TEST_GLUTILS_H
#define IQ_OPTION_ENTRANCE_TEST_GLUTILS_H

////////////////////////////////////////////////////////////////////////////////////////////////////
#include <GLES2/gl2.h>

#include "glm/mat4x4.hpp"

#define RENDER_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////
//========================================= API ====================================================
//--------------------------------------- Program --------------------------------------------------
GLuint shaderCompileAndSend(const char *inSource, GLenum inType);
void deleteShader(GLuint inShaderID);

GLuint createAndLinkShaderProgram(GLuint inVertexShaderID, GLuint inFragmentShaderID);
void deleteShaderProgram(GLuint inShaderProgramID);

//----------------------------------------- Data ---------------------------------------------------
//TODO: Provide error tracking for VBO creation
GLuint createVBO(const void *inBuffer, int inSize, GLenum inTarget, GLenum inUsage);
void deleteVBO(GLuint inVBOID);

#ifdef RENDER_DEBUG
//======================================== Debug ===================================================
//-------------------------------------Program debug -----------------------------------------------
void printShaderDebugInfo(GLuint inShaderID);
void printShaderProgramDebugInfo(GLuint inShaderProgramID);

//----------------------------- General error tracking utils ---------------------------------------
enum class GLErrorContext : int {
    Shader,
    ShaderProgram,
    Unknown,
    Invalid
};

void processGLError(GLErrorContext inContext, GLuint inGLObjectID, const char *inMessage = "");
void dropGLErrors(const char *inMessage = "<no message>");

//--------------------------------- General purpose printing ---------------------------------------
void printGLInfo(bool inPrintExtensions = false);

//- - - - - - - - - - - - - - - - - - - - Math - - - - - - - - - - - - - - - - - - - - - - - - - - -
void printGLMMat4(const glm::mat4x4 &inMatrix, const char *inMessage = "");

//--------------------------------- Enum to string conversions -------------------------------------
//NB: This functions return const literal strings. Don't delete[] them, please!
const char *getGLErrorFlagString(GLenum inErrorFlag);
const char *getGLTypeStringName(GLenum inType, bool inGetAsInGLSLName = true);

#endif //RENDER_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //IQ_OPTION_ENTRANCE_TEST_GLUTILS_H
