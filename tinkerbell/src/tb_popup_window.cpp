// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_widgets_listener.h"
#include "tb_popup_window.h"

namespace tinkerbell {

// == TBPopupWindow ==========================================

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

bool TBPopupWindow::Show(const TBPoint *pos_in_root, TB_ALIGN align)
{
	// Calculate and set a good size for the popup window
	SetRect(GetAlignedRect(pos_in_root, align));

	TBWidget *root = m_target.Get()->GetParentRoot();
	root->AddChild(this);
	return true;
}

TBRect TBPopupWindow::GetAlignedRect(const TBPoint *pos_in_root, TB_ALIGN align)
{
	TBWidget *target = m_target.Get();
	TBWidget *root = target->GetParentRoot();
	PreferredSize ps = GetPreferredSize();

	TBRect target_rect;
	TBPoint pos;
	int w = MIN(ps.pref_w, root->GetRect().w);
	int h = MIN(ps.pref_h, root->GetRect().h);
	if (pos_in_root)
	{
		pos = *pos_in_root;
	}
	else
	{
		target->ConvertToRoot(pos.x, pos.y);
		// FIX: add a minimum_width! This would shrink it for every submenu!
		//w = CLAMP(ps.pref_w, target->m_rect.w, root->m_rect.w);

		// If the menu is aligned top or bottom, limit its height to the worst case available height.
		// Being in the center of the root, that is half the root height minus the target rect.
		if (align == TB_ALIGN_TOP || align == TB_ALIGN_BOTTOM)
			h = MIN(h, root->GetRect().h / 2 - target->GetRect().h);
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

void TBPopupWindow::OnWidgetFocusChanged(TBWidget *widget, bool focused)
{
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
	// If the target widget is deleted, close!
	if (widget == m_target.Get())
		Close();
	return false;
}

}; // namespace tinkerbell
