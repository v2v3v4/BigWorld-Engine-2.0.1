/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LISTENERS_HPP
#define LISTENERS_HPP

#include "network/machine_guard.hpp"

class BWMachined;

class Listeners
{
private:
	class Member
	{
	public:
		Member( const ListenerMessage & lm, u_int32_t addr ) :
			lm_( lm ),
			addr_( addr ) {}
		ListenerMessage lm_;
		u_int32_t addr_;
	};

public:
	Listeners( BWMachined &machined );
	void add( const ListenerMessage & lm, u_int32_t addr )
	{
		members_.push_back( Member( lm, addr ) );
	}

	void handleNotify( const ProcessMessage & pm, in_addr addr );
	void checkListeners();

private:
	typedef std::vector< Member > Members;
	Members members_;
	BWMachined &machined_;
};

#endif
