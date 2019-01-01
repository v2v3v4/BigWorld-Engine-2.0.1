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

#include "physics2/bsp.hpp"

// Not compiled on server, due to unusual linkage issues to be fixed later.
#ifndef MF_SERVER

TEST( BSPTree_Construction )
{
	// No triangles
	RealWTriangleSet empty;
	
	// One triangle
	RealWTriangleSet single;
	single.push_back( WorldTriangle( Vector3::zero(),
									 Vector3( 1.0f, 2.0f, 3.0f ),
									 Vector3( 3.0f, 2.0f, 1.0f ),
										0 ) );

	// Store number of nodes and triangles
	int nodes = 0, tris = 0;

	// Empty BSP is empty and has no root
	BSPTree emptyBsp( empty );
	CHECK_EQUAL( true ,		emptyBsp.empty() );

	// Empty BSP has one node and zero triangles
	emptyBsp.pRoot()->getNumNodes( nodes, tris);
	CHECK_EQUAL( 1,			nodes );
	CHECK_EQUAL( 0,			tris );

	// Single BSP is not empty
	BSPTree singleBsp( single );
	CHECK_EQUAL( false,		singleBsp.empty() );
	
	// Single BSP has two nodes and one triangle
	singleBsp.pRoot()->getNumNodes( nodes, tris);
	CHECK_EQUAL( 2,			nodes );
	CHECK_EQUAL( 1,			tris );
}

TEST( BSP_Intersection )
{
	// create a simple tree with a single triangle on the x plane
	RealWTriangleSet		single;
	single.push_back( 
		WorldTriangle( 
			Vector3::zero(),
			Vector3( 0.0f, 0.0f, 5.0f ),
			Vector3( 0.0f, 5.0f, 0.0f ),
			0 ) 
		);
	BSPTree					simpleBSP( single );
	float					interval = 1.0f;
	const WorldTriangle*	pHitTriangle;
	
	// test intersection against a ray
	bool hit = simpleBSP.pRoot()->intersects(	Vector3(-5.0f, 0.0f, 2.0f ),
												Vector3( 5.0f, 2.0f, 0.0f ),
												interval,
												&pHitTriangle );

	CHECK_EQUAL( true, hit );
	CHECK_EQUAL( 0.5f, interval );
};

class TestVisitor: public CollisionVisitor
{
public:
	TestVisitor()
		: numHits_( 0 )
	{
	}

	virtual bool visit( const WorldTriangle & hitTriangle, float dist )
	{
		numHits_++;		
		return numHits_ == 2; // terminate after two hits
	}

	uint32 numHits_;
};

TEST( BSP_Multiple_Intersection )
{
	// create a simple tree with two coplanar triangles on the x plane
	RealWTriangleSet		multiple;
	multiple.push_back( 
		WorldTriangle( 
			Vector3::zero(),
			Vector3( 0.0f, 0.0f, 5.0f ),
			Vector3( 0.0f, 5.0f, 0.0f ),
			0 ) 
		);
	multiple.push_back( 
		WorldTriangle( 
			Vector3::zero(),
			Vector3( 0.0f, 0.0f, 6.0f ),
			Vector3( 0.0f, 6.0f, 0.0f ),
			0 ) 
		);

	BSPTree					simpleBSP( multiple );
	float					interval = 1.0f;
	TestVisitor				visitor;

	// test intersection against a ray - visitor should return a hit for each
	// triangle (even though they are coplanar).
	bool hit = simpleBSP.pRoot()->intersects(	Vector3(-5.0f, 2.0f, 2.0f ),
												Vector3( 5.0f, 2.0f, 2.0f ),
												interval,
												NULL,
												&visitor );
	CHECK_EQUAL( true,	hit );
	CHECK_EQUAL( 0.5f,	interval );
	CHECK_EQUAL( 2,		visitor.numHits_ );
}

#endif
