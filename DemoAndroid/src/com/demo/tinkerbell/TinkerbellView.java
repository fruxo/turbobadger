package com.demo.tinkerbell;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.View;
import android.view.SurfaceHolder;
import android.view.MotionEvent;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

class EventRunnable implements Runnable {
	float m_x, m_y, m_x2, m_y2;
	int m_action;
	public EventRunnable(float x, float y, float x2, float y2, int action) {
		m_x = x;
		m_y = y;
		m_x2 = x2;
		m_y2 = y2;
		m_action = action;
	}
	public void run() {
		if (m_action == MotionEvent.ACTION_POINTER_DOWN) {
			TinkerbellLib.OnPointer2(m_x2, m_y2, 1);
		}
		else if (m_action == MotionEvent.ACTION_POINTER_UP) {
			TinkerbellLib.OnPointer2(m_x2, m_y2, 0);
		}
		else if (m_action == MotionEvent.ACTION_DOWN) {
			TinkerbellLib.OnPointer(m_x, m_y, 1);
		}
		else if (m_action == MotionEvent.ACTION_UP) {
			TinkerbellLib.OnPointer(m_x, m_y, 0);
		}
		else if (m_action == MotionEvent.ACTION_MOVE) {
			TinkerbellLib.OnPointerMove(m_x, m_y, m_x2, m_y2);
		}
		// FIX: ACTION_SCROLL
	}
}

class TinkerbellView extends GLSurfaceView
{
	public TinkerbellView(Context context) {
		super(context);

		// FIX: Use this when i have timer code in place, for correct scheduleing.
		// setRenderMode(RENDERMODE_WHEN_DIRTY);

		// Create an OpenGL ES 2.0 context
		//setEGLContextClientVersion(2);

		// Choose 32bit and no depth buffer.
		//setEGLConfigChooser(this);

		// Set the renderer associated with this view
		setRenderer(new TinkerbellRenderer());
	}

	/*@Override public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
		Phint[] num_conf = new int[1];
		egl.eglGetConfigs(display, null, 0, num_conf);
		for (int i = 0; i < num_conf[0]; i++) {

		}
		return null;
	}*/

	@Override public void onPause() {
		super.onPause();
		// FIX: We should probably wait here until the queued event has been processed.
		queueEvent(new Runnable() {
			public void run() {
				TinkerbellLib.OnPauseApp();
			}});
	}

	@Override public void onResume() {
		super.onResume();
		queueEvent(new Runnable() {
			public void run() {
				TinkerbellLib.OnResumeApp();
			}});
	}

	public boolean onTouchEvent(final MotionEvent event) {
			float x2 = 0;
			float y2 = 0;
			if (event.getPointerCount() > 1) {
				x2 = event.getX(1);
				y2 = event.getY(1);
			}
			queueEvent(new EventRunnable(event.getX(), event.getY(), x2, y2, event.getAction()));
			return true;
	}

	public void surfaceDestroyed(SurfaceHolder holder) {
		super.surfaceDestroyed(holder);
		queueEvent(new Runnable() {
			public void run() {
				TinkerbellLib.OnContextLost();
			}});
	}

	private static class TinkerbellRenderer implements GLSurfaceView.Renderer
	{
		public void onDrawFrame(GL10 gl) {
			TinkerbellLib.RunSlice();
		}

		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
			if (TinkerbellLib.sInitiated) {
				TinkerbellLib.OnContextRestored();
			}
		}

		public void onSurfaceChanged(GL10 gl, int width, int height) {
			if (!TinkerbellLib.sInitiated) {
				TinkerbellLib.sInitiated = true;
				TinkerbellLib.InitApp(width, height, "", "");
			}
			TinkerbellLib.OnSurfaceResized(width, height);
		}
	}
}
