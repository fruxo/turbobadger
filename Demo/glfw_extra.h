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

void glfwRescheduleTimer(double fire_time);
void glfwKillTimer();
void glfwSetTimerCallback(GLFWtimerfun cbfun);

#endif // GLFW_EXTRA_H
