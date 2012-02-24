// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef PStyleEdit_H
#define PStyleEdit_H

#include "tinkerbell.h"
#include "tb_widgets_common.h"
#include "tb_linklist.h"
#include "tb_list.h"

namespace tinkerbell {

class PStyleEdit;
class PStyle;
class PBlock;
class PElement;

/** Listener for PStyleEdit. Implement in the enviorment the PStyleEdit should render its content. */

class PStyleEditListener
{
public:
	virtual ~PStyleEditListener() {}

	// onbeginpanit onendpaint?
	virtual void OnChange() {};
	virtual bool OnEnter() { return false; };
	virtual void Invalidate(const TBRect &rect) = 0;
	virtual void SetStyle(PStyle *style) = 0;
	virtual void DrawString(int32 x, int32 y, const TBColor &color, const char *str, int32 len) = 0;
	virtual void DrawBackground(const TBRect &rect, PBlock *block) = 0;
	virtual void DrawRect(const TBRect &rect, const TBColor &color) = 0;
	virtual void DrawRectFill(const TBRect &rect, const TBColor &color) = 0;
	virtual void DrawTextSelectionBg(const TBRect &rect) = 0;
	virtual void DrawContentSelectionFg(const TBRect &rect) = 0;
	virtual void DrawCaret(const TBRect &rect) = 0;
	virtual void Scroll(int32 dx, int32 dy) = 0;
	virtual void UpdateScrollbars() = 0;
	virtual void CaretBlinkStart() = 0;
	virtual void CaretBlinkStop() = 0;
};

/*class PStyleEditLayoutListener
{
public:
	// ha extra row hantering. så att man kan fåtill markeringsyta med brakepoints etc.
	// refcounten
	// makes it possible to add a special C++ highlighter with specific breakproperties.
	virtual bool IsNeverBreakBefore();
	virtual bool IsNeverBreakAfter();
};*/

class TBTextOfs
{
public:
	TBTextOfs() : block(nullptr), ofs(0) {}
	TBTextOfs(PBlock *block, int32 ofs) : block(block), ofs(ofs) {}

	void Set(PBlock *new_block, int32 new_ofs) { block = new_block; ofs = new_ofs; }
	void Set(const TBTextOfs &pos) { block = pos.block; ofs = pos.ofs; }

	int32 GetGlobalOfs(PStyleEdit *se);
	bool SetGlobalOfs(PStyleEdit *se, int32 gofs);

public:
	PBlock *block;
	int32 ofs;
};

/** Handles the selected text in a PStyleEdit. */

class PSelection
{
public:
	PSelection(PStyleEdit *styledit);
	void InvalidateChanged(PBlock *start_block1, int32 start_ofs1, PBlock *stop_block1, int32 stop_ofs1,
							PBlock *start_block2, int32 start_ofs2, PBlock *stop_block2, int32 stop_ofs2);
	void Invalidate();
	void Select(const TBTextOfs &new_start, const TBTextOfs &new_stop);
	void Select(const TBPoint &from, const TBPoint &to);
	void SelectToCaret(PBlock *old_caret_block, int32 old_caret_ofs);
	void SelectAll();
	void SelectNothing();
	void CorrectOrder();
	void CopyToClipboard();
	bool IsElementSelected(PElement *elm);
	bool IsSelected();
	void RemoveContent();
	bool GetText(TBStr &text);
public:
	PStyleEdit *styledit;
	TBTextOfs start, stop;
};

enum P_CARET_POS {
	P_CARET_POS_BEGINNING,
	P_CARET_POS_END
};

/** The caret in a PStyleEdit. */

class PCaret
{
public:
	PCaret(PStyleEdit *styledit);
	void Invalidate();
	void UpdatePos();
	bool Move(bool forward, bool word);
	bool Place(const TBPoint &point);
	bool Place(PBlock *block, int ofs, bool allow_snap = true, bool snap_forward = false);
	void Place(P_CARET_POS place);
	void AvoidLineBreak();
	void Paint(int32 translate_x, int32 translate_y);
	void ResetBlink();
	void UpdateWantedX();

	int32 GetGlobalOfs() { return pos.GetGlobalOfs(styledit); }
	void SetGlobalOfs(int32 gofs);

	PElement *GetElement();
private:
	void SwitchBlock(bool second);
public:
	int32 x, y; ///< Relative to the styledit
	int width;
	int height;
	bool on;
	int32 wanted_x;
	bool prefer_first;
	PStyleEdit *styledit;
	TBTextOfs pos;
};

enum TB_TEXT_DECORATION {
	TB_TEXT_DECORATION_NONE,
	TB_TEXT_DECORATION_UNDERLINE
};

/** The style attributes holder for elements in PStyleEdit. */

class PStyle
{
public:
	PStyle();
	PStyle(const PStyle &style);
	~PStyle();

	void IncRef();
	void DecRef();

