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

#include "math/matrix.hpp"
#include "cstdmf/cstdmf.hpp"

struct Fixture
{
	Fixture()
		: matrix_(  Vector4( 0.0f, 1.0f, 2.0f, 3.0f),
					Vector4( 4.0f, 5.0f, 6.0f, 7.0f),
					Vector4( 8.0f, 9.0f,10.0f,11.0f),
					Vector4(12.0f,13.0f,14.0f,15.0f) )
	{
		new CStdMf;
	}

	~Fixture()
	{
		delete CStdMf::pInstance();
	}

	Matrix matrix_;
};

TEST_F( Fixture, Matrix_testConstruction )
{
	// Test initial value, should be a zero matrix
	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{
			matrix_( i, j ) = float(i) * float(j);
		}
	}
}

TEST_F( Fixture, Matrix_testSetZero )
{
	matrix_.setZero();

	// Should be a zero matrix
	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{
			CHECK_EQUAL( 0.0f, matrix_(i,j) );
		}
	}
}

TEST_F( Fixture, Matrix_testIndexing )
{
	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{
			CHECK_EQUAL( float(i) * 4.0f + float(j), matrix_( i, j ) );
		}
	}
}

TEST_F( Fixture, Matrix_testRow )
{	
	// read rows;
	Vector4 r[4];

	for ( uint i = 0; i < 4; i++ )
	{
		r[i] = matrix_.row(i);
	}

	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{
			CHECK_EQUAL( r[i][j], matrix_( i, j ) );
		}
	}

	// write rows
	matrix_.row( 0, Vector4( 1.0f, 0.0f, 0.0f, 0.0f  ) );
	matrix_.row( 1, Vector4( 0.0f, 1.0f, 0.0f, 0.0f  ) );
	matrix_.row( 2, Vector4( 0.0f, 0.0f, 1.0f, 0.0f  ) );
	matrix_.row( 3, Vector4( 0.0f, 0.0f, 0.0f, 1.0f  ) );

	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{
			CHECK_EQUAL( Matrix::identity(i, j) , matrix_( i, j ) );
		}
	}
}

TEST_F( Fixture, Matrix_testSetIdentity )
{
	matrix_.setIdentity();

	// Validate identity
	// Test initial value, should be a zero matrix
	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{
			if ( j == i)
			{
				CHECK_EQUAL( 1.0f, matrix_(i,j) );
			}
			else
			{
				CHECK_EQUAL( 0.0f, matrix_(i,j) );
			}
		}
	}
}

TEST_F( Fixture, Matrix_testTranspose )
{
	Matrix mt;

	// Set up matrix
	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{		
			matrix_(i,j) = float(i) * float(j);
		}
	}

	mt.transpose(matrix_);

	// check matrix
	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{		
			CHECK_EQUAL(  matrix_(i,j) , mt(j,i) );
		}
	}
}

TEST_F( Fixture, Matrix_testMultiply )
{
	Matrix zero, id, x;

	// set up some values
	zero.setZero();
	id.setIdentity();
	// x 
	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{		
			x(i,j) = float(i) * float(j);
		}
	}

	// 0 by 0 = 0
	matrix_.multiply( zero, zero );

	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{		
			CHECK_EQUAL( 0.0f , matrix_(i,j) );
		}
	}

	// 0 by anything = 0
	matrix_.multiply( zero, x );

	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{		
			CHECK_EQUAL( 0.0f , matrix_(i,j) );
		}
	}

	// 0 by anything = 0
	matrix_.multiply( x, zero );

	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{		
			CHECK_EQUAL( 0.0f , matrix_(i,j) );
		}
	}

	// identity by X (if X is non-zero) = X
	matrix_.multiply( id, x );

	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{		
			CHECK_EQUAL( float(i) * float(j) , matrix_(i,j) );
		}
	}

	// identity by X (if X is non-zero) = X
	matrix_.multiply( x, id );

	for ( uint32 i = 0; i < 4; i++ )
	{
		for ( uint32 j = 0; j < 4; j++ )
		{		
			CHECK_EQUAL( float(i) * float(j) , matrix_(i,j) );
		}
	}
}

// test_matrix.cpp
