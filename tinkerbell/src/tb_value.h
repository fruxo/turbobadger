// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2012, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_VALUE_H
#define TB_VALUE_H

#include "tinkerbell.h"
#include "tb_list.h"

namespace tinkerbell {

class TBValue;

/** Return true if the given string starts with a number. */
bool is_number(const char *str);

/** Return true if the given number string is a float number.
	Should only be called when you've verified it's a number with is_number(). */
bool is_number_float(const char *str);

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
public:
	/** The current type of the value.
		It may change when using a getter of a different type. */
	enum TYPE {
		TYPE_NULL,
		TYPE_STRING,
		TYPE_FLOAT,
		TYPE_INT,
		TYPE_ARRAY
	};

	/** How to deal with the dynamic memory when setting string and array. */
	enum SET {
		SET_NEW_COPY,			///< A new copy of the data will be made.
		SET_TAKE_OWNERSHIP,		///< The data passed in will be stored and freed.
		SET_AS_STATIC			///< The data passed in will be stored but never freed.
	};

	TBValue();
	TBValue(const TBValue &value);
	TBValue(TYPE type);

	TBValue(int value);
	TBValue(float value);
	TBValue(const char *value);

	~TBValue();

	/** Take over ownership of content of source_value.
		Note:	-If source_value has string or array that are set with SET_AS_STATIC, it will make new copies of those.
				-value will be nulled on source_value after this call. */
	void TakeOver(TBValue &source_value);
	void Copy(const TBValue &source_value);

	void SetNull();
	void SetInt(int val);
	void SetFloat(float val);

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

	TYPE GetType() const { return (TYPE) m_packed.type; }
	bool IsString() const { return m_packed.type == TYPE_STRING; }
	bool IsFloat() const { return m_packed.type == TYPE_FLOAT; }
	bool IsInt() const { return m_packed.type == TYPE_INT; }
	bool IsArray() const { return m_packed.type == TYPE_ARRAY; }
	int GetArrayLength() const { return IsArray() ? val_arr->GetLength() : 0; }

	const TBValue& operator = (const TBValue &val) { Copy(val); return *this; }
private:
	union {
		float val_float;
		int val_int;
		char *val_str;
		TBValueArray *val_arr;
	};
	union {
		struct {
			uint32 type : 8;
			uint32 allocated : 1;
		} m_packed;
		uint32 m_packed_init;
	};
};

}; // namespace tinkerbell

#endif // TB_VALUE_H
