// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#include "tb_msg.h"
#include "tb_system.h"
#include <stddef.h>

namespace tinkerbell {
/*
FIX:
dokumnentera: Widget::OnProcess should NOT do stuff like deleting widgets (no advanced ui decisions, only local widget related, like layout).
document: att delayed events har högre prioritet än vanliga messages. delay 0 kommer alltså före kön av vanliga posted messages.
*/

// Some casts for getting a TBMessage back from a Link (The links are members at known offsets
// since we can't inherit 2 Links). gcc will warn about this because it thinks TBMessage is a
// POD because of the constructor, but that has no meaning for the offset.
#define GLOBAL_LINK_TO_MSG(link) (TBMessage *) (((char *)link) - offsetof(TBMessage, link_in_global))
#define MH_LINK_TO_MSG(link)  (TBMessage *) (((char *)link) - offsetof(TBMessage, link_in_mh))

/** Used during message list iteration. It's updated if the message it points to is deleted. */
TBLink *g_message_linklist_iterator;

/** List of all delayed messages */
TBLinkList g_all_delayed_messages;

/** List of all nondelayed messages. */
TBLinkList g_all_normal_messages;

// == TBMessage =========================================================================

TBMessage::TBMessage(TBID message, TBMessageData *data, double fire_time_ms, TBMessageHandler *mh)
	: message(message), data(data), fire_time_ms(fire_time_ms), mh(mh)
{
}

TBMessage::~TBMessage()
{
	delete data;
}

// == TBMessageHandler ==================================================================

TBMessageHandler::TBMessageHandler()
{
}

TBMessageHandler::~TBMessageHandler()
{
	DeleteAllMessages();
}

bool TBMessageHandler::PostMessageDelayed(TBID message, TBMessageData *data, uint32 delay_in_ms)
{
	return PostMessageOnTime(message, data, TBSystem::GetTimeMS() + (double)delay_in_ms);
}

bool TBMessageHandler::PostMessageOnTime(TBID message, TBMessageData *data, double fire_time)
{
	if (TBMessage *msg = new TBMessage(message, data, fire_time, this))
	{
		// Find the message that is already in the list that should fire later, so we can
		// insert msg just before that. (Always keep the list ordered after fire time)

		// NOTE: If another message is added during OnMessageReceived, it might or might not be fired
		// in the right order compared to other delayed messages, depending on if it's inserted before or
		// after the message being processed!

		TBMessage *later_msg = nullptr;
		TBLink *link = g_all_delayed_messages.first;
		while (link)
		{
			TBMessage *msg_in_list = GLOBAL_LINK_TO_MSG(link);
			if (msg_in_list->fire_time_ms > msg->fire_time_ms)
			{
				later_msg = msg_in_list;
				break;
			}
			link = link->next;
		}

		// Add it to the global list in the right order.
		if (later_msg)
			g_all_delayed_messages.AddBefore(&msg->link_in_global, &later_msg->link_in_global);
		else
			g_all_delayed_messages.AddLast(&msg->link_in_global);

		// Add it to the list in messagehandler.
		m_messages.AddLast(&msg->link_in_mh);

		// If we added it first and there's no normal messages, the next fire time has
		// changed and we have to reschedule the timer.
		if (!g_all_normal_messages.first && g_all_delayed_messages.first == &msg->link_in_global)
			TBSystem::RescheduleTimer(msg->fire_time_ms);
		return true;
	}
	return false;
}

bool TBMessageHandler::PostMessage(TBID message, TBMessageData *data)
{
	if (TBMessage *msg = new TBMessage(message, data, 0, this))
	{
		g_all_normal_messages.AddLast(&msg->link_in_global);
		m_messages.AddLast(&msg->link_in_mh);

		// If we added it and there was no messages, the next fire time has
		// changed and we have to rescedule the timer.
		if (g_all_normal_messages.first == &msg->link_in_global)
			TBSystem::RescheduleTimer(0);
		return true;
	}
	return false;
}

TBMessage *TBMessageHandler::GetMessageByID(TBID message)
{
	for (TBLink *link = m_messages.first; link; link = link->next)
	{
		TBMessage *msg = MH_LINK_TO_MSG(link);
		if (msg->message == message)
			return msg;
	}
	return nullptr;
}

void TBMessageHandler::DeleteMessage(TBMessage *msg)
{
	assert(msg->mh == this); // This is not the message handler owning the message!
	assert(msg->link_in_global.linklist); // This message is not in any list! (Corrupt memmory?)

	// When we delete messages while iterating, we must make sure that the next iteration isn't
	// pointing to a deleted message. So if this is the iterator, move it to the next message.
	if (&msg->link_in_global == g_message_linklist_iterator ||
		&msg->link_in_mh == g_message_linklist_iterator)
		g_message_linklist_iterator = g_message_linklist_iterator->next;

	// Remove from global list (g_all_delayed_messages or g_all_normal_messages)
	msg->link_in_global.linklist->Remove(&msg->link_in_global);
	// Remove from local list
	m_messages.Remove(&msg->link_in_mh);

	delete msg;

	// Note: We could call TBSystem::RescheduleTimer if we think that deleting
	// this message changed the time for the next message.
}

void TBMessageHandler::DeleteAllMessages()
{
	while (m_messages.first)
	{
		TBMessage *msg = MH_LINK_TO_MSG(m_messages.first);
		DeleteMessage(msg);
	}
}

//FIX: Skicka in max tid den får köra!
//static
void TBMessageHandler::ProcessMessages()
{
	g_message_linklist_iterator = g_all_delayed_messages.first;
	while (g_message_linklist_iterator)
	{
		TBMessage *msg = GLOBAL_LINK_TO_MSG(g_message_linklist_iterator);

		// Update iterator to the next message. This will be updated if the message is deleted.
		g_message_linklist_iterator = g_message_linklist_iterator->next;

		if (TBSystem::GetTimeMS() >= msg->fire_time_ms)
		{
			// Remove from global list (g_all_delayed_messages or g_all_normal_messages)
			msg->link_in_global.linklist->Remove(&msg->link_in_global);
			// Remove from local list
			msg->mh->m_messages.Remove(&msg->link_in_mh);

			msg->mh->OnMessageReceived(msg);

			delete msg;
		}
		else
			break; // Since the list is sorted, all remaining messages should fire later
	}

	g_message_linklist_iterator = g_all_normal_messages.first;
	while (g_message_linklist_iterator)
	{
		TBMessage *msg = GLOBAL_LINK_TO_MSG(g_message_linklist_iterator);

		// Update iterator to the next message. This will be updated if the message is deleted.
		g_message_linklist_iterator = g_message_linklist_iterator->next;

		// Remove from global list (g_all_delayed_messages or g_all_normal_messages)
		msg->link_in_global.linklist->Remove(&msg->link_in_global);
		// Remove from local list
		msg->mh->m_messages.Remove(&msg->link_in_mh);

		msg->mh->OnMessageReceived(msg);

		delete msg;
	}
}

//static
double TBMessageHandler::GetNextMessageFireTime()
{
	if (g_all_normal_messages.first)
		return 0;

	if (g_all_delayed_messages.first)
	{
		TBMessage *first_delayed_msg = GLOBAL_LINK_TO_MSG(g_all_delayed_messages.first);
		return first_delayed_msg->fire_time_ms;
	}

	return TB_NOT_SOON;
}

}; // namespace tinkerbell
