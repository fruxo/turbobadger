#include "glfw_extra.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "tb_skin.h"
#include "tb_system.h"
#include "tb_widgets.h"
#include "tb_msg.h"
#include "renderers/tb_renderer_gl.h"
#include "tb_font_renderer.h"
#include "platform/Application.h"

using namespace tinkerbell;

class ApplicationBackendGLFW;
ApplicationBackendGLFW *g_backend = nullptr;

int mouse_x = 0;
int mouse_y = 0;
bool key_alt = false;
bool key_ctrl = false;
bool key_shift = false;
bool has_pending_update = false;

class RootWidget : public TBWidget
{
public:
	virtual void OnInvalid();
};

class ApplicationBackendGLFW : public ApplicationBackend
{
public:
	bool Init(Application *app, int width, int height, const char *title);
	ApplicationBackendGLFW() : m_application(nullptr), m_renderer(nullptr), mainWindow(0) {}
	~ApplicationBackendGLFW();

	virtual void Run();
	virtual TBWidget *GetRoot() { return &m_root; }
	virtual TBRenderer *GetRenderer() { return m_renderer; }

	int GetWidth() const { return m_root.GetRect().w; }
	int GetHeight() const { return m_root.GetRect().h; }

	Application *m_application;
	TBRendererGL *m_renderer;
	RootWidget m_root;

	GLFWwindow mainWindow;
};

void RootWidget::OnInvalid()
{
	if (!has_pending_update)
	{
		has_pending_update = true;
		glfwWakeUpMsgLoop(g_backend->mainWindow);
	}
}

MODIFIER_KEYS GetModifierKeys()
{
	MODIFIER_KEYS code = TB_MODIFIER_NONE;
	if (key_alt)	code |= TB_ALT;
	if (key_ctrl)	code |= TB_CTRL;
	if (key_shift)	code |= TB_SHIFT;
	return code;
}

static void char_callback(GLFWwindow window, int character)
{
	g_backend->GetRoot()->InvokeKey(character, TB_KEY_UNDEFINED, GetModifierKeys(), true);
	g_backend->GetRoot()->InvokeKey(character, TB_KEY_UNDEFINED, GetModifierKeys(), false);
}

static void key_callback(GLFWwindow window, int key, int action)
{
	MODIFIER_KEYS modifier = GetModifierKeys();
	bool down = (action == GLFW_PRESS);
	switch (key)
	{
	case GLFW_KEY_F1:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_F1, modifier, down); break;
	case GLFW_KEY_F2:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_F2, modifier, down); break;
	case GLFW_KEY_F3:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_F3, modifier, down); break;
	case GLFW_KEY_F4:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_F4, modifier, down); break;
	case GLFW_KEY_F5:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_F5, modifier, down); break;
	case GLFW_KEY_F6:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_F6, modifier, down); break;
	case GLFW_KEY_F7:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_F7, modifier, down); break;
	case GLFW_KEY_F8:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_F8, modifier, down); break;
	case GLFW_KEY_F9:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_F9, modifier, down); break;
	case GLFW_KEY_F10:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_F10, modifier, down); break;
	case GLFW_KEY_F11:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_F11, modifier, down); break;
	case GLFW_KEY_F12:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_F12, modifier, down); break;
	case GLFW_KEY_LEFT:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_LEFT, modifier, down); break;
	case GLFW_KEY_UP:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_UP, modifier, down); break;
	case GLFW_KEY_RIGHT:		g_backend->GetRoot()->InvokeKey(0, TB_KEY_RIGHT, modifier, down); break;
	case GLFW_KEY_DOWN:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_DOWN, modifier, down); break;
	case GLFW_KEY_PAGEUP:		g_backend->GetRoot()->InvokeKey(0, TB_KEY_PAGE_UP, modifier, down); break;
	case GLFW_KEY_PAGEDOWN:		g_backend->GetRoot()->InvokeKey(0, TB_KEY_PAGE_DOWN, modifier, down); break;
	case GLFW_KEY_HOME:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_HOME, modifier, down); break;
	case GLFW_KEY_END:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_END, modifier, down); break;
	case GLFW_KEY_INSERT:		g_backend->GetRoot()->InvokeKey(0, TB_KEY_INSERT, modifier, down); break;
	case GLFW_KEY_TAB:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_TAB, modifier, down); break;
	case GLFW_KEY_DEL:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_DELETE, modifier, down); break;
	case GLFW_KEY_BACKSPACE:	g_backend->GetRoot()->InvokeKey(0, TB_KEY_BACKSPACE, modifier, down); break;
	case GLFW_KEY_ENTER:		g_backend->GetRoot()->InvokeKey(0, TB_KEY_ENTER, modifier, down); break;
	case GLFW_KEY_ESC:			g_backend->GetRoot()->InvokeKey(0, TB_KEY_ESC, modifier, down); break;
	case GLFW_KEY_MENU:
		if (TBWidget::focused_widget && !down)
		{
			TBWidgetEvent ev(EVENT_TYPE_CONTEXT_MENU, 0, 0, GetModifierKeys());
			TBWidget::focused_widget->InvokeEvent(ev);
		}
		break;
	case GLFW_KEY_LSHIFT:
	case GLFW_KEY_RSHIFT:
		key_shift = down;
		break;
	case GLFW_KEY_LCTRL:
	case GLFW_KEY_RCTRL:
		key_ctrl = down;
		break;
	case GLFW_KEY_LALT:
	case GLFW_KEY_RALT:
		key_alt = down;
		break;
	default:
