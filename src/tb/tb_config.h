// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================
//
// This file contains defines for the default configuration of Turbo Badger.
// You may change these here, but to make upgrades easier it's better to create a
// copy of this file in a include path that is searched before Turbo Badger during
// build (F.ex the solution directory for Visual Studio).

#ifndef TB_CONFIG_H
#define TB_CONFIG_H

/** Enable for some handy runtime debugging, enabled by modifying
	the various settings in g_tb_debug. A settings window can be
	shown by calling ShowDebugInfoSettingsWindow. */
#ifdef _DEBUG
#define TB_RUNTIME_DEBUG_INFO
#endif

/** Enable if the focus state should automatically be set on edit fields even
	when using the pointer. It is normally set only while moving focus by keyboard. */
//#define TB_ALWAYS_SHOW_EDIT_FOCUS

/** Enable to use premultiplied alpha. Warning: This is not handled everywhere in
	the default backends, so consider it an experimental and unfinished feature! */
//#define TB_PREMULTIPLIED_ALPHA

#endif // TB_CONFIG_H
