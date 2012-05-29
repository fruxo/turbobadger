// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_test.h"
#include "tb_editfield.h"

#ifdef TB_UNIT_TESTING

using namespace tinkerbell;

TB_TEST_GROUP(tb_editfield)
{
	TBEditField *edit;
	TBStyleEdit *sedit;

	// == Setup & helpers =====================================================

	TB_TEST(Setup)
	{
		TB_VERIFY(edit = new TBEditField());
		edit->SetMultiline(true);
		sedit = edit->GetStyleEdit();

		/** Set a size so the layout code will be called and we can do some layout tests. */
		edit->SetRect(TBRect(0, 0, 100, 100));

		/** Force windows style line breaks so testing is the same on all platforms. */
		sedit->SetWindowsStyleBreak(true);
	}
	TB_TEST(Cleanup) { delete edit; }

	// == Tests ===============================================================

	TB_TEST(settext_singleline)
	{
		edit->SetMultiline(false);
		edit->SetText("One\nTwo", true);
		TB_VERIFY_STR(edit->GetText(), "One");
	}

	TB_TEST(settext_multiline)
	{
		// Both unix and windows line endings should be ok.
		edit->SetText("One\nTwo", true);
		TB_VERIFY_STR(edit->GetText(), "One\nTwo");
		edit->SetText("One\r\nTwo", true);
		TB_VERIFY_STR(edit->GetText(), "One\r\nTwo");
	}

	TB_TEST(settext_undoredo_ins)
	{
		// 1 len insertions in sequence should be merged to word boundary.
		sedit->InsertText("O"); sedit->InsertText("N"); sedit->InsertText("E"); sedit->InsertText(" ");
		sedit->InsertText("T"); sedit->InsertText("W"); sedit->InsertText("O");
		TB_VERIFY_STR(edit->GetText(), "ONE TWO");

		sedit->Undo();
		TB_VERIFY_STR(edit->GetText(), "ONE ");
		sedit->Undo();
		TB_VERIFY_STR(edit->GetText(), "");

		sedit->Redo();
		TB_VERIFY_STR(edit->GetText(), "ONE ");
		sedit->Redo();
		TB_VERIFY_STR(edit->GetText(), "ONE TWO");
	}

	TB_TEST(settext_undoredo_ins_scattered)
	{
		sedit->InsertText("AAA");
		sedit->caret.SetGlobalOfs(2);
		sedit->InsertText(".");
		sedit->caret.SetGlobalOfs(1);
		sedit->InsertText(".");
		TB_VERIFY_STR(edit->GetText(), "A.A.A");

		sedit->Undo();
		TB_VERIFY_STR(edit->GetText(), "AA.A");
		sedit->Undo();
		TB_VERIFY_STR(edit->GetText(), "AAA");

		sedit->Redo();
		TB_VERIFY_STR(edit->GetText(), "AA.A");
		sedit->Redo();
		TB_VERIFY_STR(edit->GetText(), "A.A.A");
	}

	TB_TEST(settext_undoredo_ins_multiline)
	{
		sedit->InsertText("ONE\nTWO");
		TB_VERIFY_STR(edit->GetText(), "ONE\nTWO");

		sedit->Undo();
		TB_VERIFY_STR(edit->GetText(), "");
		sedit->Redo();
		TB_VERIFY_STR(edit->GetText(), "ONE\nTWO");
	}

	TB_TEST(settext_undoredo_del)
	{
		sedit->InsertText("ONE TWO");
		sedit->selection.Select(3, 7);
		sedit->selection.RemoveContent();
		TB_VERIFY_STR(edit->GetText(), "ONE");

		sedit->Undo();
		TB_VERIFY_STR(edit->GetText(), "ONE TWO");
		sedit->Redo();
		TB_VERIFY_STR(edit->GetText(), "ONE");
	}

	TB_TEST(settext_undoredo_ins_linebreak_1)
	{
		sedit->InsertText("ONETWO");
		sedit->caret.SetGlobalOfs(3);
		sedit->InsertText("\n");
		TB_VERIFY_STR(edit->GetText(), "ONE\nTWO");

		sedit->Undo();
		TB_VERIFY_STR(edit->GetText(), "ONETWO");
		sedit->Redo();
		TB_VERIFY_STR(edit->GetText(), "ONE\nTWO");
	}

	TB_TEST(settext_undoredo_ins_linebreak_2)
	{
		// Inserting a linebreak at the end when we don't end
		// the line with a linebreak character must generate
		// one extra linebreak.
		sedit->InsertBreak();
		TB_VERIFY_STR(edit->GetText(), "\r\n\r\n");

		sedit->InsertBreak();
		TB_VERIFY_STR(edit->GetText(), "\r\n\r\n\r\n");

		sedit->Undo();
		TB_VERIFY_STR(edit->GetText(), "\r\n\r\n");
		sedit->Undo();
		TB_VERIFY_STR(edit->GetText(), "");
	}

	TB_TEST(settext_undoredo_ins_linebreak_3)
	{
		sedit->SetWindowsStyleBreak(false);

		sedit->InsertBreak();
		TB_VERIFY_STR(edit->GetText(), "\n\n");

		sedit->InsertBreak();
		TB_VERIFY_STR(edit->GetText(), "\n\n\n");

		sedit->Undo();
		TB_VERIFY_STR(edit->GetText(), "\n\n");
		sedit->Undo();
		TB_VERIFY_STR(edit->GetText(), "");
	}

	TB_TEST(settext_undoredo_ins_linebreak_4)
	{
		sedit->InsertText("ONE");
		sedit->InsertBreak();
		TB_VERIFY_STR(edit->GetText(), "ONE\r\n\r\n");

		sedit->Undo();
		TB_VERIFY_STR(edit->GetText(), "ONE");

		sedit->Redo();
		TB_VERIFY_STR(edit->GetText(), "ONE\r\n\r\n");
	}

}

#endif // TB_UNIT_TESTING
