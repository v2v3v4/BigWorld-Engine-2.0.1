/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LOG_COMPONENT_NAMES_HPP
#define LOG_COMPONENT_NAMES_HPP

#include "text_file_handler.hpp"

#include <vector>
#include <string>


/**
 * This abstract class provides a template which can be used to retrieve
 * the names of components that have generated logs.
 */
class LogComponentsVisitor
{
public:
	virtual bool onComponent( const std::string &componentName ) = 0;
};


/**
 * Handles the mapping between ids and component names, i.e. 0 -> cellapp,
 * 1 -> baseapp etc.  This is shared amongst all users, and is based on the
 * order in which messages from unique components are delivered.
 */
class LogComponentNames : public TextFileHandler
{
public:
	static const unsigned MAX_COMPONENTS = 31;

	bool init( const char *root, const char *mode );

	virtual void flush();
	virtual bool handleLine( const char *line );

	int getIDFromName( const std::string &name );
	const char * getNameFromID( int componentID ) const;

	bool visitAllWith( LogComponentsVisitor &visitor ) const;

private:
	typedef std::vector< std::string > StringList;
	StringList componentNames_;
};

#endif // LOG_COMPONENT_NAMES_HPP
