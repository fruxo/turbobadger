#include <stdlib.h>

#include "App.h"
#include "renderers/tb_renderer_gl.h"

#include "tb_core.h"
#include "tb_system.h"
#include "tb_widgets.h"
#include "tb_select.h"
#include "tb_font_renderer.h"
#include "tb_language.h"

#include "tb_widgets_reader.h"
#include "tb_window.h"
#include "tb_message_window.h"
#include "tb_editfield.h"

using namespace tb;

TBRendererGL *renderer;
TBWidget *root;
int g_width, g_height;

class AppRoot : public TBWidget
{
public:
	enum TEST {
		TEST_INFLATE,
		TEST_RESIZE
	};
	void TestSpeed(TEST test)
	{
		const char *title = "";
		const int iteration_count = 300;
		double total_time;

		if (test == TEST_INFLATE)
		{
			title = "Inflate + layout speed";
			double start_time = TBSystem::GetTimeMS();
			for(int i = 0; i < iteration_count; i++)
			{
				TBWindow *win = new TBWindow;
				g_widgets_reader->LoadFile(win, "layout/main_layout.tb.txt");
				win->SetText(title);
				win->SetSize(100 + i, 100 + i);
				win->Close();
			}
			total_time = TBSystem::GetTimeMS() - start_time;
		}
		else
		{
			title = "Resizing layout speed";
			TBWindow *win = new TBWindow;
			win->SetText(title);
			g_widgets_reader->LoadFile(win, "layout/main_layout.tb.txt");

			double start_time = TBSystem::GetTimeMS();
			for(int i = 0; i < iteration_count; i++)
				win->SetSize(100 + i, 100 + i);
			total_time = TBSystem::GetTimeMS() - start_time;

			win->Close();
		}

		TBStr text;
		text.SetFormatted(	"Total time for %d iterations:\n"
							"%dms\n"
							"Average time per iteration:\n"
							"%.2fms",
							iteration_count, (int)(total_time), (float)(total_time) / (float)iteration_count);
		TBMessageWindow *msg_win = new TBMessageWindow(GetParentRoot(), TBIDC(""));
		msg_win->Show(title, text);
	}

	void ShowScreenInfo()
	{
		TBStr text;
		text.SetFormatted(	"Screen DPI: %d\n"
							"Closest skin DPI: %d\n"
							"Root dimensions: %dx%dpx\n"
							"100dp equals %dpx (Based on closest skin DPI and screen DPI)",
							TBSystem::GetDPI(),
							g_tb_skin->GetDimensionConverter()->GetDstDPI(),
							GetParentRoot()->GetRect().w, GetParentRoot()->GetRect().h,
							g_tb_skin->GetDimensionConverter()->DpToPx(100));
		TBMessageWindow *msg_win = new TBMessageWindow(GetParentRoot(), TBIDC(""));
		msg_win->Show("Screen info", text);
	}

	virtual bool OnEvent(const TBWidgetEvent &ev)
	{
		if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("speed test inflate"))
			TestSpeed(TEST_INFLATE);
		else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("speed test resize"))
			TestSpeed(TEST_RESIZE);
		else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("screen-info"))
			ShowScreenInfo();
		else
			return false;
		return true;
	}
};

// == LISTEN TO KEYBOARD FOCUS TO CALL ANDROID KEYBOARD ON JAVA SIDE ====================

class EditListener : public TBWidgetListener
{
public:
	// == TBWidgetListener ========================================================
	virtual void OnWidgetFocusChanged(TBWidget *widget, bool focused)
	{
		if (TBEditField *edit = TBSafeCast<TBEditField>(widget))
			if (!edit->GetReadOnly())
				ShowKeyboard(focused);
	}
};

EditListener edit_listener;

// == CALLS FROM JAVA TO NATIVE =========================================================

void Init(unsigned int width, unsigned int height)
{
	renderer = new TBRendererGL();
	tb_core_init(renderer);
	root = new AppRoot();
	Resize(width, height);

	// Start listening to keyboard focus
	TBWidgetListener::AddGlobalListener(&edit_listener);

	// Load language file
	g_tb_lng->Load("language/lng_en.tb.txt");

	// Load the default skin, and override skin that contains the graphics specific to the demo.
	g_tb_skin->Load("skin/skin.tb.txt", "demo_skin/skin.tb.txt");

	// Register tbbf font renderer
	void register_tbbf_font_renderer();
	register_tbbf_font_renderer();

	// Add a font to the font manager.
	g_font_manager->AddFontInfo("font/segoe_white_with_shadow.tb.txt", "Segoe");

	// Set the default font description for widgets to one of the fonts we just added
	TBFontDescription fd;
	fd.SetID(TBIDC("Segoe"));
	fd.SetSize(g_tb_skin->GetDimensionConverter()->DpToPx(14));
	g_font_manager->SetDefaultFontDescription(fd);

	// Create the font now.
	TBFontFace *font = g_font_manager->CreateFontFace(g_font_manager->GetDefaultFontDescription());

	// Give the root widget a background skin
	root->SetSkinBg("background_solid");

	g_widgets_reader->LoadFile(root, "layout/main_layout.tb.txt");
}

void Resize(unsigned int width, unsigned int height)
{
	g_width = width;
	g_height = height;
	root->SetSize(width, height);
}

void Update()
{
	// FIX: This should be schedueled, and we should only paint when needed.
	TBMessageHandler::ProcessMessages();

	root->InvokeProcessStates();
	root->InvokeProcess();
}

void Render()
{
	// ... any gl code goes here ...
	// We have a background skin on root, so we don't need to clear the screen.
	// glViewport(0, 0, g_Width, g_Height);
	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	g_renderer->BeginPaint(root->GetRect().w, root->GetRect().h);

	root->InvokePaint(TBWidget::PaintProps());

	static int fps = 0;
	static uint32 frame_counter = 0;
	static double frame_counter_reset_time = TBSystem::GetTimeMS();

	frame_counter++;

	// Update the FPS counter
	double time = TBSystem::GetTimeMS();
	if (time > frame_counter_reset_time + 1000)
	{
		fps = (int) ((frame_counter / (time - frame_counter_reset_time)) * 1000);
		frame_counter_reset_time = time;
		frame_counter = 0;
	}

	// Draw FPS
	TBStr str;
	str.SetFormatted("FPS: %d", fps);
	root->GetFont()->DrawString(5, 5, TBColor(255, 255, 255), str);

	g_renderer->EndPaint();
}

void Shutdown()
{
	delete root;
	TBWidgetListener::RemoveGlobalListener(&edit_listener);
	tb_core_shutdown();
	delete renderer;
}
