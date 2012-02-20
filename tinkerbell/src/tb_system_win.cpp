// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_system.h"
#include <Windows.h>
#include <mmsystem.h>
#include <stdio.h>

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

// == TBFile =====================================

class TBWinFile : public TBFile
{
public:
	TBWinFile(FILE *f) : file(f) {}
	virtual ~TBWinFile() { fclose(file); }

	virtual long Size()
	{
		long oldpos = ftell(file);
		fseek(file, 0, SEEK_END);
		long num_bytes = ftell(file);
		fseek(file, oldpos, SEEK_SET);
		return num_bytes;
	}
	virtual size_t Read(void *buf, size_t elemSize, size_t count)
	{
		return fread(buf, elemSize, count, file);
	}
private:
	FILE *file;
};

TBFile *TBFile::Open(const char *filename, TBFileMode mode)
{
	FILE *f = nullptr;
	switch (mode)
	{
	case MODE_READ:
		f = fopen(filename, "rb");
		break;
	default:
		break;
	}
	if (!f)
		return nullptr;
	TBWinFile *tbf = new TBWinFile(f);
	if (!tbf)
		fclose(f);
	return tbf;
}

}; // namespace tinkerbell
