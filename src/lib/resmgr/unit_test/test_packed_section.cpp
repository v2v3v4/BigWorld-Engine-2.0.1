/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "stdafx.h"

#include "test_harness.hpp"

#include "cstdmf/cstdmf.hpp"
#include "resmgr/packed_section.hpp"
#include "resmgr/xml_section.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/multi_file_system.hpp"
#include "cstdmf/debug.hpp"


class Fixture : public ResMgrUnitTestHarness
{
public:
	Fixture():
		ResMgrUnitTestHarness(),
		fixture_( NULL )
	{
		new CStdMf;
		// Convert and open section
		const char * sourceXMLFile = "test_xml_section.xml";
		const char * fixtureFile = "test_packed_section";
		BWResource::instance().fileSystem()->eraseFileOrDirectory( fixtureFile );
		if (PackedSection::convert( BWResolver::resolveFilename( sourceXMLFile ),
									BWResolver::resolveFilename( fixtureFile ) ))
		{
			fixture_ = BWResource::openSection( fixtureFile );
		}
	}

	~Fixture()
	{
		fixture_ = NULL;
		BWResource::instance().purgeAll();	// Clear any cached values
		delete CStdMf::pInstance();
	}

	DataSectionPtr fixture_;
};

// Conversion tests

TEST_F( Fixture, PackedSection_AsBool )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	DataSectionPtr boolSection = fixture_->openSection( "test_bool" );
	CHECK( boolSection != NULL );

	CHECK_EQUAL( true, boolSection->asBool() );
}

TEST_F( Fixture, PackedSection_AsInt )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	
	DataSectionPtr int8Section = fixture_->openSection( "test_int8" );
	CHECK( int8Section != NULL );
	CHECK_EQUAL( std::numeric_limits<int8>::min(), (int8)int8Section->asInt() );

	DataSectionPtr int16Section = fixture_->openSection( "test_int16" );
	CHECK( int16Section != NULL );
	CHECK_EQUAL( std::numeric_limits<int16>::min(), (int16)int16Section->asInt() );

	DataSectionPtr int32Section = fixture_->openSection( "test_int32" );
	CHECK( int32Section != NULL );
	CHECK_EQUAL( std::numeric_limits<int32>::min(), int32Section->asInt() );
}

TEST_F( Fixture, PackedSection_AsUInt )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	
	DataSectionPtr uint8Section = fixture_->openSection( "test_uint8" );
	CHECK( uint8Section != NULL );
	CHECK_EQUAL( std::numeric_limits<uint8>::max(), (uint8)uint8Section->asInt() );

	DataSectionPtr uint16Section = fixture_->openSection( "test_uint16" );
	CHECK( uint16Section != NULL );
	CHECK_EQUAL( std::numeric_limits<uint16>::max(), (uint16)uint16Section->asInt() );

	DataSectionPtr uint32Section = fixture_->openSection( "test_uint32" );
	CHECK( uint32Section != NULL );
	CHECK_EQUAL( std::numeric_limits<uint32>::max(), uint32Section->asUInt() );
}

TEST_F( Fixture, PackedSection_AsLong )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	
	DataSectionPtr longSection = fixture_->openSection( "test_long" );
	CHECK( longSection != NULL );

	CHECK_EQUAL( std::numeric_limits<int32>::min(), longSection->asLong() );
}

TEST_F( Fixture, PackedSection_AsInt64 )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	
	DataSectionPtr int64Section = fixture_->openSection( "test_int64" );
	CHECK( int64Section != NULL );

	CHECK_EQUAL( std::numeric_limits<int64>::min(), int64Section->asInt64() );

	int64Section = fixture_->openSection( "test_int64_2" );
	CHECK( int64Section != NULL );

	CHECK_EQUAL( std::numeric_limits<int64>::max(), int64Section->asInt64() );

	char conversionBuf[ 64 ];
	bw_snprintf( conversionBuf, 63, "%"PRI64, int64Section->asInt64() );
	conversionBuf[ 63 ] = '\0';

	CHECK_EQUAL( std::string( conversionBuf ), int64Section->asString() );
}

TEST_F( Fixture, PackedSection_AsUInt64 )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	
	DataSectionPtr uint64Section = fixture_->openSection( "test_uint64" );
	CHECK( uint64Section != NULL );

	CHECK_EQUAL( std::numeric_limits<uint64>::max(), uint64Section->asUInt64() );

	uint64Section = fixture_->openSection( "test_uint64_2" );
	CHECK( uint64Section != NULL );

	CHECK_EQUAL( uint64( std::numeric_limits<int64>::max() ) + 1, uint64Section->asUInt64() );
}

TEST_F( Fixture, PackedSection_AsFloat )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	
	DataSectionPtr floatSection = fixture_->openSection( "test_float" );
	CHECK( floatSection != NULL );

	CHECK_EQUAL( 3.142f, floatSection->asFloat() );
}

