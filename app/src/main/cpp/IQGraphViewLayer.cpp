//
// Created by test on 5/19/2018.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#define GRAPH_DEBUG

#include "IQGraphViewLayer.h"
#include "IQUtils.h"

#include <vector>
#include <cstring>

#ifdef GRAPH_DEBUG
#include <android/log.h>
#endif //GRAPH_DEBUG

////////////////////////////////////////////////////////////////////////////////////////////////////
//--------------------------------- Methods --------------------------------------------------------
//- - - - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - - - - - -
IQGraphViewLayer::IQGraphViewLayer(const float *inPointData, int inPointsCount)
        : _vboID(0), _vboVertexesNumber(0), _pointData(nullptr), _pointsCount(inPointsCount)
{
    if (!inPointData) return;

    //NB: It's more safe to copy the data - to prevent memory problems on external java management
    // of the memory.
    float *theData = new float[inPointsCount*2];
    memcpy(theData, inPointData, sizeof(float)*inPointsCount*2);
    _pointData = theData;

#   ifdef GRAPH_DEBUG
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Set up points for graph");
#   endif //GRAPH_DEBUG
}

IQGraphViewLayer::~IQGraphViewLayer() {
    internal_clearVBO();
    if (nullptr != _pointData) delete _pointData;
}

//- - - - - - - - - - - - - - - - - Rendering data lifecycle - - - - - - - - - - - - - - - - - - - -
//@ - - - - - - - - - - - - - - Initializing rendering data VBO - - - - - - - - - - - - - - - - - -@
void helper_pushVertexPosition(
        std::vector<float> &outVertexPosition, float inX, float inY)
{
    outVertexPosition.push_back(inX);
    outVertexPosition.push_back(inY);
}

float helper_getX(const float *inData) { return *(inData + 0); }
float helper_getY(const float *inData) { return *(inData + 1); }

float helper_middleZeroYX(const float *inPoint1Data, const float *inPoint2Data)
{
    const float x1 = helper_getX(inPoint1Data), y1 = helper_getY(inPoint1Data);
    const float x2 = helper_getX(inPoint2Data), y2 = helper_getY(inPoint2Data);

    return (y1*x2 - y2*x1)/(y1 - y2);
}

void IQGraphViewLayer::graphViewAccess_initDLData() {
    if (!_pointData) return;

#   ifdef RENDER_DEBUG
    processGLErrors("graph data init begin");
#   endif //RENDER_DEBUG

    //NB: Method build vertex data for drawing using gl_triangle_strip drawing mode

    //Vector used for easier manipulating with buffer. Stores data for 2D vertexes
    //NB: We multiply to 2 for getting coordinates count. And then doing another multiply for render
    // purpose. As a rule we'll need to vertexes per each point (point+point on axis for triangle).
    std::vector<GLfloat> theVertexPositions; theVertexPositions.reserve(_pointsCount*2 *2);

    //Process others
    IQComparationResult theLastZeroComparision = compareFloats(helper_getY(_pointData), 0.f);
    bool theIsZeroSequence = false;

    for (int theIndex = 0; theIndex < _pointsCount; ++theIndex) {

        const float *thePointData = _pointData + theIndex*2;

#       ifdef GRAPH_DEBUG
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "~~~~~~");
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "%d: %f %f",
                            theIndex, helper_getX(thePointData), helper_getY(thePointData));
