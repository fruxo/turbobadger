// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_config.h"
#include "tb_core.h"
#include "tb_skin.h"
#include "tb_widgets_reader.h"
#include "tb_language.h"
#include "tb_font_renderer.h"
#include "tb_addon.h"
#include "tb_system.h"
#include "animation/tb_animation.h"
#include "image/tb_image_manager.h"

namespace tb {

TBRenderer *g_renderer = nullptr;
TBSkin *g_tb_skin = nullptr;
TBWidgetsReader *g_widgets_reader = nullptr;
TBLanguage *g_tb_lng = nullptr;
TBFontManager *g_font_manager = nullptr;

bool tb_core_init(TBRenderer *renderer, const char *lng_file)
{
	TBDebugPrint("Initiating Turbo Badger - version %s\n", TB_VERSION_STR);
	g_renderer = renderer;
	g_tb_lng = new TBLanguage;
	g_tb_lng->Load(lng_file);
	g_font_manager = new TBFontManager();
	g_tb_skin = new TBSkin();
	g_widgets_reader = TBWidgetsReader::Create();
#ifdef TB_IMAGE
	g_image_manager = new TBImageManager();
#endif

	#ifdef TB_SYSTEM_LINUX
	TBSystem::Init();
	#endif
	
	return TBInitAddons();
}

void tb_core_shutdown()
{
	TBAnimationManager::AbortAllAnimations();
	TBShutdownAddons();
#ifdef TB_IMAGE
	delete g_image_manager;
#endif
	delete g_widgets_reader;
	delete g_tb_skin;
	delete g_font_manager;
	delete g_tb_lng;

	#ifdef TB_SYSTEM_LINUX
	TBSystem::Shutdown();
	#endif
}

bool tb_core_is_initialized()
{
	return g_widgets_reader ? true : false;
}

}; // namespace tb
