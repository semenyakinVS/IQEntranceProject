//
// Created by test on 5/23/2018.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "IQGraphView.h"

//Graph layer
#include "IQGraphViewLayer.h"

//Utils
#ifdef RENDER_DEBUG
#include <android/log.h>
#endif //RENDER_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------- Methods --------------------------------------------------
//- - - - - - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - - - -
IQGraphView::IQGraphView()
    : _graphLayer(nullptr),
      _frameBegin(-1.f, -1.f),
      _frameEnd(1.f, 1.f),
      _programID(0),
      _viewportWidth(1.f), _viewportHeight(1.f) { }

IQGraphView::~IQGraphView() { }

//- - - - - - - - - - - - - - - - - GL drawing lifecycle - - - - - - - - - - - - - - - - - - - - - -
void IQGraphView::init() {
    //TODO: Put here an errors messages tracking

#   ifdef RENDER_DEBUG
    processGLErrors("graph view init begin");
#   endif //RENDER_DEBUG

    //Setup shader program
    const char *theVertexShaderSource =
            "attribute vec2 vertexPosition;                                                     \n"
            //TODO: Make sending of depth uniform works
            //"uniform float depth;\n"
            "uniform mat4 MVPMatrix;                                                            \n"
            "void main() {                                                                      \n"
            "   gl_Position = MVPMatrix * vec4(vertexPosition.xy, 0.0, 1.0);                    \n"
            "} \n";

    const char *theFragmentShaderSource =
            "void main() {                                                                      \n"
            "   gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);                                        \n"
            "}                                                                                  \n";

    GLuint theVertexShaderID = shaderCompileAndSend(theVertexShaderSource, GL_VERTEX_SHADER);
    if (0 == theVertexShaderID) return;

    GLuint theFragmentShaderID = shaderCompileAndSend(theFragmentShaderSource, GL_FRAGMENT_SHADER);
    if (0 == theFragmentShaderID) return;

    _programID = createAndLinkShaderProgram(theVertexShaderID, theFragmentShaderID);
    if (0 == _programID) return;

    //Setup global GL properties
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#   ifdef RENDER_DEBUG
    processGLErrors("graph view init end");
#   endif //RENDER_DEBUG
}

//@ - - - - - - - - - - - - - - - - - - Drawing - - - - - - - - - - - - - - - - - - - - - - - - - -@
void helper_fillCameraFrameToNDCMatrix(glm::mat4x4 &outMatrix,
                                float inFrameBeginX, float inFrameBeginY,
                                float inFrameEndX, float inFrameEndY,
                                float inDepthLevel)
{
    // X: 0  1  2  3
    outMatrix[0][0] = 2/(inFrameEndX - inFrameBeginX); //0
    outMatrix[1][0] = 0; //1
    outMatrix[2][0] = 0; //2
    outMatrix[3][0] = -(inFrameEndX + inFrameBeginX)/(inFrameEndX - inFrameBeginX); //3

    // Y: 4  5  6  7
    outMatrix[0][1] = 0; //4
    outMatrix[1][1] = 2/(inFrameEndY - inFrameBeginY); //5
    outMatrix[2][1] = 0; //6
    outMatrix[3][1] = -(inFrameEndY + inFrameBeginY)/(inFrameEndY - inFrameBeginY); //7

    // Z: 8  9  10 11
    outMatrix[0][2] = 0; //4
    outMatrix[1][2] = 0; //5
    outMatrix[2][2] = -2/(0.f - inDepthLevel); //6
    outMatrix[3][2] = -(inDepthLevel + 0.f)/(inDepthLevel - 0.f); //7

    // W: 12 13 14 15
    outMatrix[0][3] = 0;
    outMatrix[1][3] = 0;
    outMatrix[2][3] = 0;
    outMatrix[3][3] = 1;

#   ifdef RENDER_DEBUG
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", " = = = = = = = = = = = =");
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Frame to NDC matrix (for %f %f %f %f %f):",
                        inFrameBeginX, inFrameBeginY, inFrameEndX, inFrameEndY, inDepthLevel);
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "%f %f %f %f",
          outMatrix[0][0], outMatrix[1][0], outMatrix[2][0], outMatrix[3][0]);
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "%f %f %f %f",
          outMatrix[0][1], outMatrix[1][1], outMatrix[2][1], outMatrix[3][1]);
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "%f %f %f %f",
          outMatrix[0][2], outMatrix[1][2], outMatrix[2][2], outMatrix[3][2]);
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "%f %f %f %f",
          outMatrix[0][3], outMatrix[1][3], outMatrix[2][3], outMatrix[3][3]);
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", " = = = = = = = = = = = =");
#   endif //RENDER_DEBUG
}

