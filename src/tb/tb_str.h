// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#ifndef TB_STR_H
#define TB_STR_H

#include "tb_types.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <utility>
#include <string>

namespace tb {

/** Use as parameter for string length if you know the string is null terminated.
	Can be used in functions that support it. */
#define TB_ALL_TO_TERMINATION 2147483647

/** Some useful C-like functions that's missing in the standard. */
const char *stristr(const char *arg1, const char *arg2);

/** Simple string class that doesn't own or change the string pointer. */

class TBStrC
{
protected:
	char *s;
public:
	TBStrC(const char *str) : s(const_cast<char *>(str)) {}

	inline int Length() const							{ return (int)strlen(s); }
	inline bool IsEmpty() const							{ return s[0] == 0; }

	inline int Compare(const char* str) const			{ return strcmp(s, str); }
	inline bool Equals(const char* str) const			{ return !strcmp(s, str); }
	const char *CStr() const							{ return s; }

	inline const char & operator[](int n) const			{ assert(n >= 0); assert(n <= Length()); return s[n]; }
	explicit inline operator const char *() const		{ return s; }
	bool operator ==(const char *b) const				{ return strcmp(s, b) == 0; }
	bool operator ==(const TBStrC &b) const				{ return strcmp(s, b.s) == 0; }
	bool operator !=(const TBStrC &b) const				{ return strcmp(s, b.s) != 0; }
	bool operator <(const TBStrC &b) const				{ return strcmp(s, b.s) < 0; }

	bool operator ==(const std::string &b) const		{ return s == b; }
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
	TBStr(TBStr && str);
	TBStr(const char* str);
	TBStr(const char* str, int len);

	bool Set(TBStr str) { *this = str; return true; }
	bool SetFormatted(const char* format, ...)
		__attribute__ ((format (printf, 2, 3)));

	void Clear();

	void Remove(int ofs, int len);

	/// Insert a c-string to this TBStr at 'ofs'
	bool Insert(int ofs, const char *ins, int ins_len = TB_ALL_TO_TERMINATION);

	/// Append a c-string to this TBStr.
	bool Append(const char *ins, int ins_len = TB_ALL_TO_TERMINATION) {
		return Insert((int)strlen(s), ins, ins_len);
	}

	/// Append another TBStr to this string.
	bool Append(const TBStr & str) {
		return Insert((int)strlen(s), (const char *)str, str.Length());
	}

	/// Get the c-string raw (char *)
	char *CStr() const									{ return s; }

	/// Call atof() on the string.
	double atof() const									{ return ::atof(s); }

	/// Call atoi() on the string.
	int atoi() const									{ return ::atoi(s); }

	/// Call atol() on the string.
	long atol() const									{ return ::atol(s); }

	/// Cast to a writeable (char *) byob
	explicit inline operator char *() const				{ return s; }

	/// Cast to a bool - answers whether this TBStr is truly empty: s
	/// == empty.  This is different from IsEmpty() because you can
	/// still do a Set("") which will allocate a single byte for the
	/// string terminator.
	explicit operator bool() const;

	/// Assignment operator
	TBStr & operator = (TBStr str)						{ std::swap(s, str.s); return *this; }

	/// Pointer dereference, returns a reference to the zeroth char of
	/// the string (or the terminator of an empty string).
	char & operator *()									{ return s[0]; }

	/// Const pointer dereference - returns a const reference to the
	/// zeroth character (or terminator of an empty string).
	const char & operator *() const						{ return s[0]; }

	using TBStrC::operator ==;

	friend class TBValue;
};

} // namespace tb

#endif // TB_STR_H
