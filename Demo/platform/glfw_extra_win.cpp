#include "glfw_extra.h"
#include "tb_msg.h"
#include "tb_system.h"

#define GLFW_EXPOSE_NATIVE_WIN32_WGL
#include "GL/glfw3native.h"

GLFWtimerfun timerCallback;
UINT_PTR timer_id;

#define GLFW_EXTRA_MSG_NULL WM_USER + 1

void glfwWakeUpMsgLoop(GLFWwindow window)
{
	PostMessage(glfwGetWin32Window(window), GLFW_EXTRA_MSG_NULL, 0, 0);
}

void glfwWaitMsgLoop(GLFWwindow window)
{
	glfwWaitEvents();
}

void glfwPollMsgLoop(GLFWwindow window)
{
	glfwPollEvents();
}

VOID CALLBACK windows_timer_proc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	if (timerCallback)
		timerCallback();
}

void glfwRescheduleTimer(unsigned int delay_ms)
{
	timer_id = SetTimer(NULL, timer_id, delay_ms, windows_timer_proc);
}

void glfwKillTimer()
{
	if (timer_id)
	{
		KillTimer(NULL, timer_id);
		timer_id = 0;
	}
}

void glfwSetTimerCallback(GLFWtimerfun cbfun)
{
	timerCallback = cbfun;
}
