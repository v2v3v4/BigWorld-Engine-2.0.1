/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _DATA_SECTION_CACHE_PURGER_HPP_
#define _DATA_SECTION_CACHE_PURGER_HPP_


/**
 *	Data section cache resource manager class.  This class purges all entries
 *	from the data section cache on destruction.
 */
class DataSectionCachePurger
{
public:
	DataSectionCachePurger();
	~DataSectionCachePurger();
};


#endif // data_section_cache_purger.hpp