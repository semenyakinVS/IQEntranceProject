////////////////////////////////////////////////////////////////////////////////////////////////////
package com.example.graphicsviewer.graphicsviewer;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLSurfaceView;
import android.os.Build;
import android.view.MotionEvent;
import android.view.View;

import java.util.Vector;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

////////////////////////////////////////////////////////////////////////////////////////////////////
public class IQGraphView extends GLSurfaceView {
    //------------------------------------- Methods ------------------------------------------------
    //- - - - - - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - -
    public IQGraphView(Context inContext) {
        super(inContext);

        //Initialize state
        _cppInstanceID = nativeCreateInstance();

        _layers = new Vector<IQGraphViewLayer>();

        _screenX1Old = 0.0f;
        _screenX2Old = 0.0f;

        //Test GL support
        ActivityManager activityManager
                = (ActivityManager)inContext.getSystemService(Context.ACTIVITY_SERVICE);

        ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();

        final boolean supportsEs2 =
                configurationInfo.reqGlEsVersion >= 0x20000 || helper_isProbablyEmulator();
        if (!supportsEs2) {
            // Should never be seen in production, since the manifest filters
            // unsupported devices.
            //Toast.makeText(this, "This device does not support OpenGL ES 2.0.",
            //        Toast.LENGTH_LONG).show();
            //TODO: Notify here an error - GL not supported
            return;
        }

        //Setup GL
        //-Setup for emulator
        if (helper_isProbablyEmulator()) {
            // Avoids crashes on startup with some emulator images.
            setEGLConfigChooser(8, 8, 8, 8, 16, 0);
        }

        //-Setup GL setting
        //NB: The order of methods calling should be next:
        //1. super(context);
        //2. setEGLContextClientVersion(...);
        //3. setRenderer (...);
        //4. setRenderMode(...);
        //Another order will cause a crash

        setEGLContextClientVersion(2);

        //-Setup renderer
        setRenderer(new Renderer() {
            @Override
            public void onSurfaceCreated(GL10 unused, EGLConfig config) {
                nativeReinit(_cppInstanceID);
            }

            @Override
            public void onSurfaceChanged(GL10 unused, int inWidth, int inHeight) {
                nativeSetViewport(_cppInstanceID, inWidth, inHeight);
                nativeDraw(_cppInstanceID);
            }

            @Override
            public void onDrawFrame(GL10 unused) {
                nativeDraw(_cppInstanceID);
            }
        });
        setRenderMode(android.opengl.GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        //Setup touch event routing for gestures processing
        setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent inEvent) {
                switch (inEvent.getActionMasked()) {
                    case MotionEvent.ACTION_DOWN:
                    case MotionEvent.ACTION_POINTER_DOWN:
                        if (inEvent.getActionIndex() == 0) {
                            _screenX1Old = inEvent.getX(0);
                        } else if (inEvent.getActionIndex() == 1) {
                            _screenX2Old = inEvent.getX(1);
                        } else {
                            //NB: We drop other touches
                        }
                        break;

                    case MotionEvent.ACTION_MOVE:
                        if (inEvent.getPointerCount() < 2) break;

                        float theScreenX1New = inEvent.getX(0);
                        float theScreenX2New = inEvent.getX(1);

                        nativeSetFrameX(_cppInstanceID,
                                _screenX1Old, _screenX2Old, theScreenX1New, theScreenX2New);

                        requestRender();

                        _screenX1Old = theScreenX1New;
                        _screenX2Old = theScreenX2New;

                        break;
                }
                return true;
            }
        });

        nativeSetFrame(_cppInstanceID, 0.0f, -3.0f, 10.0f, 4.0f);
        requestRender();
    }

    private boolean helper_isProbablyEmulator() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH_MR1
                && (Build.FINGERPRINT.startsWith("generic")
                || Build.FINGERPRINT.startsWith("unknown")
                || Build.MODEL.contains("google_sdk")
                || Build.MODEL.contains("Emulator")
                || Build.MODEL.contains("Android SDK built for x86"));
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        nativeDeleteInstance(_cppInstanceID);
    }

    //- - - - - - - - - - - - - - - - - - Graph layer  - - - - - - - - - - - - - - - - - - - - - - -
    //TODO: Use here declared constant for "native-null" value (or even shared with C++ code)
    public void setActiveGraphLayer(IQGraphViewLayer inLayer) {
        if (!_layers.contains(inLayer)) _layers.add(inLayer);

        nativeSetActiveGraphLayer(_cppInstanceID, (null == inLayer) ?
                -1 : inLayer.wrappersInteraction_cppInstanceID());

        requestRender();
    }

    public void linkLayer(IQGraphViewLayer inLayer) {
        if (inLayer != null && !_layers.contains(inLayer)) _layers.add(inLayer);

        nativeLinkGraphLayer(_cppInstanceID, (null == inLayer) ?
                -1 : inLayer.wrappersInteraction_cppInstanceID());
    }

    //TODO: Java methods provide immidiatly owning/unowning of Layers, while native code do\
    // deferred. It may cause an error - when unlinking. Fix it later. Best way to fix - remove this
    // methods from Java user API and control linking/unlinking only in native code (but this will
    // not fix the problem as itself).
    public void unlinkLayer(IQGraphViewLayer inLayer) {
        if (inLayer != null && _layers.contains(inLayer)) _layers.remove(inLayer);

        nativeUnlinkGraphLayer(_cppInstanceID, (null == inLayer) ?
                -1 : inLayer.wrappersInteraction_cppInstanceID());

        //Redraw after possible unlinking of active graph
        requestRender();
    }

    //- - - - - - - - - - - - - - - - - - - C++ binding - - - - - - - - - - - - - - - - - - - - - -
    //NB: We may call System.loadLibrary(...) multiple time - for all wrapper class
    static { System.loadLibrary("native-lib"); }

    //@ - - - - - - - - - - - - - - - - -Memory lifecycle - - - - - - - - - - - - - - - - - - - - -@
    private native int nativeCreateInstance();
    private native void nativeDeleteInstance(int inInstanceID);

    //@ - - - - - - - - - - - - - - - - -Render lifecycle - - - - - - - - - - - - - - - - - - - - -@
    private native void nativeReinit(int inInstanceID);
    private native void nativeDraw(int inInstanceID);

    //@ - - - - - - - - - - - - - - - - -Viewport control - - - - - - - - - - - - - - - - - - - - -@
    private native void nativeSetViewport(int inInstanceID, float inWidth, float inHeight);

    //@ - - - - - - - - - - - - - - - - - Frame control - - - - - - - - - - - - - - - - - - - - - -@
    private native void nativeSetFrame(int inInstanceID,
                                       float inX1, float inY1, float inX2, float inY2);

    private native void nativeSetFrameX(int inInstanceID,
            float inX1ToBind, float inX2ToBind, float inBindingX1, float inBindingX2);

    //- - - - - - - - - - - - - - - - - - Graph layers - - - - - - - - - - - - - - - - - - - - - - -
    private native void nativeSetActiveGraphLayer(int inInstanceID, int inGraphInstanceID);

    private native void nativeLinkGraphLayer(int inInstanceID, int inGraphInstanceID);
    private native void nativeUnlinkGraphLayer(int inInstanceID, int inGraphInstanceID);

    //--------------------------------------- State ------------------------------------------------
    private int _cppInstanceID = -1;

    //NB: Layer are connected in native code. We save layer link in Java code too. Without it
    // connected layer may be removed by garbage collector - and not free it's state correctly
    Vector<IQGraphViewLayer> _layers = null;

    private float _screenX1Old = 0.0f;
    private float _screenX2Old = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//Refs:
//1. http://www.learnopengles.com/calling-opengl-from-android-using-the-ndk/
//2. https://stackoverflow.com/questions/21877127/calling-system-loadlibrary-twice-for-the-same-shared-library
//3. https://developer.android.com/training/gestures/multi
//
//About order of gl initializing calls
//4. https://stackoverflow.com/questions/12115655/why-is-my-opengl-code-failing
//
//Rerun on orientation change
//5. https://stackoverflow.com/questions/19067089/onsurfacecreated-called-every-time-orientation-changes
//6. https://developer.android.com/guide/topics/resources/runtime-changes
//
//Finalizing
//7. https://thebreakfastpost.com/2012/02/09/wrapping-a-c-library-with-jni-part-3/
