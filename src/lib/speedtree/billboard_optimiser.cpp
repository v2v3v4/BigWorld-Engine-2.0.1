/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/***
 *

The basic principle of the billboard optimiser is drawing a group of
billboard with a single draw call. 

The biggest challenge is aggregating all billboard textures into a 
single map in a dynamic way (tree types may be added and removed as 
the player walks around the map), but the TextureAggregator class 
takes care of doing that.

When a tree type is registered with the optimiser, its billboard vertex 
data are stored in a cache. When a tree instance of that type is added,
its vertices are transformed to world space and added to a large vertex
buffer together with other trees.

The large vertex buffers are created on demand, as trees become visible.
As the camera walks around the maps, trees may get ejected, and buffers 
will be reused or freed as allowed.

>> Structs
	>>	AlphaBillboardVertex
	>>	BillboardVertex
	>>	BillboardsVBuffer

>> Functions:
	>>	addTreeType()
	>>	addTree()
	>>	updateTreeAlpha()
	>>	recycleBuffers()
	>>	drawVisible()
	>>	touch()
	>>	compile()
 
 *
 ***/


// Module Interface
#include "billboard_optimiser.hpp"

// BW Tech Hearders
#include "cstdmf/debug.hpp"
#include "cstdmf/watcher.hpp"

DECLARE_DEBUG_COMPONENT2("SpeedTree", 0)


#if SPEEDTREE_SUPPORT // -------------------------------------------------------

// BW Tech Hearders
#include "cstdmf/avector.hpp"
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/concurrency.hpp"
#include "resmgr/auto_config.hpp"
#include "moo/material.hpp"
#include "moo/render_context.hpp"
#include "moo/sys_mem_texture.hpp"
#include "moo/effect_lighting_setter.hpp"
#include "moo/effect_material.hpp"

// DX Wrappers
#include "d3dx9core.h"

// STD Headers
#include <vector>
#include <list>

// Import dx wrappers
using Moo::VertexBufferWrapper;


namespace speedtree {
namespace { // anonymous

/**
 *	The vertex type used to render optimised billboards.
 */
struct AlphaBillboardVertex
{
	Vector3 position_;
	Vector3 lightNormal_;
	Vector3 alphaNormal_;
	
	//TODO: use Vector2
	FLOAT   texCoords_[2];
	FLOAT   alphaIndex_;
	FLOAT   alphaMask_[4];
#if SPT_ENABLE_NORMAL_MAPS
	Vector3 binormal_;
	Vector3 tangent_;
#endif // SPT_ENABLE_NORMAL_MAPS
	Vector4 diffuseNAdjust_;
	Vector3 ambient_;

