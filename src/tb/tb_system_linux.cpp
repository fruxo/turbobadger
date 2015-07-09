// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_system.h"
#include "tb_msg.h"

#ifdef TB_SYSTEM_LINUX

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/timerfd.h>
#include <unistd.h>

#ifdef TB_RUNTIME_DEBUG_INFO


void TBDebugOut(const char *str)
{
	printf("%s", str);
}

#endif // TB_RUNTIME_DEBUG_INFO

namespace tb {

// == TBSystem ========================================

double TBSystem::GetTimeMS()
{
	struct timeval now;
	gettimeofday( &now, NULL );
	return now.tv_usec/1000 + now.tv_sec*1000;
}


/** FIXME: Timer facility only available in Linux **/
#ifdef TB_SYSTEM_LINUX

int	TBSystem::timer_fd;
bool TBSystem::timer_state;
	
int TBSystem::Init()
{

	timer_fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);

	errno = 0;
	
	if(timer_fd < 0){
		TBDebugOut("Failed to obtain timer descriptor!");
		return errno;
	}

	return 0;
}

int	TBSystem::Shutdown()
{

	timer_state = false;
	
	errno = 0;

	if(timer_fd >= 0)
		close(timer_fd);

	if(timer_fd < 0){

		TBDebugOut("Failed to close timer descriptor!");
		return errno;
	}

	return 0;
}

void TBSystem::RescheduleTimer(double fire_time)
{

	if(fire_time == TB_NOT_SOON)
		return;

	if(fire_time == 0){
		timer_state = true;
		return;
	}
	int status = 0;

	struct timespec fire_spec;

	fire_time /= 1000;
	fire_spec.tv_sec = fire_time;
	fire_spec.tv_nsec = (fire_time - fire_spec.tv_sec)*1e9;

	struct timespec zero_spec = {0,0};
	struct itimerspec timer_spec  = { .it_interval = zero_spec,
									  .it_value = fire_spec };

	timer_state = false;

	errno = 0;
	status = timerfd_settime(timer_fd,TFD_TIMER_ABSTIME,
							 &timer_spec, NULL);
	if(status < 0)
		TBDebugOut("Failed to arm timer!");
}

int TBSystem::PollEvents()
{

	if(!timer_state){

		ssize_t rc = 0;
		uint64_t count = 0;
		
		errno = 0;
		rc = read(timer_fd, &count, sizeof(count));
		
		if(rc > 0 && count > 0)
			timer_state = true;
		
		else{

			if(errno == EAGAIN || errno == EWOULDBLOCK);

			else{
				TBDebugPrint("Failed to read from timer file!:%s", strerror(errno));
				return errno;
			}
		}
	}
	
	if(timer_state){

		timer_state = false;
		
		TBMessageHandler::ProcessMessages();
		// If we still have things to do (because we didn't process all messages,
		// or because there are new messages), we need to rescedule, so call RescheduleTimer.
		TBSystem::RescheduleTimer(TBMessageHandler::GetNextMessageFireTime());

	}


	return 0;
}
#endif /* TB_SYSTEM_LINUX */

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

int TBSystem::GetDPI()
{
	// FIX: Implement!
	return 96;
}

}; // namespace tb

#endif // TB_SYSTEM_LINUX
