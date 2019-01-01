/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "udo_ref_mapping.hpp"

#include "resmgr/datasection.hpp"

/**
 *	Constructor.
 */
UDORefMapping::UDORefMapping( const Namer& namer, const std::string& propName,
		DataSectionPtr pDefaultValue ) :
	UniqueIDMapping( namer, propName, getGuidSection( pDefaultValue ) )
{
}


/**
 *	This static method retrieves the GUID of the UDO from the provided
 *	DataSection.
 *
 *	@param pParentSection  The DataSection containing the GUID to use.
 *
 *	@returns The DataSectionPtr of the GUID if it exists, otherwise NULL.
 */
DataSectionPtr UDORefMapping::getGuidSection( DataSectionPtr pParentSection )
{
	return (pParentSection) ? pParentSection->openSection( "guid" ) : NULL;
}

// udo_ref_mapping.cpp
