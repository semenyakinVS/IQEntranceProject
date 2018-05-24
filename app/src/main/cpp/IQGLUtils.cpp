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
//------------------------------------ Program -----------------------------------------------------
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

void deleteShader(GLuint inShaderID) {
    glDeleteShader(inShaderID);
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

#   ifdef RENDER_DEBUG
    glValidateProgram(theProgramID);

    GLint theValue = GL_FALSE;
    glGetProgramiv(theProgramID, GL_VALIDATE_STATUS, &theValue);
    if (theValue == GL_TRUE) {
        int theMessageLength = 0;
        glGetProgramiv(theProgramID, GL_INFO_LOG_LENGTH, &theMessageLength);
        char *theMessage = new char[theMessageLength];
        glGetProgramInfoLog(theProgramID, theMessageLength, NULL, theMessage);
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Shader validation result (length=%d): %s",
                            theMessageLength, theMessage);
        delete [] theMessage;
    } else {
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Shader validation was not correct");
    }

    glGetProgramiv(theProgramID, GL_LINK_STATUS, &theValue);
    if (theValue == GL_TRUE) {
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Program linked successful");
    } else {
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "!!! Program linked incorrect !!!");
    }

    glGetProgramiv(theProgramID, GL_ACTIVE_ATTRIBUTES, &theValue);
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Program contains [%d] active attributes",
                        theValue);

    glGetProgramiv(theProgramID, GL_ACTIVE_UNIFORMS, &theValue);
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Program contains [%d] active uniforms",
                        theValue);
#endif //RENDER_DEBUG

    return theProgramID;
}

void deleteShaderProgram(GLuint inShaderProgramID) {
    glDeleteProgram(inShaderProgramID);
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

//------------------------------------- Debug ------------------------------------------------------
//TODO: Move this code to GL utils
#ifdef RENDER_DEBUG
//NB: Function return const literal strings. Don't remove them!
const char *getGLErrorFlagString(GLenum inErrorFlag) {
    switch(inErrorFlag) {
        case GL_NO_ERROR:                       return "No error";
        case GL_INVALID_ENUM:                   return "Invalid enum";
        case GL_INVALID_VALUE:                  return "Invalid value";
        case GL_INVALID_OPERATION:              return "Invalid operation";
        case GL_INVALID_FRAMEBUFFER_OPERATION:  return "Invalid framebuffer operation";
        case GL_OUT_OF_MEMORY:                  return "Out of memory";
        default:                                return "UNKNOWN ENUM";
    }
    return "!!!inaccessible!!!";
}

void processGLErrors(const char *inMessage) {
    int theErrorsCount = 0;
    while (glGetError() != GL_NO_ERROR) { ++theErrorsCount; }

    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "There was %d errors on %s",
                        theErrorsCount, inMessage);
}
#endif //RENDER_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////
//Refs:
//1. https://www.khronos.org/assets/uploads/books/openglr_es_20_programming_guide_sample.pdf

////////////////////////////////////////////////////////////////////////////////////////////////////
