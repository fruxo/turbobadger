#include "Demo.h"
#include "ListWindow.h"
#include "ResourceEditWindow.h"
#include <stdio.h>
#include <stdarg.h>
#include "tests/tb_test.h"
#include "tb_system.h"
#include "tb_inline_select.h"
#include "tb_select.h"
#include "tb_editfield.h"
#include "tb_tab_container.h"
#include "tb_bitmap_fragment.h"
#include "tbanimation/tb_animation.h"
#include "parser/TBNodeTree.h"
#include "tb_tempbuffer.h"
#include "tb_font_renderer.h"
#include "addons/tbimage/tb_image_manager.h"
#include "utf8/utf8.h"

static Application *application;

AdvancedItemSource advanced_source;
TBGenericStringItemSource name_source;
TBGenericStringItemSource popup_menu_source;

#ifdef TB_SUPPORT_CONSTEXPR

void const_expr_test()
{
	// Some code here just to see if the compiler really did
	// implement constexpr (and not just ignored it)
	// Should obviosly only compile if it really works. If not,
	// disable TB_SUPPORT_CONSTEXPR in tb_hash.h for your compiler.
	TBID id("foo");
	switch(id)
	{
		case TBIDC("foo"):
			break;
		case TBIDC("baar"):
			break;
		default:
			break;
	};
}

#endif // TB_SUPPORT_CONSTEXPR

// == DemoWindow ==============================================================

DemoWindow::DemoWindow()
{
	application->GetRoot()->AddChild(this);
}

bool DemoWindow::LoadResourceFile(const char *filename)
{
	// We could do g_widgets_reader->LoadFile(this, filename) but we want
	// some extra data we store under "WindowInfo", so read into node tree.
	TBNode node;
	if (!node.ReadFile(filename))
		return false;
	LoadResource(node);
	return true;
}

void DemoWindow::LoadResourceData(const char *data)
{
	// We could do g_widgets_reader->LoadData(this, filename) but we want
	// some extra data we store under "WindowInfo", so read into node tree.
	TBNode node;
	node.ReadData(data);
	LoadResource(node);
}

void DemoWindow::LoadResource(TBNode &node)
{
	g_widgets_reader->LoadNodeTree(this, &node);

	// Get title from the WindowInfo section (or use "" if not specified)
	SetText(node.GetValueString("WindowInfo>title", ""));

	// Use specified size or adapt to the preferred content size.
	TBNode *tmp = node.GetNode("WindowInfo>size");
	if (tmp && tmp->GetValue().GetArrayLength() == 2)
		SetSize(tmp->GetValue().GetArray()->GetValue(0)->GetInt(),
				tmp->GetValue().GetArray()->GetValue(1)->GetInt());
	else
		ResizeToFitContent();

	// Use the specified position or center in parent.
	tmp = node.GetNode("WindowInfo>position");
	if (tmp && tmp->GetValue().GetArrayLength() == 2)
		SetPosition(TBPoint(tmp->GetValue().GetArray()->GetValue(0)->GetInt(),
							tmp->GetValue().GetArray()->GetValue(1)->GetInt()));
	else
		SetPosition(TBPoint((GetParent()->GetRect().w - GetRect().w) / 2,
							(GetParent()->GetRect().h - GetRect().h) / 2));

	// Ensure we have focus - now that we've filled the window with possible focusable
	// widgets. EnsureFocus was automatically called when the window was activated (by
	// adding the window to the root), but then we had nothing to focus.
	// Alternatively, we could add the window after setting it up properly.
	EnsureFocus();
}

void DemoWindow::Output(const char *format, ...)
{
	if (!format)
		return;
	static char buf[1024];
	va_list ap;
	va_start(ap, format);
	int len = vsnprintf(buf, 1024, format, ap);
	va_end(ap);

	// Append the text at the last line of the debug field and scroll.
	if (TBEditField *edit = TBSafeGetByIDInRoot(application->GetRoot(), TBEditField, "debug_output"))
	{
		edit->GetStyleEdit()->AppendText(buf, len, true);
		edit->GetStyleEdit()->ScrollIfNeeded();
	}
}

