// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_color.h"
#include "tb_core.h"
#include "tb_system.h"
#include "tb_node_tree.h"
#include <stdio.h>

namespace tb {

// == TBColor ===============================================================================
void TBColor::SetFromString(const TBStr & str_)
{
	int len = str_.Length();
	const char * str = (const char *)str_;
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
	//if (g_color_manager)
	//	g_color_manager->Define(str, *this);
}

void TBColor::GetString(TBStr & str) const
{
	str.SetFormatted("#%02x%02x%02x%02x", r, g, b, a);
}

// == TBColorManager ========================================================================

void TBColorManager::Load(TBNode * n, TBSkin * skin)
{
	// Go through all nodes and add to the strings hash table
	for (n = n->GetFirstChild(); n; n = n->GetNext())
	{
		TBColor c;
		c.SetFromString(n->GetValue().GetString());
		Define(n->GetName(), c);
	}
}

bool TBColorManager::Define(const std::string & id, TBColor color)
{
	if (!_id2color.count(id)) {
		//TBDebugPrint("Define color '%s' -> %x\n", id.c_str(), (uint32_t)color);
		_id2color[id] = color;
		_color2id[color] = id;
		return true;
	}
	return false;
}

void TBColorManager::ReDefine(const std::string & id, TBColor color)
{
	//TBDebugPrint("ReDefine color '%s' -> %x\n", id.c_str(), (uint32_t)color);
	_id2color[id] = color;
	_color2id[color] = id;
}

void TBColorManager::Clear()
{
	_id2color.clear();
	_color2id.clear();
}

TBColor TBColorManager::GetColor(const std::string &id) const
{
	if (_id2color.count(id))
		return _id2color.at(id);
	return TBColor(0, 0, 0, 0);
}

std::string TBColorManager::GetColorID(const TBColor & color) const
{
	if (_color2id.count(color))
		return _color2id.at(color);
	return std::string();
}

void TBColorManager::Dump(const char * filename)
{
#ifdef TB_RUNTIME_DEBUG_INFO
	TBFile * file = TBFile::Open(filename, TBFile::MODE_WRITETRUNC);
	if (file) {
		TBStr str;
		str.SetFormatted("colors\n");
		file->Write(str);
		for (auto & id_co : _id2color) {
			TBStr cs;
			id_co.second.GetString(cs);
			str.SetFormatted("	%s %s\n", id_co.first.c_str(), cs.CStr());
			file->Write(str);
		}
		delete file;
	}
#endif
}

} // namespace tb
