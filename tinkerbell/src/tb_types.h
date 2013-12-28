// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2014, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_TYPES_H
#define TB_TYPES_H

#include <string.h>

namespace tinkerbell {

#ifndef nullptr
#define nullptr NULL
#endif

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;

template <class T>
T Max(const T& left, const T& right) { return left > right ? left : right; }

template <class T>
T Min(const T& left, const T& right) { return left < right ? left : right; }

template <class T>
T Abs(const T& value) { return value < 0 ? -value : value; }

template <class T>
T Clamp(const T& value, const T& min, const T& max)
	{ return (value > max) ? max : ((value < min) ? min : value); }

#ifndef MAX
/** This is deprecated! Use Max(a, b)! */
#define MAX(a, b) Max(a, b)
#endif

#ifndef MIN
/** This is deprecated! Use Min(a, b)! */
#define MIN(a, b) Min(a, b)
#endif

#ifndef ABS
/** This is deprecated! Use Abs(value)! */
#define ABS(value) Abs(value)
#endif

#ifndef CLAMP
/** This is deprecated! Use Clamp(value, min, max)! */
#define CLAMP(value, min, max) Clamp(value, min, max)
#endif

/** Makes it possible to use the given enum types as flag combinations.
	That will catch use of incorrect type during compilation, that wouldn't be caught
	using a uint32 flag. */
#define MAKE_ENUM_FLAG_COMBO(Enum) \
	inline Enum operator | (Enum a, Enum b)  { return static_cast<Enum>(static_cast<uint32>(a) | static_cast<uint32>(b)); } \
	inline Enum operator & (Enum a, Enum b)  { return static_cast<Enum>(static_cast<uint32>(a) & static_cast<uint32>(b)); } \
	inline Enum operator ^ (Enum a, Enum b)  { return static_cast<Enum>(static_cast<uint32>(a) ^ static_cast<uint32>(b)); } \
	inline void operator |= (Enum &a, Enum b) { a = static_cast<Enum>(static_cast<uint32>(a) | static_cast<uint32>(b)); } \
	inline void operator &= (Enum &a, Enum b) { a = static_cast<Enum>(static_cast<uint32>(a) & static_cast<uint32>(b)); } \
	inline void operator ^= (Enum &a, Enum b) { a = static_cast<Enum>(static_cast<uint32>(a) ^ static_cast<uint32>(b)); } \
	inline Enum operator ~ (Enum a)  { return static_cast<Enum>(~static_cast<uint32>(a)); }

}; // namespace tinkerbell

#endif // TB_TYPES_H
