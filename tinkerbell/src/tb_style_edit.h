// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TBStyleEdit_H
#define TBStyleEdit_H

#include "tinkerbell.h"
#include "tb_linklist.h"
#include "tb_widgets_common.h"
#include "tb_list.h"

namespace tinkerbell {

class TBStyleEdit;
class TBBlock;
class TBTextFragment;
class TBTextFragmentContent;
class TBTextFragmentContentFactory;

/** Listener for TBStyleEdit. Implement in the enviorment the TBStyleEdit should render its content. */

class TBStyleEditListener
{
public:
	virtual ~TBStyleEditListener() {}

	virtual void OnChange() {};
	virtual bool OnEnter() { return false; };
	virtual void Invalidate(const TBRect &rect) = 0;
	virtual void DrawString(int32 x, int32 y, const TBColor &color, const char *str, int32 len) = 0;
	virtual void DrawBackground(const TBRect &rect, TBBlock *block) = 0;
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

/** Creates TBTextFragmentContent if the sequence of text matches known content. */

class TBTextFragmentContentFactory
{
public:
	virtual ~TBTextFragmentContentFactory() {}
	/** Should return then length of the text that represents content
		that can be created by this factory, or 0 there's no match with any content.

		F.ex if we can create contet for "<u>" it should return 3 if that is the beginning of
		text. That length will be consumed from the text output for the created content. */
	virtual int GetContent(const char *text);

	/** Create content for a string previosly consumed by calling GetContent. */
	virtual TBTextFragmentContent *CreateFragmentContent(const char *text, int text_len);
};

class TBTextOfs
{
public:
	TBTextOfs() : block(nullptr), ofs(0) {}
	TBTextOfs(TBBlock *block, int32 ofs) : block(block), ofs(ofs) {}

	void Set(TBBlock *new_block, int32 new_ofs) { block = new_block; ofs = new_ofs; }
	void Set(const TBTextOfs &pos) { block = pos.block; ofs = pos.ofs; }

	int32 GetGlobalOfs(TBStyleEdit *se);
	bool SetGlobalOfs(TBStyleEdit *se, int32 gofs);

public:
	TBBlock *block;
	int32 ofs;
};

/** Handles the selected text in a TBStyleEdit. */

class TBSelection
{
public:
	TBSelection(TBStyleEdit *styledit);
	void Invalidate();
	void Select(const TBTextOfs &new_start, const TBTextOfs &new_stop);
	void Select(const TBPoint &from, const TBPoint &to);
	void Select(int glob_ofs_from, int glob_ofs_to);
	void SelectToCaret(TBBlock *old_caret_block, int32 old_caret_ofs);
	void SelectAll();
	void SelectNothing();
	void CorrectOrder();
	void CopyToClipboard();
	bool IsFragmentSelected(TBTextFragment *elm);
	bool IsSelected();
	void RemoveContent();
	bool GetText(TBStr &text);
public:
	TBStyleEdit *styledit;
	TBTextOfs start, stop;
};

enum TB_CARET_POS {
	TB_CARET_POS_BEGINNING,
	TB_CARET_POS_END
};

/** The caret in a TBStyleEdit. */

class TBCaret
{
public:
	TBCaret(TBStyleEdit *styledit);
	void Invalidate();
	void UpdatePos();
	bool Move(bool forward, bool word);
	bool Place(const TBPoint &point);
	bool Place(TBBlock *block, int ofs, bool allow_snap = true, bool snap_forward = false);
	void Place(TB_CARET_POS place);
	void AvoidLineBreak();
	void Paint(int32 translate_x, int32 translate_y);
	void ResetBlink();
	void UpdateWantedX();

	int32 GetGlobalOfs() { return pos.GetGlobalOfs(styledit); }
	void SetGlobalOfs(int32 gofs, bool allow_snap = true, bool snap_forward = false);