	void SetDecoration(TB_TEXT_DECORATION decoration) { this->decoration = decoration; }

	void Update();
	int32 GetStringWidth(const char *str, int len = -1);
	int32 GetTabWidth();
	int32 GetHeight();
	int32 GetBaseline();
public:
	TBFontDescription font;
	TBColor color;
	TB_TEXT_DECORATION decoration;
private:
	uint32 ref_count;
};

/*class PStyleA
{
public:
	PStyle *ref;
	PStyleA(PStyleA &style) { ref = style.ref; ref->IncRef(); }
	PStyleA(PStyle *new_ref) { ref = new_ref; ref->IncRef(); }
	~PStyleA() { ref->DecRef(); }
	void operator = (const PStyleA& style) { ref->DecRef(); ref = style.ref; ref->IncRef(); }
};*/

class PPaintInfo
{
public:
	TBColor text_color;
};

/** A block of text (a line, that might be wrapped) */

class PBlock : public TBLinkOf<PBlock>
{
public:
	PBlock(PStyleEdit *styledit);
	~PBlock();

	void Clear();
	void Set(const char *newstr, int32 len);
	void SetAlign(TB_TEXT_ALIGN align);

	int32 InsertText(int32 ofs, const char *text, int32 len, bool allow_line_recurse);
	void RemoveContent(int32 ofs, int32 len);

	void Split();
	void Merge();
	void Layout(bool propagate_height);
	void SetSize(int32 new_w, int32 new_h, bool propagate_height); ///< Moves all following blocks if the height changed.

	PElement *FindElement(int32 ofs, bool prefer_first = false);
	PElement *FindElement(int32 x, int32 y);

	void Invalidate();
	void Paint(int32 translate_x, int32 translate_y, const PPaintInfo &paint_props);
public:
	PStyleEdit *styledit;
	TBLinkListOf<PElement> elements;

	int32 ypos;
	int16 height;
	int8 align;

	uint32 extra_data; ///< Use to whatever you want. It is not used internally but is set to 0 initially.

	TBStr str;
	int32 str_len;

private:
	int GetStartIndentation();
	void AdjustElementPosition(PElement *start_element, int32 line_height, int32 line_baseline, int32 line_w);
};

/** Event in the PUndoRedoStack. Each insert or remove change is stored as a PUndoEvent, but they may also be merged when appropriate. */

class PUndoEvent
{
public:
	int32 gofs;
	TBStr text;
	bool insert;
};

/** Keeps track of all PUndoEvents used for undo and redo functionality. */

class PUndoRedoStack
{
public:
	PUndoRedoStack() : applying(false) {}
	~PUndoRedoStack();

	void Undo(PStyleEdit *styledit);
	void Redo(PStyleEdit *styledit);
	void Clear(bool clear_undo, bool clear_redo);

	PUndoEvent *Commit(PStyleEdit *styledit, int32 gofs, int32 len, const char *text, bool insert);
public:
	TBListOf<PUndoEvent> undos;
	TBListOf<PUndoEvent> redos;
	bool applying;
private:
	void Apply(PStyleEdit *styledit, PUndoEvent *e, bool reverse);
};

/** Content for a non-text PElement. */

class PElementContent
{
public:
	virtual ~PElementContent() {}
	virtual void Paint(PElement *element, int32 translate_x, int32 translate_y) {}
	virtual void Click(PElement *element, int button, uint32 modifierkeys) {}
	virtual int32 GetWidth(PElement *element) { return 0; }
	virtual int32 GetHeight(PElement *element) { return 0; }
	virtual int32 GetBaseline(PElement *element) { return GetHeight(element); }

	/** Get type of element content. All standard elements return 0. */
	virtual uint32 GetType()		{ return 0; }
};

/** The textfragment baseclass for PStyleEdit. */

class PElement : public TBLinkOf<PElement>
{
public:
	PElement(PElementContent *content = NULL)
				: xpos(0)
				, ypos(0)
				, ofs(0)
				, len(0)
				, line_ypos(0)
				, line_height(0)
				, block(NULL)
				, content(content) {}
	~PElement() { delete content; }

	void Init(PBlock *block, uint16 ofs, uint16 len);

	void Paint(int32 translate_x, int32 translate_y, const PPaintInfo &paint_props);
	void Click(int button, uint32 modifierkeys);

	bool IsText()					{ return !IsEmbedded(); }
	bool IsEmbedded()				{ return content ? true : false; }
	bool IsBreak();
	bool IsSpace();
	bool IsTab();

	/** Get type of element content. All standard elements return 0. */
	uint32 GetType()				{ return content ? content->GetType() : 0; }

	//enum { P_BREAK_NEVER_BEFORE, P_BREAK_NEVER_AFTER, P_BREAK_NEUTRAL };
	//virtual int16 GetBreakRule();

