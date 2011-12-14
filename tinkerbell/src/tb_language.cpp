// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_language.h"
#include "tb_system.h"
#include "parser/TBNodeTree.h"

namespace tinkerbell {

TBLanguage::~TBLanguage()
{
	Clear();
}

bool TBLanguage::Load(const char *filename)
{
	// Read the file into a node tree (even though it's only a flat list)
	TBNode node;
	if (!node.ReadFile(filename))
		return false;

	// Go through all nodes and add to the strings hash table
	TBNode *n = node.GetFirstChild();
	while (n)
	{
		const char *str = n->GetValue().GetString();
		TBStr *new_str = new TBStr(str);
		if (!new_str || !strings.Add(TBID(n->GetName()), new_str))
		{
			delete new_str;
			return false;
		}
		n = n->GetNext();
	}
	return true;
}

void TBLanguage::Clear()
{
	strings.DeleteAll();
}

const char *TBLanguage::GetString(const TBID &id)
{
	if (TBStr *str = strings.Get(id))
		return *str;
#ifdef _DEBUG
	static TBStr tmp;
	tmp.SetFormatted("<TRANSLATE:%s>", id.debug_string);
	return tmp;
#else
	return "<TRANSLATE!>";
#endif
}

}; // namespace tinkerbell