	TBTextFragment *GetFragment();
private:
	void SwitchBlock(bool second);
public:
	int32 x, y; ///< Relative to the styledit
	int width;
	int height;
	bool on;
	int32 wanted_x;
	bool prefer_first;
	TBStyleEdit *styledit;
	TBTextOfs pos;
};

/** TBTextProps is a stack of properties used during layout & paint of TBStyleEdit. */

class TBTextProps
{
public:
	class Data : public TBLinkOf<Data>
	{
	public:
		TBFontDescription font;
		TBColor text_color;
		bool underline;
	};
	TBTextProps(const TBFontDescription &font, const TBColor &text_color);
	Data *Push();
	void Pop();
public:
	TBLinkListOf<Data> data_list;
	Data base_data;
	Data *data;
};

/** A block of text (a line, that might be wrapped) */

class TBBlock : public TBLinkOf<TBBlock>
{
public:
	TBBlock(TBStyleEdit *styledit);
	~TBBlock();

	void Clear();
	void Set(const char *newstr, int32 len);
	void SetAlign(TB_TEXT_ALIGN align);

	int32 InsertText(int32 ofs, const char *text, int32 len, bool allow_line_recurse);
	void RemoveContent(int32 ofs, int32 len);

	/** Check if this block contains extra line breaks and split into new blocks if it does. */
	void Split();

	/** Check if we've lost the ending break on this block and if so merge it with the next block. */
	void Merge();

	/** Layout the block. To be called when the text has changed or the layout width has changed.
		@param update_fragments Should be true if the text has been changed (will recreate elements).
		@param propagate_height If true, all following blocks will be moved if the height changed. */
	void Layout(bool update_fragments, bool propagate_height);

	/** Update the size of this block. If propagate_height is true, all following blocks will be
		moved if the height changed. */
	void SetSize(int32 new_w, int32 new_h, bool propagate_height);

	TBTextFragment *FindFragment(int32 ofs, bool prefer_first = false);
	TBTextFragment *FindFragment(int32 x, int32 y);

	int32 CalculateStringWidth(const char *str, int len);
	int32 CalculateTabWidth(int32 xpos);
	int32 CalculateLineHeight();
	int32 CalculateBaseline();

	void Invalidate();
	void Paint(int32 translate_x, int32 translate_y, TBTextProps *props);
public:
	TBStyleEdit *styledit;
	TBLinkListOf<TBTextFragment> fragments;

	int32 ypos;
	int16 height;
	int8 align;

	TBStr str;
	int32 str_len;

private:
	int GetStartIndentation(int first_line_len);
};

/** Event in the TBUndoRedoStack. Each insert or remove change is stored as a TBUndoEvent, but they may also be merged when appropriate. */

class TBUndoEvent
{
public:
	int32 gofs;
	TBStr text;
	bool insert;
};

/** Keeps track of all TBUndoEvents used for undo and redo functionality. */

class TBUndoRedoStack
{
public:
	TBUndoRedoStack() : applying(false) {}
	~TBUndoRedoStack();

	void Undo(TBStyleEdit *styledit);
	void Redo(TBStyleEdit *styledit);
	void Clear(bool clear_undo, bool clear_redo);

	TBUndoEvent *Commit(TBStyleEdit *styledit, int32 gofs, int32 len, const char *text, bool insert);
public:
	TBListOf<TBUndoEvent> undos;
	TBListOf<TBUndoEvent> redos;
	bool applying;
private:
	void Apply(TBStyleEdit *styledit, TBUndoEvent *e, bool reverse);
};

/** The textfragment baseclass for TBStyleEdit. */

class TBTextFragment : public TBLinkOf<TBTextFragment>
{
public:
	TBTextFragment(TBTextFragmentContent *content = NULL)
				: xpos(0)
				, ypos(0)
				, ofs(0)
				, len(0)
				, line_ypos(0)
				, line_height(0)
				, block(NULL)
				, content(content) {}
	~TBTextFragment();

	void Init(TBBlock *block, uint16 ofs, uint16 len);

	void UpdateContentPos();

	void Paint(int32 translate_x, int32 translate_y, TBTextProps *props);
	void Click(int button, uint32 modifierkeys);

	bool IsText()					{ return !IsEmbedded(); }
	bool IsEmbedded()				{ return content ? true : false; }
	bool IsBreak();
	bool IsSpace();
	bool IsTab();

	int32 GetCharX(int32 ofs);
	int32 GetCharOfs(int32 x);

	/** Get the stringwidth. Handles passwordmode, tab, linebreaks etc automatically. */
	int32 GetStringWidth(const char *str, int len = -1);

