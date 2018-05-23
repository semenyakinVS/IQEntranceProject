//
// Created by test on 5/23/2018.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef IQ_OPTION_ENTRANCE_TEST_IQGRAPHVIEW_H
#define IQ_OPTION_ENTRANCE_TEST_IQGRAPHVIEW_H

////////////////////////////////////////////////////////////////////////////////////////////////////
//GL
//-GL utils
//NB: GL includes are there
#include "IQGLUtils.h"

//-GL math
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////
class IQGraphViewLayer;

////////////////////////////////////////////////////////////////////////////////////////////////////
class IQGraphView {
public:
    //------------------------------------- Methods ------------------------------------------------
    //- - - - - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - - -
    IQGraphView();

    //NB: Currently non-virtual - we have no derived classes
    ~IQGraphView(); //TODO: Implement

    //- - - - - - - - - - - - - - - - - Render lifecycle - - - - - - - - - - - - - - - - - - - - - -
    void init();
    void draw();

    //- - - - - - - - - - - - - - - - - Viewport control - - - - - - - - - - - - - - - - - - - - - -
    void setViewport(float inWidth, float inHeight);

    //- - - - - - - - - - - - - - - - - Frame control - - - - - - - - - - - - - - - - - - - - - - -
    //NB: Setting frame not cause a drawing. It should be called manually
    void setFrame(float inX1, float inY1, float inX2, float inY2);

private:
    float helper_getWorldX(float inX);
public:
    void setFrameX(float inX1ToBind, float inX2ToBind, float inBindingX1, float inBindingX2);

    //- - - - - - - - - - - - - - - - - - Graph layers - - - - - - - - - - - - - - - - - - - - - - -
    //NB: Setting graph layer not cause a drawing. It should be called manually
    void setGraphLayer(IQGraphViewLayer *inGraphLayer);

    //-------------------------------------- State -------------------------------------------------
private:
    //Graph layers
    IQGraphViewLayer *_graphLayer;

    //Frame info
    glm::vec2 _frameBegin, _frameEnd;
    glm::mat4 _fromToNDC;

    //Render state
    GLuint _programID;

    //Viewport info
    float _viewportWidth, _viewportHeight;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //IQ_OPTION_ENTRANCE_TEST_IQGRAPHVIEW_H
