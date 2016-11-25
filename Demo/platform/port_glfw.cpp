#include "glfw_extra.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "tb_skin.h"
#include "tb_system.h"
#include "tb_msg.h"
#include "tb_editfield.h"
#include "renderers/tb_renderer_gl.h"
#include "tb_font_renderer.h"
#include "Application.h"

#ifdef TB_TARGET_MACOSX
#include <unistd.h>
#include <mach-o/dyld.h>
#endif

using namespace tb;

int mouse_x = 0;
int mouse_y = 0;
bool key_alt = false;
bool key_ctrl = false;
bool key_shift = false;
bool key_super = false;

class AppBackendGLFW;

void SetBackend(GLFWwindow *window, AppBackendGLFW *backend)
{
	glfwSetWindowUserPointer(window, backend);
}

AppBackendGLFW *GetBackend(GLFWwindow *window)
{
	return static_cast<AppBackendGLFW*>(glfwGetWindowUserPointer(window));
}

class AppBackendGLFW : public AppBackend
{
public:
	bool Init(App *app);
	AppBackendGLFW()	: m_app(nullptr)
						, m_renderer(nullptr)
						, mainWindow(nullptr)
						, m_cursor_i_beam(nullptr)
						, m_has_pending_update(false)
						, m_quit_requested(false) {}
	~AppBackendGLFW();

	virtual void OnAppEvent(const EVENT &ev);

	TBWidget *GetRoot() const { return m_app->GetRoot(); }
	int GetWidth() const { return m_app->GetWidth(); }
	int GetHeight() const { return m_app->GetHeight(); }

	App *m_app;
	TBRendererGL *m_renderer;
	GLFWwindow *mainWindow;
	GLFWcursor *m_cursor_i_beam;
	bool m_has_pending_update;
	bool m_quit_requested;
};

MODIFIER_KEYS GetModifierKeys()
{
	MODIFIER_KEYS code = TB_MODIFIER_NONE;
	if (key_alt)	code |= TB_ALT;
	if (key_ctrl)	code |= TB_CTRL;
	if (key_shift)	code |= TB_SHIFT;
	if (key_super)	code |= TB_SUPER;
	return code;
}

MODIFIER_KEYS GetModifierKeys(int glfwmod)
{
	MODIFIER_KEYS code = TB_MODIFIER_NONE;
	if (glfwmod & GLFW_MOD_ALT)			code |= TB_ALT;
	if (glfwmod & GLFW_MOD_CONTROL)		code |= TB_CTRL;
	if (glfwmod & GLFW_MOD_SHIFT)		code |= TB_SHIFT;
	if (glfwmod & GLFW_MOD_SUPER)		code |= TB_SUPER;
	return code;
}

static bool ShouldEmulateTouchEvent()
{
	// Used to emulate that mouse events are touch events when alt, ctrl and shift are pressed.
	// This makes testing a lot easier when there is no touch screen around :)
	return (GetModifierKeys() & (TB_ALT | TB_CTRL | TB_SHIFT)) ? true : false;
}

// @return Return the upper case of a ascii charcter. Only for shortcut handling.
static int toupr_ascii(int ascii)
{
	if (ascii >= 'a' && ascii <= 'z')
		return ascii + 'A' - 'a';
	return ascii;
}

static bool InvokeShortcut(int key, SPECIAL_KEY special_key, MODIFIER_KEYS modifierkeys, bool down)
{
#ifdef TB_TARGET_MACOSX
	bool shortcut_key = (modifierkeys & TB_SUPER) ? true : false;
#else
	bool shortcut_key = (modifierkeys & TB_CTRL) ? true : false;
#endif
	if (!TBWidget::focused_widget || !down || !shortcut_key)
		return false;
	bool reverse_key = (modifierkeys & TB_SHIFT) ? true : false;
	int upper_key = toupr_ascii(key);
	TBID id;
	if (upper_key == 'X')
		id = TBIDC("cut");
	else if (upper_key == 'C' || special_key == TB_KEY_INSERT)
		id = TBIDC("copy");
	else if (upper_key == 'V' || (special_key == TB_KEY_INSERT && reverse_key))
		id = TBIDC("paste");
	else if (upper_key == 'A')
		id = TBIDC("selectall");
	else if (upper_key == 'Z' || upper_key == 'Y')
	{
		bool undo = upper_key == 'Z';
		if (reverse_key)
			undo = !undo;
		id = undo ? TBIDC("undo") : TBIDC("redo");
	}
	else if (upper_key == 'N')
		id = TBIDC("new");
	else if (upper_key == 'O')
		id = TBIDC("open");
	else if (upper_key == 'S')
		id = TBIDC("save");
	else if (upper_key == 'W')
		id = TBIDC("close");
	else if (special_key == TB_KEY_PAGE_UP)
		id = TBIDC("prev_doc");
	else if (special_key == TB_KEY_PAGE_DOWN)
		id = TBIDC("next_doc");
	else
		return false;

	TBWidgetEvent ev(EVENT_TYPE_SHORTCUT);
	ev.modifierkeys = modifierkeys;
	ev.ref_id = id;
	return TBWidget::focused_widget->InvokeEvent(ev);
}

