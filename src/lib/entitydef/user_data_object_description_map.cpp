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

#include "user_data_object_description.hpp"
#include "user_data_object_description_map.hpp"
// #include "cstdmf/md5.hpp"
#include "cstdmf/debug.hpp"
#include "resmgr/bwresource.hpp"

int UserDataObjectDescriptionMap_Token =1;
DECLARE_DEBUG_COMPONENT2( "UserDataObjectDescriptionMap", 0 )

/*
 *	This class is used to clear entity description state before PyFinalize is
 *	called.
 */
class UserDataObjectDefFiniTimeJob : public Script::FiniTimeJob
{
protected:
	void fini()
	{
		DataType::clearAllScript();
	}
};

static UserDataObjectDefFiniTimeJob s_udoDefFiniTimeJob;


/**
 *	Constructor.
 */
UserDataObjectDescriptionMap::UserDataObjectDescriptionMap()
{
}

/**
 *	Destructor.
 */
UserDataObjectDescriptionMap::~UserDataObjectDescriptionMap()
{
}

/**
 *	This method parses the udo description map from a datasection.
 *
 *	@param pSection	Datasection containing the entity descriptions.
 *
 *	@return true if successful, false otherwise.
 */
bool UserDataObjectDescriptionMap::parse( DataSectionPtr pSection )
{
	if (!pSection)
	{
		ERROR_MSG( "UserDataObjectDescriptionMap::parse: pSection is NULL\n" );
		return false;
	}

	bool isOkay = true;
	int size = pSection->countChildren();

	for (int i = 0; i < size; i++)
	{
		DataSectionPtr pSubSection = pSection->openChild( i );
		UserDataObjectDescription desc;

		std::string typeName = pSubSection->sectionName();

		if (!desc.parse( typeName ))
		{
			ERROR_MSG( "UserDataObjectDescriptionMap: "
				"Failed to parse def for entity type %s\n",
				typeName.c_str() );

			isOkay = false;
			continue;
		}
		#ifdef MF_SERVER
			if ( (desc.domain() & (UserDataObjectDescription::BASE |
				UserDataObjectDescription::CELL) ) == 0){
					continue;
			}
		#else //client or editor
			#ifndef EDITOR_ENABLED //therefore client
				if ( (desc.domain() & UserDataObjectDescription::CLIENT) == 0){
					continue;
				}
			#endif
		#endif

		map_[ desc.name() ] = desc ;
	}
	return isOkay;
}


/**
 *	This method returns the number of UserDataObjects.
 *
 *	@return Number of User Data Objects.
 */
int UserDataObjectDescriptionMap::size() const
{
	return map_.size();
}


/**
 *	This method returns the entity description with the given index.
 */
const UserDataObjectDescription& UserDataObjectDescriptionMap::udoDescription(
		std::string name ) const
{
	DescriptionMap::const_iterator result = map_.find(name);
	IF_NOT_MF_ASSERT_DEV( result != map_.end() )
	{
		MF_EXIT( "can't find UDO description" );
	}

    return result->second;
}



/**
 *	This method clears all the entity descriptions stored in this object.
 */
void UserDataObjectDescriptionMap::clear()
{
	map_.clear();
}

bool UserDataObjectDescriptionMap::isUserDataObject( const std::string& name ) const
{
	return map_.find( name ) != map_.end();
}
/* user_data_object_description_map.cpp */