#ifdef WIN32 // FIX: Move this fix into glfw!
		// At least the windows implementation of glfw calls KeyboardSpecial
		// when pressing a character while ctrl is also pressed.
		if (key_ctrl && !key_alt && key >= 32 && key <= 255)
			g_backend->GetRoot()->InvokeKey(key, TB_KEY_UNDEFINED, modifier, down);
#endif // WIN32
		break;
	}
}

static void mouse_button_callback(GLFWwindow window, int button, int action)
{
	int x = mouse_x;
	int y = mouse_y;
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			// This is a quick fix with n-click support :)
			static double last_time = 0;
			static int last_x = 0;
			static int last_y = 0;
			static int counter = 1;

			double time = TBSystem::GetTimeMS();
			if (time < last_time + 600 && last_x == x && last_y == y)
				counter++;
			else
				counter = 1;
			last_x = x;
			last_y = y;
			last_time = time;

			g_backend->GetRoot()->InvokePointerDown(x, y, counter, GetModifierKeys());
		}
		else
			g_backend->GetRoot()->InvokePointerUp(x, y, GetModifierKeys());
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		g_backend->GetRoot()->InvokePointerMove(x, y, GetModifierKeys());
		if (TBWidget::hovered_widget)
		{
			TBWidget::hovered_widget->ConvertFromRoot(x, y);
			TBWidgetEvent ev(EVENT_TYPE_CONTEXT_MENU, x, y, GetModifierKeys());
			TBWidget::hovered_widget->InvokeEvent(ev);
		}
	}
}

void cursor_position_callback(GLFWwindow window, int x, int y)
{
	mouse_x = x;
	mouse_y = y;
	if (g_backend->GetRoot())
		g_backend->GetRoot()->InvokePointerMove(x, y, GetModifierKeys());
}

static void scroll_callback(GLFWwindow window, double x, double y)
{
	if (g_backend->GetRoot())
		g_backend->GetRoot()->InvokeWheel(mouse_x, mouse_y, (int)x, -(int)y, GetModifierKeys());
}

/** Reschedule the platform timer, or cancel it if fire_time is TB_NOT_SOON.
	If force is true, it will ask the platform to schedule it again, even if
	the fire_time is the same as last time. */
static void ReschedulePlatformTimer(double fire_time, bool force)
{
	static double set_fire_time = -1;
	if (fire_time == TB_NOT_SOON)
	{
		set_fire_time = -1;
		glfwKillTimer();
	}
	else if (fire_time != set_fire_time || force)
	{
		set_fire_time = fire_time;
		double delay = fire_time - tinkerbell::TBSystem::GetTimeMS();
		unsigned int idelay = (unsigned int) MAX(delay, 0);
		glfwRescheduleTimer(idelay);
	}
}

