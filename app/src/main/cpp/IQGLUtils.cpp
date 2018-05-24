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
            *outErrorLog = new char[theLogMessageLength];
            inLogAccess(inObjectID, theLogMessageLength, NULL, *outErrorLog);
        }
    }
    return (0 != theIsCompileSuccessful);
}

//=============================== API implementation ===============================================
//------------------------------------ Program -----------------------------------------------------
GLuint shaderCompileAndSend(const char *inSource, GLenum inType) {
    GLuint theShaderID = glCreateShader(inType);
    if (0 == theShaderID) {
#       ifdef RENDER_DEBUG
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "!!! GL cannot create shader !!!");
#       endif //RENDER_DEBUG
        return 0;
    }

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
            delete [] theMessage;
        }

        glDeleteShader(theShaderID);
        theShaderID = 0;
    } else {
#       ifdef RENDER_DEBUG
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP",
                            "Shader [%d] created and compiled successful", theShaderID);
        printShaderDebugInfo(theShaderID);
#       endif //RENDER_DEBUG
    }

    return theShaderID;
}

void deleteShader(GLuint inShaderID) {
    glDeleteShader(inShaderID);
}

GLuint createAndLinkShaderProgram(GLuint inVertexShaderID, GLuint inFragmentShaderID) {
    GLuint theProgramID = glCreateProgram();
    if(0 == theProgramID) {
#       ifdef RENDER_DEBUG
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "!!! GL cannot create shader program !!!");
#       endif //RENDER_DEBUG
        return 0;
    }

    glAttachShader(theProgramID, inVertexShaderID);
    glAttachShader(theProgramID, inFragmentShaderID);

    glLinkProgram(theProgramID);

    char *theMessage = NULL;

    if (!testGLSuccess(
            glGetProgramiv, glGetProgramInfoLog, theProgramID, GL_LINK_STATUS, &theMessage))
    {
        if (theMessage) {
            __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "%s", theMessage);
            delete [] theMessage;
        }

        glDeleteProgram(theProgramID);
        theProgramID = 0;
    } else {
#       ifdef RENDER_DEBUG
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP",
                            "Program [%d] created and linked successful", theProgramID);
        printShaderProgramDebugInfo(theProgramID);
#       endif //RENDER_DEBUG
    }

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

#ifdef RENDER_DEBUG
//======================================== Debug ===================================================
//-------------------------------------Program debug -----------------------------------------------
void processGLError_programContext(GLuint inGLObjectID, const char *inMessage = "") {
    GLenum theError = GL_NO_ERROR;

    theError = glGetError();
    if (GL_NO_ERROR != theError) {
        switch(theError) {
            case GL_INVALID_VALUE:
                __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "!!! Program %d is not a value"
                        " generated by OpenGL [Message: %s]",
                                    inGLObjectID, inMessage);
                break;

            case GL_INVALID_OPERATION:
                __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "!!! Program %d is not a program"
                        " object or has not been successfully linked [Message: %s]",
                                    inGLObjectID, inMessage);
                break;

            default:
                __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "!!! Undefined error while getting"
                        " uniform location using program %d [Message: %s]",
                                    inGLObjectID, inMessage);
                break;
        }
    }
}

void printShaderDebugInfo(GLuint inShaderID) {
    //TODO: Implement
}

