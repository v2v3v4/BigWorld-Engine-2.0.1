/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_DESCRIPTION_MAP_HPP
#define ENTITY_DESCRIPTION_MAP_HPP

#include "entity_description.hpp"
#include "resmgr/datasection.hpp"

#include <map>
#include <vector>

class MD5;

/**
 * 	This class parses the entities.xml file, and stores a map of entity
 * 	descriptions.  It provides access to entity descriptions by their name, or
 * 	by index. It also provides fast mapping from name to index.
 *
 * 	@ingroup entity
 */
class EntityDescriptionMap
{
public:
	EntityDescriptionMap();
	~EntityDescriptionMap();
	bool 	parse( DataSectionPtr pSection );
	bool	nameToIndex( const std::string& name, EntityTypeID& index ) const;
	int		size() const;

	const EntityDescription&	entityDescription( EntityTypeID index ) const;

	void addToMD5( MD5 & md5 ) const;
	void addPersistentPropertiesToMD5( MD5 & md5 ) const;

	void clear();
	bool isEntity( const std::string& name ) const;
	typedef std::map<std::string, unsigned int> DescriptionMap;
	DescriptionMap::const_iterator begin() const { return map_.begin(); }
	DescriptionMap::const_iterator end() const{ return map_.end(); }
private:
	bool checkCount( const char * description,
		unsigned int (EntityDescription::*fn)() const,
		int maxEfficient, int maxAllowed ) const;

	typedef std::vector<EntityDescription> DescriptionVector;


	DescriptionVector 	vector_;
	DescriptionMap 		map_;
};

#endif // ENTITY_DESCRIPTION_MAP_HPP
