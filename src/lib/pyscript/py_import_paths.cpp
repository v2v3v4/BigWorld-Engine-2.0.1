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

#include "py_import_paths.hpp"

#include <sstream>


/**
 *	Constructor.
 *
 *	@param delimiter 	The delimiter to use when converting to a paths string.
 *	@param prefix 		A prefix to use for each of the paths when converting
 *						to a paths string.
 */
PyImportPaths::PyImportPaths( char delimiter, const std::string & prefix ) :
		paths_(), 
		delimiter_( delimiter ), 
		prefix_( prefix )
{}


/**
 *	Return a string containing all the import paths in the format expected by
 *	Python.
 */
const std::string PyImportPaths::pathsAsString() const
{
	std::ostringstream oss;

	OrderedPaths::const_iterator iPath = paths_.begin();
	bool first = true;
	while (iPath != paths_.end())
	{
		if (!first)
		{
			oss << delimiter_;
		}

		if ((*iPath)[0] == '/')
		{
			// absolute path, don't add prefix
			oss << *iPath;
		}
		else
		{
			oss << prefix_ << *iPath;
		}
		first = false;
		++iPath;
	}

	return oss.str();
}


/**
 *	Adds a path.
 */
void PyImportPaths::addPath( const std::string & path )
{
	paths_.push_back( path );
}


/**
 *	Appends the input PyImportPaths to this PyImportPaths, ensuring that the other
 *	paths are after the last path in this collection.
 */
void PyImportPaths::append( const PyImportPaths & other )
{
	OrderedPaths::const_iterator iOtherPath = other.paths_.begin();

	while (iOtherPath != other.paths_.end())
	{
		this->addPath( *iOtherPath );
		++iOtherPath;
	}
}


// python_paths.cpp
