/** @mainpage TinkerBell UI toolkit

TinkerBell UI toolkit
Copyright (C) 2011-2013 Emil Segerås

License:

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#ifndef TINKERBELL_H
#define TINKERBELL_H

#include <string.h>
#include "tb_config.h"
#include "tb_hash.h"
#include "tb_debug.h"

namespace tinkerbell {

/** Use as parameter for string length if you know the string is null terminated.
	Can be used in functions that support it. */
#define TB_ALL_TO_TERMINATION 2147483647

/** Simple point class. */

class TBPoint
{
public:
	int x, y;
	TBPoint() : x(0), y(0) {}
	TBPoint(int x, int y) : x(x), y(y) {}
};

/** Simple rectangle class. */

class TBRect
{
public:
	int x, y, w, h;
	TBRect() : x(0), y(0), w(0), h(0) {}
	TBRect(int x, int y, int w, int h) : x(x), y(y), w(w), h(h) {}

	inline bool IsEmpty() const						{ return w <= 0 || h <= 0; }
	inline bool IsInsideOut() const					{ return w < 0 || h < 0; }
	inline bool Equals(const TBRect &rect) const	{ return rect.x == x && rect.y == y && rect.w == w && rect.h == h; }
	bool Intersects(const TBRect &rect) const;

	inline void Reset()								{ x = y = w = h = 0; }
	inline void Set(int x, int y, int w, int h)		{ this->x = x; this->y = y; this->w = w; this->h = h; }

	inline TBRect Shrink(int left, int top, int right, int bottom) const	{ return TBRect(x + left, y + top, w - left - right, h - top - bottom); }
	inline TBRect Expand(int left, int top, int right, int bottom) const	{ return Shrink(-left, -top, -right, -bottom); }
	inline TBRect Shrink(int tx, int ty) const		{ return TBRect(x + tx, y + ty, w - tx * 2, h - ty * 2); }
	inline TBRect Expand(int tx, int ty) const		{ return Shrink(-tx, -ty); }
	inline TBRect Offset(int dx, int dy) const		{ return TBRect(x + dx, y + dy, w, h); }

	TBRect Union(const TBRect &rect) const;
	TBRect Clip(const TBRect &clip_rect) const;
};

/** Some useful C-like functions that's missing in the standard. */
const char *stristr(const char *arg1, const char *arg2);

/** Simple string class that doesn't own or change the string pointer. */

class TBStrC
{
protected:
	char *s;
public:
	TBStrC(const char *str) : s(const_cast<char *>(str)) {}

	inline int Length() const							{ return strlen(s); }
	inline bool IsEmpty() const							{ return s[0] == 0; }

	inline int Compare(const char* str) const			{ return strcmp(s, str); }
	inline bool Equals(const char* str) const			{ return !strcmp(s, str); }

	inline char operator[](int n) const					{ return s[n]; }
	inline operator const char *() const				{ return s; }
	const char *CStr() const							{ return s; }
};

/** TBStr is a simple string class.
	It's a compact wrapper for a char array, and doesn't do any storage magic to
	avoid buffer copying or remember its length. It is intended as "final storage"
	of strings since its buffer is compact.

	Serious work on strings is better done using TBTempBuffer and then set on a TBStr for
	final storage (since TBTempBuffer is optimized for speed rather than being compact).

	It is guaranteed to have a valid pointer at all times. If uninitialized, emptied or on
	out of memory, its storage will be a empty ("") const string.
*/

class TBStr : public TBStrC
{
public:
	~TBStr();
	TBStr();
	TBStr(const TBStr &str);
	TBStr(const char* str);
	TBStr(const char* str, int len);

	bool Set(const char* str, int len = TB_ALL_TO_TERMINATION);
	bool SetFormatted(const char* format, ...);

	void Clear();

	void Remove(int ofs, int len);
	bool Insert(int ofs, const char *ins, int ins_len = TB_ALL_TO_TERMINATION);
	bool Append(const char *ins, int ins_len = TB_ALL_TO_TERMINATION)	{ return Insert(strlen(s), ins, ins_len); }

	inline operator char *() const						{ return s; }
	char *CStr() const									{ return s; }
	const TBStr& operator = (const TBStr &str)			{ Set(str); return *this; }
};

/** TBID is a wrapper for a uint32 to be used as ID.
	The uint32 can be set directly to any uint32, or it can be
	set from a string which will be hashed into the uint32. */
class TBID
{
public:
	TBID(uint32 id = 0)				{ Set(id); }
	TBID(const char *string)		{ Set(string); }
	TBID(const TBID &id)			{ Set(id); }

#ifdef _DEBUG
	void Set(uint32 newid);
	void Set(const TBID &newid);
	void Set(const char *string);
#else
	void Set(uint32 newid)			{ id = newid; }
	void Set(const TBID &newid)		{ id = newid; }
	void Set(const char *string)	{ id = TBGetHash(string); }
#endif

	operator uint32 () const		{ return id; }
	const TBID& operator = (const TBID &id) { Set(id); return *this; }
private:
	uint32 id;
	/** This string is here to aid debugging (Only in debug builds!)
		It should not to be used in your code! */
#ifdef _DEBUG
	friend class TBLanguage;
	TBStr debug_string;
#endif
};

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

	operator uint32 () const		{ return *((uint32*)&b); }
	bool operator == (const TBColor &c) const { return *this == (uint32)c; }
	bool operator != (const TBColor &c) const { return !(*this == c); }
};