	bool GetAllowBreakBefore();
	bool GetAllowBreakAfter();

	const char *Str() const			{ return block->str.CStr() + ofs; }

	int32 GetWidth();
	int32 GetHeight();
	int32 GetBaseline();
public:
	int16 xpos, ypos;
	uint16 ofs, len;
	uint16 line_ypos;
	uint16 line_height;
	TBBlock *block;
	TBTextFragmentContent *content;
};

/** Edit and formats TBTextFragment's. It handles the text in a TBStyleEditView. */

class TBStyleEdit
{
public:
	TBStyleEdit();
	virtual ~TBStyleEdit();

	void SetListener(TBStyleEditListener *listener);
	void SetContentFactory(TBTextFragmentContentFactory *content_factory);

	void Paint(const TBRect &rect, const TBColor &text_color);
	bool KeyDown(char ascii, uint16 function, uint32 modifierkeys);
	void MouseDown(const TBPoint &point, int button, int clicks, uint32 modifierkeys);
	void MouseUp(const TBPoint &point, int button, uint32 modifierkeys);
	void MouseMove(const TBPoint &point);
	void Focus(bool focus);

	void Clear(bool init_new = true);
	bool Load(const char *filename);
	bool SetText(const char *text, bool place_caret_at_end = false);
	bool SetText(const char *text, int text_len, bool place_caret_at_end);
	bool GetText(TBStr &text);
	bool IsEmpty();

	void SetAlign(TB_TEXT_ALIGN align);
	void SetMultiline(bool multiline = true);
	void SetStyling(bool styling = true);
	void SetReadOnly(bool readonly = true);
	void SetPassword(bool password = true);
	void SetWrapping(bool wrapping = true);

	/** Set if line breaks should be inserted in windows style (\r\n)
		or unix style (\n). The default is windows style on the windows
		platform and disabled elsewhere.

		Note: This only affects InsertBreak (pressing enter). Content set from
		      SetText (and clipboard etc.) maintains the used line break. */
	void SetWindowsStyleBreak(bool win_style_br) { packed.win_style_br = win_style_br; }

	void Cut();
	void Copy();
	void Paste();
	void Delete();

	void Undo() { undoredo.Undo(this); }
	void Redo() { undoredo.Redo(this); }
	bool CanUndo() { return undoredo.undos.GetNumItems() ? true : false; }
	bool CanRedo() { return undoredo.redos.GetNumItems() ? true : false; }

	void InsertText(const char *text, int32 len = -1, bool after_last = false, bool clear_undo_redo = false);
	void AppendText(const char *text, int32 len = -1, bool clear_undo_redo = false) { InsertText(text, len, true, clear_undo_redo); }
	void InsertBreak();

	TBBlock *FindBlock(int32 y);

	void ScrollIfNeeded(bool x = true, bool y = true);
	void SetScrollPos(int32 x, int32 y);
	void SetLayoutSize(int32 width, int32 height);
	void Reformat(bool update_fragments);

	int32 GetContentWidth();
	int32 GetContentHeight();

	int32 GetOverflowX() { return MAX(content_width - layout_width, 0); }
	int32 GetOverflowY() { return MAX(content_height - layout_height, 0); }
public:
	TBStyleEditListener *listener;
	TBTextFragmentContentFactory default_content_factory;
	TBTextFragmentContentFactory *content_factory;
	int32 layout_width;
	int32 layout_height;
	int32 content_width;
	int32 content_height;

	TBLinkListOf<TBBlock> blocks;

	TBCaret caret;
	TBSelection selection;
	TBUndoRedoStack undoredo;

	int32 scroll_x;
	int32 scroll_y;

	int8 select_state;
	TBPoint mousedown_point;
	TBTextFragment *mousedown_fragment;

	TB_TEXT_ALIGN align;
	union { struct {
		uint32 multiline_on : 1;
		uint32 styling_on : 1;
		uint32 read_only : 1;
		uint32 show_whitespace : 1;
		uint32 password_on : 1;
		uint32 wrapping : 1;
		uint32 win_style_br : 1;
	} packed;
	uint32 packed_init;
	};
};

}; // namespace tinkerbell

#endif
