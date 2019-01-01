/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITYDEF_CONSTANTS_HPP
#define ENTITYDEF_CONSTANTS_HPP


namespace EntityDef
{

class Constants
{
public:
	
	// common constants
	
	inline static const char*
		commonPath() { return "scripts/common"; }

	inline static const char *
		serverCommonPath() { return "scripts/server_common"; }

	inline static const char*
		aliasesFile() { return "scripts/entity_defs/alias.xml"; }

	
	// database constants

	inline static const char*
		databasePath() { return "scripts/db"; }

	inline static const char*
		xmlDatabaseFile() { return "scripts/db.xml"; }


	// bot scripts constants

	inline static const char*
		botScriptsPath() { return "scripts/bot"; }


	// entities constants

	inline static const char*
		entitiesFile() { return "scripts/entities.xml"; }

	inline static const char*
		entitiesDefsPath() { return "scripts/entity_defs"; }

	inline static const char*
		entitiesClientPath() { return "scripts/client"; }

	inline static const char*
		entitiesCellPath() { return "scripts/cell"; }

	inline static const char*
		entitiesBasePath() { return "scripts/base"; }

	inline static const char*
		entitiesEditorPath() { return "scripts/editor"; }

	inline static const char*
		entitiesCapabilitiesFile() { return "scripts/common/capabilities.xml"; }


	// user data objects constants

	inline static const char*
		userDataObjectsFile() { return "scripts/user_data_objects.xml"; }

	inline static const char*
		userDataObjectsDefsPath() { return "scripts/user_data_object_defs"; }

	inline static const char*
		userDataObjectsClientPath() { return "scripts/client"; }

	inline static const char*
		userDataObjectsCellPath() { return "scripts/cell"; }

	inline static const char*
		userDataObjectsBasePath() { return "scripts/base"; }

	inline static const char*
		userDataObjectsEditorPath() { return "scripts/editor"; }


	// marker constants

	inline static const char*
		markerCategoriesFile() { return "scripts/marker_categories.xml"; }

	inline static const char*
		markerEntitiesPath() { return "scripts/marker_entities"; }

};

}


#endif // ENTITYDEF_CONSTANTS_HPP
