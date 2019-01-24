// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#include "tb_str.h"
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>

namespace tb {

static const char *empty = "";
inline void safe_delete(char *&str)
{
	if (str != empty) {
		if (str)
			free(str);
		str = const_cast<char*>(empty);
	}
}

const char *stristr(const char *arg1, const char *arg2)
{
	const char *a, *b;

	for(;*arg1;arg1++)
	{
		a = arg1;
		b = arg2;
		while (toupper(*a++) == toupper(*b++))
			if (!*b)
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
	: TBStrC((str == empty || str == nullptr) ? empty : strdup(str))
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

TBStr::TBStr(TBStr && str)
	: TBStrC(str.s)
{
	str.s = const_cast<char*>(empty);
}

TBStr::TBStr(const char* str, int len)
	: TBStrC(empty)
{
	if (str == nullptr)
		s = const_cast<char*>(empty);
	else
		s = strndup(str, len);
}

TBStr::~TBStr()
{
	safe_delete(s);
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
	assert(ofs >= 0);
	assert(ofs <= len1);
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

TBStr::operator bool() const
{
	return s != empty;
}

} // namespace tb
