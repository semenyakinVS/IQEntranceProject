////////////////////////////////////////////////////////////////////////////////////////////////////
package com.example.graphicsviewer.graphicsviewer;

////////////////////////////////////////////////////////////////////////////////////////////////////
public class IQGraphViewLayer {
    //-------------------------------------- Methods -----------------------------------------------
    //- - - - - - - - - - - - - - - - - - Memory lifecycle - - - - - - - - - - - - - - - - - - - - -
    public IQGraphViewLayer(float[] inPointData) {
        _cppInstanceID = nativeCreateInstance(inPointData);
    }

    protected int wrappersInteraction_cppInstanceID() { return _cppInstanceID; }

    //- - - - - - - - - - - - - - - - - - - C++ binding - - - - - - - - - - - - - - - - - - - - - -
    //NB: We may call System.loadLibrary(...) multiple time - for all wrapper class
    static { System.loadLibrary("native-lib"); }

    //@ - - - - - - - - - - - - - - - - -Memory lifecycle - - - - - - - - - - - - - - - - - - - - -@
    private native int nativeCreateInstance(float[] inPointData);

    //TODO: Perform correct delete of instance
    private native void nativeDeleteInstance(int inInstanceID);

    //--------------------------------------- State ------------------------------------------------
    private int _cppInstanceID;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