void printShaderProgramDebugInfo(GLuint inShaderProgramID) {
    __android_log_print(ANDROID_LOG_INFO, "IQ_APP",
                        "========= Shader program debug info =========[%d]{", inShaderProgramID);

    GLint theValue = GL_FALSE;
    GLenum theType = 0; //TODO: Use here "gl invalid" enum - find it's value

    //Make general program validation
    glValidateProgram(inShaderProgramID);

    glGetProgramiv(inShaderProgramID, GL_VALIDATE_STATUS, &theValue);
    if (theValue == GL_TRUE) {
        int theMessageLength = 0;
        glGetProgramiv(inShaderProgramID, GL_INFO_LOG_LENGTH, &theMessageLength);
        char *theMessage = new char[theMessageLength];
        glGetProgramInfoLog(inShaderProgramID, theMessageLength, NULL, theMessage);

        if (0 == theMessageLength) {
            __android_log_print(ANDROID_LOG_INFO, "IQ_APP", "Program validation with no messages");
        } else {
            __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Program validation: %s", theMessage);
        }
        delete [] theMessage;
    } else {
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Program validation was not correct");
        return;
    }

    //Collect program info
    //-Initialize state
    glUseProgram(inShaderProgramID);

    //-Input
    //--Attributes
    glGetProgramiv(inShaderProgramID, GL_ACTIVE_ATTRIBUTES, &theValue);
    processGLError_programContext(inShaderProgramID, "debug_utils");

    __android_log_print(ANDROID_LOG_INFO, "IQ_APP", "------------ Attributes -------------");
    __android_log_print(ANDROID_LOG_INFO, "IQ_APP", "Count: %d", theValue);

    GLint theLength = 0;
    glGetProgramiv(inShaderProgramID, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &theLength);

    GLsizei theMaxLength = theLength;
    char *theName = new char[theMaxLength];

    for (int theIndex = 0; theIndex < theValue; ++theIndex) {
        glGetActiveAttrib(inShaderProgramID, (GLuint)theIndex,
                          theMaxLength, &theMaxLength, &theLength, &theType, theName);
        GLint theAttributeLocation = glGetAttribLocation(inShaderProgramID, theName);
        processGLError_programContext(inShaderProgramID, "debug_utils");

        __android_log_print(ANDROID_LOG_INFO, "IQ_APP",
                            "Attribute #%d: [type: %s] [name: %s] [location: %d]",
                            theIndex, getGLTypeStringName(theType), theName, theAttributeLocation);
    }

    //--Uniforms
    glGetProgramiv(inShaderProgramID, GL_ACTIVE_UNIFORMS, &theValue);
    __android_log_print(ANDROID_LOG_INFO, "IQ_APP", "------------ Uniforms -------------");
    __android_log_print(ANDROID_LOG_INFO, "IQ_APP", "Count: %d", theValue);

    theLength = 0;
    glGetProgramiv(inShaderProgramID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &theLength);

    //If uniform longest name is shorter then attribute longest name - reuse string
    if (theLength > theMaxLength) {
        delete [] theName;
        theName = new char[theMaxLength];
    }
    theMaxLength = theLength;

    for (int theIndex = 0; theIndex < theValue; ++theIndex) {
        glGetActiveUniform(inShaderProgramID, (GLuint)theIndex,
                          theMaxLength, &theMaxLength, &theLength, &theType, theName);
        GLint theUniformLocation =  glGetUniformLocation(inShaderProgramID, theName);
        processGLError_programContext(inShaderProgramID, "debug_utils");

        __android_log_print(ANDROID_LOG_INFO, "IQ_APP",
                            "Uniform #%d: [type: %s] [name: %s] [location: %d]",
                            theIndex, getGLTypeStringName(theType), theName, theUniformLocation);
    }

    //Deinitialize state
    delete [] theName;
    glUseProgram(0);

    __android_log_print(ANDROID_LOG_INFO, "IQ_APP", "}[%d]====================", inShaderProgramID);
}

//----------------------------- General error tracking utils ---------------------------------------
//TODO: Implement rest of processing
void processGLError(GLErrorContext inContext, GLuint inGLObjectID, const char *inMessage) {
    switch(inContext) {
        case GLErrorContext::Shader:        /* ... */                                                 return;
        case GLErrorContext::ShaderProgram: processGLError_programContext(inGLObjectID, inMessage);   return;
        case GLErrorContext::Unknown:       /* ... */                                                 return;
        case GLErrorContext::Invalid:       /* ... */                                                 return;
        default:                            /* ... */                                                 return;
    }
    //TODO: Put here inaccessible error assert
}

void dropGLErrors(const char *inMessage) {
    int theErrorsCount = 0;
    while (glGetError() != GL_NO_ERROR) { ++theErrorsCount; }

    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "There was %d errors on %s",
                        theErrorsCount, inMessage);
}

