// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TBParser_H
#define TBParser_H

#include "tb_value.h"
#include "tb_tempbuffer.h"

namespace tinkerbell {

class ParserTarget
{
public:
	virtual ~ParserTarget() {}
	virtual void OnError(int line_nr, const char *error) = 0;
	virtual void OnComment(int line_nr, const char *comment) = 0;
	virtual void OnToken(int line_nr, const char *name, TBValue &value) = 0;
	virtual void Enter() = 0;
	virtual void Leave() = 0;
};

class ParserStream
{
public:
	virtual ~ParserStream() {}
	virtual int GetMoreData(char *buf, int buf_len) = 0;
};

class Parser
{
public:
	enum STATUS {
		STATUS_OK,
		STATUS_NO_MEMORY,
		STATUS_PARSE_ERROR
	};
	Parser() {}
	STATUS Read(ParserStream *stream, ParserTarget *target);
private:
	int current_indent;
	int current_line_nr;
	TBStr multi_line_token;
	TBTempBuffer multi_line_value;
	int multi_line_sub_level;
	bool pending_multiline;
	void OnLine(char *line, ParserTarget *target);
	void OnCompactLine(char *line, ParserTarget *target);
	void OnMultiline(char *line, ParserTarget *target);
	void ConsumeValue(TBValue &dst_value, char *&line);
};

}; // namespace tinkerbell

#endif // TBParser_H
