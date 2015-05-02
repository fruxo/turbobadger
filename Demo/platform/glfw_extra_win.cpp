#include "tb_system.h"

#ifdef TB_TARGET_WINDOWS
#include "glfw_extra.h"
#include "tb_msg.h"
#include "GLFW/glfw3native.h"

GLFWtimerfun timerCallback;
UINT_PTR timer_id;
HWND timer_hwnd;
WNDCLASS wndclass;
bool prevent_starvation;

#define GLFW_EXTRA_MSG_NULL		WM_USER + 1
#define FAKE_WM_TIMER			WM_USER + 2

LRESULT PASCAL TimerProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if ((message == WM_TIMER || message == FAKE_WM_TIMER) && timerCallback)
		timerCallback();
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void CreateTimerWindow()
{
	// Set up and register window class
	memset(&wndclass, 0, sizeof(WNDCLASS));
	wndclass.lpfnWndProc = TimerProc;
	wndclass.lpszMenuName = "TB_TIMER_WINDOW";
	wndclass.lpszClassName = "TB_TIMER_WINDOW";
	RegisterClass((WNDCLASS*)&wndclass);

	timer_hwnd = CreateWindow("TB_TIMER_WINDOW", "TB_TIMER_WINDOW", 0,
								  10, 10, 100, 100,
								  HWND_MESSAGE, (HMENU)NULL, NULL, NULL);
}

void glfwWakeUpMsgLoop(GLFWwindow *window)
{
	prevent_starvation = true;
	PostMessage(glfwGetWin32Window(window), GLFW_EXTRA_MSG_NULL, 0, 0);
}

void glfwWaitMsgLoop(GLFWwindow *window)
{
	glfwWaitEvents();
	prevent_starvation = false;
}

void glfwPollMsgLoop(GLFWwindow *window)
{
	glfwPollEvents();
	prevent_starvation = false;
}

void glfwRescheduleTimer(unsigned int delay_ms)
{
	if (!timer_hwnd)
		CreateTimerWindow();

	if (delay_ms == 0)
	{
		// Some hackery:
		// If we're supposed to handle messages ASAP, we can't use SetTimer since
		// it's capped at 10ms minimum delay. Instead post the FAKE_WM_TIMER
		// message immediately. This will however starve the message loop
		// completely (freeze input) if we have a lot to do since posting
		// messages have higher priority in windows. To prevent this, we'll take
		// the 10ms penalty if we have any other messages in the queue or
		// glfwWakeUpMsgLoop was called (prevent_starvation is true).

		// Warning: Windows may actually *deliver* certain messages during this
		// call which may be unexpected with our current stack. This should
		// hopefully not be a problem in this demo app :)

		MSG msg;
		if (!prevent_starvation && !PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			glfwKillTimer();
			PostMessage(timer_hwnd, FAKE_WM_TIMER, 0, 0);
			return;
		}
	}
	timer_id = SetTimer(timer_hwnd, 1, delay_ms, 0);
}

void glfwKillTimer()
{
	if (timer_id)
	{
		KillTimer(timer_hwnd, timer_id);
		timer_id = 0;
	}
}

void glfwSetTimerCallback(GLFWtimerfun cbfun)
{
	timerCallback = cbfun;
}

#endif // TB_TARGET_WINDOWS
