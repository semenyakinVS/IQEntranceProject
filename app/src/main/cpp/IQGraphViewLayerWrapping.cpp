//
// Created by test on 5/23/2018.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <jni.h>

#include "IQWrappedClassManager.h"
#include "IQGraphViewLayer.h"

#include <android/log.h>

////////////////////////////////////////////////////////////////////////////////////////////////////
static IQWrappedClassManager<IQGraphViewLayer> gClassManager;

////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" {
    //------------------------------------ Methods -------------------------------------------------
    //- - - - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - - - -
    jint JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphViewLayer_nativeCreateInstance(
            JNIEnv *inJavaEnvironment, jobject /* this */, jfloatArray inArray)
    {
        jsize thePointsCount = inJavaEnvironment->GetArrayLength(inArray)/2;
        jfloat *thePointData = inJavaEnvironment->GetFloatArrayElements(inArray, 0);

        return gClassManager.createInstance(thePointData, thePointsCount);
    }

    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphViewLayer_nativeDeleteInstance(
            JNIEnv *env, jobject /* this */, jint inInstanceID)
    {
        gClassManager.deleteInstance(inInstanceID);
    }
}

//================================= Wrappers interaction ===========================================
IQGraphViewLayer *wrappersInteraction_getGraphViewInstance(int inInstanceID) {
    return gClassManager.getInstance(inInstanceID);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