bool DemoWindow::OnEvent(const TBWidgetEvent &ev)
{
	// FIX: Let a special debug output window be a TBGlobalWidgetListener and listen to all events
	// Now we only know about events that are not handled.
	if (ev.type == EVENT_TYPE_CHANGED)
	{
		// Output the new value and text
		TBStr text;
		if (ev.target->GetText(text) && text.Length() > 24)
			sprintf(text.CStr() + 20, "...");
		Output("Changed to: %.2f (\"%s\")\n", ev.target->GetValueDouble(), text.CStr());
	}
	else if (ev.type == EVENT_TYPE_CLICK)
	{
		// Output the id if it's a target with a id.
		if (ev.target->GetID())
		{
			TBStr text;
			if (ev.target->GetText(text) && text.Length() > 24)
				sprintf(text.CStr() + 20, "...");
			Output("Click with id: %u\n", (uint32)ev.target->GetID());
		}
	}
	else if (ev.type == EVENT_TYPE_KEY_DOWN && ev.special_key == TB_KEY_ESC)
	{
		// We could call Die() to fade away and die, but click the close button instead.
		// That way the window has a chance of intercepting the close and f.ex ask if it really should be closed.
		TBWidgetEvent click_ev(EVENT_TYPE_CLICK);
		m_close_button.InvokeEvent(click_ev);
		return true;
	}
	return TBWindow::OnEvent(ev);
}

// == EditWindow ==============================================================

class EditWindow : public DemoWindow
{
public:
	EditWindow()
	{
		LoadResourceFile("Demo/demo01/ui_resources/test_textwindow.tb.txt");
	}
	virtual void OnProcessStates()
	{
		// Update the disabled state of undo/redo buttons, and caret info.

		if (TBEditField *edit = TBSafeGetByID(TBEditField, "editfield"))
		{
			if (TBWidget *undo = GetWidgetByID("undo"))
				undo->SetState(WIDGET_STATE_DISABLED, !edit->GetStyleEdit()->CanUndo());
			if (TBWidget *redo = GetWidgetByID("redo"))
				redo->SetState(WIDGET_STATE_DISABLED, !edit->GetStyleEdit()->CanRedo());
			if (TBTextField *info = TBSafeGetByID(TBTextField, "info"))
			{
				TBStr text;
				text.SetFormatted("Caret ofs: %d", edit->GetStyleEdit()->caret.GetGlobalOfs());
				info->SetText(text);
			}
		}
	}
	virtual bool OnEvent(const TBWidgetEvent &ev)
	{
		if (ev.type == EVENT_TYPE_CLICK)
		{
			TBEditField *edit = TBSafeGetByID(TBEditField, "editfield");
			if (!edit)
				return false;

			if (ev.target->GetID() == TBIDC("clear"))
			{
				edit->SetText("");
				return true;
			}
			else if (ev.target->GetID() == TBIDC("undo"))
			{
				edit->GetStyleEdit()->Undo();
				return true;
			}
			else if (ev.target->GetID() == TBIDC("redo"))
			{
				edit->GetStyleEdit()->Redo();
				return true;
			}
			else if (ev.target->GetID() == TBIDC("menu"))
			{
				static TBGenericStringItemSource source;
				if (!source.GetNumItems())
				{
					source.AddItem(new TBGenericStringItem("Default font", TBIDC("default font")));
					source.AddItem(new TBGenericStringItem("Default font (larger)", TBIDC("large font")));
					source.AddItem(new TBGenericStringItem("RGB font (Neon)", TBIDC("rgb font Neon")));
					source.AddItem(new TBGenericStringItem("RGB font (Orangutang)", TBIDC("rgb font Orangutang")));
					source.AddItem(new TBGenericStringItem("RGB font (Orange)", TBIDC("rgb font Orange")));
					source.AddItem(new TBGenericStringItem("-"));
					source.AddItem(new TBGenericStringItem("Glyph cache stresstest (CJK)", TBIDC("CJK")));
					source.AddItem(new TBGenericStringItem("-"));
					source.AddItem(new TBGenericStringItem("Toggle wrapping", TBIDC("toggle wrapping")));
					source.AddItem(new TBGenericStringItem("-"));
					source.AddItem(new TBGenericStringItem("Align left", TBIDC("align left")));
					source.AddItem(new TBGenericStringItem("Align center", TBIDC("align center")));
					source.AddItem(new TBGenericStringItem("Align right", TBIDC("align right")));
				}

				if (TBMenuWindow *menu = new TBMenuWindow(ev.target, TBIDC("popup_menu")))
					menu->Show(&source);
				return true;
			}
			else if (ev.target->GetID() == TBIDC("popup_menu"))
			{
				if (ev.ref_id == TBIDC("default font"))
					edit->SetFontDescription(TBFontDescription());
				else if (ev.ref_id == TBIDC("large font"))
				{
					TBFontDescription fd = g_font_manager->GetDefaultFontDescription();
					fd.SetSize(28);
					edit->SetFontDescription(fd);
				}
				else if (ev.ref_id == TBIDC("rgb font Neon"))
				{
					TBFontDescription fd = edit->GetCalculatedFontDescription();
					fd.SetID(TBIDC("Neon"));
					edit->SetFontDescription(fd);
				}
				else if (ev.ref_id == TBIDC("rgb font Orangutang"))
				{
					TBFontDescription fd = edit->GetCalculatedFontDescription();
					fd.SetID(TBIDC("Orangutang"));
					edit->SetFontDescription(fd);
				}
				else if (ev.ref_id == TBIDC("rgb font Orange"))
				{
					TBFontDescription fd = edit->GetCalculatedFontDescription();
					fd.SetID(TBIDC("Orange"));
					edit->SetFontDescription(fd);
				}
				else if (ev.ref_id == TBIDC("CJK"))
				{
					TBTempBuffer buf;
					for (int i = 0, cp = 0x4E00; cp <= 0x9FCC; cp++, i++)
					{
						char utf8[8];
						int len = utf8::encode(cp, utf8);
						buf.Append(utf8, len);
						if (i % 64 == 63)
							buf.Append("\n", 1);
					}
					edit->GetStyleEdit()->SetText(buf.GetData(), buf.GetAppendPos());
				}
				else if (ev.ref_id == TBIDC("toggle wrapping"))
					edit->SetWrapping(!edit->GetWrapping());
				else if (ev.ref_id == TBIDC("align left"))
					edit->SetTextAlign(TB_TEXT_ALIGN_LEFT);
				else if (ev.ref_id == TBIDC("align center"))
					edit->SetTextAlign(TB_TEXT_ALIGN_CENTER);
				else if (ev.ref_id == TBIDC("align right"))
					edit->SetTextAlign(TB_TEXT_ALIGN_RIGHT);
				return true;
			}
		}
		return DemoWindow::OnEvent(ev);
	}
};

