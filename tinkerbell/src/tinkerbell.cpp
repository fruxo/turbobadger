// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tinkerbell.h"
#include "tb_skin.h"
#include "tb_widgets_reader.h"
#include "tb_language.h"
#include "tb_font_renderer.h"
#include "tb_addon.h"
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

namespace tinkerbell {

static const char *empty = "";
inline void safe_delete(char *&str)
{
	if(str != empty && str)
		free(str);
	str = const_cast<char*>(empty);
}

// ================================================================

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

// == TBRect ======================================================

bool TBRect::Intersects(const TBRect &rect) const
{
	if (IsEmpty() || rect.IsEmpty())
		return false;
	if (x + w > rect.x && x < rect.x + rect.w &&
		y + h > rect.y && y < rect.y + rect.h)
		return true;
	return false;
}

TBRect TBRect::Union(const TBRect &rect) const
{
	assert(!IsInsideOut());
	assert(!rect.IsInsideOut());

	if (IsEmpty())
		return rect;
	if (rect.IsEmpty())
		return *this;

	int minx = MIN(x, rect.x);
	int miny = MIN(y, rect.y);
	int maxx = x + w > rect.x + rect.w ?
				x + w : rect.x + rect.w;
	int maxy = y + h > rect.y + rect.h ?
				y + h : rect.y + rect.h;
	return TBRect(minx, miny, maxx - minx, maxy - miny);
}

TBRect TBRect::Clip(const TBRect &clip_rect) const
{
	assert(!clip_rect.IsInsideOut());
	TBRect tmp;
	if (!Intersects(clip_rect))
		return tmp;
	tmp.x = MAX(x, clip_rect.x);
	tmp.y = MAX(y, clip_rect.y);
	tmp.w = MIN(x + w, clip_rect.x + clip_rect.w) - tmp.x;
	tmp.h = MIN(y + h, clip_rect.y + clip_rect.h) - tmp.y;
	return tmp;
}

// ===================================================================

const char *stristr(const char *arg1, const char *arg2)
{
	const char *a, *b;

	for(;*arg1;*arg1++)
	{
		a = arg1;
		b = arg2;
		while(toupper(*a++) == toupper(*b++))
			if(!*b)
				return arg1;
	}
	return nullptr;
}

// == TBStr ==========================================================

TBStr::TBStr()
	: TBStrC(empty)
{
}

TBStr::TBStr(const char* str)
	: TBStrC(str == empty ? empty : strdup(str))
{
	if (!s)
		s = const_cast<char*>(empty);
}

TBStr::TBStr(const TBStr &str)
	: TBStrC(str.s == empty ? empty : strdup(str.s))
{
	if (!s)
		s = const_cast<char*>(empty);
}

TBStr::TBStr(const char* str, int len)
	: TBStrC(empty)
{
	Set(str, len);
}

TBStr::~TBStr()
{
	safe_delete(s);
}

bool TBStr::Set(const char* str, int len)
{
	safe_delete(s);
	if (len == TB_ALL_TO_TERMINATION)
		len = strlen(str);
	if (char *new_s = (char *) malloc(len + 1))
	{
		s = new_s;
		memcpy(s, str, len);
		s[len] = 0;
		return true;
	}
	return false;
}

bool TBStr::SetFormatted(const char* format, ...)
{
	safe_delete(s);
	if (!format)
		return true;
	va_list ap;
	int max_len = 64;
	char *new_s = nullptr;
	while (true)
	{
		if (char *tris_try_new_s = (char *) realloc(new_s, max_len))
		{
			new_s = tris_try_new_s;

			va_start(ap, format);
			int ret = vsnprintf(new_s, max_len, format, ap);
			va_end(ap);

			if (ret > max_len) // Needed size is known (+2 for termination and avoid ambiguity)
				max_len = ret + 2;
			else if (ret == -1 || ret >= max_len - 1) // Handle some buggy vsnprintf implementations.
				max_len *= 2;
			else // Everything fit for sure
			{
				s = new_s;
				return true;
			}
		}
		else
		{
			// Out of memory
			free(new_s);
			break;
		}
	}
	return false;
}

void TBStr::Clear()
{
	safe_delete(s);
}

void TBStr::Remove(int ofs, int len)
{
	assert(ofs >= 0 && (ofs + len <= (int)strlen(s)));
	if (!len)
		return;
	char *dst = s + ofs;
	char *src = s + ofs + len;
	while (*src != 0)
		*(dst++) = *(src++);
	*dst = *src;
}

bool TBStr::Insert(int ofs, const char *ins, int ins_len)
{
	int len1 = strlen(s);
	if (ins_len == TB_ALL_TO_TERMINATION)
		ins_len = strlen(ins);
	int newlen = len1 + ins_len;
	if (char *news = (char *) malloc(newlen + 1))
	{
		memcpy(&news[0], s, ofs);
		memcpy(&news[ofs], ins, ins_len);
		memcpy(&news[ofs + ins_len], &s[ofs], len1 - ofs);
		news[newlen] = 0;
		safe_delete(s);
		s = news;
		return true;
	}
	return false;
}

// == TBID ==================================================================================================

#ifdef _DEBUG

// Hash table for checking if we get any collisions (same hash value for TBID's created
// from different strings)
static TBHashTableAutoDeleteOf<TBID> all_id_hash;
static bool is_adding = false;

void TBID::Set(uint32 newid)
{
	id = newid;
	debug_string.Clear();
	if (!is_adding)
	{
		if (TBID *other_id = all_id_hash.Get(id))
		{
			assert(other_id->debug_string.IsEmpty());
		}
		else
		{
			is_adding = true;
			all_id_hash.Add(id, new TBID(*this));
			is_adding = false;
		}
	}
}

void TBID::Set(const TBID &newid)
{
	id = newid;
	TB_IF_DEBUG(debug_string.Set(newid.debug_string));
	if (!is_adding)
	{
		if (TBID *other_id = all_id_hash.Get(id))
		{
			// If this happens, 2 different strings result in the same hash.
			// It might be a good idea to change one of them, but it might not matter.
			assert(other_id->debug_string.Equals(debug_string));
		}
		else
		{
			is_adding = true;
			all_id_hash.Add(id, new TBID(*this));
			is_adding = false;
		}
	}
}

void TBID::Set(const char *string)
{
	id = TBGetHash(string);
	TB_IF_DEBUG(debug_string.Set(string));
	if (!is_adding)
	{
		if (TBID *other_id = all_id_hash.Get(id))
		{
			assert(other_id->debug_string.Equals(debug_string));
		}
		else
		{
			is_adding = true;
			all_id_hash.Add(id, new TBID(*this));
			is_adding = false;
		}
	}
}

#endif // _DEBUG

// == TBColor ===============================================================================================

void TBColor::SetFromString(const char *str, int len)
{
	int r, g, b, a;
	if (len == 9 && sscanf(str, "#%2x%2x%2x%2x", &r, &g, &b, &a) == 4)			// rrggbbaa
		Set(TBColor(r, g, b, a));
	else if (len == 7 && sscanf(str, "#%2x%2x%2x", &r, &g, &b) == 3)			// rrggbb
		Set(TBColor(r, g, b));
	else if (len == 5 && sscanf(str, "#%1x%1x%1x%1x", &r, &g, &b, &a) == 4)		// rgba
		Set(TBColor(r + (r << 4), g + (g << 4), b + (b << 4), a + (a << 4)));
	else if (len == 4 && sscanf(str, "#%1x%1x%1x", &r, &g, &b) == 3)			// rgb
		Set(TBColor(r + (r << 4), g + (g << 4), b + (b << 4)));
	else
		Set(TBColor());
}

// == TBDimensionConverter ==================================================================================

void TBDimensionConverter::SetDPI(int src_dpi, int dst_dpi)
{
	m_src_dpi = src_dpi;
	m_dst_dpi = dst_dpi;
	m_dst_dpi_str.Clear();
	if (NeedConversion())
		m_dst_dpi_str.SetFormatted("@%d", m_dst_dpi);
}

void TBDimensionConverter::GetDstDPIFilename(const char *filename, TBTempBuffer *tempbuf) const
{
	int dot_pos = 0;
	for (dot_pos = strlen(filename) - 1; dot_pos > 0; dot_pos--)
		if (filename[dot_pos] == '.')
			break;
	tempbuf->ResetAppendPos();
	tempbuf->Append(filename, dot_pos);
	tempbuf->AppendString(GetDstDPIStr());
	tempbuf->AppendString(filename + dot_pos);
}

int TBDimensionConverter::DpToPx(int dp) const
{
	if (dp <= TB_INVALID_DIMENSION || dp == 0 || !NeedConversion())
		return dp;
	if (dp > 0)
	{
		dp = dp * m_dst_dpi / m_src_dpi;
		return MAX(dp, 1);
	}
	else
	{
		dp = dp * m_dst_dpi / m_src_dpi;
		return MIN(dp, -1);
	}
}

int TBDimensionConverter::GetPxFromString(const char *str, int def_value) const
{
	if (!str || !isdigit(*str))
		return def_value;
	int len = strlen(str);
	int val = atoi(str);
	// "dp" and unspecified unit is dp.
	if ((len > 0 && isdigit(str[len - 1])) ||
		(len > 2 && strcmp(str + len - 2, "dp") == 0))
		return DpToPx(val);
	else
		return val;
}

int TBDimensionConverter::GetPxFromValue(TBValue *value, int def_value) const
{
	if (!value)
		return def_value;
	if (value->GetType() == TBValue::TYPE_INT)
		return DpToPx(value->GetInt());
	else if (value->GetType() == TBValue::TYPE_FLOAT)
		// FIX: We might want float versions of all dimension functions.
		return DpToPx((int)value->GetFloat());
	return GetPxFromString(value->GetString(), def_value);
}

}; // namespace tinkerbell
