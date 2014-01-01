// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2014, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tinkerbell.h"
#include "tb_skin.h"
#include "tb_widgets_reader.h"
#include "tb_language.h"
#include "tb_font_renderer.h"
#include "tb_addon.h"
#include "tb_system.h"

namespace tinkerbell {

TBRenderer *g_renderer = nullptr;
TBSkin *g_tb_skin = nullptr;
TBWidgetsReader *g_widgets_reader = nullptr;
TBLanguage *g_tb_lng = nullptr;
TBFontManager *g_font_manager = nullptr;

bool init_tinkerbell(TBRenderer *renderer, const char *lng_file)
{
	g_renderer = renderer;
	g_tb_lng = new TBLanguage;
	g_tb_lng->Load(lng_file);
	g_font_manager = new TBFontManager();
	g_tb_skin = new TBSkin();
	g_widgets_reader = TBWidgetsReader::Create();
	return TBInitAddons();
}

void shutdown_tinkerbell()
{
	TBShutdownAddons();
	delete g_widgets_reader;
	delete g_tb_skin;
	delete g_font_manager;
	delete g_tb_lng;
}

}; // namespace tinkerbell
