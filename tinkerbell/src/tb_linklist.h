// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_LINKLIST_H
#define TB_LINKLIST_H

#include "tinkerbell.h"
#include <assert.h>

namespace tinkerbell {

class TBLinkList;

/** Link - To be inserted in LinkList */

class TBLink
{
public:
	TBLink() : prev(nullptr), next(nullptr), linklist(nullptr) {}
	virtual ~TBLink() { assert(!linklist); }
public:
	TBLink *prev;
	TBLink *next;
	TBLinkList *linklist;
};

template<class T>
class TBLinkOf : public TBLink
{
public:
	inline T *GetPrev() const { return (T *) prev; }
	inline T *GetNext() const { return (T *) next; }
};

/** LinkList - This is the backend for TBLinkListOf and TBLinkListAutoDeleteOf.
	You should use the typed TBLinkListOf or TBLinkListAutoDeleteOf for object storing! */

class TBLinkList
{
public:
	TBLinkList() : first(nullptr), last(nullptr) {}

	void Remove(TBLink *link);
	void Delete(TBLink *link);
	void RemoveAll();
	void DeleteAll();

	void AddFirst(TBLink *link);
	void AddLast(TBLink *link);

	void AddBefore(TBLink *link, TBLink *reference);
	void AddAfter(TBLink *link, TBLink *reference);

	int CountItems() const;
public:
	TBLink *first;
	TBLink *last;
};

/** TBLinkListOf is a double linked linklist. */

template<class T>
class TBLinkListOf
{
public:
	/** Remove link from this linklist. */
	void Remove(T *link)			{ m_linklist.Remove(link); }

	/** Remove link from this linklist and delete it. */
	void Delete(T *link)			{ m_linklist.Delete(link); }

	/** Remove all links without deleding them. */
	void RemoveAll()				{ m_linklist.RemoveAll(); }

	/** Delete all links in this linklist. */
	void DeleteAll()				{ m_linklist.DeleteAll(); }

	/** Add link first in this linklist. */
	void AddFirst(T *link)			{ m_linklist.AddFirst(link); }

	/** Add link last in this linklist. */
	void AddLast(T *link)			{ m_linklist.AddLast(link); }

	/** Add link before the reference link (which must be added to this linklist). */
	void AddBefore(T *link, T *reference) { m_linklist.AddBefore(link, reference); }

	/** Add link after the reference link (which must be added to this linklist). */
	void AddAfter(T *link, T *reference) { m_linklist.AddAfter(link, reference); }

	/** Get the first link, or nullptr. */
	T *GetFirst() const { return (T *) m_linklist.first; }

	/** Get the last link, or nullptr. */
	T *GetLast() const { return (T *) m_linklist.last; }

	/** Return true if this linklist contains any links. */
	bool HasLinks() const { return m_linklist.first ? true : false; }

	/** Count the number of links in this list by iterating through all links. */
	int CountItems() const { return m_linklist.CountItems(); }
private:
	TBLinkList m_linklist;
};

/** TBLinkListAutoDeleteOf is a double linked linklist that deletes all links on destruction. */

template<class T>
class TBLinkListAutoDeleteOf : public TBLinkListOf<T>
{
public:
	~TBLinkListAutoDeleteOf() { TBLinkListOf<T>::DeleteAll(); }
};

}; // namespace tinkerbell

#endif // TB_LINKLIST_H
