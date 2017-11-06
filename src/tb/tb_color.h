// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#ifndef TB_COLOR_H
#define TB_COLOR_H

#include "tb_types.h"
#include "tb_str.h"
#include "tb_id.h"
#include <map>


namespace tb {

class TBNode;
class TBSkin;

/** TBColor contains a 32bit color. */

class TBColor
{
public:
	TBColor() : b(0), g(0), r(0), a(255) {}
	TBColor(int r, int g, int b, int a = 255) : b(b), g(g), r(r), a(a) {}

	uint8 b, g, r, a;

	void Set(const TBColor &color) { *this = color; }

	/** Set the color from string in any of the following formats:
		"#rrggbbaa", "#rrggbb", "#rgba", "#rgb" */
	void SetFromString(const char *str, int len);

	void GetString(TBStr & str) const;

	operator uint32 () const		{ return *((uint32*)this); }
	bool operator == (const TBColor &c) const { return *this == (uint32)c; }
	bool operator != (const TBColor &c) const { return !(*this == c); }
};

/** TBColorManager contains a map of global color names. */

class TBColorManager
{
public:

	/** Load a list of colors from a node. */
	void Load(TBNode *n, TBSkin *skin);

	/** Define a color, if not already defined. */
	bool Define(const TBID & id, TBColor color);

	/** Is the color defined? */
	bool IsDefined(const TBID & id) { return 0 != _colors.count(id); }

	/** (Re)Define a color, no matter what. */
	void ReDefine(const TBID & id, TBColor color);

	/** Clear the list of colors. */
	void Clear();

	/** Return the color with the given id.
	 * If there is no color with that id, 0 will be returned.
	 */
	TBColor GetColor(const TBID &id) const;

private:
	std::map<TBID, TBColor> _colors;
};

} // namespace tb

#endif // TB_COLOR_H
