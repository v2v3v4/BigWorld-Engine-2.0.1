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
#include "data_section_cache_purger.hpp"

#include "resmgr/bwresource.hpp"


/**
 *	DataSectionCachePurger constructor
 */
DataSectionCachePurger::DataSectionCachePurger()
{
}


/**
 *	DataSectionCachePurger destructor
 */
DataSectionCachePurger::~DataSectionCachePurger()
{
	BWResource::instance().purgeAll();
}


// data_section_cache_purger.cpp