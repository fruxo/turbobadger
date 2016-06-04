// -*-  Mode: C++; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*-

#ifdef __EMSCRIPTEN__
#include "SDL/SDL.h"
#else
#include "SDL2/SDL.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <algorithm>
#include <iostream>
#include "tb_skin.h"
#include "tb_system.h"
#include "tb_msg.h"
#include "renderers/tb_renderer_gl.h"
#include "tb_font_renderer.h"
#include "Application.h"

#ifdef TB_TARGET_MACOSX
#include <unistd.h>
#include <mach-o/dyld.h>
#endif

using namespace tb;

class AppBackendSDL2 : public AppBackend
{
public:
	bool Init(App *app);
	AppBackendSDL2()	: m_app(nullptr)
						, m_renderer(nullptr)
						, mainWindow(0)
						, m_has_pending_update(false)
						, m_quit_requested(false) {}
	~AppBackendSDL2();

	virtual void OnAppEvent(const EVENT &ev);

	TBWidget *GetRoot() const { return m_app->GetRoot(); }
	int GetWidth() const { return m_app->GetWidth(); }
	int GetHeight() const { return m_app->GetHeight(); }


	bool InvokeKey(unsigned int key, SPECIAL_KEY special_key,
				   MODIFIER_KEYS modifierkeys, bool down);
	bool HandleSDLEvent(SDL_Event & event);

	App *m_app;
	TBRendererGL *m_renderer;
	SDL_Window *mainWindow;
	SDL_GLContext glContext;
	bool m_has_pending_update;
	bool m_quit_requested;
};

MODIFIER_KEYS GetModifierKeys()
{
	MODIFIER_KEYS code = TB_MODIFIER_NONE;
	SDL_Keymod mods = SDL_GetModState();
	if (mods & KMOD_ALT)	code |= TB_ALT;
	if (mods & KMOD_CTRL)	code |= TB_CTRL;
	if (mods & KMOD_SHIFT)	code |= TB_SHIFT;
	if (mods & KMOD_GUI)	code |= TB_SUPER; // no idea what SUPER means, but doesn't seem to be used
	return code;
}

