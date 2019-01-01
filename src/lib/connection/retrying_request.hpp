/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef RETRYING_REQUEST_HPP
#define RETRYING_REQUEST_HPP

#include "cstdmf/smartpointer.hpp"
#include "cstdmf/timer_handler.hpp"

#include "network/basictypes.hpp"
#include "network/interfaces.hpp"
#include "network/misc.hpp"

class BinaryIStream;
class LoginHandler;

namespace Mercury
{
	class Bundle;
	class EventDispatcher;
	class InterfaceElement;
	class NetworkInterface;
	class NubException;
	class UnpackedMessageHeader;
} // namespace Mercury

/**
 *  This class provides client-push reliability for off-channel Mercury
 *  messages.  Basically it keeps sending requests until one comes back.
 *  This object deletes itself.
 */
class RetryingRequest :
		public Mercury::ShutdownSafeReplyMessageHandler,
		public TimerHandler,
		public SafeReferenceCount
{
public:
	/// Default retry period for requests (1s).
	static const int DEFAULT_RETRY_PERIOD = 1000000;

	/// Default timeout period for requests (8s).
	static const int DEFAULT_TIMEOUT_PERIOD = 8000000;

	/// Default limit for attempts.
	static const int DEFAULT_MAX_ATTEMPTS = 10;

	RetryingRequest( LoginHandler & parent,
		const Mercury::Address & addr,
		const Mercury::InterfaceElement & ie,
		int retryPeriod = DEFAULT_RETRY_PERIOD,
		int timeoutPeriod = DEFAULT_TIMEOUT_PERIOD,
		int maxAttempts = DEFAULT_MAX_ATTEMPTS,
		bool useParentInterface = true );

	virtual ~RetryingRequest();

	// Inherited interface

	virtual void handleMessage( const Mercury::Address & srcAddr,
		Mercury::UnpackedMessageHeader & header,
		BinaryIStream & data,
		void * arg );

	virtual void handleException( const Mercury::NubException & exc,
		void * arg );

	virtual void handleTimeout( TimerHandle handle, void * arg );

	// Interface to be implemented by derived classes

	/**
	 *  This method should stream on the args for the request.  The
	 *  startRequest() call is taken care of beforehand.
	 */
	virtual void addRequestArgs( Mercury::Bundle & bundle ) = 0;

	/**
	 *  This method will be called on the first reply.  This object will be
	 *  deleted immediately afterwards.
	 */
	virtual void onSuccess( BinaryIStream & data ) = 0;

	/**
	 *  This method is called if the request should fail for any reason.  This
	 *  object will be deleted immediately afterwards.
	 */
	virtual void onFailure( Mercury::Reason reason ) {}

	/**
	 *  This method removes this request from the parent's childRequests_ list,
	 *  which means it will be destroyed as soon as all of its outstanding 
	 *  requests have either been replied to (and ignored) or timed out.
	 */
	void cancel();

protected:
	void setInterface( Mercury::NetworkInterface * pInterface );
	void send();

	Mercury::EventDispatcher & dispatcher();

protected:
	SmartPointer< LoginHandler > pParent_;
	Mercury::NetworkInterface * pInterface_;
	const Mercury::Address addr_;
	const Mercury::InterfaceElement & ie_;
	TimerHandle timerHandle_;
	bool done_;

private:
	int retryPeriod_;
	int timeoutPeriod_;

	/// The number of attempts that have been initiated from this object.
	int numAttempts_;

	/// The number of attempts that have been initiated but not terminated.
	int numOutstandingAttempts_;

	/// The maximum number of attempts that this object will make.
	int maxAttempts_;
};

typedef SmartPointer< RetryingRequest > RetryingRequestPtr;


#endif // RETRYING_REQUEST_HPP