#       endif //GRAPH_DEBUG

        IQComparationResult theZeroComparison = compareFloats(helper_getY(thePointData), 0.f);

        if (IQComparationResult::More == theZeroComparison ||
                IQComparationResult::Less == theZeroComparison)
        {
            if (theIsZeroSequence) {
                //NB: This is copy-paste code [0]
                //__android_log_print(ANDROID_LOG_WARN, "IQ_APP", "more finish zero sequence");

                //Close zero sequence by creating another one degenerative triangle. Add last zero
                // point twice
                const float *thePreviousPointData = thePointData - 2;
                const float theX = helper_getX(thePreviousPointData);
                helper_pushVertexPosition(theVertexPositions, theX, 0.f);
                helper_pushVertexPosition(theVertexPositions, theX, 0.f);

                theIsZeroSequence = false;
            } else if (theZeroComparison != theLastZeroComparision) {
                //__android_log_print(ANDROID_LOG_WARN, "IQ_APP", "from less to more");

                //If signs of current and previous points are not same - generate helping vertex
                // on intersection of graph and X axis:
                //
                //           _.current vertex [n+2]               . previous vertex [n]
                //           /|                                   |\
                //         /  v/axis vertex [n+3]                 v \ /middle vertex [n+1]
                //  -.--_.----.-                                 -.->.->.---
                //   |  /\middle vertex [n+1]                         \ |\axis vertex [n+2]
                //   v/                                                \v
                //   . previous vertex [n]                              . current vertex [n+3]
                //
                const float *thePreviousPointData = thePointData - 2;

#               ifdef GRAPH_DEBUG
                __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "%f %f",
                                    helper_getX(thePreviousPointData),
                                    helper_getY(thePreviousPointData));
#               endif //GRAPH_DEBUG

                helper_pushVertexPosition(theVertexPositions,
                                          helper_middleZeroYX(thePreviousPointData, thePointData),
                                          0.f);
            }

            const float theX = helper_getX(thePointData);
            const float theY = helper_getY(thePointData);
            if (IQComparationResult::More == theZeroComparison) {
                helper_pushVertexPosition(theVertexPositions, theX, theY);
                helper_pushVertexPosition(theVertexPositions, theX, 0.f);
            } else if (IQComparationResult::Less == theZeroComparison) {
                helper_pushVertexPosition(theVertexPositions, theX, 0.f);
                helper_pushVertexPosition(theVertexPositions, theX, theY);
            }
        } else if (IQComparationResult::Equals == theZeroComparison) {
            if (IQComparationResult::Equals != theLastZeroComparision) {
#               ifdef GRAPH_DEBUG
                __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "start zero sequence");
#               endif ///GRAPH_DEBUG

                //Create degenerative triangle for "collecting" drawing vertexes in
                // one position. We should do it to prevent artifacts on strip render
                const float theX = helper_getX(thePointData);
                helper_pushVertexPosition(theVertexPositions, theX, 0.f);
                helper_pushVertexPosition(theVertexPositions, theX, 0.f);
            } else {
#               ifdef GRAPH_DEBUG
                __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "continue zero sequence");
#               endif ///GRAPH_DEBUG

                //NB: We ignore all zero points after first - to reduce number of unnecessary
                // degenerative triangles. Just save status - we have sequence of zero points.
                // We should close sequence on nonzero vertex appears.
                theIsZeroSequence = true;
            }
        } else {
            //TODO: Put here an error notification - inaccessible branch
        }

        theLastZeroComparision = theZeroComparison;
    }

    const int theVBOSize = theVertexPositions.size();

#ifdef GRAPH_DEBUG
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "-------------------------------------------");
    const float *theVBOData = theVertexPositions.data();
    for (int theIndex = 0; theIndex < theVBOSize/2; ++theIndex) {
        const float *thePointData = theVBOData + theIndex*2;
        __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "%f %f",
                helper_getX(thePointData), helper_getY(thePointData));
    }
    __android_log_print(ANDROID_LOG_WARN, "IQ_APP", "Graph VBO size: %d", theVBOSize);
#endif //GRAPH_DEBUG

    _vboVertexesNumber = theVBOSize/2;
    _vboID = createVBO(theVertexPositions.data(), sizeof(GLfloat)*theVBOSize,
                       GL_ARRAY_BUFFER, GL_STATIC_DRAW);

    //We don't need points data now - it was converted to VBO vertex data and sent to the GPU.
    // Remove it's CPU copy.
    delete _pointData; _pointData = nullptr;

#   ifdef RENDER_DEBUG
    processGLErrors("graph data init end");
#   endif //RENDER_DEBUG
}

//@ - - - - - - - - - - - - - - Deinitializing rendering data VBO - - - - - - - - - - - - - - - - -@
void IQGraphViewLayer::graphViewAccess_deinitDLData() {
    internal_clearVBO();
}

//- - - - - - - - - - - - - - - - Clearing - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void IQGraphViewLayer::internal_clearVBO() {
    if (0 == _vboID) return;

    //NB: glDeleteBuffers(...) set _vboID to zero on call
    glDeleteBuffers(1, &_vboID);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
