/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef USERMAP_HPP
#define USERMAP_HPP

#include "network/machine_guard.hpp"

class UserMap
{
public:
	UserMap();

	void add( const UserMessage &um );
	UserMessage* add( struct passwd *ent );

	bool getEnv( UserMessage & um, bool userAlreadyKnown = false );
	UserMessage* fetch( uint16 uid );
	bool setEnv( const UserMessage &um );
	void flush();

	friend class BWMachined;

protected:
	typedef std::map< uint16, UserMessage > Map;
	Map map_;
	UserMessage notfound_;

	void queryUserConfs();
};

#endif
