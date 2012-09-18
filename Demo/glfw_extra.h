#ifndef GLFW_EXTRA_H
#define GLFW_EXTRA_H

#ifdef WIN32
#include <Windows.h> // Avoid compilation warnings in GL/glfw.h
#endif
#include "GL/glfw3.h"

/* Function pointer types */
typedef void (* GLFWtimerfun)();

/** Post a message to the message loop so we'll spin it once more. */
void glfwPostNull(GLFWwindow window);

/** Start a timer that after the delay will call the callback set by glfwSetTimerCallback.
	Calling this should cancel any previously scheduled time out that has not yet happened. */
void glfwRescheduleTimer(unsigned int delay_ms);

/** Kill the timer started by glfwRescheduleTimer. */
void glfwKillTimer();

/** Set the callback that should be called on timeout scheduled by glfwRescheduleTimer. */
void glfwSetTimerCallback(GLFWtimerfun cbfun);

#endif // GLFW_EXTRA_H