/** TBFontDescription describes a font.
	By default when nothing is set, the font is unspecified and means it should be inherited
	from a parent widget that specifies a font, or use the default font if no parent does. */

class TBFontDescription
{
public:
	/** Set the font ID of the font to use.
		This ID maps to the font names in TBFontInfo, which is managed from
		TBFontManager::AddFontInfo, TBFontManager::GetFontInfo.

		Example:
		If a font was added to the font manager with the name "Vera", you can
		do font_description.SetID(TBIDC("Vera")).
		*/
	void SetID(const TBID &id)											{ m_id = id; }

	/** Get the TBID for the font name (See SetID). */
	TBID GetID() const { return m_id; }

	/** Get the TBID for the TBFontFace that matches this font description.
		This is a ID combining both the font file, and variation (such as size and style),
		and should be used to identify a certain font face.

		If this is 0, the font description is unspecified. For a widget, that means that the font
		should be inherited from the parent widget. */
	TBID GetFontFaceID() const { return m_id + m_packed_init; }

	void SetSize(uint32 size)											{ m_packed.size = MIN(size, 0x8000); }
	uint32 GetSize() const												{ return m_packed.size; }

	//not connected to anything yet
	//void SetBold(bool bold)											{ m_packed.bold = bold; }
	//bool GetBold() const												{ return m_packed.bold; }

	//not connected to anything yet
	//void SetItalic(bool italic)										{ m_packed.italic = italic; }
	//bool GetItalic() const											{ return m_packed.italic; }

	TBFontDescription() : m_packed_init(0) {}
	TBFontDescription(const TBFontDescription &src)						{ m_packed_init = src.m_packed_init; m_id = src.m_id; }
	const TBFontDescription& operator = (const TBFontDescription &src)	{ m_packed_init = src.m_packed_init; m_id = src.m_id; return *this; }
	bool operator == (const TBFontDescription &fd) const { return m_packed_init == fd.m_packed_init && m_id == fd.m_id; }
	bool operator != (const TBFontDescription &fd) const { return !(*this == fd); }
private:
	TBID m_id;
	union {
		struct {
			uint32 size : 15;
			uint32 italic : 1;
			uint32 bold : 1;
		} m_packed;
		uint32 m_packed_init;
	};
};

/** Dimensions <= this value will be untouched by conversion in TBDimensionConverter.
	To preserve special constants, those must be <= this value. */
#define TB_INVALID_DIMENSION -5555

class TBTempBuffer;
class TBValue;

/** TBDimensionConverter converts device independant points
	to pixels, based on two DPI values.
	Dimensions tinkerbell are normally in pixels (if not specified differently)
	and conversion normally take place when loading skin. */
class TBDimensionConverter
{
	int m_src_dpi; ///< The source DPI (Normally the base_dpi from skin).
	int m_dst_dpi; ///< The destination DPI (Normally the supported skin DPI nearest to TBSystem::GetDPI).
	TBStr m_dst_dpi_str; ///< The file suffix that should be used to load bitmaps in destinatin DPI.
public:
	TBDimensionConverter() : m_src_dpi(100), m_dst_dpi(100) {}

	/** Set the source and destination DPI that will affect the conversion. */
	void SetDPI(int src_dpi, int dst_dpi);

	/** Get the source DPI. */
	int GetSrcDPI() const { return m_src_dpi; }

	/** Get the destination DPI. */
	int GetDstDPI() const { return m_dst_dpi; }

	/** Get the file name suffix that should be used to load bitmaps in the destination DPI.
		Examples: "@96", "@196" */
	const char *GetDstDPIStr() const { return m_dst_dpi_str; }

	/** Get the file name with destination DPI suffix (F.ex "foo.png" becomes "foo@192.png").
		The temp buffer will contain the resulting file name. */
	void GetDstDPIFilename(const char *filename, TBTempBuffer *tempbuf) const;

	/** Return true if the source and destinatin DPI are different. */
	bool NeedConversion() const { return m_src_dpi != m_dst_dpi; }

	/** Convert device independant point to pixel. */
	int DpToPx(int dp) const;

	/** Convert millimeter to pixel. */
	int MmToPx(int mm) const;

	/** Get a pixel value from string in any of the following formats:
		str may be nullptr. def_value is returned on fail.

		Device independent point:		"1", "1dp"
		Pixel value:					"1px"
		*/
	int GetPxFromString(const char *str, int def_value) const;

	/** Get a pixel value from TBValue.
		value may be nullptr. def_value is returned on fail.

		Number formats are treated as dp.
		String format is treated like for GetPxFromString.
		*/
	int GetPxFromValue(TBValue *value, int def_value) const;
};

class TBRenderer;
class TBSkin;
class TBWidgetsReader;
class TBLanguage;
class TBFontManager;

extern TBRenderer *g_renderer;
extern TBSkin *g_tb_skin;
extern TBWidgetsReader *g_widgets_reader;
extern TBLanguage *g_tb_lng;
extern TBFontManager *g_font_manager;

/** Initialize tinkerbell. Call this before using any tinkerbell API. */
bool init_tinkerbell(TBRenderer *renderer, const char *lng_file);

/** Shutdown tinkerbell. Call this after deleting the last widget, to free all tinkerbell data. */
void shutdown_tinkerbell();

}; // namespace tinkerbell

#endif // TINKERBELL_H
