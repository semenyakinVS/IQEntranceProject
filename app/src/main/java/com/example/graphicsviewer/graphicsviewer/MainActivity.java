////////////////////////////////////////////////////////////////////////////////////////////////////
package com.example.graphicsviewer.graphicsviewer;

////////////////////////////////////////////////////////////////////////////////////////////////////
//Application
import android.graphics.Point;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;

//GL surface
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.opengl.GLES20;

//import static android.opengl.GLES20.*;

//Emulator support
import android.os.Build;

//Gesture
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.MotionEvent;

//Debugging
import android.util.Log;
import android.widget.Button;
import android.widget.FrameLayout;

////////////////////////////////////////////////////////////////////////////////////////////////////
public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        //Connect button events
        findViewById(R.id.showFirstGraphButton).setOnClickListener(new View.OnClickListener() {
            public void onClick(View unused) {
                _graphView.setActiveGraphLayer(_firstGraphLayer);
            }
        });

        findViewById(R.id.showSecondGraphButton).setOnClickListener(new View.OnClickListener() {
            public void onClick(View unused) {
                _graphView.setActiveGraphLayer(_secondGraphLayer);
            }
        });

        //Setup graph view
        _graphView = new IQGraphView(this);

        FrameLayout theGraphViewContainer = findViewById(R.id.graphViewContainer);
        theGraphViewContainer.addView(_graphView);

        //Create graph layer
        _firstGraphLayer = new IQGraphViewLayer(new float[]{
                1.f, 1.f,
                2.f, 3.f,
                3.f, 2.5f,
                3.5f, -0.5f,
                4.f, -1.f,
                5.f, 1.f,
                5.5f, -1.0f,
                6.f, -2.f,
                7.f, 3.f,
                8.f, 2.f
        });

        _secondGraphLayer = new IQGraphViewLayer(new float[]{
                2.f, -3.f,
                3.f, -2.5f,
                3.5f, 0.5f,
                4.f, 1.f,
                5.f, -1.f,
                5.5f, 1.0f,
                6.f, 2.f,
                7.f, -3.f
        });

        _graphView.setActiveGraphLayer(_firstGraphLayer);
    }

    @Override
    protected void onResume() {
        super.onResume();
        if(_graphView != null) _graphView.onResume();
    }

    @Override
    protected void onPause() {
        super.onPause();
        if (_graphView != null) _graphView.onPause();
    }

    //Views
    IQGraphView _graphView;
    IQGraphViewLayer _firstGraphLayer;
    IQGraphViewLayer _secondGraphLayer;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
