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
#include <string>

namespace tb {

class TBNode;
class TBSkin;

/** TBColor contains a 32bit color. */

class TBColor
{
public:
	TBColor() : b(0), g(0), r(0), a(255) {}
	TBColor(int r, int g, int b, int a = 255) : b(b), g(g), r(r), a(a) {}
	TBColor(uint32_t bgra_) : bgra(bgra_) {}

	union {
		struct { uint8_t b, g, r, a; };
		uint32_t bgra;
	};

	void Set(const TBColor &color) { *this = color; }

	/** Set the color from string in any of the following formats:
		"#rrggbbaa", "#rrggbb", "#rgba", "#rgb" */
	void SetFromString(const char *str, int len);

	/** Write color to string with format #rrggbbaa */
	void GetString(TBStr & str) const;

	operator uint32_t () const		{ return bgra; }
	//bool operator == (const TBColor &c) const { return bgra == (uint32_t)c; }
	//bool operator != (const TBColor &c) const { return bgra != (uint32_t)c; }
};

/** TBColorManager contains a map of global color names. */

class TBColorManager
{
public:

	/** Load a list of colors from a node. */
	void Load(TBNode *n, TBSkin *skin);

	/** Define a color, if not already defined. */
	bool Define(const std::string & id, TBColor color);

	/** Is the color defined? */
	bool IsDefined(const std::string & id) { return 0 != _id2color.count(id); }

	/** Is the color defined? */
	bool IsDefined(const TBColor & color) { return 0 != _color2id.count(color); }

	/** (Re)Define a color, no matter what. */
	void ReDefine(const std::string & id, TBColor color);

	/** Clear the list of colors. */
	void Clear();

	/** Return the color with the given id.
	 * If there is no color with that id, 0 will be returned.
	 */
	TBColor GetColor(const std::string &id) const;

	/** Return the id of the given color, or 0.
	 * If there is no color with that id, 0 will be returned.
	 */
	std::string GetColorID(const TBColor & color) const;

	/** Return the id of the given color, or 0.
	 * If there is no color with that id, 0 will be returned.
	 */
	const std::map<std::string, TBColor> GetColorMap() const { return _id2color; }

	/** Dump the current color map */
	void Dump(const char * filename);

private:
	std::map<std::string, TBColor> _id2color;
	std::map<TBColor, std::string> _color2id;
};

} // namespace tb

#endif // TB_COLOR_H
