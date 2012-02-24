// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_EDITFIELD_H
#define TB_EDITFIELD_H

#include "tb_widgets_common.h"
#include "tb_msg.h"
#include "PStyleEdit.h"

namespace tinkerbell {

/** EDIT_TYPE - Theese types does not restrict input (may change in the future).
	They are just hints for virtual keyboard, so it can show special keys. */
enum EDIT_TYPE {
	EDIT_TYPE_TEXT,				///< Any text allowed
	EDIT_TYPE_EMAIL,			///< Any text allowed
	EDIT_TYPE_PHONE,			///< Any text allowed
	EDIT_TYPE_URL,				///< Any text allowed
	EDIT_TYPE_NUMBER			///< Any text allowed
};

/** TBEditField is a one line or multi line textfield that is editable.
	It can also be a passwordfield by calling SetPassword(true). */

class TBEditField : public Widget, private PStyleEditListener, public TBMessageHandler
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBEditField", Widget);

	TBEditField();
	~TBEditField();

	/** Get the visible rect (the GetPaddingRect() minus any scrollbars) */
	TBRect GetVisibleRect();

	void SetMultiline(bool multiline);
	void SetReadOnly(bool readonly);
	void SetPassword(bool password);
	void SetWrapping(bool wrapping);

	PStyleEdit *GetStyleEdit() { return &m_style_edit; }

	void SetEditType(EDIT_TYPE type) { m_edit_type = type; }
	EDIT_TYPE GetEditType() { return m_edit_type; }

	/** Set which alignment the text should have if the space
		given when painting is larger than the text. */
	void SetTextAlign(TB_TEXT_ALIGN align) { m_style_edit.SetAlign(align); }
	TB_TEXT_ALIGN GetTextAlign() { return m_style_edit.align; }

	virtual bool SetText(const char *text) { return m_style_edit.SetText(text); }
	virtual bool SetText(const char *text, bool move_caret_to_end) { return m_style_edit.SetText(text, move_caret_to_end); }
	virtual bool GetText(TBStr &text) { return m_style_edit.GetText(text); }

	/** Set the placeholder text. It will be visible only when the textfield is empty. */
	virtual bool SetPlaceholderText(const char *text) { return m_placeholder.SetText(text); }
	virtual bool GetPlaceholderText(TBStr &text) { return m_placeholder.GetText(text); }

	virtual bool OnEvent(const WidgetEvent &ev);
	virtual void OnPaint(const PaintProps &paint_props);
	virtual void OnFocusChanged(bool focused);
	virtual void OnResized(int old_w, int old_h);

	virtual PreferredSize GetPreferredContentSize();

	virtual void OnMessageReceived(TBMessage *msg);
private:
	PStyleEdit m_style_edit;
	TBScrollBar m_scrollbar_x;
	TBScrollBar m_scrollbar_y;
	TBWidgetString m_placeholder;
	EDIT_TYPE m_edit_type;

	// == PStyleEditListener =======================
	virtual void OnChange();
	virtual bool OnEnter();
	virtual void Invalidate(const TBRect &rect);
	virtual void SetStyle(PStyle *style);
	virtual void DrawString(int32 x, int32 y, const TBColor &color, const char *str, int32 len);
	virtual void DrawBackground(const TBRect &rect, PBlock *block);
	virtual void DrawRect(const TBRect &rect, const TBColor &color);
	virtual void DrawRectFill(const TBRect &rect, const TBColor &color);
	virtual void DrawTextSelectionBg(const TBRect &rect);
	virtual void DrawContentSelectionFg(const TBRect &rect);
	virtual void DrawCaret(const TBRect &rect);
	virtual void Scroll(int32 dx, int32 dy);
	virtual void UpdateScrollbars();
	virtual void CaretBlinkStart();
	virtual void CaretBlinkStop();
};

}; // namespace tinkerbell

#endif // TB_EDITFIELD_H
