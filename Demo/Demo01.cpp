#include "Demo.h"
#include <stdio.h>
#include <stdarg.h>
#include "tb_system.h"
#include "tb_select.h"
#include "tb_editfield.h"
#include "tb_tab_container.h"
#include "tb_bitmap_fragment.h"
#include "tbanimation/tb_animation.h"

TBGenericStringItemSource position_toggle_source;
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

void DemoOutput(const char *format, ...)
{
	if (!format)
		return;
	static char buf[1024];
	va_list ap;
	va_start(ap, format);
	int len = vsnprintf(buf, 1024, format, ap);
	va_end(ap);

	// Append the text at the last line of the debug field and scroll.
	if (TBEditField *edit = TBSafeGetByIDInRoot(Application::GetApp()->GetRoot(), TBEditField, "debug_output"))
	{
		edit->GetStyleEdit()->AppendText(buf, len, true);
		edit->GetStyleEdit()->ScrollIfNeeded();
	}
}

class TestItemSource : public TBGenericStringItemSource
{
public:
	virtual Widget *CreateItemWidget(int index);
};

Widget *TestItemSource::CreateItemWidget(int index)
{
	if (TBLayout *layout = new TBLayout)
	{
		layout->SetSkinBg("TBSelectItem");
		layout->SetLayoutDistribution(LAYOUT_DISTRIBUTION_AVAILABLE);
		layout->SetPaintOverflowFadeout(false);

		if (TBSkinImage *image = new TBSkinImage)
		{
			image->SetSkinBg("Icon48");
			image->SetIgnoreInput(true);
			layout->AddChild(image);
		}

		if (TBTextField *textfield = new TBTextField)
		{
			textfield->SetText(GetItemString(index));
			textfield->SetTextAlign(TB_TEXT_ALIGN_LEFT);
			textfield->SetIgnoreInput(true);
			layout->AddChild(textfield);
		}

		if (TBCheckBox *checkbox = new TBCheckBox)
			layout->AddChild(checkbox);
		return layout;
	}
	return nullptr;
}
TestItemSource advanced_source;

class TestWindow : public TBWindow
{
public:
	TestWindow(const char *title, const char *resource_file)
	{
		SetText(title);
		g_widgets_reader->LoadFile(this, resource_file);
		ResizeToFitContent();
	}
};

class ListWindow : public TestWindow
{
public:
	ListWindow(TBSelectItemSource *source, SCROLL_MODE scrollmode = SCROLL_MODE_Y_AUTO)
		: TestWindow("List and filter", "Demo/ui_resources/test_select.tb.txt")
	{
		if (TBSelectList *select = TBSafeGetByID(TBSelectList, "list"))
		{
			select->SetSource(source);
			select->GetScrollContainer()->SetScrollMode(scrollmode);
		}
	}
	virtual bool OnEvent(const WidgetEvent &ev)
	{
		if (ev.type == EVENT_TYPE_CHANGED && ev.target->GetID() == TBIDC("filter"))
		{
			TBStr filter;
			ev.target->GetText(filter);
			if (TBSelectList *select = TBSafeGetByID(TBSelectList, "list"))
				select->SetFilter(filter);
			return true;
		}
		return TBWindow::OnEvent(ev);
	}
};

class EditWindow : public TestWindow
{
public:
	EditWindow() : TestWindow("Text editing", "Demo/ui_resources/test_textwindow.tb.txt")
	{
	}
	virtual void OnProcessStates()
	{
		if (TBEditField *edit = TBSafeGetByID(TBEditField, "editfield"))
		{
			if (Widget *undo = GetWidgetByID("undo"))
				undo->SetState(WIDGET_STATE_DISABLED, !edit->GetStyleEdit()->CanUndo());
			if (Widget *redo = GetWidgetByID("redo"))
				redo->SetState(WIDGET_STATE_DISABLED, !edit->GetStyleEdit()->CanRedo());
		}
	}
	virtual bool OnEvent(const WidgetEvent &ev)
	{
		if (ev.type == EVENT_TYPE_CLICK)
		{
			TBCheckBox *wrap = TBSafeGetByID(TBCheckBox, "wrap");
			TBEditField *edit = TBSafeGetByID(TBEditField, "editfield");
			if (!wrap || !edit)
				return false;

			edit->SetWrapping(wrap->GetValue() ? true : false);
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
		}
		return TBWindow::OnEvent(ev);
	}
};

class MyToolbarWindow : public TBWindow
{
public:
	MyToolbarWindow(int x, int y, const char *filename, WINDOW_SETTINGS settings = WINDOW_SETTINGS_DEFAULT);
	virtual bool OnEvent(const WidgetEvent &ev);
};

MyToolbarWindow::MyToolbarWindow(int x, int y, const char *filename, WINDOW_SETTINGS settings)
{
	SetSettings(settings);

	bool ret = g_widgets_reader->LoadFile(this, filename);
	
	PreferredSize ps = GetPreferredSize();
	SetRect(TBRect(x, y, ps.pref_w, ps.pref_h));

	if (TBSelectDropdown *select = TBSafeGetByID(TBSelectDropdown, "select position"))
		select->SetSource(&position_toggle_source);
}