void IQGraphView::draw() {

#   ifdef RENDER_DEBUG
    processGLErrors("draw begin");
#   endif //RENDER_DEBUG

    //Clear
    glClear(GL_COLOR_BUFFER_BIT);

    //Process layer
    if (!_graphLayer) return;
    _graphLayer->graphViewAccess_initDLData();

    //Setup programm
    glUseProgram(_programID);

#   ifdef RENDER_DEBUG
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Started program %d", _programID);
    if (glIsProgram(_programID) != GL_TRUE) {
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "...But this is not a program!!!");
    }
#   endif //RENDER_DEBUG

    //Setup input data
    //-Update and setup MVP Matrix (currently simple transform from frame to NDC)
    //TODO: Find why Matrix is sent to shader only if it setup was before VBO with vertex data setup
    //NB: Matrix should be local value - glUniformMatrix4fv(...) send matrix not at the call moment
    helper_fillCameraFrameToNDCMatrix(_fromToNDC,
                                      _frameBegin.x, _frameBegin.y, _frameEnd.x, _frameEnd.y,
                                      1.f);

    GLuint theMVPMatrixID;
    if (-1 == glGetUniformLocation(theMVPMatrixID, "MVPMatrix")) {
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Cannot find [MVPMatrix] uniform");
    }

#   ifdef RENDER_DEBUG
//    GLenum theError = GL_NO_ERROR;
//
//    theError = glGetError();
//    if (GL_NO_ERROR != theError) {
//        switch(theError) {
//            case GL_INVALID_VALUE:
//                __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "!!! Program %d is not a value"
//                        " generated by OpenGL !!!", _programID);
//                break;
//
//            case GL_INVALID_OPERATION:
//                __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "!!! Program %d is not a program"
//                        " object or has not been successfully linked !!!", _programID);
//                break;
//
//            default:
//                __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "!!! Undefined error while getting"
//                        " uniform location using program %d !!!", _programID);
//                break;
//        }
//    }
#   endif //RENDER_DEBUG

    glUniformMatrix4fv(theMVPMatrixID, 1, GL_FALSE, glm::value_ptr(_fromToNDC));

    //-Send depth data
    //TODO: Make sending of depth uniform works
    //int theDepthID; glGetUniformLocation(theDepthID, "depth");
    //glUniform1f(theDepthID, 0.f);

    //-Setup vertex data
    GLuint theVertexAttributeID;
    if (-1 == glGetAttribLocation(theVertexAttributeID, "vertexPosition")) {
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Cannot find [vertexPosition] attribute");
    }

#   ifdef RENDER_DEBUG
    GLenum theError = glGetError();
    if (theError != GL_NO_ERROR) {
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP",
                            "!!! Error on attribute [vertexPosition] location %s !!!",
                            getGLErrorFlagString(theError));
    }
#   endif //RENDER_DEBUG

    //TODO: Using 1xGL_FLOAT_VEC3 instead 3xGL_FLOAT as attribute setting cause crash on draw. Why?
    glBindBuffer(GL_ARRAY_BUFFER, _graphLayer->graphViewAccess_getVBOID());
    glVertexAttribPointer(
            theVertexAttributeID,    /*Attribute ID*/
            2,                       /*Components per vertex*/
            GL_FLOAT,                /*Component type*/
            GL_FALSE,                /*If true - value should be normalized [0,1]*/
            0,                       /*Stride = data offset = 0*/
            0                        /*If is 0 - get data from binded GL_ARRAY_BUFFER*/
    );
    glEnableVertexAttribArray(theVertexAttributeID);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //Draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, _graphLayer->graphViewAccess_getVBOVertexesNumber());

#   ifdef RENDER_DEBUG
    processGLErrors("draw end");
#   endif //RENDER_DEBUG
}

