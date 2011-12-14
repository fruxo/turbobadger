// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TBValue_H
#define TBValue_H

#include "tinkerbell.h"
#include "tb_list.h"

namespace tinkerbell {

class TBValue;

/** TBValueArray is a array of TBValue */
class TBValueArray
{
public:
	TBValueArray();
	~TBValueArray();
	TBValue *AddValue();
	TBValue *GetValue(int index);
	static TBValueArray *Clone(TBValueArray *source);
	int GetLength() const { return m_list.GetNumItems(); }
private:
	TBListAutoDeleteOf<TBValue> m_list;
};

/** TBValue is a value with a specific type. On getting the value as a different type,
	it may convert its internal representation to that type.
	It may also contain a array of attributes (TBValueArray). If you try to get a array
	as a value (instead or accessing the arrays values), it will return 0 (or "" for string). */
class TBValue
{
private:
	union {
		float val_float;
		int val_int32;
		char *val_str;
		TBValueArray *val_arr;
	};
	enum TYPE {
		TYPE_NULL,
		TYPE_STRING,
		TYPE_FLOAT,
		TYPE_INT32,
		TYPE_ARRAY
	} t;
	bool allocated;
public:
	TBValue() : t(TYPE_NULL) {}
	~TBValue();

	/** Take over ownership of content of source_value.
		Note:	-If source_value has string or array that are set with SET_AS_STATIC, it will make new copies of those.
				-value will be nulled on source_value after this call. */
	void TakeOver(TBValue &source_value);
	void Copy(const TBValue &source_value);

	void SetNull();
	void SetInt(int val);
	void SetFloat(float val);

	/** How to deal with the dynamic memory for string and array. */
	enum SET {
		SET_NEW_COPY,			///< A new copy of the data will be made.
		SET_TAKE_OWNERSHIP,		///< The data passed in will be stored and freed.
		SET_AS_STATIC			///< The data passed in will be stored but never freed.
	};

	/** Set the passed in string */
	void SetString(const char *val, SET set);

	/** Set the passed in array */
	void SetArray(TBValueArray *arr, SET set);

	/** Set the value either as a string, number or array of numbers, depending of the string syntax. */
	void SetFromStringAuto(const char *str, SET set);

	int GetInt();
	float GetFloat();
	const char *GetString();
	TBValueArray *GetArray() const { return IsArray() ? val_arr : nullptr; }

	bool IsString() const { return t == TYPE_STRING; }
	bool IsArray() const { return t == TYPE_ARRAY; }
	int GetArrayLength() const { return IsArray() ? val_arr->GetLength() : 0; }
};

};

#endif // TBValue_H
