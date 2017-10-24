// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_system.h"
#include <iostream>

#ifdef TB_SYSTEM_SDL

#include "tb_msg.h"
#include "tb_types.h"
//#include <sys/time.h>
#include <stdio.h>
#ifndef _WIN32
#include <strings.h>
#endif

#if __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef __APPLE__
#include "SDL2/SDL.h"
#else
#include "SDL.h"
#endif

#if defined(TB_RUNTIME_DEBUG_INFO) || 1
#ifdef ANDROID
#include <android/log.h>
#define  LOG_TAG    "TB"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

void tb::TBDebugOut(const char *str)
{
	LOGI("%s", str);
}

#else // ANDROID

void tb::TBDebugOut(const char *str)
{
	std::cerr << str;
	SDL_Log("%s", str);
	SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s", str);
}
#endif // ANDROID
#endif // TB_RUNTIME_DEBUG_INFO


namespace tb {

// == TBSystem ========================================

#ifndef __EMSCRIPTEN__
double TBSystem::GetTimeMS()
{
#if 1
	Uint64 freq = SDL_GetPerformanceFrequency();
	Uint64 now = SDL_GetPerformanceCounter();
	return 1000. * ((double)now / (double)freq);
#elif 0
	return SDL_GetTicks();
#else
	struct timeval now;
	gettimeofday( &now, NULL );
	return now.tv_usec/1000 + now.tv_sec*1000;
#endif
}

static SDL_TimerID tb_sdl_timer_id = 0;
static Uint32 tb_sdl_timer_callback(Uint32 /*interval*/, void * /*param*/)
{
	double next_fire_time = TBMessageHandler::GetNextMessageFireTime();
	double now = TBSystem::GetTimeMS();
	if (next_fire_time != TB_NOT_SOON && (next_fire_time - now) > 1.0)
	{
		// We timed out *before* we were supposed to (the OS is not playing nice).
		// Calling ProcessMessages now won't achieve a thing so force a reschedule
		// of the platform timer again with the same time.
		return next_fire_time - now;
	}

	{
		// queue a user event to cause the event loop to run
		SDL_Event event;
		SDL_UserEvent userevent;
		userevent.type = SDL_USEREVENT;
		userevent.code = 3;
		userevent.data1 = NULL;
		userevent.data2 = NULL;
		event.type = SDL_USEREVENT;
		event.user = userevent;
		SDL_PushEvent(&event);
	}

	// next event will be scheduled from the event loop
	return 0;
}

/** Reschedule the platform timer, or cancel it if fire_time is TB_NOT_SOON.
	If fire_time is 0, it should be fired ASAP.
	If force is true, it will ask the platform to schedule it again, even if
	the fire_time is the same as last time. */
void TBSystem::RescheduleTimer(double fire_time)
{
	// cancel existing timer
	if (tb_sdl_timer_id)
	{
		SDL_RemoveTimer(tb_sdl_timer_id);
		tb_sdl_timer_id = 0;
	}
	// set new timer
	if (fire_time != TB_NOT_SOON)
	{
		double now = TBSystem::GetTimeMS();
		double delay = fire_time - now;
		tb_sdl_timer_id = SDL_AddTimer((Uint32)MAX(delay, 1.), tb_sdl_timer_callback, NULL);
		if (!tb_sdl_timer_id)
			TBDebugOut("ERROR: RescheduleTimer failed to SDL_AddTimer\n");
	}
}

#else // __EMSCRIPTEN__

double TBSystem::GetTimeMS()
{
	return emscripten_get_now();
}

static SDL_TimerID tb_sdl_timer_id = 0;
static void tb_sdl_timer_callback(void *param)
{

	double next_fire_time = TBMessageHandler::GetNextMessageFireTime();
	double now = TBSystem::GetTimeMS();
	if (next_fire_time != TB_NOT_SOON && (next_fire_time - now) > 1.0)
	{
		// We timed out *before* we were supposed to (the OS is not playing nice).
		// Calling ProcessMessages now won't achieve a thing so force a reschedule
		// of the platform timer again with the same time.
		emscripten_async_call(tb_sdl_timer_callback, param, int(next_fire_time - now));
		return;
	}

	TBMessageHandler::ProcessMessages();

	// If we still have things to do (because we didn't process all messages,
	// or because there are new messages), we need to rescedule, so call RescheduleTimer.
	next_fire_time = TBMessageHandler::GetNextMessageFireTime();
	if (next_fire_time == TB_NOT_SOON)
	{
		tb_sdl_timer_id = 0;
		return;
	}
	next_fire_time -= TBSystem::GetTimeMS();
	emscripten_async_call(tb_sdl_timer_callback, param, next_fire_time - now);
	return;
}

/** Reschedule the platform timer, or cancel it if fire_time is TB_NOT_SOON.
	If fire_time is 0, it should be fired ASAP.
	If force is true, it will ask the platform to schedule it again, even if
	the fire_time is the same as last time. */
void TBSystem::RescheduleTimer(double fire_time)
{
	// cancel existing timer
	if (tb_sdl_timer_id)
	{
		return;
	}
	// set new timer
	if (fire_time != TB_NOT_SOON && !tb_sdl_timer_id)
	{
		double now = TBSystem::GetTimeMS();
		double delay = fire_time - now;
		tb_sdl_timer_id = 1;
		emscripten_async_call(tb_sdl_timer_callback, NULL, (int)delay);
	}
}

#endif

int TBSystem::GetLongClickDelayMS()
{
	return 500;
}

int TBSystem::GetPanThreshold()
{
	return 5 * GetDPI() / 96;
}

int TBSystem::GetPixelsPerLine()
{
	return 40 * GetDPI() / 96;
}

int TBSystem::_dpi = 96;

int TBSystem::GetDPI()
{
	return _dpi;
}

void TBSystem::SetDPI(int dpi)
{
	_dpi = dpi;
}

const char * TBSystem::GetRoot()
{
	static char * basepath = NULL;
	if (!basepath)
	{
#ifdef ANDROID
		TBStr ExtPath(SDL_AndroidGetExternalStoragePath());
		ExtPath.Append("/");
		basepath = strdup(ExtPath.CStr());
#else
		basepath = SDL_GetBasePath();
#endif
	}
	return basepath;
}

const char * TBSystem::GetPrefPath()
{
	static const char * prefpath = NULL;
	if (!prefpath)
	{
#ifdef __EMSCRIPTEN__
		EM_ASM(
			   // /data should have been mounted in preRun(), check if Module.syncdone
			   if (!Module.syncdone) {
				   Module.print("WARNING GetPrefPath() : fs syncdone=" + Module.syncdone);
				   assert(Module.syncdone);
			   }
			   );
		prefpath = "/data/";
#else
		prefpath = SDL_GetPrefPath("spindrops", "SpinDrops");
#endif
	}
	return prefpath;
}

} // namespace tb

#endif // TB_SYSTEM_SDL
