// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_system.h"

#ifdef TB_FILE_POSIX

#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#else
/* This is intended as a drop-in replacement for unistd.h on Windows.
 * Please add functionality as neeeded.
 * https://stackoverflow.com/a/826027/1202830
 */
#include <stdlib.h>
#include <io.h>
//#include <getopt.h> /* getopt at: https://gist.github.com/ashelly/7776712 */
#include <process.h> /* for getpid() and the exec..() family */
#include <direct.h> /* for _getcwd() and _chdir() */
#define getcwd _getcwd
/* -- cut -- */
#endif

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
	virtual size_t Write(const void *buf, size_t elemSize, size_t count)
	{
		return fwrite(buf, elemSize, count, file);
	}
	virtual size_t Write(const TBStr & str)
	{
		return Write(str.CStr(), 1, str.Length());
	}
private:
	FILE *file;
};

// static
TBFile *TBFile::Open(const TBStr & filename, TBFileMode mode)
{
	FILE *f = nullptr;
	switch (mode)
	{
	case MODE_READ:
		f = fopen(filename.CStr(), "rb");
		break;
	case MODE_WRITETRUNC:
		f = fopen(filename.CStr(), "w");
		break;
	default:
		break;
	}
#if defined(TB_RUNTIME_DEBUG_INFO) && 1
	if (!f) {
		char tmp[256];
		TBDebugPrint("Cwd: '%s'\n", getcwd(tmp, sizeof(tmp)));
		TBDebugPrint("TBFile::Open, unable to open file '%s'\n", filename.CStr());
	}
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
