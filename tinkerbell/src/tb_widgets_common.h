// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_WIDGETS_COMMON_H
#define TB_WIDGETS_COMMON_H

#include "tb_widgets.h"
#include "tb_layout.h"
#include "tb_msg.h"

namespace tinkerbell {

/** TB_TEXT_ALIGN specifies horizontal text alignment */
enum TB_TEXT_ALIGN {
	TB_TEXT_ALIGN_LEFT,		///< Aligned left
	TB_TEXT_ALIGN_RIGHT,	///< Aligned right
	TB_TEXT_ALIGN_CENTER	///< Aligned center
};

/** TBWidgetString shows a one line string with the set alignment. */
class TBWidgetString
{
public:
// FIX: Use PStyleEdit if wanted, to support rich text
	TBWidgetString();

	void Paint(const TBRect &rect, const TBColor &color);

	int GetWidth();
	int GetHeight();

	bool SetText(const char *text) { return m_text.Set(text); }
	bool GetText(TBStr &text) const { return text.Set(m_text); }

	/** Set which alignment the text should have if the space
		given when painting is larger than the text. */
	void SetTextAlign(TB_TEXT_ALIGN align) { m_text_align = align; }
	TB_TEXT_ALIGN GetTextAlign() { return m_text_align; }
public:
	TBStr m_text;
	TB_TEXT_ALIGN m_text_align;
};

/** TBTextField is a one line textfield that is not editable. */

class TBTextField : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBTextField", Widget);

	TBTextField();

	/** Set the text of the textfield. */
	virtual bool SetText(const char *text);
	virtual bool GetText(TBStr &text) { return m_text.GetText(text); }

	/** Set which alignment the text should have if the space
		given when painting is larger than the text. */
	void SetTextAlign(TB_TEXT_ALIGN align) { m_text.SetTextAlign(align); }
	TB_TEXT_ALIGN GetTextAlign() { return m_text.GetTextAlign(); }

	/** Set if this textfield should be allowed to squeeze below its
		preferred size. If squeezable it may shrink to width 0. */
	void SetSqueezable(bool squeezable);
	bool GetSqueezable() { return m_squeezable; }

	virtual PreferredSize GetPreferredContentSize();
	virtual void OnPaint(const PaintProps &paint_props);
protected:
	TBWidgetString m_text;
	bool m_squeezable;
};

/** TBButton is a regular button widget.
	Has a textfield in its internal layout by default. Other widgets can be added
	under GetContentRoot(). */

class TBButton : public Widget, protected TBMessageHandler
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBButton", Widget);

	TBButton();
	~TBButton();

	/** Set along which axis the content should layouted (If the button has more content than the text) */
	void SetAxis(AXIS axis) { m_layout.SetAxis(axis); }
	AXIS GetAxis() const { return m_layout.GetAxis(); }

	/** Set if the textfield should be allowed to squeeze below its
		preferred size. If squeezable it may shrink to width 0. */
	void SetSqueezable(bool squeezable) { m_textfield.SetSqueezable(squeezable); }
	bool GetSqueezable() { return m_textfield.GetSqueezable(); }

	/** Set to true if the button should fire repeatedly while pressed. */
	void SetAutoRepeat(bool auto_repeat_click) { m_auto_repeat_click = auto_repeat_click; }
	bool GetAutoRepeat() { return m_auto_repeat_click; }

	/** Set the text of the button. */
	virtual bool SetText(const char *text) { return m_textfield.SetText(text); }
	virtual bool GetText(TBStr &text) { return m_textfield.GetText(text); }

	virtual void SetValue(int value) { SetState(WIDGET_STATE_PRESSED, value ? true : false); }
	virtual int GetValue() { return GetState(WIDGET_STATE_PRESSED); }

	virtual void OnCaptureChanged(bool captured);
	virtual void OnSkinChanged();
	virtual WIDGET_HIT_STATUS GetHitStatus(int x, int y);
	virtual PreferredSize GetPreferredContentSize() { return m_layout.GetPreferredSize(); }

	virtual Widget *GetContentRoot() { return &m_layout; }

	// == TBMessageHandler ==============================================================
	virtual void OnMessageReceived(TBMessage *msg);
protected:
	TBLayout m_layout;
	TBTextField m_textfield;
	bool m_auto_repeat_click;
};

/** TBClickLabel has a textfield in its internal layout by default. Pointer input on the textfield
	will be redirected to another child widget (that you add) to it.
	Typically useful for creating checkboxes, radiobuttons with labels. */

class TBClickLabel : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBClickLabel", Widget);

	TBClickLabel();
	~TBClickLabel();

	/** Set along which axis the content should layouted (If the label has more content than the text) */
	void SetAxis(AXIS axis) { m_layout.SetAxis(axis); }
	AXIS GetAxis() const { return m_layout.GetAxis(); }

	/** Set the text of the label. */
	virtual bool SetText(const char *text) { return m_textfield.SetText(text); }
	virtual bool GetText(TBStr &text) { return m_textfield.GetText(text); }

	virtual PreferredSize GetPreferredContentSize() { return m_layout.GetPreferredSize(); }

	virtual Widget *GetContentRoot() { return &m_layout; }

	virtual bool OnEvent(const WidgetEvent &ev);
