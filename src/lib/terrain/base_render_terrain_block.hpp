/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_BASE_RENDER_TERRAIN_BLOCK_HPP
#define TERRAIN_BASE_RENDER_TERRAIN_BLOCK_HPP

#ifndef MF_SERVER

#include "moo/forward_declarations.hpp"

class Matrix;

namespace Terrain
{
	class EffectMaterial;
	class HorizonShadowMap;
}

namespace Moo
{
	class EffectMaterial;
}

namespace Terrain
{
	/**
	 *	This class is the base class for terrain blocks used in the client and
	 *	tools.  The server uses BaseTerrainBlock as the base class.  This class
	 *	implements rendering interfaces and terrain hole map interfaces.
	 */
	class BaseRenderTerrainBlock : public SafeReferenceCount
	{
	public:
        /**
         *  Render this terrain block immediately.
         *
         *  @param pMaterial The material to render with.
         */
        virtual bool draw( Moo::EffectMaterialPtr pMaterial ) = 0;

        /**
         *  This function returns the horizon shadow map for this block of 
         *  terrain.
		 *
		 *  @returns				The HorizonShadowMap or the terrain.
         */
        virtual HorizonShadowMap &shadowMap() = 0;

        /**
         *  This function returns the horizon shadow map for this block of 
         *  terrain as a const reference.
		 *
 		 *  @returns				The HorizonShadowMap or the terrain.
         */
        virtual HorizonShadowMap const &shadowMap() const = 0;


		struct UMBRAMesh
		{
			std::vector<Vector3>	testVertices_;
			std::vector<Vector3>	writeVertices_;
			std::vector<uint32>		testIndices_;
			std::vector<uint32>		writeIndices_;
		};

        /**
         *  This function creates the UMBRA mesh and returns it in the provided
		 *  structure.
		 *
 		 *  @param umbraMesh			DVPS mesh info.
         */
		virtual void createUMBRAMesh( UMBRAMesh& umbraMesh ) const = 0;

		/**
		 *	This function stores the current ambient, directional, and 
		 *	(optionally) specular lighting state so this block can defer 
		 *	rendering correctly.
		 *
		 *	@param cacheSpecular	Cache specular lighting, defaults to true.
		 */
		virtual void cacheCurrentLighting( bool cacheSpecular = true ) = 0;
	};
}

#endif // MF_SERVER

#endif // TERRAIN_BASE_RENDER_TERRAIN_BLOCK_HPP
