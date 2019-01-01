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
#include "terrain_block1.hpp"

#include "terrain_height_map1.hpp"
#include "../terrain_data.hpp"
#include "terrain_texture_layer1.hpp"
#include "terrain_hole_map1.hpp"
#include "../dominant_texture_map.hpp"

#include "cstdmf/debug.hpp"
#include "resmgr/bwresource.hpp"

#ifndef MF_SERVER
#	include "d3dx9mesh.h"
#	include "terrain_renderer1.hpp"
#	include "horizon_shadow_map1.hpp"
#	include "moo/vertex_buffer.hpp"
#	include "moo/vertex_formats.hpp"
#endif

using namespace Terrain;

DECLARE_DEBUG_COMPONENT2("Terrain", 0)

#ifndef MF_SERVER

/**
 *  This is the TerrainBlock1 constructor.
 */
TerrainBlock1::TerrainBlock1():
    CommonTerrainBlock1(),
	pHorizonMap_( NULL ),	
	diffLights_( new Moo::LightContainer ),
	specLights_( new Moo::LightContainer )
{
}


/**
 *  This is the TerrainBlock1 destructor.
 */
/*virtual*/ TerrainBlock1::~TerrainBlock1()
{
}


/**
 *	This virtual function is called after loading is done.  The TerrainBlock1
 *	uses this as an opportunity to save the texture layers, etc.
 *
 *  @param filename         A file to load the terrain block from.
 *  @param error            If not null and there was an error loading
 *                          the terrain then this will be set to a
 *                          description of the problem.
 *  @returns                True if the load was completely successful, 
 *                          false if a problem occurred.
 */
bool TerrainBlock1::postLoad(std::string const &filename, 
            Matrix          const &worldTransform,
            Vector3         const &cameraPosition,
            DataSectionPtr  pTerrain,
            TextureLayers   &textureLayers,
            std::string     *error /* = NULL*/)
{
	BW_GUARD;
	textureLayers_.swap( textureLayers );

    pHorizonMap_ = new HorizonShadowMap1(*this);
	if ( !pHorizonMap_->load( pTerrain, error ) )
	{
		std::stringstream msg;
			msg << "TerrainBlock1::load: "; 
			msg << "Couldn't load shadow data for " << filename;
		if ( error )
			*error = msg.str();
		else
			ERROR_MSG( "%s\n", msg.str().c_str() );
		return false;
	}

	rebuildCombinedLayers();
	return true;
}


/**
 *	Returns the four blends of an old terrain block in a single 32bit value.
 *
 *  @param x      x coord in texture layer map coordinates
 *  @param y      y coord in texture layer map coordinates
 *  @returns      four packed blends in a single 32bit value, as required by
 *                the legacy terrain shader.
 */
/*static*/ uint32 TerrainBlock1::packedBlends( uint32 x, uint32 y )
{
	BW_GUARD;
	MF_ASSERT( textureLayers_.size() == 4 );
	return
		  (textureLayers_[ 0 ]->image().get( x, y ) / 2 + 128) |
		( (textureLayers_[ 1 ]->image().get( x, y ) / 2 + 128) << 8 ) |
		( (textureLayers_[ 2 ]->image().get( x, y ) / 2 + 128) << 16 ) |
		(  textureLayers_[ 3 ]->image().get( x, y ) << 24 );
}


/**
 * Returns true if the managed objects have been created.
 *
 * @returns           true if the managed objects have been created.
 */
bool TerrainBlock1::managedObjectsCreated() const
{
	BW_GUARD;
	return vertexBuffer_.valid() && indexBuffer_.valid();
}


/**
 * Implementation of DeviceCallback::createManagedObjects.
 * This method creates the vertex and index buffers.
 */
