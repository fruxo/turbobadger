#ifdef MACOSX
#include "GLUT/glut.h"
#define glutLeaveMainLoop() exit(0)
#else
#include "GL/freeglut.h"
#endif
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

Widget *g_root = nullptr;

Application *application = nullptr;
/*static */Application *Application::GetApp() { return application; }

int mainWindow;
int window_w = 1280;
int window_h = 740;
bool has_pending_update = false;

MODIFIER_KEYS GlutModToTBMod()
{
	int glut_modifier = glutGetModifiers();
	MODIFIER_KEYS modifier = 0;
	if (glut_modifier & GLUT_ACTIVE_SHIFT)	modifier |= TB_SHIFT;
	if (glut_modifier & GLUT_ACTIVE_CTRL)	modifier |= TB_CTRL;
	if (glut_modifier & GLUT_ACTIVE_ALT)	modifier |= TB_ALT;
	return modifier;
}

void Keyboard(unsigned char key, int x, int y, bool down)
{
	MODIFIER_KEYS modifier = GlutModToTBMod();
	switch (key)
	{
	case 27:	g_root->InvokeKey(0, TB_KEY_ESC, modifier, down);		break;
	case 127:	g_root->InvokeKey(0, TB_KEY_DELETE, modifier, down);	break;
	case 8:		g_root->InvokeKey(0, TB_KEY_BACKSPACE, modifier, down);	break;
	case 9:		g_root->InvokeKey(0, TB_KEY_TAB, modifier, down);		break;
	case 13:	g_root->InvokeKey(0, TB_KEY_ENTER, modifier, down);		break;
	default:
		g_root->InvokeKey(key, 0, modifier, down);
		break;
	}
}

void KeyboardSpecial(int key, int x, int y, bool down)
{
	MODIFIER_KEYS modifier = GlutModToTBMod();
	switch (key)
	{
	case GLUT_KEY_F1:			g_root->InvokeKey(0, TB_KEY_F1, modifier, down); break;
	case GLUT_KEY_F2:			g_root->InvokeKey(0, TB_KEY_F2, modifier, down); break;
	case GLUT_KEY_F3:			g_root->InvokeKey(0, TB_KEY_F3, modifier, down); break;
	case GLUT_KEY_F4:			g_root->InvokeKey(0, TB_KEY_F4, modifier, down); if (modifier & TB_ALT) glutLeaveMainLoop(); break;
	case GLUT_KEY_F5:			g_root->InvokeKey(0, TB_KEY_F5, modifier, down); break;
	case GLUT_KEY_F6:			g_root->InvokeKey(0, TB_KEY_F6, modifier, down); break;
	case GLUT_KEY_F7:			g_root->InvokeKey(0, TB_KEY_F7, modifier, down); break;
	case GLUT_KEY_F8:			g_root->InvokeKey(0, TB_KEY_F8, modifier, down); break;
	case GLUT_KEY_F9:			g_root->InvokeKey(0, TB_KEY_F9, modifier, down); break;
	case GLUT_KEY_F10:			g_root->InvokeKey(0, TB_KEY_F10, modifier, down); break;
	case GLUT_KEY_F11:			g_root->InvokeKey(0, TB_KEY_F11, modifier, down); break;
	case GLUT_KEY_F12:			g_root->InvokeKey(0, TB_KEY_F12, modifier, down); break;
	case GLUT_KEY_LEFT:			g_root->InvokeKey(0, TB_KEY_LEFT, modifier, down); break;
	case GLUT_KEY_UP:			g_root->InvokeKey(0, TB_KEY_UP, modifier, down); break;
	case GLUT_KEY_RIGHT:		g_root->InvokeKey(0, TB_KEY_RIGHT, modifier, down); break;
	case GLUT_KEY_DOWN:			g_root->InvokeKey(0, TB_KEY_DOWN, modifier, down); break;
	case GLUT_KEY_PAGE_UP:		g_root->InvokeKey(0, TB_KEY_PAGE_UP, modifier, down); break;
	case GLUT_KEY_PAGE_DOWN:	g_root->InvokeKey(0, TB_KEY_PAGE_DOWN, modifier, down); break;
	case GLUT_KEY_HOME:			g_root->InvokeKey(0, TB_KEY_HOME, modifier, down); break;
	case GLUT_KEY_END:			g_root->InvokeKey(0, TB_KEY_END, modifier, down); break;
	case GLUT_KEY_INSERT:		g_root->InvokeKey(0, TB_KEY_INSERT, modifier, down); break;
	default: break;
	}
}

