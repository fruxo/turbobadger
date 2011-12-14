// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_linklist.h"

namespace tinkerbell {

void TBLinkList::AddFirst(TBLink *link)
{
	link->linklist = this;
	link->next = first;
	if (first)
		first->prev = link;
	first = link;
	if (!last)
		last = link;
}

void TBLinkList::AddLast(TBLink *link)
{
	link->linklist = this;
	link->prev = last;
	if (last)
		last->next = link;
	last = link;
	if (!first)
		first = link;
}

void TBLinkList::AddBefore(TBLink *link, TBLink *reference)
{
	assert(reference->linklist == this); // Reference is not added to this list!
	link->linklist = this;
	link->prev = reference->prev;
	link->next = reference;
	if (reference->prev)
		reference->prev->next = link;
	else
		first = link;
	reference->prev = link;
}

void TBLinkList::AddAfter(TBLink *link, TBLink *reference)
{
	assert(reference->linklist == this); // Reference is not added to this list!
	link->linklist = this;
	link->prev = reference;
	link->next = reference->next;
	if (reference->next)
		reference->next->prev = link;
	else
		last = link;
	reference->next = link;
}

void TBLinkList::Remove(TBLink *link)
{
	if (link->next)
		link->next->prev = link->prev;
	if (link->prev)
		link->prev->next = link->next;
	if (first == link)
		first = link->next;
	if (last == link)
		last = link->prev;
	link->linklist = 0;
	link->prev = 0;
	link->next = 0;
}

void TBLinkList::RemoveAll()
{
	TBLink *link = first;
	while (link)
	{
		TBLink *next = link->next;
		link->linklist = 0;
		link->prev = 0;
		link->next = 0;
		link = next;
	}
	first = 0;
	last = 0;
}

void TBLinkList::DeleteAll()
{
	TBLink *link = first;
	while (link)
	{
		TBLink *next = link->next;
		Remove(link);
		delete link;
		link = next;
	}
}

int TBLinkList::CountItems() const
{
	int count = 0;
	for (TBLink *link = first; link; link = link->next)
		count++;
	return count;
}

}; // namespace tinkerbell