void TerrainBlock1::createManagedObjects()
{
	BW_GUARD;
	if (Moo::rc().device())
	{
		// Create the vertex buffer.
		Moo::VertexBuffer pVBuffer;
		nVertices_ = verticesWidth() * verticesHeight();

		HRESULT hr = pVBuffer.create( nVertices_ * sizeof( Moo::VertexXYZNDS ),
						D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED );
		if (SUCCEEDED( hr ))
		{
			// Fill the vertex buffer with the heights from the heightmap,
			// the blendvalues from the blend map, and the shadow values from
			// the shadowmap, and generate x and z positions.
			Moo::VertexLock<Moo::VertexXYZNDS> 
				vl( pVBuffer, 0, nVertices_ * sizeof( Moo::VertexXYZNDS ), 0 );

			if (vl)
			{
				float zPos = 0;
				int index = 0;
				for (uint32 z = 0; z < verticesHeight(); z++)
				{
					float xPos = 0;
					for (uint32 x = 0; x < verticesWidth(); x++ )
					{
						vl[index].pos_.set( xPos, heightMap().heightAt( (int)x, (int)z ), zPos );
						vl[index].normal_ = normalAt(
							(float)x * heightMap().spacingX(), (float)z * heightMap().spacingZ() );
						vl[index].colour_ = packedBlends( x, z );
						HorizonShadowPixel shadowPixel = shadowMap().shadowAt( (int)x, (int)z );
						vl[index].specular_ =
							DWORD( ((shadowPixel.west & 0xFF00) >> 8) | (shadowPixel.east & 0xFF00) ) << 8;
						xPos += spacing();
						++index;
					}
					zPos += spacing();
				}
				vertexBuffer_ = pVBuffer;
			}
			else
			{
				nVertices_ = 0;
				CRITICAL_MSG( "Moo::TerrainBlock::createManagedObjects: Unable to lock vertex buffer\n" );
			}
		}
		else
		{
			WARNING_MSG( "Moo::TerrainBlock::createManagedObjects: Unable to create vertex buffer, error code %lx\n", hr );
		}

		MF_ASSERT( nVertices_ < 65536 );// otherwise, we MUST revise the index buffer usage here
		// Create the indices, the indices are made up of one big strip with degenerate
		// triangles where there is a break in the strip, breaks occur on the edges of the
		// strip and where a hole is encountered.

		std::vector<uint16> indices;
		bool instrip = false;
		uint16 lastIndex = 0;

		for (uint32 z = 0; z < blocksHeight(); z++)
		{
			for (uint32 x = 0; x < blocksWidth(); x++)
			{
				if (instrip)
				{
					if (holeMap().image().isEmpty() || !holeMap().image().get( x, z ))
					{
						indices.push_back( uint16( x + 1 + ((z + 1) * verticesWidth()) ) );
						indices.push_back( uint16( x + 1 + (z * verticesWidth()) ) );
					}
					else
					{
						instrip = false;
					}
				}
				else
				{
					if (holeMap().image().isEmpty() || !holeMap().image().get( x, z ))
					{
						if (indices.size())
							indices.push_back( indices.back() );
						indices.push_back( uint16( x + ((z + 1) * verticesWidth()) ) );
						indices.push_back( indices.back() );
						indices.push_back( uint16( x + (z * verticesWidth()) ) );
						indices.push_back( uint16( x + 1 + ((z + 1) * verticesWidth()) ) );
						indices.push_back( uint16( x + 1 + (z * verticesWidth()) ) );
						instrip = true;
					}
				}
			}
			instrip = false;
		}

		// Create the index buffer and fill it with the generated indices.
		if (indices.size())
		{
			nIndices_ = indices.size();
			nPrimitives_ = nIndices_ - 2;

			Moo::IndexBuffer pIBuffer;
			hr = pIBuffer.create( nIndices_, D3DFMT_INDEX16,
				D3DUSAGE_WRITEONLY, D3DPOOL_MANAGED );
			if (SUCCEEDED( hr ))
			{
				Moo::IndicesReference ir = pIBuffer.lock();

				if (ir.valid())
				{
					ir.fill( &indices.front(), nIndices_ );
					pIBuffer.unlock();
					indexBuffer_ = pIBuffer;
				}
				else
				{
					CRITICAL_MSG( "Moo::TerrainBlock::createManagedObjects: unable to lock index buffer, error code %lx\n", hr );
				}
				pIBuffer.release();
			}
			else
			{
				WARNING_MSG( "Moo::TerrainBlock::createManagedObjects: unable to create index buffer, error code %lx\n", hr );
			}
		}
	}
}

