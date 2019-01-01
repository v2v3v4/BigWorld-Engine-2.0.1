/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_TERRAIN_FINDER_HPP
#define TERRAIN_TERRAIN_FINDER_HPP

#include "math/boundbox.hpp"
#include "math/matrix.hpp"

namespace Terrain
{
    class BaseTerrainBlock;
}

namespace Terrain
{
    /**
     *  This class is used to find terrain at the given position.
     */
    class TerrainFinder
    {
    public:
        /**
         *  This is the return result of findOutsideBlock.
         */
        struct Details
        {
            BaseTerrainBlock        *pBlock_;
            Matrix                  const *pMatrix_;
            Matrix                  const *pInvMatrix_;

            Details();
        };

	    virtual ~TerrainFinder();

	    virtual Details findOutsideBlock(Vector3 const &pos) = 0;
    };
}


#endif // TERRAIN_TERRAIN_FINDER_HPP
