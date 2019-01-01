/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TERRAIN_PHOTOGRAPHER_HPP
#define TERRAIN_PHOTOGRAPHER_HPP

namespace Moo
{
	class RenderTarget;
	typedef SmartPointer<RenderTarget> RenderTargetPtr;
}

namespace Terrain
{
	class BaseTerrainBlock;
	/**
	 */
	class TerrainPhotographer
	{
	public:
		TerrainPhotographer();
		~TerrainPhotographer();

		bool init( uint32 basePhotoSize );
		bool photographBlock(	BaseTerrainBlock*	pBlock,
								const Matrix&		transform );
		
		bool output(ComObjectWrap<DX::Texture>&	pDestTexture,
					D3DFORMAT					destImageFormat );

	private:

		Moo::RenderTargetPtr	pBasePhoto_;
	};

};

#endif // TERRAIN_BLOCK2_HPP
