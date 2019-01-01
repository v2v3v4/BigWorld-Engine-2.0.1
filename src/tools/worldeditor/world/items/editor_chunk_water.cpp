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
#include "worldeditor/world/items/editor_chunk_water.hpp"
#include "worldeditor/world/items/editor_chunk_substance.ipp"
#include "worldeditor/world/items/editor_chunk_vlo.hpp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/world/vlo_manager.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/editor/item_editor.hpp"
#include "worldeditor/editor/item_properties.hpp"
#include "worldeditor/editor/snaps.hpp"
#include "model/super_model.hpp"
#include "chunk/chunk_model.hpp"
#include "chunk/chunk_manager.hpp"
#include "chunk/chunk_space.hpp"
#include "resmgr/xml_section.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/resource_cache.hpp"

#define MAX_INDEX_COUNT 100000


std::vector<EditorChunkWater*>	EditorChunkWater::instances_;
SimpleMutex						EditorChunkWater::instancesMutex_;


namespace
{
	AutoConfigString s_chunkItemMetaDataConfig( "editor/metaDataConfig/chunkItem", "helpers/meta_data/chunk_item.xml" );


	bool equalOrGreater( float v1, float v2 )
	{
		if (almostEqual( v1, v2 ))
		{
			return true;
		}
		return v1 > v2;
	}

} // anonymous namespace


class ChunkWaterConfig
{
public:
	ChunkWaterConfig( Water::WaterState& config ) :
		config_( config )
	{
	}

	int numIndices() const
	{
		return numIndicesInternal( config_.tessellation_, config_.size_.x, config_.size_.y );
	}

	bool validateTessellation( float &tess )
	{
		float width = config_.size_.x;
		float height = config_.size_.y;
		int count = numIndicesInternal( tess, width, height );

		//TODO: this limitation is kinda wrong...... it's just checking the 
		// max number of indices possible for a certain size/tessellation ..
		// if the water intersects terrain there will be a lot less indices, 
		// but be rejected by this test...

		if ((count < MAX_INDEX_COUNT) && width >= tess && height >= tess)	
			return true;
		else
			return false;
	}

	bool validateCellSize( float &size )
	{
		//TODO: the cell size needs to be limited based on tessellation and size
		if ( size < (2.f*config_.tessellation_) )	
			size = 2.f*config_.tessellation_;
		else if ( size< (config_.size_.x/10.f))
			size = (config_.size_.x/10.f);

		return true;
	}

	bool validateSize( float sizeX, float sizeY )
	{
		int count = numIndicesInternal( config_.tessellation_, sizeX, sizeY );

		if (config_.simCellSize_<=0)
			config_.simCellSize_ = 100.f;

		if ((count < MAX_INDEX_COUNT) && equalOrGreater( sizeX, config_.tessellation_ ) && 
				equalOrGreater( sizeY, config_.tessellation_ ))	
		{
			return true;	
		}
		else
		{
			return false;
		}
	}

protected:
	int numIndicesInternal( float tess, float sizeX, float sizeY ) const
	{
		int countX = int( ceilf( sizeX / tess ) ) + 1;
		int countZ = int( ceilf( sizeY / tess ) ) + 1;
		return countX * countZ;
	}

	Water::WaterState&	config_;
};


/**
 *	This is a MatrixProxy for chunk item pointers.
 */
class MyChunkItemMatrix : public ChunkWaterConfig, public ChunkItemMatrix
{
public:
	MyChunkItemMatrix( ChunkItemPtr pItem, Water::WaterState&	config)
		: ChunkItemMatrix( pItem ), ChunkWaterConfig( config )  {}

	bool EDCALL setMatrix( const Matrix & m );
};

