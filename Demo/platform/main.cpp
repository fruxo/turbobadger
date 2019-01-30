// -*-  Mode: C++; c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*-

#include "Application.h"
#include "port_glfw.hpp"
#include "port_sdl2.hpp"

bool port_main(int argc, char* argv[])
{
	App *app = app_create();

#ifdef TB_CLIPBOARD_GLFW
	AppBackendGLFW *backend = new AppBackendGLFW();
#endif
#ifdef TB_SYSTEM_SDL2
	AppBackendSDL2 *backend = new AppBackendSDL2();
#endif

	if (!backend || !backend->Init(app))
		return false;

#ifdef TB_TARGET_MACOSX
	// Change working directory to the executable path. We expect it
	// to be where the demo resources are.
	char exec_path[2048];
	uint32_t exec_path_size = sizeof(exec_path);
	if (_NSGetExecutablePath(exec_path, &exec_path_size) == 0)
	{
		TBTempBuffer path;
		path.AppendPath(exec_path);
		chdir(path.GetData());
	}
#endif

#ifdef TB_TARGET_WINDOWS
	// Set the current path to the directory of the app so we find
	// assets also when visual studio starts it.
	char modname[MAX_PATH];
	GetModuleFileName(NULL, modname, MAX_PATH);
	TBTempBuffer buf;
	buf.AppendPath(modname);
	SetCurrentDirectory(buf.GetData());
#endif

	bool success = app->Init();
	if (success)
	{
		// Main loop - run until backend gets EVENT_QUIT_REQUEST
		backend->EventLoop();
		app->ShutDown();
	}

	delete backend;
	delete app;

	return success;
}

#ifdef TB_TARGET_WINDOWS

#include <mmsystem.h>
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Crank up windows timer resolution (it's awfully low res normally). Note: This affects battery time!
    timeBeginPeriod(1);
	bool success = port_main(0, nullptr);
	timeEndPeriod(1);
	return success ? 0 : 1;
}

#else // TB_TARGET_WINDOWS

int main(int argc, char* argv[])
{
	return port_main(argc, argv) ? 0 : 1;
}

#endif // !TB_TARGET_WINDOWS
