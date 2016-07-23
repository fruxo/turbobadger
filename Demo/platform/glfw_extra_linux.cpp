#include "tb_system.h"

#if defined(TB_TARGET_LINUX) || defined(TB_TARGET_MACOSX)

#include "glfw_extra.h"
#include "tb_msg.h"
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#define errExit(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

#define CLOCK_ID CLOCK_REALTIME
#define SIG_NO SIGRTMIN

static bool timerexists = false;
static timer_t timerid;
static struct sigevent sev;
static struct sigaction sa;
static struct itimerspec its;

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

static void timerHandler( int sig)
{
  if (timerCallback)
		timerCallback();
}

static void makeTimer(timer_t *timerID)
{
	// Establish handler for signal
  sa.sa_handler = timerHandler;
  sigemptyset(&sa.sa_mask);
  if (sigaction(SIG_NO, &sa, NULL) == -1)
		errExit("Can't change timer signal action");

	// Create timer
	sev.sigev_notify = SIGEV_SIGNAL;
  sev.sigev_signo = SIG_NO;
  sev.sigev_value.sival_ptr = timerID;
  if (timer_create(CLOCK_ID, &sev, timerID) == -1)
		errExit("Can't create timer");
}

static void startTimer(unsigned int ms)
{
	its.it_value.tv_sec = ms / 1000;
	its.it_value.tv_nsec = (ms % 1000) * 1000000;
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;

	if (timer_settime(timerid, 0, &its, NULL) == -1)
		errExit("Can't start timer");
}

static void stopTimer()
{
	its.it_value.tv_sec = 0;
	its.it_value.tv_nsec = 0;
	if (timer_settime(timerid, 0, &its, NULL) == -1)
		errExit("Can't stop timer");
}


void glfwWakeUpMsgLoop(GLFWwindow *window)
{
}

void glfwWaitMsgLoop(GLFWwindow *window)
{
	glfwPollEvents();
}

void glfwPollMsgLoop(GLFWwindow *window)
{
	glfwPollEvents();
}

void glfwRescheduleTimer(unsigned int delay_ms)
{
	if (!timerexists)
	{
		makeTimer(&timerid);
		timerexists = true;
	}

	startTimer(delay_ms);
}

void glfwKillTimer()
{
	stopTimer();
}

void glfwSetTimerCallback(GLFWtimerfun cbfun)
{
	timerCallback = cbfun;
}

#endif // defined(TB_TARGET_LINUX) || defined(TB_TARGET_MACOSX)