/**
 * Implementation of DeviceCallback::deleteManagedObjects.
 * Removes the vertex and index buffers
 */
void TerrainBlock1::deleteManagedObjects()
{
	BW_GUARD;
	memoryCounterSub( terrainAc );
	if (indexBuffer_.valid())
	{
		memoryClaim( indexBuffer_ );
		indexBuffer_.release();
	}
	if (vertexBuffer_.valid())
	{
		memoryClaim( vertexBuffer_ );
		vertexBuffer_.release();
	}
}



/**
 *  This function gets the shadow map for the terrain.
 *
 *  @returns                The shadow map for the terrain.
 */
/*virtual*/ HorizonShadowMap &TerrainBlock1::shadowMap()
{
    return *pHorizonMap_;
}


/**
 *  This function gets the shadow map for the terrain.
 *
 *  @returns                The shadow map for the terrain.
 */
/*virtual*/ HorizonShadowMap const &TerrainBlock1::shadowMap() const
{
    return *pHorizonMap_;
}


/**
 *	Returns the four blends of an old terrain block in a single 32bit value.
 *
 *  @param blockPosition      ignored
 *  @param cameraPosition     ignored
 *  @returns                  true if successful
 */
/*virtual*/ bool TerrainBlock1::set(
	const Vector3 & /*blockPosition*/, const Vector3 & /*cameraPosition*/ )
{
	BW_GUARD;
	Moo::rc().lightContainer( diffLights_ );
	Moo::rc().specularLightContainer( specLights_ );
	return true;
}


/**
 *	Renders the terrain using the material defined by 'effect' if it's not NULL
 *  or, if it's NULL, just renders the terrain geometry without any effect.
 *
 *  @param effect             effect to be used to render the terrain block, or
 *                            NULL to just render the plain terrain geometry.
 *  @returns                  true if successful
 */
/*virtual*/ bool TerrainBlock1::draw( Moo::EffectMaterialPtr effect )
{
	BW_GUARD;
	if ( effect )
	{
		if ( effect->begin())
		{
			for (uint32 i=0; i<effect->nPasses(); i++)
			{
				if (effect->beginPass(i))
				{
					draw(); // draw with NULL texture setter
					effect->endPass();
				}
			}
			effect->end();		
		}
	}
	else
	{
		draw(); // draw with NULL texture setter
	}
	return true;
}


/**
 *	Renders the terrain using the material defined by 'effect' if it's not null,
 *  or just render the terrain geometry without any effect.
 *
 *  @param tts      texture setter to be used to render the terrain block, or
 *                  NULL to just render the plain terrain geometry.
 */
/*virtual*/ void TerrainBlock1::draw( TerrainTextureSetter* tts /*= NULL */)
{
	BW_GUARD;
	// Allocate d3d resources if they have not been allocated already.
	if (!managedObjectsCreated())
	{
		createManagedObjects();

		if (!managedObjectsCreated())
			return;
	}	
	if (Moo::rc().device() && !holeMap().allHoles())
	{
		if (tts)
		{
			static std::vector< Moo::BaseTexturePtr > textures;
			const TextureLayers& layers = textureLayers();
			for ( TextureLayers::const_iterator i = layers.begin();
				i != layers.end(); ++i )
			{
				textures.push_back( (*i)->texture() );
			}
			tts->setTextures(textures);
			textures.clear();
		}

		// Set up buffers and draw.
		indexBuffer_.set();
		vertexBuffer_.set( 0, 0, sizeof( Moo::VertexXYZNDS ) );
		Moo::rc().drawIndexedPrimitive( D3DPT_TRIANGLESTRIP,
				0, 0, nVertices_, 0, nPrimitives_ );
	}
}