bool MyChunkItemMatrix::setMatrix( const Matrix & m )
{
	BW_GUARD;

	Matrix newTransform = m;

	// Snap the transform of the matrix if it's asking for a different
	// translation
	Matrix currentTransform;
	getMatrix( currentTransform, false );
	if (!almostEqual( currentTransform.applyToOrigin(), newTransform.applyToOrigin() ))
	{
		Vector3 t = newTransform.applyToOrigin();

		Vector3 snaps = movementSnaps_;
		if (snaps == Vector3( 0.f, 0.f, 0.f ) && WorldManager::instance().snapsEnabled() )
			snaps = WorldManager::instance().movementSnaps();

		Snap::vector3( t, snaps );

		newTransform.translation( t );
	}

	BoundingBox bbox;
	pItem_->edBounds(bbox);
	bbox.transformBy( newTransform );
	Vector3 boxVolume = bbox.maxBounds() - bbox.minBounds();	

	float width = boxVolume.x;
	float height = boxVolume.z;
	bool result = false;

	float spaceSizeX = 0.f;
	float spaceSizeZ = 0.f;

	if (pItem_->chunk())
	{
		spaceSizeX = pItem_->chunk()->space()->maxCoordX() - pItem_->chunk()->space()->minCoordX();
		spaceSizeZ = pItem_->chunk()->space()->maxCoordZ() - pItem_->chunk()->space()->minCoordZ();		
	}

	if ( (width <= spaceSizeX && height <= spaceSizeZ) && validateSize(width, height) )
	{
		// always transient
		pItem_->edTransform( newTransform, true );
		result = true;
	}
	return result;
}

// -----------------------------------------------------------------------------
// Section: EditorChunkWater
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
EditorChunkWater::EditorChunkWater( std::string uid ) :
	changed_( false ),
	localOri_( 0.0f )
{
	BW_GUARD;

	SimpleMutexHolder holder(instancesMutex_);

	// Add us to the list of instances
	instances_.push_back( this );
	setUID(uid);

	waterModel_ = Model::get( "resources/models/water.model" );
	ResourceCache::instance().addResource( waterModel_ );
}


/**
 *	Destructor.
 */
EditorChunkWater::~EditorChunkWater()
{
	BW_GUARD;

	SimpleMutexHolder holder(instancesMutex_);

	// Remove us from the list of instances
	std::vector<EditorChunkWater*>::iterator p = std::find(
		instances_.begin(), instances_.end(), this );

	MF_ASSERT( p != instances_.end() );

	instances_.erase( p );
}
//
//void EditorChunkSubstance<ChunkWater>::draw()
//{
//}

//bool EditorChunkSubstance<ChunkWater>::load(DataSectionPtr pSect, Chunk * pChunk )
//{
//	pOwnSect_ = pSect;
//
//	//perhaps we dont need this specialisation now....
//	edCommonLoad( pSect );
//	return this->ChunkWater::load( pSect, pChunk );
//}

//
//bool EditorChunkSubstance<ChunkWater>::addYBounds( BoundingBox& bb ) const
//{
//	// sorry for the conversion
////	bb.addBounds( ( ( EditorChunkSubstance<LT>* ) this)->edTransform().applyToOrigin().y );
//	return true;
//}

void EditorChunkSubstance<ChunkWater>::addAsObstacle()
{
}

void EditorChunkWater::edDelete( ChunkVLO* instigator )
{
	BW_GUARD;

	VeryLargeObject::edDelete( instigator );
}

/**
 *	This method saves the data section pointer before calling its
 *	base class's load method
 */
bool EditorChunkWater::load( DataSectionPtr pSection, Chunk * pChunk )
{
	BW_GUARD;

	bool ok = this->EditorChunkSubstance<ChunkWater>::load( pSection, pChunk );
	if (ok)
	{
		localOri_ = config_.orientation_;
		worldPos_ = config_.position_;

		origin_.setIdentity();
		Matrix rtr; rtr.setRotateY( config_.orientation_ );
		origin_.postMultiply( rtr );
		origin_.translation( config_.position_ );

		bb_.setBounds( 
			config_.position_ + Vector3(	-config_.size_.x * 0.5f,
											0, 
											-config_.size_.y * 0.5f ),
			config_.position_ + Vector3(	config_.size_.x * 0.5f,
											0,
											config_.size_.y * 0.5f ) );

		static MetaData::Desc s_dataDesc( s_chunkItemMetaDataConfig.value() );
		ok = metaData_.load( pSection, s_dataDesc );
	}

	return ok;
}

