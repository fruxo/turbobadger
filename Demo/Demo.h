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

	static Application *GetApp();
	Widget *GetRoot() { return m_root; }

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

class DemoWindow : public TBWindow
{
public:
	DemoWindow();
	bool LoadResourceFile(const char *filename);
	void LoadResourceData(const char *data);
	void LoadResource(TBNode &node);
};

class MainWindow : public DemoWindow, public TBMessageHandler
{
public:
	MainWindow();
	virtual bool OnEvent(const WidgetEvent &ev);

	// Implement TBMessageHandler
	virtual void OnMessageReceived(TBMessage *msg);
};

class MyToolbarWindow : public DemoWindow
{
public:
	MyToolbarWindow(const char *filename);
	virtual bool OnEvent(const WidgetEvent &ev);
};

class ScrollContainerWindow : public DemoWindow, public TBMessageHandler
{
public:
	ScrollContainerWindow();
	virtual bool OnEvent(const WidgetEvent &ev);

	// Implement TBMessageHandler
	virtual void OnMessageReceived(TBMessage *msg);
};

#endif // DEMO_H
