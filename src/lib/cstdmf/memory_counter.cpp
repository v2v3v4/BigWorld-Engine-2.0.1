/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "memory_counter.hpp"
#include "debug.hpp"

// TODO: implement for win32

void registerAccountBudget( const std::string & account, uint32 budget ) { }

void registerAccountContributor( const std::string & account,
	const std::string & name, uint32 & c ) { }

void registerAccountContributor( const std::string & account,
	const std::string & name, uint32 (*c)() ) { }

// memory_counter.cpp
