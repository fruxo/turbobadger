// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2012, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "parser/TBParser.h"
#include "tb_tempbuffer.h"
#include <assert.h>

namespace tinkerbell {

// == Util functions ====================================================================

void UnescapeString(char *str)
{
	char *dst = str, *src = str;
	while (*src)
	{
		if (*src == '\\')
		{
			bool code_found = true;
			if (src[1] == 'n')
				*dst = '\n';
			else if (src[1] == 'r')
				*dst = '\r';
			else if (src[1] == 't')
				*dst = '\t';
			else if (src[1] == '\"')
				*dst = '\"';
			else if (src[1] == '\'')
				*dst = '\'';
			else if (src[1] == '\\')
				*dst = '\\';
			else
				code_found = false;
			if (code_found)
			{
				src += 2;
				dst++;
				continue;
			}
		}
		*dst = *src;
		dst++;
		src++;
	}
	*dst = 0;
}

bool is_white_space(const char *str)
{
	switch (*str)
	{
	case ' ':
	case '\t':
		return true;
	default:
		return false;
	}
}

bool is_pending_multiline(const char *str)
{
	while (is_white_space(str))
		str++;
	return str[0] == '\\' && str[1] == 0;
}

// == Parser ============================================================================

Parser::STATUS Parser::Read(ParserStream *stream, ParserTarget *target)
{
	TBTempBuffer line, work;
	if (!line.Reserve(1024) || !work.Reserve(1024))
		return STATUS_NO_MEMORY;

	current_indent = 0;
	current_line_nr = 1;
	pending_multiline = false;
	multi_line_sub_level = 0;

	while (int read_len = stream->GetMoreData((char *)work.GetData(), work.GetCapacity()))
	{
		char *buf = work.GetData();

		// Skip BOM (BYTE ORDER MARK) character, often in the beginning of UTF-8 documents.
		if (current_line_nr == 1 && read_len > 3 &&
			(uint8)buf[0] == 239 &&
			(uint8)buf[1] == 187 &&
			(uint8)buf[2] == 191)
		{
			read_len -= 3;
			buf += 3;
		}

		int line_pos = 0;
		while (true)
		{
			// Find line end
			int line_start = line_pos;
			while (line_pos < read_len && buf[line_pos] != '\n')
				line_pos++;

			if (line_pos < read_len)
			{
				// We have a line
				// Skip preceding \r (if we have one)
				int line_len = line_pos - line_start;
				if (!line.Append(buf + line_start, line_len))
					return STATUS_NO_MEMORY;

				// Strip away trailing '\r' if the line has it
				char *linebuf = line.GetData();
				int linebuf_len = line.GetAppendPos();
				if (linebuf_len > 0 && linebuf[linebuf_len - 1] == '\r')
					linebuf[linebuf_len - 1] = 0;

				// Terminate the line string
				if (!line.Append("", 1))
					return STATUS_NO_MEMORY;

				// Handle line
				OnLine(line.GetData(), target);
				current_line_nr++;

				line.ResetAppendPos();
				line_pos++; // Skip this \n
				// Find next line
				continue;
			}
			// No more lines here so push the rest and break for more data
			if (!line.Append(buf + line_start, read_len - line_start))
				return STATUS_NO_MEMORY;
			break;
		}
	}
	if (line.GetAppendPos())
	{
		if (!line.Append("", 1))
			return STATUS_NO_MEMORY;
		OnLine(line.GetData(), target);
		current_line_nr++;
	}
	return STATUS_OK;
}

void Parser::OnLine(char *line, ParserTarget *target)
{
	if (*line == '#')
	{
		target->OnComment(line + 1);
		return;
	}
	if (pending_multiline)
	{
		OnMultiline(line, target);
		return;
	}

	// Check indent
	int indent = 0;
	while (line[indent] == '\t' && line[indent] != 0)
		indent++;
	line += indent;

	if (indent - current_indent > 1)
	{
		target->OnError(current_line_nr, "Indentation error. (Line skipped)");
		return;
	}

	if (indent > current_indent)
	{
		// FIX: om den är mer än 1 högre är det indentation error!
		assert(indent - current_indent == 1);
		target->Enter();
		current_indent++;
	}
	else if (indent < current_indent)
	{
		while (indent < current_indent)
		{
			target->Leave();
			current_indent--;
		}
	}

	if (*line == 0)
		return;
	else
	{
		char *token = line;
		// Read line while consuming it and copy over to token buf
		while (!is_white_space(line) && *line != 0)
			line++;
		int token_len = line - token;
		// Consume any white space after the token
		while (is_white_space(line))
			line++;

		bool is_compact_line = token_len && token[token_len - 1] == ':';

		TBValue value;
		if (is_compact_line)
		{
			token_len--;
			token[token_len] = 0;

			// Check if the first argument is not a child but the value for this token
			if (is_number(line) || *line == '[' || *line == '\"' || *line == '\'' || *line == '@')
			{
				ConsumeValue(value, line);

				if (pending_multiline)
				{
					// The value wrapped to the next line, so we should remember the token and continue.
					multi_line_token.Set(token);
					return;
				}
			}
		}
		else if (token[token_len])
		{
			token[token_len] = 0;
			UnescapeString(line);
			value.SetFromStringAuto(line, TBValue::SET_AS_STATIC);
		}
		target->OnToken(token, value);

		if (is_compact_line)
			OnCompactLine(line, target);
	}
}

/** Check if buf is pointing at a end quote. It may need to iterate
	buf backwards toward buf_start to check if any preceding backslashes
	make it a escaped quote (which should not be the end quote) */
bool IsEndQuote(const char *buf_start, const char *buf, const char quote_type)
{
	if (*buf != quote_type)
		return false;
	int num_backslashes = 0;
	while (buf_start < buf && *(buf-- - 1) == '\\')
		num_backslashes++;
	return !(num_backslashes & 1);
}

void Parser::OnCompactLine(char *line, ParserTarget *target)
{
	target->Enter();
	while (*line)
	{
		// consume any whitespace
		while (is_white_space(line))
			line++;

		// Find token
		char *token = line;
		while (*line != ':' && *line != 0)
			line++;
		if (!*line)
			break; // Syntax error, expected token
		*line++ = 0;

		// consume any whitespace
		while (is_white_space(line))
			line++;

		TBValue v;
		ConsumeValue(v, line);

		if (pending_multiline)
		{
			// The value wrapped to the next line, so we should remember the token and continue.
			multi_line_token.Set(token);
			// Since we need to call target->Leave when the multiline is ready, set multi_line_sub_level.
			multi_line_sub_level = 1;
			return;
		}

		// Ready
		target->OnToken(token, v);
	}

	target->Leave();
}

void Parser::OnMultiline(char *line, ParserTarget *target)
{
	// consume any whitespace
	while (is_white_space(line))
		line++;

	TBValue value;
	ConsumeValue(value, line);

	if (!pending_multiline)
	{
		// Ready with all lines
		value.SetString(multi_line_value.GetData(), TBValue::SET_AS_STATIC);
		target->OnToken(multi_line_token, value);

		if (multi_line_sub_level)
			target->Leave();

		// Reset
		multi_line_value.SetAppendPos(0);
		multi_line_sub_level = 0;
	}
}

void Parser::ConsumeValue(TBValue &dst_value, char *&line)
{
	// Find value (As quoted string, or as auto)
	char *value = line;
	if (*line == '\"' || *line == '\'')
	{
		const char quote_type = *line;
		// Consume starting quote
		line++;
		value++;
		// Find ending quote or end
		while (!IsEndQuote(value, line, quote_type) && *line != 0)
			line++;
		// Terminate away the quote
		if (*line == quote_type)
			*line++ = 0;

		// consume any whitespace
		while (is_white_space(line))
			line++;
		// consume any comma
		if (*line == ',')
			line++;

		UnescapeString(value);
		dst_value.SetString(value, TBValue::SET_AS_STATIC);
	}
	else
	{
		// Find next comma or end
		while (*line != ',' && *line != 0)
			line++;
		// Terminate away the comma
		if (*line == ',')
			*line++ = 0;

		UnescapeString(value);
		dst_value.SetFromStringAuto(value, TBValue::SET_AS_STATIC);
	}

	// Check if we still have pending value data on the following line and set pending_multiline.
	bool continuing_multiline = pending_multiline;
	pending_multiline = is_pending_multiline(line);

	// Append the multi line value to the buffer.
	if (continuing_multiline || pending_multiline)
		multi_line_value.AppendString(dst_value.GetString());
}

}; // namespace tinkerbell
