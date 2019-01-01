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
#include "terrain_lod_map2.hpp"

#include "../terrain_data.hpp"
#include "terrain_height_map2.hpp"
#include "moo/png.hpp"

using namespace Terrain;
DECLARE_DEBUG_COMPONENT2( "Terrain", 0 );

TerrainLodMap2::TerrainLodMap2()
{
}

TerrainLodMap2::~TerrainLodMap2()
{
}

/**
 *  This method loads the lod map from a data section
 *	@param pLodMapSection the datasection of the lod map
 *	@return true if the lod map loads successfully
 */
bool TerrainLodMap2::load( DataSectionPtr pLodMapSection )
{
	BW_GUARD;
	bool ret = false;

	BinaryPtr pData = pLodMapSection->asBinary();
	if (pData)
	{
		pLodMap_ = Moo::rc().createTextureFromFileInMemoryEx( 
			pData->data(), pData->len(),
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, 
			D3DPOOL_MANAGED, D3DX_DEFAULT, 
			D3DX_DEFAULT, 0, NULL, NULL, "Terrain/LodMap2/Texture" ); 

		if (pLodMap_)
		{
			D3DSURFACE_DESC levelDesc;
			pLodMap_->GetLevelDesc( 0, &levelDesc );
			
			if ( levelDesc.Width != levelDesc.Height )
			{
				WARNING_MSG("TerrainLodMap2::load - lod texture must be square.\n" );
				pLodMap_ = NULL;
				ret = false;
			}
			else
			{
				size_ = levelDesc.Width;
				ret = true;
			}
			// Add the texture to the preload list so that it can get uploaded
			// to video memory
			Moo::rc().addPreloadResource( pLodMap_.pComObject() );
		}
		else
		{
			WARNING_MSG( "TerrainLodMap2::load - unable to open lod map\n" );
		}
	}
	else
	{
		WARNING_MSG( "TerrainLodMap2::load - unable to get lod map from data section\n" );
	}

	return ret;
}

bool TerrainLodMap2::save( DataSectionPtr parentSection, const std::string& name ) const
{
	BW_GUARD;
	bool ok = false;

	// copy texture to a file buffer in memory
	ID3DXBuffer* pBuffer = NULL;
	HRESULT hr = D3DXSaveTextureToFileInMemory( &pBuffer, D3DXIFF_DDS, 
										   pLodMap_.pComObject(), NULL  );

	if (SUCCEEDED(hr))
	{
		// copy buffer to binary block
		BinaryPtr pBlock = new BinaryBlock( pBuffer->GetBufferPointer(), 
			pBuffer->GetBufferSize(), "BinaryBlock/TerrainPhotographer" );
		pBuffer->Release();

		// write binary block to data section
		ok = parentSection->writeBinary( name, pBlock );
	}

	return ok;
}

uint32 TerrainLodMap2::size() const
{
	BW_GUARD;
	return pLodMap_ ? size_ : 0;
}