//--------------------------------- General purpose printing ---------------------------------------
void printGLInfo(bool inPrintExtensions) {
    const GLubyte *theGLVendor = glGetString(GL_VENDOR);
    const GLubyte *theGLRender = glGetString(GL_RENDERER);
    const GLubyte *theGLVersion = glGetString(GL_VERSION);
    const GLubyte *theGLSLVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

    if (inPrintExtensions) {
        const GLubyte *theGLExtensions = glGetString(GL_EXTENSIONS);

        const char *theFormat =
                        "======== GL Info ========\n"
                        "Vendor: %s\n"
                        "Renderer: %s\n"
                        "GL version: %s\n"
                        "GLSL version: %s\n"
                        "GL extensions: %s\n"
                        "=========================";

        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", theFormat,
                           theGLVendor, theGLRender, theGLVersion, theGLSLVersion, theGLExtensions);
    } else {
        const char *theFormat =
                        "======== GL Info ========\n"
                        "Vendor: %s\n"
                        "Renderer: %s\n"
                        "GL version: %s\n"
                        "GLSL version: %s\n"
                        "=========================";

        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", theFormat,
                           theGLVendor, theGLRender, theGLVersion, theGLSLVersion);
    }
}

//--------------------------------- Enum to string conversions -------------------------------------
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
    return "<!!!inaccessible!!!>";
}

