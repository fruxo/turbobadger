// ================================================================================
// == This file is a part of Tinkerbell UI Toolkit. (C) 2011-2012, Emil Segerås ==
// ==                   See tinkerbell.h for more information.                   ==
// ================================================================================

#ifndef TB_MSG_H
#define TB_MSG_H

#include "tinkerbell.h"
#include "tb_linklist.h"

namespace tinkerbell {

class TBMessageHandler;

/** TB_NOT_SOON is returned from TBMessageHandler::GetNextMessageFireTime
	and means that there is currently no more messages to process. */
#define TB_NOT_SOON 0xffffffff

/** TBMessageData holds custom data to send with a posted message. */

class TBMessageData
{
public:
	TBMessageData() : param1(0), param2(0) {}
	TBMessageData(uint32 param1, uint32 param2) : param1(param1), param2(param2) {}
	virtual ~TBMessageData() {}
public:
// FIX: Add a TBValueArray here by default?
	uint32 param1;
	uint32 param2;
};

/** TBMessage is a message created and owned by TBMessageHandler.
	It carries a message id, and may also carry a TBMessageData with
	additional parameters. */

class TBMessage
{
private:
	TBMessage(TBID message, TBMessageData *data, double fire_time_ms, TBMessageHandler *mh);
	~TBMessage();

public:
	TBID message;			///< The message id
	TBMessageData *data;	///< The message data, or nullptr if no data is set

	/** The time which a delayed message should have fired (0 for non delayed messages) */
	double GetFireTime() { return fire_time_ms; }

private:
	friend class TBMessageHandler;
	double fire_time_ms;
	TBLink link_in_global;
	TBLink link_in_mh;
	TBMessageHandler *mh;
};

/** TBMessageHandler handles a list of pending messages posted to itself.
	Messages can be delivered immediately or after a delay.
	Delayed message are delivered as close as possible to the time they should fire.
	Immediate messages are put on a queue and delivered as soon as possible, after any delayed
	messages that has passed their delivery time. This queue is global (among all TBMessageHandlers) */

class TBMessageHandler
{
public:
	TBMessageHandler();
	virtual ~TBMessageHandler();

	/** Posts a message to the target after a delay.
		data may be nullptr if no extra data need to be sent. It will be deleted
		automatically when the message is deleted. */
	bool PostMessageDelayed(TBID message, TBMessageData *data, uint32 delay_in_ms);

	/** Posts a message to the target at the given time (relative to TBSystem::GetTimeMS()).
		data may be nullptr if no extra data need to be sent. It will be deleted
		automatically when the message is deleted. */
	bool PostMessageOnTime(TBID message, TBMessageData *data, double fire_time);

	/** Posts a message to the target.
		data may be nullptr if no extra data need to be sent. It will be deleted
		automatically when the message is deleted. */
	bool PostMessage(TBID message, TBMessageData *data);

	/** Check if this messagehandler has a pending message with the given id.
		Returns the message if found, or nullptr.
		If you want to delete the message, call DeleteMessage. */
	TBMessage *GetMessageByID(TBID message);

	/** Delete the message from this message handler. */
	void DeleteMessage(TBMessage *msg);

	/** Delete all messages from this message handler. */
	void DeleteAllMessages();

	/** Called when a message is delivered.

		This message won't be found using GetMessageByID. It is already removed from the list.
		You should not call DeleteMessage on this message. That is done automatically after this method exit. */
	virtual void OnMessageReceived(TBMessage *msg) {}

	// == static methods to handle the queue of messages ====================================================

	/** Process any messages in queue. */
	static void ProcessMessages();

	/** Get when the time when ProcessMessages needs to be called again.
		Always returns 0 if there is nondelayed messages to process, which means it needs to be called asap.
		If there's only delayed messages to process, it returns the time that the earliest delayed message should be fired.
		If there's no more messages to process at the moment, it returns TB_NOT_SOON (No call to ProcessMessages is needed). */
	static double GetNextMessageFireTime();
private:
	TBLinkList m_messages;
};

}; // namespace tinkerbell

#endif // TB_MSG_H
