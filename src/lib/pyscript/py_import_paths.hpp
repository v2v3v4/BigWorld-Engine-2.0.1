/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_IMPORT_PATHS_HPP
#define PY_IMPORT_PATHS_HPP

#include <string>
#include <vector>

/**
 *	A helper class to generate a Python import paths from path components.
 */
class PyImportPaths
{
public:
	static const char DEFAULT_DELIMITER = ';';

	PyImportPaths( char delimiter = DEFAULT_DELIMITER, 
		const std::string & prefix = "" );

	const std::string pathsAsString() const;

	bool empty() const 
	{ return paths_.empty(); }

	void append( const PyImportPaths & other );

	void addPath( const std::string & path );

	void setPrefix( const std::string & prefix )
	{ prefix_ = prefix; }

	void setDelimiter( char delimiter )
	{ delimiter_ = delimiter; }

private:
	typedef std::vector< std::string > OrderedPaths;
	OrderedPaths paths_;
	char delimiter_;
	std::string prefix_;
};

#endif // PY_IMPORT_PATHS_HPP
