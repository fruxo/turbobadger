#ifndef APPLICATION_H
#define APPLICATION_H

#include "tb_widgets.h"

// --------------------------------------------------------------------------------------
// This file contains some platform glue that is optional. It may help you set up a TB UI
// for a game or application quicker or just function as a sample of how it can be done.
// --------------------------------------------------------------------------------------

class App;
class AppBackend;

/** The root of widgets in a platform backend. */
class AppRootWidget : public tb::TBWidget
{
public:
	// For safe typecasting
	TBOBJECT_SUBCLASS(AppRootWidget, tb::TBWidget);

	AppRootWidget(App *app) : m_app(app) {}
	virtual void OnInvalid();

	App *GetApp() { return m_app; }
private:
	App *m_app;
};

/** Backend interface that handles platform window & creating the renderer. */
class AppBackend
{
public:
	enum EVENT {
		EVENT_PAINT_REQUEST,
		EVENT_QUIT_REQUEST,
		EVENT_TITLE_CHANGED
	};
	virtual ~AppBackend() {}
	virtual void OnAppEvent(const EVENT &ev) = 0;
};

/** Application interface, for setting up the application using turbo badger. */
class App
{
public:
	App(int width, int height);
	virtual ~App() {}

	virtual const char *GetTitle() const { return ""; }
	int GetWidth() const { return m_root.GetRect().w; }
	int GetHeight() const { return m_root.GetRect().h; }

	tb::TBWidget *GetRoot() { return &m_root; }
	AppBackend *GetBackend() { return m_backend; }

	virtual void OnBackendAttached(AppBackend *backend, int width, int height);
	virtual void OnBackendDetached() { m_backend = nullptr; }
	virtual void OnResized(int width, int height);

	virtual bool Init();
	virtual void ShutDown();
	virtual void Process();
	virtual void RenderFrame();
protected:
	AppBackend *m_backend;
	AppRootWidget m_root;
};

/** Should return new instance of App. */
App *app_create();

#endif // APPLICATION_H
