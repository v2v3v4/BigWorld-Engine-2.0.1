/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LIB__SERVER__SIGNAL_PROCESSOR_HPP
#define LIB__SERVER__SIGNAL_PROCESSOR_HPP

#include "cstdmf/concurrency.hpp"
#include "cstdmf/singleton.hpp"

#include "network/frequent_tasks.hpp"

#include "server/signal_set.hpp"

#include <map>
#include <utility>
#include <vector>

#include <signal.h>

namespace Mercury
{
	class EventDispatcher;
}

namespace Signal
{
	class Set;
}

/**
 *	Signal handler abstract class.
 */
class SignalHandler
{
public:
	virtual ~SignalHandler();

	virtual void handleSignal( int sigNum ) = 0;
};


/**
 *	Signal processing class.
 */
class SignalProcessor : private Mercury::FrequentTask,
		public Singleton< SignalProcessor >
{
public:
	SignalProcessor( Mercury::EventDispatcher & dispatcher );
	virtual ~SignalProcessor();

	void ignoreSignal( int sigNum );

	void setDefaultSignalHandler( int sigNum );

	void addSignalHandler( int sigNum, SignalHandler * pSignalHandler, 
		int flags = 0 );

	void clearSignalHandlers( int sigNum );
	void clearSignalHandler( int sigNum, SignalHandler * pSignalHandler );
	void clearSignalHandler( SignalHandler * pSignalHandler );

	int waitForSignals( const Signal::Set & signalSet );

	void handleSignal( int sigNum );

	static const char * signalNumberToString( int sigNum );
private:
	// Override from Mercury::FrequentTask
	virtual void doTask()
		{ this->dispatch(); }

	void dispatch();
	void dispatchSignal( int sigNum );

	void enableDetectSignal( int sigNum, int flags = 0 );

// Member data
	Mercury::EventDispatcher & dispatcher_;

	typedef std::multimap< int, SignalHandler * > SignalHandlers;
	SignalHandlers signalHandlers_;

	SignalHandler * pSigQuitHandler_;

	Signal::Set signals_;
};


#endif // LIB__SERVER__SIGNAL_PROCESSOR_HPP