// == MyToolbarWindow =========================================================

MyToolbarWindow::MyToolbarWindow(const char *filename)
{
	LoadResourceFile(filename);
}

bool MyToolbarWindow::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CHANGED && ev.target->GetID() == TBIDC("select position"))
	{
		LAYOUT_POSITION pos = LAYOUT_POSITION_CENTER;
		if (TBSelectDropdown *select = TBSafeGetByID(TBSelectDropdown, "select position"))
			pos = static_cast<LAYOUT_POSITION>(select->GetValue());
		for (int i = 0; i < 3; i++)
			if (TBLayout *layout = TBSafeGetByID(TBLayout, i + 1))
				layout->SetLayoutPosition(pos);
		return true;
	}
	else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("toggle axis"))
	{
		static AXIS axis = AXIS_Y;
		for (int i = 0; i < 3; i++)
			if (TBLayout *layout = TBSafeGetByID(TBLayout, i + 1))
				layout->SetAxis(axis);
		axis = axis == AXIS_X ? AXIS_Y : AXIS_X;
		if (TBLayout *layout = TBSafeGetByID(TBLayout, 10))
			layout->SetAxis(axis);
		ResizeToFitContent(RESIZE_FIT_CURRENT_OR_NEEDED);
		return true;
	}
	else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("set_align"))
	{
		if (TBTabContainer *tc = TBSafeGetByID(TBTabContainer, "tabcontainer"))
			tc->SetAlignment(static_cast<TB_ALIGN>(ev.target->m_data));
		ResizeToFitContent(RESIZE_FIT_CURRENT_OR_NEEDED);
	}
	else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("toggle_axis"))
	{
		static AXIS axis = AXIS_Y;
		axis = axis == AXIS_X ? AXIS_Y : AXIS_X;
		if (TBTabContainer *tc = TBSafeGetByID(TBTabContainer, "tabcontainer"))
			tc->SetAxis(axis);
		ResizeToFitContent(RESIZE_FIT_CURRENT_OR_NEEDED);
	}
	else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("toggle_tab_axis"))
	{
		static AXIS axis = AXIS_X;
		axis = axis == AXIS_X ? AXIS_Y : AXIS_X;
		if (TBTabContainer *tc = TBSafeGetByID(TBTabContainer, "tabcontainer"))
		{
			for (TBWidget *child = tc->GetTabLayout()->GetFirstChild(); child; child = child->GetNext())
			{
				if (TBButton *button = TBSafeCast(TBButton, child))
					button->SetAxis(axis);
			}
		}
		ResizeToFitContent(RESIZE_FIT_CURRENT_OR_NEEDED);
	}
	else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("start_spinner"))
	{
		if (TBProgressSpinner *spinner = TBSafeGetByID(TBProgressSpinner, TBIDC("spinner")))
			spinner->SetValue(1);
	}
	else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("stop_spinner"))
	{
		if (TBProgressSpinner *spinner = TBSafeGetByID(TBProgressSpinner, TBIDC("spinner")))
			spinner->SetValue(0);
	}
	else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("reset-master-volume"))
	{
		if (TBWidgetValue *val = g_value_group.GetValue(TBIDC("master-volume")))
			val->SetInt(50);
	}
	else if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("reset-user-name"))
	{
		if (TBWidgetValue *val = g_value_group.GetValue(TBIDC("user-name")))
			val->SetText("");
	}
	return DemoWindow::OnEvent(ev);
}

