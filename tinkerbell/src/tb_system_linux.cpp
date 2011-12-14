// ================================================================================
// == This file is a part of Tinker Bell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_system.h"
#include <sys/time.h>

namespace tinkerbell {

#ifdef _DEBUG

void TBDebugOut(const char *str)
{
	printf(str);
}

#endif // _DEBUG

// == TBSystem ========================================

double TBSystem::GetTimeMS()
{
	struct timeval now;
	gettimeofday( &now, NULL );
	return now.tv_usec/1000 + now.tv_sec*1000;
}

// Implementation currently done in port_glut.cpp.
// FIX: Implement here for linux-desktop/android/macos?
//void TBSystem::RescheduleTimer(double fire_time)
//{
//}

int TBSystem::GetPanThreshold()
{
	return 5;
}

// == TBClipboard =====================================

TBStr clipboard; ///< FIX: Obviosly not the full implementation :)

void TBClipboard::Empty()
{
	clipboard.Clear();
}

bool TBClipboard::HasText()
{
	return !clipboard.IsEmpty();
}

bool TBClipboard::SetText(const char *text)
{
	return clipboard.Set(text);
}

bool TBClipboard::GetText(TBStr &text)
{
	return text.Set(clipboard);
}

}; // namespace tinkerbell
