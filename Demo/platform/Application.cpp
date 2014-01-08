#include "animation/tb_widget_animation.h"
#include "Application.h"

using namespace tinkerbell;

void Application::Run()
{
	m_backend->Run();
}

bool Application::Init()
{
	TBWidgetsAnimationManager::Init();
	return true;
}

void Application::ShutDown()
{
	TBWidgetsAnimationManager::Shutdown();
	delete m_backend;
	m_backend = nullptr;
}

void Application::Process()
{
	TBAnimationManager::Update();
	GetRoot()->InvokeProcessStates();
	GetRoot()->InvokeProcess();
}

void Application::RenderFrame(int window_w, int window_h)
{
	g_renderer->BeginPaint(window_w, window_h);
	GetRoot()->InvokePaint(TBWidget::PaintProps());
	g_renderer->EndPaint();

	// If animations are running, reinvalidate immediately
	if (TBAnimationManager::HasAnimationsRunning())
		GetRoot()->Invalidate();
}
