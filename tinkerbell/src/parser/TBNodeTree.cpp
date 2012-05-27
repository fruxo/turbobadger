// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "parser/TBNodeTree.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tb_system.h"

namespace tinkerbell {

TBNode::~TBNode()
{
	Clear();
}

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

const char *get_next_end(const char *request)
{
	while (*request != 0 && *request != '>')
		request++;
	return request;
}

TBNode *TBNode::GetNode(const char *request)
{
	TBNode *n = this;
	while (*request && n)
	{
		const char *nextend = get_next_end(request);
		n = n->GetNode(request, nextend - request);
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
	TBNodeTarget(TBNode *root) { m_root_node = m_target_node = root; }
	virtual void OnError(int line_nr, const char *error)
	{
	}
	virtual void OnComment(const char *comment)
	{
	}
	virtual void OnToken(const char *name, TBValue &value)
	{
		if (TBNode *n = TBNode::Create(name))
		{
			n->m_value.TakeOver(value);
			m_target_node->Add(n);
		}
	}
	virtual void Enter()
	{
		m_target_node = m_target_node->GetLastChild();
	}
	virtual void Leave()
	{
		assert(m_target_node != m_root_node);
		m_target_node = m_target_node->m_parent;
	}
private:
	TBNode *m_root_node;
	TBNode *m_target_node;
};

bool TBNode::ReadFile(const char *filename)
{
	Clear();
	FileParser p;
	TBNodeTarget t(this);
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
	TBNodeTarget t(this);
	p.Read(data, data_len, &t);
}

void TBNode::Clear()
{
	free(m_name);
	m_name = nullptr;
	m_children.DeleteAll();
}

}; // namespace tinkerbell
