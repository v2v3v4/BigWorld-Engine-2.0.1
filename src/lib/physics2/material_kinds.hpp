/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MATERIAL_KINDS_HPP
#define MATERIAL_KINDS_HPP

#include "cstdmf/init_singleton.hpp"
#include "cstdmf/stringmap.hpp"
#include "resmgr/datasection.hpp"
#include <map>

/**
 *	This singleton class handles loading and parsing the material
 *	kinds xml file, and provides accessors to the information
 *	contained therein.
 */
class MaterialKinds : public InitSingleton<MaterialKinds>
{
public:
	MaterialKinds();

	typedef std::map< uint32, DataSectionPtr > Map;

	//retrieve material kind ID by texture Name
	uint32 get( const std::string& textureName );

	//retrieve a string associated with a material kind
	std::string userString( uint32 materialKind,
		const std::string& keyName );

	//retrieve all data associated with a material kind
	DataSectionPtr userData( uint32 materialKind );

	//retrieve data associated with a material kind's texture.
	DataSectionPtr textureData( uint32 materialKind,
		const std::string& textureName );

	//indicate whether the provided material kind is valid
	bool isValid( uint32 materialKind );

	//the list of material kinds
	const MaterialKinds::Map& materialKinds() const
	{
		return materialKinds_;
	}

#ifdef EDITOR_ENABLED
	///Populate a data section with ID, Description pairs
	void populateDataSection( DataSectionPtr pSection );
	///Copy material kinds to a string map
	void createDescriptionMap( StringHashMap<uint32>& retMap );
	///Reload material kinds
	void reload();
#endif	//EDITOR_ENABLED

protected:
	bool doInit();
	bool doFini();

private:
	void addMaterialKind( DataSectionPtr pSection );
	static std::string format( const std::string& str );
	StringHashMap<uint32> textureToID_;
	MaterialKinds::Map materialKinds_;

	typedef MaterialKinds This;
};

#endif
