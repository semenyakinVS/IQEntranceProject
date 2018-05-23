//
// Created by test on 5/23/2018.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <jni.h>

#include "IQWrappedClassManager.h"
#include "IQGraphView.h"

#include "IQGraphViewLayerWrapping.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
static IQWrappedClassManager<IQGraphView> gClassManager;

////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" {
    //------------------------------------ Methods -------------------------------------------------
    //- - - - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - - - -
    jint JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeCreateInstance(
            JNIEnv *, jobject /* this */)
    {
        return gClassManager.createInstance();
    }

    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeDeleteInstance(
            JNIEnv *, jobject /* this */, jint inInstanceID)
    {
        gClassManager.deleteInstance(inInstanceID);
    }

    //- - - - - - - - - - - - - - - - - Render lifecycle - - - - - - - - - - - - - - - - - - - - - -
    void
    JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeInit(
            JNIEnv *, jobject /* this */, jint inInstanceID)
    {
        gClassManager.getInstance(inInstanceID)->init();
    }

    void
    JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeDraw(
            JNIEnv *, jobject /* this */, jint inInstanceID)
    {
        gClassManager.getInstance(inInstanceID)->draw();
    }

    //- - - - - - - - - - - - - - - - - Viewport control - - - - - - - - - - - - - - - - - - - - - -
    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeSetViewport(
            JNIEnv *, jobject /* this */, jint inInstanceID,
            jfloat inWidth, jfloat inHeight)
    {
        gClassManager.getInstance(inInstanceID)->setViewport(inWidth, inHeight);
    }

    //- - - - - - - - - - - - - - - - - Frame control - - - - - - - - - - - - - - - - - - - - - - -
    void JNICALL
    Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeSetFrame(
            JNIEnv *, jobject /* this */, jint inInstanceID,
            jfloat inX1, jfloat inY1, jfloat inX2, jfloat inY2)
    {
        gClassManager.getInstance(inInstanceID)->setFrame(inX1, inY1, inX2, inY2);
    }

    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeSetFrameX(
            JNIEnv *, jobject /* this */, jint inInstanceID,
            jfloat inX1ToBind, jfloat inX2ToBind, jfloat inBindingX1, jfloat inBindingX2)
    {
        gClassManager.getInstance(inInstanceID)->setFrameX(
                inX1ToBind, inX2ToBind, inBindingX1, inBindingX2);
    }

    //- - - - - - - - - - - - - - - - - - Graph layers - - - - - - - - - - - - - - - - - - - - - - -
    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeSetGraphLayer(
            JNIEnv *, jobject /* this */, jint inInstanceID,
            jint inGraphInstanceID)
    {
        gClassManager.getInstance(inInstanceID)->setGraphLayer(
                wrappersInteraction_getGraphViewInstance(inGraphInstanceID));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
