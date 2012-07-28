// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_window.h"
#include <assert.h>

namespace tinkerbell {

// == TBWindow ==========================================================================

TBWindow::TBWindow()
	: m_settings(WINDOW_SETTINGS_DEFAULT)
{
	m_skin_bg.Set("TBWindow");
	AddChild(&m_mover);
	AddChild(&m_resizer);
	m_mover.SetSkinBg("TBWindow.mover");
	m_mover.AddChild(&m_textfield);
	m_textfield.SetIgnoreInput(true);
	m_mover.AddChild(&m_close_button);
	m_close_button.SetSkinBg("TBWindow.close");
	m_close_button.SetIsFocusable(false);
	m_close_button.GetID().Set("TBWindow.close");
	SetIsGroupRoot(true);
}

TBWindow::~TBWindow()
{
	if (m_resizer.m_parent)			RemoveChild(&m_resizer);
	if (m_mover.m_parent)			RemoveChild(&m_mover);
	if (m_close_button.m_parent)	m_mover.RemoveChild(&m_close_button);
	m_mover.RemoveChild(&m_textfield);
}

void TBWindow::ResizeToFitContent(RESIZE_FIT fit)
{
	PreferredSize ps = GetPreferredSize();
	int new_w = ps.pref_w;
	int new_h = ps.pref_h;
	if (fit == RESIZE_FIT_MINIMAL)
	{
		new_w = ps.min_w;
		new_h = ps.min_h;
	}
	else if (fit == RESIZE_FIT_CURRENT_OR_NEEDED)
	{
		new_w = CLAMP(m_rect.w, ps.min_w, ps.max_w);
		new_h = CLAMP(m_rect.h, ps.min_h, ps.max_h);
	}
	if (m_parent)
	{
		new_w = MIN(new_w, m_parent->m_rect.w);
		new_h = MIN(new_h, m_parent->m_rect.h);
	}
	SetRect(TBRect(m_rect.x, m_rect.y, new_w, new_h));
}

void TBWindow::Close()
{
	Die();
}

bool TBWindow::IsActive()
{
	return GetState(WIDGET_STATE_SELECTED);
}

TBWindow *TBWindow::GetTopMostOtherWindow(bool only_activable_windows)
{
	TBWindow *other_window = nullptr;
	TBWidget *sibling = m_parent->GetLastChild();
	while (sibling && !other_window)
	{
		if (sibling != this)
			other_window = TBSafeCast(TBWindow, sibling);

		if (only_activable_windows && other_window && !(other_window->m_settings & WINDOW_SETTINGS_CAN_ACTIVATE))
			other_window = nullptr;

		sibling = sibling->GetPrev();
	}
	return other_window;
}

void TBWindow::Activate()
{
	if (!m_parent || IsActive() || !(m_settings & WINDOW_SETTINGS_CAN_ACTIVATE))
		return;

	// Deactivate currently active window
	TBWindow *active_window = GetTopMostOtherWindow(true);
	if (active_window)
		active_window->DeActivate();

	// Activate this window

	SetZ(WIDGET_Z_TOP);
	SetWindowActiveState(true);
	EnsureFocus();
}

bool TBWindow::EnsureFocus()
{
	// If we already have focus, we're done.
	if (focused_widget && IsAncestorOf(focused_widget))
		return true;

	// Focus last focused widget (if we have one)
	bool success = false;
	if (m_last_focus.Get())
		success = m_last_focus.Get()->SetFocus(WIDGET_FOCUS_REASON_UNKNOWN);
	if (!success)
	{
		// Search for a child widget that accepts focus
		TBWidget *child = GetFirstChild();
		while (child && IsAncestorOf(child))
		{
			if (child->SetFocus(WIDGET_FOCUS_REASON_UNKNOWN))
			{
				success = true;
				break;
			}
			child = child->GetNextDeep();
		}
	}
	return success;
}

void TBWindow::DeActivate()
{
	if (!IsActive())
		return;
	SetWindowActiveState(false);
}

void TBWindow::SetWindowActiveState(bool active)
{
	SetState(WIDGET_STATE_SELECTED, active);
	m_mover.SetState(WIDGET_STATE_SELECTED, active);
}

void TBWindow::SetSettings(WINDOW_SETTINGS settings)
{
	if (settings == m_settings)
		return;
	m_settings = settings;

	if (settings & WINDOW_SETTINGS_TITLEBAR)
	{
		if (!m_mover.m_parent)			AddChild(&m_mover);
	}
	else if (!(settings & WINDOW_SETTINGS_TITLEBAR))
	{
		if (m_mover.m_parent)			RemoveChild(&m_mover);
	}
	if (settings & WINDOW_SETTINGS_RESIZABLE)
	{
		if (!m_resizer.m_parent)		AddChild(&m_resizer);
	}
	else if (!(settings & WINDOW_SETTINGS_RESIZABLE))
	{
		if (m_resizer.m_parent)			RemoveChild(&m_resizer);
	}
	if (settings & WINDOW_SETTINGS_CLOSE_BUTTON)
	{
		if (!m_close_button.m_parent)	AddChild(&m_close_button);
	}
	else if (!(settings & WINDOW_SETTINGS_CLOSE_BUTTON))
	{
		if (m_close_button.m_parent)	RemoveChild(&m_close_button);
	}

	// FIX: invalidate layout / resize window!
	Invalidate();
}

int TBWindow::GetTitleHeight()
{
	if (m_settings & WINDOW_SETTINGS_TITLEBAR)
		return m_mover.GetPreferredSize().pref_h;
	return 0;
}

TBRect TBWindow::GetPaddingRect()
{
	TBRect padding_rect = TBWidget::GetPaddingRect();
	int title_height = GetTitleHeight();
	padding_rect.y += title_height;
	padding_rect.h -= title_height;
	return padding_rect;
}

PreferredSize TBWindow::GetPreferredSize()
{
	PreferredSize ps = GetPreferredContentSize();

	// Add window skin padding
	if (TBSkinElement *e = GetSkinBgElement())
	{
		ps.min_w += e->padding_left + e->padding_right;
		ps.pref_w += e->padding_left + e->padding_right;
		ps.min_h += e->padding_top + e->padding_bottom;
		ps.pref_h += e->padding_top + e->padding_bottom;
	}
	// Add window title bar height
	int title_height = GetTitleHeight();
	ps.min_h += title_height;
	ps.pref_h += title_height;
	return ps;
}

bool TBWindow::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.target == &m_close_button)
	{
		if (ev.type == EVENT_TYPE_CLICK)
			Close();
		return true;
	}
	return false;
}

