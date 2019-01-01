/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "log_component_names.hpp"

#include "cstdmf/debug.hpp"

#include <string>

bool LogComponentNames::init( const char *root, const char *mode )
{
	const char *logComponentsPath = this->join( root, "component_names" );
	return TextFileHandler::init( logComponentsPath, mode );
}


void LogComponentNames::flush()
{
	componentNames_.clear();
}


bool LogComponentNames::handleLine( const char *line )
{
	std::string name = line;

	// Ignore any components past our maximum limit
	if (componentNames_.size() >= this->MAX_COMPONENTS)
	{
		CRITICAL_MSG( "LogComponentNames::handleLine:"
			"Dropping component '%u'; max number of components reached",
			this->MAX_COMPONENTS );
	}

	componentNames_.push_back( name );
	return true;
}


/**
 * This method retrieves the ID associated with the specified component name.
 *
 * If no component name -> ID mapping is already known, a new ID will be
 * allocated and written to the components file.
 *
 * @returns ID associated with the component.
 */
int LogComponentNames::getIDFromName( const std::string &componentName )
{
	int id = 0;
	StringList::const_iterator iter = componentNames_.begin();

	// Search for existing records
	while (iter != componentNames_.end())
	{
		if (*iter == componentName)
		{
			return id;
		}

		++iter;
		++id;
	}

	if (id >= (int)MAX_COMPONENTS)
	{
		CRITICAL_MSG( "LogComponentNames::resolve: "
			"You have registered more components than is supported (%d)\n",
			MAX_COMPONENTS );
	}

	// Make a new entry if none existed
	componentNames_.push_back( componentName );
	this->writeLine( componentName.c_str() );

	return id;
}


/**
 * This method retrieves the component name associated to the specified
 * component ID.
 *
 * @returns Component name as a string on success, NULL on error.
 */
const char * LogComponentNames::getNameFromID( int componentID ) const
{
	if (componentID >= (int)componentNames_.size())
	{
		ERROR_MSG( "LogComponentNames::getNameFromID:"
			"Cannot resolve unknown typeid (%d) from %"PRIzu" known records.\n",
			componentID, componentNames_.size() );
		return NULL;
	}

	return componentNames_[ componentID ].c_str();
}



/**
 * This method invokes onComponent on the visitor for all the known component
 * names stored in the component map.
 *
 * @returns true on success, false on error.
 */
bool LogComponentNames::visitAllWith( LogComponentsVisitor &visitor ) const
{
	StringList::const_iterator iter = componentNames_.begin();
	bool status = true;

	while ((iter != componentNames_.end()) && (status == true))
	{
		status = visitor.onComponent( *iter );
		++iter;
	}

	return status;
}
