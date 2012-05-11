// ================================================================================
// == This file is a part of Tinker Bell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_system.h"
#include <sys/time.h>
#include <stdio.h>

namespace tinkerbell {

#ifdef _DEBUG

void TBDebugOut(const char *str)
{
	printf("%s", str);
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

// == TBFile =====================================

class TBLinuxFile : public TBFile
{
public:
	TBLinuxFile(FILE *f) : file(f) {}
	virtual ~TBLinuxFile() { fclose(file); }

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
	TBLinuxFile *tbf = new TBLinuxFile(f);
	if (!tbf)
		fclose(f);
	return tbf;
}

}; // namespace tinkerbell
