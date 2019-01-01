/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EVENT_POLLER_HPP
#define EVENT_POLLER_HPP

#include "cstdmf/stdmf.hpp"
#include "network/interfaces.hpp"

#include <map>

namespace Mercury
{

class InputNotificationHandler;
typedef std::map< int, InputNotificationHandler * > FDHandlers;


class EventPoller : public InputNotificationHandler
{
public:
	EventPoller();
	virtual ~EventPoller();

	bool registerForRead( int fd, InputNotificationHandler * handler );
	bool registerForWrite( int fd, InputNotificationHandler * handler );

	bool deregisterForRead( int fd );
	bool deregisterForWrite( int fd );

	/**
	 *	This method polls the registered file descriptors for events.
	 *
	 *	@param maxWait The maximum number of seconds to wait for events.
	 */
	virtual int processPendingEvents( double maxWait ) = 0;

	virtual int getFileDescriptor() const;

	// Override from InputNotificationHandler.
	virtual int handleInputNotification( int fd );

	void clearSpareTime()		{ spareTime_ = 0; }
	uint64 spareTime() const	{ return spareTime_; }

	static EventPoller * create();

protected:
	virtual bool doRegisterForRead( int fd ) = 0;
	virtual bool doRegisterForWrite( int fd ) = 0;

	virtual bool doDeregisterForRead( int fd ) = 0;
	virtual bool doDeregisterForWrite( int fd ) = 0;

	void triggerRead( int fd );
	void triggerWrite( int fd );

	bool isRegistered( int fd, bool isForRead ) const;

	int maxFD() const;

private:
	static int maxFD( const FDHandlers & handlerMap );

	// Maps from file descriptor to their callbacks
	FDHandlers fdReadHandlers_;
	FDHandlers fdWriteHandlers_;

protected:
	uint64 spareTime_;
};

} // namespace Mercury

#endif // EVENT_POLLER_HPP
