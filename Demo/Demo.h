#ifndef DEMO_H
#define DEMO_H

#include "tb_widgets.h"
#include "tb_widgets_common.h"
#include "tb_widgets_reader.h"
#include "tb_widgets_listener.h"
#include "tb_message_window.h"
#include "tb_msg.h"

using namespace tinkerbell;

class Application
{
public:
	Application(Widget *root) : m_root(root) {}
	virtual ~Application() {}

	virtual bool Init() = 0;
	virtual void Process() = 0;
	virtual void RenderFrame(int window_w, int window_h) = 0;
protected:
	Widget *m_root;
};

class DemoApplication : public Application, public TBMessageHandler
{
public:
	DemoApplication(Widget *root) : Application(root) {}
	~DemoApplication();
	bool Init();
	void Process();
	void RenderFrame(int window_w, int window_h);

	virtual void OnMessageReceived(TBMessage *msg);
};

class UIEditorApplication : public Application, public TBMessageHandler, public TBGlobalWidgetListener
{
public:
	UIEditorApplication(Widget *root) : Application(root) {}
	~UIEditorApplication();

	// == Application ===================================================================
	bool Init();
	void Process();
	void RenderFrame(int window_w, int window_h);

	// == TBMessageHandler ==============================================================
	virtual void OnMessageReceived(TBMessage *msg);

	// == TBGlobalWidgetListener ========================================================
	virtual bool OnWidgetInvokeEvent(const WidgetEvent &ev);
	virtual void OnWidgetAdded(Widget *widget);
	virtual void OnWidgetRemove(Widget *widget);
};

#endif // DEMO_H
