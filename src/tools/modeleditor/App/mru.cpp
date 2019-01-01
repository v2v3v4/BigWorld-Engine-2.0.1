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

#include "mru.hpp"

#include "appmgr/options.hpp"

/*static*/ MRU& MRU::instance()
{
	static MRU s_instance_;
	return s_instance_;
}

MRU::MRU():
	maxMRUs_(10)
{}

void MRU::update( const std::string& mruName, const std::string& file, bool add /* = true */ )
{
	BW_GUARD;

	std::vector<std::string> files;
	if (add)
	{
		files.push_back( file );
	}

	char entry[8];
	std::string tempStr;

	for (unsigned i=0; i<maxMRUs_; i++)
	{
		bw_snprintf( entry, sizeof(entry), "file%d", i );
		tempStr = Options::getOptionString( mruName +"/"+ entry, file );
		Options::setOptionString( mruName +"/"+ entry, "" );
		if ( tempStr != file )
		{
			files.push_back( tempStr );
		}
	}

	for (unsigned i=0; i<min( maxMRUs_, files.size() ); i++)
	{
		bw_snprintf( entry, sizeof(entry), "file%d", i );
		Options::setOptionString( mruName +"/"+ entry, files[i] );
	}

	Options::save();
}

void MRU::read( const std::string& mruName, std::vector<std::string>& files )
{
	BW_GUARD;

	char entry[8];
			
	for (unsigned i=0; i<maxMRUs_; i++)
	{
		bw_snprintf( entry, sizeof(entry), "file%d", i );
		std::string tempStr = Options::getOptionString( mruName +"/"+ entry, "" );
		if ( tempStr != "" )
		{
			files.push_back( tempStr );
		}
	}
}

void MRU::getDir( const std::string& mruName, std::string& dir, const std::string& defaultDir /* = "" */ )
{
	BW_GUARD;

	dir = Options::getOptionString( mruName +"/file0", defaultDir );

	if ( dir != "" )
	{
		dir = BWResource::resolveFilename( dir );
		std::replace( dir.begin(), dir.end(), '/', '\\' );
		dir = dir.substr( 0, dir.find_last_of( "\\" ));
	}
}