// == ScrollContainerWindow ===================================================

ScrollContainerWindow::ScrollContainerWindow()
{
	LoadResourceFile("Demo/demo01/ui_resources/test_scrollcontainer.tb.txt");

	if (TBSelectDropdown *select = TBSafeGetByID(TBSelectDropdown, "name dropdown"))
		select->SetSource(&name_source);
}

bool ScrollContainerWindow::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CLICK)
	{
		if (ev.target->GetID() == TBIDC("add img"))
		{
			TBButton *button = TBSafeCast(TBButton, ev.target);
			TBSkinImage *skin_image = new TBSkinImage;
			skin_image->SetSkinBg("Icon16");
			button->GetContentRoot()->AddChild(skin_image, WIDGET_Z_BOTTOM);
			return true;
		}
		else if (ev.target->GetID() == TBIDC("new buttons"))
		{
			for(uint32 i = 0; i < ev.target->m_data; i++)
			{
				TBStr str;
				str.SetFormatted("Remove %d", i);
				TBButton *button = new TBButton;
				button->GetID().Set("remove button");
				button->SetText(str);
				ev.target->GetParent()->AddChild(button);
			}
			return true;
		}
		else if (ev.target->GetID() == TBIDC("new buttons delayed"))
		{
			for(uint32 i = 0; i < ev.target->m_data; i++)
			{
				TBMessageData *data = new TBMessageData();
				data->id1 = ev.target->GetParent()->GetID();
				data->v1.SetInt(i);
				PostMessageDelayed(TBIDC("new button"), data, 100 + i * 500);
			}
			return true;
		}
		else if (ev.target->GetID() == TBIDC("remove button"))
		{
			ev.target->GetParent()->RemoveChild(ev.target);
			delete ev.target;
			return true;
		}
		else if (ev.target->GetID() == TBIDC("showpopupmenu1"))
		{
			if (TBMenuWindow *menu = new TBMenuWindow(ev.target, TBIDC("popupmenu1")))
				menu->Show(&popup_menu_source);
			return true;
		}
		else if (ev.target->GetID() == TBIDC("popupmenu1"))
		{
			TBStr str;
			str.SetFormatted("Menu event received!\nref_id: %d", (int)ev.ref_id);
			TBMessageWindow *msg_win = new TBMessageWindow(this, TBIDC("popup_dialog"));
			msg_win->Show("Info", str);
			return true;
		}
	}
	return DemoWindow::OnEvent(ev);
}

void ScrollContainerWindow::OnMessageReceived(TBMessage *msg)
{
	if (msg->message == TBIDC("new button") && msg->data)
	{
		if (TBWidget *target = GetWidgetByID(msg->data->id1))
		{
			TBStr str;
			str.SetFormatted("Remove %d", msg->data->v1.GetInt());
			TBButton *button = new TBButton;
			button->GetID().Set("remove button");
			button->SetText(str);
			target->AddChild(button);
		}
	}
}

// == AnimationsWindow ========================================================

AnimationsWindow::AnimationsWindow()
{
	LoadResourceFile("Demo/demo01/ui_resources/test_animations.tb.txt");
	Animate();
}

