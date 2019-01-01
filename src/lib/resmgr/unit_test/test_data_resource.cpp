/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "stdafx.h"
#include "resmgr/bwresource.hpp"
#include "resmgr/bin_section.hpp"
#include "resmgr/zip_section.hpp"
#include "resmgr/xml_section.hpp"

#include "test_harness.hpp"

/**
 *	This tests saving an XML DataResource.
 */
TEST_F( ResMgrUnitTestHarness, XMLDataResource_Save )
{
	CHECK( this->isOK() );

	DataResource dataRes1( "xmldataresource.xml", RESOURCE_TYPE_XML );
	CHECK( dataRes1.save() == DataHandle::DHE_NoError );

	DataResource dataRes2( "", RESOURCE_TYPE_XML );
	CHECK( dataRes2.save() == DataHandle::DHE_SaveFailed );
}

// test_data_resource.cpp
