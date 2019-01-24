// -*-  Mode: C++; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*-
#include "Application.h"

#ifdef TB_SYSTEM_SDL

#include "tb_widgets.h"
#include "renderers/tb_renderer_gl.h"

#ifdef __EMSCRIPTEN__
#include "SDL/SDL.h"
#else
#include "SDL2/SDL.h"
#endif

using namespace tb;

class AppBackendSDL2 : public AppBackend
{
public:
	bool Init(App *app);
	AppBackendSDL2()	: m_quit_requested(false)
						, m_app(nullptr)
						, m_renderer(nullptr)
						, mainWindow(0)
						, m_has_pending_update(false) {}
	~AppBackendSDL2();

	virtual void EventLoop();
	virtual void OnAppEvent(const EVENT &ev);

	TBWidget *GetRoot() const { return m_app->GetRoot(); }
	int GetWidth() const { return m_app->GetWidth(); }
	int GetHeight() const { return m_app->GetHeight(); }

	bool HandleSDLEvent(SDL_Event & event);
	bool m_quit_requested;
private:
	bool InvokeKey(unsigned int key, SPECIAL_KEY special_key,
				   MODIFIER_KEYS modifierkeys, bool down);
	void QueueUserEvent(Sint32 code, void * data1 = NULL, void * data2 = NULL);

	App *m_app;
	TBRendererGL *m_renderer;
	SDL_Window *mainWindow;
	SDL_GLContext glContext;
	bool m_has_pending_update;
};

#endif
