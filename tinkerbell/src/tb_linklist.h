// ================================================================================
// == This file is a part of TinkerBell UI Toolkit. (C) 2011-2012, Emil Segerås  ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_LINKLIST_H
#define TB_LINKLIST_H

#include "tinkerbell.h"
#include <assert.h>

namespace tinkerbell {

class TBLinkList;
class TBLink;

/** TBLinkListIterator - The backend class for a safe iteration of a TBLinkList.

	You would normally recieve a typed iterator from a TBLinkListOf::IterateForward
	or TBLinkListOf::IterateBackward, instead of creating this object directly.

	Safe iteration means that if a link is removed from a linked list, _all_ iterators that currently
	point to that link will automatically step to the next link in the iterators direction. */

class TBLinkListIterator
{
public:
	TBLinkListIterator(const TBLinkListIterator &iter);
	TBLinkListIterator(TBLinkList *linklist, bool forward);
	~TBLinkListIterator();

	/** Set the iterator to the first link in we iterate forward,
		or set it to the last link if we iterate backward.  */
	void Reset();

	/** Get the current link or nullptr if out of bounds. */
	TBLink *Get() const { return m_current_link; }

	/** Get the current link and step the iterator to the next (forward or backward). */
	TBLink *GetAndStep();

	operator TBLink *() const { return m_current_link; }

	const TBLinkListIterator& operator = (const TBLinkListIterator &iter);
private:
	TBLinkList *m_linklist;			///< The linklist we are iterating.
	TBLink *m_current_link;			///< The current link, or nullptr.
	bool m_forward;					///< true if we iterate from first to last item.

	TBLinkListIterator *m_prev;		///< Link in list of iterators for m_linklist
	TBLinkListIterator *m_next;		///< Link in list of iterators for m_linklist

	/** RemoveLink is called when removing/deleting links in the target linklist.
		This will make sure iterators skip the deleted item. */
	void RemoveLink(TBLink *link);
	friend class TBLinkList;

	/** Add ourself to the chain of iterators in the linklist. */
	void Register();

	/** Unlink ourself from the chain of iterators in the linklist. */
	void Unregister();
};

/** TBLink - The backend class to be inserted in TBLinkList.
	Use the typed TBLinkOf for object storing! */

class TBLink
{
public:
	TBLink() : prev(nullptr), next(nullptr), linklist(nullptr) {}
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

/** TBLinkList - This is the backend for TBLinkListOf and TBLinkListAutoDeleteOf.
	You should use the typed TBLinkListOf or TBLinkListAutoDeleteOf for object storing! */

class TBLinkList
{
public:
	TBLinkList() : first(nullptr), last(nullptr), first_iterator(nullptr) {}
	~TBLinkList();

	void Remove(TBLink *link);
	void RemoveAll();

	void AddFirst(TBLink *link);
	void AddLast(TBLink *link);

	void AddBefore(TBLink *link, TBLink *reference);
	void AddAfter(TBLink *link, TBLink *reference);

	bool ContainsLink(TBLink *link) const { return link->linklist == this; }

	int CountLinks() const;
public:
	TBLink *first;
	TBLink *last;
	TBLinkListIterator *first_iterator;
};

/** TBLinkListOf is a double linked linklist. */

template<class T>
class TBLinkListOf
{
public:
	/** Remove link from this linklist. */
	void Remove(T *link)			{ m_linklist.Remove((TBLinkOf<T>*)link); }

	/** Remove link from this linklist and delete it. */
	void Delete(T *link)			{ m_linklist.Remove((TBLinkOf<T>*)link); delete link; }

	/** Remove all links without deleting them. */
	void RemoveAll()				{ m_linklist.RemoveAll(); }

	/** Delete all links in this linklist. */
	void DeleteAll()				{ while (T *t = GetFirst()) Delete(t); }

	/** Add link first in this linklist. */
	void AddFirst(T *link)			{ m_linklist.AddFirst((TBLinkOf<T>*)link); }

	/** Add link last in this linklist. */
	void AddLast(T *link)			{ m_linklist.AddLast((TBLinkOf<T>*)link); }

	/** Add link before the reference link (which must be added to this linklist). */
	void AddBefore(T *link, T *reference) { m_linklist.AddBefore((TBLinkOf<T>*)link, reference); }

	/** Add link after the reference link (which must be added to this linklist). */
	void AddAfter(T *link, T *reference) { m_linklist.AddAfter((TBLinkOf<T>*)link, reference); }

	/** Return true if the link is currently added to this linklist. */
	bool ContainsLink(T *link) const { return m_linklist.ContainsLink((TBLinkOf<T>*)link); }

	/** Get the first link, or nullptr. */
	T *GetFirst() const { return (T *) (TBLinkOf<T>*) m_linklist.first; }

	/** Get the last link, or nullptr. */
	T *GetLast() const { return (T *) (TBLinkOf<T>*) m_linklist.last; }

	/** Return true if this linklist contains any links. */
	bool HasLinks() const { return m_linklist.first ? true : false; }

	/** Count the number of links in this list by iterating through all links. */
	int CountLinks() const { return m_linklist.CountLinks(); }

	/** Typed iterator for safe iteration. For more info, see TBLinkListIterator. */
	class Iterator : public TBLinkListIterator
	{
	public:
		Iterator(TBLinkListOf<T> *linklistof, bool forward) : TBLinkListIterator(&linklistof->m_linklist, forward) {}
		inline T *Get() const { return (T *) (TBLinkOf<T>*) TBLinkListIterator::Get(); }
		inline T *GetAndStep() { return (T *) (TBLinkOf<T>*) TBLinkListIterator::GetAndStep(); }
		inline operator T *() const { return (T *) (TBLinkOf<T>*) Get(); }
	};

	/** Get a forward iterator that starts with the first link. */
	Iterator IterateForward() { return Iterator(this, true); }

	/** Get a backward iterator that starts with the last link. */
	Iterator IterateBackward() { return Iterator(this, false); }
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