bool MyToolbarWindow::OnEvent(const WidgetEvent &ev)
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
			for (Widget *child = tc->GetTabLayout()->GetFirstChild(); child; child = child->GetNext())
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
	return TBWindow::OnEvent(ev);
}

class MyWindow : public TBWindow, public TBMessageHandler
{
public:
	MyWindow();
	bool Load(const char *filename);
	virtual bool OnEvent(const WidgetEvent &ev);
	virtual void OnMessageReceived(TBMessage *msg);
};

MyWindow::MyWindow()
{
	SetRect(TBRect(50, 100, 200, 300));

	Load("Demo/ui_resources/test_ui.tb.txt");

	SetText("Demo");

	SetOpacity(0.97f);

	if (TBSelectDropdown *select = TBSafeGetByID(TBSelectDropdown, "name dropdown"))
		select->SetSource(&name_source);
}

bool MyWindow::Load(const char *filename)
{
	bool ret = g_widgets_reader->LoadFile(this, filename);

	ResizeToFitContent();
	return ret;
}

void MyWindow::OnMessageReceived(TBMessage *msg)
{
	if (msg->message == TBIDC("delayedmsg"))
	{
		TBStr text;
		text.SetFormatted("This message window was created when a delayed message fired!\n\nIt was received %d ms after its intended fire time.", (int)(TBSystem::GetTimeMS() - msg->GetFireTime()));
		TBMessageWindow *msg_win = new TBMessageWindow(this, TBIDC(""));
		msg_win->Show("Message window", text);
	}
}

