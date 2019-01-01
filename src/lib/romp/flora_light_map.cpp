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
#include "flora_light_map.hpp"
#include "flora.hpp"
#include "chunk/chunk.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_manager.hpp"
#include "terrain/base_terrain_renderer.hpp"
#include "terrain/terrain_height_map.hpp"
#include "resmgr/auto_config.hpp"
#include "time_of_day.hpp"
#include "terrain/manager.hpp"

static float s_lightU = 0.f;
static float s_lightV = 0.f;


/**
 *	This class exposes a 2x4 matrix to the effect file engine, which
 *	transforms from world position to texture coordinate space.
 */
class FloraLightMapTransformSetter : public Moo::EffectConstantValue, public Aligned
{
	bool operator()(ID3DXEffect* pEffect, D3DXHANDLE constantHandle)
	{		
		pEffect->SetVectorArray(constantHandle, (const Vector4*)&worldToLight_, 2);
		return true;
	}

public:
	void worldToLight( const Vector4* tr )
	{		
		worldToLight_[0] = tr[0];
		worldToLight_[1] = tr[1];
	}	

private:
	Vector4 worldToLight_[2];
};


/**
 *	Constructor.
 */
static IncrementalNameGenerator s_floraLightMapNames( "FloraLightMap" );

FloraLightMap::FloraLightMap( Flora* flora ):	
	EffectLightMap( s_floraLightMapNames.nextName() ),
	inited_( false ),	
	flora_( flora )
{
	//Create the watchers once only
	static bool s_createdWatchers = false;
	if (!s_createdWatchers)
	{
		MF_WATCH( "Client Settings/Flora/Light Offset U",
			s_lightU,
			Watcher::WT_READ_WRITE,
			"U offset for calculating the world -> lightmap transform. "
			"Changing this will offset where lighting information is drawn.");
		MF_WATCH( "Client Settings/Flora/Light Offset V",
			s_lightV,
			Watcher::WT_READ_WRITE,
			"V offset for calculating the world -> lightmap transform. "
			"Changing this will offset where lighting information is drawn.");
		s_createdWatchers = true;
	}

	for ( int i=0; i<4; i++ )
		blocks_[i] = NULL;

	this->createUnmanagedObjects();

	// moved initialisation to constructor 
	// to avoid loading files while the game 
	// simulation is already running.
	//DataSectionPtr pSection = BWResource::openSection( s_data );
	if (flora_->data())
	{

		// Initialise the correct version of flora effect according to terrain
		// renderer version.
		std::string sectionName = "light_map";

		if ( flora_->terrainVersion() == 200 )
		{
			sectionName = "light_map2";
		}
		else if ( flora_->terrainVersion() != 100 )
		{
			WARNING_MSG( "Unknown terrain version for flora, defaulting to "
				"classic flora.\n");
		}

		DataSectionPtr floraSect = flora_->data()->openSection( sectionName );

		if (floraSect)
		{
			inited_ = EffectLightMap::init( floraSect );
		}
		else if (flora_->vbSize())
		{
			ERROR_MSG( "FloraLightMap::FloraLightMap - flora data section was NULL\n" );
		}
	}
}


/**
 *	Destructor.
 */
FloraLightMap::~FloraLightMap()
{
}