MODIFIER_KEYS GetModifierKeys(SDL_Keymod mods)
{
	MODIFIER_KEYS code = TB_MODIFIER_NONE;
	if (mods & KMOD_ALT)	code |= TB_ALT;
	if (mods & KMOD_CTRL)	code |= TB_CTRL;
	if (mods & KMOD_SHIFT)	code |= TB_SHIFT;
	if (mods & KMOD_GUI)	code |= TB_SUPER; // no idea what SUPER means, but doesn't seem to be used
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

bool
AppBackendSDL2::InvokeKey(unsigned int key, SPECIAL_KEY special_key, MODIFIER_KEYS modifierkeys, bool down)
{
	if (InvokeShortcut(key, special_key, modifierkeys, down))
		return true;
	return m_app->GetRoot()->InvokeKey(key, special_key, modifierkeys, down);
}

static SDL_TimerID my_timer_id = 0;
static Uint32 timer_callback(Uint32 interval, void *param)
{
	double next_fire_time = TBMessageHandler::GetNextMessageFireTime();
	double now = tb::TBSystem::GetTimeMS();
	if (now < next_fire_time)
	{
		// We timed out *before* we were supposed to (the OS is not playing nice).
		// Calling ProcessMessages now won't achieve a thing so force a reschedule
		// of the platform timer again with the same time.
		return next_fire_time - now;
	}

	TBMessageHandler::ProcessMessages();

	// If we still have things to do (because we didn't process all messages,
	// or because there are new messages), we need to rescedule, so call RescheduleTimer.
	next_fire_time = TBMessageHandler::GetNextMessageFireTime();
	if (next_fire_time == TB_NOT_SOON)
	{
		my_timer_id = 0;
		return 0; // never - no longer scheduled
	}
	next_fire_time -= tb::TBSystem::GetTimeMS();
	return std::max((Uint32)next_fire_time, (Uint32)1); // asap
}

// This doesn't really belong here (it belongs in tb_system_[linux/windows].cpp.
// This is here since the proper implementations has not yet been done.

/** Reschedule the platform timer, or cancel it if fire_time is TB_NOT_SOON.
	If fire_time is 0, it should be fired ASAP.
	If force is true, it will ask the platform to schedule it again, even if
	the fire_time is the same as last time. */
void TBSystem::RescheduleTimer(double fire_time)
{
	// cancel existing timer
	if (my_timer_id)
	{
		SDL_RemoveTimer(my_timer_id);
		my_timer_id = 0;
	}
	// set new timer
	if (fire_time != TB_NOT_SOON)
	{
		double delay = fire_time - tb::TBSystem::GetTimeMS();
		my_timer_id = SDL_AddTimer(std::max((Uint32)delay, (Uint32)1), timer_callback, NULL);
		if (!my_timer_id)
			std::cout << "ERROR: RescheduleTimer failed to SDL_AddTimer\n";
	}
}

bool AppBackendSDL2::Init(App *app)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
	{
		printf("Unable to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	const int width = app->GetWidth() > 0 ? app->GetWidth() : 1920;
	const int height = app->GetHeight() > 0 ? app->GetHeight() : 1080;
	mainWindow = SDL_CreateWindow(app->GetTitle(),
								  SDL_WINDOWPOS_UNDEFINED,
								  SDL_WINDOWPOS_UNDEFINED,
								  width, height,
								  SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
	if (!mainWindow)
	{
		printf("Unable to create window: %s\n", SDL_GetError());
		return 1;
	}

	//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
#if defined(TB_RENDERER_GLES_2)
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(TB_RENDERER_GL3)
#if 0
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
#endif

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	glContext = SDL_GL_CreateContext(mainWindow);
	SDL_GL_MakeCurrent(mainWindow, glContext);

	glClearColor(0.3f, 0.3f, 0.3f, 1);

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

AppBackendSDL2::~AppBackendSDL2()
{
	m_app->OnBackendDetached();
	m_app = nullptr;

	tb_core_shutdown();

	SDL_Quit();

	delete m_renderer;
}

void AppBackendSDL2::OnAppEvent(const EVENT &ev)
{
	switch (ev)
	{
	case EVENT_PAINT_REQUEST:
		if (!m_has_pending_update)
		{
			m_has_pending_update = true;
			// queue a user event to cause the event loop to run
			SDL_Event event;
			SDL_UserEvent userevent;
			userevent.type = SDL_USEREVENT;
			userevent.code = 0;
			userevent.data1 = NULL;
			userevent.data2 = NULL;
			event.type = SDL_USEREVENT;
			event.user = userevent;
			SDL_PushEvent(&event);
		}
		break;
	case EVENT_QUIT_REQUEST:
		m_quit_requested = true;
		// queue a user event to cause the event loop to run
		SDL_Event event;
		SDL_UserEvent userevent;
		userevent.type = SDL_USEREVENT;
		userevent.code = 0;
		userevent.data1 = NULL;
		userevent.data2 = NULL;
		event.type = SDL_USEREVENT;
		event.user = userevent;
		SDL_PushEvent(&event);
		break;
	case EVENT_TITLE_CHANGED:
		SDL_SetWindowTitle(mainWindow, m_app->GetTitle());
		break;
	default:
		assert(!"Unhandled app event!");
	}
}

// Attempt to convert an sdl event to a TB event, return true if handled
bool
AppBackendSDL2::HandleSDLEvent(SDL_Event & event)
{
	bool handled = true;
	switch (event.type) {
	case SDL_KEYUP:
	case SDL_KEYDOWN:
		{
		// SDL_KeyboardEvent
		// Handle any key presses here.
		bool down = event.type == SDL_KEYDOWN;
		MODIFIER_KEYS modifier = GetModifierKeys((SDL_Keymod)event.key.keysym.mod);
		if (event.key.keysym.sym >= SDLK_SPACE && event.key.keysym.sym <= SDLK_z)
		{
			unsigned int character = event.key.keysym.sym;
			InvokeKey(character, TB_KEY_UNDEFINED, modifier, down);
		}
		else
		{
			// handle special keys
			switch (event.key.keysym.sym)
			{
			case SDLK_F1:			InvokeKey(0, TB_KEY_F1, modifier, down); break;
			case SDLK_F2:			InvokeKey(0, TB_KEY_F2, modifier, down); break;
			case SDLK_F3:			InvokeKey(0, TB_KEY_F3, modifier, down); break;
			case SDLK_F4:			InvokeKey(0, TB_KEY_F4, modifier, down); break;
			case SDLK_F5:			InvokeKey(0, TB_KEY_F5, modifier, down); break;
			case SDLK_F6:			InvokeKey(0, TB_KEY_F6, modifier, down); break;
			case SDLK_F7:			InvokeKey(0, TB_KEY_F7, modifier, down); break;
			case SDLK_F8:			InvokeKey(0, TB_KEY_F8, modifier, down); break;
			case SDLK_F9:			InvokeKey(0, TB_KEY_F9, modifier, down); break;
			case SDLK_F10:			InvokeKey(0, TB_KEY_F10, modifier, down); break;
			case SDLK_F11:			InvokeKey(0, TB_KEY_F11, modifier, down); break;
			case SDLK_F12:			InvokeKey(0, TB_KEY_F12, modifier, down); break;
			case SDLK_LEFT:			InvokeKey(0, TB_KEY_LEFT, modifier, down); break;
			case SDLK_UP:			InvokeKey(0, TB_KEY_UP, modifier, down); break;
			case SDLK_RIGHT:		InvokeKey(0, TB_KEY_RIGHT, modifier, down); break;
			case SDLK_DOWN:			InvokeKey(0, TB_KEY_DOWN, modifier, down); break;
			case SDLK_PAGEUP:		InvokeKey(0, TB_KEY_PAGE_UP, modifier, down); break;
			case SDLK_PAGEDOWN:		InvokeKey(0, TB_KEY_PAGE_DOWN, modifier, down); break;
			case SDLK_HOME:			InvokeKey(0, TB_KEY_HOME, modifier, down); break;
			case SDLK_END:			InvokeKey(0, TB_KEY_END, modifier, down); break;
			case SDLK_INSERT:		InvokeKey(0, TB_KEY_INSERT, modifier, down); break;
			case SDLK_TAB:			InvokeKey(0, TB_KEY_TAB, modifier, down); break;
			case SDLK_DELETE:		InvokeKey(0, TB_KEY_DELETE, modifier, down); break;
			case SDLK_BACKSPACE:	InvokeKey(0, TB_KEY_BACKSPACE, modifier, down); break;
			case SDLK_RETURN:		
			case SDLK_KP_ENTER:		InvokeKey(0, TB_KEY_ENTER, modifier, down); break;
			case SDLK_ESCAPE:		InvokeKey(0, TB_KEY_ESC, modifier, down); break;
			case SDLK_MENU:
				if (TBWidget::focused_widget && !down)
				{
					TBWidgetEvent ev(EVENT_TYPE_CONTEXT_MENU);
					ev.modifierkeys = modifier;
					TBWidget::focused_widget->InvokeEvent(ev);
				}
				break;
			default:
				handled = false;
				break;
			}
		}

		break;
	}
	case SDL_FINGERMOTION:
	case SDL_FINGERDOWN:
	case SDL_FINGERUP:
		//event.tfinger;
		break;

	case SDL_MOUSEMOTION: {
		if (m_app->GetRoot() && !(ShouldEmulateTouchEvent() && !TBWidget::captured_widget))
			m_app->GetRoot()->InvokePointerMove(event.motion.x, event.motion.y, 
												GetModifierKeys(),
												ShouldEmulateTouchEvent());

		break;
	}
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEBUTTONDOWN: {
		// Handle mouse clicks here.
		MODIFIER_KEYS modifier = GetModifierKeys();
		int x = event.button.x;
		int y = event.button.y;
		if (event.button.button == SDL_BUTTON_LEFT)
		{
			if (event.type == SDL_MOUSEBUTTONDOWN)
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

				m_app->GetRoot()->InvokePointerDown(x, y, counter, modifier, ShouldEmulateTouchEvent());
			}
			else
				m_app->GetRoot()->InvokePointerUp(x, y, modifier, ShouldEmulateTouchEvent());
		}
		else if (event.button.button == SDL_BUTTON_RIGHT && event.type == SDL_MOUSEBUTTONUP)
		{
			m_app->GetRoot()->InvokePointerMove(x, y, modifier, ShouldEmulateTouchEvent());
			if (TBWidget::hovered_widget)
			{
				TBWidget::hovered_widget->ConvertFromRoot(x, y);
				TBWidgetEvent ev(EVENT_TYPE_CONTEXT_MENU, x, y, false, modifier);
				TBWidget::hovered_widget->InvokeEvent(ev);
			}
		}
	}
		break;
	case SDL_MOUSEWHEEL: {
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		if (m_app->GetRoot())
			m_app->GetRoot()->InvokeWheel(mouse_x, mouse_y,
										  (int)event.wheel.x, -(int)event.wheel.y,
										  GetModifierKeys());
		break;
	}
	case SDL_MULTIGESTURE:
		//event.mgesture;
		break;
	case SDL_SYSWMEVENT:
		//event.syswm;
		break;
	case SDL_TEXTEDITING:
		//event.edit;
		break;
	case SDL_TEXTINPUT:
		//event.text;
		break;
	case SDL_WINDOWEVENT: {
#undef SDL_Log
#define SDL_Log(...) //do { ;} while(0)
        switch (event.window.event) {
        case SDL_WINDOWEVENT_SHOWN:
            SDL_Log("Window %d shown", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_HIDDEN:
            SDL_Log("Window %d hidden", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_EXPOSED:
            SDL_Log("Window %d exposed", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_MOVED:
            SDL_Log("Window %d moved to %d,%d",
                    event.window.windowID, event.window.data1,
                    event.window.data2);
            break;
        case SDL_WINDOWEVENT_RESIZED:
			if (m_app)
				m_app->OnResized(event.window.data1, event.window.data2);
            SDL_Log("Window %d resized to %dx%d",
                    event.window.windowID, event.window.data1,
                    event.window.data2);
            break;
        case SDL_WINDOWEVENT_SIZE_CHANGED:
            SDL_Log("Window %d size changed to %dx%d",
                    event.window.windowID, event.window.data1,
                    event.window.data2);
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            SDL_Log("Window %d minimized", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_MAXIMIZED:
            SDL_Log("Window %d maximized", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_RESTORED:
            SDL_Log("Window %d restored", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_ENTER:
            SDL_Log("Mouse entered window %d",
                    event.window.windowID);
            break;
        case SDL_WINDOWEVENT_LEAVE:
            SDL_Log("Mouse left window %d", event.window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            SDL_Log("Window %d gained keyboard focus",
                    event.window.windowID);
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            SDL_Log("Window %d lost keyboard focus",
                    event.window.windowID);
            break;
        case SDL_WINDOWEVENT_CLOSE:
            SDL_Log("Window %d closed", event.window.windowID);
            break;
        default:
			handled = false;
            SDL_Log("Window %d got unknown event %d",
                    event.window.windowID, event.window.event);
            break;
		}
		break;
	}
	case SDL_USEREVENT:
		// event.user;
		break;
	case SDL_QUIT:
		m_quit_requested = true;
		return true;
	default:
		handled = false;
	}
	return handled;
}


bool port_main()
{
	App *app = app_create();

	AppBackendSDL2 *backend = new AppBackendSDL2();
	if (!backend || !backend->Init(app))
		return false;

	bool success = app->Init();
	if (success)
	{
		// Main loop
		SDL_Event event;
		do
		{
			// draw
			if (backend->m_has_pending_update)
			{
				backend->m_app->Process();
				backend->m_has_pending_update = false;
				// Bail out if we get here with invalid dimensions.
				// This may happen when minimizing windows (GLFW 3.0.4, Windows 8.1).
				if (backend->GetWidth() == 0 || backend->GetHeight() == 0)
					; // ignore
				else
				{
					backend->m_app->RenderFrame();
					SDL_GL_SwapWindow(backend->mainWindow);
				}
			}
			// handle events
			SDL_WaitEvent(&event);
			if (!backend->HandleSDLEvent(event))
				std::cout << "unhandled SDL event: " << event.type << "\n";

		} while (!backend->m_quit_requested /* && !glfwWindowShouldClose(backend->mainWindow) */);

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
