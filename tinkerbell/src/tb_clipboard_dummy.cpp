// ================================================================================
// == This file is a part of Tinker Bell UI Toolkit. (C) 2011-2013, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_system.h"
#include <sys/time.h>
#include <stdio.h>

namespace tinkerbell {

// == TBClipboard =====================================

TBStr clipboard; ///< Obviosly not a full implementation since it ignores the OS :)

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

}; // namespace tinkerbell
