// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_DEBUG_H
#define TB_DEBUG_H

#include "tb_config.h"

#ifdef _DEBUG
#define TB_IF_DEBUG(debug) debug
#else
#define TB_IF_DEBUG(debug) 
#endif

namespace tinkerbell {

#ifdef TB_RUNTIME_DEBUG_INFO

class TBDebugInfo
{
public:
	TBDebugInfo();

	enum SETTING {
		/** Show widgets bounds */
		LAYOUT_BOUNDS,
		/** Show child widget clipping set by some widgets. */
		LAYOUT_CLIPPING,
		/** Show highlights on widgets that recalculate their preferred
			size, and those who recalculate their layout. */
		LAYOUT_PS_DEBUGGING,

		NUM_SETTINGS
	};
	int settings[NUM_SETTINGS];
};

extern TBDebugInfo g_tb_debug;

/** Show a window containing runtime debugging settings. */
void ShowDebugInfoSettingsWindow(class TBWidget *root);

#define TB_DEBUG_SETTING(setting) g_tb_debug.settings[TBDebugInfo::setting]
#define TB_IF_DEBUG_SETTING(setting, code) if (TB_DEBUG_SETTING(setting)) { code; }

#else // TB_RUNTIME_DEBUG_INFO

/** Show a window containing runtime debugging settings. */
#define ShowDebugInfoSettingsWindow(root) ((void)0)

#define TB_DEBUG_SETTING(setting) false
#define TB_IF_DEBUG_SETTING(setting, code) 

#endif // TB_RUNTIME_DEBUG_INFO

}; // namespace tinkerbell

#endif // TB_DEBUG_H
