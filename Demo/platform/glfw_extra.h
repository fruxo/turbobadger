#ifndef GLFW_EXTRA_H
#define GLFW_EXTRA_H

#ifdef WIN32
#include <Windows.h> // Avoid compilation warnings in GLFW/glfw.h
#endif
#include "GLFW/glfw3.h"

/* Function pointer types */
typedef void (* GLFWtimerfun)();

/** Do something (like posting a dummy message) to get the message loop to stop waiting
	in glfwWaitMsgLoop. */
void glfwWakeUpMsgLoop(GLFWwindow *window);

/** Wait for a new message in the message loop. Should stop waiting if glfwWakeUpMsgLoop is called. */
void glfwWaitMsgLoop(GLFWwindow *window);

/** Handle message(s) waiting in the message loop, or return immediately if there are none. */
void glfwPollMsgLoop(GLFWwindow *window);

/** Start a timer that after the delay will call the callback set by glfwSetTimerCallback.
	Calling this should cancel any previously scheduled time out that has not yet happened. */
void glfwRescheduleTimer(unsigned int delay_ms);

/** Kill the timer started by glfwRescheduleTimer. */
void glfwKillTimer();

/** Set the callback that should be called on timeout scheduled by glfwRescheduleTimer. */
void glfwSetTimerCallback(GLFWtimerfun cbfun);

#endif // GLFW_EXTRA_H
