#include "animation/tb_widget_animation.h"
#include "Application.h"

using namespace tb;

// == AppRootWidget ===============================================================================

void AppRootWidget::OnInvalid()
{
	if (m_app->GetBackend())
		m_app->GetBackend()->OnAppEvent(AppBackend::EVENT_PAINT_REQUEST);
}

bool AppRootWidget::OnEvent(const TBWidgetEvent & ev)
{
    switch (ev.type) {
	case EVENT_TYPE_KEY_UP:
		if ((ev.modifierkeys & (TB_CTRL | TB_SUPER))) {
			switch (ev.key) {
			case 'q':
			case 'Q':
				m_app->GetBackend()->OnAppEvent(AppBackend::EVENT_QUIT_REQUEST);
				return true;
			}
		}
		//TBDebugPrint("Unhandled key event: %c, modifiers:0x%x\n", ev.key, ev.modifierkeys);
	default:
		break;
	}

	return false;
}

// == App =========================================================================================

App::App(int width, int height)
	: m_backend(nullptr)
	, m_root(this)
{
	// Set initial size which suggest to the backend which size we want the window to be.
	m_root.SetRect(TBRect(0, 0, width, height));
}

void App::OnBackendAttached(AppBackend *backend, int width, int height)
{
	m_backend = backend;
	OnResized(width, height);
}

void App::OnBackendDetached()
{
	m_root.DeleteAllChildren();
	m_backend = nullptr;
}

void App::OnResized(int width, int height)
{
	m_root.SetRect(TBRect(0, 0, width, height));
}

bool App::Init()
{
	TBWidgetsAnimationManager::Init();
	return true;
}

void App::ShutDown()
{
	TBWidgetsAnimationManager::Shutdown();
}

void App::Process()
{
	TBAnimationManager::Update();
	m_root.InvokeProcessStates();
	m_root.InvokeProcess();
}

void App::RenderFrame()
{
	g_renderer->BeginPaint(m_root.GetRect().w, m_root.GetRect().h);
	m_root.InvokePaint(TBWidget::PaintProps());
	g_renderer->EndPaint();

	// If animations are running, reinvalidate immediately
	if (TBAnimationManager::HasAnimationsRunning())
		GetRoot()->Invalidate();
}
