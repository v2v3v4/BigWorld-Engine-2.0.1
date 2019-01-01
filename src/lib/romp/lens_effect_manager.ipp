/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifdef CODE_INLINE
#define INLINE inline
#else
#define INLINE
#endif


/**
 * This method adds a photon occluder to the lens effects visibility
 * determination.
 */
INLINE void LensEffectManager::addPhotonOccluder( PhotonOccluder * occluder )
{
	photonOccluders_.push_back( occluder );
}


/*lens_effect_manager.ipp*/
