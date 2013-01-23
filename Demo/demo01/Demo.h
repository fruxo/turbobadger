#ifndef DEMO_H
#define DEMO_H

#include "tb_widgets.h"
#include "tb_widgets_common.h"
#include "tb_widgets_reader.h"
#include "tb_widgets_listener.h"
#include "tb_message_window.h"
#include "tb_msg.h"
#include "platform/Application.h"

using namespace tinkerbell;

class DemoApplication : public Application
{
public:
	DemoApplication() : Application() {}

	virtual bool Init();
	virtual void RenderFrame(int window_w, int window_h);
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

class ImageWindow : public DemoWindow
{
public:
	ImageWindow();
	virtual bool OnEvent(const TBWidgetEvent &ev);
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
