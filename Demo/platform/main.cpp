#include "Application.h"

#ifdef TB_SYSTEM_MACOSX
#include <unistd.h>
#include <mach-o/dyld.h>

// newness
#include "TargetConditionals.h"
#include "CoreFoundation/CoreFoundation.h"
#include <unistd.h>
#include <libgen.h>
#endif
#ifdef TB_TARGET_LINUX
#include <unistd.h>
#include <sys/auxv.h>
#endif
#ifdef TB_TARGET_WINDOWS
#include <tchar.h>
#include <Windows.h>
/* This is intended as a drop-in replacement for unistd.h on Windows.
 * Please add functionality as neeeded.
 * https://stackoverflow.com/a/826027/1202830
 */
#include <stdlib.h>
#include <io.h>
//#include <getopt.h> /* getopt at: https://gist.github.com/ashelly/7776712 */
#include <process.h> /* for getpid() and the exec..() family */
#include <direct.h> /* for _getcwd() and _chdir() */
#define chdir _chdir
/* -- cut -- */
#endif

#include "port_glfw.hpp"
#include "port_sdl2.hpp"
#include "tb_tempbuffer.h"

using namespace tb;

bool port_main(int argc, char* argv[])
{
#if defined(TB_BACKEND_SDL2) && !defined(__EMSCRIPTEN__)
	if (char *base_path = SDL_GetBasePath())
	{
#if !TARGET_OS_IPHONE
		chdir(base_path);
#endif
		SDL_free(base_path);
		printf("SDL pwd: %s\n", base_path);
	}
#elif defined(TB_TARGET_MACOSX)
	{
		CFBundleRef bundle = CFBundleGetMainBundle();
        CFURLRef bundleURL = CFBundleCopyBundleURL(bundle);
        char path[PATH_MAX];
        Boolean success = CFURLGetFileSystemRepresentation(bundleURL, TRUE, (UInt8 *)path, PATH_MAX);
        assert(success);
        CFRelease(bundleURL);
        chdir(dirname(path));
	}
	// Change working directory to the executable path. We expect it
	// to be where the demo resources are.
	char exec_path[2048];
	uint32_t exec_path_size = sizeof(exec_path);
	if (_NSGetExecutablePath(exec_path, &exec_path_size) == 0)
	{
		for (int n = strlen(exec_path); n > 0 && exec_path[n-1] != '/'; n--)
			exec_path[n-1] = '\0';
		chdir(exec_path);
		chdir("../Resources");
		printf("nsexe pwd: %s\n", exec_path);
	}
#elif defined(TB_TARGET_LINUX)
	if (getauxval(AT_EXECFN))
	{
		char exec_path[2048];
		exec_path[0] = '\0';
		strncpy(exec_path, (char *)getauxval(AT_EXECFN), sizeof(exec_path));
		for (int n = strlen(exec_path); n > 0 && exec_path[n-1] != '/'; n--)
			exec_path[n-1] = '\0';
		if (chdir(exec_path))
		{
			printf("Unable to find resource directory '%s'\n", exec_path);
			//exit(0);
		}
		printf("pwd: %s\n", exec_path);
	}
#endif
	printf("GetRoot: %s\n", TBSystem::GetRoot());
#ifdef TB_TARGET_WINDOWS
	// Set the current path to the directory of the app so we find
	// assets also when visual studio starts it.
	char modname[MAX_PATH];
	GetModuleFileName(NULL, modname, MAX_PATH);
	TBTempBuffer buf;
	buf.AppendPath(modname);
	SetCurrentDirectory(buf.GetData());
#endif

	App *app = app_create();

#ifdef TB_BACKEND_GLFW
	AppBackendGLFW *backend = new AppBackendGLFW();
#endif
#ifdef TB_BACKEND_SDL2
	AppBackendSDL2 *backend = new AppBackendSDL2();
#endif

	if (!backend || !backend->Init(app))
		return false;

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
