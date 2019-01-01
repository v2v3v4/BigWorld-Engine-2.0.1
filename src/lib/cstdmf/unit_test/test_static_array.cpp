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

#include "cstdmf/static_array.hpp"

TEST( StaticArray_testSize )
{	
	StaticArray< uint32, 1 >			unitArray;
	StaticArray< uint8, 1000 * 1000 >	bigArray;

	CHECK_EQUAL( 1U,			unitArray.size() );
	CHECK_EQUAL( 1000U * 1000U,	bigArray.size() );
}

TEST( StaticArray_testIndex )
{
	StaticArray< float, 10 >	floatArray;

	for ( size_t i = 0; i < floatArray.size(); i++ )
	{
		floatArray[ i ] = (float)i;
	}

	for ( size_t i = 0; i < floatArray.size(); i++ )
	{
		CHECK_EQUAL( (float)i, floatArray[ i ] );
	}
}

TEST( StaticArray_testAssign )
{
	StaticArray< int16, 100 >	testArray;

	// Fill with stuff
	for ( size_t i = 0; i < testArray.size(); i++ )
	{
		testArray[ i ] = i + 123;
	}

	// Assign 0
	testArray.assign( 0);
	for ( size_t i = 0; i < testArray.size(); i++ )
	{
		CHECK_EQUAL( 0, testArray[ i ] );
	}

	// Assign 321
	testArray.assign( 321 );
	for ( size_t i = 0; i < testArray.size(); i++ )
	{
		CHECK_EQUAL( 321, testArray[ i ] );
	}	
}

// test_static_array.cpp