static bool InvokeKey(GLFWwindow *window, unsigned int key, SPECIAL_KEY special_key, MODIFIER_KEYS modifierkeys, bool down)
{
	if (InvokeShortcut(key, special_key, modifierkeys, down))
		return true;
	GetBackend(window)->GetRoot()->InvokeKey(key, special_key, modifierkeys, down);
	return true;
}

static void char_callback(GLFWwindow *window, unsigned int character)
{
	// glfw on osx seems to send us characters from the private
	// use block when using f.ex arrow keys on osx.
	if (character >= 0xE000 && character <= 0xF8FF)
		return;

	InvokeKey(window, character, TB_KEY_UNDEFINED, GetModifierKeys(), true);
	InvokeKey(window, character, TB_KEY_UNDEFINED, GetModifierKeys(), false);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int glfwmod)
{
	MODIFIER_KEYS modifier = GetModifierKeys(glfwmod);
	bool down = (action == GLFW_PRESS || action == GLFW_REPEAT);
	switch (key)
	{
	case GLFW_KEY_F1:			InvokeKey(window, 0, TB_KEY_F1, modifier, down); break;
	case GLFW_KEY_F2:			InvokeKey(window, 0, TB_KEY_F2, modifier, down); break;
	case GLFW_KEY_F3:			InvokeKey(window, 0, TB_KEY_F3, modifier, down); break;
	case GLFW_KEY_F4:			InvokeKey(window, 0, TB_KEY_F4, modifier, down); break;
	case GLFW_KEY_F5:			InvokeKey(window, 0, TB_KEY_F5, modifier, down); break;
	case GLFW_KEY_F6:			InvokeKey(window, 0, TB_KEY_F6, modifier, down); break;
	case GLFW_KEY_F7:			InvokeKey(window, 0, TB_KEY_F7, modifier, down); break;
	case GLFW_KEY_F8:			InvokeKey(window, 0, TB_KEY_F8, modifier, down); break;
	case GLFW_KEY_F9:			InvokeKey(window, 0, TB_KEY_F9, modifier, down); break;
	case GLFW_KEY_F10:			InvokeKey(window, 0, TB_KEY_F10, modifier, down); break;
	case GLFW_KEY_F11:			InvokeKey(window, 0, TB_KEY_F11, modifier, down); break;
	case GLFW_KEY_F12:			InvokeKey(window, 0, TB_KEY_F12, modifier, down); break;
	case GLFW_KEY_LEFT:			InvokeKey(window, 0, TB_KEY_LEFT, modifier, down); break;
	case GLFW_KEY_UP:			InvokeKey(window, 0, TB_KEY_UP, modifier, down); break;
	case GLFW_KEY_RIGHT:		InvokeKey(window, 0, TB_KEY_RIGHT, modifier, down); break;
	case GLFW_KEY_DOWN:			InvokeKey(window, 0, TB_KEY_DOWN, modifier, down); break;
	case GLFW_KEY_PAGE_UP:		InvokeKey(window, 0, TB_KEY_PAGE_UP, modifier, down); break;
	case GLFW_KEY_PAGE_DOWN:	InvokeKey(window, 0, TB_KEY_PAGE_DOWN, modifier, down); break;
	case GLFW_KEY_HOME:			InvokeKey(window, 0, TB_KEY_HOME, modifier, down); break;
	case GLFW_KEY_END:			InvokeKey(window, 0, TB_KEY_END, modifier, down); break;
	case GLFW_KEY_INSERT:		InvokeKey(window, 0, TB_KEY_INSERT, modifier, down); break;
	case GLFW_KEY_TAB:			InvokeKey(window, 0, TB_KEY_TAB, modifier, down); break;
	case GLFW_KEY_DELETE:		InvokeKey(window, 0, TB_KEY_DELETE, modifier, down); break;
	case GLFW_KEY_BACKSPACE:	InvokeKey(window, 0, TB_KEY_BACKSPACE, modifier, down); break;
	case GLFW_KEY_ENTER:		
	case GLFW_KEY_KP_ENTER:		InvokeKey(window, 0, TB_KEY_ENTER, modifier, down); break;
	case GLFW_KEY_ESCAPE:		InvokeKey(window, 0, TB_KEY_ESC, modifier, down); break;
	case GLFW_KEY_MENU:
		if (TBWidget::focused_widget && !down)
		{
			TBWidgetEvent ev(EVENT_TYPE_CONTEXT_MENU);
			ev.modifierkeys = modifier;
			TBWidget::focused_widget->InvokeEvent(ev);
		}
		break;
	case GLFW_KEY_LEFT_SHIFT:
	case GLFW_KEY_RIGHT_SHIFT:
		key_shift = down;
		break;
	case GLFW_KEY_LEFT_CONTROL:
	case GLFW_KEY_RIGHT_CONTROL:
		key_ctrl = down;
		break;
	case GLFW_KEY_LEFT_ALT:
	case GLFW_KEY_RIGHT_ALT:
		key_alt = down;
		break;
	case GLFW_KEY_LEFT_SUPER:
	case GLFW_KEY_RIGHT_SUPER:
		key_super = down;
		break;
	default:
		// glfw calls key_callback instead of char_callback
		// when pressing a character while ctrl is also pressed.
		if ((key_ctrl || key_super) && !key_alt && key >= 32 && key <= 255)
			InvokeKey(window, key, TB_KEY_UNDEFINED, modifier, down);
		break;
	}
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int glfwmod)
{
	MODIFIER_KEYS modifier = GetModifierKeys(glfwmod);
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

			GetBackend(window)->GetRoot()->InvokePointerDown(x, y, counter, modifier, ShouldEmulateTouchEvent());
		}
		else
			GetBackend(window)->GetRoot()->InvokePointerUp(x, y, modifier, ShouldEmulateTouchEvent());
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		GetBackend(window)->GetRoot()->InvokePointerMove(x, y, modifier, ShouldEmulateTouchEvent());
		if (TBWidget::hovered_widget)
		{
			TBWidget::hovered_widget->ConvertFromRoot(x, y);
			TBWidgetEvent ev(EVENT_TYPE_CONTEXT_MENU, x, y, false, modifier);
			TBWidget::hovered_widget->InvokeEvent(ev);
		}
	}
}

