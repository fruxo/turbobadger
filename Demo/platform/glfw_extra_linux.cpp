#include "tb_system.h"

#if defined(TB_TARGET_LINUX) || defined(TB_TARGET_MACOSX)

#include "glfw_extra.h"
#include "tb_msg.h"
#include <stdio.h>

//#include <X11/Xlib.h>

//#define GLX_GLXEXT_LEGACY
//#include <GL/glx.h>

//#define GLFW_EXPOSE_NATIVE_X11_GLX
//#include "GLFW/glfw3native.h"

//#include "GLFW/glfw3.h"

// ## NOTE ############################################
// FIX: Implement message loop and timer on linux!
//      For now, just poll using glfwPollEvents and
//      always call timer callback, so we keep spinning
//      and can at least run our code.

GLFWtimerfun timerCallback;

void glfwWakeUpMsgLoop(GLFWwindow *window)
{
}

void glfwWaitMsgLoop(GLFWwindow *window)
{
	glfwPollEvents();
	if (timerCallback)
		timerCallback();
}

void glfwPollMsgLoop(GLFWwindow *window)
{
	glfwPollEvents();
	if (timerCallback)
		timerCallback();
}

void glfwRescheduleTimer(unsigned int delay_ms)
{
}

void glfwKillTimer()
{
}

void glfwSetTimerCallback(GLFWtimerfun cbfun)
{
	timerCallback = cbfun;
}

#endif // defined(TB_TARGET_LINUX) || defined(TB_TARGET_MACOSX)
