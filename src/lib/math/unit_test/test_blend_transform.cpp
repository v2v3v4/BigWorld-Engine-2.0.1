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

#include "cstdmf/cstdmf.hpp"
#include "math/blend_transform.hpp"

namespace 
{
	struct Fixture
	{
		Fixture()
		{
			new CStdMf;
		}

		~Fixture()
		{
			delete CStdMf::pInstance();
		}

		BlendTransform transform_;
	};
}

TEST_F( Fixture, BlendTransform_testValid )
{	
	// Identity matrix should be valid
	BlendTransform bt( Matrix::identity );
	CHECK( bt.valid() );
}

TEST_F( Fixture, BlendTransform_testValid2 )
{
	Quaternion	okRotation( 0.70711f, 0.0f, 0.0f, 0.70711f );
	Vector3		okScale( 10.0f, 1.0f, 1.5f );
	Vector3		okTranslation( -10.0f, 10.0f, -10.f );

	Quaternion	nonUnitRotation( 1.0f, -1.0f, -1.0f, 1.0f );
	Vector3		badScale( 1100.0f, -1100.0f, 10.0f );
	Vector3		badTranslation( 250.0f, -250.0f, 10.0f );

	// Control test - this should be fine
	BlendTransform bt1( okRotation,
						okScale,
						okTranslation );
	CHECK( bt1.valid() );

	// Bad rotation, should not be valid
	BlendTransform bt2(	nonUnitRotation,
						okScale,
						okTranslation );
	CHECK( !bt2.valid() );

	// Bad scale, should not be valid
	BlendTransform bt3(	okRotation,
						badScale,
						okTranslation );
	CHECK( !bt3.valid() );

	// Bad translation, should not be valid
	BlendTransform bt4( okRotation,
						okScale,
						badTranslation );

	CHECK( !bt4.valid() );
}
