/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/



#include "packers.hpp"
#include "cstdmf/bw_util.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/packed_section.hpp"
#include "resmgr/xml_section.hpp"
#include "xml_packer.hpp"


// Priority set to the lowest, so it's executed last
IMPLEMENT_PRIORITISED_PACKER( XmlPacker, Packers::LOWEST_PRIORITY )

int XmlPacker_token = 0;

bool XmlPacker::s_shouldEncrypt = false;

bool XmlPacker::prepare( const std::string& src, const std::string& dst )
{
	// Copied the functionality of createAppropriateSection, since it needs to
	// know if it's neither an XMLSection nor a packed section.
	// It adds itself to the packers as a LOWEST_PRIORITY packer. It should be
	// the last one to be executed, so it tries to process any file extension

	// Open the file
	FILE *fh = bw_fopen( src.c_str(), "rb" );
	if ( !fh )
	{
		printf( "Error opening source file: %s\n", src.c_str() );
		return false;
	}

	// Find out length
	fseek( fh, 0, SEEK_END );
	int len = ftell( fh );
	fseek( fh, 0, SEEK_SET );

	// Read file data
	BinaryPtr data = new BinaryBlock( NULL, len, "BinaryBlock/XmlPacker" );
	size_t bytes = fread( data->cdata(), len, 1, fh );
	fclose( fh );
	if ( !bytes )
	{
		printf( "Error reading source file: %s\n", src.c_str() );
		return false;
	}

	ds_ = DataSection::createAppropriateSection( "root", data );

	if ( !ds_ )
		return false; // not an XML file

	if (!ds_->canPack() && !s_shouldEncrypt)
	{
		return false;
	}

	src_ = src;
	dst_ = dst;

	return true;
}

bool XmlPacker::print()
{
	if ( !ds_ )
	{
		printf( "Error: XmlPacker Not initialised properly\n" );
		return false; // error: datasection not created
	}

	printRecursive( ds_ );
	return true;
}

bool XmlPacker::pack()
{
	if ( !ds_ )
	{
		printf( "Error: XmlPacker Not initialised properly\n" );
		return false; // error: datasection not created
	}

	if ( src_.empty() || dst_.empty() )
	{
		printf( "Error: XmlPacker Not initialised properly\n" );
		return false;
	}

	// Convert data section. If its an text XML file, it will be packed and
	// copied. If it's already packed, then it will just copy it.
	std::vector< std::string > stripSections;
	stripSections.push_back( "metaData" ); // particles are .xml files, so remove metadata.
	return PackedSection::convert( src_, dst_, &stripSections, s_shouldEncrypt );
}

void XmlPacker::printRecursive( DataSectionPtr pDS, int indent /*= 0*/ )
{
	for (int i = 0; i < indent; ++i)
	{
		printf( "  " );
	}
	printf( "%s: '%s'\n",
			pDS->sectionName().c_str(),
			pDS->asString().c_str() );

	DataSection::iterator iter = pDS->begin();

	while (iter != pDS->end())
	{
		printRecursive( *iter, indent + 1 );

		++iter;
	}
}