void EditorChunkSubstance<ChunkWater>::toss(Chunk * pChunk  )
{
}

void EditorChunkWater::toss( )
{
	BW_GUARD;

	ChunkItemList::iterator it;
	for (it = itemList_.begin(); it != itemList_.end(); it++)
	{
		//only need to toss one for all the refs to be re-evaluated
		if ((*it))
		{
			(*it)->toss( (*it)->chunk() );
			break;
		}
	}
}


void EditorChunkWater::cleanup()
{
	BW_GUARD;

	if ( VLOManager::instance()->needsCleanup( uid_ ) )
	{
		// If the object was saved (not dirty) and was deleted, then we can 
		// remove the VLO file.
		if (pWater_)
			pWater_->deleteData();

		std::string fileName( '_' + uid_ + ".vlo" );
		DataSectionPtr ds = WorldManager::instance().geometryMapping()->pDirSection();
		ds->deleteSection( fileName );
			
		if (BWResource::fileExists( fileName ))
			WorldManager::instance().eraseAndRemoveFile( fileName );

		fileName = chunkPath_ + fileName;

		if (BWResource::fileExists( fileName ))
			WorldManager::instance().eraseAndRemoveFile( fileName );

		fileName = uid_ + ".vlo";
		ds = WorldManager::instance().geometryMapping()->pDirSection();
		ds->deleteSection( fileName );
			
		if (BWResource::fileExists( fileName ))
			WorldManager::instance().eraseAndRemoveFile( fileName );

		fileName = chunkPath_ + fileName;

		if (BWResource::fileExists( fileName ))
			WorldManager::instance().eraseAndRemoveFile( fileName );

		dataSection_=NULL;
	}
}


/**
 *
 */
void EditorChunkWater::saveFile( Chunk* pChunk )
{
	BW_GUARD;

	if ( VLOManager::instance()->isDeleted( uid_ ) )
		return;

	if (changed_ && dataSection_)
	{
		if (pWater_)
		{
			pWater_->saveTransparencyTable();
			config_.depth_ = pWater_->getDepth();
			DataSectionPtr waterSection = dataSection_->openSection( "water" );
			if (waterSection)
				waterSection->writeFloat( "depth", config_.depth_ );
		}

		dataSection_->save();
		changed_ = false;
	}
}

/**
 *
 */