//- - - - - - - - - - - - - - - - - Viewport control - - - - - - - - - - - - - - - - - - - - - - - -
void IQGraphView::setViewport(float inWidth, float inHeight) {
    _viewportWidth = inWidth;
    _viewportHeight = inHeight;

    glViewport(0.f, 0.f, _viewportWidth, _viewportHeight);

#   ifdef RENDER_DEBUG
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Setup viewport: [%f, %f]",
                        _viewportWidth, _viewportHeight);
#   endif //RENDER_DEBUG
}

//- - - - - - - - - - - - - - - - - - Frame control - - - - - - - - - - - - - - - - - - - - - - - -
void IQGraphView::setFrame(float inX1, float inY1, float inX2, float inY2) {
    _frameBegin.x = inX1; _frameBegin.y = inY1;
    _frameEnd.x = inX2; _frameEnd.y = inY2;

#   ifdef RENDER_DEBUG
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Setup frame: [[%f, %f], [%f, %f]]",
                        _frameBegin.x, _frameBegin.y, _frameEnd.x, _frameEnd.y);
#   endif //RENDER_DEBUG
}

float IQGraphView::helper_getWorldX(float inX) {
    return _frameBegin.x + inX/_viewportWidth*(_frameEnd.x - _frameBegin.x);
}

void IQGraphView::setFrameX(
        float inX1ToBind, float inX2ToBind, float inBindingX1, float inBindingX2)
{
#   ifdef RENDER_DEBUG
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Set frame X (screen): [%f, %f] -> [%f, %f]",
                        inBindingX1, inBindingX2, inX1ToBind, inX2ToBind);
#   endif //RENDER_DEBUG

    //Collect data
    float theWorldX1New = helper_getWorldX(inBindingX1);
    float theWorldX2New = helper_getWorldX(inBindingX2);

    float theWorldX1Old = helper_getWorldX(inX1ToBind);
    float theWorldX2Old = helper_getWorldX(inX2ToBind);

#   ifdef RENDER_DEBUG
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "world [%f, %f] -> [%f, %f]",
                        theWorldX1Old, theWorldX2Old, theWorldX1New, theWorldX2New);
#   endif //RENDER_DEBUG

    float theFrameX1Old = _frameBegin.x;
    float theFrameX2Old = _frameEnd.x;

    //Get new frame position
    _frameBegin.x = (theFrameX1Old * (theWorldX1Old - theWorldX2Old)
                + theWorldX2Old*theWorldX1New - theWorldX1Old * theWorldX2New) /
               (theWorldX1New - theWorldX2New);

    _frameEnd.x = ((theFrameX2Old - theFrameX1Old)*theWorldX1Old +
            _frameBegin.x * (theWorldX1New - theFrameX2Old)) /
             (theWorldX1New - theFrameX1Old);
}

//- - - - - - - - - - - - - - - - - - Graph layers - - - - - - - - - - - - - - - - - - - - - - -
void IQGraphView::setGraphLayer(IQGraphViewLayer *inGraphLayer) {
    _graphLayer = inGraphLayer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Refs:
//1. http://www.learnopengles.com/calling-opengl-from-android-using-the-ndk/
//2. https://www.scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/orthographic-projection-matrix
//3. https://stackoverflow.com/questions/13633395/how-do-you-access-the-individual-elements-of-a-glsl-mat4
//4. https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glVertexAttribPointer.xhtml
//5. https://stackoverflow.com/questions/25053786/opengl-es-2-on-android-how-to-use-vbos
//6. https://stackoverflow.com/questions/8704801/glvertexattribpointer-clarification
//7. https://arm-software.github.io/opengl-es-sdk-for-android/vbos.html
//8. https://stackoverflow.com/questions/15722803/opengl-shader-error-1282

//Others:
//1. https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glDebugMessageCallback.xhtml - won't help...
//2. https://stackoverflow.com/questions/23066181/glgetuniformlocation-gives-glerror-gl-invalid-value - won't help...

// GLM matrix access:
//        [0]_  [1]_  [2]_  [3]_
// _[0]  [0][0]  .     .     .
// _[1]    .     .     .     .
// _[2]    .     .     .     .
// _[3]    .     .  [2][3]   .
