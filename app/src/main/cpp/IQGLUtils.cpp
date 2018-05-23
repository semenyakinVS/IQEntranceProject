////////////////////////////////////////////////////////////////////////////////////////////////////
#include "IQGLUtils.h"

#include <android/log.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
//================================== Helpers =======================================================
typedef void (GLIntegerParameterAccessFunction)(
        GLuint inObjectID, GLenum inParameterName, GLint *outValue);

typedef void (GLLogAccessFunction)(
        GLuint inObjectID, GLsizei inBufferSize, GLsizei *inLength, GLchar *outLog);

bool testGLSuccess(
        GLIntegerParameterAccessFunction *inIntegerAccess, GLLogAccessFunction *inLogAccess,
        GLuint inObjectID, GLenum inTestingParameter, char **outErrorLog)
{
    GLint theIsCompileSuccessful = 0;
    inIntegerAccess(inObjectID, inTestingParameter, &theIsCompileSuccessful);

    if(0 == theIsCompileSuccessful) {
        GLint theLogMessageLength = 0;
        inIntegerAccess(inObjectID, GL_INFO_LOG_LENGTH, &theLogMessageLength);

        if(theLogMessageLength > 1) {
            *outErrorLog = (char *)malloc(sizeof(char) * theLogMessageLength);
            inLogAccess(inObjectID, theLogMessageLength, NULL, *outErrorLog);
        }
    }
    return (0 != theIsCompileSuccessful);
}

//=============================== API implementation ===============================================
//------------------------------------ Shaders -----------------------------------------------------
GLuint shaderCompileAndSend(const char *inSource, GLenum inType) {
    GLuint theShaderID = glCreateShader(inType);
    if (0 == theShaderID) return 0;

    //Compile
    //NB: Here we have type casting - from "const char *" to "const GLbyte *"
    glShaderSource(theShaderID, 1, &inSource, NULL);
    glCompileShader(theShaderID);

    //Process compilation result
    GLint theIsCompileSuccessful = 0;
    glGetShaderiv(theShaderID, GL_COMPILE_STATUS, &theIsCompileSuccessful);

    char *theMessage = NULL;
    if(!testGLSuccess(
            glGetShaderiv, glGetShaderInfoLog, theShaderID, GL_COMPILE_STATUS, &theMessage))
    {
        if (NULL != theMessage) {
            __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "%s", theMessage);
            free(theMessage);
        }

        glDeleteShader(theShaderID);
        theShaderID = 0;
    }

    return theShaderID;
}

GLuint createAndLinkShaderProgram(GLuint inVertexShaderID, GLuint inFragmentShaderID) {
    GLuint theProgramID = glCreateProgram();
    if(0 == theProgramID) return 0;

    glAttachShader(theProgramID, inVertexShaderID);
    glAttachShader(theProgramID, inFragmentShaderID);

    glLinkProgram(theProgramID);

    char *theMessage = NULL;
    if (!testGLSuccess(
            glGetProgramiv, glGetProgramInfoLog, theProgramID, GL_LINK_STATUS, &theMessage))
    {
        if (theMessage) {
            __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "%s", theMessage);
            free(theMessage);
        }

        glDeleteProgram(theProgramID);
        theProgramID = 0;
    }

    return theProgramID;
}

//-------------------------------------- Data ------------------------------------------------------
GLuint createVBO(const void *inBuffer, int inSize, GLenum inTarget, GLenum inUsage) {
    GLuint theVBOID; glGenBuffers(1, &theVBOID);
    if (0 == theVBOID) return 0;

    glBindBuffer(inTarget, theVBOID);
    glBufferData(inTarget, inSize, inBuffer, inUsage);

    glBindBuffer(inTarget, 0);

    return theVBOID;
}

void deleteVBO(GLuint inVBOID) {
    glDeleteBuffers(1, &inVBOID);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Refs:
//1. https://www.khronos.org/assets/uploads/books/openglr_es_20_programming_guide_sample.pdf

////////////////////////////////////////////////////////////////////////////////////////////////////
