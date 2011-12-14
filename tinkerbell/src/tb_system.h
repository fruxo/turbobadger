// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_SYSTEM_H
#define TB_SYSTEM_H

#include "tinkerbell.h"

namespace tinkerbell {

#ifdef _DEBUG
void TBDebugOut(const char *str);
#else
#define TBDebugOut(str) ((void)0)
#endif

// == Platform interface ===================================================

/** TBSystem is porting interface for the underlaying OS. */
class TBSystem
{
public:
	/** Get the system time in milliseconds since some undefined epoch. */
	static double GetTimeMS();

	/** Called when the need to call TBMessageHandler::ProcessMessages has changed due to changes in the
		message queue. fire_time is the new time is needs to be called.
		It may be 0 which means that ProcessMessages should be called asap (but NOT from this call!)
		It may also be TB_NOT_SOON which means that ProcessMessages doesn't need to be called. */
	static void RescheduleTimer(double fire_time);

	/** Get how many pixels of dragging should start panning scrollable widgets. */
	static int GetPanThreshold();
};

/** TBClipboard is a porting interface for the clipboard. */
class TBClipboard
{
public:
	static void Empty();
	static bool HasText();
	static bool SetText(const char *text);
	static bool GetText(TBStr &text);
};

}; // namespace tinkerbell

#endif // TB_SYSTEM_H
