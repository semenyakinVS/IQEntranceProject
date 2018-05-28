////////////////////////////////////////////////////////////////////////////////////////////////////
package com.example.graphicsviewer.graphicsviewer;

////////////////////////////////////////////////////////////////////////////////////////////////////
public class IQGraphViewLayer {
    //-------------------------------------- Methods -----------------------------------------------
    //- - - - - - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - -
    public IQGraphViewLayer(float[] inPointData) {
        _cppInstanceID = nativeCreateInstance(inPointData);
    }

    @Override
    protected void finalize() throws Throwable {
        super.finalize();
        nativeDeleteInstance(_cppInstanceID);
    }

    //- - - - - - - - - - - - - - - - - - - C++ binding - - - - - - - - - - - - - - - - - - - - - -
    //NB: We may call System.loadLibrary(...) multiple time - for all wrapper class
    static { System.loadLibrary("native-lib"); }

    protected int wrappersInteraction_cppInstanceID() { return _cppInstanceID; }

    //@ - - - - - - - - - - - - - - - - -Memory lifecycle - - - - - - - - - - - - - - - - - - - - -@
    private native int nativeCreateInstance(float[] inPointData);
    private native void nativeDeleteInstance(int inInstanceID);

    //--------------------------------------- State ------------------------------------------------
    private int _cppInstanceID = -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
