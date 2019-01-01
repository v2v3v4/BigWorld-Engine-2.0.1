/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_FLARE_COLLIDER_HPP
#define ENTITY_FLARE_COLLIDER_HPP

#pragma warning ( disable: 4786 )
#pragma warning ( disable: 4503 )

#include <iostream>
#include "romp/photon_occluder.hpp"
#include "entity.hpp"


/**
 *	This class determines when photons are occluded by an entity.
 *
 *	@see PhotonOccluder
 */
class EntityPhotonOccluder : public PhotonOccluder
{
public:
	EntityPhotonOccluder( Entity & ent );
	~EntityPhotonOccluder();

	float collides(
			const Vector3 & lightSourcePosition,
			const Vector3 & cameraPosition,
			const LensEffect& le );

private:
	float checkTorso(
		const Matrix & objectToClip,
		const Matrix & sunToClip );

	float checkHead(
		const Matrix & objectToClip,
		const Matrix & sunToClip );

	EntityPhotonOccluder(const EntityPhotonOccluder&);
	EntityPhotonOccluder& operator=(const EntityPhotonOccluder&);

	friend std::ostream& operator<<(std::ostream&, const EntityPhotonOccluder&);

	Entity * entity_;
};

#ifdef CODE_INLINE
#include "entity_flare_collider.ipp"
#endif




#endif
/*entity_flare_collider.hpp*/