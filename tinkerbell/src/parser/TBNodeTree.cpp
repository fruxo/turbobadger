// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "parser/TBNodeTree.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tb_system.h"
#include "tb_tempbuffer.h"

namespace tinkerbell {

TBNode::~TBNode()
{
	Clear();
}

// static
TBNode *TBNode::Create(const char *name)
{
	TBNode *n = new TBNode;
	if (!n || !(n->m_name = strdup(name)))
	{
		delete n;
		return nullptr;
	}
	return n;
}

// static
TBNode *TBNode::Create(const char *name, int name_len)
{
	TBNode *n = new TBNode;
	if (!n || !(n->m_name = (char *) malloc(name_len + 1)))
	{
		delete n;
		return nullptr;
	}
	memcpy(n->m_name, name, name_len);
	n->m_name[name_len] = 0;
	return n;
}

//static
const char *TBNode::GetNextNodeSeparator(const char *request)
{
	while (*request != 0 && *request != '>')
		request++;
	return request;
}

TBNode *TBNode::GetNode(const char *request, GET_MISS_POLICY mp)
{
	// Iterate one node deeper for each sub request (non recursive)
	TBNode *n = this;
	while (*request && n)
	{
		const char *nextend = GetNextNodeSeparator(request);
		int name_len = nextend - request;
		TBNode *n_child = n->GetNode(request, name_len);
		if (!n_child && mp == GET_MISS_POLICY_CREATE)
		{
			n_child = n->Create(request, name_len);
			if (n_child)
				n->Add(n_child);
		}
		n = n_child;
		request = *nextend == 0 ? nextend : nextend + 1;
	}
	return n;
}

TBNode *TBNode::GetNode(const char *name, int name_len) const
{
	for (TBNode *n = GetFirstChild(); n; n = n->GetNext())
	{
		if (strncmp(n->m_name, name, name_len) == 0 && n->m_name[name_len] == 0)
			return n;
	}
	return nullptr;
}

bool TBNode::CloneChildren(TBNode *source)
{
	TBNode *item = source->GetFirstChild();
	while(item)
	{
		TBNode *new_child = Create(item->m_name);
		if (!new_child)
			return false;

		new_child->m_value.Copy(item->m_value);
		Add(new_child);

		if (!new_child->CloneChildren(item))
			return false;
		item = item->GetNext();
	}
	return true;
}

int TBNode::GetValueInt(const char *request, int def)
{
	TBNode *n = GetNode(request);
	return n ? n->m_value.GetInt() : def;
}

float TBNode::GetValueFloat(const char *request, float def)
{
	TBNode *n = GetNode(request);
	return n ? n->m_value.GetFloat() : def;
}

const char *TBNode::GetValueString(const char *request, const char *def)
{
	TBNode *n = GetNode(request);
	return n ? n->m_value.GetString() : def;
}

class FileParser : public ParserStream
{
public:
	bool Read(const char *filename, ParserTarget *target)
	{
		f = TBFile::Open(filename, TBFile::MODE_READ);
		if (!f)
			return false;
		Parser p;
		Parser::STATUS status = p.Read(this, target);
		delete f;
		return status == Parser::STATUS_OK ? true : false;
	}
	virtual int GetMoreData(char *buf, int buf_len)
	{
		return f->Read(buf, 1, buf_len);
	}
private:
	TBFile *f;
};

class DataParser : public ParserStream
{
public:
	bool Read(const char *data, int data_len, ParserTarget *target)
	{
		m_data = data;
		m_data_len = data_len;
		Parser p;
		Parser::STATUS status = p.Read(this, target);
		return true;
	}
	virtual int GetMoreData(char *buf, int buf_len)
	{
		int consume = MIN(buf_len, m_data_len);
		memcpy(buf, m_data, consume);
		m_data += consume;
		m_data_len -= consume;
		return consume;
	}
private:
	const char *m_data;
	int m_data_len;
};

class TBNodeTarget : public ParserTarget
{
public:
	TBNodeTarget(TBNode *root, const char *filename)
	{
		m_root_node = m_target_node = root;
		m_filename = filename;
	}
	virtual void OnError(int line_nr, const char *error)
	{
#ifdef _DEBUG
		TBStr err;
		err.SetFormatted("%s(%d):Parse error: %s\n", m_filename, line_nr, error);
		TBDebugOut(err);
#endif // _DEBUG
	}
	virtual void OnComment(int line_nr, const char *comment)
	{
	}
	virtual void OnToken(int line_nr, const char *name, TBValue &value)
	{
		if (!m_target_node)
			return;
		if (strcmp(name, "@file") == 0)
		{
			IncludeFile(line_nr, value.GetString());
		}
		else if (TBNode *n = TBNode::Create(name))
		{
			n->m_value.TakeOver(value);
			m_target_node->Add(n);
		}
	}
	virtual void Enter()
	{
		if (m_target_node)
			m_target_node = m_target_node->GetLastChild();
	}
	virtual void Leave()
	{
		assert(m_target_node != m_root_node);
		if (m_target_node)
			m_target_node = m_target_node->m_parent;
	}
	void IncludeFile(int line_nr, const char *filename)
	{
		// Read the included file into a new TBNode and then
		// move all the children to m_target_node.
		TBTempBuffer include_filename;
		include_filename.AppendPath(m_filename);
		include_filename.AppendString(filename);
		TBNode content;
		if (content.ReadFile(include_filename.GetData()))
		{
			while (TBNode *content_n = content.GetFirstChild())
			{
				content.Remove(content_n);
				m_target_node->Add(content_n);
			}
		}
		else
		{
			TBStr err;
			err.SetFormatted("Referenced file \"%s\" was not found!", include_filename.GetData());
			OnError(line_nr, err);
		}
	}
private:
	TBNode *m_root_node;
	TBNode *m_target_node;
	const char *m_filename;
};

bool TBNode::ReadFile(const char *filename)
{
	Clear();
	FileParser p;
	TBNodeTarget t(this, filename);
	return p.Read(filename, &t);
}

void TBNode::ReadData(const char *data)
{
	ReadData(data, strlen(data));
}

void TBNode::ReadData(const char *data, int data_len)
{
	Clear();
	DataParser p;
	TBNodeTarget t(this, "{data}");
	p.Read(data, data_len, &t);
}

void TBNode::Clear()
{
	free(m_name);
	m_name = nullptr;
	m_children.DeleteAll();
}

}; // namespace tinkerbell
