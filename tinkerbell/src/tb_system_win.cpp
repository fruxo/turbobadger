// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2014, Emil Segerås  ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_system.h"
#include <Windows.h>
#include <mmsystem.h>
#include <stdio.h>

#ifdef _DEBUG

void TBDebugOut(const char *str)
{
	OutputDebugString(str);
}

#endif // _DEBUG

namespace tinkerbell {

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
	return 5 * GetDPI() / 96;
}

int TBSystem::GetPixelsPerLine()
{
	return 40 * GetDPI() / 96;
}

int TBSystem::GetDPI()
{
	HDC hdc = GetDC(nullptr);
	int DPI_x = GetDeviceCaps(hdc, LOGPIXELSX);
	ReleaseDC(nullptr, hdc);
#if 0 // TEST CODE!
	DPI_x *= 2;
#endif
	return DPI_x;
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