void AnimationsWindow::Animate()
{
	// Abort any still unfinished animations.
	WidgetsAnimationManager::AbortAnimations(this);

	ANIMATION_CURVE curve = ANIMATION_CURVE_SLOW_DOWN;
	double duration = 500;
	bool fade = true;

	if (TBSelectList *curve_select = TBSafeGetByID(TBSelectList, "curve"))
		curve = static_cast<ANIMATION_CURVE>(curve_select->GetValue());
	if (TBInlineSelect *duration_select = TBSafeGetByID(TBInlineSelect, "duration"))
		duration = duration_select->GetValueDouble();
	if (TBCheckBox *fade_check = TBSafeGetByID(TBCheckBox, "fade"))
		fade = fade_check->GetValue() ? true : false;

	// Start move animation
	if (AnimationObject *anim = new WidgetAnimationRect(this, GetRect().Offset(-GetRect().x - GetRect().w, 0), GetRect()))
		AnimationManager::StartAnimation(anim, curve, duration);
	// Start fade animation
	if (fade)
	{
		if (AnimationObject *anim = new WidgetAnimationOpacity(this, ALMOST_ZERO_OPACITY, 1, false))
			AnimationManager::StartAnimation(anim, ANIMATION_CURVE_SLOW_DOWN, duration);
	}
}

bool AnimationsWindow::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CLICK && ev.target->GetID() == TBIDC("Animate!"))
		Animate();
	return DemoWindow::OnEvent(ev);
}

// == MainWindow ==============================================================

MainWindow::MainWindow()
{
	LoadResourceFile("Demo/demo01/ui_resources/test_ui.tb.txt");

	SetOpacity(0.97f);
}

void MainWindow::OnMessageReceived(TBMessage *msg)
{
	if (msg->message == TBIDC("delayedmsg"))
	{
		TBStr text;
		text.SetFormatted("This message window was created when a delayed message fired!\n\n"
							"It was received %d ms after its intended fire time.",
							(int)(TBSystem::GetTimeMS() - msg->GetFireTime()));
		TBMessageWindow *msg_win = new TBMessageWindow(this, TBIDC(""));
		msg_win->Show("Message window", text);
	}
}

bool MainWindow::OnEvent(const TBWidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CLICK)
	{
		if (ev.target->GetID() == TBIDC("new"))
		{
			new MainWindow();
			return true;
		}
		if (ev.target->GetID() == TBIDC("msg"))
		{
			TBMessageWindow *msg_win = new TBMessageWindow(this, TBIDC("test_dialog"));
			msg_win->Show("Message window", "Message!");
			return true;
		}
		else if (ev.target->GetID() == TBIDC("delayedmsg"))
		{
			PostMessageDelayed(TBIDC("delayedmsg"), nullptr, 2000);
			return true;
		}
		else if (ev.target->GetID() == TBIDC("TBWindow.close"))
		{
			// Intercept the TBWindow.close message and stop it from bubbling
			// to TBWindow (prevent the window from closing)
			TBMessageWindow *msg_win = new TBMessageWindow(this, TBIDC("confirm_close_dialog"));
			TBMessageWindowSettings settings(TB_MSG_YES_NO, TBIDC("Icon48"));
			settings.dimmer = true;
			settings.styling = true;
			msg_win->Show("Are you sure?", "Really <color #0794f8>close</color> the window?", &settings);
			return true;
		}
		else if (ev.target->GetID() == TBIDC("confirm_close_dialog"))
		{
			if (ev.ref_id == TBIDC("TBMessageWindow.yes"))
				Close();
			return true;
		}
		else if (ev.target->GetID() == TBIDC("reload skin bitmaps"))
		{
			int reload_count = 10;
			double t1 = TBSystem::GetTimeMS();
			for (int i = 0; i < reload_count; i++)
				g_tb_skin->ReloadBitmaps();
			double t2 = TBSystem::GetTimeMS();

			TBStr message;
			message.SetFormatted("Reloading the skin graphics %d times took %dms", reload_count, (int)(t2 - t1));
			TBMessageWindow *msg_win = new TBMessageWindow(ev.target, TBID());
			msg_win->Show("GFX load performance", message);
			return true;
		}
		else if (ev.target->GetID() == TBIDC("test context lost"))
		{
			g_renderer->InvokeContextLost();
			g_renderer->InvokeContextRestored();
			TBMessageWindow *msg_win = new TBMessageWindow(ev.target, TBID());
			msg_win->Show("Context lost & restore",
							"Called InvokeContextLost and InvokeContextRestored.\n\n"
							"Does everything look fine?");
			return true;
		}
		else if (ev.target->GetID() == TBIDC("test-layout"))
		{
			new MyToolbarWindow("Demo/demo01/ui_resources/test_layout01.tb.txt");
			new MyToolbarWindow("Demo/demo01/ui_resources/test_layout02.tb.txt");
			return true;
		}
		else if (ev.target->GetID() == TBIDC("test-connections"))
		{
			new MyToolbarWindow("Demo/demo01/ui_resources/test_connections.tb.txt");
			return true;
		}
		else if (ev.target->GetID() == TBIDC("test-radio-check"))
		{
			new MyToolbarWindow("Demo/demo01/ui_resources/test_radio_checkbox.tb.txt");
			return true;
		}
		else if (ev.target->GetID() == TBIDC("test-list"))
		{
			new AdvancedListWindow(&advanced_source);
			return true;
		}
		else if (ev.target->GetID() == TBIDC("test-image"))
		{
			new MyToolbarWindow("Demo/demo01/ui_resources/test_image_widget.tb.txt");
			return true;
		}
		else if (ev.target->GetID() == TBIDC("test-animations"))
		{
			new AnimationsWindow();
			return true;
		}
		else if (ev.target->GetID() == TBIDC("test-scroll-container"))
		{
			new ScrollContainerWindow();
			return true;
		}
		else if (ev.target->GetID() == TBIDC("test-skin-conditions"))
		{
			new MyToolbarWindow("Demo/demo01/ui_resources/test_skin_conditions01.tb.txt");
			new MyToolbarWindow("Demo/demo01/ui_resources/test_skin_conditions02.tb.txt");
			return true;
		}
		else if (ev.target->GetID() == TBIDC("test-toggle-container"))
		{
			new MyToolbarWindow("Demo/demo01/ui_resources/test_toggle_containers.tb.txt");
			return true;
		}
		else if (ev.target->GetID() == TBIDC("test-resource-edit"))
		{
			ResourceEditWindow *res_edit_win = new ResourceEditWindow();
			res_edit_win->Load("Demo/demo01/ui_resources/resource_edit_test.tb.txt");
			GetParent()->AddChild(res_edit_win);
			return true;
		}
	}
	return DemoWindow::OnEvent(ev);
}

