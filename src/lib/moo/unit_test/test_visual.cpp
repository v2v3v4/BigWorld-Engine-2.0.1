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
#include "moo/visual.hpp"
#include "physics2/bsp.hpp"

TEST( BSPTreeHelper_testCreateVertexList )
{
	// No triangles
	RealWTriangleSet empty;
	BSPTree emptyBsp( empty );

	// One triangle, no collision
	RealWTriangleSet singleNoCollide;
	singleNoCollide.push_back( WorldTriangle( Vector3::zero(),
		Vector3( 1.0f, 2.0f, 3.0f ),
		Vector3( 3.0f, 2.0f, 1.0f ),
		TRIANGLE_NOCOLLIDE ) );
	BSPTree singleNoCollideBsp( singleNoCollide );

	// One triangle, collision
	RealWTriangleSet singleCollide;
	singleCollide.push_back( WorldTriangle( Vector3::zero(),
		Vector3( 1.0f, 2.0f, 3.0f ),
		Vector3( 3.0f, 2.0f, 1.0f ),
		0 ) );
	BSPTree singleCollideBsp( singleCollide );

	// Test Empty
	std::vector<Moo::VertexXYZL> results;
	Moo::BSPTreeHelper::createVertexList( emptyBsp, results );
	CHECK_EQUAL( 0, results.size() );

	// Test Single (no collision)
	results.resize(0);
	Moo::BSPTreeHelper::createVertexList( singleNoCollideBsp, results );
	CHECK_EQUAL( 0, results.size() );

	// Test Single (with collision)
	results.resize(0);
	Moo::BSPTreeHelper::createVertexList( singleCollideBsp, results );
	CHECK_EQUAL( 3, results.size() );

	CHECK_EQUAL( results[0].pos_ , singleCollideBsp.triangles()[0].v0() );
	CHECK_EQUAL( results[1].pos_ , singleCollideBsp.triangles()[0].v1() );
	CHECK_EQUAL( results[2].pos_ , singleCollideBsp.triangles()[0].v2() );
};