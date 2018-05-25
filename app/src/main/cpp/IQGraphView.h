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

//Thread-safe deferred actions support
#include <mutex>

//Utils
#include <vector>

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
private:
    void internal_init();
public:
    //NB: Called when GL context was lost
    void reinit();
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

    //- - - - - - - - - - - - - - - - - - Layers - - - - - - - - - - - - - - - - - - - - - - - - - -
    //NB: Setting graph layer not cause a drawing. It should be called manually
    //NB: Currently layer should be manually unlinked from other graph view
    //NB: This methods should be called only after GL context initialization.
    //TODO: Make linking control automaticly by storing reference to graph view in layers
    //TODO: Maybe, make possible to make deffered init after context creation
    void setActiveGraphLayer(IQGraphViewLayer *inLayer);
private:
    void internal_registerDeferred_setActiveLayer(IQGraphViewLayer *inLayer);
    void internal_implementDeferred_setActiveLayer(IQGraphViewLayer *inLayer);

public:
    void linkGraphLayer(IQGraphViewLayer *inLayer);
private:
    void internal_registerDeferred_linkLayer(IQGraphViewLayer *inLayer);
    void internal_implementDeferred_linkLayer(IQGraphViewLayer *inLayer);

public:
    void unlinkGraphLayer(IQGraphViewLayer *inLayer);
private:
    void internal_registerDeferred_unlinkLayer(IQGraphViewLayer *inLayer);
    void internal_implementDeferred_unlinkLayer(IQGraphViewLayer *inLayer);

    //- - - - - - - - - - - - - - - - Deferred actions processing - - - - - - - - - - - - - - - - -
    //NB: This method created for calling ONLY from draw!
    void helper_processDeferredActions();

    //-------------------------------------- State -------------------------------------------------
private:
    //Graph layers
    IQGraphViewLayer *_activeLayer;
    std::vector<IQGraphViewLayer *> _linkedLayers;

    //Frame info
    glm::vec2 _frameBegin, _frameEnd;
    glm::mat4 _fromToNDC;

    //Viewport info
    float _viewportWidth, _viewportHeight;

    //Deferred actions
    std::mutex _deferredActionsLock;

    //TODO: Make some control for deferred values - to prevent it's untracked changes
    //TODO: Create general class for such actions (like redo/undo manager))
    IQGraphViewLayer *_layerToBeActive; bool _isLayerToBeActiveSetNeed = false;
    std::vector<IQGraphViewLayer *> _layersToLink;
    std::vector<IQGraphViewLayer *> _layersToUnlink;

    //Render state
    GLuint _programID;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //IQ_OPTION_ENTRANCE_TEST_IQGRAPHVIEW_H