TEST_F( Fixture, PackedSection_AsString )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	
	// test that a long number-like string is read as a string.

	DataSectionPtr strSection = fixture_->openSection( "test_string" );
	CHECK( strSection != NULL );

	CHECK_EQUAL(
			std::string( "000600060006000600060006000300030003000300030003" ),
			strSection->asString() );
}

TEST_F( Fixture, PackedSection_AsVector2 )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	
	DataSectionPtr vec2Section = fixture_->openSection( "test_vector2" );
	CHECK( vec2Section != NULL );

	Vector2 result( vec2Section->asVector2() );
	CHECK_EQUAL( 1.0f, result.x );
	CHECK_EQUAL( 2.0f, result.y );
}

TEST_F( Fixture, PackedSection_AsVector3 )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	
	DataSectionPtr vec3Section = fixture_->openSection( "test_vector3" );
	CHECK( vec3Section != NULL );

	Vector3 result( vec3Section->asVector3() );
	CHECK_EQUAL( 1.0f, result.x );
	CHECK_EQUAL( 2.0f, result.y );
	CHECK_EQUAL( 3.0f, result.z );
}

TEST_F( Fixture, PackedSection_AsVector4 )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	
	DataSectionPtr vec4Section = fixture_->openSection( "test_vector4" );
	CHECK( vec4Section != NULL );

	Vector4 result( vec4Section->asVector4() );
	CHECK_EQUAL( 1.0f, result.x );
	CHECK_EQUAL( 2.0f, result.y );
	CHECK_EQUAL( 3.0f, result.z );
	CHECK_EQUAL( 4.0f, result.w );
}

TEST_F( Fixture, PackedSection_AsMatrix34 )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	
	DataSectionPtr mat34Section = fixture_->openSection( "test_matrix34" );
	CHECK( mat34Section != NULL );

	Matrix result( mat34Section->asMatrix34() );

	CHECK_EQUAL( 1.0f, result[0].x );
	CHECK_EQUAL( 2.0f, result[0].y );
	CHECK_EQUAL( 3.0f, result[0].z );

	CHECK_EQUAL( 4.0f, result[1].x );
	CHECK_EQUAL( 5.0f, result[1].y );
	CHECK_EQUAL( 6.0f, result[1].z );

	CHECK_EQUAL( 7.0f, result[2].x );
	CHECK_EQUAL( 8.0f, result[2].y );
	CHECK_EQUAL( 9.0f, result[2].z );

	CHECK_EQUAL( 10.0f, result[3].x );
	CHECK_EQUAL( 11.0f, result[3].y );
	CHECK_EQUAL( 12.0f, result[3].z );
}

// Conversion tests

TEST_F( Fixture, PackedSection_countChildren_prepacked )
{
	CHECK( this->isOK() );

	CHECK( fixture_ != NULL );
	
 	// Count children for each subsection, using pre-packed section, and compare
	// with xml output

	DataSectionPtr x = BWResource::openSection( "test_xml_section.xml" );
	CHECK( x != NULL );
	CHECK( fixture_->countChildren() > 0 );

	if ( x != NULL )
	{
		CHECK_EQUAL( x->countChildren(), fixture_->countChildren() );
	}
}

TEST_F( Fixture, PackedSection_countChildren_convertInMemory )
{
	CHECK( this->isOK() );

	MultiFileSystemPtr	fileSystem = BWResource::instance().fileSystem();
	fileSystem->eraseFileOrDirectory( "result_packed_section" );

	// Count children for each subsection, after converting from an xml section
	// to a binary section on disk.

	DataSectionPtr	x = BWResource::openSection( "test_xml_section.xml" );
	CHECK( x != NULL );

	if ( x != NULL )
	{
		int		n = x->countChildren();
		bool	r = PackedSection::convert( x , "result_packed_section" );

		CHECK_EQUAL( true, r );

		if ( r )
		{
			CHECK_EQUAL( n, x->countChildren() );
		}
	}
}

TEST_F( Fixture, PackedSection_countChildren_convertOnDisk )
{
	CHECK( this->isOK() );

	MultiFileSystemPtr	fileSystem = BWResource::instance().fileSystem();
	fileSystem->eraseFileOrDirectory( "result_packed_section2" );

	// Count children for each subsection, after converting from an xml section
	// to a binary section on disk. This uses the "other" convert method.

	DataSectionPtr	x = BWResource::openSection( "test_xml_section.xml" );
	std::string		i = BWResolver::resolveFilename( "test_xml_section.xml" );
	std::string		o = BWResolver::resolveFilename( "result_packed_section2" );
	bool			r = PackedSection::convert( i, o, NULL, false );

	CHECK_EQUAL( true, r );

	DataSectionPtr p = BWResource::openSection("result_packed_section2");

	CHECK( p != NULL );

	if ( x != NULL && p != NULL )
	{
		CHECK_EQUAL( x->countChildren(), p->countChildren() );
	}
}

// test_packed_section.cpp
