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

#include "physics2/worldtri.hpp"

struct Fixture
{
	Fixture()
	{
		triangle_ = new WorldTriangle(	Vector3( 0.0f, 0.0f, 0.0f ),
										Vector3( 0.0f, 0.0f, 1.0f ),
										Vector3( 0.0f, 1.0f, 0.0f ) );
	}

	~Fixture()
	{
		delete triangle_;
	}

	WorldTriangle* triangle_;
};

TEST_F( Fixture, WorldTri_Construction )
{
	WorldTriangle wt;

	CHECK( wt.v0()		== Vector3::zero() );
	CHECK( wt.v1()		== Vector3::zero() );
	CHECK( wt.v2()		== Vector3::zero() );
	CHECK( wt.flags()	== 0			   );

	WorldTriangle wt2(	Vector3::zero(),
						Vector3( 1.0f, 2.0f, 3.0f ),
						Vector3( 3.0f, 2.0f, 1.0f ),
						65535 );

	CHECK( wt2.v0()		== Vector3::zero()				);
	CHECK( wt2.v1()		== Vector3( 1.0f, 2.0f, 3.0f )	);
	CHECK( wt2.v2()		== Vector3( 3.0f, 2.0f, 1.0f )	);
	CHECK( wt2.flags()	== 65535						);
}

TEST_F( Fixture, WorldTri_VMethods )
{
	CHECK( triangle_->v0() == triangle_->v(0) );
	CHECK( triangle_->v1() == triangle_->v(1) );
	CHECK( triangle_->v2() == triangle_->v(2) );
}

TEST_F( Fixture, WorldTri_Normal )
{
	Vector3 n = triangle_->normal();

	CHECK_EQUAL( n.x, -1.0f );
	CHECK_EQUAL( n.y, 0.0f );
	CHECK_EQUAL( n.z, 0.0f );
}

TEST_F( Fixture, WorldTri_Flags )
{
	CHECK_EQUAL( false, triangle_->isBlended() );
	CHECK_EQUAL( false, triangle_->isTransparent() );

	triangle_->flags( TRIANGLE_BLENDED | TRIANGLE_TRANSPARENT );

	CHECK_EQUAL( true, triangle_->isBlended() );
	CHECK_EQUAL( true, triangle_->isTransparent() );
}

TEST_F( Fixture, WorldTri_Equality )
{
	WorldTriangle wt;

	CHECK( !(wt == *triangle_) );

	WorldTriangle wt2(	Vector3::zero(),
						Vector3( 0.0f, 0.0f, 1.0f ),
						Vector3( 0.0f, 1.0f, 0.0f )  );

	CHECK( wt2 == *triangle_ );
}

TEST_F( Fixture, WorldTri_Intersects )
{
	// Null triangle should not intersect
	WorldTriangle null;

	CHECK_EQUAL( false, triangle_->intersects( null ) );
	CHECK_EQUAL( false, null.intersects( *triangle_ ) );

	// Good triangle should intersect (on same plane)
	WorldTriangle good(	Vector3( 0.0f, 0.0f, 0.0f ),
						Vector3( 0.0f, 0.0f, 1.0f ),
						Vector3( 0.0f, 1.0f, 1.0f ) );

	CHECK_EQUAL( false, triangle_->intersects( good ) );
	CHECK_EQUAL( false, good.intersects( *triangle_ ) );

	// TODO add more interesting test cases here...
}

// test_particle.cpp