void EditorChunkWater::save()
{
	BW_GUARD;

	VeryLargeObject::save();

	if (!dataSection_)
		return;

	DataSectionPtr waterSection = dataSection_->openSection( "water" );

	if (!waterSection)
		return;

	waterSection->writeVector3( "position", worldPos_ );
	waterSection->writeFloat( "orientation", localOri_ );
	waterSection->writeVector3( "size", Vector3( config_.size_.x, 0.f, config_.size_.y ) );

	waterSection->writeFloat( "fresnelConstant", config_.fresnelConstant_ );
	waterSection->writeFloat( "fresnelExponent", config_.fresnelExponent_ );

	waterSection->writeVector4( "reflectionTint", config_.reflectionTint_ );
	waterSection->writeFloat( "reflectionStrength", config_.reflectionScale_ );

	waterSection->writeVector4( "refractionTint", config_.refractionTint_ );
	waterSection->writeFloat( "refractionStrength", config_.refractionScale_ );

	waterSection->writeFloat( "tessellation", config_.tessellation_ );
	waterSection->writeFloat( "consistency", config_.consistency_ );

	waterSection->writeFloat( "textureTessellation", config_.textureTessellation_ );

	waterSection->writeVector2( "scrollSpeed1", config_.scrollSpeed1_ );	
	waterSection->writeVector2( "scrollSpeed2", config_.scrollSpeed2_ );
	waterSection->writeVector2( "waveScale", config_.waveScale_ );
	

	waterSection->writeFloat( "windVelocity", config_.windVelocity_ );

	waterSection->writeFloat( "sunPower", config_.sunPower_ );
	waterSection->writeFloat( "sunScale", config_.sunScale_ );

	waterSection->writeString( "waveTexture", config_.waveTexture_ );

	waterSection->writeFloat( "cellsize", config_.simCellSize_ );	
	waterSection->writeFloat( "smoothness", config_.smoothness_ );

	waterSection->writeFloat( "depth", config_.depth_ );

	waterSection->writeString( "foamTexture", config_.foamTexture_ );
	waterSection->writeString( "reflectionTexture", config_.reflectionTexture_ );

	waterSection->writeVector4( "deepColour", config_.deepColour_ );
	waterSection->writeFloat( "fadeDepth", config_.fadeDepth_ );
	waterSection->writeFloat( "foamIntersection", config_.foamIntersection_ );
	waterSection->writeFloat( "foamMultiplier", config_.foamMultiplier_ );
	waterSection->writeFloat( "foamTiling", config_.foamTiling_ );
	waterSection->writeBool( "bypassDepth", config_.bypassDepth_ );

	waterSection->writeBool( "useCubeMap", config_.useCubeMap_ );

	waterSection->writeBool( "useEdgeAlpha", config_.useEdgeAlpha_ );
	waterSection->writeBool( "useSimulation", config_.useSimulation_ );	

	waterSection->writeInt( "visibility", config_.visibility_ );	

	metaData_.save( waterSection );
}


/**
 *	Save any property changes to this data section
 */
bool EditorChunkWater::edSave( DataSectionPtr pSection )
{
	return true;
}


/**
 *  Draw water red when it's read-only
 */
void EditorChunkWater::drawRed(bool val)
{
	BW_GUARD;

    if (pWater_)
		pWater_->drawRed(val);
}


/**
 *  Highlight water when selecting
 */
void EditorChunkWater::highlight(bool val)
{
	BW_GUARD;

    if (pWater_)
		pWater_->highlight(val);
}


/**
 *
 */
const Matrix & EditorChunkWater::origin()
{
	origin_.setIdentity();
	origin_.translation( worldPos_ );
	return origin_;
}


/**
 *	Get the current transform
 */
const Matrix & EditorChunkWater::edTransform()
{
	transform_.setScale( config_.size_.x, 1.f, config_.size_.y );
	Matrix rtr; rtr.setRotateY( localOri_ );
	transform_.postMultiply( rtr );
	transform_.translation( worldPos_ );
	return transform_;
}


/**
 *
 */
const Matrix & EditorChunkWater::localTransform( )
{
	transform_.setScale( config_.size_.x, 1.f, config_.size_.y );
	Matrix rtr; rtr.setRotateY( localOri_ );
	transform_.postMultiply( rtr );
	transform_.translation( worldPos_ );

	return transform_;
}


/**
 *
 */
const Matrix & EditorChunkWater::localTransform( Chunk * pChunk )
{
	BW_GUARD;

	edTransform();
	if (pChunk)
		transform_.postMultiply( pChunk->transformInverse() );

	return transform_;
}


/**
 *	This is a helper class for setting a float property on a chunk water item
 */
class ChunkWaterFloat : public UndoableDataProxy<FloatProxy>
{
public:
	explicit ChunkWaterFloat( EditorChunkWaterPtr pItem, float & f,
		float min=0.f, float max=1.f, int digits=1,
		bool limitMin=true, bool limitMax=true  ) :
		pItem_( pItem ),
		f_( f ),
		minValue_( min ),
		maxValue_( max ),
		digits_( digits ),
		limitMax_( limitMax ),
		limitMin_( limitMin )
	{ }


	virtual float EDCALL get() const			{ return f_; }
	virtual void EDCALL setTransient( float f )	{ f_ = f; }

