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

#include "photon_occluder.hpp"

#ifndef CODE_INLINE
#include "photon_occluder.ipp"
#endif

std::ostream& operator<<(std::ostream& o, const PhotonOccluder& t)
{
	o << "PhotonOccluder\n";
	return o;
}


/*photon_occluder.cpp*/
