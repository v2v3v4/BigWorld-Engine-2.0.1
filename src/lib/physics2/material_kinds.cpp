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
#include "material_kinds.hpp"
#include "cstdmf/debug.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/string_provider.hpp"

DECLARE_DEBUG_COMPONENT2( "romp", 0 )

// Implementation of the singleton static pointer.
BW_INIT_SINGLETON_STORAGE( MaterialKinds );

static AutoConfigString s_materialKinds( "environment/materialKinds" );


/**
 *	This method retuns the material kind for a given texture resource name.
 *
 *	@param	textureName		texture resource name.
 *	@return uint32			material kind for the resource, or 0 by default.
 */
uint32 MaterialKinds::get( const std::string& textureName )
{
	uint32 kind = 0;
	StringHashMap<uint32>::iterator it = textureToID_.find( format( textureName ) );
    if (it != textureToID_.end())
		kind = it->second;
	return kind;
}


/**
 *	This method returns a user string associated with a material kind.
 *
 *	@param	materialKind				id of the materialKind
 *	@param	keyName						name of the user string requested.
 *	@return the requested string, or "invalid material kind"
 */
std::string MaterialKinds::userString( uint32 materialKind,
											 const std::string& keyName )
{
	DataSectionPtr pSection = this->userData( materialKind );
	if (pSection)
	{
		return pSection->readString( keyName );
	}

	std::string err = LocaliseUTF8(L"PHYSICS/MATERIAL_KINDS/INVALID_MATERIAL_KIND");
	return err;
}


/**
 *	This method retuns the data section associated with a material kind.
 *
 *	@param	materialKind	id of the materialKind
 *	@return DataSectionPtr	the data section associated with the material kind.
 */
DataSectionPtr MaterialKinds::userData( uint32 materialKind )
{
	std::map<uint32,DataSectionPtr>::iterator it = materialKinds_.find( materialKind );
	if (it != materialKinds_.end())
	{
		return it->second;
	}
	return NULL;
}


/**
 *	This method retuns the data section associated with a material kind's
 *	texture map.
 *
 *	@param	materialKind	id of the materialKind
 *	@param	textureName		resource id of the texture map
 *	@return DataSectionPtr	the data section associated with the texture map.
 */
DataSectionPtr MaterialKinds::textureData( uint32 materialKind,
										const std::string& textureName )
{
	std::map<uint32,DataSectionPtr>::iterator it = materialKinds_.find( materialKind );
	if (it != materialKinds_.end())
	{
		//the material kinds.xml file contains no terrain map extensions.
		std::string baseFilename = BWResource::removeExtension(textureName);

		DataSectionPtr pMK = it->second;
		DataSection::iterator tit = pMK->begin();
		DataSection::iterator ten = pMK->end();
		while ( tit != ten )
		{
			if ( (*tit)->asString() == baseFilename )
			{
				return *tit;
			}
			++tit;
		}
	}
	return NULL;
}


/**
 *  Indicates whether the provided material kind is valid or not.
 *
 *  @param	materialKind	id of the materialKind
 *  @return bool			validity
 */
bool MaterialKinds::isValid( uint32 materialKind )
{
	Map::iterator it = materialKinds_.find( materialKind );
	return it != materialKinds_.end();
}


/**
 *	Constructor.
 */
MaterialKinds::MaterialKinds()
{
}


/**
 *	This method initialises any objects that require initialisation in the
 *	library.
 *
 *	@return		True if init went on without errors, false otherwise.
 */
bool MaterialKinds::doInit()
{
	DataSectionPtr pSection = BWResource::openSection( s_materialKinds );
	if (pSection)
	{
		DataSectionIterator it = pSection->begin();
		while (it != pSection->end())
		{
			this->addMaterialKind( *it );
			it++;
		}

		INFO_MSG( "Material Kinds loaded from %s\n",
			s_materialKinds.value().c_str() );
	}
	else
	{
		ERROR_MSG( "Material Kinds file not found at %s\n",
			s_materialKinds.value().c_str() );
		return false;
	}
	return true;
}


/**
 *	This method finalises any objects that require finalises in the
 *	library, releasing resources.
 *
 *	@return		True if fini went on without errors, false otherwise.
 */
bool MaterialKinds::doFini()
{
	return true;
}


/**
 *	This private method adds a material kind to our map.
 */
void MaterialKinds::addMaterialKind( DataSectionPtr pSection )
{
	uint32 id = pSection->readInt( "id" );
	DataSectionIterator it = pSection->begin();
	while (it != pSection->end())
	{
		DataSectionPtr pSection = *it++;
		if (pSection->sectionName() == "terrain")
		{
			textureToID_[ pSection->asString() ] = id;
		}
	}
	materialKinds_[ id ] = pSection;
}


/**
 *	This private method formats a texture resource name by normalising
 *	all slashes and removing the extension.
 */
std::string MaterialKinds::format( const std::string& str )
{
	std::string s = str;
	int index = -1;
	while ((index = s.find_first_of('\\')) >= 0)
	{
		s[index] = '/';
	}
	index  = s.find_first_of('.');
	if (index > 0)
	{
		s = s.substr( 0, index );
	}
	return s;
}



#ifdef EDITOR_ENABLED
/**
 *	This method is used by editors to populate a combo box with
 *	ID, Description pairs for material kinds.  The passed in
 *	datasection ends up containing a list of all material kinds.
 */
void MaterialKinds::populateDataSection( DataSectionPtr pSection )
{
	std::map<uint32, DataSectionPtr>::iterator it = materialKinds_.begin();
	while (it != materialKinds_.end())
	{
		pSection->writeInt( it->second->readString("desc"), it->first );
		it++;
	}
}

/**
 *	This method creates a description map (string -> int) used by WorldEditor.
 */
void MaterialKinds::createDescriptionMap( StringHashMap<uint32>& retMap )
{
	std::map<uint32, DataSectionPtr>::iterator it = materialKinds_.begin();
	while (it != materialKinds_.end())
	{
		retMap.insert( std::make_pair(it->second->readString("desc").c_str(),it->first) );
		it++;
	}
}

/**
 *	This method reloads material kinds, but note it does not kick on any
 *	dependent tasks, (like recreating the dominant texture maps).  That is
 *	up to the caller.
 */
void MaterialKinds::reload()
{
	materialKinds_.clear();
	textureToID_.clear();
	BWResource::instance().purge( s_materialKinds.value() );	
	this->doInit();
}

#endif //EDITOR_ENABLED
