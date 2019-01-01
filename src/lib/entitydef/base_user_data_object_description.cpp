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
 *	This file provides the implementation of the BaseUserDataObjectDescription class.
 *  It is intended that EntityDescription and UserDataObjectDescription both descend from this class to minimise code 
 *  duplication.
 *
 * 	@ingroup UserDataObject
 */

#include "pch.hpp"

#include "Python.h"

#include "base_user_data_object_description.hpp"
#include "cstdmf/md5.hpp"
#include "cstdmf/debug.hpp"

#include "cstdmf/binary_stream.hpp"

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"

#include "resmgr/bwresource.hpp"
#include "resmgr/multi_file_system.hpp"

#include "float.h"

DECLARE_DEBUG_COMPONENT2( "DataDescription", 0 )

#ifndef CODE_INLINE
#include "base_user_data_object_description.ipp"
#endif


// -----------------------------------------------------------------------------
// Section: BaseUserDataObjectDescription
// This class is responsible for reading a UserDataObject defined in a def file
// 
// -----------------------------------------------------------------------------

/**
 *	The constructor.
 */
BaseUserDataObjectDescription::BaseUserDataObjectDescription()
{
}


/**
 *	Destructor.
 */
BaseUserDataObjectDescription::~BaseUserDataObjectDescription()
{
}

/**
 *	This method parses an user data object description from a datasection.
 *
 *	@param name		The name of the  type.
 *	@param pSection	If not NULL, this data section is used, otherwise the
 *					data section is opened based on the name.
 *	@param isFinal	False if parsing an ancestor otherwise true.
 *
 *	@return true if successful, false otherwise.
 */
bool BaseUserDataObjectDescription::parse( const std::string & name,
		DataSectionPtr pSection, bool isFinal )
{
	if (!pSection)
	{
		std::string filename = this->getDefsDir() + "/" + name + ".def";
		pSection = BWResource::openSection( filename );

		if (!pSection)
		{
			ERROR_MSG( "BaseUserDataObjectDescription::parse: Could not open %s\n",
					filename.c_str() );
			return false;
		}
	}

	std::string parentName = pSection->readString( "Parent" );

	if (!parentName.empty())
	{
		if (!this->parse( parentName, NULL, false ))
		{
			ERROR_MSG( "BaseUserDataObjectDescription::parse: "
						"Could not parse %s, parent of %s\n",
					parentName.c_str(), name.c_str() );
			return false;
		}
	}

	name_ = name;
	bool result = this->parseInterface( pSection, name_.c_str() );
	return result;
}


/**
 *	This method parses a data section for the properties and methods associated
 *	with this entity description.
 */
bool BaseUserDataObjectDescription::parseInterface( DataSectionPtr pSection,
									const char * interfaceName )
{
	if (!pSection)
	{
		return false;
	}

	bool result =
		this->parseImplements( pSection->openSection( "Implements" ) ) &&
		this->parseProperties( pSection->openSection( "Properties" ) ); 
	return result;
}


/**
 *	This method parses an "Implements" section. This is used so that defs can
 *	share interfaces. It adds each of the referred interfaces to this
 *	description.
 */
bool BaseUserDataObjectDescription::parseImplements( DataSectionPtr pInterfaces )
{

	bool result = true;

	if (pInterfaces)
	{
		DataSection::iterator iter = pInterfaces->begin();

		while (iter != pInterfaces->end())
		{
			std::string interfaceName = (*iter)->asString();

			DataSectionPtr pInterface = BWResource::openSection(
				 	this->getDefsDir() + "/interfaces/" + interfaceName + ".def" );

			if (!this->parseInterface( pInterface, interfaceName.c_str()  ))
			{
				ERROR_MSG( "BaseUserDataObjectDescription::parseImplements: "
					"Failed parsing interface %s\n", interfaceName.c_str() );
				result = false;
			}

			iter++;
		}
	}

	return result;

}


/**
 *	This method searches a datasection for properties belonging to
 *	this entity description, and adds them to the given python
 *	dictionary.
 *
 *	@param pSection		Datasection containing properties
 *	@param pDict		Dictionary to add the properties to
 */
void BaseUserDataObjectDescription::addToDictionary( DataSectionPtr pSection,
	PyObject* pDict ) const
{
	for(Properties::const_iterator it = properties_.begin();
		it != properties_.end(); ++it)
	{
		DataDescription* pDesc = this->findProperty( (*it).name() );

		if(!pDesc)
		{
			WARNING_MSG("BaseUserDataObjectDescription %s has no property %s\n",
				this->name().c_str(), (*it).name().c_str());
			continue;
		}

		DataSectionPtr pPropSec = pSection->openSection( pDesc->name() );
		PyObjectPtr pValue = pDesc->pInitialValue();

		// change to created value if it parses ok
		if (pPropSec)
		{
			PyObjectPtr pTemp = pDesc->createFromSection( pPropSec );
			if (pTemp)
				pValue = pTemp;
		}

		MF_ASSERT_DEV( pValue );

		if (pValue)
		{
			PyDict_SetItemString( pDict,
				const_cast<char*>(pDesc->name().c_str()), pValue.get() );
		}
	}
}


// -----------------------------------------------------------------------------
// Section: Property related.
// -----------------------------------------------------------------------------

/**
 *	This method returns the number of data properties of this entity class.
 */
unsigned int BaseUserDataObjectDescription::propertyCount() const
{
	return properties_.size();
}


/**
 *	This method returns a given data property for this entity class.
 *  TODO: move to entity
 */
DataDescription* BaseUserDataObjectDescription::property( unsigned int n ) const
{
	MF_ASSERT_DEV(n < properties_.size());
	if (n >= properties_.size()) return NULL;

	return const_cast<DataDescription *>( &properties_[n] );
}

/**
 *	This method returns a given data property for this entity class.
 */
DataDescription*
BaseUserDataObjectDescription::findProperty( const std::string& name ) const
{
	PropertyMap::const_iterator iter = propertyMap_.find( name );

	return (iter != propertyMap_.end()) ? this->property( iter->second ) : NULL;
}

// base_custom_item_description.cpp
