// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_tempbuffer.h"
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

namespace tinkerbell {

static char *p_realloc(char *buf, size_t size) { return (char *) realloc(buf, size); }
static void p_free(char *buf) { free(buf); }

TBTempBuffer::TBTempBuffer()
	: m_data(0)
	, m_data_size(0)
	, m_append_pos(0)
{
}

TBTempBuffer::~TBTempBuffer()
{
	p_free(m_data);
}

void TBTempBuffer::SetAppendPos(int append_pos)
{
	assert(append_pos >= 0 && append_pos <= m_data_size);
	m_append_pos = append_pos;
}

bool TBTempBuffer::Reserve(int size)
{
	if (size > m_data_size)
	{
		char *new_data = p_realloc(m_data, size);
		if (!new_data)
			return false;
		m_data = new_data;
		m_data_size = size;
	}
	return true;
}

bool TBTempBuffer::Append(const char *data, int size)
{
	int needed_size = m_append_pos + size;
	if (needed_size > m_data_size)
	{
		// Reserve some extra memory to reduce the reserve calls.
		needed_size = needed_size + size + 32;
		if (!Reserve(needed_size))
			return false;
	}
	memcpy(m_data + m_append_pos, data, size);
	m_append_pos += size;
	return true;
}

bool TBTempBuffer::AppendString(const char *str)
{
	return Append(str, strlen(str));
}

}; // namespace tinkerbell
