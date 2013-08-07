// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2013, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TBNodeTree_H
#define TBNodeTree_H

#include "parser/TBParser.h"
#include "tb_linklist.h"

namespace tinkerbell {

/** TBNode is a tree node with a string name and a value (TBValue). */
class TBNode : public TBLinkOf<TBNode>
{
public:
	TBNode() : m_name(nullptr), m_parent(nullptr) {}
	~TBNode();

	/** Create a new node with the given name. */
	static TBNode *Create(const char *name);

	/** Read a tree of nodes from file into this node. Returns true on success. */
	bool ReadFile(const char *filename);

	/** Read a tree of nodes from a null terminated string buffer. */
	void ReadData(const char *data);

	/** Read a tree of nodes from a buffer with a known length. */
	void ReadData(const char *data, int data_len);

	/** Clear the contens of this node. */
	void Clear();

	// FIX: Add write support!
	//bool WriteFile(const char *filename);

	/** Add node as child to this node. */
	void Add(TBNode *n) { m_children.AddLast(n); n->m_parent = this; }

	/** Remove child node n from this node. */
	void Remove(TBNode *n) { m_children.Remove(n); n->m_parent = nullptr; }

	/** Create duplicates of all items in source and add them to this node.
		Note: Nodes does not replace existing nodes with the same name. Cloned nodes
		are added after any existing nodes. */
	bool CloneChildren(TBNode *source);

	/** Get a node from the given request or nullptr if no such node was found.
		It can find nodes in children as well. Names are separated by a ">".
		Ex: GetNode("dishes>pizza>special>batman") */
	TBNode *GetNode(const char *request);

	/** Returns the name of this node. */
	const char *GetName() const { return m_name; }

	/** Returns the value of this node. */
	TBValue &GetValue() { return m_value; }

	/** Get a value from the given request as an integer.
		If the value is not specified, it returns the default value (def). */
	int GetValueInt(const char *request, int def);

	/** Get a value from the given request as an float.
		If the value is not specified, it returns the default value (def). */
	float GetValueFloat(const char *request, float def);

	/** Get a value from the given request as an string.
		If the value is not specified, it returns the default value (def). */
	const char *GetValueString(const char *request, const char *def);

	inline TBNode *GetFirstChild() const { return m_children.GetFirst(); }
	inline TBNode *GetLastChild() const { return m_children.GetLast(); }
private:
friend class TBNodeTarget;
	TBNode *GetNode(const char *name, int name_len) const;
	char *m_name;
	TBValue m_value;
	TBLinkListOf<TBNode> m_children;
	TBNode *m_parent;
};

}; // namespace tinkerbell

#endif // TBNodeTree_H
