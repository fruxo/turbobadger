package com.demo.tinkerbell;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.inputmethod.InputMethodManager;
import android.view.MotionEvent;
import android.view.SurfaceHolder;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
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
	private static TinkerbellView instance;
	private static boolean show_keyboard;
	private TinkerbellRenderer renderer;

	public TinkerbellView(Context context) {
		super(context);

		instance = this;

		// Make focusable so the keyboard works.
		setFocusableInTouchMode(true);

		// FIX: Use this when i have timer code in place, for correct scheduleing.
		// setRenderMode(RENDERMODE_WHEN_DIRTY);

		// Create an OpenGL ES 2.0 context
		//setEGLContextClientVersion(2);

		// Choose 32bit and no alpha, depth or stencil buffer.
		setEGLConfigChooser(8, 8, 8, 0, 0, 0);

		// Set the renderer associated with this view
		renderer = new TinkerbellRenderer();
		setRenderer(renderer);
	}

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

	@Override public boolean onTouchEvent(final MotionEvent event) {
			float x2 = 0;
			float y2 = 0;
			if (event.getPointerCount() > 1) {
				x2 = event.getX(1);
				y2 = event.getY(1);
			}
			queueEvent(new EventRunnable(event.getX(), event.getY(), x2, y2, event.getAction()));
			return true;
	}

	@Override public void surfaceDestroyed(SurfaceHolder holder) {
		super.surfaceDestroyed(holder);
		queueEvent(new Runnable() {
			public void run() {
				TinkerbellLib.OnContextLost();
			}});
	}

	private static class TinkerbellRenderer implements GLSurfaceView.Renderer
	{
		private int m_width, m_height;
		public void onDrawFrame(GL10 gl) {

			if (EGLContext.getEGL() instanceof EGL10) {
				// There seems to be a backend bug (race condition?) that we sometimes
				// get one onDrawFrame call before onSurfaceChanged is called after the
				// surface change size. Work around this by checking the size of the
				// surface and resize the rendering accordingly.
				int w[] = { 0 };
				int h[] = { 0 };
				EGL10 egl = (EGL10)EGLContext.getEGL();
				EGLDisplay disp = egl.eglGetCurrentDisplay();
				EGLSurface surf = egl.eglGetCurrentSurface(EGL10.EGL_DRAW);
				if (egl.eglQuerySurface(disp, surf, EGL10.EGL_WIDTH, w) &&
					egl.eglQuerySurface(disp, surf, EGL10.EGL_HEIGHT, h))
					handleSurfaceSizeChanged(w[0], h[0]);
			}

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
			handleSurfaceSizeChanged(width, height);
		}

		private void handleSurfaceSizeChanged(int width, int height) {
			if (!TinkerbellLib.sInitiated ||
				(width == m_width && height == m_height))
				return;
			m_width = width;
			m_height = height;
			TinkerbellLib.OnSurfaceResized(width, height);
		}
	}

	public static void ShowKeyboard(int show) {
		// Post to the view instance so we run on the correct thread.
		// Use some delay to prevent flickering the keyboard off/on
		// when moving from one editable field to another.
		show_keyboard = show == 1;
		instance.postDelayed(new Runnable() {
			@Override
			public void run() {
				InputMethodManager imm = (InputMethodManager)instance.getContext().getSystemService(Context.INPUT_METHOD_SERVICE);
				if (show_keyboard)
					imm.showSoftInput(instance, 0);
				else
					imm.hideSoftInputFromWindow(instance.getWindowToken(), 0);
				Log.d("tinkerbell", show_keyboard ? "ShowKeyboard" : "HideKeyboard");
			}
		}, 20);
	}
}
