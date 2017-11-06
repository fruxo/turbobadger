// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_color.h"
#include "tb_core.h"
#include "tb_node_tree.h"
#include <stdio.h>

namespace tb {

// == TBColor ===============================================================================

void TBColor::SetFromString(const char *str, int len)
{
	int r, g, b, a;
	if (len && str[0] != '#' && g_color_manager->IsDefined(str))
		Set(g_color_manager->GetColor(str));
	else if (len == 9 && sscanf(str, "#%2x%2x%2x%2x", &r, &g, &b, &a) == 4)		// rrggbbaa
		Set(TBColor(r, g, b, a));
	else if (len == 7 && sscanf(str, "#%2x%2x%2x", &r, &g, &b) == 3)			// rrggbb
		Set(TBColor(r, g, b));
	else if (len == 5 && sscanf(str, "#%1x%1x%1x%1x", &r, &g, &b, &a) == 4)		// rgba
		Set(TBColor(r + (r << 4), g + (g << 4), b + (b << 4), a + (a << 4)));
	else if (len == 4 && sscanf(str, "#%1x%1x%1x", &r, &g, &b) == 3)			// rgb
		Set(TBColor(r + (r << 4), g + (g << 4), b + (b << 4)));
	else {
		TBDebugPrint("Invalid color '%s'\n", str);
		Set(TBColor());
	}
}

void TBColor::GetString(TBStr & str) const
{
	str.SetFormatted("#%2x%2x%2x%2x", r, g, b, a);
}

// == TBColorManager ========================================================================

void TBColorManager::Load(TBNode * n, TBSkin * skin)
{
	// Go through all nodes and add to the strings hash table
	for (TBNode * n = n->GetFirstChild(); n; n = n->GetNext())
	{
		TBColor c;
		const char * cname = n->GetValue().GetString();
		c.SetFromString(cname, strlen(cname));
		Define(n->GetName(), c);
	}
}

bool TBColorManager::Define(const TBID & id, TBColor color)
{
	if (!_colors.count(id)) {
		_colors[id] = color;
		return true;
	}
	return false;
}

void TBColorManager::ReDefine(const TBID & id, TBColor color)
{
	_colors[id] = color;
}

void TBColorManager::Clear()
{
	_colors.clear();
}

TBColor TBColorManager::GetColor(const TBID &id) const
{
	if (_colors.count(id))
		return _colors.at(id);
	return TBColor(0, 0, 0, 0);
}

} // namespace tb
