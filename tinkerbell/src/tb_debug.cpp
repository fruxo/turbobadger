// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tinkerbell.h"
#include "tb_widgets_reader.h"
#include "tb_window.h"

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
};

void ShowDebugInfoSettingsWindow(TBWidget *root)
{
	new DebugSettingsWindow(root);
}

#endif // TB_RUNTIME_DEBUG_INFO

}; // namespace tinkerbell
