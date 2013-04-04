// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_widgets_listener.h"
#include "tb_popup_window.h"

namespace tinkerbell {

// == TBPopupAlignment ======================================================================================

TBRect TBPopupAlignment::GetAlignedRect(TBWidget *popup, TBWidget *target) const
{
	TBWidget *root = target->GetParentRoot();
	PreferredSize ps = popup->GetPreferredSize();

	TBRect target_rect;
	TBPoint pos;
	int w = MIN(ps.pref_w, root->GetRect().w);
	int h = MIN(ps.pref_h, root->GetRect().h);
	if (pos_in_root.x != UNSPECIFIED &&
		pos_in_root.y != UNSPECIFIED)
	{
		pos = pos_in_root;
	}
	else
	{
		target->ConvertToRoot(pos.x, pos.y);

		if (align == TB_ALIGN_TOP || align == TB_ALIGN_BOTTOM)
		{
			if (expand_to_target_width)
				w = MAX(w, target->GetRect().w);

			// If the menu is aligned top or bottom, limit its height to the worst case available height.
			// Being in the center of the root, that is half the root height minus the target rect.
			h = MIN(h, root->GetRect().h / 2 - target->GetRect().h);
		}
		target_rect = target->GetRect();
	}

	int x, y;
	if (align == TB_ALIGN_BOTTOM)
	{
		x = pos.x;
		y = pos.y + target_rect.h + h > root->GetRect().h ? pos.y - h : pos.y + target_rect.h;
	}
	else if (align == TB_ALIGN_TOP)
	{
		x = pos.x;
		y = pos.y - h < 0 ? pos.y + target_rect.h : pos.y - h;
	}
	else if (align == TB_ALIGN_RIGHT)
	{
		x = pos.x + target_rect.w + w > root->GetRect().w ? pos.x - w : pos.x + target_rect.w;
		y = MIN(pos.y, root->GetRect().h - h);
	}
	else //if (align == TB_ALIGN_LEFT)
	{
		x = pos.x - w < 0 ? pos.x + target_rect.w : pos.x - w;
		y = MIN(pos.y, root->GetRect().h - h);
	}

	return TBRect(x, y, w, h);
}

// == TBPopupWindow =========================================================================================

TBPopupWindow::TBPopupWindow(TBWidget *target)
	: m_target(target)
{
	TBGlobalWidgetListener::AddListener(this);
	SetSkinBg("TBPopupWindow", WIDGET_INVOKE_INFO_NO_CALLBACKS);
	SetSettings(WINDOW_SETTINGS_NONE);
}

TBPopupWindow::~TBPopupWindow()
{
	TBGlobalWidgetListener::RemoveListener(this);
}

bool TBPopupWindow::Show(const TBPopupAlignment &alignment)
{
	// Calculate and set a good size for the popup window
	SetRect(alignment.GetAlignedRect(this, m_target.Get()));

	TBWidget *root = m_target.Get()->GetParentRoot();
	root->AddChild(this);
	return true;
}

bool TBPopupWindow::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_KEY_DOWN && ev.special_key == TB_KEY_ESC)
	{
		Close();
		return true;
	}
	return TBWindow::OnEvent(ev);
}

void TBPopupWindow::OnWidgetFocusChanged(TBWidget *widget, bool focused)
{
	if (focused && !IsEventDestinationFor(widget))
		Close();
}

bool TBPopupWindow::OnWidgetInvokeEvent(TBWidget *widget, const TBWidgetEvent &ev)
{
	if ((ev.type == EVENT_TYPE_POINTER_DOWN || ev.type == EVENT_TYPE_CONTEXT_MENU) &&
		!IsEventDestinationFor(ev.target))
		Close();
	return false;
}

void TBPopupWindow::OnWidgetDelete(TBWidget *widget)
{
	// If the target widget is deleted, close!
	if (!m_target.Get())
		Close();
}

bool TBPopupWindow::OnWidgetDying(TBWidget *widget)
{
	// If the target widget or an ancestor of it is dying, close!
	if (widget == m_target.Get() || widget->IsAncestorOf(m_target.Get()))
		Close();
	return false;
}

}; // namespace tinkerbell
