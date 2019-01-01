/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 * 	@file
 *
 *	This file provides the implementation of the UserDataObjectDescription class.
 *
 * 	@ingroup udo
 */

#include "pch.hpp"

#include "Python.h"

#include "user_data_object_description.hpp"
#include "constants.hpp"

#include "cstdmf/debug.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include "resmgr/bwresource.hpp"

DECLARE_DEBUG_COMPONENT2( "DataDescription", 0 )
#include <algorithm>
#include <cctype>

#include "user_data_object_description.ipp"


/**
 *	This method parses a data section for the properties and methods associated
 *	with this user data object description.
 */
bool UserDataObjectDescription::parseInterface( DataSectionPtr pSection,
									const char * interfaceName )
{
	if (!pSection)
	{
		return false;
	}
	
	bool result = BaseUserDataObjectDescription::parseInterface( pSection, interfaceName);
	// Read domain here to ensure no interface or parent overrides it
	std::string domainString = pSection->readString("Domain");
	std::transform(domainString.begin(), domainString.end(), domainString.begin(), 
		(int(*)(int)) toupper);
	if ( domainString.empty() ){
		//DEFAULT TO CELL
		domain_ = CELL; 
		WARNING_MSG("UserDataObjectDescription::parseInterface:"
				"domain not set for user data object default is cell");
	} else if (domainString == "BASE"){
		domain_ = BASE;
	} else if (domainString == "CELL"){
		domain_ = CELL;
	} else if (domainString == "CLIENT"){
		domain_ = CLIENT;
	}
	return result;
}


/**
 *	This method parses a data section for the properties associated with this
 *	entity description.
 *
 *	@param pProperties	The datasection containing the properties.
 *
 *	@return true if successful, false otherwise.
 */
bool UserDataObjectDescription::parseProperties( DataSectionPtr pProperties )
{
//	MF_ASSERT( properties_.empty() );

	if (pProperties)
	{
		for (DataSectionIterator iter = pProperties->begin();
				iter != pProperties->end();
				++iter)
		{
			DataDescription dataDescription;

			if (!dataDescription.parse( *iter, name_,
					DataDescription::PARSE_IGNORE_FLAGS ))
			{
				WARNING_MSG( "Error parsing properties for %s\n",
						name_.c_str() );
				return false;
			}

#ifndef EDITOR_ENABLED
			if (dataDescription.isEditorOnly())
			{
				continue;
			}
#endif

			int index = properties_.size();
			PropertyMap::const_iterator propIter =
					propertyMap_.find( dataDescription.name() );
			if (propIter != propertyMap_.end())
			{
				INFO_MSG( "UserDataObjectDescription::parseProperties: "
						"property %s.%s is being overridden.\n",
					name_.c_str(), dataDescription.name().c_str() );
				index = propIter->second;
			}
			dataDescription.index( index );
			propertyMap_[dataDescription.name()] = dataDescription.index();
#ifdef EDITOR_ENABLED
			DataSectionPtr widget = (*iter)->openSection( "Widget" );
			if ( !!widget )
			{
				dataDescription.widget( widget );
			}
#endif
			if (index == int(properties_.size()))
			{
				properties_.push_back( dataDescription );
			}
			else
			{
				properties_[index] = dataDescription;
			}
		}
	}
	/*
	else
	{
		// Not really the correct message since the data section may be an
		// interface. Also probably not worthwhile since it's fine for the files
		// not to have this section.
		WARNING_MSG( "%s has no Properties section.\n", name_.c_str() );
	}
	*/

	return true;
}


/**
  * Tell udo description which directory it should try read
  * the .def files from
  */
const std::string UserDataObjectDescription::getDefsDir() const
{
	return EntityDef::Constants::userDataObjectsDefsPath();
}


/**
  * Tell udo description which directory it should try read
  * the client script files from
  */
const std::string UserDataObjectDescription::getClientDir() const
{
	return EntityDef::Constants::userDataObjectsClientPath();
}


/**
  * Tell udo description which directory it should try read
  * the cell server files from
  */
const std::string UserDataObjectDescription::getCellDir() const
{
	return EntityDef::Constants::userDataObjectsCellPath();
}


/**
  * Tell udo description which directory it should try read
  * the base server files from
  */
const std::string UserDataObjectDescription::getBaseDir() const
{
	return EntityDef::Constants::userDataObjectsBasePath();
}
