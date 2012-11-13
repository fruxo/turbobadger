#include "tbanimation/tb_animation.h"
#include "Application.h"

using namespace tinkerbell;

void Application::Run()
{
	m_backend->Run();
}

bool Application::Init()
{
	WidgetsAnimationManager::Init();
	return true;
}

void Application::ShutDown()
{
	WidgetsAnimationManager::Shutdown();
	delete m_backend;
	m_backend = nullptr;
}

void Application::Process()
{
	WidgetsAnimationManager::Update();
	GetRoot()->InvokeProcessStates();
	GetRoot()->InvokeProcess();
}

void Application::RenderFrame(int window_w, int window_h)
{
	g_renderer->BeginPaint(window_w, window_h);
	GetRoot()->InvokePaint(TBWidget::PaintProps());
	g_renderer->EndPaint();

	// If animations are running, reinvalidate immediately
	if (WidgetsAnimationManager::HasAnimationsRunning())
		GetRoot()->Invalidate();
}
