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
#include "app_config.hpp"

#ifndef CODE_INLINE
#include "app_config.ipp"
#endif

#include "resmgr/xml_section.hpp"

DECLARE_DEBUG_COMPONENT2( "App", 0 )

// -----------------------------------------------------------------------------
// Section: AppConfig
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
AppConfig::AppConfig() :
	pRoot_( NULL )
{
}


/**
 *	Destructor.
 */
AppConfig::~AppConfig()
{
}


/**
 *	Init method
 */
bool AppConfig::init( DataSectionPtr configSection )
{
	BW_GUARD;
	if (!configSection.exists() || configSection->countChildren() == 0) 
	{
		return false;
	}

	pRoot_ = configSection;
	return true;
}


/**
 *	Instance accessor
 */
AppConfig & AppConfig::instance()
{
	static AppConfig	inst;
	return inst;
}

// app_config.cpp