void cursor_position_callback(GLFWwindow *window, double x, double y)
{
	mouse_x = (int)x;
	mouse_y = (int)y;
	if (GetBackend(window)->GetRoot() && !(ShouldEmulateTouchEvent() && !TBWidget::captured_widget)) {
		GetBackend(window)->GetRoot()->InvokePointerMove(mouse_x, mouse_y, GetModifierKeys(), ShouldEmulateTouchEvent());

		// Update cursor.
		TBWidget *active_widget = TBWidget::captured_widget ? TBWidget::captured_widget : TBWidget::hovered_widget;
		if (TBSafeCast<TBEditField>(active_widget)) {
			glfwSetCursor(window, GetBackend(window)->m_cursor_i_beam);
		} else {
			glfwSetCursor(window, nullptr);
		}
	}
}

static void scroll_callback(GLFWwindow *window, double x, double y)
{
	if (GetBackend(window)->GetRoot())
		GetBackend(window)->GetRoot()->InvokeWheel(mouse_x, mouse_y, (int)x, -(int)y, GetModifierKeys());
}

/** Reschedule the platform timer, or cancel it if fire_time is TB_NOT_SOON.
	If fire_time is 0, it should be fired ASAP.
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
	else if (fire_time != set_fire_time || force || fire_time == 0)
	{
		set_fire_time = fire_time;
		double delay = fire_time - tb::TBSystem::GetTimeMS();
		unsigned int idelay = (unsigned int) MAX(delay, 0.0);
		glfwRescheduleTimer(idelay);
	}
}

static void timer_callback()
{
	double next_fire_time = TBMessageHandler::GetNextMessageFireTime();
	double now = tb::TBSystem::GetTimeMS();
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

static void window_refresh_callback(GLFWwindow *window)
{
	AppBackendGLFW *backend = GetBackend(window);

	backend->m_app->Process();

	backend->m_has_pending_update = false;
	// Bail out if we get here with invalid dimensions.
	// This may happen when minimizing windows (GLFW 3.0.4, Windows 8.1).
	if (backend->GetWidth() == 0 || backend->GetHeight() == 0)
		return;

	backend->m_app->RenderFrame();

	glfwSwapBuffers(window);
}

static void window_size_callback(GLFWwindow *window, int w, int h)
{
	AppBackendGLFW *backend = GetBackend(window);
	if (backend->m_app)
		backend->m_app->OnResized(w, h);
}

#if (GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 1)
static void drop_callback(GLFWwindow *window, int count, const char **files_utf8)
{
	AppBackendGLFW *backend = GetBackend(window);
	TBWidget *target = TBWidget::hovered_widget;
	if (!target)
		target = TBWidget::focused_widget;
	if (!target)
		target = backend->GetRoot();
	if (target)
	{
		TBWidgetEventFileDrop ev;
		for (int i = 0; i < count; i++)
			ev.files.Add(new TBStr(files_utf8[i]));
		target->InvokeEvent(ev);
	}
}
#endif

bool AppBackendGLFW::Init(App *app)
{
	if (!glfwInit())
		return false;
	const int width = app->GetWidth() > 0 ? app->GetWidth() : 1920;
	const int height = app->GetHeight() > 0 ? app->GetHeight() : 1080;
	mainWindow = glfwCreateWindow(width, height, app->GetTitle(), nullptr, nullptr);
	if (!mainWindow)
	{
		glfwTerminate();
		return false;
	}
	SetBackend(mainWindow, this);
    glfwMakeContextCurrent(mainWindow);

	// Ensure we can capture the escape key being pressed below
	//glfwSetInputMode(mainWindow, GLFW_STICKY_KEYS, GL_TRUE);
	//glfwSetInputMode(mainWindow, GLFW_SYSTEM_KEYS, GL_TRUE);
    //glfwSetInputMode(mainWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	m_cursor_i_beam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);

	// Set callback functions
	glfwSetWindowSizeCallback(mainWindow, window_size_callback);
	glfwSetWindowRefreshCallback(mainWindow, window_refresh_callback);
	glfwSetCursorPosCallback(mainWindow, cursor_position_callback);
    glfwSetMouseButtonCallback(mainWindow, mouse_button_callback);
    glfwSetScrollCallback(mainWindow, scroll_callback);
    glfwSetKeyCallback(mainWindow, key_callback);
    glfwSetCharCallback(mainWindow, char_callback);
    glfwSetTimerCallback(timer_callback);
#if (GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 1)
	glfwSetDropCallback(mainWindow, drop_callback);
#endif

#ifdef TB_TARGET_MACOSX
	// Change working directory to the executable path. We expect it to be
	// where the demo resources are.
	char exec_path[2048];
	uint32_t exec_path_size = sizeof(exec_path);
	if (_NSGetExecutablePath(exec_path, &exec_path_size) == 0)
	{
		TBTempBuffer path;
		path.AppendPath(exec_path);
		chdir(path.GetData());
	}
#endif

	m_renderer = new TBRendererGL();
	tb_core_init(m_renderer);

	// Create the App object for our demo
	m_app = app;
	m_app->OnBackendAttached(this, width, height);

	return true;
}

AppBackendGLFW::~AppBackendGLFW()
{
	m_app->OnBackendDetached();
	m_app = nullptr;

	tb_core_shutdown();

	glfwDestroyCursor(m_cursor_i_beam);

	glfwTerminate();

	delete m_renderer;
}

void AppBackendGLFW::OnAppEvent(const EVENT &ev)
{
	switch (ev)
	{
		case EVENT_PAINT_REQUEST:
			if (!m_has_pending_update)
			{
				m_has_pending_update = true;
				glfwWakeUpMsgLoop(mainWindow);
			}
			break;
		case EVENT_QUIT_REQUEST:
			m_quit_requested = true;
			glfwWakeUpMsgLoop(mainWindow);
			break;
		case EVENT_TITLE_CHANGED:
			glfwSetWindowTitle(mainWindow, m_app->GetTitle());
			break;
		default:
			assert(!"Unhandled app event!");
	}
}

bool port_main() {
	App *app = app_create();

	AppBackendGLFW *backend = new AppBackendGLFW();
	if (!backend || !backend->Init(app))
		return false;

	bool success = app->Init();
	if (success) {
		// Main loop
		do
		{
			if (backend->m_has_pending_update)
				window_refresh_callback(backend->mainWindow);
			glfwWaitMsgLoop(backend->mainWindow);
		} while (!backend->m_quit_requested && !glfwWindowShouldClose(backend->mainWindow));

		app->ShutDown();
	}

	delete backend;

	delete app;

	return success;
}

#ifdef TB_TARGET_WINDOWS

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
	bool success = port_main();
	timeEndPeriod(1);
	return success ? 0 : 1;
}

#else // TB_TARGET_WINDOWS

int main(int argc, char** argv)
{
	return port_main() ? 0 : 1;
}

#endif // !TB_TARGET_WINDOWS
