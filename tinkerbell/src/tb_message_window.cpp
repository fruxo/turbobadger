// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_message_window.h"
#include "tb_widgets_reader.h"
#include "tb_editfield.h"
#include "tb_language.h"
#include <assert.h>

namespace tinkerbell {

// == TBMessageWindow =======================================================================================

TBMessageWindow::TBMessageWindow(TBWidget *target, TBID id)
	: TBWidgetSafePointer(target)
{
	m_id.Set(id);
}

TBMessageWindow::~TBMessageWindow()
{
	if (TBWidget *dimmer = m_dimmer.Get())
	{
		dimmer->m_parent->RemoveChild(dimmer);
		delete dimmer;
	}
}

bool TBMessageWindow::Show(const char *title, const char *message, TBMessageWindowSettings *settings)
{
	TBWidget *target = Get();
	if (!target)
		return false;

	TBMessageWindowSettings default_settings;
	if (!settings)
		settings = &default_settings;

	TBWidget *root = target->GetParentRoot();

	const char *source =	"TBLayout: axis: y, distribution: available\n"
							"	TBLayout: distribution: available, size: available\n"
							"		TBSkinImage: id: 2\n"
							"		TBEditField: multiline: 1, readonly: 1, id: 1\n"
							"	TBLayout: distribution_position: right bottom, id: 3\n";
	g_widgets_reader->LoadData(GetContentRoot(), source);

	SetText(title);

	TBSafeGetByID(TBSkinImage, 2)->SetSkinBg(settings->icon_skin);

	TBEditField *editfield = TBSafeGetByID(TBEditField, 1);
	editfield->SetText(message);
	editfield->SetSkinBg("");

	// Create buttons
	if (settings->msg == TB_MSG_OK)
	{
		AddButton("TBMessageWindow.ok", true);
	}
	else if (settings->msg == TB_MSG_OK_CANCEL)
	{
		AddButton("TBMessageWindow.ok", true);
		AddButton("TBMessageWindow.cancel", false);
	}
	else if (settings->msg == TB_MSG_YES_NO)
	{
		AddButton("TBMessageWindow.yes", true);
		AddButton("TBMessageWindow.no", false);
	}

	// Size to fit content. This will use the default size of the textfield.
	ResizeToFitContent();

	// Get how much we overflow the textfield has given the current width, and grow our height to show all we can.
	int new_height = m_rect.h + editfield->GetStyleEdit()->GetOverflowY();
	new_height = MIN(new_height, root->m_rect.h - 50); // 50px for some window title and padding

	// Create background dimmer
	if (settings->dimmer)
	{
		if (TBDimmer *dimmer = new TBDimmer)
		{
			root->AddChild(dimmer);
			m_dimmer.Set(dimmer);
		}
	}

	// Center and size to the new height
	SetRect(TBRect((root->m_rect.w - m_rect.w) / 2,
					(root->m_rect.h - new_height) / 2,
					m_rect.w, new_height));
	root->AddChild(this);
	return true;
}

void TBMessageWindow::AddButton(TBID id, bool focused)
{
	TBLayout *layout = TBSafeGetByID(TBLayout, 3);
	if (!layout)
		return;
	if (TBButton *btn = new TBButton)
	{
		btn->GetID().Set(id);
		btn->SetText(g_tb_lng->GetString(btn->GetID()));
		layout->AddChild(btn);
		if (focused)
			btn->SetFocus(WIDGET_FOCUS_REASON_UNKNOWN);
	}
}

bool TBMessageWindow::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CLICK && ev.target->IsOfType("TBButton"))
	{
		TBWidgetSafePointer this_widget(this);

		// Invoke the click on the target
		TBWidgetEvent target_ev(EVENT_TYPE_CLICK);
		target_ev.ref_id = ev.target->m_id;
		InvokeEvent(target_ev);

		// If target got deleted, close
		if (this_widget.Get())
			Close();
		return true;
	}
	else if (ev.type == EVENT_TYPE_KEY_DOWN && ev.special_key == TB_KEY_ESC)
	{
		m_close_button.InvokeEvent(TBWidgetEvent(EVENT_TYPE_CLICK));
		return true;
	}
	return TBWindow::OnEvent(ev);
}

void TBMessageWindow::OnDie()
{
	if (TBWidget *dimmer = m_dimmer.Get())
		dimmer->Die();
}

void TBMessageWindow::OnWidgetDelete(TBWidget *widget)
{
	TBWidgetSafePointer::OnWidgetDelete(widget);
	// If the target widget is deleted, close!
	if (!Get())
		Close();
}

}; // namespace tinkerbell
