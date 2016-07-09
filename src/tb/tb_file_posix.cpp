// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_system.h"

#ifdef TB_FILE_POSIX

#include <stdio.h>

namespace tb {

class TBPosixFile : public TBFile
{
public:
	TBPosixFile(FILE *f) : file(f) {}
	virtual ~TBPosixFile() { fclose(file); }

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
#ifdef TB_RUNTIME_DEBUG_INFO
	virtual size_t Write(void *buf, size_t elemSize, size_t count)
	{
		return fwrite(buf, elemSize, count, file);
	}
#endif
private:
	FILE *file;
};

// static
TBFile *TBFile::Open(const char *filename, TBFileMode mode)
{
	FILE *f = nullptr;
	TBStr pathfile(TBSystem::GetRoot());
	pathfile.Append(filename);
	switch (mode)
	{
	case MODE_READ:
		f = fopen(pathfile.CStr(), "rb");
		break;
#ifdef TB_RUNTIME_DEBUG_INFO
	case MODE_WRITETRUNC:
		f = fopen(pathfile.CStr(), "w");
		break;
#endif
	default:
		break;
	}
#ifdef TB_RUNTIME_DEBUG_INFO
	if (!f)
		TBDebugPrint("TBFile::Open, unable to open file '%s'\n", pathfile.CStr());
#endif
	if (!f)
		return nullptr;
	TBPosixFile *tbf = new TBPosixFile(f);
	if (!tbf)
		fclose(f);
	return tbf;
}

} // namespace tb

#endif // TB_FILE_POSIX
