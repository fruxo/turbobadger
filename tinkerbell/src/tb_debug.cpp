// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tinkerbell.h"
#include "tb_widgets_reader.h"
#include "tb_window.h"
#include "tb_font_renderer.h"

namespace tinkerbell {

#ifdef TB_RUNTIME_DEBUG_INFO

TBDebugInfo g_tb_debug;

TBDebugInfo::TBDebugInfo()
{
	memset(settings, 0, sizeof(int) * NUM_SETTINGS);
}

/** Window showing runtime debug settings. */
class DebugSettingsWindow : public TBWindow
{
public:
	DebugSettingsWindow(TBWidget *root)
	{
		SetText("Debug settings");
		g_widgets_reader->LoadData(this, "TBLayout: id: 'container', axis: y, size: available");

		AddCheckbox(TBDebugInfo::LAYOUT_BOUNDS, "Layout bounds");
		AddCheckbox(TBDebugInfo::LAYOUT_CLIPPING, "Layout clipping");
		AddCheckbox(TBDebugInfo::LAYOUT_PS_DEBUGGING, "Layout size calculation");
		AddCheckbox(TBDebugInfo::RENDER_BATCHES, "Render batches");
		AddCheckbox(TBDebugInfo::RENDER_SKIN_BITMAP_FRAGMENTS, "Render skin bitmap fragments");
		AddCheckbox(TBDebugInfo::RENDER_FONT_BITMAP_FRAGMENTS, "Render font bitmap fragments");

		ResizeToFitContent();
		SetPosition(TBPoint((root->GetRect().w - GetRect().w) / 2,
							(root->GetRect().h - GetRect().h) / 2));

		root->AddChild(this);
	}

	void AddCheckbox(TBDebugInfo::SETTING setting, const char *str)
	{
		TBCheckBox *check = new TBCheckBox();
		check->SetValue(g_tb_debug.settings[setting]);
		check->data.SetInt(setting);
		check->SetID(TBIDC("check"));

		TBClickLabel *label = new TBClickLabel();
		label->SetText(str);
		label->GetContentRoot()->AddChild(check, WIDGET_Z_BOTTOM);

		GetWidgetByID(TBIDC("container"))->AddChild(label);
	}

	virtual bool OnEvent(const TBWidgetEvent &ev)
	{
		if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("check"))
		{
			// Update setting and invalidate
			g_tb_debug.settings[ev.target->data.GetInt()] = ev.target->GetValue();
			GetParentRoot()->Invalidate();
			return true;
		}
		return TBWindow::OnEvent(ev);
	}

	virtual void OnPaint(const PaintProps &paint_props)
	{
		// Draw stuff to the right of the debug window
		g_renderer->Translate(GetRect().w, 0);

		// Draw skin bitmap fragments
		if (TB_DEBUG_SETTING(RENDER_SKIN_BITMAP_FRAGMENTS))
			g_tb_skin->Debug();

		// Draw font glyph fragments (the font of the hovered widget)
		if (TB_DEBUG_SETTING(RENDER_FONT_BITMAP_FRAGMENTS))
		{
			TBWidget *widget = TBWidget::hovered_widget ? TBWidget::hovered_widget : TBWidget::focused_widget;
			g_font_manager->GetFontFace(widget ?
										widget->GetCalculatedFontDescription() :
										g_font_manager->GetDefaultFontDescription())->Debug();
		}

		g_renderer->Translate(-GetRect().w, 0);
	}
};

void ShowDebugInfoSettingsWindow(TBWidget *root)
{
	new DebugSettingsWindow(root);
}

#endif // TB_RUNTIME_DEBUG_INFO

}; // namespace tinkerbell