	virtual bool EDCALL setPermanent( float f )
	{
		BW_GUARD;

		// complain if it's invalid
		//if (f < 0.f || f > 1.f) return false;
		//if (f < 0.f) return false;

		if ( (limitMin_ && (f < minValue_)) || (limitMax_ && (f > maxValue_)) ) return false;

		// set it
		this->setTransient( f );

		// flag the chunk as having changed
		// update its data section
		pItem_->edSave( pItem_->pOwnSect() );

		// and get it to regenerate stuff
		pItem_->toss( );
		pItem_->dirty();
		return true;
	}

	bool getRange( float& min, float& max, int& digits ) const
	{
		if ((limitMin_) && (limitMax_))
		{
			min = minValue_;
			max = maxValue_;
			digits = digits_;
			return true;
		}
		return false;
	}

	virtual std::string EDCALL opName()
	{
		//return "Set " + pItem_->edDescription() + " float property";
		BW_GUARD;

		return LocaliseUTF8("WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/SET_FLOAT_PROPERTY");
	}

private:
	EditorChunkWaterPtr	pItem_;
	float &				f_;

	float				minValue_,maxValue_;
	int					digits_;
	bool				limitMax_, limitMin_;
};

class ChunkWaterTessellation : public ChunkWaterConfig, public ChunkWaterFloat
{
public:
	ChunkWaterTessellation( EditorChunkWaterPtr pItem, Water::WaterState& config ) :
		ChunkWaterFloat( pItem, config.tessellation_, 1.f,1000.f,1,true,false),
		ChunkWaterConfig( config )
	{ }

	virtual bool EDCALL setPermanent( float f )
	{
		BW_GUARD;

		if ( validateTessellation( f ) )
		{
			return ChunkWaterFloat::setPermanent(f);
		}
		else
		{
			WorldManager::instance().addCommentaryMsg(LocaliseUTF8("WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/BAD_MESH_SIZE"));
			return false;
		}
	}
};

class ChunkWaterSimCellSize :  public ChunkWaterConfig, public ChunkWaterFloat
{
public:
	ChunkWaterSimCellSize( EditorChunkWaterPtr pItem, Water::WaterState& config ) :
		ChunkWaterFloat( pItem, config.simCellSize_, 0.f,1000.f,1),
		ChunkWaterConfig( config )
	{ }

	virtual bool EDCALL setPermanent( float f )
	{
		BW_GUARD;

		if ( validateCellSize( f ) )
			return ChunkWaterFloat::setPermanent(f);
		else
			return false;
	}
};


/**
 *	This helper class wraps up a water data property
 */
template <class DTYPE> class ChunkWaterData : public UndoableDataProxy<DTYPE>
{
public:
	explicit ChunkWaterData(EditorChunkWaterPtr pItem,
		typename DTYPE::Data& v, bool updateReferences = false, bool doSyncInit = false ) :
		pItem_( pItem ),
		v_( v ),
		updateReferences_( updateReferences ),
		doSyncInit_( doSyncInit )
	{
	}

	virtual typename DTYPE::Data EDCALL get() const
	{
		return v_;
	}

	virtual void EDCALL setTransient( typename DTYPE::Data v )
	{
		v_ = v;
	}

	virtual bool EDCALL setPermanent( typename DTYPE::Data v )
	{
		BW_GUARD;

		setValue( v );
		return true;
	}

	virtual std::string EDCALL opName()
	{
		BW_GUARD;

		//return "Set " + pItem_->edDescription() + " colour";
		return LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/SET_DATA");
	}

protected:	
	void setValue( typename DTYPE::Data v )
	{
		BW_GUARD;

		// set it
		this->setTransient( v );

		// update its data section
		pItem_->edSave( pItem_->pOwnSect() );

		// and get it to regenerate stuff
		pItem_->toss( );
		pItem_->dirty();

		if (doSyncInit_)
		{
			VeryLargeObject::ChunkItemList & items = pItem_->chunkItems();
			if (!items.empty())
			{
				pItem_->syncInit( items.front() );
			}
		}

		if ( updateReferences_ )
			VLOManager::instance()->updateReferences( pItem_ );
	}
private:
	EditorChunkWaterPtr pItem_;
	typename DTYPE::Data & v_;
	bool updateReferences_;
	bool doSyncInit_;
};


