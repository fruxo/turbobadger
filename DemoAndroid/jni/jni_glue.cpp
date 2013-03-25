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
#include "tb_widgets_listener.h"
#include "tb_system.h"
#include "tb_editfield.h"

using namespace tinkerbell;

// Bug Work around
bool context_lost = false;

// Forward declare set asset manager.
void SetAssetManager(AAssetManager* pManager);

// == CALLS FROM C TO JAVA ==============================================================

JNIEnv *jnienv = nullptr;
void set_jnienv(JNIEnv *env) { jnienv = env; }
JNIEnv *get_jnienv() { return jnienv; }

jclass TinkerbellLib_class;
jmethodID TinkerbellLib_mid_show_keyboard;

void ValidateClassAndMethods()
{
	static bool mid_valid = false;
	if (mid_valid)
		return;
	JNIEnv *env = get_jnienv();
	TinkerbellLib_class = env->FindClass("com.demo.tinkerbell/TinkerbellLib");
	TinkerbellLib_mid_show_keyboard = env->GetStaticMethodID(TinkerbellLib_class, "ShowKeyboard", "(I)V");

	mid_valid = true;
}

void CallStaticVoidMethod(jmethodID mid, int param)
{
	// FIX: there should be some exception handling here
	JNIEnv *env = get_jnienv();
	ValidateClassAndMethods();
	env->CallStaticVoidMethod(TinkerbellLib_class, TinkerbellLib_mid_show_keyboard, param);
}

void ShowKeyboard(bool show)
{
	CallStaticVoidMethod(TinkerbellLib_mid_show_keyboard, show ? 1 : 0);
}

// == CALLS FROM JAVA TO C ==============================================================

#define JNI_VOID_TINKERBELL(func) JNIEXPORT void JNICALL Java_com_demo_tinkerbell_TinkerbellLib_##func
#define JNI_INT_TINKERBELL(func) JNIEXPORT jint JNICALL Java_com_demo_tinkerbell_TinkerbellLib_##func

extern "C"
{
	JNI_VOID_TINKERBELL(createAssetManager)(JNIEnv *env, jobject obj, jobject assetManager);

	JNI_VOID_TINKERBELL(InitApp)(JNIEnv *env, jobject obj, jint width, jint height, jstring settings_path, jstring custom_data_path);
	JNI_VOID_TINKERBELL(ShutDownApp)(JNIEnv *env, jobject obj);

	JNI_VOID_TINKERBELL(OnPauseApp)(JNIEnv *env, jobject obj);
	JNI_VOID_TINKERBELL(OnResumeApp)(JNIEnv *env, jobject obj);

	JNI_VOID_TINKERBELL(OnPointer)(JNIEnv *env, jobject obj, jfloat x, jfloat y, jint down);
	JNI_VOID_TINKERBELL(OnPointer2)(JNIEnv *env, jobject obj, jfloat x, jfloat y, jint down);
	JNI_VOID_TINKERBELL(OnPointerMove)(JNIEnv *env, jobject obj, jfloat x, jfloat y, jfloat x2, jfloat y2);

	JNI_INT_TINKERBELL(OnBackKey)(JNIEnv *env, jobject obj) {return 0;}
	JNI_INT_TINKERBELL(OnMenuKey)(JNIEnv *env, jobject obj) {return 0;}

	JNI_VOID_TINKERBELL(OnContextLost)(JNIEnv *env, jobject obj);
	JNI_VOID_TINKERBELL(OnContextRestored)(JNIEnv *env, jobject obj);

	JNI_VOID_TINKERBELL(OnSurfaceResized)(JNIEnv *env, jobject obj, jint width, jint height);

	JNI_VOID_TINKERBELL(RunSlice)(JNIEnv *env, jobject obj);
}

JNI_VOID_TINKERBELL(InitApp)(JNIEnv *env, jobject obj, jint width, jint height, jstring settings_path, jstring custom_data_path)
{
	set_jnienv(env);
	//TBDebugOut("InitApp");

	Init(width, height);
}

JNI_VOID_TINKERBELL(ShutDownApp)(JNIEnv *env, jobject obj)
{
	set_jnienv(env);
	//TBDebugOut("ShutDownApp");

	Shutdown();
	exit(0);
}

JNI_VOID_TINKERBELL(OnPauseApp)(JNIEnv *env, jobject obj)
{
	set_jnienv(env);
	//TBDebugOut("OnPauseApp");
}

JNI_VOID_TINKERBELL(OnResumeApp)(JNIEnv *env, jobject obj)
{
	set_jnienv(env);
	//TBDebugOut("OnResumeApp");
}

JNI_VOID_TINKERBELL(OnContextLost)(JNIEnv *env, jobject obj)
{
	set_jnienv(env);
	//TBDebugOut("OnContextLost");

	g_renderer->InvokeContextLost(); // Forget all bitmaps
	context_lost = true;
}

JNI_VOID_TINKERBELL(OnContextRestored)(JNIEnv *env, jobject obj)
{
	set_jnienv(env);

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

JNI_VOID_TINKERBELL(OnSurfaceResized)(JNIEnv *env, jobject obj, jint width, jint height)
{
	set_jnienv(env);
	//TBDebugOut("OnSurfaceResized");

	Resize(width, height);

	// Make sure any focused editfield remains visible.
	if (TBWidget::focused_widget)
		if (TBWidget::show_focus_state || TBSafeCast<TBEditField>(TBWidget::focused_widget))
			TBWidget::focused_widget->ScrollIntoViewRecursive();
}

JNI_VOID_TINKERBELL(RunSlice)(JNIEnv *env, jobject obj)
{
	set_jnienv(env);
	//TBDebugOut("RunSlice");

	// Update the app
	Update();

	// Render a frame from the app
	Render();
}

JNI_VOID_TINKERBELL(createAssetManager)(JNIEnv* env, jobject obj, jobject assetManager)
{
	set_jnienv(env);
	//TBDebugOut("createAssetManager");

	AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
	assert(mgr);

	// Store the assest manager for future use.
	SetAssetManager(mgr);
}

extern TBWidget *root;

JNI_VOID_TINKERBELL(OnPointer)(JNIEnv *env, jobject obj, jfloat x, jfloat y, jint down)
{
	set_jnienv(env);
	//TBDebugOut("OnPointer");

	int counter = 0;
	if (down)
		root->InvokePointerDown(x, y, counter, TB_MODIFIER_NONE);
	else
		root->InvokePointerUp(x, y, TB_MODIFIER_NONE);
}

JNI_VOID_TINKERBELL(OnPointer2)(JNIEnv *env, jobject obj, jfloat x, jfloat y, jint down)
{
	set_jnienv(env);
	//TBDebugOut("OnPointer2");
}

JNI_VOID_TINKERBELL(OnPointerMove)(JNIEnv *env, jobject obj, jfloat x, jfloat y, jfloat x2, jfloat y2)
{
	set_jnienv(env);
	//TBDebugOut("OnPointerMove");

	root->InvokePointerMove(x, y, TB_MODIFIER_NONE);
}