void TBWindow::OnAdded()
{
	// If we was added last, call Activate to update status etc.
	if (m_parent->GetLastChild() == this)
		Activate();
}

void TBWindow::OnRemove()
{
	DeActivate();

	// Active the top most other window
	if (TBWindow *active_window = GetTopMostOtherWindow(true))
		active_window->Activate();
}

void TBWindow::OnChildAdded(TBWidget *child)
{
	m_resizer.SetZ(WIDGET_Z_TOP);
}

void TBWindow::OnResized(int old_w, int old_h)
{
	// Apply gravity on children
	TBWidget::OnResized(old_w, old_h);
	// Manually move our own decoration children
	// FIX: Put a layout in the TBMover so we can add things there nicely.
	int title_height = GetTitleHeight();
	m_mover.SetRect(TBRect(0, 0, m_rect.w, title_height));
	m_resizer.SetRect(TBRect(m_rect.w - 20, m_rect.h - 20, 20, 20));
	TBRect mover_rect = m_mover.GetPaddingRect();
	int button_size = mover_rect.h;
	m_close_button.SetRect(TBRect(mover_rect.x + mover_rect.w - button_size, mover_rect.y, button_size, button_size));
	if (m_settings & WINDOW_SETTINGS_CLOSE_BUTTON)
		mover_rect.w -= button_size;
	m_textfield.SetRect(mover_rect);
}

}; // namespace tinkerbell
