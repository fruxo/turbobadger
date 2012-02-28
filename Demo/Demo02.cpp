#include "Demo.h"
#include <stdio.h>
#include "tb_system.h"
#include "tb_select.h"
#include "tb_editfield.h"
#include "tbanimation/tb_animation.h"

#define PROPERTY_WINDOW_WIDTH 300

class BuildWindow : public TBWindow
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("BuildWindow", TBWindow);

	bool Load(const char *resource_file);
private:
	TBStr m_resource_filename;
};

class PropertyWindow : public TBWindow, public TBMessageHandler
{
public:
	// For safe typecasting
	WIDGET_SUBCLASS("PropertyWindow", TBWindow);

	~PropertyWindow();
	void Init();
	void UpdateWidgetList(bool immediately);

	// == TBWindow ======================================================================
	virtual bool OnEvent(const WidgetEvent &ev);

	// == TBMessageHandler ==============================================================
	virtual void OnMessageReceived(TBMessage *msg);
private:
	TBSelectList *m_widget_list;
	TBGenericStringItemSource m_widget_list_source;
	void AddWidgetListItemsRecursive(Widget *widget, int depth);
};

BuildWindow *g_build_window;
PropertyWindow *g_property_window;

// == BuildWindow =================================================================================

bool BuildWindow::Load(const char *resource_file)
{
	m_resource_filename.Set(resource_file);
	SetText(resource_file);
	if (g_widgets_reader->LoadFile(this, resource_file))
	{
		// Success, so position the window
		ResizeToFitContent();
		SetRect(TBRect(PROPERTY_WINDOW_WIDTH + 50, 50, m_rect.w, m_rect.h));
		return true;
	}
	// Show error message
	TBStr text;
	text.SetFormatted("Could not load file %s", resource_file);
	if (TBMessageWindow *msg_win = new TBMessageWindow(GetParentRoot(), TBIDC("")))
		msg_win->Show("Error loading resource", text);
	return false;
}

// == PropertyWindow ==============================================================================

PropertyWindow::~PropertyWindow()
{
	// avoid assert
	m_widget_list->SetSource(nullptr);
}

void PropertyWindow::Init()
{
	SetSettings(WINDOW_SETTINGS_CAN_ACTIVATE);

	g_widgets_reader->LoadFile(this, "Demo/ui_resources/property_window.tb.txt");

	m_widget_list = TBSafeGetByID(TBSelectList, "widget_list");
	m_widget_list->SetSource(&m_widget_list_source);

	// Follow the left edge of the root
	Widget *root = GetParentRoot();
	SetGravity(WIDGET_GRAVITY_TOP_BOTTOM);
	SetRect(TBRect(0, 0, PROPERTY_WINDOW_WIDTH, root->m_rect.h));
}

void PropertyWindow::UpdateWidgetList(bool immediately)
{
	if (!immediately)
	{
		TBID id("update_widget_list");
		if (!GetMessageByID(id))
			PostMessage(id, nullptr);
	}
	else
	{
		m_widget_list_source.DeleteAllItems();
		AddWidgetListItemsRecursive(g_build_window, 0);

		m_widget_list->InvalidateList();
	}
}

void PropertyWindow::AddWidgetListItemsRecursive(Widget *widget, int depth)
{
	TBStr str;
	str.SetFormatted("% *s%s", depth, "", widget->GetClassName());
	m_widget_list_source.AddItem(new TBGenericStringItem(str));

	for (Widget *child = widget->GetFirstChild(); child; child = child->GetNext())
		AddWidgetListItemsRecursive(child,depth + 1);
}

bool PropertyWindow::OnEvent(const WidgetEvent &ev)
{
	if (ev.type == EVENT_TYPE_CHANGED && ev.target->GetID() == TBIDC("widget_list_search"))
	{
		TBStr filter;
		ev.target->GetText(filter);
		m_widget_list->SetFilter(filter);
		return true;
	}
	return TBWindow::OnEvent(ev);
}

void PropertyWindow::OnMessageReceived(TBMessage *msg)
{
	if (msg->message == TBIDC("update_widget_list"))
		UpdateWidgetList(true);
}

// == UIEditorApplication =========================================================================

bool UIEditorApplication::Init()
{
	// Register as global listener to intercept events in the build window
	TBGlobalWidgetListener::AddListener(this);

	// Create property window
	g_property_window = new PropertyWindow();
	m_root->AddChild(g_property_window);
	g_property_window->Init();

	// Create build window
	g_build_window = new BuildWindow();
	m_root->AddChild(g_build_window);
	g_build_window->Load("Demo/ui_resources/test_editor_my_window.tb.txt");
	return true;
}

UIEditorApplication::~UIEditorApplication()
{
	TBGlobalWidgetListener::RemoveListener(this);
}

void UIEditorApplication::Process()
{
	m_root->InvokeProcess();
	m_root->InvokeProcessStates();
}

void UIEditorApplication::RenderFrame(int window_w, int window_h)
{
	// Render
	g_renderer->BeginPaint(window_w, window_h);
	m_root->InvokePaint(Widget::PaintProps());
	g_renderer->EndPaint();
}

void UIEditorApplication::OnMessageReceived(TBMessage *msg)
{
}

bool UIEditorApplication::OnWidgetInvokeEvent(const WidgetEvent &ev)
{
	if (ev.IsPointerEvent())
	{
		// Intercept pointer events to the build window
		TBWindow *window = ev.target->GetParentWindow();
		if (window == g_build_window)
		{
			return true;
		}
	}
	return false;
}

void UIEditorApplication::OnWidgetAdded(Widget *widget)
{
	TBWindow *window = widget->GetParentWindow();
	if (window == g_build_window && g_property_window)
		g_property_window->UpdateWidgetList(false);
}

void UIEditorApplication::OnWidgetRemove(Widget *widget)
{
	TBWindow *window = widget->GetParentWindow();
	if (window == g_build_window && g_property_window)
		g_property_window->UpdateWidgetList(false);
}
