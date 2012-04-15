// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_editfield.h"
#include "tb_select.h"
#include "tb_system.h"
#include "tb_language.h"

namespace tinkerbell {

const int CARET_BLINK_TIME = 500;
const int SELECTION_SCROLL_DELAY = 1000/30;

/** Get the delta that should be scrolled if dragging the pointer outside the range min-max */
int GetSelectionScrollSpeed(int pointerpos, int min, int max)
{
	int d = 0;
	if (pointerpos < min)
		d = pointerpos - min;
	else if (pointerpos > max)
		d = pointerpos - max;
	d *= d;
	d /= 40;
	return (pointerpos < min) ? -d : d;
}

TBEditField::TBEditField()
	: m_edit_type(EDIT_TYPE_TEXT)
{
	SetIsFocusable(true);
	AddChild(&m_scrollbar_x);
	AddChild(&m_scrollbar_y);
	m_scrollbar_x.SetGravity(WIDGET_GRAVITY_BOTTOM | WIDGET_GRAVITY_LEFT_RIGHT);
	m_scrollbar_y.SetGravity(WIDGET_GRAVITY_RIGHT | WIDGET_GRAVITY_TOP_BOTTOM);
	m_scrollbar_y.SetAxis(AXIS_Y);
	int scrollbar_y_w = m_scrollbar_y.GetPreferredSize().pref_w;
	int scrollbar_x_h = m_scrollbar_x.GetPreferredSize().pref_h;
	m_scrollbar_x.SetRect(TBRect(0, m_rect.h - scrollbar_x_h, m_rect.w - scrollbar_y_w, scrollbar_x_h));
	m_scrollbar_y.SetRect(TBRect(m_rect.w - scrollbar_y_w, 0, scrollbar_y_w, m_rect.h));
	m_scrollbar_x.SetOpacity(0);
	m_scrollbar_y.SetOpacity(0);

	m_skin_bg.Set("TBEditField");
	m_style_edit.SetListener(this);

	m_placeholder.SetTextAlign(TB_TEXT_ALIGN_LEFT);
}

TBEditField::~TBEditField()
{
	RemoveChild(&m_scrollbar_y);
	RemoveChild(&m_scrollbar_x);
}

TBRect TBEditField::GetVisibleRect()
{
	TBRect rect = GetPaddingRect();
	if (m_scrollbar_y.GetOpacity())
		rect.w -= m_scrollbar_y.m_rect.w;
	if (m_scrollbar_x.GetOpacity())
		rect.h -= m_scrollbar_x.m_rect.h;
	return rect;
}

void TBEditField::SetMultiline(bool multiline)
{
	m_scrollbar_y.SetOpacity(multiline ? 1.f : 0.f);
	m_style_edit.SetMultiline(multiline);
	m_style_edit.SetWrapping(multiline);
}

void TBEditField::SetReadOnly(bool readonly)
{
	m_style_edit.SetReadOnly(readonly);
}

void TBEditField::SetWrapping(bool wrapping)
{
	m_style_edit.SetWrapping(wrapping);
}

void TBEditField::SetEditType(EDIT_TYPE type)
{
	m_edit_type = type;
	m_style_edit.SetPassword(type == EDIT_TYPE_PASSWORD);
}

bool TBEditField::OnEvent(const WidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CHANGED && (ev.target == &m_scrollbar_x || ev.target == &m_scrollbar_y))
	{
		m_style_edit.SetScrollPos(m_scrollbar_x.GetValue(), m_scrollbar_y.GetValue());
		return true;
	}
	else if (ev.type == EVENT_TYPE_WHEEL)
	{
		int old_val = m_scrollbar_y.GetValue();
		m_scrollbar_y.SetValue(old_val + ev.delta * 40);
		return m_scrollbar_y.GetValue() != old_val;
	}
	else if (ev.type == EVENT_TYPE_POINTER_DOWN && ev.target == this)
	{
		// Post a message to start selection scroll
		PostMessageDelayed(TBIDC("selscroll"), nullptr, SELECTION_SCROLL_DELAY);
		TBRect padding_rect = GetPaddingRect();
		m_style_edit.MouseDown(TBPoint(ev.target_x - padding_rect.x, ev.target_y - padding_rect.y), 1, ev.count, 0);
		return true;
	}
	else if (ev.type == EVENT_TYPE_POINTER_MOVE && ev.target == this)
	{
		TBRect padding_rect = GetPaddingRect();
		m_style_edit.MouseMove(TBPoint(ev.target_x - padding_rect.x, ev.target_y - padding_rect.y));
		return true;
	}
	else if (ev.type == EVENT_TYPE_POINTER_UP && ev.target == this)
	{
		TBRect padding_rect = GetPaddingRect();
		m_style_edit.MouseUp(TBPoint(ev.target_x - padding_rect.x, ev.target_y - padding_rect.y), 1, 0);
		return true;
	}
	else if (ev.type == EVENT_TYPE_KEY_DOWN)
	{
		return m_style_edit.KeyDown(ev.key, ev.special_key, ev.modifierkeys);
	}
	else if (ev.type == EVENT_TYPE_KEY_UP)
	{
		return true;
	}
	else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("popupmenu"))
	{
		if (ev.ref_id == TBIDC("cut") && !m_style_edit.packed.read_only)
			m_style_edit.Cut();
		else if (ev.ref_id == TBIDC("copy"))
			m_style_edit.Copy();
		else if (ev.ref_id == TBIDC("paste") && !m_style_edit.packed.read_only)
			m_style_edit.Paste();
		else if (ev.ref_id == TBIDC("delete") && !m_style_edit.packed.read_only)
			m_style_edit.Delete();
		else if (ev.ref_id == TBIDC("selectall"))
			m_style_edit.selection.SelectAll();
	}
	else if (ev.type == EVENT_TYPE_CONTEXT_MENU && ev.target == this)
	{
		TBPoint pos_in_root(ev.target_x, ev.target_y);
		ev.target->ConvertToRoot(pos_in_root.x, pos_in_root.y);

		if (TBGenericStringItemSource *source = new TBGenericStringItemSource)
		{
			source->AddItem(new TBGenericStringItem(g_tb_lng->GetString(TBIDC("cut")), TBIDC("cut")));
			source->AddItem(new TBGenericStringItem(g_tb_lng->GetString(TBIDC("copy")), TBIDC("copy")));
			source->AddItem(new TBGenericStringItem(g_tb_lng->GetString(TBIDC("paste")), TBIDC("paste")));
			source->AddItem(new TBGenericStringItem(g_tb_lng->GetString(TBIDC("delete")), TBIDC("delete")));
			source->AddItem(new TBGenericStringItem("-"));
			source->AddItem(new TBGenericStringItem(g_tb_lng->GetString(TBIDC("selectall")), TBIDC("selectall")));
			if (TBMenuWindow *menu = new TBMenuWindow(ev.target, TBIDC("popupmenu")))
				menu->Show(source, -1, &pos_in_root);
		}
		return true;
	}
	return false;
}

