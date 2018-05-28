//
// Created by test on 5/23/2018.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "IQWrappedClassManager.h"

#include "IQGraphView.h"
#include "IQGraphViewLayerWrapping.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
static IQWrappedClassManager<IQGraphView, 2> gClassManager;

////////////////////////////////////////////////////////////////////////////////////////////////////
extern "C" {
    //------------------------------------ Methods -------------------------------------------------
    //- - - - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - - - -
    jint JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeCreateInstance(
            JNIEnv *, jobject)
    {
        return gClassManager.createInstance();
    }

    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeDeleteInstance(
            JNIEnv *, jobject, jint inInstanceID)
    {
        gClassManager.deleteInstance(inInstanceID);
    }

    //- - - - - - - - - - - - - - - - - Render lifecycle - - - - - - - - - - - - - - - - - - - - - -
    void
    JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeReinit(
            JNIEnv *, jobject, jint inInstanceID)
    {
        gClassManager.getInstance(inInstanceID)->reinit();
    }

    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeDraw(
            JNIEnv *, jobject, jint inInstanceID)
    {
        gClassManager.getInstance(inInstanceID)->draw();
    }

    //- - - - - - - - - - - - - - - - - Viewport control - - - - - - - - - - - - - - - - - - - - - -
    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeSetViewport(
            JNIEnv *, jobject, jint inInstanceID,
            jfloat inWidth, jfloat inHeight)
    {
        gClassManager.getInstance(inInstanceID)->setViewport(inWidth, inHeight);
    }

    //- - - - - - - - - - - - - - - - - Frame control - - - - - - - - - - - - - - - - - - - - - - -
    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeSetFrame(
            JNIEnv *, jobject, jint inInstanceID,
            jfloat inX1, jfloat inY1, jfloat inX2, jfloat inY2)
    {
        gClassManager.getInstance(inInstanceID)->setFrame(inX1, inY1, inX2, inY2);
    }

    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeSetFrameX(
            JNIEnv *, jobject, jint inInstanceID,
            jfloat inX1ToBind, jfloat inX2ToBind, jfloat inBindingX1, jfloat inBindingX2)
    {
        gClassManager.getInstance(inInstanceID)->setFrameX(
                inX1ToBind, inX2ToBind, inBindingX1, inBindingX2);
    }

    //- - - - - - - - - - - - - - - - - - Graph layers - - - - - - - - - - - - - - - - - - - - - - -
    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeSetActiveGraphLayer(
            JNIEnv *, jobject, jint inInstanceID,
            jint inGraphInstanceID)
    {
        gClassManager.getInstance(inInstanceID)->setActiveGraphLayer(
                wrappersInteraction_getGraphViewLayerInstance(inGraphInstanceID));
    }

    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeLinkGraphLayer(
            JNIEnv *, jobject, jint inInstanceID,
            jint inGraphInstanceID)
    {
        gClassManager.getInstance(inInstanceID)->linkGraphLayer(
                wrappersInteraction_getGraphViewLayerInstance(inGraphInstanceID));
    }

    void JNICALL
        Java_com_example_graphicsviewer_graphicsviewer_IQGraphView_nativeUnlinkGraphLayer(
            JNIEnv *, jobject, jint inInstanceID,
            jint inGraphInstanceID)
    {
        gClassManager.getInstance(inInstanceID)->unlinkGraphLayer(
                wrappersInteraction_getGraphViewLayerInstance(inGraphInstanceID));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