// ======================================================

int fps = 0;
uint32 frame_counter_total = 0;
uint32 frame_counter = 0;
double frame_counter_reset_time = 0;

const char *girl_names[] = {
	"Maja", "Alice", "Julia", "Linnéa", "Wilma", "Ella", "Elsa", "Emma", "Alva", "Olivia", "Molly", "Ebba", "Klara", "Nellie", "Agnes",
	"Isabelle", "Ida", "Elin", "Ellen", "Moa", "Emilia", "Nova", "Alma", "Saga", "Amanda", "Isabella", "Lilly", "Alicia", "Astrid",
	"Matilda", "Tuva", "Tilde", "Stella", "Felicia", "Elvira", "Tyra", "Hanna", "Sara", "Vera", "Thea", "Freja", "Lova", "Selma",
	"Meja", "Signe", "Ester", "Lovisa", "Ellie", "Lea", "Tilda", "Tindra", "Sofia", "Nora", "Nathalie", "Leia", "Filippa", "Siri",
	"Emelie", "Inez", "Edith", "Stina", "Liv", "Lisa", "Linn", "Tove", "Emmy", "Livia", "Jasmine", "Evelina", "Cornelia", "Märta",
	"Svea", "Ingrid", "My", "Rebecca", "Joline", "Mira", "Ronja", "Hilda", "Melissa", "Anna", "Frida", "Maria", "Iris", "Josefine",
	"Elise", "Elina", "Greta", "Vilda", "Minna", "Lina", "Hedda", "Nicole", "Kajsa", "Majken", "Sofie", "Annie", "Juni", "Novalie", "Hedvig", 0 };