static void timer_callback()
{
	double next_fire_time = TBMessageHandler::GetNextMessageFireTime();
	double now = tinkerbell::TBSystem::GetTimeMS();
	if (now < next_fire_time)
	{
		// We timed out *before* we were supposed to (the OS is not playing nice).
		// Calling ProcessMessages now won't achieve a thing so force a reschedule
		// of the platform timer again with the same time.
		ReschedulePlatformTimer(next_fire_time, true);
		return;
	}

	TBMessageHandler::ProcessMessages();

	// If we still have things to do (because we didn't process all messages,
	// or because there are new messages), we need to rescedule, so call RescheduleTimer.
	TBSystem::RescheduleTimer(TBMessageHandler::GetNextMessageFireTime());
}

// This doesn't really belong here (it belongs in tb_system_[linux/windows].cpp.
// This is here since the proper implementations has not yet been done.
void TBSystem::RescheduleTimer(double fire_time)
{
	ReschedulePlatformTimer(fire_time, false);
}

static void window_refresh_callback(GLFWwindow window)
{
	g_backend->m_application->Process();

	has_pending_update = false;

	g_backend->m_application->RenderFrame(g_backend->GetWidth(), g_backend->GetHeight());

	glfwSwapBuffers(window);
}

static void window_size_callback(GLFWwindow window, int w, int h)
{
	if (g_backend->GetRoot())
		g_backend->GetRoot()->SetRect(TBRect(0, 0, w, h));
}

//static
ApplicationBackend *ApplicationBackend::Create(Application *app, int width, int height, const char *title)
{
	g_backend = new ApplicationBackendGLFW();
	if (g_backend && g_backend->Init(app, width, height, title))
		return g_backend;
	delete g_backend;
	g_backend = nullptr;
	return nullptr;
}

bool ApplicationBackendGLFW::Init(Application *app, int width, int height, const char *title)
{
	if (!glfwInit())
		return false;
	mainWindow = glfwCreateWindow(width, height, GLFW_WINDOWED, "", NULL);
	if (!mainWindow)
	{
		glfwTerminate();
		return false;
	}
    glfwMakeContextCurrent(mainWindow);

	glfwSetWindowTitle(mainWindow, title);

	// Ensure we can capture the escape key being pressed below
	//glfwSetInputMode(mainWindow, GLFW_STICKY_KEYS, GL_TRUE);
	//glfwSetInputMode(mainWindow, GLFW_SYSTEM_KEYS, GL_TRUE);
    glfwSetInputMode(mainWindow, GLFW_KEY_REPEAT, GL_TRUE);

	// Set callback functions
	glfwSetWindowSizeCallback(window_size_callback);
	glfwSetWindowRefreshCallback(window_refresh_callback);
	glfwSetCursorPosCallback(cursor_position_callback);
    glfwSetMouseButtonCallback(mouse_button_callback);
    glfwSetScrollCallback(scroll_callback);
    glfwSetKeyCallback(key_callback);
    glfwSetCharCallback(char_callback);
    glfwSetTimerCallback(timer_callback);

	m_renderer = new TBRendererGL();
	m_root.SetRect(TBRect(0, 0, width, height));

	// Create the application object for our demo
	m_application = app;
	m_application->OnBackendAttached(this);

	return true;
}

ApplicationBackendGLFW::~ApplicationBackendGLFW()
{
	m_application->OnBackendDetached();
	m_application = nullptr;

	shutdown_tinkerbell();

	glfwTerminate();

	delete m_renderer;
}

void ApplicationBackendGLFW::Run()
{
	do
	{
		if (has_pending_update)
			window_refresh_callback(mainWindow);
        glfwWaitMsgLoop(mainWindow);
	} while (!glfwGetWindowParam(mainWindow, GLFW_CLOSE_REQUESTED));
}

#ifdef WIN32

#include <mmsystem.h>
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Set the current path to the directory of the app so we find assets also when visual studio start it.
	char modname[MAX_PATH];
	GetModuleFileName(NULL, modname, MAX_PATH);
	TBTempBuffer buf;
	buf.AppendPath(modname);
	SetCurrentDirectory(buf.GetData());

	// Crank up windows timer resolution (it's awfully low res normally). Note: This affects battery time!
	timeBeginPeriod(1);
	int ret = app_main();
	timeEndPeriod(1);
	return ret;
}

#else // WIN32

int main(int argc, char** argv)
{
	return app_main();
}

#endif // !WIN32
