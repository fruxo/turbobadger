#include "glfw_extra.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "tb_skin.h"
#include "tb_system.h"
#include "tb_widgets.h"
#include "tb_renderer_gl.h"
#include "tb_font_renderer.h"
#include "Demo.h"

using namespace tinkerbell;

TBWidget *g_root = nullptr;

Application *application = nullptr;
/*static */Application *Application::GetApp() { return application; }

GLFWwindow mainWindow;
int window_w = 1280;
int window_h = 740;
int mouse_x = 0;
int mouse_y = 0;
bool key_alt = false;
bool key_ctrl = false;
bool key_shift = false;
bool has_pending_update = false;

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
	g_root->InvokeKey(character, TB_KEY_UNDEFINED, GetModifierKeys(), true);
	g_root->InvokeKey(character, TB_KEY_UNDEFINED, GetModifierKeys(), false);
}

static void key_callback(GLFWwindow window, int key, int action)
{
	MODIFIER_KEYS modifier = GetModifierKeys();
	bool down = (action == GLFW_PRESS);
	switch (key)
	{
	case GLFW_KEY_F1:			g_root->InvokeKey(0, TB_KEY_F1, modifier, down); break;
	case GLFW_KEY_F2:			g_root->InvokeKey(0, TB_KEY_F2, modifier, down); break;
	case GLFW_KEY_F3:			g_root->InvokeKey(0, TB_KEY_F3, modifier, down); break;
	case GLFW_KEY_F4:			g_root->InvokeKey(0, TB_KEY_F4, modifier, down); break;
	case GLFW_KEY_F5:			g_root->InvokeKey(0, TB_KEY_F5, modifier, down); break;
	case GLFW_KEY_F6:			g_root->InvokeKey(0, TB_KEY_F6, modifier, down); break;
	case GLFW_KEY_F7:			g_root->InvokeKey(0, TB_KEY_F7, modifier, down); break;
	case GLFW_KEY_F8:			g_root->InvokeKey(0, TB_KEY_F8, modifier, down); break;
	case GLFW_KEY_F9:			g_root->InvokeKey(0, TB_KEY_F9, modifier, down); break;
	case GLFW_KEY_F10:			g_root->InvokeKey(0, TB_KEY_F10, modifier, down); break;
	case GLFW_KEY_F11:			g_root->InvokeKey(0, TB_KEY_F11, modifier, down); break;
	case GLFW_KEY_F12:			g_root->InvokeKey(0, TB_KEY_F12, modifier, down); break;
	case GLFW_KEY_LEFT:			g_root->InvokeKey(0, TB_KEY_LEFT, modifier, down); break;
	case GLFW_KEY_UP:			g_root->InvokeKey(0, TB_KEY_UP, modifier, down); break;
	case GLFW_KEY_RIGHT:		g_root->InvokeKey(0, TB_KEY_RIGHT, modifier, down); break;
	case GLFW_KEY_DOWN:			g_root->InvokeKey(0, TB_KEY_DOWN, modifier, down); break;
	case GLFW_KEY_PAGEUP:		g_root->InvokeKey(0, TB_KEY_PAGE_UP, modifier, down); break;
	case GLFW_KEY_PAGEDOWN:		g_root->InvokeKey(0, TB_KEY_PAGE_DOWN, modifier, down); break;
	case GLFW_KEY_HOME:			g_root->InvokeKey(0, TB_KEY_HOME, modifier, down); break;
	case GLFW_KEY_END:			g_root->InvokeKey(0, TB_KEY_END, modifier, down); break;
	case GLFW_KEY_INSERT:		g_root->InvokeKey(0, TB_KEY_INSERT, modifier, down); break;
	case GLFW_KEY_TAB:			g_root->InvokeKey(0, TB_KEY_TAB, modifier, down); break;
	case GLFW_KEY_DEL:			g_root->InvokeKey(0, TB_KEY_DELETE, modifier, down); break;
	case GLFW_KEY_BACKSPACE:	g_root->InvokeKey(0, TB_KEY_BACKSPACE, modifier, down); break;
	case GLFW_KEY_ENTER:		g_root->InvokeKey(0, TB_KEY_ENTER, modifier, down); break;
	case GLFW_KEY_ESC:			g_root->InvokeKey(0, TB_KEY_ESC, modifier, down); break;
	case GLFW_KEY_MENU:
		if (TBWidget::focused_widget)
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
		// At least the windows implementation of glfw calls KeyboardSpecial
		// when pressing a character while ctrl is also pressed.
		if (key_ctrl && !key_alt && key >= 32 && key <= 255)
			g_root->InvokeKey(key, TB_KEY_UNDEFINED, modifier, down);
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

			g_root->InvokePointerDown(x, y, counter, GetModifierKeys());
		}
		else
			g_root->InvokePointerUp(x, y, GetModifierKeys());
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
	{
		g_root->InvokePointerMove(x, y, GetModifierKeys());
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
	if (g_root)
		g_root->InvokePointerMove(x, y, GetModifierKeys());
}

static void scroll_callback(GLFWwindow window, double x, double y)
{
	if (g_root)
		g_root->InvokeWheel(mouse_x, mouse_y, (int)x, -(int)y, GetModifierKeys());
}

static void timer_callback()
{
	TBMessageHandler::ProcessMessages();

	// If we still have things to do (because we didn't process all messages,
	// or because there are new messages), we need to rescedule, so call RescheduleTimer.
	TBSystem::RescheduleTimer(TBMessageHandler::GetNextMessageFireTime());
}

// This doesn't really belong here (it belongs in tb_system_[linux/windows].cpp.
// This is here since the proper implementations has not yet been done.
void TBSystem::RescheduleTimer(double fire_time)
{
	if (fire_time == TB_NOT_SOON)
	{
		glfwKillTimer();
		return;
	}
	glfwRescheduleTimer(fire_time);
}

static void window_refresh_callback(GLFWwindow window)
{
	application->Process();

	has_pending_update = false;

	application->RenderFrame(window_w, window_h);

	glfwSwapBuffers(mainWindow);
}

static void window_size_callback(GLFWwindow window, int w, int h)
{
	window_w = w;
	window_h = h;
	if (g_root)
		g_root->SetRect(TBRect(0, 0, window_w, window_h));
}

class RootWidget : public TBWidget
{
public:
	virtual void OnInvalid()
	{
		has_pending_update = true;
		glfwPostNull(mainWindow);
	}
};

int main(int argc, char** argv)
{
	if (!glfwInit())
		exit(1);
	mainWindow = glfwCreateWindow(window_w, window_h, GLFW_WINDOWED, "", NULL);
	if (!mainWindow)
	{
		glfwTerminate();
		exit(1);
	}
    glfwMakeContextCurrent(mainWindow);

	glfwSetWindowTitle(mainWindow, "Demo");

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

	init_tinkerbell(new TBRendererGL(), "tinkerbell/lng_en.tb.txt");

	// Register tbbf font renderer
	void register_tbbf_font_renderer();
	register_tbbf_font_renderer();

	// Register freetype font renderer - if you compile with tb_font_renderer_freetype.cpp
	//void register_freetype_font_renderer();
	//register_freetype_font_renderer();

	// Add a font to the font manager.
	// If you use the freetype or stb backend, you can add true type files
	//g_font_manager->AddFontInfo("vera.ttf", "Vera");
	g_font_manager->AddFontInfo("tinkerbell/default_font/segoe_white_with_shadow.tb.txt", "Segoe");
	g_font_manager->AddFontInfo("tinkerbell/default_font/neon.tb.txt", "Neon");
	g_font_manager->AddFontInfo("tinkerbell/default_font/orangutang.tb.txt", "Orangutang");
	g_font_manager->AddFontInfo("tinkerbell/default_font/orange.tb.txt", "Orange");

	// Set the default font description for widgets to one of the fonts we just added
	TBFontDescription fd;
	fd.SetID(TBIDC("Segoe"));
	fd.SetSize(14);
	g_font_manager->SetDefaultFontDescription(fd);

	// Create the font now.
	TBFontFace *font = g_font_manager->CreateFontFace(g_font_manager->GetDefaultFontDescription());

	// Render some glyphs in one go now since we know we are going to use them. It would work fine
	// without this since glyphs are rendered when needed, but with some extra updating of the glyph bitmap.
	if (font)
		font->RenderGlyphs(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
							"€‚ƒ„…†‡ˆ‰Š‹ŒŽ‘’“”•–—˜™š›œžŸ¡¢£¤¥¦§¨©ª«¬®"
							"¯°±²³´µ¶·¸¹º»¼½¾¿ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßà�"
							"�âãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ");

	// Load the default skin, and override skin that contains the graphics specific to the demo.
	g_tb_skin->Load("tinkerbell/default_skin/skin.tb.txt", "Demo/skin/skin.tb.txt");

	// Create the root widget and give it the background skin
	g_root = new RootWidget;
	g_root->SetRect(TBRect(0, 0, window_w, window_h));
	g_root->SetSkinBg("background");

	// Create the application object for our demo
	application = new DemoApplication(g_root);
	application->Init();

	do
	{
		if (has_pending_update)
			window_refresh_callback(mainWindow);
		glfwWaitEvents();
	}
    while (!glfwGetWindowParam(mainWindow, GLFW_CLOSE_REQUESTED));

	delete application;
	application = nullptr;

	delete g_root;

	shutdown_tinkerbell();

	glfwTerminate();

	return 0;
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

	char *argv[2] = { "", 0 };
	// Crank up windows timer resolution (it's awfully low res normally). Note: This affects battery time!
	timeBeginPeriod(1);
	int ret = main(1, argv);
	timeEndPeriod(1);
	return ret;
}

#endif // WIN32
