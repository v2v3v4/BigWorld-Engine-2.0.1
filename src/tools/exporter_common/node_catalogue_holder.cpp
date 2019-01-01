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
#include "node_catalogue_holder.hpp"


/**
 *	Constructor
 */
NodeCatalogueHolder::NodeCatalogueHolder()
{
	new Moo::NodeCatalogue();
}


/**
 *	Destructor
 */
NodeCatalogueHolder::~NodeCatalogueHolder()
{
	delete Moo::NodeCatalogue::pInstance();
}

// exporter_utility.cpp