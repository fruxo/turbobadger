// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "parser/TBParser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#ifdef WIN32
#define strtok_r strtok_s
#endif

namespace tinkerbell {

// FIX: Floating point string conversions might be locale dependant. Force "." as decimal!

// == Helper functions ============================

bool is_number(const char *str)
{
	if (*str == '-' || *str == '.')
		str++;
	return *str >= '0' && *str <= '9';
}

bool is_number_float(const char *str)
{
	while (*str) if (*str++ == '.') return true;
	return false;
}

// == TBValueArray ==================================

TBValueArray::TBValueArray()
{
}

TBValueArray::~TBValueArray()
{
}

TBValue *TBValueArray::AddValue()
{
	TBValue *v;
	if ((v = new TBValue()) && m_list.Add(v))
		return v;
	delete v;
	return nullptr;
}

TBValue *TBValueArray::GetValue(int index)
{
	if (index >= 0 && index < m_list.GetNumItems())
		return m_list[index];
	return nullptr;
}

TBValueArray *TBValueArray::Clone(TBValueArray *source)
{
	TBValueArray *new_arr = new TBValueArray;
	if (!new_arr)
		return nullptr;
	for (int i = 0; i < source->m_list.GetNumItems(); i++)
	{
		TBValue *new_val = new_arr->AddValue();
		if (!new_val)
		{
			delete new_arr;
			return nullptr;
		}
		new_val->Copy(*source->GetValue(i));
	}
	return new_arr;
}

// == TBValue =======================================

TBValue::TBValue()
	: m_packed_init(0)
{
}

TBValue::TBValue(const TBValue &value)
	: m_packed_init(0)
{
	Copy(value);
}

TBValue::TBValue(TYPE type)
	: m_packed_init(0)
{
	switch (type)
	{
	case TYPE_NULL:
		SetNull();
		break;
	case TYPE_STRING:
		SetString("", SET_AS_STATIC);
		break;
	case TYPE_FLOAT:
		SetFloat(0);
		break;
	case TYPE_INT:
		SetInt(0);
		break;
	case TYPE_ARRAY:
		if (TBValueArray *arr = new TBValueArray())
			SetArray(arr, SET_TAKE_OWNERSHIP);
		break;
	default:
		assert(!"Not implemented!");
	};
}

TBValue::TBValue(int value)
	: m_packed_init(0)
{
	SetInt(value);
}

TBValue::TBValue(float value)
	: m_packed_init(0)
{
	SetFloat(value);
}

TBValue::TBValue(const char *value)
	: m_packed_init(0)
{
	SetString(value, SET_NEW_COPY);
}

TBValue::~TBValue()
{
	SetNull();
}

void TBValue::TakeOver(TBValue &source_value)
{
	if (source_value.m_packed.type == TYPE_STRING)
		SetString(source_value.val_str, source_value.m_packed.allocated ? SET_TAKE_OWNERSHIP : SET_NEW_COPY);
	else if (source_value.m_packed.type == TYPE_ARRAY)
		SetArray(source_value.val_arr, source_value.m_packed.allocated ? SET_TAKE_OWNERSHIP : SET_NEW_COPY);
	else
		*this = source_value;
	source_value.m_packed.type = TYPE_NULL;
}

void TBValue::Copy(const TBValue &source_value)
{
	if (source_value.m_packed.type == TYPE_STRING)
		SetString(source_value.val_str, SET_NEW_COPY);
	else if (source_value.m_packed.type == TYPE_ARRAY)
		SetArray(source_value.val_arr, SET_NEW_COPY);
	else
	{
		// Assumes we are a POD, which we should be
		memcpy(this, &source_value, sizeof(TBValue));
	}
}

void TBValue::SetNull()
{
	if (m_packed.type == TYPE_STRING && m_packed.allocated)
		free(val_str);
	else if (m_packed.type == TYPE_ARRAY && m_packed.allocated)
		delete val_arr;
	m_packed.type = TYPE_NULL;
}

void TBValue::SetInt(int val)
{
	SetNull();
	m_packed.type = TYPE_INT;
	val_int = val;
}

void TBValue::SetFloat(float val)
{
	SetNull();
	m_packed.type = TYPE_FLOAT;
	val_float = val;
}

void TBValue::SetString(const char *val, SET set)
{
	SetNull();
	m_packed.allocated = (set == SET_NEW_COPY || set == SET_TAKE_OWNERSHIP);
	if (set != SET_NEW_COPY)
	{
		val_str = const_cast<char *>(val);
		m_packed.type = TYPE_STRING;
	}
	else if (val_str = strdup(val))
		m_packed.type = TYPE_STRING;
}

void TBValue::SetArray(TBValueArray *arr, SET set)
{
	SetNull();
	m_packed.allocated = (set == SET_NEW_COPY || set == SET_TAKE_OWNERSHIP);
	if (set != SET_NEW_COPY)
	{
		val_arr = arr;
		m_packed.type = TYPE_ARRAY;
	}
	else if (val_arr = TBValueArray::Clone(arr))
		m_packed.type = TYPE_ARRAY;
}

void TBValue::SetFromStringAuto(const char *str, SET set)
{
	if (!str)
		SetNull();
	else if (is_number(str))
	{
		// If the number has spaces, we'll assume a list of numbers (example: "10 -4 3.5")
		if (strstr(str, " "))
		{
			SetNull();
			if (TBValueArray *arr = new TBValueArray)
			{
				TBStr tmpstr;
				char *s3;
				if (tmpstr.Set(str))
				{
					char * pch = strtok_r(tmpstr, ", ", &s3);
					while (pch)
					{
						if (TBValue *new_val = arr->AddValue())
							new_val->SetFromStringAuto(pch, SET_NEW_COPY);
						pch = strtok_r(NULL, ", ", &s3);
					}
				}
				SetArray(arr, SET_TAKE_OWNERSHIP);
			}
		}
		else if (is_number_float(str))
			SetFloat((float)atof(str));
		else
			SetInt(atoi(str));
	}
	else if (*str == '[')
	{
		SetNull();
		if (TBValueArray *arr = new TBValueArray)
		{
			assert(!"not implemented! Split out the tokenizer code above!");
			SetArray(arr, SET_TAKE_OWNERSHIP);
		}
	}
	else
	{
		SetString(str, set);
		return;
	}
	// We didn't set as string, so we might need to deal with the passed in string data.
	if (set == SET_TAKE_OWNERSHIP)
	{
		// Delete the passed in data
		TBValue tmp;
		tmp.SetString(str, SET_TAKE_OWNERSHIP);
	}
}

int TBValue::GetInt()
{
	if (m_packed.type == TYPE_STRING)
		SetInt(atoi(val_str));
	else if (m_packed.type == TYPE_FLOAT)
		return (int) val_float;
	else if (m_packed.type == TYPE_ARRAY)
		return 0;
	return m_packed.type == TYPE_INT ? val_int : 0;
}

float TBValue::GetFloat()
{
	if (m_packed.type == TYPE_STRING)
		SetFloat((float)atof(val_str));
	else if (m_packed.type == TYPE_INT)
		return (float) val_int;
	else if (m_packed.type == TYPE_ARRAY)
		return 0;
	return m_packed.type == TYPE_FLOAT ? val_float : 0;
}

const char *TBValue::GetString()
{
	if (m_packed.type == TYPE_INT)
	{
		char tmp[32];
		sprintf(tmp, "%d", val_int);
		SetString(tmp, SET_NEW_COPY);
	}
	else if (m_packed.type == TYPE_FLOAT)
	{
		char tmp[32];
		sprintf(tmp, "%f", val_float);
		SetString(tmp, SET_NEW_COPY);
	}
	else if (m_packed.type == TYPE_ARRAY)
		return "";
	return m_packed.type == TYPE_STRING ? val_str : "";
}

}; // namespace tinkerbell