/**
 *	Stores the current lighting for later use.
 *
 *  @param addSpecular        true to cache the specular lighting along with
 *                            the diffuse, false to store only the diffuse.
 */
void TerrainBlock1::cacheCurrentLighting( bool addSpecular /* = true */ )
{
	BW_GUARD;
	BoundingBox bb = boundingBox();	
	bb.transformBy( Moo::rc().world() );
	diffLights_->init( Moo::rc().lightContainer(), bb );
	if ( addSpecular )
	{
		specLights_->init( Moo::rc().specularLightContainer(), bb );
	}
	else
	{
		specLights_->init( NULL, bb );
	}
}


/**
 *  This function creates the UMBRA mesh and returns it in the provided
 *  structure.
 *
 *  @param umbraMesh				umbra mesh info.
 */
void TerrainBlock1::createUMBRAMesh( UMBRAMesh& umbraMesh ) const
{
	BW_GUARD;
	umbraMesh.testIndices_.reserve( blocksWidth()* blocksHeight() * 2 * 3 );

	// Create the terrain indices making sure we do not
	// make occlusion geometry for any holes in the terrain
	for (uint32 z = 0; z < blocksHeight(); z++)
	{
		uint32 zRow = z * verticesHeight();
		float zPos = (z + 0.5f) * this->spacing();
		for (uint32 x = 0; x < blocksWidth(); x++)
		{
			float xPos = (x + 0.5f) * this->spacing();
			if ( !holeMap().holeAt( xPos, zPos ) )
			{
				umbraMesh.testIndices_.push_back( zRow + x );
				umbraMesh.testIndices_.push_back( zRow + x + verticesWidth() );
				umbraMesh.testIndices_.push_back( zRow + x + verticesWidth() + 1 );
				umbraMesh.testIndices_.push_back( zRow + x  );
				umbraMesh.testIndices_.push_back( zRow + x + verticesWidth() + 1 );
				umbraMesh.testIndices_.push_back( zRow + x + 1 );
			}
		}
	}

	umbraMesh.testVertices_.reserve( verticesWidth() * verticesHeight() );

	// Create the terrain vertices using the height map
	for (uint32 z = 0; z < verticesHeight(); z++)
	{
		float zPos = float(z) * 4.f;
		for (uint32 x = 0; x < verticesWidth(); x++)
		{
			float xPos = float(x) * 4.f;
			umbraMesh.testVertices_.push_back(
				Vector3( xPos, heightMap().heightAt( int(x), int(z) ), zPos ) );
		}
	}

	umbraMesh.writeIndices_ = umbraMesh.testIndices_;
	umbraMesh.writeVertices_ = umbraMesh.testVertices_;
}


/**
 *  This method accesses the texture layers.
 */
TerrainBlock1::TextureLayers &TerrainBlock1::textureLayers()
{
    return textureLayers_;
}


/**
 *  This method accesses the texture layers.
 */
TerrainBlock1::TextureLayers const &TerrainBlock1::textureLayers() const
{
    return textureLayers_;
}


/**
 *  Recreates the managed objects so the texture layers are updated, and if
 *  generateDominantTextureMap is true, generates the dominant texture map.
 *
 *  @param compressTextures                 ignored
 *  @param generateDominantTextureMap        true to generate the dominant texture map.
 */
void TerrainBlock1::rebuildCombinedLayers(
	bool /*compressTextures = true*/, bool generateDominantTextureMap /*=true*/ )
{
	BW_GUARD;
	if (managedObjectsCreated())
	{
		deleteManagedObjects();
	}
	createManagedObjects();

	if( generateDominantTextureMap )
	{
		// Update the dominant texture map:
		dominantTextureMap( new DominantTextureMap( textureLayers(), 1.0f ) );
	}
}

#endif

// terrain_block1.cpp
