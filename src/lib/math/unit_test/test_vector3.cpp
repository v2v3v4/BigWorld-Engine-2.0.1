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

#include "math/vector3.hpp"

TEST( Vector3_testConstruction )
{	
	// test constructors
	Vector3 v1(1.0f,2.0f,3.0f);
	CHECK_EQUAL( 1.0f, v1.x );
	CHECK_EQUAL( 2.0f, v1.y );
	CHECK_EQUAL( 3.0f, v1.z );

	Vector3 v2( v1 );
	CHECK_EQUAL( 1.0f, v2.x );
	CHECK_EQUAL( 2.0f, v2.y );
	CHECK_EQUAL( 3.0f, v2.z );
}

TEST( Vector3_testSet )
{
	Vector3 v1;

	// test setting
	v1.setZero();
	CHECK_EQUAL( 0.0f, v1.x );
	CHECK_EQUAL( 0.0f, v1.y );
	CHECK_EQUAL( 0.0f, v1.z );

	v1.set( 1.0f, 2.0f, 3.0f );
	CHECK_EQUAL( 1.0f, v1.x );
	CHECK_EQUAL( 2.0f, v1.y );
	CHECK_EQUAL( 3.0f, v1.z );
}

TEST( Vector3_testAlmostEqual )
{
	Vector3 v1( 1.0f, 0.0f, 0.0f );

	// AlmostEqual epsilon is hard-coded to 0.0004f. This will detect if that
	// changes.
	Vector3 yes( 1.0004f, 0.0f, 0.0f );
	Vector3 no( 1.0005f, 0.0f, 0.0f );

	CHECK( almostEqual( v1, yes) == true );
	CHECK( almostEqual( v1, no) == false );
}

TEST( Vector3_testDotProduct )
{
	Vector3 v1;

	// zero dot product
	v1.setZero();
	CHECK_EQUAL( 0.0f, v1.dotProduct( v1 ) );

	// unit vector product
	v1.set( 1.0f, 1.0f, 1.0f );
	CHECK_EQUAL( 3.0f, v1.dotProduct( v1 ) );

}

TEST( Vector3_testNormaliseAndLength )
{
	Vector3 v1;

	// zero
	v1.setZero();
	CHECK( almostEqual( 0.0f, v1.length() ) );

	// unit length
	v1.set( 1.0f, 0.0f, 0.0f );
	CHECK( almostEqual( 1.0f, v1.length() ) );

	// unit length, normalised
	v1.normalise();
	CHECK( almostEqual( 1.0f, v1.length() ) );

	// other simple normalised
	v1.set( 1.0f, 2.0f, 3.0f );
	v1.normalise();
	CHECK( almostEqual(1.0f, v1.length()) );
}

TEST( Vector3_testLerp )
{
	// test simple lerp
	Vector3 v1(0.0f,0.0f,0.0f), v2( 1.0f, 2.0f, 3.0f ), rv;
	rv.lerp( v1, v2, 0.5f );

	CHECK_EQUAL( 0.5f, rv.x );
	CHECK_EQUAL( 1.0f, rv.y );
	CHECK_EQUAL( 1.5f, rv.z );
}

TEST( Vector3_testIndexing )
{
	Vector3 v( 1.0f, 2.0f, 3.0f );

	CHECK_EQUAL( v.x, v[0] );
	CHECK_EQUAL( v.y, v[1] );
	CHECK_EQUAL( v.z, v[2] );
}
// test_vector3.cpp