void FloraLightMap::update( float gameTime )
{
	if ( !inited_ )
	{
		return;
	}

	if (!material_.get() || !material_->pEffect() || !material_->pEffect()->pEffect())
		return;

	//Find the 4 relevant terrain blocks and chunks.
	Vector3 center = Moo::rc().invView().applyToOrigin();
	Vector3 relative;	
	Vector3 pos( center );
	for ( int i=0; i<4; i++ )
	{
		//note - is important that i=3 gets (-50,-50)
		pos.x = center.x + ((i&1) ? -50.f : 50.f);
		pos.z = center.z + ((i&2) ? -50.f : 50.f);

		Terrain::BaseTerrainBlockPtr pBlock = flora_->getTerrainBlock(pos, relative);
		if ( blocks_[i] != pBlock )
		{
			blocks_[i] = pBlock;
			ChunkSpacePtr pSpace = ChunkManager::instance().cameraSpace();
			ChunkSpace::Column* pColumn = pSpace.exists() ? pSpace->column( pos, false ) : NULL;
			if ( pColumn != NULL )
			{
				Chunk * pChunk = pColumn->pOutsideChunk();
				if (pChunk != NULL)
				{
					chunks_[i] = pChunk;
				}
			}
		}
	}		

	//calculate the world -> light map transform
	//note - from the checking all 4 terrain blocks, relative is left with the relative position
	//of the test point from the origin of the bottom,left block.  This is ensured by the
	//comment above noting that i=3 gets (-50,-50)
	//this translation moves the bottom,left point of the flora to the
	//bottom-left point of the bottom-left terrain block.

	float bottom = BaseChunkSpace::gridToPoint( BaseChunkSpace::pointToGrid(pos.z) );
	float left = BaseChunkSpace::gridToPoint( BaseChunkSpace::pointToGrid(pos.x) );

	float scale = 1.f/200.f;
	Vector4 worldToLight[2];
	worldToLight[0].set( scale, 0.f, 0.f, scale*(-left + s_lightU) );
	worldToLight[1].set( 0.f, 0.f, scale, scale*(-bottom + s_lightV) );
	Moo::EffectConstantValue* ecv = &*transformSetter_;
	FloraLightMapTransformSetter* slmts = static_cast<FloraLightMapTransformSetter*>(ecv);	
	slmts->worldToLight( worldToLight );

	if ( pRT_ && pRT_->push() )
	{
		Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, 0x80808080, 1, 0 );

		DX::Viewport oldViewport;
		Moo::rc().getViewport( &oldViewport );
		DX::Viewport newViewport = oldViewport;
		newViewport.Width /= 2;
		newViewport.Height /= 2;		

		Moo::rc().push();
		Terrain::BaseTerrainRenderer::instance()->clearBlocks();

		for ( int j=0; j<4; j++ )
		{
			newViewport.X = (j&1) ? 0 : width_/2;
			newViewport.Y = (j&2) ? 0 : height_/2;
			Moo::rc().setViewport( &newViewport );
			Terrain::BaseTerrainBlockPtr pBlock = blocks_[j];
			if ( pBlock )
			{						
				this->setProjection(pBlock, chunks_[j]->transform());
				Terrain::BaseTerrainRenderer::instance()->drawSingle( 
											pBlock.getObject(), 
											chunks_[j]->transform(), 
											material_.get(),
											true );
			}
		}

		Moo::rc().pop();

		Moo::rc().setViewport( &oldViewport );
		pRT_->pop();
	}	
}


/**
 *	This method calculates and sets the orthogonal projection matrix on the
 *	effect.
 */
void FloraLightMap::setProjection(Terrain::BaseTerrainBlockPtr pValidBlock, 
								  const Matrix& chunkTransform)
{	
	Matrix m;	

    Terrain::TerrainHeightMap &thm = pValidBlock->heightMap();
	
	float xExtent = thm.spacingX() * thm.blocksWidth();
	float zExtent = thm.spacingZ() * thm.blocksHeight();

	LightMap::orthogonalProjection(xExtent,zExtent,m);
	EffectLightMap::setLightMapProjection(m);


	// Set up the orientation of the chunk in the light map
	ComObjectWrap<ID3DXEffect> pEffect = material_->pEffect()->pEffect();
	D3DXHANDLE h = pEffect->GetParameterByName(NULL,"orientationX");
	if (h)
	{
		Vector4 orientationX( chunkTransform._11, chunkTransform._13, 0, 0 );
		pEffect->SetVector( h, &orientationX );
	}
	h = pEffect->GetParameterByName(NULL,"orientationY");
	if (h)
	{
		Vector4 orientationY( chunkTransform._31, chunkTransform._33, 0, 0 );
		pEffect->SetVector( h, &orientationY );
	}
}


/**
 *	This method implements the LightMap base class interface and
 *	creates a FloraLightMapTransformSetter.
 */
void FloraLightMap::createTransformSetter()
{
	transformSetter_ = new FloraLightMapTransformSetter;
}
