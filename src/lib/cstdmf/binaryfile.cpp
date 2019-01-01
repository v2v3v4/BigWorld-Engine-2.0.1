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
#include "binaryfile.hpp"


// does some tests of the BinaryFile class
// TODO: Move into unit test
inline bool BinaryFile::test()
{
	{
		// test no-cached file read write
		FILE* fp = bw_fopen( "bin_file.tst", "wb+" );
		if( !fp )
			return false;
		BinaryFile bf( fp );

		int i1, i2;
		std::string s1, s2;
		bf << 1 << "test string" << 2 << "another string";
		fseek( fp, 0l, SEEK_SET );
		bf >> i1 >> s1 >> i2 >> s2;

		MF_ASSERT( i1 == 1 );
		MF_ASSERT( i2 == 2 );
		MF_ASSERT( s1 == "test string" );
		MF_ASSERT( s2 == "another string" );
		MF_ASSERT( bf );
	}
	DeleteFile( L"bin_file.tst" );

	{
		// test cached file read write
		FILE* fp = bw_fopen( "bin_file.tst", "wb+" );
		if( !fp )
			return false;
		BinaryFile bf( fp );

		int i1, i2;
		std::string s1, s2;
		bf << 1 << "test string" << 2 << "another string";

		fseek( fp, 0l, SEEK_SET );
		bf.cache();

		bf >> i1 >> s1 >> i2 >> s2;

		MF_ASSERT( i1 == 1 );
		MF_ASSERT( i2 == 2 );
		MF_ASSERT( s1 == "test string" );
		MF_ASSERT( s2 == "another string" );
		MF_ASSERT( bf );
	}
	DeleteFile( L"bin_file.tst" );

	{
		// test failure
		FILE* fp = bw_fopen( "bin_file.tst", "wb+" );
		if( !fp )
			return false;
		BinaryFile bf( fp );

		std::string s;
		bf << 1024 << 2048;

		fseek( fp, 0l, SEEK_SET );
		bf.cache();

		bf >> s;

		MF_ASSERT( !bf );
	}
	DeleteFile( L"bin_file.tst" );

	return true;
}
