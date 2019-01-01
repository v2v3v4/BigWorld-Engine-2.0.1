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

#include "cstdmf/aalloc.hpp"
#include <time.h>


typedef unsigned int AAllocTestType1;


class AAllocTestType2
{
	char dummyBuffer[ 32768 ];
};


class AAllocTestType3
{
	char dummyBuffer1[ 32768 ];
	char dummyBuffer2[ 32768 ];
	char dummyBuffer3[ 32768 ];
	char dummyBuffer4[ 32768 ];
};


TEST( AAlloc_test_border_case1_32bits )
{
	// This will try to allocate one over the maximum, and should fail (in 32-bit systems).
	AAllocTestType1* pTestBuf = std::_alignedAllocate< AAllocTestType1 >( 1024 * 1024 * 1024, NULL );
	CHECK_EQUAL( (AAllocTestType1*)NULL, pTestBuf );
	_aligned_free( pTestBuf );
}


TEST( AAlloc_test_border_case2_32bits )
{
	// This will try to allocate one over the maximum, and should fail (in 32-bit systems).
	AAllocTestType2* pTestBuf = std::_alignedAllocate< AAllocTestType2 >( 1024 * 128, NULL );
	CHECK_EQUAL( (AAllocTestType2*)NULL, pTestBuf );
	_aligned_free( pTestBuf );
}


TEST( AAlloc_test_size_wrap_32bits )
{
	// This tests 65537 elements of size 128K, which when multiplied exceeds
	// 32bits and gets wrapped to less that 20KB, which was a known bug.
	AAllocTestType3* pTestBuf = std::_alignedAllocate< AAllocTestType3 >( 65537, NULL );
	CHECK_EQUAL( (AAllocTestType3*)NULL, pTestBuf );
	_aligned_free( pTestBuf );
}


TEST( AAlloc_test_pass1 )
{
	// This will allocate around 32mb of data, and should succeed.
	AAllocTestType1* pTestBuf = std::_alignedAllocate< AAllocTestType1 >( 1024 * 1024 * 8, NULL );
	CHECK( pTestBuf != NULL );
	_aligned_free( pTestBuf );
}


TEST( AAlloc_test_pass2 )
{
	// This will allocate around 32mb of data, and should succeed.
	AAllocTestType2* pTestBuf = std::_alignedAllocate< AAllocTestType2 >( 1024, NULL );
	CHECK( pTestBuf != NULL );
	_aligned_free( pTestBuf );
}


TEST( AAlloc_test_pass3 )
{
	// This will allocate around 32mb of data, and should succeed.
	AAllocTestType3* pTestBuf = std::_alignedAllocate< AAllocTestType3 >( 256, NULL );
	CHECK( pTestBuf != NULL );
	_aligned_free( pTestBuf );
}