bool MyWindow::OnEvent(const WidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CLICK)
	{
		if (ev.target->GetID() == TBIDC("new"))
		{
			m_parent->AddChild(new MyWindow());
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
		else if (ev.target->GetID() == TBIDC("add img"))
		{
			TBButton *button = TBSafeCast(TBButton, ev.target);
			TBSkinImage *skin_image = new TBSkinImage;
			skin_image->SetSkinBg("Icon16");
			button->GetContentRoot()->AddChild(skin_image, WIDGET_Z_BOTTOM);
			return true;
		}
		else if (ev.target->GetID() == TBIDC("new buttons"))
		{
			char str[100];
			for(int i = 0; i < 100; i++)
			{
				sprintf(str, "Remove %d", i);
				TBButton *button = new TBButton;
				button->GetID().Set("remove button");
				button->SetText(str);
				ev.target->m_parent->AddChild(button);
			}
			return true;
		}
		else if (ev.target->GetID() == TBIDC("remove button"))
		{
			ev.target->m_parent->RemoveChild(ev.target);
			delete ev.target;
			return true;
		}
		else if (ev.target->GetID() == TBIDC("TBWindow.close"))
		{
			TBMessageWindow *msg_win = new TBMessageWindow(this, TBIDC("confirm_close_dialog"));
			TBMessageWindowSettings settings(TB_MSG_YES_NO, TBIDC("Icon48"));
			settings.dimmer = true;
			msg_win->Show("Are you sure?", "Really close the window?", &settings);
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
			g_tb_skin->ReloadBitmaps();
			return true;
		}
		else if (ev.target->GetID() == TBIDC("showpopupmenu1"))
		{
			TBMenuWindow *menu = new TBMenuWindow(ev.target, TBIDC("popupmenu1"));
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
		else if (ev.target->GetID() == TBIDC("misc tests"))
		{
			m_parent->AddChild(new MyToolbarWindow(100, 100, "Demo/ui_resources/test_toolbar01.tb.txt"));
			m_parent->AddChild(new MyToolbarWindow(100, 170, "Demo/ui_resources/test_toolbar02.tb.txt"));
			m_parent->AddChild(new MyToolbarWindow(100, 650, "Demo/ui_resources/test_toolbar03.tb.txt"));
			m_parent->AddChild(new MyToolbarWindow(100, 240, "Demo/ui_resources/test_layout01.tb.txt"));
			return true;
		}
	}
	else if (ev.type == EVENT_TYPE_CHANGED)
	{
		// Output the new value and text
		TBStr text;
		if (ev.target->GetText(text) && text.Length() > 24)
			sprintf(text.CStr() + 20, "...");
		DemoOutput("Changed to: %.2f (\"%s\")\n", ev.target->GetValueDouble(), text);
	}
	return TBWindow::OnEvent(ev);
}

// ======================================================

int fps = 0;
uint32 frame_counter_total = 0;
uint32 frame_counter = 0;
double frame_counter_reset_time = 0;
TBTextField *fps_field = nullptr;
TBCheckBox *fps_checkbox = nullptr;

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
	position_toggle_source.AddItem(new TBGenericStringItem("LAYOUT_POSITION_CENTER"));
	position_toggle_source.AddItem(new TBGenericStringItem("LAYOUT_POSITION_LEFT_TOP"));
	position_toggle_source.AddItem(new TBGenericStringItem("LAYOUT_POSITION_RIGHT_BOTTOM"));
	position_toggle_source.AddItem(new TBGenericStringItem("LAYOUT_POSITION_GRAVITY"));
	int i = 0;
	while (boy_names[i])
		advanced_source.AddItem(new TBGenericStringItem(boy_names[i++], TBIDC("boy_item")));
	i = 0;
	while (girl_names[i])
		name_source.AddItem(new TBGenericStringItem(girl_names[i++], TBIDC("girl_item")));
	i = 0;
	while (boy_names[i])
		name_source.AddItem(new TBGenericStringItem(boy_names[i++], TBIDC("boy_item")));
	name_source.SetSort(TB_SORT_ASCENDING);

// FIX: separator, shortcuts, disabled, icon, submenus!
// FIX: Also positioning of multiple menus.. Upper-right combos from point.
// behöver detta i stringitem implementationen för att enkelt kunna göra menyer, även om det är ett extra onödigt steg
// SKAPAR TEXTFIELD om det bara är text. finns ikon skapar den layout med flera saker i. Är det check, skapas layout med check osv.

	popup_menu_source.AddItem(new TBGenericStringItem("Option 1", TBIDC("opt 1")));
	popup_menu_source.AddItem(new TBGenericStringItem("Option 2", TBIDC("opt 2")));
	popup_menu_source.AddItem(new TBGenericStringItem("-"));
	popup_menu_source.AddItem(new TBGenericStringItem("Same submenu", &popup_menu_source));
	popup_menu_source.AddItem(new TBGenericStringItem("Long submenu", &name_source));
	// Give the first item a skin image
	popup_menu_source.GetItem(0)->SetSkinImage(TBIDC("Icon16"));

	MyWindow *win = new MyWindow();
	m_root->AddChild(win);

	TBWindow *textwindow = new EditWindow;
	textwindow->SetPosition(TBPoint(600, 100));
	m_root->AddChild(textwindow);

	ListWindow *listwindow = new ListWindow(&name_source);
	listwindow->SetPosition(TBPoint(1050, 450));
	m_root->AddChild(listwindow);

	listwindow = new ListWindow(&advanced_source, SCROLL_MODE_X_AUTO_Y_AUTO);
	listwindow->SetRect(TBRect(900, 100, 300, 300));
	m_root->AddChild(listwindow);

	m_root->AddChild(new MyToolbarWindow(600, 500, "Demo/ui_resources/test_tabcontainer01.tb.txt"));

	// We could have put this UI inside a resource file too, and read it with TBWidgetsReader,
	// but something has to demo how to build ui programmatically :)
	fps_field = new TBTextField;
	fps_field->SetRect(TBRect(10, 10, 300, 30));
	m_root->AddChild(fps_field);

	TBClickLabel *click_label = new TBClickLabel;
	click_label->SetText("Continous repaint (FPS test)");
	click_label->SetRect(TBRect(10, 40, 300, 30));
	m_root->AddChild(click_label);

	fps_checkbox = new TBCheckBox;
	click_label->GetContentRoot()->AddChild(fps_checkbox, WIDGET_Z_BOTTOM);

	WidgetsAnimationManager::Init();
	return true;
}

DemoApplication::~DemoApplication()
{
	WidgetsAnimationManager::Shutdown();
}

void DemoApplication::Process()
{
	WidgetsAnimationManager::Update();
	m_root->InvokeProcess();
	m_root->InvokeProcessStates();

	// Update the FPS counter
	double time = TBSystem::GetTimeMS();
	if (time > frame_counter_reset_time + 1000)
	{
		fps = (int) ((frame_counter / (time - frame_counter_reset_time)) * 1000);
		frame_counter_reset_time = time;
		frame_counter = 0;
	}
	// Update FPS field
	TBStr str;
	if (fps_checkbox->GetState(WIDGET_STATE_SELECTED))
		str.SetFormatted("FPS: %d Frame %d", fps, frame_counter_total);
	else
		str.SetFormatted("Frame %d", frame_counter_total);
	fps_field->SetText(str);
}

void DemoApplication::RenderFrame(int window_w, int window_h)
{
	// Render
	g_renderer->BeginPaint(window_w, window_h);
	m_root->InvokePaint(Widget::PaintProps());
#ifdef _DEBUG
	//g_tb_skin->Debug(); // Enable to debug skin bitmap fragments
#endif
	g_renderer->EndPaint();

	frame_counter++;
	frame_counter_total++;

	// If we want continous updates, reinvalidate any widget immediately
	if (fps_checkbox->GetState(WIDGET_STATE_SELECTED) ||
		WidgetsAnimationManager::HasAnimationsRunning())
		m_root->Invalidate();
}

void DemoApplication::OnMessageReceived(TBMessage *msg)
{
}
