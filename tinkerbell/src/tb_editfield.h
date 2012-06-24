// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_EDITFIELD_H
#define TB_EDITFIELD_H

#include "tb_widgets_common.h"
#include "tb_msg.h"
#include "tb_style_edit.h"

namespace tinkerbell {

/** EDIT_TYPE - Theese types does not restrict input (may change in the future).
	They are just hints for virtual keyboard, so it can show special keys. */
enum EDIT_TYPE {
	EDIT_TYPE_TEXT,				///< Any text allowed
	EDIT_TYPE_PASSWORD,			///< Any text allowed
	EDIT_TYPE_EMAIL,			///< Any text allowed
	EDIT_TYPE_PHONE,			///< Any text allowed
	EDIT_TYPE_URL,				///< Any text allowed
	EDIT_TYPE_NUMBER			///< Any text allowed
};

/** The default content factory for embedded content in TBEditField with styling enabled.

	Creates all that TBTextFragmentContentFactory creates by default,
	and any type of widget from a inline resource string.

	Syntax: <widget xxx> Where xxx is parsed by TBWidgetsReader.

	Example - Create a button with id "hello":

		<widget TBButton: text: "Hello world!" id: "hello">

	Example - Create a image from skin element "Icon48":

		<widget TBSkinImage: skin: "Icon48">
*/

class TBEditFieldContentFactory : public TBTextFragmentContentFactory
{
public:
	class TBEditField *editfield;
	virtual int GetContent(const char *text);
	virtual TBTextFragmentContent *CreateFragmentContent(const char *text, int text_len);
};

/** TBEditFieldScrollRoot - Internal for TBEditField.
	Acts as a scrollable container for any widget created as embedded content. */

class TBEditFieldScrollRoot : public Widget
{
private: // May only be used by TBEditField.
	friend class TBEditField;
	TBEditFieldScrollRoot() {}
public:
	virtual void OnPaintChildren(const PaintProps &paint_props);
	virtual void GetChildTranslation(int &x, int &y) const;
	virtual WIDGET_HIT_STATUS GetHitStatus(int x, int y);
};

/** TBEditField is a one line or multi line textfield that is editable or
	read-only. It can also be a passwordfield by calling
	SetEditType(EDIT_TYPE_PASSWORD).
	
	It may perform styling of text and contain custom embedded content,
	if enabled by SetStyling(true). Disabled by default.
*/

class TBEditField : public Widget, private TBStyleEditListener, public TBMessageHandler
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBEditField", Widget);

	TBEditField();
	~TBEditField();

	/** Get the visible rect (the GetPaddingRect() minus any scrollbars) */
	TBRect GetVisibleRect();

	/** Set if multiple lines should be allowed or not.
		Will also set wrapping (to true if multiline, and false if not). */
	void SetMultiline(bool multiline);
	bool GetMultiline() const { return m_style_edit.packed.multiline_on; }

	/** Set if styling should be enabled or not. Default is disabled. */
	void SetStyling(bool styling);
	bool GetStyling() const { return m_style_edit.packed.styling_on; }

	void SetReadOnly(bool readonly);
	bool GetReadOnly() const { return m_style_edit.packed.read_only; }

	void SetWrapping(bool wrapping);
	bool GetWrapping() const { return m_style_edit.packed.wrapping; }

	TBStyleEdit *GetStyleEdit() { return &m_style_edit; }

	void SetEditType(EDIT_TYPE type);
	EDIT_TYPE GetEditType() { return m_edit_type; }

	/** Set which alignment the text should have if the space
		given when painting is larger than the text. */
	void SetTextAlign(TB_TEXT_ALIGN align) { m_style_edit.SetAlign(align); }
	TB_TEXT_ALIGN GetTextAlign() { return m_style_edit.align; }

	virtual bool SetText(const char *text) { return m_style_edit.SetText(text, TB_CARET_POS_BEGINNING); }
	virtual bool GetText(TBStr &text) { return m_style_edit.GetText(text); }
	using Widget::GetText; ///< Make all versions in base class available.

	/** Set the text and also specify if the caret should be positioned at the beginning
		or end of the text. */
	bool SetText(const char *text, TB_CARET_POS pos)
											{ return m_style_edit.SetText(text, pos); }
	/** Set the text of the given length and also specify if the caret should be positioned
		at the beginning or end of the text. */
	bool SetText(const char *text, int text_len, TB_CARET_POS pos = TB_CARET_POS_BEGINNING)
											{ return m_style_edit.SetText(text, text_len, pos); }

	/** Set the placeholder text. It will be visible only when the textfield is empty. */
	virtual bool SetPlaceholderText(const char *text) { return m_placeholder.SetText(text); }
	virtual bool GetPlaceholderText(TBStr &text) { return m_placeholder.GetText(text); }

	virtual bool OnEvent(const WidgetEvent &ev);
	virtual void OnPaint(const PaintProps &paint_props);
	virtual void OnAdded();
	virtual void OnFontChanged();
	virtual void OnFocusChanged(bool focused);
	virtual void OnResized(int old_w, int old_h);
	virtual Widget *GetContentRoot() { return &m_root; }

	virtual PreferredSize GetPreferredContentSize();

	virtual void OnMessageReceived(TBMessage *msg);
private:
	TBScrollBar m_scrollbar_x;
	TBScrollBar m_scrollbar_y;
	TBWidgetString m_placeholder;
	EDIT_TYPE m_edit_type;
	TBEditFieldScrollRoot m_root;
	TBEditFieldContentFactory m_content_factory;
	TBStyleEdit m_style_edit;

	// == TBStyleEditListener =======================
	virtual void OnChange();
	virtual bool OnEnter();
	virtual void Invalidate(const TBRect &rect);
	virtual void DrawString(int32 x, int32 y, TBFontFace *font, const TBColor &color, const char *str, int32 len);
	virtual void DrawBackground(const TBRect &rect, TBBlock *block);
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
