#ifndef DEMO_CONFIG_H
#define DEMO_CONFIG_H

// This sucks, we need to define this to be able to build glfw without
// an extra step after checkout (to generate config.h using cmake).

#ifdef WIN32
# define _GLFW_USE_OPENGL
# define _GLFW_WGL
#elif defined(MACOSX)
# define _GLFW_COCOA_NSGL
#else
# define _GLFW_X11_GLX
# define _GLFW_HAS_GLXGETPROCADDRESS
#endif

#define _GLFW_VERSION_FULL "3.0.1"

#endif // DEMO_CONFIG_H
