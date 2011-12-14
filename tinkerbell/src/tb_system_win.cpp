// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_system.h"
#include <Windows.h>
#include <mmsystem.h>

namespace tinkerbell {

#ifdef _DEBUG

void TBDebugOut(const char *str)
{
	OutputDebugString(str);
}

#endif // _DEBUG

// == TBSystem ========================================

double TBSystem::GetTimeMS()
{
	return timeGetTime();
}

// Implementation currently done in port_glut.cpp.
// Windows timer suck. Glut timers suck too (can't be canceled) but that will do for now.
//void TBSystem::RescheduleTimer(double fire_time)
//{
//}

int TBSystem::GetPanThreshold()
{
	return 5;
}

// == TBClipboard =====================================

void TBClipboard::Empty()
{
	EmptyClipboard();
}

bool TBClipboard::HasText()
{
	bool has_text = false;
	if (OpenClipboard(NULL))
	{
		has_text = IsClipboardFormatAvailable(CF_TEXT) || IsClipboardFormatAvailable(CF_OEMTEXT);
		CloseClipboard();
	}
	return has_text;
}

bool TBClipboard::SetText(const char *text)
{
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();

		HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, strlen(text) + 1);
		char *pchData = (char*) GlobalLock(hClipboardData);
		strcpy(pchData, text);
		GlobalUnlock(hClipboardData);

		SetClipboardData(CF_TEXT, hClipboardData);
		CloseClipboard();
		return true;
	}
	return false;
}

bool TBClipboard::GetText(TBStr &text)
{
	if (HasText() && OpenClipboard(NULL))
	{
		HANDLE hClipboardData = GetClipboardData(CF_TEXT);
		char *pchData = (char*)GlobalLock(hClipboardData);
		bool ret = text.Set(pchData);
		GlobalUnlock(hClipboardData);
		CloseClipboard();
		return ret;
	}
	return false;
}

}; // namespace tinkerbell
