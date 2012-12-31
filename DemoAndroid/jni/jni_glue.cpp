#include <jni.h>
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "App.h"
#include "tb_widgets.h"
#include "tb_system.h"

using namespace tinkerbell;

// Bug Work around
bool context_lost = false;

// Forward declare set asset manager.
void SetAssetManager(AAssetManager* pManager);

#define JNI_VOID_TINKERBELL(func) JNIEXPORT void JNICALL Java_com_demo_tinkerbell_TinkerbellLib_##func
#define JNI_INT_TINKERBELL(func) JNIEXPORT jint JNICALL Java_com_demo_tinkerbell_TinkerbellLib_##func

extern "C"
{
	JNI_VOID_TINKERBELL(createAssetManager)(JNIEnv* env, jobject obj, jobject assetManager);

	JNI_VOID_TINKERBELL(InitApp)(JNIEnv * env, jobject obj, jint width, jint height, jstring settings_path, jstring custom_data_path);
	JNI_VOID_TINKERBELL(ShutDownApp)(JNIEnv * env, jobject obj);

	JNI_VOID_TINKERBELL(OnPauseApp)(JNIEnv * env, jobject obj);
	JNI_VOID_TINKERBELL(OnResumeApp)(JNIEnv * env, jobject obj);

	JNI_VOID_TINKERBELL(OnPointer)(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint down);
	JNI_VOID_TINKERBELL(OnPointer2)(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint down);
	JNI_VOID_TINKERBELL(OnPointerMove)(JNIEnv * env, jobject obj, jfloat x, jfloat y, jfloat x2, jfloat y2);

	JNI_INT_TINKERBELL(OnBackKey)(JNIEnv * env, jobject obj) {return 0;}
	JNI_INT_TINKERBELL(OnMenuKey)(JNIEnv * env, jobject obj) {return 0;}

	JNI_VOID_TINKERBELL(OnContextLost)(JNIEnv * env, jobject obj);
	JNI_VOID_TINKERBELL(OnContextRestored)(JNIEnv * env, jobject obj);

	JNI_VOID_TINKERBELL(OnSurfaceResized)(JNIEnv * env, jobject obj, jint width, jint height);

	JNI_VOID_TINKERBELL(RunSlice)(JNIEnv * env, jobject obj);
}

JNI_VOID_TINKERBELL(InitApp)(JNIEnv * env, jobject obj, jint width, jint height, jstring settings_path, jstring custom_data_path)
{
	//TBDebugOut("InitApp");
	Init(width, height);
}

JNI_VOID_TINKERBELL(ShutDownApp)(JNIEnv * env, jobject obj)
{
	//TBDebugOut("ShutDownApp");
	Shutdown();
	exit(0);
}

JNI_VOID_TINKERBELL(OnPauseApp)(JNIEnv * env, jobject obj)
{
	//TBDebugOut("OnPauseApp");
}

JNI_VOID_TINKERBELL(OnResumeApp)(JNIEnv * env, jobject obj)
{
	//TBDebugOut("OnResumeApp");
}

JNI_VOID_TINKERBELL(OnContextLost)(JNIEnv * env, jobject obj)
{
	//TBDebugOut("OnContextLost");
	g_renderer->InvokeContextLost(); // Forget all bitmaps
	context_lost = true;
}

JNI_VOID_TINKERBELL(OnContextRestored)(JNIEnv * env, jobject obj)
{
	if (!context_lost)
	{
		// Work around bugs in the java code. When turning the screen off
		// and on when active, result in a OnContextRestore but missing the
		// OnContextLost.
		//TBDebugOut("InvokeContextLost (bug workaround)");
		g_renderer->InvokeContextLost(); // Forget all bitmaps
	}
	//TBDebugOut("OnContextRestored");
	g_renderer->InvokeContextRestored(); // Reload all bitmaps
	context_lost = false;
}

JNI_VOID_TINKERBELL(OnSurfaceResized)(JNIEnv * env, jobject obj, jint width, jint height)
{
	//TBDebugOut("OnSurfaceResized");
	Resize(width, height);
}

JNI_VOID_TINKERBELL(RunSlice)(JNIEnv * env, jobject obj)
{
	//TBDebugOut("RunSlice");
	// Update the app
	Update();

	// Render a frame from the app
	Render();
}

JNI_VOID_TINKERBELL(createAssetManager)(JNIEnv* env, jobject obj, jobject assetManager)
{
	//TBDebugOut("createAssetManager");
	AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
	assert(mgr);

	// Store the assest manager for future use.
	SetAssetManager(mgr);
}

extern TBWidget *root;

JNI_VOID_TINKERBELL(OnPointer)(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint down)
{
	//TBDebugOut("OnPointer");
	int counter = 0;
	if (down)
		root->InvokePointerDown(x, y, counter, TB_MODIFIER_NONE);
	else
		root->InvokePointerUp(x, y, TB_MODIFIER_NONE);
}

JNI_VOID_TINKERBELL(OnPointer2)(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint down)
{
	//TBDebugOut("OnPointer2");
}

JNI_VOID_TINKERBELL(OnPointerMove)(JNIEnv * env, jobject obj, jfloat x, jfloat y, jfloat x2, jfloat y2)
{
	//TBDebugOut("OnPointerMove");
	root->InvokePointerMove(x, y, TB_MODIFIER_NONE);
}