protected:
	TBLayout m_layout;
	TBTextField m_textfield;
};

/** TBSkinImage is a widget only showing a skin, constrained in size to its skin. */

class TBSkinImage : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBSkinImage", Widget);

	TBSkinImage() {}
	TBSkinImage(TBID skin_bg) { SetSkinBg(skin_bg); }

	virtual PreferredSize GetPreferredSize();
};

/** TBSeparator is a widget only showing a skin.
	It is disabled by default. */
class TBSeparator : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBSeparator", Widget);

// FIX: Should take base skin, and add .x or .y depending on parent AXIS!
	TBSeparator() { SetState(WIDGET_STATE_DISABLED, true); }
};

/** TBProgressSpinner is a animation that is running while its value is 1.
	Typically used to indicate that the application is working. */
class TBProgressSpinner : public Widget, protected TBMessageHandler
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBProgressSpinner", Widget);

	TBProgressSpinner();

	/** Return true if the animation is running. */
	bool IsRunning() { return m_value > 0; }

	/** Begin/End are used to start or stop the animation in a incremental way.
		If several sources may activate the same spinner, calling Begin/End instead
		of using SetValue, so it will keep running as long as any source wants it to. */
	void Begin() { SetValue(GetValue() + 1); }
	void End() { SetValue(GetValue() - 1); }

	virtual void SetValue(int value);
	virtual int GetValue() { return m_value; }

	virtual void OnPaint(const PaintProps &paint_props);

	// == TBMessageHandler ==============================================================
	virtual void OnMessageReceived(TBMessage *msg);
protected:
	int m_value;
	int m_frame;
	TBID m_skin_fg;
};

/** TBRadioCheckBox has shared functionality for TBCheckBox and TBRadioButton. */
class TBRadioCheckBox : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBRadioCheckBox", Widget);

	TBRadioCheckBox();

	virtual void SetValue(int value);
	virtual int GetValue() { return m_value; }

	virtual PreferredSize GetPreferredSize();
	virtual bool OnEvent(const WidgetEvent &ev);
protected:
	void ToggleGroup(Widget *root, Widget *toggled);
	int m_value;
};

/** TBCheckBox is a box toggling a check mark on click.
	For a labeled checkbox, use a TBClickLabel containing a TBCheckBox. */
class TBCheckBox : public TBRadioCheckBox
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBCheckBox", TBRadioCheckBox);

	TBCheckBox() { m_skin_bg.Set("TBCheckBox"); }
};

/** TBRadioButton is a button which unselects other radiobuttons of the same
	group number when clicked.
	For a labeled radio button, use a TBClickLabel containing a TBRadioButton. */
class TBRadioButton : public TBRadioCheckBox
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBRadioButton", TBRadioCheckBox);

	TBRadioButton() { m_skin_bg.Set("TBRadioButton"); }
};

/** TBScrollBar is a scroll bar in the given axis. */

class TBScrollBar : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBScrollBar", Widget);

	TBScrollBar();
	~TBScrollBar();

	/** Set along which axis the scrollbar should scroll */
	void SetAxis(AXIS axis);
	AXIS GetAxis() const { return m_axis; }

	/** Set the min, max limits for the scrollbar.
		The visible parameter is how much of the range that is visible.
		When this is called, the scrollbar might change value and invoke if the current value is
		outside of the new limits. */
	void SetLimits(double min, double max, double visible);

	/** Return true if the scrollbar has anywhere to go with the current limits. */
	bool CanScroll() const { return m_visible > 0; }

	double GetMinValue() const { return m_min; }
	double GetMaxValue() const { return m_max; }
	double GetVisible() const { return m_visible; }

	void SetValueDouble(double value);
	double GetValueDouble() { return m_value; }

	void SetValue(int value) { SetValueDouble(value); }
	int GetValue() { return (int) GetValueDouble(); }

	virtual bool OnEvent(const WidgetEvent &ev);
protected:
	Widget m_handle;
	AXIS m_axis;
	double m_value;
	double m_min, m_max, m_visible;
	double m_to_pixel_factor;
	void UpdateHandle();
};

/** TBContainer is just a Widget with border and padding (using skin "TBContainer") */
class TBContainer : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBContainer", Widget);

	TBContainer();
};

/** TBMover is moving its parent widget when dragged. */
class TBMover : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBMover", Widget);

	TBMover();

	virtual bool OnEvent(const WidgetEvent &ev);
};

/** TBResizer is a lower right corner resize grip. It will resize its parent widget. */
class TBResizer : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBResizer", Widget);

	TBResizer();
	virtual bool OnEvent(const WidgetEvent &ev);
};

/** TBDimmer dim widgets in the background and block input. */
class TBDimmer : public Widget
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("TBDimmer", Widget);

	TBDimmer();

	virtual void OnAdded();
};

}; // namespace tinkerbell

#endif // TB_WIDGETS_COMMON_H