const char *boy_names[] = {
	"Oscar", "William", "Lucas", "Elias", "Alexander", "Hugo", "Oliver", "Theo", "Liam", "Leo", "Viktor", "Erik", "Emil",
	"Isak", "Axel", "Filip", "Anton", "Gustav", "Edvin", "Vincent", "Arvid", "Albin", "Ludvig", "Melvin", "Noah", "Charlie", "Max",
	"Elliot", "Viggo", "Alvin", "Alfred", "Theodor", "Adam", "Olle", "Wilmer", "Benjamin", "Simon", "Nils", "Noel", "Jacob", "Leon",
	"Rasmus", "Kevin", "Linus", "Casper", "Gabriel", "Jonathan", "Milo", "Melker", "Felix", "Love", "Ville", "Sebastian", "Sixten",
	"Carl", "Malte", "Neo", "David", "Joel", "Adrian", "Valter", "Josef", "Jack", "Hampus", "Samuel", "Mohammed", "Alex", "Tim",
	"Daniel", "Vilgot", "Wilhelm", "Harry", "Milton", "Maximilian", "Robin", "Sigge", "Måns", "Eddie", "Elton", "Vidar", "Hjalmar",
	"Loke", "Elis", "August", "John", "Hannes", "Sam", "Frank", "Svante", "Marcus", "Mio", "Otto", "Ali", "Johannes", "Fabian",
	"Ebbe", "Aron", "Julian", "Elvin", "Ivar", 0 };

bool DemoApplication::Init()
{
	if (!Application::Init())
		return false;

	// Block new animations during Init.
	AnimationBlocker anim_blocker;

	// Run unit tests
	int num_failed_tests = TBRunTests();

	// TBSelectList and TBSelectDropdown widgets have a default item source that are fed with any items
	// specified in the resource files. But it is also possible to set any source which can save memory
	// and improve performance. Then you don't have to populate each instance with its own set of items,
	// for widgets that occur many times in a UI, always with the same items.
	// Here we prepare the name source, that is used in a few places.
	int i = 0;
	while (boy_names[i])
		advanced_source.AddItem(new AdvancedItem(boy_names[i++], TBIDC("boy_item")));
	i = 0;
	while (girl_names[i])
		name_source.AddItem(new TBGenericStringItem(girl_names[i++], TBIDC("girl_item")));
	i = 0;
	while (boy_names[i])
		name_source.AddItem(new TBGenericStringItem(boy_names[i++], TBIDC("boy_item")));
	name_source.SetSort(TB_SORT_ASCENDING);

	// Prepare a source with submenus (with eternal recursion) so we can test sub menu support.
	popup_menu_source.AddItem(new TBGenericStringItem("Option 1", TBIDC("opt 1")));
	popup_menu_source.AddItem(new TBGenericStringItem("Option 2", TBIDC("opt 2")));
	popup_menu_source.AddItem(new TBGenericStringItem("-"));
	popup_menu_source.AddItem(new TBGenericStringItem("Same submenu", &popup_menu_source));
	popup_menu_source.AddItem(new TBGenericStringItem("Long submenu", &name_source));
	// Give the first item a skin image
	popup_menu_source.GetItem(0)->SetSkinImage(TBIDC("Icon16"));

	MainWindow *win = new MainWindow();

	TBWindow *textwindow = new EditWindow;

	ListWindow *listwindow = new ListWindow(&name_source);

	AdvancedListWindow *listwindow2 = new AdvancedListWindow(&advanced_source);

	new MyToolbarWindow("Demo/demo01/ui_resources/test_tabcontainer01.tb.txt");

	if (num_failed_tests)
	{
		TBStr text;
		text.SetFormatted("There is %d failed tests!\nCheck the output for details.", num_failed_tests);
		TBMessageWindow *msg_win = new TBMessageWindow(GetRoot(), TBIDC(""));
		msg_win->Show("Testing results", text);
	}
	return true;
}

