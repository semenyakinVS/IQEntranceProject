//
// Created by test on 5/23/2018.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "IQGraphView.h"

//Graph layer
#include "IQGraphViewLayer.h"

//Utils
#include "algorithm" //for find(...) in graph layer link/unlink methods

#ifdef RENDER_DEBUG
#include <android/log.h>
#include <cstdio> // for sprintf(...) in NDC matrix setup function
#endif //RENDER_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////
static const char *kVertexShaderSource =
        "#version 100                                                                       \n"
        "attribute vec2 vertexPosition;                                                     \n"
        //TODO: Make sending of depth uniform works
        //"uniform float depth;\n"
        "uniform mat4 MVPMatrix;                                                            \n"
        "void main() {                                                                      \n"
        "   gl_Position = MVPMatrix * vec4(vertexPosition.xy, 0.0, 1.0);                    \n"
        "}                                                                                  \n";

static const char *kFragmentShaderSource =
        "#version 100                                                                       \n"
        "void main() {                                                                      \n"
        "   gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);                                        \n"
        "}                                                                                  \n";

////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------------- Methods --------------------------------------------------
//- - - - - - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - - - -
IQGraphView::IQGraphView()
    : _activeLayer(nullptr), _linkedLayers(),
      _frameBegin(-1.f, -1.f), _frameEnd(1.f, 1.f),
      _viewportWidth(1.f), _viewportHeight(1.f),
      _deferredActionsLock(),
      _layerToBeActive(nullptr), _isLayerToBeActiveSetNeed(false),
      _layersToLink(), _layersToUnlink(),
      _programID(0) { }

IQGraphView::~IQGraphView() {
    //TODO: Process destructor with preventing errors on java interaction
}

//- - - - - - - - - - - - - - - - - GL drawing lifecycle - - - - - - - - - - - - - - - - - - - - - -
//@ - - - - - - - - - - - - - - - - - - Initializing - - - - - - - - - - - - - - - - - - - - - - - @
void IQGraphView::internal_init() {
#   ifdef RENDER_DEBUG
    dropGLErrors("graph view init begin");
    printGLInfo();
#   endif //RENDER_DEBUG

    //Setup shader program
    GLuint theVertexShaderID = shaderCompileAndSend(kVertexShaderSource, GL_VERTEX_SHADER);
    if (0 == theVertexShaderID) return;

    GLuint theFragmentShaderID = shaderCompileAndSend(kFragmentShaderSource, GL_FRAGMENT_SHADER);
    if (0 == theFragmentShaderID) return;

    _programID = createAndLinkShaderProgram(theVertexShaderID, theFragmentShaderID);
    if (0 == _programID) return;

#   ifdef RENDER_DEBUG
    printShaderDebugInfo(_programID);
#   endif //RENDER_DEBUG

    //Setup global GL properties
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#   ifdef RENDER_DEBUG
    dropGLErrors("graph view init end");
#   endif //RENDER_DEBUG
}

void IQGraphView::reinit() {
    std::unique_lock<std::mutex> theWaitReinitLock(_deferredActionsLock);

    helper_processDeferredActions();

    internal_init();

    for (IQGraphViewLayer *theLayer : _linkedLayers) {
        theLayer->graphViewAccess_initGLData();
    }
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
    //TODO: Move this code to appropriate function
    const char *kMessageFormat = "Frame to NDC matrix (for %f %f %f %f %f):";
    const int theSize = snprintf(nullptr, 0, kMessageFormat,
                              inFrameBeginX, inFrameBeginY, inFrameEndX, inFrameEndY, inDepthLevel);
    char *theMessage = new char[theSize];
    sprintf(theMessage, kMessageFormat,
                              inFrameBeginX, inFrameBeginY, inFrameEndX, inFrameEndY, inDepthLevel);

    printGLMMat4(outMatrix, theMessage);
#   endif //RENDER_DEBUG
}

