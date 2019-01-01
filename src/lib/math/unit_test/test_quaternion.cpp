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

#include "math/quat.hpp"

TEST( Quaternion_testConstruction )
{	
	// test constructors
	Quaternion q1( Matrix::identity );

	CHECK_EQUAL( 0.0f, q1.x );
	CHECK_EQUAL( 0.0f, q1.y );
	CHECK_EQUAL( 0.0f, q1.z );
	CHECK_EQUAL( 1.0f, q1.w );

	Quaternion q2( 1.0f, 2.0f, 3.0f, 4.0 );
	CHECK_EQUAL( 1.0f, q2.x );
	CHECK_EQUAL( 2.0f, q2.y );
	CHECK_EQUAL( 3.0f, q2.z );
	CHECK_EQUAL( 4.0f, q2.w );

	Quaternion q3( Vector3(1.0f,2.0f,3.0f), 4.0f );
	CHECK_EQUAL( 1.0f, q3.x );
	CHECK_EQUAL( 2.0f, q3.y );
	CHECK_EQUAL( 3.0f, q3.z );
	CHECK_EQUAL( 4.0f, q3.w );
}

TEST( Quaternion_testSet )
{
	// test setting

	Quaternion q1(1.0f,2.0f,3.0f,4.0f);
	q1.setZero();
	CHECK_EQUAL( 0.0f, q1.x );
	CHECK_EQUAL( 0.0f, q1.y );
	CHECK_EQUAL( 0.0f, q1.z );
	CHECK_EQUAL( 0.0f, q1.w );

	Quaternion q2;
	q2.set( 1.0f, 2.0f, 3.0f, 4.0f );
	CHECK_EQUAL( 1.0f, q2.x );
	CHECK_EQUAL( 2.0f, q2.y );
	CHECK_EQUAL( 3.0f, q2.z );
	CHECK_EQUAL( 4.0f, q2.w );

	Quaternion q3;
	q3.set( Vector3(1.0f,2.0f,3.0f), 4.0f );
	CHECK_EQUAL( 1.0f, q3.x );
	CHECK_EQUAL( 2.0f, q3.y );
	CHECK_EQUAL( 3.0f, q3.z );
	CHECK_EQUAL( 4.0f, q3.w );
}

TEST( Quaternion_testEqual )
{
	Quaternion q1( 1.0f, 2.0f, 3.0f, 4.0f );
	Quaternion q2( 1.0f, 2.0f, 3.0f, 4.0f );
	Quaternion q3( 1.0f, 2.0f, 3.0f, 5.0f );

	// equal
	CHECK( q1 == q2 );
	CHECK( q2 == q1 );

	// not equal
	CHECK( !( q1 == q3 ) );
	CHECK( !( q3 == q1 ) );
}

TEST( Quaternion_testLength )
{
	Quaternion q1( 1.0f, 2.0f, 3.0f, 4.0f );
	float lsTest = 1.0f + 4.0f + 9.0f + 16.0f ;

	CHECK( almostEqual( q1.lengthSquared(), lsTest ) );
	CHECK( almostEqual( q1.length() ,sqrtf( lsTest ) ) );
	CHECK( almostEqual( q1.lengthSquared(), lsTest ) );
	CHECK( almostEqual( q1.length(), sqrtf( lsTest ) ) );
}

TEST( Quaternion_testIndexing )
{
	Quaternion q( 1.0f, 2.0f, 3.0f, 4.0f );

	// read
	CHECK_EQUAL( 1.0f, q[0] );
	CHECK_EQUAL( 2.0f, q[1] );
	CHECK_EQUAL( 3.0f, q[2] );
	CHECK_EQUAL( 4.0f, q[3] );

	// write
	q[0] = 4.0f;
	q[1] = 3.0f;
	q[2] = 2.0f;
	q[3] = 1.0f;

	CHECK_EQUAL( 4.0f, q[0] );
	CHECK_EQUAL( 3.0f, q[1] );
	CHECK_EQUAL( 2.0f, q[2] );
	CHECK_EQUAL( 1.0f, q[3] );
}

TEST( Quaternion_testFromAngleAxis )
{
	Quaternion q; q.setZero();

	// Rotate 0 radians around z axis.
	q.fromAngleAxis( 0, Vector3( 0.0f, 0.0f, 1.0f ) );

	// Should be identity quaternion
	CHECK( almostEqual( 0.0f, q.x ) );
	CHECK( almostEqual( 0.0f, q.y ) );
	CHECK( almostEqual( 0.0f, q.z ) );
	CHECK( almostEqual( 1.0f, q.w ) );

	// Rotate pi/2 radians around x axis

	q.fromAngleAxis( MATH_PI/2.0f, Vector3( 1.0f, 0.0f, 0.0f ) );
	CHECK( almostEqual( 0.707107f,	q.x ) );
	CHECK( almostEqual( 0.0f,		q.y ) );
	CHECK( almostEqual( 0.0f,		q.z ) );
	CHECK( almostEqual( 0.707107f,	q.w ) );	
}

TEST( Quaternion_testFromMatrix )
{
   Quaternion q; q.setZero();

   // Set from identity matrix
   q.fromMatrix( Matrix::identity );

   // Should be identity quaternion
   CHECK( almostEqual( 0.0f, q.x ) );
   CHECK( almostEqual( 0.0f, q.y ) );
   CHECK( almostEqual( 0.0f, q.z ) );
   CHECK( almostEqual( 1.0f, q.w ) );
}

TEST( Quaternion_testMultiply )
{
	Quaternion r; r.setZero();
	Quaternion q( 0.707f, 0.0f, 0.0f, 0.707f );
	Quaternion i( 0.0f, 0.0f, 0.0f, 1.0f );

	// Multiply q by identity
	r.multiply( i, q );

	// Should be the same
	CHECK( almostEqual( 0.707107f, q.x ) );
	CHECK( almostEqual( 0.0f,		q.y ) );
	CHECK( almostEqual( 0.0f,		q.z ) );
	CHECK( almostEqual( 0.707107f, q.w ) );

	// Multiply q by identity ( other way )
	r.setZero();
	r.multiply( q, i );

	// Should be the same
	CHECK( almostEqual( 0.707107f, q.x ) );
	CHECK( almostEqual( 0.0f,		q.y ) );
	CHECK( almostEqual( 0.0f,		q.z ) );
	CHECK( almostEqual( 0.707107f, q.w ) );
}

TEST( Quaternion_testNormalise )
{
	// Create a quaternion and set to unit length.
	Quaternion n( 1.0f, 2.0f, 3.0f, 4.0f );
	n.normalise();
	
	CHECK( almostEqual( n.length(), 1.0f ) );
}

TEST( Quaternion_testInvert )
{
	Quaternion r; r.setZero();
	Quaternion i( 0.0f, 0.0f, 0.0f, 1.0f );
	Quaternion q( 0.707f, 0.0f, 0.0f, 0.707f );

	// Invert identity should be identity
	i.invert();

	CHECK( almostEqual( 0.0f, i.x ) );
	CHECK( almostEqual( 0.0f, i.y ) );
	CHECK( almostEqual( 0.0f, i.z ) );
	CHECK( almostEqual( 1.0f, i.w ) );

	// Invert rotation pi/2 around x
	q.invert();

	CHECK( almostEqual( -.707107f, q.x ) );
	CHECK( almostEqual( 0.0f,		q.y ) );
	CHECK( almostEqual( 0.0f,		q.z ) );
	CHECK( almostEqual( 0.707107f, q.w ) );
}

// test_quaterion.cpp
