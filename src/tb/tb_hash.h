// ================================================================================
// ==      This file is a part of Turbo Badger. (C) 2011-2014, Emil Segerås      ==
// ==                     See tb_core.h for more information.                    ==
// ================================================================================

#ifndef TB_HASH_H
#define TB_HASH_H

#include "tb_types.h"

namespace tb {

// On C++ compilers that support it, use const expr for hash so that
// TBID comparisions turn into simple uint32_t comparisions compiletime.
// Disabled for TB_RUNTIME_DEBUG_INFO builds, so TBID string debugging
// is available.
//
// FNV constants
static constexpr uint32_t basis = 2166136261U;
static constexpr uint32_t prime = 16777619U;

// compile-time hash helper function
constexpr uint32_t TBGetHash_one(char c, const char* remain, uint32_t value)
{
	return c == 0 ? value : TBGetHash_one(remain[0], remain + 1, (value ^ c) * prime);
}

// compile-time hash
constexpr uint32_t TBGetHash(const char* str)
{
	return (str && *str) ? TBGetHash_one(str[0], str + 1, basis) : 0;
}

#define TBIDC(str) tb::TBGetHash(str)

} // namespace tb

#endif // TB_HASH_H