void IQGraphView::draw() {
    std::unique_lock<std::mutex> theWaitDeferredActionsAndRenderLock(_deferredActionsLock);

    helper_processDeferredActions();

#   ifdef RENDER_DEBUG
    dropGLErrors("draw begin");
#   endif //RENDER_DEBUG

    //Clear
    glClear(GL_COLOR_BUFFER_BIT);

    //Process layer
    if (!_activeLayer) return;

    //Setup programm
    glUseProgram(_programID);

    //Setup input data
    //-Update and setup MVP Matrix (currently simple transform from frame to NDC)
    //NB: Next two problems may caused because of incorrect program problem
    //TODO: Find why Matrix is sent to shader only if it setup was before VBO with vertex data setup
    //TODO: Find why using of local matrix here cause an error (see ref 11) - next NB is not correct
    //NB: Matrix  be local value - glUniformMatrix4fv(...) send matrix not at the call moment
    //TODO: Left using _fromToNDC as class member - to prevent useless constructor/destructor calls
    helper_fillCameraFrameToNDCMatrix(_fromToNDC,
                                      _frameBegin.x, _frameBegin.y, _frameEnd.x, _frameEnd.y, 1.f);

    GLuint theMVPMatrixID = glGetUniformLocation(_programID, "MVPMatrix");
    glUniformMatrix4fv(theMVPMatrixID, 1, GL_FALSE, glm::value_ptr(_fromToNDC));

    //-Send depth data
    //TODO: Make sending of depth uniform works
    //int theDepthID; glGetUniformLocation(theDepthID, "depth");
    //glUniform1f(theDepthID, 0.f);

    //-Setup vertex data
    //TODO: Using 1xGL_FLOAT_VEC3 instead 3xGL_FLOAT as attribute setting cause crash on draw. Why?
    GLuint theVertexAttributeID = glGetAttribLocation(_programID, "vertexPosition");
    glBindBuffer(GL_ARRAY_BUFFER, _activeLayer->graphViewAccess_getVBOID());
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
    glDrawArrays(GL_TRIANGLE_STRIP, 0, _activeLayer->graphViewAccess_getVBOVertexesNumber());

    glUseProgram(0);

#   ifdef RENDER_DEBUG
    dropGLErrors("draw end");
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

//- - - - - - - - - - - - - - - - - - - - Layers - - - - - - - - - - - - - - - - - - - - - - - - - -
//@ - - - - - - - - - - - - - - - - Setting as active - - - - - - - - - - - - - - - - - - - - - - -@
void IQGraphView::setActiveGraphLayer(IQGraphViewLayer *inLayer) {
    std::unique_lock<std::mutex> theWaitDeferredActionsRegistering(_deferredActionsLock);

    internal_registerDeferred_linkLayer(inLayer);
    internal_registerDeferred_setActiveLayer(inLayer);
}

void IQGraphView::internal_registerDeferred_setActiveLayer(IQGraphViewLayer *inLayer) {
    if (inLayer == _activeLayer) {
        _isLayerToBeActiveSetNeed = false;
    } else {
        _isLayerToBeActiveSetNeed = true;
        _layerToBeActive = inLayer;
    }
}

void IQGraphView::internal_implementDeferred_setActiveLayer(IQGraphViewLayer *inLayer) {
    if (!_isLayerToBeActiveSetNeed) return;

    auto theIterator = std::find(_linkedLayers.begin(), _linkedLayers.end(), inLayer);
    if (_linkedLayers.end() == theIterator){
        //TODO: Maybe drop here message that layer is not connected.\
        // Create argument to control messages then.
        return;
    }
    _activeLayer = inLayer;
}

//@ - - - - - - - - - - - - - - - - - - Linking - - - - - - - - - - - - - - - - - - - - - - - - - -@
void IQGraphView::linkGraphLayer(IQGraphViewLayer *inLayer) {
    std::unique_lock<std::mutex> theWaitDeferredActionsRegistering(_deferredActionsLock);

    internal_registerDeferred_linkLayer(inLayer);
}

void IQGraphView::internal_registerDeferred_linkLayer(IQGraphViewLayer *inLayer) {
    auto theLayerToUnlink = std::find(_layersToUnlink.begin(), _layersToUnlink.end(), inLayer);
    if (theLayerToUnlink != _layersToUnlink.end()) {
        _layersToUnlink.erase(theLayerToUnlink);
    } else {
        _layersToLink.push_back(inLayer);
    }
}

void IQGraphView::internal_implementDeferred_linkLayer(IQGraphViewLayer *inLayer) {
    if (_linkedLayers.end() != std::find(_linkedLayers.begin(), _linkedLayers.end(), inLayer)){
        //TODO: Maybe drop here message that layer was already connected.\
        // Create argument to control it then.
        return;
    }
    inLayer->graphViewAccess_initGLData();
    _linkedLayers.push_back(inLayer);
}

//@ - - - - - - - - - - - - - - - - - - Unlinking - - - - - - - - - - - - - - - - - - - - - - - - -@
void IQGraphView::unlinkGraphLayer(IQGraphViewLayer *inLayer) {
    std::unique_lock<std::mutex> theWaitDeferredActionsRegistering(_deferredActionsLock);

    internal_registerDeferred_unlinkLayer(inLayer);
}

void IQGraphView::internal_registerDeferred_unlinkLayer(IQGraphViewLayer *inLayer) {
    auto theLayerToLink = std::find(_layersToLink.begin(), _layersToLink.end(), inLayer);
    if (theLayerToLink != _layersToLink.end()) {
        _layersToLink.erase(theLayerToLink);
    } else {
        _layersToUnlink.push_back(inLayer);
    }
}

void IQGraphView::internal_implementDeferred_unlinkLayer(IQGraphViewLayer *inLayer) {
    auto theIterator = std::find(_linkedLayers.begin(), _linkedLayers.end(), inLayer);
    if (_linkedLayers.end() == theIterator){
        //TODO: Maybe drop here message that layer is not connected\
           Create argument to control it then
        return;
    }
    (*theIterator)->graphViewAccess_deinitGLData();
    _linkedLayers.erase(theIterator);
}

//- - - - - - - - - - - - - - - - - Deferred actions processing - - - - - - - - - - - - - - - - - -
//NB: This method created for calling ONLY from draw!
void IQGraphView::helper_processDeferredActions() {
    //Unlink first - this may reduce deferred actions buffer realocations
    for (IQGraphViewLayer *theLayer : _layersToUnlink) {
        internal_implementDeferred_unlinkLayer(theLayer);
    }

    for (IQGraphViewLayer *theLayer : _layersToLink) {
        internal_implementDeferred_linkLayer(theLayer);
    }

    internal_implementDeferred_setActiveLayer(_layerToBeActive);
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

//SL version:
//9. https://www.opengl.org/discussion_boards/showthread.php/168930-Problem-with-version
//10. https://stackoverflow.com/questions/27407774/get-supported-glsl-versions/27410925

//GL calls data copying:
//11. https://www.opengl.org/discussion_boards/showthread.php/177309-glUniformMatrix4fv-when-is-the-data-copied
//
//TODO: This ref is used for generating formated string. Move it to appropriate place
//12. https://stackoverflow.com/questions/29087129/how-to-calculate-the-length-of-output-that-sprintf-will-generate

// GLM matrix access:
//        [0]_  [1]_  [2]_  [3]_
// _[0]  [0][0]  .     .     .
// _[1]    .     .     .     .
// _[2]    .     .     .     .
// _[3]    .     .  [2][3]   .
