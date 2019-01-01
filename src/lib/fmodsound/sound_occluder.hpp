/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SOUND_OCCLUDER_HPP
#define SOUND_OCCLUDER_HPP


#include "fmod_config.hpp"
#if FMOD_SUPPORT
# include <fmod.hpp>
#endif
#if FMOD_SUPPORT

//#include <vector>
//#include "math/vector3.hpp"
class Model;
class SuperModel;
class Vector3;

namespace Terrain
{
    class TerrainHeightMap;
}

/*
    This class wraps FMOD::Geometry objects.
*/
class SoundOccluder
{
public:
    SoundOccluder();
    SoundOccluder( SuperModel * pSuperModel );
    ~SoundOccluder();


    const bool constructed() const { return (geometries_.size() > 0); }

    bool construct( SuperModel * pSuperModel );
    bool construct( Model *pModel );
    bool construct( const Terrain::TerrainHeightMap& map, float directOcclusion, float reverbOcclusion );
    bool setActive( bool active );
    bool update( const Vector3& position, const Vector3& forward = Vector3::zero(), const Vector3& up = Vector3::zero() );
	bool update( const Matrix& transform );

	void debugDraw();

protected:
    std::vector<FMOD::Geometry *> geometries_;

private:    
};

#endif //FMOD_SUPPORT

#endif //SOUND_OCCLUDER_HPP