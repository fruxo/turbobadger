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
	Application(TBWidget *root) : m_root(root) {}
	virtual ~Application() {}

	static Application *GetApp();
	TBWidget *GetRoot() { return m_root; }

	virtual bool Init() = 0;
	virtual void Process() = 0;
	virtual void RenderFrame(int window_w, int window_h) = 0;
protected:
	TBWidget *m_root;
};

class DemoApplication : public Application, public TBMessageHandler
{
public:
	DemoApplication(TBWidget *root) : Application(root) {}
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
	void Output(const char *format, ...);

	virtual bool OnEvent(const TBWidgetEvent &ev);
};

class MainWindow : public DemoWindow, public TBMessageHandler
{
public:
	MainWindow();
	virtual bool OnEvent(const TBWidgetEvent &ev);

	// Implement TBMessageHandler
	virtual void OnMessageReceived(TBMessage *msg);
};

class AnimationsWindow : public DemoWindow
{
public:
	AnimationsWindow();
	void Animate();
	virtual bool OnEvent(const TBWidgetEvent &ev);
};

class MyToolbarWindow : public DemoWindow
{
public:
	MyToolbarWindow(const char *filename);
	virtual bool OnEvent(const TBWidgetEvent &ev);
};

class ScrollContainerWindow : public DemoWindow, public TBMessageHandler
{
public:
	ScrollContainerWindow();
	virtual bool OnEvent(const TBWidgetEvent &ev);

	// Implement TBMessageHandler
	virtual void OnMessageReceived(TBMessage *msg);
};

#endif // DEMO_H
