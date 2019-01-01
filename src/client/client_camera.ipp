/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// client_camera.ipp

#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif


INLINE Entity* Targeting::hasAnAutoAimTarget( float& autoAimTargetDistance, float& autoAimTargetAngle,
								bool useFriction, bool wantHorizontalAngle )
{
	BW_GUARD;
	EntityPicker& picker = EntityPicker::instance();
	return picker.hasAnAutoAimTarget( autoAimTargetDistance, autoAimTargetAngle,
										useFriction, wantHorizontalAngle );
}




// client_camera.ipp