void DemoApplication::RenderFrame(int window_w, int window_h)
{
	// Override RenderFrame without calling super, since we want
	// to inject code between BeginPaint/EndPaint.
	// Application::RenderFrame(window_w, window_h);

	// Render
	g_renderer->BeginPaint(window_w, window_h);
	GetRoot()->InvokePaint(TBWidget::PaintProps());

#ifdef _DEBUG
	// Enable to debug skin bitmap fragments
	//g_tb_skin->Debug();

	// Enable to debug font glyph fragments (the font of the hovered widget)
	//g_font_manager->GetFontFace(TBWidget::hovered_widget ?
	//							TBWidget::hovered_widget->GetCalculatedFontDescription() :
	//							g_font_manager->GetDefaultFontDescription())->Debug();

	// Enable to debug image manager fragments (if addons/tbimage/* is used)
	//g_image_manager->Debug();
#endif

	frame_counter++;
	frame_counter_total++;

	// Update the FPS counter
	double time = TBSystem::GetTimeMS();
	if (time > frame_counter_reset_time + 1000)
	{
		fps = (int) ((frame_counter / (time - frame_counter_reset_time)) * 1000);
		frame_counter_reset_time = time;
		frame_counter = 0;
	}

	// Draw FPS
	TBWidgetValue *continuous_repaint_val = g_value_group.GetValue(TBIDC("continous-repaint"));
	bool continuous_repaint = continuous_repaint_val ? !!continuous_repaint_val->GetInt() : 0;

	TBStr str;
	if (continuous_repaint)
		str.SetFormatted("FPS: %d Frame %d", fps, frame_counter_total);
	else
		str.SetFormatted("Frame %d", frame_counter_total);
	GetRoot()->GetFont()->DrawString(5, 5, TBColor(255, 255, 255), str);

	g_renderer->EndPaint();

	// If we want continous updates or got animations running, reinvalidate immediately
	if (continuous_repaint || WidgetsAnimationManager::HasAnimationsRunning())
		GetRoot()->Invalidate();
}

int app_main()
{
	application = new DemoApplication();

	ApplicationBackend *application_backend = ApplicationBackend::Create(application, 1280, 720, "Demo");
	if (!application_backend)
		return 1;

	init_tinkerbell(application_backend->GetRenderer(), "tinkerbell/lng_en.tb.txt");

	// Register tbbf font renderer
	void register_tbbf_font_renderer();
	register_tbbf_font_renderer();

	// Register freetype font renderer - if you compile with tb_font_renderer_freetype.cpp
	//void register_freetype_font_renderer();
	//register_freetype_font_renderer();

	// Add a font to the font manager.
	// If you use the freetype or stb backend, you can add true type files
	//g_font_manager->AddFontInfo("vera.ttf", "Vera");
	g_font_manager->AddFontInfo("tinkerbell/default_font/segoe_white_with_shadow.tb.txt", "Segoe");
	g_font_manager->AddFontInfo("tinkerbell/default_font/neon.tb.txt", "Neon");
	g_font_manager->AddFontInfo("tinkerbell/default_font/orangutang.tb.txt", "Orangutang");
	g_font_manager->AddFontInfo("tinkerbell/default_font/orange.tb.txt", "Orange");

	// Set the default font description for widgets to one of the fonts we just added
	TBFontDescription fd;
	fd.SetID(TBIDC("Segoe"));
	fd.SetSize(14);
	g_font_manager->SetDefaultFontDescription(fd);

	// Create the font now.
	TBFontFace *font = g_font_manager->CreateFontFace(g_font_manager->GetDefaultFontDescription());

	// Render some glyphs in one go now since we know we are going to use them. It would work fine
	// without this since glyphs are rendered when needed, but with some extra updating of the glyph bitmap.
	if (font)
		font->RenderGlyphs(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"
							"â‚¬â€šÆ’â€žâ€¦â€ â€¡Ë†â€°Å â€¹Å’Å½â€˜â€™â€œâ€â€¢â€“â€”Ëœâ„¢Å¡â€ºÅ“Å¾Å¸Â¡Â¢Â£Â¤Â¥Â¦Â§Â¨Â©ÂªÂ«Â¬Â®"
							"Â¯Â°Â±Â²Â³Â´ÂµÂ¶Â·Â¸Â¹ÂºÂ»Â¼Â½Â¾Â¿Ã€ÃÃ‚ÃƒÃ„Ã…Ã†Ã‡ÃˆÃ‰ÃŠÃ‹ÃŒÃÃŽÃÃÃ‘Ã’Ã“Ã”Ã•Ã–Ã—Ã˜Ã™ÃšÃ›ÃœÃÃžÃŸÃ Ã"
							"¡Ã¢Ã£Ã¤Ã¥Ã¦Ã§Ã¨Ã©ÃªÃ«Ã¬Ã­Ã®Ã¯Ã°Ã±Ã²Ã³Ã´ÃµÃ¶Ã·Ã¸Ã¹ÃºÃ»Ã¼Ã½Ã¾Ã¿");

	// Load the default skin, and override skin that contains the graphics specific to the demo.
	g_tb_skin->Load("tinkerbell/default_skin/skin.tb.txt", "Demo/demo01/skin/skin.tb.txt");

	// Give the root widget a background skin
	application->GetRoot()->SetSkinBg("background");

	application->Init();
	application->Run();
	application->ShutDown();

	delete application;

	return 0;
}