void KeyboardDown(unsigned char key, int x, int y)	{ Keyboard(key, x, y, true); }
void KeyboardUp(unsigned char key, int x, int y)	{ Keyboard(key, x, y, false); }
void KeyboardSpecialDown(int key, int x, int y)		{ KeyboardSpecial(key, x, y, true); }
void KeyboardSpecialUp(int key, int x, int y)		{ KeyboardSpecial(key, x, y, false); }

void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			// Glut doesn't support doubleclick. This is a quick fix with n-click support :)
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

			g_root->InvokePointerDown(x, y, counter, GlutModToTBMod());
		}
		else
			g_root->InvokePointerUp(x, y, GlutModToTBMod());
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
	{
		g_root->InvokePointerMove(x, y, GlutModToTBMod());
		if (Widget::hovered_widget)
		{
			Widget::hovered_widget->ConvertFromRoot(x, y);
			TBWidgetEvent ev(EVENT_TYPE_CONTEXT_MENU, x, y, GlutModToTBMod());
			Widget::hovered_widget->InvokeEvent(ev);
		}
	}
}

void MouseMotion(int x, int y)
{
	g_root->InvokePointerMove(x, y, GlutModToTBMod());
}

void MouseMotionPassive(int x, int y)
{
	g_root->InvokePointerMove(x, y, GlutModToTBMod());
}

void MouseWheel(int wheel, int direction, int x, int y)
{
	g_root->InvokeWheel(x, y, direction > 0 ? -1 : 1, GlutModToTBMod());
}

void TimerFunc(int value)
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
	// Note: Unfortunately, glut can't cancel old set timers we no longer want to fire,
	//       so this will bleed timer callbacks if rescedule while already waiting for a callback.

	if (fire_time == TB_NOT_SOON)
		return;

	static double set_fire_time = -1;
	if (fire_time != set_fire_time)
	{
		set_fire_time = fire_time;
		glutTimerFunc((unsigned int)(fire_time - GetTimeMS()), TimerFunc, 0);
	}
}

void DisplayFunc()
{
	// Make sure pending update is true, so anything causing invalidate in Process
	// doesn't end up with another glutPostRedisplay we don't want from OnInvalid.
	// We know we're rendering the frame below anyway.
	has_pending_update = true;
	application->Process();
	has_pending_update = false;

	application->RenderFrame(window_w, window_h);

	glutSwapBuffers();
}

void ResizeFunc(int w, int h)
{
	window_w = w;
	window_h = h;
	g_root->SetRect(TBRect(0, 0, window_w, window_h));
}

class RootWidget : public Widget
{
public:
	virtual void OnInvalid()
	{
		if (has_pending_update)
			return;
		has_pending_update = true;
		glutPostRedisplay();
	}
};

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(window_w, window_h);
	mainWindow = glutCreateWindow("Demo");
#ifndef MACOSX
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif

	glutDisplayFunc(DisplayFunc);
	glutReshapeFunc(ResizeFunc);
	glutKeyboardFunc(KeyboardDown);
	glutKeyboardUpFunc(KeyboardUp);
	glutSpecialFunc(KeyboardSpecialDown);
	glutSpecialUpFunc(KeyboardSpecialUp);
	glutMouseFunc(Mouse);
#ifndef MACOSX
	glutMouseWheelFunc(MouseWheel);
#endif
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(MouseMotionPassive);

	init_tinkerbell(new TBRendererGL(), "tinkerbell/lng_en.tb.txt");

	// Register tbbf font renderer
	void register_tbbf_font_renderer();
	register_tbbf_font_renderer();

	// Register freetype font renderer - if you compile with tb_font_renderer_freetype.cpp
	//void register_freetype_font_renderer();
	//register_freetype_font_renderer();

	// Add a font to the font manager.
	// If you use the freetype or stb backend, you can add true type files
	// TBFontInfo *font_info = g_font_manager->AddFontInfo("vera.ttf");
	TBFontInfo *font_info = g_font_manager->AddFontInfo("tinkerbell/default_font/segoe_white_with_shadow.tb.txt");
	TBFontInfo *font_info2 = g_font_manager->AddFontInfo("tinkerbell/default_font/neon.tb.txt");

	// Set the default font description for widgets to the index of the font info we just added
	TBFontDescription fd;
	fd.SetIndex(font_info->GetIndex());
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

	glutMainLoop();

	delete application;
	application = nullptr;

	delete g_root;

	shutdown_tinkerbell();

	return 0;
}

#ifdef WIN32

#include <mmsystem.h>
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	char *argv[2] = { "", 0 };
	// Crank up windows timer resolution (it's awfully low res normally). Note: This affects battery time!
	timeBeginPeriod(1);
	int ret = main(1, argv);
	timeEndPeriod(1);
	return ret;
}

#endif // WIN32
