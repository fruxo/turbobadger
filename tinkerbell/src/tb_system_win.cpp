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
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();
		CloseClipboard();
	}
}

bool TBClipboard::HasText()
{
	bool has_text = false;
	if (OpenClipboard(NULL))
	{
		has_text =	IsClipboardFormatAvailable(CF_TEXT) ||
					IsClipboardFormatAvailable(CF_OEMTEXT) ||
					IsClipboardFormatAvailable(CF_UNICODETEXT);
		CloseClipboard();
	}
	return has_text;
}

bool TBClipboard::SetText(const char *text)
{
	if (OpenClipboard(NULL))
	{
		int num_wide_chars_needed = MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
		if (HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE, num_wide_chars_needed * sizeof(wchar_t)))
		{
			LPWSTR pchData = (LPWSTR) GlobalLock(hClipboardData);
			MultiByteToWideChar(CP_UTF8, 0, text, -1, pchData, num_wide_chars_needed);
			GlobalUnlock(hClipboardData);

			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, hClipboardData);
		}

		CloseClipboard();
		return true;
	}
	return false;
}

bool TBClipboard::GetText(TBStr &text)
{
	bool success = false;
	if (HasText() && OpenClipboard(NULL))
	{
		if (HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT))
		{
			wchar_t *pchData = (wchar_t*) GlobalLock(hClipboardData);
			int len = WideCharToMultiByte(CP_UTF8, 0, pchData, -1, NULL, 0, NULL, NULL);
			if (char *utf8 = new char[len])
			{
				WideCharToMultiByte(CP_UTF8, 0, pchData, -1, utf8, len, NULL, NULL);
				success = text.Set(utf8);
				delete [] utf8;
			}
			GlobalUnlock(hClipboardData);
		}
		CloseClipboard();
	}
	return success;
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