// @formatter:off
//TODO: Change SOME_LATER_GL_VERSION to real tracking macro of GL version
const char *getGLTypeStringName(GLenum inType, bool inGetAsInGLSLName) {
    bool f = !inGetAsInGLSLName;
    switch(inType) {
        case GL_FLOAT                                    : return f ? "GL_FLOAT"                                     : "float";
        case GL_FLOAT_VEC2                               : return f ? "GL_FLOAT_VEC2"                                : "vec2";
        case GL_FLOAT_VEC3                               : return f ? "GL_FLOAT_VEC3"                                : "vec3";
        case GL_FLOAT_VEC4                               : return f ? "GL_FLOAT_VEC4"                                : "vec4";
#ifdef SOME_LATER_GL_VERSION
        case GL_DOUBLE                                   : return f ? "GL_DOUBLE"                                    : "double";
        case GL_DOUBLE_VEC2                              : return f ? "GL_DOUBLE_VEC2"                               : "dvec2";
        case GL_DOUBLE_VEC3                              : return f ? "GL_DOUBLE_VEC3"                               : "dvec3";
        case GL_DOUBLE_VEC4                              : return f ? "GL_DOUBLE_VEC4"                               : "dvec4";
#endif //SOME_LATER_GL_VERSION
        case GL_INT                                      : return f ? "GL_INT"                                       : "int";
        case GL_INT_VEC2                                 : return f ? "GL_INT_VEC2"                                  : "ivec2";
        case GL_INT_VEC3                                 : return f ? "GL_INT_VEC3"                                  : "ivec3";
        case GL_INT_VEC4                                 : return f ? "GL_INT_VEC4"                                  : "ivec4";
        case GL_UNSIGNED_INT                             : return f ? "GL_UNSIGNED_INT"                              : "unsigned int";
#ifdef SOME_LATER_GL_VERSION
        case GL_UNSIGNED_INT_VEC2                        : return f ? "GL_UNSIGNED_INT_VEC2"                         : "uvec2";
        case GL_UNSIGNED_INT_VEC3                        : return f ? "GL_UNSIGNED_INT_VEC3"                         : "uvec3";
        case GL_UNSIGNED_INT_VEC4                        : return f ? "GL_UNSIGNED_INT_VEC4"                         : "uvec4";
#endif //SOME_LATER_GL_VERSION
        case GL_BOOL                                     : return f ? "GL_BOOL"                                      : "bool";
        case GL_BOOL_VEC2                                : return f ? "GL_BOOL_VEC2"                                 : "bvec2";
        case GL_BOOL_VEC3                                : return f ? "GL_BOOL_VEC3"                                 : "bvec3";
        case GL_BOOL_VEC4                                : return f ? "GL_BOOL_VEC4"                                 : "bvec4";
        case GL_FLOAT_MAT2                               : return f ? "GL_FLOAT_MAT2"                                : "mat2";
        case GL_FLOAT_MAT3                               : return f ? "GL_FLOAT_MAT3"                                : "mat3";
        case GL_FLOAT_MAT4                               : return f ? "GL_FLOAT_MAT4"                                : "mat4";
#ifdef SOME_LATER_GL_VERSION
        case GL_FLOAT_MAT2x3                             : return f ? "GL_FLOAT_MAT2x3"                              : "mat2x3";
        case GL_FLOAT_MAT2x4                             : return f ? "GL_FLOAT_MAT2x4"                              : "mat2x4";
        case GL_FLOAT_MAT3x2                             : return f ? "GL_FLOAT_MAT3x2"                              : "mat3x2";
        case GL_FLOAT_MAT3x4                             : return f ? "GL_FLOAT_MAT3x4"                              : "mat3x4";
        case GL_FLOAT_MAT4x2                             : return f ? "GL_FLOAT_MAT4x2"                              : "mat4x2";
        case GL_FLOAT_MAT4x3                             : return f ? "GL_FLOAT_MAT4x3"                              : "mat4x3";
        case GL_DOUBLE_MAT2                              : return f ? "GL_DOUBLE_MAT2"                               : "dmat2";
        case GL_DOUBLE_MAT3                              : return f ? "GL_DOUBLE_MAT3"                               : "dmat3";
        case GL_DOUBLE_MAT4                              : return f ? "GL_DOUBLE_MAT4"                               : "dmat4";
        case GL_DOUBLE_MAT2x3                            : return f ? "GL_DOUBLE_MAT2x3"                             : "dmat2x3";
        case GL_DOUBLE_MAT2x4                            : return f ? "GL_DOUBLE_MAT2x4"                             : "dmat2x4";
        case GL_DOUBLE_MAT3x2                            : return f ? "GL_DOUBLE_MAT3x2"                             : "dmat3x2";
        case GL_DOUBLE_MAT3x4                            : return f ? "GL_DOUBLE_MAT3x4"                             : "dmat3x4";
        case GL_DOUBLE_MAT4x2                            : return f ? "GL_DOUBLE_MAT4x2"                             : "dmat4x2";
        case GL_DOUBLE_MAT4x3                            : return f ? "GL_DOUBLE_MAT4x3"                             : "dmat4x3";
        case GL_SAMPLER_1D                               : return f ? "GL_SAMPLER_1D"                                : "sampler1D";
#endif //SOME_LATER_GL_VERSION
        case GL_SAMPLER_2D                               : return f ? "GL_SAMPLER_2D"                                : "sampler2D";
#ifdef SOME_LATER_GL_VERSION
        case GL_SAMPLER_3D                               : return f ? "GL_SAMPLER_3D"                                : "sampler3D";
#endif //SOME_LATER_GL_VERSION
        case GL_SAMPLER_CUBE                             : return f ? "GL_SAMPLER_CUBE"                              : "samplerCube";
#ifdef SOME_LATER_GL_VERSION
        case GL_SAMPLER_1D_SHADOW                        : return f ? "GL_SAMPLER_1D_SHADOW"                         : "sampler1DShadow";
        case GL_SAMPLER_2D_SHADOW                        : return f ? "GL_SAMPLER_2D_SHADOW"                         : "sampler2DShadow";
        case GL_SAMPLER_1D_ARRAY                         : return f ? "GL_SAMPLER_1D_ARRAY"                          : "sampler1DArray";
        case GL_SAMPLER_2D_ARRAY                         : return f ? "GL_SAMPLER_2D_ARRAY"                          : "sampler2DArray";
        case GL_SAMPLER_1D_ARRAY_SHADOW                  : return f ? "GL_SAMPLER_1D_ARRAY_SHADOW"                   : "sampler1DArrayShadow";
        case GL_SAMPLER_2D_ARRAY_SHADOW                  : return f ? "GL_SAMPLER_2D_ARRAY_SHADOW"                   : "sampler2DArrayShadow";
        case GL_SAMPLER_2D_MULTISAMPLE                   : return f ? "GL_SAMPLER_2D_MULTISAMPLE"                    : "sampler2DMS";
        case GL_SAMPLER_2D_MULTISAMPLE_ARRAY             : return f ? "GL_SAMPLER_2D_MULTISAMPLE_ARRAY"              : "sampler2DMSArray";
        case GL_SAMPLER_CUBE_SHADOW                      : return f ? "GL_SAMPLER_CUBE_SHADOW"                       : "samplerCubeShadow";
        case GL_SAMPLER_BUFFER                           : return f ? "GL_SAMPLER_BUFFER"                            : "samplerBuffer";
        case GL_SAMPLER_2D_RECT                          : return f ? "GL_SAMPLER_2D_RECT"                           : "sampler2DRect";
        case GL_SAMPLER_2D_RECT_SHADOW                   : return f ? "GL_SAMPLER_2D_RECT_SHADOW"                    : "sampler2DRectShadow";
        case GL_INT_SAMPLER_1D                           : return f ? "GL_INT_SAMPLER_1D"                            : "isampler1D";
        case GL_INT_SAMPLER_2D                           : return f ? "GL_INT_SAMPLER_2D"                            : "isampler2D";
        case GL_INT_SAMPLER_3D                           : return f ? "GL_INT_SAMPLER_3D"                            : "isampler3D";
        case GL_INT_SAMPLER_CUBE                         : return f ? "GL_INT_SAMPLER_CUBE"                          : "isamplerCube";
        case GL_INT_SAMPLER_1D_ARRAY                     : return f ? "GL_INT_SAMPLER_1D_ARRAY"                      : "isampler1DArray";
        case GL_INT_SAMPLER_2D_ARRAY                     : return f ? "GL_INT_SAMPLER_2D_ARRAY"                      : "isampler2DArray";
        case GL_INT_SAMPLER_2D_MULTISAMPLE               : return f ? "GL_INT_SAMPLER_2D_MULTISAMPLE"                : "isampler2DMS";
        case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY         : return f ? "GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY"          : "isampler2DMSArray";
        case GL_INT_SAMPLER_BUFFER                       : return f ? "GL_INT_SAMPLER_BUFFER"                        : "isamplerBuffer";
        case GL_INT_SAMPLER_2D_RECT                      : return f ? "GL_INT_SAMPLER_2D_RECT"                       : "isampler2DRect";
        case GL_UNSIGNED_INT_SAMPLER_1D                  : return f ? "GL_UNSIGNED_INT_SAMPLER_1D"                   : "usampler1D";
        case GL_UNSIGNED_INT_SAMPLER_2D                  : return f ? "GL_UNSIGNED_INT_SAMPLER_2D"                   : "usampler2D";
        case GL_UNSIGNED_INT_SAMPLER_3D                  : return f ? "GL_UNSIGNED_INT_SAMPLER_3D"                   : "usampler3D";
        case GL_UNSIGNED_INT_SAMPLER_CUBE                : return f ? "GL_UNSIGNED_INT_SAMPLER_CUBE"                 : "usamplerCube";
        case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY            : return f ? "GL_UNSIGNED_INT_SAMPLER_1D_ARRAY"             : "usampler2DArray";
        case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY            : return f ? "GL_UNSIGNED_INT_SAMPLER_2D_ARRAY"             : "usampler2DArray";
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE      : return f ? "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE"       : "usampler2DMS";
        case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY: return f ? "GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY" : "usampler2DMSArray";
        case GL_UNSIGNED_INT_SAMPLER_BUFFER              : return f ? "GL_UNSIGNED_INT_SAMPLER_BUFFER"               : "usamplerBuffer";
        case GL_UNSIGNED_INT_SAMPLER_2D_RECT             : return f ? "GL_UNSIGNED_INT_SAMPLER_2D_RECT"              : "usampler2DRect";
        case GL_IMAGE_1D                                 : return f ? "GL_IMAGE_1D"                                  : "image1D";
        case GL_IMAGE_2D                                 : return f ? "GL_IMAGE_2D"                                  : "image2D";
        case GL_IMAGE_3D                                 : return f ? "GL_IMAGE_3D"                                  : "image3D";
        case GL_IMAGE_2D_RECT                            : return f ? "GL_IMAGE_2D_RECT"                             : "image2DRect";
        case GL_IMAGE_CUBE                               : return f ? "GL_IMAGE_CUBE"                                : "imageCube";
        case GL_IMAGE_BUFFER                             : return f ? "GL_IMAGE_BUFFER"                              : "imageBuffer";
        case GL_IMAGE_1D_ARRAY                           : return f ? "GL_IMAGE_1D_ARRAY"                            : "image1DArray";
        case GL_IMAGE_2D_ARRAY                           : return f ? "GL_IMAGE_2D_ARRAY"                            : "image2DArray";
        case GL_IMAGE_2D_MULTISAMPLE                     : return f ? "GL_IMAGE_2D_MULTISAMPLE"                      : "image2DMS";
        case GL_IMAGE_2D_MULTISAMPLE_ARRAY               : return f ? "GL_IMAGE_2D_MULTISAMPLE_ARRAY"                : "image2DMSArray";
        case GL_INT_IMAGE_1D                             : return f ? "GL_INT_IMAGE_1D"                              : "iimage1D";
        case GL_INT_IMAGE_2D                             : return f ? "GL_INT_IMAGE_2D"                              : "iimage2D";
        case GL_INT_IMAGE_3D                             : return f ? "GL_INT_IMAGE_3D"                              : "iimage3D";
        case GL_INT_IMAGE_2D_RECT                        : return f ? "GL_INT_IMAGE_2D_RECT"                         : "iimage2DRect";
        case GL_INT_IMAGE_CUBE                           : return f ? "GL_INT_IMAGE_CUBE"                            : "iimageCube";
        case GL_INT_IMAGE_BUFFER                         : return f ? "GL_INT_IMAGE_BUFFER"                          : "iimageBuffer";
        case GL_INT_IMAGE_1D_ARRAY                       : return f ? "GL_INT_IMAGE_1D_ARRAY"                        : "iimage1DArray";
        case GL_INT_IMAGE_2D_ARRAY                       : return f ? "GL_INT_IMAGE_2D_ARRAY"                        : "iimage2DArray";
        case GL_INT_IMAGE_2D_MULTISAMPLE                 : return f ? "GL_INT_IMAGE_2D_MULTISAMPLE"                  : "iimage2DMS";
        case GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY           : return f ? "GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY"            : "iimage2DMSArray";
        case GL_UNSIGNED_INT_IMAGE_1D                    : return f ? "GL_UNSIGNED_INT_IMAGE_1D"                     : "uimage1D";
        case GL_UNSIGNED_INT_IMAGE_2D                    : return f ? "GL_UNSIGNED_INT_IMAGE_2D"                     : "uimage2D";
        case GL_UNSIGNED_INT_IMAGE_3D                    : return f ? "GL_UNSIGNED_INT_IMAGE_3D"                     : "uimage3D";
        case GL_UNSIGNED_INT_IMAGE_2D_RECT               : return f ? "GL_UNSIGNED_INT_IMAGE_2D_RECT"                : "uimage2DRect";
        case GL_UNSIGNED_INT_IMAGE_CUBE                  : return f ? "GL_UNSIGNED_INT_IMAGE_CUBE"                   : "uimageCube";
        case GL_UNSIGNED_INT_IMAGE_BUFFER                : return f ? "GL_UNSIGNED_INT_IMAGE_BUFFER"                 : "uimageBuffer";
        case GL_UNSIGNED_INT_IMAGE_1D_ARRAY              : return f ? "GL_UNSIGNED_INT_IMAGE_1D_ARRAY"               : "uimage1DArray";
        case GL_UNSIGNED_INT_IMAGE_2D_ARRAY              : return f ? "GL_UNSIGNED_INT_IMAGE_2D_ARRAY"               : "uimage2DArray";
        case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE        : return f ? "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE"         : "uimage2DMS";
        case GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY  : return f ? "GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY"   : "uimage2DMSArray";
        case GL_UNSIGNED_INT_ATOMIC_COUNTER              : return f ? "GL_UNSIGNED_INT_ATOMIC_COUNTER"               : "atomic_uint";
#endif //SOME_LATER_GL_VERSION
        default                                          : return "<unknown type>";
    }
    return "<!!!inaccessible!!!>";
}
// @formatter:on

#endif //RENDER_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////
//Refs:
//1. https://www.khronos.org/assets/uploads/books/openglr_es_20_programming_guide_sample.pdf
//2. https://stackoverflow.com/questions/440144/in-opengl-is-there-a-way-to-get-a-list-of-all-uniforms-attribs-used-by-a-shade
//3. https://stackoverflow.com/questions/3375307/how-to-disable-code-formatting-for-some-part-of-the-code-using-comments
