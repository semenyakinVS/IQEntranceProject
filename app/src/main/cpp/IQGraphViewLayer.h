//
// Created by test on 5/19/2018.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef IQ_OPTION_ENTRANCE_TEST_IQGRAPHVIEWLAYER_H
#define IQ_OPTION_ENTRANCE_TEST_IQGRAPHVIEWLAYER_H

////////////////////////////////////////////////////////////////////////////////////////////////////
#include <GLES2/gl2.h>

#include "IQGLUtils.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
class IQGraphViewLayer {
public:
    //----------------------------- Methods --------------------------------------------------------
    //- - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - - - - - -
    //NB: Should be provided vector, sorted by x coordinates
    IQGraphViewLayer(const float *inPointData, int inPointsCount);

    //NB: Currently non-virtual - we have no derived classes
    ~IQGraphViewLayer();

    //- - - - - - - - - - - - - - - - - Render data lifecycle - - - - - - - - - - - - - - - - - - -
private: friend class IQGraphView;
    void graphViewAccess_initGLData();
    void graphViewAccess_deinitGLData();

    GLuint graphViewAccess_getVBOID() {  return _vboID; }
    int graphViewAccess_getVBOVertexesNumber() { return _vboVertexesNumber; }

    //-------------------------------------- State -------------------------------------------------
private:
    //Points
    //NB: Owns point data
    const float *_pointData;
    int _pointsCount;

    //GL state
    GLuint _vboID;
    int _vboVertexesNumber;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//Refs:
//1. http://www.learnopengles.com/tag/degenerate-triangles/

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //IQ_OPTION_ENTRANCE_TEST_IQGRAPHVIEWLAYER_H
