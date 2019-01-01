/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SCRIPT_BIGWORLD_HPP
#define SCRIPT_BIGWORLD_HPP

class Bases;

namespace Mercury
{
class EventDispatcher;
class NetworkInterface;
}

// TODO: This is not the right place for these.
enum LogOnAttemptResult
{
	LOG_ON_REJECT = 0,
	LOG_ON_ACCEPT = 1,
	LOG_ON_WAIT_FOR_DESTROY = 2,
};


namespace BigWorldBaseAppScript
{

bool init( const Bases & bases,
		Mercury::EventDispatcher & dispatcher,
		Mercury::NetworkInterface & intInterface );

}

#endif // SCRIPT_BIGWORLD_HPP