	// TODO: try not using 8 tex 
	// coords for lower specs cards
	static DWORD fvf()
	{
		return
			D3DFVF_XYZ |
			D3DFVF_NORMAL |
			D3DFVF_TEXCOORDSIZE3(0) |
			D3DFVF_TEXCOORDSIZE2(1) |
			D3DFVF_TEXCOORDSIZE1(2) |
			D3DFVF_TEXCOORDSIZE4(3) |
			D3DFVF_TEXCOORDSIZE3(4) |
			D3DFVF_TEXCOORDSIZE3(5) |
			D3DFVF_TEXCOORDSIZE4(6) |
			D3DFVF_TEXCOORDSIZE3(7) |
			D3DFVF_TEX8;
	}
};

// Helper functions
void saveTextureAtlas( bool save, DX::BaseTexture * texture );

} // namespace anonymous

// Billboard alpha fading constants
float BillboardVertex::s_minAlpha_ = 84.0f;
float BillboardVertex::s_maxAlpha_ = 255.0;
static float s_mungeNormalDepth_ = -10.f;

/**
 *	Holds the vertex buffer to draw the billboards of a chunk/optimiser. 
 *	The number of BillboardsVBuffer are kept to a minimum to save memory.
 *	Buffers that have been inactive for a couple of frames are reclycled 
 *	to work for chunks that have just became active. Chunks not reclycled 
 *	for several frames are deleted.
 */
class BillboardsVBuffer : 
	public ReferenceCount,
	private Aligned
{
public:
	typedef BillboardOptimiser::TreeTypeMap TreeTypeMap;
	typedef BillboardOptimiser::TreeInstanceVector TreeInstanceVector;
	typedef BillboardOptimiser::AlphaValueVector AlphaValueVector;
	typedef BillboardOptimiser::FrameData FrameData;
	
	static void init( DataSectionPtr section, const std::string & effect );
	static void initFXFiles();
	static void fini();
	
	static BillboardsVBuffer * acquire();

	static void tick();
	static void update();
	static void resetAll();
	static void recycleBuffers();
	static void drawVisible(
		const FrameData & frameData, 
		DX::Texture     * texture,
#if SPT_ENABLE_NORMAL_MAPS
		DX::Texture     * normalMap,
#endif // SPT_ENABLE_NORMAL_MAPS
		const Matrix    & texTransform );

#ifdef EDITOR_ENABLED
	void touch( const Vector4 & fogColour, float fogNear, float fogFar );
#else
	void touch();
#endif // EDITOR_ENABLED

	void draw();
	
	bool isCompiled() const;
	void compile();
	
	void bind(
		BillboardOptimiser * owner, 
		TreeInstanceVector * billboards, 
		AlphaValueVector   * alphas );

	void reset();

	static Moo::TextureAggregator* s_diffuseMapAggregator_;

#if SPT_ENABLE_NORMAL_MAPS
	static Moo::TextureAggregator* s_normalMapAggregator_;
#endif // SPT_ENABLE_NORMAL_MAPS

	static int s_shaderCap_;
private:
	BillboardsVBuffer();
	~BillboardsVBuffer();

	// forbid copy
	BillboardsVBuffer( const BillboardsVBuffer & );
	const BillboardsVBuffer & operator = ( const BillboardsVBuffer & );
	
	int     frameStamp_;
	int     activeStamp_;
#ifdef EDITOR_ENABLED
	Vector4 fogColour_;
	float   fogNear_;
	float   fogFar_;
#endif // EDITOR_ENABLED

	Moo::LightContainerPtr   lights_;
	Moo::LightContainerPtr   specLights_;
	
	BillboardOptimiser * owner_;

	TreeInstanceVector * trees_;
	AlphaValueVector   * alphas_;
	
	typedef VertexBufferWrapper< AlphaBillboardVertex > AlphaBillboardVBuffer;
	AlphaBillboardVBuffer vbuffer_;

	typedef SmartPointer<BillboardsVBuffer> BillboardsVBufferPtr;
	typedef std::list<BillboardsVBufferPtr> BillboardsVBufferList;
	typedef std::auto_ptr<Moo::EffectLightingSetter> EffectLightingSetterPtr;
	static BillboardsVBufferList   s_buffersPool_;
	static EffectLightingSetterPtr s_lightingSetter_;
	
	typedef std::vector<const BBVertexBuffer *> BuffersVector;
	static BuffersVector s_lockedBuffers_;

	BillboardsVBufferList::iterator poolIt_;

	static int s_curFrameStamp_;
	static int s_framesToRecycle_;
	static int s_framesToDelete_;

	static int s_activeCounter_;
	static int s_visibleCounter_;
	static int s_poolSize_;
	
	static Moo::EffectMaterialPtr s_effect_;
	static std::string       s_fxFile_;
};

// -------------------------------------------------------------- BillboardOptimiser

#define GROUP_SIZE 600.f

/**
 *	This function determines the grouping id for the tree
 * 	optimisation. Possible improvements include allowing
 *	the group size to be configurable per space.
 */
uint32 BillboardOptimiser::makeID( const Vector3& pos )
{
	BW_GUARD;
	uint16 groupX = (short)floorf(pos.x / GROUP_SIZE);
	uint16 groupZ = (short)floorf(pos.z / GROUP_SIZE);

	uint32 id=0;

	uint32 groupX_32 = ( uint32(groupX) ) & 0xffff;
	uint32 groupZ_32 = ( uint32(groupZ)<<16 ) & 0xffff0000;
	id = groupX_32 | groupZ_32;
	return id;
}

/**
 *	Check to see if this optimiser has reached it's limit for
 *	tree instances.
 */
bool BillboardOptimiser::isFull() const
{
	BW_GUARD;
	return ( (uint16)this->trees_.size() >= (BillboardOptimiser::s_maxInstanceCount_-1) &&
			 this->freeSlots_.size() == 0 );
}

/**
 *	Retrieves the BillboardOptimiser instance for the given position. Either 
 *	returns one created on a previous call of create and return a new one.
 *
 *	@param	pos		world position which to retrieve the optimiser instance for.
 *
 *	@return			the requested optimiser instance.
 */
BillboardOptimiser::BillboardOptimiserPtr 
	BillboardOptimiser::retrieve( const Vector3& pos )
{
	BW_GUARD;
	BillboardOptimiserPtr result;
	BillboardOptimiser::s_chunkBBMapLock_.grab();
	ChunkBBMap & chunkBBMap = BillboardOptimiser::s_chunkBBMap_;
	uint32 id = makeID( pos );
	ChunkBBMap::iterator it = chunkBBMap.find( id );
	if ( it != s_chunkBBMap_.end() )
	{
		// Has already been added so it's guaranteed to have at least one group already..
		MF_ASSERT( it->second.size()>0 );

		// Need to guarantee there is at least one slot left so
		// find the first BillboardOptimiser with space left.
		BBVector::iterator itGroup = it->second.begin();
		while ( itGroup != it->second.end() )
		{
			BillboardOptimiser* pBBOpt = (*itGroup);
			if ( pBBOpt && !pBBOpt->isFull() )
				break;

			++itGroup;
		}

		if ( itGroup == it->second.end() )
		{
			// Failed to find an empty BillboardOptimiser
			// make a new one
			result = new BillboardOptimiser( pos );
			it->second.push_back( result.getObject() );
		}
		else
		{
			result = (*itGroup);
		}

		MF_ASSERT_DEBUG( !result->isFull() );
	}
	else 
	{
		result = new BillboardOptimiser( pos );

		BBVector newVec;
		newVec.push_back( result.getObject() );

		s_chunkBBMap_.insert( std::make_pair( id, newVec ) );		
	}
	BillboardOptimiser::s_chunkBBMapLock_.give();
	return result;	
}

/**
 *	Initialises the billboard optimiser engine (static).
 *
 *	@param	section	XML section containing all initialisation settings for 
 *					the optimiser engine (usually read from speedtree.xml).
 */
void BillboardOptimiser::init( DataSectionPtr section )
{
	BW_GUARD;
	std::string fxFile;
	DataSectionPtr optSection = section->openSection( "billboardOptimiser" );
	if ( optSection.exists() )
	{
		fxFile = optSection->readString("fxFile", "");
		if ( fxFile.empty() )
		{
			CRITICAL_MSG(
				"Effect for billboards optimiser "
				"not defined in config file.\n");
		}

		BillboardOptimiser::s_mipBias_ = optSection->readFloat(
			"mipBias", BillboardOptimiser::s_mipBias_ );

		DataSectionPtr texSection = optSection->openSection("textureAtlas");
		if ( texSection.exists() )
		{	
			// Minimum size that the texture atlas
			// can assume (at the highest mipmap level)
			BillboardOptimiser::s_atlasMinSize_ = texSection->readInt(
				"minSize", BillboardOptimiser::s_atlasMinSize_ );
				
			// Maximum size that the texture atlas
			// can assume (at the highest mipmap level)
			BillboardOptimiser::s_atlasMaxSize_ = texSection->readInt(
				"maxSize", BillboardOptimiser::s_atlasMaxSize_ );
				
			// Number of mipmap levels to 
			// use for the the texture atlas
			BillboardOptimiser::s_atlasMipLevels_ = texSection->readInt(
				"mipLevels", BillboardOptimiser::s_atlasMipLevels_ );
				
			// Save texure atlas into a dds file? (useful for debugging)
			BillboardOptimiser::s_saveTextureAtlas_ = texSection->readBool(
				"save", BillboardOptimiser::s_saveTextureAtlas_ );	
		}
	};

	Moo::TextureAggregator * aggreg[] = {
		BillboardsVBuffer::s_diffuseMapAggregator_,
#if SPT_ENABLE_NORMAL_MAPS
		BillboardsVBuffer::s_normalMapAggregator_
#endif // SPT_ENABLE_NORMAL_MAPS
	};
	
	for ( int i=0; i<sizeof(aggreg)/sizeof(Moo::TextureAggregator*); ++i )
	{
		if ( aggreg[i] != NULL )
		{
			aggreg[i]->setMinSize( BillboardOptimiser::s_atlasMinSize_ );
			aggreg[i]->setMaxSize( BillboardOptimiser::s_atlasMaxSize_ );
			aggreg[i]->setMipLevels( BillboardOptimiser::s_atlasMipLevels_ );
		}
	}

	MF_WATCH("SpeedTree/BB Optimiser/Min Alpha",
		BillboardVertex::s_minAlpha_, Watcher::WT_READ_WRITE,
		"The minimum alpha value for SpeedTree billboards." );

	MF_WATCH("SpeedTree/BB Optimiser/Max Alpha",
		BillboardVertex::s_maxAlpha_, Watcher::WT_READ_WRITE,
		"The maximum alpha value for SpeedTree billboards." );
	
	BillboardsVBuffer::init( optSection, fxFile );
}

/**
 *	Initialises the billboard FX files (static).
 */
void BillboardOptimiser::initFXFiles()
{
	BW_GUARD;
	BillboardsVBuffer::initFXFiles();
}

/**
 *	Finalises the billboard optimiser engine (static).
 */
void BillboardOptimiser::fini()
{
	BW_GUARD;
	BillboardsVBuffer::fini();
	
	if ( !BillboardOptimiser::s_treeTypes_.empty() )
	{
		WARNING_MSG(
			"BillboardOptimiser::fini: tree types still registered: %d\n", 
			BillboardOptimiser::s_treeTypes_.size() );
	}
	
	#define SAFEDELETE(POINTER) if (POINTER != NULL) delete POINTER;
	SAFEDELETE( BillboardsVBuffer::s_diffuseMapAggregator_ )

	#if SPT_ENABLE_NORMAL_MAPS
		SAFEDELETE( BillboardsVBuffer::s_normalMapAggregator_ )
	#endif // SPT_ENABLE_NORMAL_MAPS

	#undef SAFEDELETE
}

/**
 *	Adds a new tree type to the optimiser (static). Will copy the 
 *	billboard diffuse and normal textures maps into the texture 
 *	atlas and also the billboard vertex data to the treeTypeMap 
 *	cache.
 *
 *	@param	textureName		name of the original billboard texture map.
 *	@param	normalMap		name of the original normal map.
 *	@param	vertices		(in/out) vertice data. Will vertices with texture 
 *							coordinates modified to use the texture atlas.
 *	@param	treeFilename	name of the spt file (for error reporting).
 *	@param	material		diffuse and ambient material values for tree.
 *	@param	leafLightAdj	leaf-light-adjust value for tree.
 *
 *	@return					id assigned for this tree type.
 */
int BillboardOptimiser::addTreeType(
	Moo::BaseTexturePtr    texture, 
#if SPT_ENABLE_NORMAL_MAPS
	Moo::BaseTexturePtr    normalMap, 
#endif // SPT_ENABLE_NORMAL_MAPS
	const BBVertexBuffer & vertices,
	const char           * treeFilename,
	const float          * material,
	float                  leafLightAdj )
{
	BW_GUARD;
	int quadCount = vertices.size() / 6;
	
	TreeType treeType;
	treeType.vertices_       = &vertices;
	treeType.diffuseNAdjust_ = Vector4( material[0], material[1], material[2],
																leafLightAdj );
	treeType.ambient_        = Vector3( material[3], material[4], material[5] );
	
	Moo::TextureAggregator & diffAggreg = 
		*BillboardsVBuffer::s_diffuseMapAggregator_;

	#if SPT_ENABLE_NORMAL_MAPS
		Moo::TextureAggregator * normAggreg = 
			BillboardsVBuffer::s_normalMapAggregator_;
	#endif // SPT_ENABLE_NORMAL_MAPS
		
	// for each quad in billboard
	bool success = true;
	
	// this will trigger D3D9's "buffer locked more than once per frame" warning
	// if this tree have been loaded on this very frame. Fixing this warning is
	// more trouble than it's worth (there is no performance penalty involved).
	uint32 oldFlags = DX::getWrapperFlags();
	DX::setWrapperFlags( DX::WRAPPER_FLAG_IMMEDIATE_LOCK );

	if ( vertices.isValid() && vertices.lock() )
	{
		for ( int i=0; i<quadCount; ++i )
		{
			// Initialise the min/max to the first texture coordinate.
			Vector2 coord0(vertices[i*6].texCoords_[0], 1 + vertices[i*6].texCoords_[1]);
			Vector2 min = coord0;
			Vector2 max = coord0;

			// Go through the texure coordinate for this quad and get the min/max values
			// for putting into the texture aggregator.
			// skip the first one because it's assigned above.
			for (uint c=1; c<6; c++)
			{
				int index = i*6+c;
				float xCoord = vertices[index].texCoords_[0];
				float yCoord = 1 + vertices[index].texCoords_[1];

				if (xCoord <  min.x)
					min.x = xCoord;

				if (xCoord >  max.x)
					max.x = xCoord;

				if (yCoord <  min.y)
					min.y = yCoord;

				if (yCoord >  max.y)
					max.y = yCoord;
			}

			int normId = -1;
			int tileId = diffAggreg.addTile( texture, min, max );

			if ( tileId == -1 )
			{
				success = false;
				break;
			}

			treeType.tileIds_.push_back( tileId );
			
#if SPT_ENABLE_NORMAL_MAPS
			if ( normAggreg != NULL )
			{
				normId = normAggreg->addTile(
					normalMap.exists() ? normalMap : texture, 
					min, max );

				if ( normId != tileId )
				{
					ERROR_MSG(
						"Could not register tiles correctly. "
						"Falling back to non-optimised billboards. "
						"Most likely cause is lack of available video memory.\n" );
					diffAggreg.delTile( tileId );
					treeType.tileIds_.pop_back();
					success = false;
					break;
				}
				//MF_ASSERT(normId == tileId);
			}
#endif // SPT_ENABLE_NORMAL_MAPS
		}
		vertices.unlock();
	}
	else
	{
		success = false;
		ERROR_MSG(
			"BillboardOptimiser::addTreeType: "
			"Could not create/lock vertex buffer\n" );
	}

	DX::setWrapperFlags( oldFlags );

	int treeTypeId = -1;
	if ( success )
	{	
		treeTypeId = s_nextTreeTypeId_++; // post-increment is intentional
		s_treeTypes_.insert( std::make_pair( treeTypeId, treeType ) );
		
		saveTextureAtlas(
			BillboardOptimiser::s_saveTextureAtlas_, 
			diffAggreg.texture() );
	}
	else
	{
		// delete tree's tiles from texture aggregator
		IntVector::const_iterator tileIt  = treeType.tileIds_.begin();
		IntVector::const_iterator tileEnd = treeType.tileIds_.end();
		while( tileIt != tileEnd )
		{
			diffAggreg.delTile( *tileIt );
			#if SPT_ENABLE_NORMAL_MAPS
				if ( normAggreg != NULL )
				{
					normAggreg->delTile(*tileIt);
				}
			#endif // SPT_ENABLE_NORMAL_MAPS
			++tileIt;
		}
	}
	return treeTypeId;
}

/**
 *	Deletes a tree type from the billboard optimiser.
 *
 *	@param	treeTypeId	id of the tree type as returned by addTreeType.
 */
void BillboardOptimiser::delTreeType( int treeTypeId )
{
	BW_GUARD;
	TreeTypeMap & treeTypes = BillboardOptimiser::s_treeTypes_;
	TreeTypeMap::iterator treeTypeIt = treeTypes.find( treeTypeId );
	MF_ASSERT( treeTypeIt != treeTypes.end() );
	
	// delete tree's tiles from texture aggregator
	IntVector::const_iterator tileIt  = treeTypeIt->second.tileIds_.begin();
	IntVector::const_iterator tileEnd = treeTypeIt->second.tileIds_.end();
	while( tileIt != tileEnd )
	{
		BillboardsVBuffer::s_diffuseMapAggregator_->delTile( *tileIt );
		#if SPT_ENABLE_NORMAL_MAPS
			if ( BillboardsVBuffer::s_normalMapAggregator_ != NULL )
			{
				BillboardsVBuffer::s_normalMapAggregator_->delTile(*tileIt);
			}
		#endif // SPT_ENABLE_NORMAL_MAPS
		++tileIt;
	}
	
	// unregister tree type	
	treeTypes.erase( treeTypeIt );
	
	saveTextureAtlas(
		BillboardOptimiser::s_saveTextureAtlas_, 
		BillboardsVBuffer::s_diffuseMapAggregator_->texture() );
}

#ifdef EDITOR_ENABLED
/**
 *	Adds a new tree to this optimiser instance.
 *
 *	@param	treeTypeId	id of the tree type as returned by addTreeType.
 *	@param	world		tree's world transform.
 *	@param	alphaValue	tree's current alpha test value.
 *  @param  fogColour   current fog colour for this tree.
 *  @param  fogNear     current fog near for this tree.
 *  @param  fogFar      current fog far for this tree.
 *
 *	@return				Id of this tree instance with this optimiser.
 */
int BillboardOptimiser::addTree(
	int             treeTypeId,
	const Matrix &  world,
	float           alphaValue,
	const Vector4 & fogColour,
	float           fogNear,
	float           fogFar )
#else
/**
 *	Adds a new tree to this optimiser instance.
 *
 *	@param	treeTypeId	id of the tree type as returned by addTreeType.
 *	@param	world		tree's world transform.
 *	@param	alphaValue	tree's current alpha test value.
 *
 *	@return				Id of this tree instance with this optimiser.
 */
int BillboardOptimiser::addTree(
	int             treeTypeId,
	const Matrix &  world,
	float           alphaValue )
#endif // EDITOR_ENABLED
{
	BW_GUARD;
	TreeInstance treeInstance;
	treeInstance.world_  = world;
	treeInstance.typeId_ = treeTypeId;
	
	int treeID = 0;
	if ( this->freeSlots_.empty() )
	{
		treeID = this->trees_.size();
		this->trees_.push_back( treeInstance );
		if ( treeID % 4 == 0 )
		{
			// because setVectorArray expects an array of 
			// float4, always insert four floats at once. 
			for ( int i=0; i<4; ++i )
			{
				this->alphas_.push_back( 1.0f );
			}
			
			// this is what I'd like to be using but, for some
			// strange reason, it doesn't seems to work in vc2005
			// this->alphas_.insert(this->alphas_.end(), 4, 1.0f);
		}
	}
	else
	{
		treeID = this->freeSlots_.back();
		this->freeSlots_.pop_back();
		this->trees_[treeID] = treeInstance;
	}

#ifdef EDITOR_ENABLED
	this->updateTreeAlpha( treeID, alphaValue, fogColour, fogNear, fogFar );
#else
	this->updateTreeAlpha( treeID, alphaValue );
#endif
	
	// reset buffer so that they 
	// get recompiled on next draw 
	this->vbuffer_->reset();
	
	return treeID;
}

/**
 *	Removes a tree from this optimiser.
 *
 *	@param	treeID	id of tree, as returned from addTree.
 */
void BillboardOptimiser::removeTree( int treeID )
{
	BW_GUARD;
	MF_ASSERT(treeID >= 0 && treeID < int(this->alphas_.size()));
	
	this->trees_[treeID].typeId_ = -1;
	this->alphas_[treeID]       = 1.0f;
	this->freeSlots_.push_back( treeID );	
	
	if ( this->vbuffer_ != NULL )
	{
		this->vbuffer_->reset();
	}
}


#ifdef EDITOR_ENABLED
/**
 *	Update tree's alpha test value.
 *
 *	@param	treeID	id of tree, as returned from addTree.
 *	@param	alphaValue	tree's current alpha test value.
 *  @param  fogColour   current fog colour for this tree. 
 *  @param  fogNear     current fog near for this tree. 
 *  @param  fogFar      current fog far for this tree. 
 */
void BillboardOptimiser::updateTreeAlpha(
	int treeID, float alphaValue,
	const Vector4 & fogColour,
	float fogNear, float fogFar )
#else
/**
 *	Update tree's alpha test value.
 *
 *	@param	treeID	id of tree, as returned from addTree.
 *	@param	alphaValue	tree's current alpha test value.
 */
void BillboardOptimiser::updateTreeAlpha(
	int treeID, float alphaValue )
#endif // EDITOR_ENABLED
{
	BW_GUARD;
	// this is our only chance to touch the 
	// buffer to make sure it is not recycled. 
	if ( this->vbuffer_ == NULL )
	{
		// Acquire it first, if needed.
		this->vbuffer_ = BillboardsVBuffer::acquire();
		this->vbuffer_->bind( this, &this->trees_, &this->alphas_ );
	}
	
#ifdef EDITOR_ENABLED
	this->vbuffer_->touch( fogColour, fogNear, fogFar );
#else
	this->vbuffer_->touch( );
#endif // EDITOR_ENABLED

	// touch will reset the alphas vector if this is the first 
	// time this chunk is being updated since first last frame. 
	// So, setting the alpha value has got to be after touch.
	MF_ASSERT(treeID >= 0 && treeID < int(this->alphas_.size()));
	const float & minAlpha = BillboardVertex::s_minAlpha_;
	const float & maxAlpha = BillboardVertex::s_maxAlpha_;
	this->alphas_[treeID]  = 1 + alphaValue * (minAlpha/maxAlpha -1);
}


void BillboardOptimiser::tick()
{
	BW_GUARD;
	BillboardsVBuffer::tick();
}

/**
 *	Updates the optimiser engine (static).
 */
void BillboardOptimiser::update()
{
	BW_GUARD;
	BillboardsVBuffer::update();
	if ( BillboardOptimiser::s_treeTypes_.empty() )
	{
		if ( BillboardsVBuffer::s_diffuseMapAggregator_ != NULL )
		{
			BillboardsVBuffer::s_diffuseMapAggregator_->repack();
		}
		#if SPT_ENABLE_NORMAL_MAPS
			if ( BillboardsVBuffer::s_normalMapAggregator_ != NULL )
			{
				BillboardsVBuffer::s_normalMapAggregator_->repack();
			}
		#endif // SPT_ENABLE_NORMAL_MAPS
	}
}

/**
 *	Draws all visible trees.
 *
 *	@param	frameData	FrameData struct full of this frame's information.
 */
void BillboardOptimiser::drawAll( const FrameData & frameData )
{
	BW_GUARD;
	// NOTE: this is not being used currently, but should be eventually
	//BillboardsVBuffer::s_shaderCap_ = Moo::EffectManager::instance().PSVersionCap();

	DX::Texture * texture = 
					BillboardsVBuffer::s_diffuseMapAggregator_->texture();

	#if SPT_ENABLE_NORMAL_MAPS
		DX::Texture * normalMap = 
			(BillboardsVBuffer::s_normalMapAggregator_ != NULL) ?
				BillboardsVBuffer::s_normalMapAggregator_->texture() :
				NULL;
	#endif // SPT_ENABLE_NORMAL_MAPS
	
	const Matrix & texXForm = BillboardsVBuffer::s_diffuseMapAggregator_->transform();
	BillboardsVBuffer::drawVisible(
		frameData, 
		texture, 
#if SPT_ENABLE_NORMAL_MAPS
		normalMap, 
#endif // SPT_ENABLE_NORMAL_MAPS
		texXForm );
}
		
/**
 *	Releases the buffer object.
 */
void BillboardOptimiser::releaseBuffer()
{
	this->vbuffer_ = NULL;
}

/**
 *	Increments the reference count.
 */
void BillboardOptimiser::incRef() const
{
	++this->refCount_;
}

/**
 *	Increments the reference count. If tree type is no 
 *	longer referenced, remove it from chunkBBMap and delete it.
 */ 
void BillboardOptimiser::decRef() const
{
	BW_GUARD;
	--this->refCount_;
	if (this->refCount_ == 0)
	{
		BillboardOptimiser::s_chunkBBMapLock_.grab();
		ChunkBBMap & chunkBBMap = BillboardOptimiser::s_chunkBBMap_;

		ChunkBBMap::iterator it = chunkBBMap.find( id_ );
		if ( it != s_chunkBBMap_.end() )
		{
			BBVector::iterator itGroup = it->second.begin();
			while ( itGroup != it->second.end() )
			{
				if ( (*itGroup) == this )
					break;
				++itGroup;
			}
			MF_ASSERT( itGroup != it->second.end() );

			it->second.erase( itGroup );

			if ( it->second.empty() )
				chunkBBMap.erase( it );		
		}
		BillboardOptimiser::s_chunkBBMapLock_.give();
		delete const_cast<BillboardOptimiser*>(this);
	}		
}

/**
 *	Returns the current reference count.
 */
int BillboardOptimiser::refCount() const
{
	return this->refCount_;
}

/**
 *	Constructor (private).
 *
 *	@param	pos		world position which this optimiser instance is related to.
 */
BillboardOptimiser::BillboardOptimiser( const Vector3& pos ) :
	vbuffer_(NULL),
	trees_(),
	alphas_(),
	freeSlots_(),
	refCount_(0),
	id_( makeID(pos) )
{
}

/**
 *	Destructor.
 */
BillboardOptimiser::~BillboardOptimiser()
{
	BW_GUARD;
	if (this->vbuffer_ != NULL)
	{
		this->vbuffer_->bind( 0, 0, 0 );
	}
}


BillboardOptimiser::ChunkBBMap BillboardOptimiser::s_chunkBBMap_;
SimpleMutex BillboardOptimiser::s_chunkBBMapLock_;

// Configuration parameters
int BillboardOptimiser::s_atlasMinSize_   = 2048;
int BillboardOptimiser::s_atlasMaxSize_   = 2048;
int BillboardOptimiser::s_atlasMipLevels_ = 4;
int BillboardOptimiser::s_nextTreeTypeId_ = 0;
float BillboardOptimiser::s_mipBias_	  = 0.f;

// (64*4): linked to the size of the g_bbAlphaRefs array in billboards_opt.fx
uint16 BillboardOptimiser::s_maxInstanceCount_ = 64*4;
//uint16 BillboardOptimiser::s_maxInstanceCount_ = 196*4; // SM2 and above

BillboardOptimiser::TreeTypeMap BillboardOptimiser::s_treeTypes_;

bool BillboardOptimiser::s_saveTextureAtlas_ = false;

// ----------------------------------------------------------- BillboardsVBuffer

/**
 *	Initialises BillboardsVBuffer (static).
 *
 *	@param	section	XML section containing all initialisation settings for 
 *					the optimiser engine (usually read from speedtree.xml).
 *	@param	fxFile	name of the FX file to use when rendering the billboards.
 */
void BillboardsVBuffer::init(
	DataSectionPtr      section, 
	const std::string & fxFile )
{
	BW_GUARD;
	BillboardsVBuffer::s_fxFile_ = fxFile;

	MF_WATCH("SpeedTree/BB Optimiser/Visible buffers", 
		BillboardsVBuffer::s_visibleCounter_, Watcher::WT_READ_ONLY, 
		"Number of buffers in pool that are currently visible" );
		
	MF_WATCH("SpeedTree/BB Optimiser/Active buffers", 
		BillboardsVBuffer::s_activeCounter_, Watcher::WT_READ_ONLY, 
		"Number of buffers in pool that are currently "
		"active (these are not available for recycling).");

	MF_WATCH("SpeedTree/BB Optimiser/Pool size", 
		BillboardsVBuffer::s_poolSize_, Watcher::WT_READ_ONLY, 
		"Total number of buffers in pool. This number "
		"minus the number of active buffers is the total "
		" of buffers currently available for recycling." );

	MF_WATCH("SpeedTree/BB Optimiser/Munge Normal Depth",
		s_mungeNormalDepth_, Watcher::WT_READ_WRITE,
		"Depth of normal munging point - the point from which "
		"lighting normals are calculated for billboards." );

	if ( section.exists() )
	{
		DataSectionPtr buffSection = section->openSection( "buffers" );
		BillboardsVBuffer::s_framesToRecycle_ = buffSection->readInt(
			"framesToRecycle", BillboardsVBuffer::s_framesToRecycle_ );
			
		BillboardsVBuffer::s_framesToDelete_ = buffSection->readInt(
			"framesToDelete", BillboardsVBuffer::s_framesToDelete_ );
	}
			
	MF_WATCH("SpeedTree/BB Optimiser/Frames to recycle", 
		BillboardsVBuffer::s_framesToRecycle_, Watcher::WT_READ_WRITE, 
		"Number of frames a billboard optimiser buffer "
		"needs to go untouched before it is set for recycling" );
	
	MF_WATCH("SpeedTree/BB Optimiser/Frames to delete", 
		BillboardsVBuffer::s_framesToDelete_, Watcher::WT_READ_WRITE, 
		"Number of frames a billboard optimiser buffer "
		"needs to go untouched before it gets deleted" );
}

/**
 *	Initialises BillboardsVBuffer FX files (static).
 */
void BillboardsVBuffer::initFXFiles()
{
	BW_GUARD;
	Moo::EffectMaterialPtr effect(new Moo::EffectMaterial);
	if ( !effect->initFromEffect( BillboardsVBuffer::s_fxFile_ ) )
	{
		CRITICAL_MSG(
			"Could not load speedtree effect file: %s\n",
			BillboardsVBuffer::s_fxFile_.c_str() );
	}

	if ( effect->pEffect() )
	{
		BillboardsVBuffer::s_effect_ = effect;
		BillboardsVBuffer::s_lightingSetter_.reset(
			new Moo::EffectLightingSetter( 
				BillboardsVBuffer::s_effect_->pEffect()) );
	}
	
	BillboardsVBuffer::s_diffuseMapAggregator_ = 
		new Moo::TextureAggregator( &BillboardsVBuffer::resetAll );
		
	#if SPT_ENABLE_NORMAL_MAPS
	if ( BillboardsVBuffer::s_effect_->bumpMapped() )
	{
		BillboardsVBuffer::s_normalMapAggregator_ =
			new Moo::TextureAggregator();
	}
	#endif // SPT_ENABLE_NORMAL_MAPS
}

/**
 *	Finalises BillboardsVBuffer (static).
 */
void BillboardsVBuffer::fini()
{
	BW_GUARD;
	BillboardsVBuffer::s_effect_ = NULL;
	BillboardsVBuffer::s_lightingSetter_.reset();
}

/**
 *	Acquires a buffer instace. First, tries to reuse an inactive buffer 
 *	(defined as one that's gone unused for more than s_framesToRecycle 
 *	frames). If none is available, creates a new one.
 *
 *	@return		recycled or newly created instance of buffer.
 */
BillboardsVBuffer * BillboardsVBuffer::acquire()
{
	BW_GUARD;
	// if pool is empty or all buffers in pool
	// are still active, add more buffers to it
	const int & curFrameStamp = BillboardsVBuffer::s_curFrameStamp_;
	const int & framesToRecycle = BillboardsVBuffer::s_framesToRecycle_;
	BillboardsVBufferList & pool = BillboardsVBuffer::s_buffersPool_;
	if ( !pool.empty() && !pool.back()->vbuffer_.isValid() )
	{
		// there is a buffer in the pool that's not being 
		// used use it, instead of creating a new one.
	}
	else 
	{
		pool.push_back( new BillboardsVBuffer );
	}

	BillboardsVBufferList::iterator poolIt = pool.end();
	--poolIt;
	BillboardsVBufferPtr buffers = *poolIt;
	buffers->poolIt_ = poolIt;
	return buffers.getObject();
}

/**
 *	Ticks the billboard buffers (static).
 */
void BillboardsVBuffer::tick()
{
	BW_GUARD;
	BillboardsVBuffer::recycleBuffers();
	++BillboardsVBuffer::s_curFrameStamp_;
}

/**
 *	Updates the billboards buffers (static).
 */
void BillboardsVBuffer::update()
{
}

/**
 *	Resets all existing buffers.
 */
void BillboardsVBuffer::resetAll()
{
	BW_GUARD;
	typedef BillboardsVBufferList::iterator iterator;
	iterator buffersIt  = BillboardsVBuffer::s_buffersPool_.begin();
	iterator buffersEnd = BillboardsVBuffer::s_buffersPool_.end();
	while ( buffersIt != buffersEnd )
	{
		(*buffersIt)->reset();
		++buffersIt;
	}
}

/**
 *	Recycles the billboards bufferss (static). Buffers that have been 
 *	inactive for longer than s_framesToRecycle, are made available to 
 *	be reused by a different chunk. Buffers that have been inactive for 
 *	more than s_framesToDelete are deleted altogether.
 */
void BillboardsVBuffer::recycleBuffers()
{
	BW_GUARD;
	typedef BillboardsVBufferList::iterator iterator;
	BillboardsVBufferList & buffersPool = BillboardsVBuffer::s_buffersPool_;
	iterator buffersIt  = buffersPool.begin();
	iterator buffersEnd = buffersPool.end();
	const int & curFrameStamp   = BillboardsVBuffer::s_curFrameStamp_;
	const int & framesToRecycle = BillboardsVBuffer::s_framesToRecycle_;
	const int & framesToDelete  = BillboardsVBuffer::s_framesToDelete_;
	int visibleCounter = 0;
	int activeCounter  = 0;
	while ( buffersIt != buffersEnd )
	{
		// was it visible this frame?
		if ( (*buffersIt)->frameStamp_ == curFrameStamp )
		{
			++visibleCounter;
			++activeCounter;
		}
		// it wasn't rendered this frame
		// may be a good candidate to be deleted
		else
		{
			// but it still has trees. We 
			// can't delete this one then.
			if ( (*buffersIt)->vbuffer_.isValid() )
			{
				(*buffersIt)->activeStamp_ = curFrameStamp;
				++activeCounter;
			}
			else if ( framesToDelete > 0 && // is deleting enabled?
					(*buffersIt)->activeStamp_ < curFrameStamp-framesToDelete )
			{
				// it is not visible and haven't had no
				// trees in it for a while. Get rid of it
				buffersPool.erase( buffersIt, buffersEnd );
				break;
			}
		}
		++buffersIt;
	}
	BillboardsVBuffer::s_visibleCounter_ = visibleCounter;
	BillboardsVBuffer::s_activeCounter_  = activeCounter;
	BillboardsVBuffer::s_poolSize_       = buffersPool.size();
}

/**
 *	Draws all currently visible buffers (static).
 *
 *	@param	frameData		FrameData struct full of this frame's information.
 *	@param	texture			the texture atlas.
 *	@param	normalMap		the normal map atlas.
 *	@param	texTransform	Texture transform matrix to use.
 */
void BillboardsVBuffer::drawVisible(
	const FrameData & frameData, 
	DX::Texture     * texture,
#if SPT_ENABLE_NORMAL_MAPS
	DX::Texture     * normalMap,
#endif // SPT_ENABLE_NORMAL_MAPS
	const Matrix    & texTransform )
{
	BW_GUARD;
	if ( BillboardsVBuffer::s_effect_.get() == NULL )
	{
		return;
	}

	ComObjectWrap<ID3DXEffect> pEffect = 
		BillboardsVBuffer::s_effect_->pEffect()->pEffect();
		
	pEffect->SetTexture( "g_diffuseMap", texture );
	#if SPT_ENABLE_NORMAL_MAPS
		pEffect->SetTexture( "g_normalMap", normalMap );
	#endif // SPT_ENABLE_NORMAL_MAPS
	pEffect->SetMatrix( "g_viewProj", &frameData.viewProj_ );
	pEffect->SetBool( "g_texturedTrees", frameData.texturedTrees_ );
	pEffect->SetFloat( "g_bias", BillboardOptimiser::s_mipBias_ );

	float uvScale[] = { texTransform._11, texTransform._22, 0, 0 };
	pEffect->SetVector( "g_UVScale", (D3DXVECTOR4*)uvScale );

	float cameraDir[] = { 
		frameData.invView_._31, frameData.invView_._32, 
		frameData.invView_._33, 1.0f };
	pEffect->SetVector( "g_cameraDir", (D3DXVECTOR4*)cameraDir );
	
	float sun[] = { 
		frameData.sunDirection_.x, frameData.sunDirection_.y, 
		frameData.sunDirection_.z, 1.0f,
		frameData.sunDiffuse_.r, frameData.sunDiffuse_.g, 
		frameData.sunDiffuse_.b, 1.0f,
		frameData.sunAmbient_.r, frameData.sunAmbient_.g, 
		frameData.sunAmbient_.b, 1.0f };
	pEffect->SetVectorArray( "g_sun", (D3DXVECTOR4*)sun, 3 );

	Moo::rc().setFVF( AlphaBillboardVertex::fvf() );

	const int & curFrameStamp = BillboardsVBuffer::s_curFrameStamp_;
	typedef BillboardsVBufferList::const_iterator citerator;
	citerator buffersIt  = BillboardsVBuffer::s_buffersPool_.begin();
	citerator buffersEnd = BillboardsVBuffer::s_buffersPool_.end();
	while ( buffersIt != buffersEnd && 
			(*buffersIt)->frameStamp_ == curFrameStamp )
	{
		(*buffersIt)->draw();
		++buffersIt;
	}

	// unlock all treetype vbuffers that have been 
	// locked during compilation of billboards vbuffers
	typedef BillboardsVBuffer::BuffersVector::const_iterator BufIterator;
	BufIterator bufIt  = BillboardsVBuffer::s_lockedBuffers_.begin();
	BufIterator bufEnd = BillboardsVBuffer::s_lockedBuffers_.end();

	uint32 oldFlags = DX::getWrapperFlags();
	DX::setWrapperFlags( DX::WRAPPER_FLAG_IMMEDIATE_LOCK );

	while ( bufIt != bufEnd )
	{
		(*bufIt)->unlock();
		++bufIt;
	}
	
	DX::setWrapperFlags( oldFlags );

	// get it ready for the next frame
	BillboardsVBuffer::s_lockedBuffers_.clear();
	
	pEffect->SetTexture( "g_diffuseMap", NULL );

	#if SPT_ENABLE_NORMAL_MAPS
		pEffect->SetTexture( "g_normalMap", NULL );
	#endif // SPT_ENABLE_NORMAL_MAPS
}


#ifdef EDITOR_ENABLED
/**
 *  Touches buffer, effectively flagging it as visible.
 *
 *  @param  fogColour   current fog colour for this tree.
 *  @param  fogNear     current fog near for this tree.
 *  @param  fogFar      current fog far for this tree.
 */
void BillboardsVBuffer::touch(
	const Vector4 & fogColour, float fogNear, float fogFar )
#else
/**
 *	Touches buffer, effectively flagging it as visible.
 *
 */
void BillboardsVBuffer::touch()
#endif // EDITOR_ENABLED
{
	BW_GUARD;
	if ( this->frameStamp_ != BillboardsVBuffer::s_curFrameStamp_ )
	{
		BillboardsVBufferList & pool = BillboardsVBuffer::s_buffersPool_;
		if ( this->poolIt_ != pool.begin() )
		{
			// move this buffer to the front of the pool
			BillboardsVBufferList::iterator nextIt = this->poolIt_;
			++nextIt;
			pool.splice( pool.begin(), pool, this->poolIt_, nextIt );
			this->poolIt_ = pool.begin();
		}
		std::fill( this->alphas_->begin(), this->alphas_->end(), 1.0f );

		this->frameStamp_	= BillboardsVBuffer::s_curFrameStamp_;
#ifdef EDITOR_ENABLED
		this->fogColour_    = fogColour;
		this->fogNear_      = fogNear;
		this->fogFar_       = fogFar;
#endif // EDITOR_ENABLED
		this->lights_		= Moo::rc().lightContainer();
		this->specLights_	= Moo::rc().specularLightContainer();
	}
}

/**
 *	Draws buffer.
 */
void BillboardsVBuffer::draw()
{
	BW_GUARD;
	if (this->trees_ == NULL || this->owner_ == NULL)
	{
		// Workaround for crash in WE when ProcessAll, to let the artists work.
		// TODO: Find out why when called from photographThumbnail, "this" has
		// NULL trees_ and owner_.
		//ERROR_MSG( "BillboardsVBuffer::draw: Tried to render billboardVB that has no trees or owner\n" );
		
		// This situation usually occurs when old trees are left around and a 'tick' has yet to be
		// called (which would clear out these old trees)...so it should be safe to return without alerting
		// the user.... this should be tested thoroughly however.
		return;
	}

	if ( !this->isCompiled() )
	{
		this->compile();
	}

	if ( this->vbuffer_.isValid() )
	{
		Moo::Material::setVertexColour();
		ComObjectWrap<ID3DXEffect> pEffect = 
			BillboardsVBuffer::s_effect_->pEffect()->pEffect();

		if ( this->alphas_->size() )
		{
			if ( this->alphas_->size() > BillboardOptimiser::s_maxInstanceCount_ )
			{
				WARNING_MSG( "BillboardsVBuffer::draw(): Too many trees in this billboard group\n" );
			}

			D3DXVECTOR4* alphas = (D3DXVECTOR4*)&(*this->alphas_)[0];
			pEffect->SetVectorArray( "g_bbAlphaRefs", alphas, 
				int(ceil(float(this->alphas_->size()/4))) );

			// Can only upload 256 instances for anything below shader model 2
			//TODO: enable selection of maximum instances (requires rebuilding of the vertex buffers).
			//if ( s_shaderCap_ < 2 )
			//{
			//	pEffect->SetVectorArray( "g_bbAlphaRefs64", alphas, 
			//		int(ceil(float(this->alphas_->size()/4))) );
			//}
			//else
			//{
			//	pEffect->SetVectorArray( "g_bbAlphaRefs196", alphas, 
			//		int(ceil(float(this->alphas_->size()/4))) );
			//}
		}

		BillboardsVBuffer::s_lightingSetter_->begin();		
		if ( BillboardsVBuffer::s_effect_->begin() &&
				BillboardsVBuffer::s_effect_->beginPass(0) )
		{
			BillboardsVBuffer::s_lightingSetter_->apply(
				this->lights_, this->specLights_, false );

#ifdef EDITOR_ENABLED
			pEffect->SetVector( "fogColour", &this->fogColour_ );
			pEffect->SetFloat( "fogStart", this->fogNear_ );
			pEffect->SetFloat( "fogEnd",   this->fogFar_ );
#endif

			pEffect->CommitChanges();

			this->vbuffer_.activate();
			int primCount = this->vbuffer_.size() / 3;
			Moo::rc().drawPrimitive( D3DPT_TRIANGLELIST, 0, primCount );

			BillboardsVBuffer::s_effect_->endPass();
			BillboardsVBuffer::s_effect_->end();

		}
		// reset alphas
		this->alphas_->assign( this->alphas_->size(), 1.f ) ;
	}
}

/**
 *	Returns true if buffer is compiled and ready to be drawn.
 */
bool BillboardsVBuffer::isCompiled() const
{
	BW_GUARD;
	return this->vbuffer_.isValid();
}

/**
 *	Compiles buffer, making it ready to be drawn. Iterates through all the
 *	trees added to it's owner, creating a vertex buffer in the process.
 */
void BillboardsVBuffer::compile()
{
	BW_GUARD;
	MF_ASSERT(!this->vbuffer_.isValid());

	const TreeTypeMap & treeTypes = BillboardOptimiser::s_treeTypes_;
	
	// count number of vertices
	int verticesCount = 0;
	typedef TreeInstanceVector::const_iterator citerator;
	citerator bbIt    = this->trees_->begin();
	citerator bbEnd   = this->trees_->end();
	while ( bbIt != bbEnd )
	{
		int typeId = bbIt->typeId_;
		if ( typeId != -1 ) // skip removed trees
		{
			TreeTypeMap::const_iterator treeTypeIt = treeTypes.find( typeId );
			MF_ASSERT(treeTypeIt != treeTypes.end());
			const BBVertexBuffer & vertices = *treeTypeIt->second.vertices_;
			verticesCount += vertices.size();
		}
		++bbIt;
	}
	
	uint32 oldFlags = DX::getWrapperFlags();
	DX::setWrapperFlags( DX::WRAPPER_FLAG_IMMEDIATE_LOCK );

	// create vertex buffer
	if ( this->vbuffer_.reset(verticesCount) &&
		 this->vbuffer_.lock() )
	{
		typedef AlphaBillboardVBuffer::iterator abbIterator;
		abbIterator oBegin = this->vbuffer_.begin();
		abbIterator oIt    = oBegin;
		
		// populate vertex buffer
		citerator bbBegin = this->trees_->begin();
		bbIt    = bbBegin;
		bbEnd   = this->trees_->end();
		while ( bbIt != bbEnd )
		{
			int bbIndex = std::distance( bbBegin, bbIt );
			typedef BBVertexBuffer::const_iterator bvvIterator;
			
			int typeId = bbIt->typeId_;
			if ( typeId != -1 ) // skip removed trees
			{
				TreeTypeMap::const_iterator treeTypeIt = treeTypes.find( typeId );
				const BBVertexBuffer & vertices = *treeTypeIt->second.vertices_;
				
				bool locked = false;
				if ( vertices.isLocked() )
				{
					locked = true;
				}
				else
				{
					// this will trigger D3D9's "buffer locked more than once per frame" warning
					// if this tree have been loaded on this very frame. Fixing this warning is
					// more trouble than it's worth (there is no performance penalty involved).
					locked = vertices.lock();
					if ( locked )
					{
						BillboardsVBuffer::s_lockedBuffers_.push_back( &vertices );
					}
				}
				
				if ( locked )
				{
					bvvIterator iIt  = vertices.begin();
					bvvIterator iEnd = vertices.end();	
					int vertexIdx = 0;
					while ( iIt != iEnd )
					{
						oIt->position_ = bbIt->world_.applyPoint( iIt->position_ );

						//PCWJ - until we have normal mapped lighting on billboards,
						//change the lighting normals for all billboard vertices
						//so they point roughly in an upwards arc.  Do this by
						//taking the vector between an arbitrary point n metres below
						//the origin of the tree.
						
						//new code
						Vector3 mungedNormal( iIt->position_ );
						mungedNormal -= Vector3(0.f,s_mungeNormalDepth_,0.f);
						mungedNormal.normalise();
						oIt->lightNormal_ = bbIt->world_.applyVector( mungedNormal );
						oIt->lightNormal_.normalise();
						
						oIt->alphaNormal_ = bbIt->world_.applyVector(iIt->alphaNormal_);
						oIt->alphaNormal_.normalise();
						
						oIt->alphaIndex_   = FLOAT(bbIndex/4);
						oIt->alphaMask_[0] = bbIndex%4 == 0 ? 1.f : 0.f;
						oIt->alphaMask_[1] = bbIndex%4 == 1 ? 1.f : 0.f;
						oIt->alphaMask_[2] = bbIndex%4 == 2 ? 1.f : 0.f;
						oIt->alphaMask_[3] = bbIndex%4 == 3 ? 1.f : 0.f;

#if SPT_ENABLE_NORMAL_MAPS
						oIt->binormal_ = bbIt->world_.applyVector( iIt->binormal_ );
						oIt->binormal_.normalise();
						
						oIt->tangent_ = bbIt->world_.applyVector( iIt->tangent_ );
						oIt->tangent_.normalise();
#endif // SPT_ENABLE_NORMAL_MAPS

						oIt->diffuseNAdjust_ = treeTypeIt->second.diffuseNAdjust_;
						oIt->ambient_        = treeTypeIt->second.ambient_;

						Vector2 min;
						Vector2 max;		
						switch ( vertexIdx % 6 )
						{
							case 0:
							{
								// for each 6 vertices, retrieve the current 
								// texture coordinates from the texture aggregator.
								int id = treeTypeIt->second.tileIds_[vertexIdx/6];
								BillboardsVBuffer::s_diffuseMapAggregator_->getTileCoords(id, min, max);
								oIt->texCoords_[0] = max.x;
								oIt->texCoords_[1] = min.y;
								break;
							}
							case 1:
								oIt->texCoords_[0] = min.x;
								oIt->texCoords_[1] = min.y;
								break;
							case 2:
								oIt->texCoords_[0] = min.x;
								oIt->texCoords_[1] = max.y;
								break;
							case 3:
								oIt->texCoords_[0] = max.x;
								oIt->texCoords_[1] = min.y;
								break;
							case 4:
								oIt->texCoords_[0] = min.x;
								oIt->texCoords_[1] = max.y;
								break;
							case 5:
								oIt->texCoords_[0] = max.x;
								oIt->texCoords_[1] = max.y;
								break;
						};
						++vertexIdx;
						++iIt;
						++oIt;
					}
				}
				else
				{
					ERROR_MSG(
						"BillboardsVBuffer::compile: "
						"Could not lock vertex buffer\n" );
				}
			}
			++bbIt;
		}
		MF_ASSERT(oIt == this->vbuffer_.end());
		this->vbuffer_.unlock();
	}
	else
	{
		ERROR_MSG(
			"BillboardsVBuffer::compile: "
			"Could not create/lock vertex buffer\n" );
	}

	DX::setWrapperFlags( oldFlags );
}

/**
 *	Binds this buffer to the given owner and his data vectors.
 *
 *	@param	owner		the owning optimiser instance.
 *	@param	billboards	its billboard vector (will build the vertex buffer).
 *	@param	alphas		its alpha vector (will upload as shader constants).
 */
void BillboardsVBuffer::bind(
	BillboardOptimiser * owner, 
	TreeInstanceVector * billboards, 
	AlphaValueVector   * alphas )
{
	BW_GUARD;
	if ( this->owner_ != NULL )
	{
		this->owner_->releaseBuffer();
	}
	this->owner_   = owner;
	this->trees_   = billboards;
	this->alphas_  = alphas;
	this->reset();	
}

/**
 *	Resets the buffer (it will require 
 *	compilation before it can be drawn again).
 */
void BillboardsVBuffer::reset()
{
	BW_GUARD;
	this->vbuffer_.reset();
}

/**
 *	Default constructor (private).
 */
BillboardsVBuffer::BillboardsVBuffer() :
	frameStamp_( 0 ),
	activeStamp_( 0 ),
#ifdef EDITOR_ENABLED
	fogColour_( 1, 1, 1, 1 ),
	fogNear_( 0 ),
	fogFar_( 0 ),
#endif // EDITOR_ENABLED
	owner_( NULL )
{
	BW_GUARD;
}

/**
 *	Destructor (private).
 */
BillboardsVBuffer::~BillboardsVBuffer()
{
	BW_GUARD;
	if ( this->owner_ != NULL )
	{
		this->owner_->releaseBuffer();
	}
}

// Static variables
int BillboardsVBuffer::s_curFrameStamp_ = 0;

// Watched variables
int BillboardsVBuffer::s_visibleCounter_ = 0;
int BillboardsVBuffer::s_activeCounter_  = 0;
int BillboardsVBuffer::s_poolSize_       = 0;

// Configuration parameters
int BillboardsVBuffer::s_framesToRecycle_ = 50;
int BillboardsVBuffer::s_framesToDelete_  = 100;

BillboardsVBuffer::BillboardsVBufferList   BillboardsVBuffer::s_buffersPool_;
BillboardsVBuffer::EffectLightingSetterPtr BillboardsVBuffer::s_lightingSetter_;
BillboardsVBuffer::BuffersVector           BillboardsVBuffer::s_lockedBuffers_;
Moo::EffectMaterialPtr					   BillboardsVBuffer::s_effect_;
std::string                                BillboardsVBuffer::s_fxFile_;

Moo::TextureAggregator * BillboardsVBuffer::s_diffuseMapAggregator_ = NULL;

#if SPT_ENABLE_NORMAL_MAPS
Moo::TextureAggregator * BillboardsVBuffer::s_normalMapAggregator_  = NULL;
#endif // SPT_ENABLE_NORMAL_MAPS

int BillboardsVBuffer::s_shaderCap_ = 0;

namespace { // anonymous

// Helper functions
void saveTextureAtlas( bool save, DX::BaseTexture * texture )
{
	BW_GUARD;
	if ( save )
	{
		HRESULT hresult = D3DXSaveTextureToFile(
			L"bboptimiser.dds", D3DXIFF_DDS, texture, NULL );
	}
}

} // namespace anonymous
} // namespace speedtree

#endif // SPEEDTREE_SUPPORT ----------------------------------------------------