	int32 GetCharX(int32 ofs);
	int32 GetCharOfs(int32 x);
	/** Get the stringwidth. Handles passwordmode, tab, linebreaks etc automatically. */
	int32 GetStringWidth(PStyle *style, const char *str, int len = -1);

	const char *Str() const			{ return block->str.CStr() + ofs; }

	int32 GetWidth();
	int32 GetHeight();
	int32 GetBaseline();

	PStyle *GetStyle(); ///< Finds the style for this element.

public:
	int16 xpos, ypos;
	uint16 ofs, len;
	uint16 line_ypos;
	uint16 line_height;
	PBlock *block;
	PElementContent *content;
};

/** A horizontal line for PStyleEdit. */

class PHorizontalLineElement : public PElementContent
{
public:
	PHorizontalLineElement(int32 width_in_percent, int32 height, const TBColor &color);
	virtual ~PHorizontalLineElement();

	virtual void Paint(PElement *element, int32 translate_x, int32 translate_y);
	virtual int32 GetWidth(PElement *element);
	virtual int32 GetHeight(PElement *element);
private:
	int32 width_in_percent, height;
	TBColor color;
};

/** */
//FIX: Needs OnMove etc when a block is moved
// connect to child translation scroll
class TBEditField;
class PWidgetElement : public PElementContent
{
public:
	PWidgetElement(TBEditField *parent, Widget *widget);
	virtual ~PWidgetElement();

	virtual void Paint(PElement *element, int32 translate_x, int32 translate_y);
	virtual int32 GetWidth(PElement *element);
	virtual int32 GetHeight(PElement *element);
	virtual int32 GetBaseline(PElement *element);
private:
	TBEditField *m_parent;
	Widget *m_widget;
};

/** Edit and formats PElement's. It handles the text in a PStyleEditView. */

class PStyleEdit
{
public:
	PStyleEdit();
	virtual ~PStyleEdit();

	void SetListener(PStyleEditListener *listener);

	void Paint(const TBRect &rect, const TBColor &text_color);
	bool KeyDown(char ascii, uint16 function, uint32 modifierkeys);
	void MouseDown(const TBPoint &point, int button, int clicks, uint32 modifierkeys);
	void MouseUp(const TBPoint &point, int button, uint32 modifierkeys);
	void MouseMove(const TBPoint &point);
	void Focus(bool focus);

	void SetStyle(PStyle *style);
	PStyle *GetStyle(int index) { return styles[index]; }

//	void SetFont(const PFont &font);

	void Clear(bool init_new = true);
	bool Load(const char *filename);
	bool SetText(const char *text, bool place_caret_at_end = false);
	bool GetText(TBStr &text);
	bool IsEmpty();

	void SetAlign(TB_TEXT_ALIGN align);
	void SetMultiline(bool multiline = true);
	void SetReadOnly(bool readonly = true);
	void SetPassword(bool password = true);
	void SetWrapping(bool wrapping = true);
	void SetEnabled(bool enabled = true);

	void Cut();
	void Copy();
	void Paste();
	void Delete();

	void Undo() { undoredo.Undo(this); }
	void Redo() { undoredo.Redo(this); }
	bool CanUndo() { return undoredo.undos.GetNumItems() ? true : false; }
	bool CanRedo() { return undoredo.redos.GetNumItems() ? true : false; }

//	find stuff
//	TBStr GetText(bool styled);
	// void SetStyle();  If there is selection this applies style to the whole selection, otherwise just for further inserting.

	void InsertText(const char *text, int32 len = -1, bool after_last = false, bool clear_undo_redo = false);

	PBlock *FindBlock(int32 y);

	void ScrollIfNeeded(bool x = true, bool y = true);
	void SetScrollPos(int32 x, int32 y);
	void SetLayoutSize(int32 width, int32 height);
	void Reformat();

	void UpdateContentWidth();
	int32 GetContentWidth();
	int32 GetContentHeight();

	int32 GetOverflowX() { return MAX(content_width - layout_width, 0); }
	int32 GetOverflowY() { return MAX(content_height - layout_height, 0); }
public:
	PStyleEditListener *listener;
	int32 layout_width;
	int32 layout_height;
	int32 content_width;
	int32 content_height;

	TBLinkListOf<PBlock> blocks;
	TBListOf<PStyle> styles;

	PCaret caret;
	PSelection selection;
	PUndoRedoStack undoredo;

	int32 scroll_x;
	int32 scroll_y;

	int8 select_state;
	TBPoint mousedown_point;
	PElement *mousedown_element;

	TB_TEXT_ALIGN align;
	union { struct {
		uint32 multiline_on:1;
		uint32 read_only:1;
		uint32 show_whitespace:1;
		uint32 password_on:1;
		uint32 wrapping:1;
		uint32 enabled:1;
	} packed;
	uint32 packed_init;
	};
};

}; // namespace tinkerbell

#endif