void TBEditField::OnPaint(const PaintProps &paint_props)
{
	TBRect visible_rect = GetVisibleRect();

	bool clip = m_scrollbar_x.CanScroll() || m_scrollbar_y.CanScroll();
	TBRect old_clip;
	if (clip)
		old_clip = g_renderer->SetClipRect(visible_rect, true);

	int trans_x = visible_rect.x, trans_y = visible_rect.y;
	g_renderer->Translate(trans_x, trans_y);

	// Draw text content, caret etc.
	visible_rect.x = visible_rect.y = 0;
	m_style_edit.Paint(visible_rect, paint_props.text_color);

	// If empty, draw placeholder text with some opacity.
	if (m_style_edit.IsEmpty())
	{
		float old_opacity = g_renderer->GetOpacity();
		g_renderer->SetOpacity(old_opacity * 0.2f);
		m_placeholder.Paint(visible_rect, paint_props.text_color);
		g_renderer->SetOpacity(old_opacity);
	}

	g_renderer->Translate(-trans_x, -trans_y);

	if (clip)
		g_renderer->SetClipRect(old_clip, false);
}

void TBEditField::OnFocusChanged(bool focused)
{
	m_style_edit.Focus(focused);
}

void TBEditField::OnResized(int old_w, int old_h)
{
	// Make the scrollbars move
	Widget::OnResized(old_w, old_h);

	TBRect visible_rect = GetVisibleRect();
	m_style_edit.SetLayoutSize(visible_rect.w, visible_rect.h);

	UpdateScrollbars();
}

