/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef USER_DATA_OBJECT_DESCRIPTION_MAP_HPP
#define USER_DATA_OBJECT_DESCRIPTION_MAP_HPP
#include "resmgr/datasection.hpp"
#include "user_data_object_description.hpp"
#include <map>
class MD5;

/**
 * 	This class parses the UserDataObjects.xml file, and stores a map of UserDataObject
 * 	descriptions.  It provides access to udo descriptions by their name.
 * 	@ingroup udo
 */
class UserDataObjectDescriptionMap
{
public:
	UserDataObjectDescriptionMap();
	virtual ~UserDataObjectDescriptionMap();
	bool 	parse( DataSectionPtr pSection );
	int	size() const;
	const UserDataObjectDescription& udoDescription( std::string UserDataObjectName ) const;
	void clear();
	bool isUserDataObject( const std::string& name ) const;
	typedef std::map<std::string, UserDataObjectDescription> DescriptionMap;
	DescriptionMap::const_iterator begin() const { return map_.begin(); }
	DescriptionMap::const_iterator end() const{ return map_.end(); }
private:
	DescriptionMap 		map_;
};

#endif // USER_DATA_OBJECT_DESCRIPTION_MAP_HPP
