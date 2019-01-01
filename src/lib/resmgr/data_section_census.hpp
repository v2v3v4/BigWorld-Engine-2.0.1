/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DATA_SECTION_CENSUS_HPP
#define DATA_SECTION_CENSUS_HPP

#include <string>
#include "cstdmf/smartpointer.hpp"

class DataSection;
typedef SmartPointer<DataSection> DataSectionPtr;


/**
 *	This maintains a 'census' of all named DataSections that are currently 
 *	alive. This allows quick lookup of named sections which are alive (but not
 *	neccessarily in the cache anymore). It does not hold onto a strong reference
 *	to the DataSections. DataSections are not automatically added to the census 
 *	(because not all DataSections have a name), but they are automatically removed 
 *	from the census when they die.
 */
namespace DataSectionCensus
{
	void store( void ** word );

	DataSectionPtr find( const std::string & id );
	DataSectionPtr add( const std::string & id, DataSectionPtr pSect );
	void del( DataSection * pSect );
	void clear();

	void fini();
};


#endif // DATA_SECTION_CENSUS_HPP
