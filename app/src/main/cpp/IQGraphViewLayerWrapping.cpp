//
// Created by test on 5/23/2018.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "IQGraphViewLayerWrapping.h"

#include "IQGraphViewLayer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
static IQWrappedClassManager<IQGraphViewLayer, 10> gClassManager;

////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" {
    //------------------------------------ Methods -------------------------------------------------
    //- - - - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - - - -
    jint JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphViewLayer_nativeCreateInstance(
            JNIEnv *inJEnv, jobject, jfloatArray inArray)
    {
        int thePointsCount = static_cast<int>(inJEnv->GetArrayLength(inArray)/2);
        const float *thePointData =
                static_cast<const float *>(inJEnv->GetFloatArrayElements(inArray, 0));

        return gClassManager.createInstance(thePointData, thePointsCount);
    }

    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphViewLayer_nativeDeleteInstance(
            JNIEnv *env, jobject, jint inInstanceID)
    {
        gClassManager.deleteInstance(inInstanceID);
    }
}

//================================= Wrappers interaction ===========================================
IQGraphViewLayer *wrappersInteraction_getGraphViewLayerInstance(jint inInstanceID){
    return gClassManager.getInstance(inInstanceID);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
