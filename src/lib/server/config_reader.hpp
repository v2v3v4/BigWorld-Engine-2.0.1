/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CONFIG_READER_HPP
#define CONFIG_READER_HPP

#include <map>
#include <string>
#include <vector>

/**
 * This class provides an interface for reading Windows INI style
 * configuration files as used by the server tools.
 */
class ConfigReader
{
public:
	ConfigReader( const char *filename );
	~ConfigReader();

	bool read();

	bool getValue( const std::string section, const std::string key,
				std::string &value ) const;

	static void separateLine( const std::string &line, char sep,
				std::vector< std::string > &resultList );

	const char *filename() const { return filename_.c_str(); }

private:
	ConfigReader();

	typedef std::map< std::string, std::string > SectionEntries;
	typedef std::map< std::string, SectionEntries * > Sections;

	Sections sections_;

	std::string filename_;
	char separator_;

	bool handleLine( const char *line, SectionEntries* & currSection );
};

#endif // CONFIG_READER_HPP