PreferredSize TBEditField::GetPreferredContentSize()
{
	PreferredSize ps;
	ps.pref_h = ps.min_h = g_renderer->GetFontHeight();
	if (m_style_edit.packed.multiline_on)
	{
		ps.pref_w = g_renderer->GetFontHeight() * 10;
		ps.pref_h = g_renderer->GetFontHeight() * 5;
	}
	else
		ps.max_h = ps.pref_h;
	return ps;
}

void TBEditField::OnMessageReceived(TBMessage *msg)
{
	if (msg->message == TBIDC("blink"))
	{
		m_style_edit.caret.on = !m_style_edit.caret.on;
		m_style_edit.caret.Invalidate();
	
		// Post another blink message so we blink again.
		PostMessageDelayed(TBIDC("blink"), nullptr, CARET_BLINK_TIME);
	}
	else if (msg->message == TBIDC("selscroll") && captured_widget == this)
	{
		// Get scroll speed from where mouse is relative to the padding rect.
		TBRect padding_rect = GetVisibleRect().Shrink(2, 2);
		int dx = GetSelectionScrollSpeed(pointer_move_widget_x, padding_rect.x, padding_rect.x + padding_rect.w);
		int dy = GetSelectionScrollSpeed(pointer_move_widget_y, padding_rect.y, padding_rect.y + padding_rect.h);
		m_scrollbar_x.SetValue(m_scrollbar_x.GetValue() + dx);
		m_scrollbar_y.SetValue(m_scrollbar_y.GetValue() + dy);

		// Handle mouse move at the new scroll position, so selection is updated
		if (dx || dy)
			m_style_edit.MouseMove(TBPoint(pointer_move_widget_x, pointer_move_widget_y));

		// Post another setscroll message so we continue scrolling if we still should.
		if (m_style_edit.select_state)
			PostMessageDelayed(TBIDC("selscroll"), nullptr, SELECTION_SCROLL_DELAY);
	}
}

void TBEditField::OnChange()
{
	WidgetEvent ev(EVENT_TYPE_CHANGED, 0, 0);
	InvokeEvent(ev);
}

bool TBEditField::OnEnter()
{
	return false;
}

void TBEditField::Invalidate(const TBRect &rect)
{
	Widget::Invalidate();
}

void TBEditField::SetStyle(PStyle *style)
{
}

void TBEditField::DrawString(int32 x, int32 y, const TBColor &color, const char *str, int32 len)
{
	g_renderer->DrawString(x, y, color, str, len);
}

void TBEditField::DrawBackground(const TBRect &rect, PBlock *block)
{
}

void TBEditField::DrawRect(const TBRect &rect, const TBColor &color)
{
	g_renderer->DrawRect(rect, color);
}

void TBEditField::DrawRectFill(const TBRect &rect, const TBColor &color)
{
	g_renderer->DrawRectFill(rect, color);
}

void TBEditField::DrawTextSelectionBg(const TBRect &rect)
{
	g_tb_skin->PaintSkin(rect, TBIDC("TBEditField.selection"), GetAutoState());
}

void TBEditField::DrawContentSelectionFg(const TBRect &rect)
{
	g_tb_skin->PaintSkin(rect, TBIDC("TBEditField.selection"), GetAutoState());
}

void TBEditField::DrawCaret(const TBRect &rect)
{
	if (GetIsFocused() && !m_style_edit.packed.read_only)
		DrawTextSelectionBg(rect);
}

void TBEditField::Scroll(int32 dx, int32 dy)
{
	Widget::Invalidate();
	m_scrollbar_x.SetValue(m_style_edit.scroll_x);
	m_scrollbar_y.SetValue(m_style_edit.scroll_y);
}

void TBEditField::UpdateScrollbars()
{
	int32 w = m_style_edit.layout_width;
	int32 h = m_style_edit.layout_height;
	m_scrollbar_x.SetLimits(0, m_style_edit.GetContentWidth() - w, w);
	m_scrollbar_y.SetLimits(0, m_style_edit.GetContentHeight() - h, h);
}

void TBEditField::CaretBlinkStart()
{
	// Post the delayed blink message if we don't already have one
	if (!GetMessageByID(TBIDC("blink")))
		PostMessageDelayed(TBIDC("blink"), nullptr, CARET_BLINK_TIME);
}

void TBEditField::CaretBlinkStop()
{
	// Remove the blink message if we have one
	if (TBMessage *msg = GetMessageByID(TBIDC("blink")))
		DeleteMessage(msg);
}

}; // namespace tinkerbell