bool EDCALL ChunkWaterData<Vector4Proxy>::setPermanent( Vector4 v )
{
	BW_GUARD;

	// make it valid
	if (v.x < 0.f) v.x = 0.f;
	if (v.x > 1.f) v.x = 1.f;
	if (v.y < 0.f) v.y = 0.f;
	if (v.y > 1.f) v.y = 1.f;
	if (v.z < 0.f) v.z = 0.f;
	if (v.z > 1.f) v.z = 1.f;
	v.w = 1.f;

	setValue( v );
	return true;
}


//need a new matrix proxy / float proxy etc etc..... and use them to update the right variables...
//perhaps i can get the scaling to work from the reference transformations ALA the translation? 

/**
 *	Add the properties to the given editor
 */
bool EditorChunkWater::edEdit( class GeneralEditor & editor, const ChunkItemPtr pItem )
{
	BW_GUARD;

	metaData_.edit( editor, L"", false );

	MatrixProxy * pMP = new MyChunkItemMatrix( pItem, config_ );
	editor.addProperty( new GenPositionProperty( 
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/POSITION"), pMP ) );
	editor.addProperty( new GenRotationProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/ORIENTATION"), pMP ) );
	editor.addProperty( new GenScaleProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/SIZE"), pMP ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/FRESNEL_CONSTANT"),
		new ChunkWaterFloat( this, config_.fresnelConstant_ ) ) );
	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/FRESNEL_EXPONENT"),
		new ChunkWaterFloat( this, config_.fresnelExponent_, 0.f, 7.f ) ) );

	editor.addProperty( new ColourProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/REFLECTION_TINT"),
		new ChunkWaterData< Vector4Proxy >( this, config_.reflectionTint_ ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/REFLECTION_STRENGTH"),
		new ChunkWaterFloat( this, config_.reflectionScale_ ) ) );

	editor.addProperty( new ColourProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/REFRACTION_TINT"),
		new ChunkWaterData< Vector4Proxy >( this, config_.refractionTint_ ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/REFRACTION_STRENGTH"),
		new ChunkWaterFloat( this, config_.refractionScale_ ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/MESH_SIZE"),
		new ChunkWaterTessellation( this, config_ ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/CONSISTENCY"),
		new ChunkWaterFloat( this, config_.consistency_, 0.f, 0.98f, 2 ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/TEXTURE_SIZE"),
		new ChunkWaterFloat( this, config_.textureTessellation_,0.f,1000.f, 1, true,false ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/SCROLL_SPEED_1_U"),
		new ChunkWaterData<FloatProxy>( this, config_.scrollSpeed1_.x ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/SCROLL_SPEED_1_V"),
		new ChunkWaterData<FloatProxy>( this, config_.scrollSpeed1_.y ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/SCROLL_SPEED_2_U"),
		new ChunkWaterData<FloatProxy>( this, config_.scrollSpeed2_.x ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/SCROLL_SPEED_2_V"),
		new ChunkWaterData<FloatProxy>( this, config_.scrollSpeed2_.y ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/WAVE_SCALE_U"),
		new ChunkWaterData<FloatProxy>( this, config_.waveScale_.x ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/WAVE_SCALE_V"),
		new ChunkWaterData<FloatProxy>( this, config_.waveScale_.y ) ) );


	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/WIND_SPEED"),
		new ChunkWaterData<FloatProxy>( this, config_.windVelocity_ ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/SURFACE_SMOOTHNESS"),
		new ChunkWaterFloat( this, config_.smoothness_ ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/SUN_POWER"),
		new ChunkWaterFloat( this, config_.sunPower_, 1.f, 256.f ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/SUN_SCALE"),
		new ChunkWaterFloat( this, config_.sunScale_, 0.f, 10.f ) ) );

	editor.addProperty( new ColourProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/DEEP_COLOUR"),
		new ChunkWaterData< Vector4Proxy >( this, config_.deepColour_ ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/FADE_DEPTH"),
		new ChunkWaterData<FloatProxy>( this, config_.fadeDepth_ ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/FOAM_INTERSECTION"),
		new ChunkWaterData<FloatProxy>( this, config_.foamIntersection_ ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/FOAM_MULTIPLIER"),
		new ChunkWaterData<FloatProxy>( this, config_.foamMultiplier_ ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/FOAM_TILING"),
		new ChunkWaterData<FloatProxy>( this, config_.foamTiling_ ) ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/BYPASS_DEPTH"),
		new ChunkWaterData<BoolProxy>( this, config_.bypassDepth_ ) ) );

	TextProperty* pRTProp = new TextProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/REFLECTION_TEXTURE"),
		new ChunkWaterData< StringProxy >(this, config_.reflectionTexture_ ) );
	pRTProp->fileFilter( L"Cube Maps(*.dds)|*.dds||" );
	editor.addProperty(pRTProp);

	TextProperty* pTProp = new TextProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/FOAM_TEXTURE"),
		new ChunkWaterData< StringProxy >(this, config_.foamTexture_ ) );
	pTProp->fileFilter( L"Texture files(*.jpg;*.tga;*.bmp;*.dds)|*.jpg;*.tga;*.bmp;*.dds||" );
	editor.addProperty(pTProp);

	TextProperty* pProp = new TextProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/WAVE_TEXTURE"),
		new ChunkWaterData< StringProxy >(this, config_.waveTexture_ ) );
	pProp->fileFilter( L"Texture files(*.jpg;*.tga;*.bmp;*.dds)|*.jpg;*.tga;*.bmp;*.dds||" );
	editor.addProperty(pProp);

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/SIM_CELL_SIZE"),
		new ChunkWaterSimCellSize( this, config_ ) ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/USE_CUBE_MAP"),
		new ChunkWaterData< BoolProxy >( this, config_.useCubeMap_ ) ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/USE_EDGE_ALPHA"),
		new ChunkWaterData< BoolProxy >( this, config_.useEdgeAlpha_ ) ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/USE_SIMULATION"),
		new ChunkWaterData< BoolProxy >( this, config_.useSimulation_ ) ) );

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/REFLECT_BOTTOM"),
		new ChunkWaterData< BoolProxy >( this, config_.reflectBottom_, false, true /* do sync init */ ) ) );

	DataSectionPtr ds = new XMLSection( "visibility" );
	ds->writeInt( LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/ALWAYS_VISIBLE"), Water::ALWAYS_VISIBLE );
	ds->writeInt( LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/INSIDE_ONLY"), Water::INSIDE_ONLY );
	ds->writeInt( LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/OUTSIDE_ONLY"), Water::OUTSIDE_ONLY );
	editor.addProperty( new ChoiceProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_WATER/VISIBILITY"),
		new ChunkWaterData< UIntProxy >( this, config_.visibility_, true ),
		ds ) );

	return true;
}


/**
 *	Called when an property has been modified
 */
void EditorChunkWater::edCommonChanged()
{
	BW_GUARD;

	VeryLargeObject::ChunkItemList& items = chunkItems();

	if (!items.empty())
	{
		ChunkItemPtr item = *items.begin();

		item->edCommonChanged();
	}
}


/**
 *	Returns true if the water should be visible inside shells.
 */
bool EditorChunkWater::visibleInside() const
{
	return
		config_.visibility_ == Water::ALWAYS_VISIBLE ||
		config_.visibility_ == Water::INSIDE_ONLY;
}


/**
 *	Returns true if the water should be visible outside shells.
 */
bool EditorChunkWater::visibleOutside() const
{
	return
		config_.visibility_ == Water::ALWAYS_VISIBLE ||
		config_.visibility_ == Water::OUTSIDE_ONLY;
}


/**
 *	This method updates our local vars from the transform
 */
void EditorChunkWater::updateLocalVars( const Matrix & m )
{
	BW_GUARD;

	const Vector3 & woriVec = m.applyToUnitAxisVector(2);
	localOri_ = atan2f( woriVec.x, woriVec.z );
	Matrix unrot; unrot.setRotateY( -localOri_ );
	unrot.preMultiply( m );

	config_.size_.x = unrot.applyToUnitAxisVector(0).length();
	config_.size_.y = unrot.applyToUnitAxisVector(2).length();

	transform_ = m;
	worldPos_ = transform_.applyToOrigin();
}


/**
 *
 */
void EditorChunkWater::dirty()
{
	BW_GUARD;

	ChunkWater::dirty();
	changed_ = true;
	updateWorldVars(Matrix::identity);

	// Make sure at least one of its chunks gets marked as dirty.
	VeryLargeObject::ChunkItemList items = this->chunkItems();
	for (VeryLargeObject::ChunkItemList::iterator it = items.begin();
		it != items.end(); ++it)
	{
		if ((*it)->chunk())
		{
			WorldManager::instance().changedChunk( (*it)->chunk(), false );
		}
	}
}


/**
 *	This method updates our caches of world space variables
 */
void EditorChunkWater::updateWorldVars( const Matrix & m )
{
	config_.position_ = ( worldPos_ );	
	bb_.setBounds( 
		worldPos_ + Vector3( -config_.size_.x*0.5f, 0, -config_.size_.y*0.5f),
		worldPos_ + Vector3(  config_.size_.x*0.5f, 0,  config_.size_.y*0.5f));
	config_.orientation_ = localOri_;
}


/**
 *
 */
void EditorChunkWater::draw( Chunk* pChunk )
{
	BW_GUARD;

	if( !WorldManager::instance().drawSelection() )
		ChunkWater::draw( pChunk );
}


/**
 *	This returns the number of triangles of this water body.
 */
int EditorChunkWater::numTriangles() const
{
	BW_GUARD;

	Water::WaterState tmpConfig = config_;
	ChunkWaterConfig tess( tmpConfig );
	return tess.numIndices() - 2;
}


/**
 *	This returns the number of primitives of this water body.
 */
int EditorChunkWater::numPrimitives() const
{
	BW_GUARD;

	return pWater_ ? pWater_->numCells() : 0;
}



/**
 *	Return a modelptr that is the representation of this chunk item
 */
ModelPtr EditorChunkWater::reprModel() const
{
	return waterModel_;
}

VLOFactory EditorChunkWater::factory_( "water", 1, EditorChunkWater::create );


/**
 *
 */
bool EditorChunkWater::create( Chunk * pChunk, DataSectionPtr pSection, std::string uid )
{
	BW_GUARD;

	//TODO: check it isnt already created?
	EditorChunkWater * pItem = new EditorChunkWater( uid );	

	if (pItem->load( pSection, pChunk ))
		return true;

	//the VLO destructor should clear out the static pointer map
	delete pItem;
	return false;
}


ChunkItemFactory::Result EditorChunkWater::oldCreate( Chunk * pChunk, DataSectionPtr pSection )
{
	BW_GUARD;

	bool converted = pSection->readBool("deprecated",false);
	if (converted)
		return ChunkItemFactory::Result( NULL, "Failed to create legacy water" );

	pSection->writeBool("deprecated", true);
	WorldManager::instance().changedChunk( pChunk );

	//TODO: generalise the want flags...
	EditorChunkVLO * pVLO = new EditorChunkVLO( );	
	if (!pVLO->legacyLoad( pSection, pChunk, std::string("water") ))	
	{
		delete pVLO;
		return ChunkItemFactory::Result( NULL, "Failed to create legacy water" );
	}
	else
	{
		pChunk->addStaticItem( pVLO );
		pVLO->edTransform( pVLO->edTransform(), false );
		return ChunkItemFactory::Result( pVLO );
	}
}

/// Static factory initialiser
ChunkItemFactory EditorChunkWater::oldWaterFactory_( "water", 0, EditorChunkWater::oldCreate );

// editor_chunk_water.cpp
