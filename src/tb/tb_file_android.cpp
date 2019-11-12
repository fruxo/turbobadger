// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_system.h"

#ifdef TB_FILE_ANDROID

#include "tb_debug.h"
#include "tb_str.h"

#include <android/log.h>
#include <stdio.h>

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/configuration.h>

// in tb_system_android
extern AAssetManager *g_pManager;

namespace tb {

// == TBFile =====================================

class TBAndroidFile : public TBFile
{
public:
	TBAndroidFile(AAsset* f) : file(f) {}
	virtual ~TBAndroidFile() { AAsset_close(file); }

	virtual long Size()
	{
		return AAsset_getLength(file);
	}
	virtual size_t Read(void *buf, size_t elemSize, size_t count)
	{
		return AAsset_read(file, buf, elemSize * count);
	}
	virtual size_t Write(const void *buf, size_t elemSize, size_t count) {return 0;}
	virtual size_t Write(const TBStr & str) {return 0;}

private:
	AAsset *file;
};

TBFile *TBFile::Open(const TBStr & filename, TBFileMode mode)
{
	AAsset *f = nullptr;
	switch (mode)
	{
	case MODE_READ:
		f = AAssetManager_open(g_pManager, (const char *)filename, AASSET_MODE_UNKNOWN);
		break;
	default:
		break;
	}
	if (!f)
		return nullptr;
	TBAndroidFile *tbf = new TBAndroidFile(f);
	if (!tbf)
		AAsset_close(f);
	return tbf;
}

} // namespace tb

#endif // TB_FILE_ANDROID
