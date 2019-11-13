// -*-  Mode: C++; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*-
#include "Application.h"

#ifdef TB_BACKEND_GLFW
#include "glfw_extra.h"
#include "tb_widgets.h"
#include "renderers/tb_renderer_gl.h"

using namespace tb;

class AppBackendGLFW : public AppBackend
{
public:
	bool Init(App *app);
	AppBackendGLFW()	: m_app(nullptr)
						, m_renderer(nullptr)
						, mainWindow(0)
						, m_cursor_i_beam(nullptr)
						, m_has_pending_update(false)
						, m_quit_requested(false) {}
	~AppBackendGLFW();

	virtual void EventLoop();
	virtual void OnAppEvent(const EVENT &ev);

	TBWidget *GetRoot() const { return m_app->GetRoot(); }
	int GetWidth() const { return m_app->GetWidth(); }
	int GetHeight() const { return m_app->GetHeight(); }

	App *m_app;
	TBRendererGL *m_renderer;
	GLFWwindow *mainWindow;
	GLFWcursor *m_cursor_i_beam;
	bool m_has_pending_update;
	bool m_quit_requested;
};

#endif // TB_BACKEND_GLFW
