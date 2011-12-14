// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_TEMP_BUFFER_H
#define TB_TEMP_BUFFER_H

namespace tinkerbell {

/** TBTempBuffer manages a buffer that will be deleted on destruction.

	The buffer size can grow by calling Reserve or Append, but it
	will never shrink during the lifetime of the object.
*/
class TBTempBuffer
{
public:
	TBTempBuffer();
	~TBTempBuffer();

	/** Make sure the buffer has at least size bytes.
		Returns false on OOM. */
	bool Reserve(int size);

	/** Get a pointer to the buffer data. */
	char *GetData() { return m_data; }

	/** Return the size of the buffer in bytes. */
	int GetCapacity() { return m_data_size; }

	/** Append data with size bytes at the end of the buffer and
		increase the append position with the same amount.
		Returns false on OOM. */
	bool Append(const char *data, int size);

	/** Set the position (in bytes) in the buffer where Append should write. */
	void SetAppendPos(int append_pos);

	/** Reset the append position to 0. */
	void ResetAppendPos() { m_append_pos = 0; }

	/** Return the current append position in in bytes. */
	int GetAppendPos() { return m_append_pos; }
private:
	char *m_data;
	int m_data_size;
	int m_append_pos;
};

}; // namespace tinkerbell

#endif // TB_TEMP_BUFFER_H
