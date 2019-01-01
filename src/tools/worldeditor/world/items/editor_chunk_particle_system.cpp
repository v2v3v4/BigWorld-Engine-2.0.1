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
#include "worldeditor/world/items/editor_chunk_particle_system.hpp"
#include "worldeditor/world/items/editor_chunk_substance.ipp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/editor/item_editor.hpp"
#include "worldeditor/editor/item_properties.hpp"
#include "worldeditor/misc/options_helper.hpp"
#include "appmgr/options.hpp"
#include "model/super_model.hpp"
#include "romp/geometrics.hpp"
#include "chunk/chunk_model.hpp"
#include "chunk/chunk_manager.hpp"
#include "gizmo/undoredo.hpp"
#include "resmgr/string_provider.hpp"
#include "resmgr/auto_config.hpp"
#include "resmgr/resource_cache.hpp"
#include "particle/meta_particle_system.hpp"
#include <algorithm>

#if UMBRA_ENABLE
#include <umbraModel.hpp>
#include <umbraObject.hpp>
#include "chunk/chunk_umbra.hpp"
#include "chunk/umbra_chunk_item.hpp"
#endif


DECLARE_DEBUG_COMPONENT2( "Editor", 0 )


static AutoConfigString s_notFoundModel( "system/notFoundModel" );	


std::map<std::string, std::set<EditorChunkParticleSystem*> > EditorChunkParticleSystem::editorChunkParticleSystem_;

void EditorChunkParticleSystem::add( EditorChunkParticleSystem* system, const std::string& filename )
{
	BW_GUARD;

	editorChunkParticleSystem_[ filename ].insert( system );
}

void EditorChunkParticleSystem::remove( EditorChunkParticleSystem* system )
{
	BW_GUARD;

	for( std::map<std::string, std::set<EditorChunkParticleSystem*> >::iterator iter =
		editorChunkParticleSystem_.begin(); iter != editorChunkParticleSystem_.end(); ++iter )
	{
		std::set<EditorChunkParticleSystem*>& particleSet = iter->second;
		for( std::set<EditorChunkParticleSystem*>::iterator siter = particleSet.begin();
			siter != particleSet.end(); ++siter )
		{
			if( *siter == system )
			{
				particleSet.erase( siter );
				if( particleSet.size() )
					editorChunkParticleSystem_.erase( iter );
				return;
			}
		}
	}
}

void EditorChunkParticleSystem::reload( const std::string& filename )
{
	BW_GUARD;

	BWResource::instance().purgeAll();
	std::set<EditorChunkParticleSystem*> particleSet =
		editorChunkParticleSystem_[ BWResource::dissolveFilename( filename ) ];
	for( std::set<EditorChunkParticleSystem*>::iterator iter = particleSet.begin();
		iter != particleSet.end(); ++iter )
	{
		Chunk* myChunk = (*iter)->chunk();
		(*iter)->toss( NULL );
		(*iter)->load( BWResource::dissolveFilename( filename ) );
		(*iter)->toss( myChunk );
	}
}
// -----------------------------------------------------------------------------
// Section: EditorChunkParticleSystem
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
EditorChunkParticleSystem::EditorChunkParticleSystem() :
	needsSyncInit_( false )
{
	BW_GUARD;

	psModel_ = Model::get( "resources/models/particle.model" );
	psModelSmall_ = Model::get( "resources/models/particle_small.model" );
	psBadModel_ = Model::get( s_notFoundModel.value() );
	ResourceCache::instance().addResource( psModel_ );
	ResourceCache::instance().addResource( psModelSmall_ );
	ResourceCache::instance().addResource( psBadModel_ );
}


/**
 *	Destructor.
 */
EditorChunkParticleSystem::~EditorChunkParticleSystem()
{
	BW_GUARD;

	remove( this );
}


void EditorChunkParticleSystem::tick( float dTime )
{
	BW_GUARD;

	int renderParticleProxy = OptionsParticleProxies::visible();
	int renderLargeProxy = OptionsParticleProxies::particlesLargeVisible();

	ModelPtr newModel;

	if (renderLargeProxy && renderParticleProxy) 
	{
		newModel = (system_ ? psModel_ : psBadModel_);
	}
	else if (renderParticleProxy)
	{
		newModel = (system_ ? psModelSmall_ : psBadModel_);
	}

	if (currentModel_ != newModel)
	{
		needsSyncInit_ = true;
		if (pChunk_)
		{
			ChunkModelObstacle::instance( *pChunk_ ).delObstacles( this );
		}
		currentModel_ = newModel;
		if (pChunk_)
		{
			this->addAsObstacle();
		}
	}

	EditorChunkSubstance<ChunkParticles>::tick( dTime );

	if (needsSyncInit_)
	{
		#if UMBRA_ENABLE
		delete pUmbraDrawItem_;
		pUmbraDrawItem_ = NULL;

		if (!system_ && currentModel_)
		{
			// Grab the visibility bounding box
			BoundingBox bb = currentModel_->boundingBox();

			// Set up object transforms
			Matrix m = pChunk_->transform();
			m.preMultiply( localTransform_ );

			// Create the umbra chunk item
			UmbraChunkItem* pUmbraChunkItem = new UmbraChunkItem();
			pUmbraChunkItem->init( this, bb, m, pChunk_->getUmbraCell());
			pUmbraDrawItem_ = pUmbraChunkItem;

			this->updateUmbraLenders();
		}
		#endif

		needsSyncInit_ = false;
	}
}


bool EditorChunkParticleSystem::edShouldDraw()
{
	BW_GUARD;

	if( !EditorChunkSubstance<ChunkParticles>::edShouldDraw() )
		return false;

	return OptionsScenery::particlesVisible();
}

void EditorChunkParticleSystem::draw()
{
	BW_GUARD;

	if (!edShouldDraw())
		return;
	
	ModelPtr model = reprModel();

	if (edIsTooDistant() && !WorldManager::instance().drawSelection())
	{
		// Set the proxy model to NULL if we are too far away.
		model = NULL;
	}

	if (WorldManager::instance().drawSelection() && model)
	{
		WorldManager::instance().registerDrawSelectionItem( this );

		// draw a some points near the centre of the reprModel, so the system
		// can be selected from the distance where the repr model might be
		// smaller than a pixel and fail to draw.
		Moo::rc().push();
		Moo::rc().world( chunk()->transform() );
		Moo::rc().preMultiply( edTransform() );
		// bias of half the size of the representation model's bounding box in
		// the vertical axis, because the object might be snapped to terrain
		// or another object, so the centre might be below something else.
		float bias = model->boundingBox().width() / 2.0f;
		Vector3 points[3];
		points[0] = Vector3( 0.0f, -bias, 0.0f );
		points[1] = Vector3( 0.0f, 0.0f, 0.0f );
		points[2] = Vector3( 0.0f, bias, 0.0f );
		Geometrics::drawPoints( points, 3, 3.0f, (DWORD)this );
		Moo::rc().pop();
	}

	if (model)
	{
		Moo::rc().push();
		Moo::rc().preMultiply( this->edTransform() );
		
		model->dress();	// should really be using a supermodel...
		model->draw( true );
		
		Moo::rc().pop();
	}
	
	if ( system_ == NULL )
		return;
	
	if ( !WorldManager::instance().drawSelection() )
	{
		ChunkParticles::draw();

		ChunkParticles::drawBoundingBoxes( 
			BoundingBox::s_insideOut_, 
			BoundingBox::s_insideOut_, 
			Matrix::identity ); 
	}
}

/**
 *	This method saves the data section pointer before calling its
 *	base class's load method
 */
bool EditorChunkParticleSystem::load( DataSectionPtr pSection, Chunk* chunk, std::string* errorString )
{
	BW_GUARD;

	remove( this );
	bool ok = this->EditorChunkSubstance<ChunkParticles>::load( pSection );
	resourceName_ = pSection->readString( "resource" );
	add( this, resourceName_ );
	if ( !ok )
	{
		pOriginalSect_ = pSection;
		WorldManager::instance().addError(
			chunk, this, "Couldn't load particle system: %s",
			pSection->readString( "resource" ).c_str() );
	}

	return true;
}


/**
 *	Save any property changes to this data section
 */
bool EditorChunkParticleSystem::edSave( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!edCommonSave( pSection ))
		return false;

	if ( system_ == NULL )
	{
		// the particle system didn't load, so just save the original section
		// but set the appropriate transform.
		pSection->copy( pOriginalSect_ );
		pSection->writeMatrix34( "transform", localTransform_ );
		return true;
	}

	pSection->writeString( "resource", resourceName_ );
	pSection->writeMatrix34( "transform", localTransform_ );
	pSection->writeBool( "reflectionVisible", getReflectionVis() );

	return true;
}


/**
 *	Get the current transform
 */
const Matrix & EditorChunkParticleSystem::edTransform()
{
	return localTransform_;
}


/**
 *	Change our transform, temporarily or permanently
 */
bool EditorChunkParticleSystem::edTransform( const Matrix & m, bool transient )
{
	BW_GUARD;

	// it's permanent, so find out where we belong now
	Chunk * pOldChunk = pChunk_;
	Chunk * pNewChunk = this->edDropChunk( m.applyToOrigin() );
	if (pNewChunk == NULL) return false;

	// if this is only a temporary change, keep it in the same chunk
	if (transient)
	{
		// move the system
		localTransform_ = m;
		Matrix world;
		world.multiply( m, chunk()->transform() );
		setMatrix( world );

		// move the bounding boxes too
		if ( system_ != NULL )
			system_->clear();
		this->syncInit();
		return true;
	}

	// make sure the chunks aren't readonly
	if (!EditorChunkCache::instance( *pOldChunk ).edIsWriteable() 
		|| !EditorChunkCache::instance( *pNewChunk ).edIsWriteable())
		return false;
	
	// clear to reset bounding boxes (otherwise will be stretched during undo/redo)
	if ( system_ != NULL )
		system_->clear();
	
	// ok, accept the transform change then
	localTransform_.multiply( m, pOldChunk->transform() );
	localTransform_.postMultiply( pNewChunk->transformInverse() );

	// note that both affected chunks have seen changes
	WorldManager::instance().changedChunk( pOldChunk );
	WorldManager::instance().changedChunk( pNewChunk );

	edMove( pOldChunk, pNewChunk );

	// check to see if we are undoing or redoing so we will reset the source action
	if ( system_ != NULL && UndoRedo::instance().isUndoing() )
		system_->setFirstUpdate();
	this->syncInit();
	return true;
}

/**
 *	Add the properties of this flare to the given editor
 */
bool EditorChunkParticleSystem::edEdit( class GeneralEditor & editor )
{
	BW_GUARD;

	if (this->edFrozen())
		return false;

	if (!edCommonEdit( editor ))
	{
		return false;
	}

	MatrixProxy * pMP = new ChunkItemMatrix( this );
	editor.addProperty( new ChunkItemPositionProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_PARTICLE/POSITION"), pMP, this ) );
	editor.addProperty( new GenRotationProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_PARTICLE/ROTATION"), pMP ) );
	editor.addProperty( new StaticTextProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_PARTICLE/PARTICLE_NAME"),
		new ConstantDataProxy<StringProxy>(
		resourceName_ ) ) );
	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_PARTICLE/REFLECTION_VISIBLE"),
			new AccessorDataProxy< EditorChunkParticleSystem, BoolProxy >(
				this, "reflectionVisible",
				&EditorChunkParticleSystem::getReflectionVis,
				&EditorChunkParticleSystem::setReflectionVis ) ) );

	static std::wstring s_assetMetadataGroup =
		Localise( L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_PARTICLE/ASSET_META_DATA" );

	if (system_ != NULL)
	{
		editor.addingAssetMetadata( true );
		system_->metaData().edit( editor, s_assetMetadataGroup, true );
		editor.addingAssetMetadata( false );
	}

	return true;
}

std::vector<std::string> EditorChunkParticleSystem::edCommand( const std::string& path ) const
{
	BW_GUARD;

	if ( system_ == NULL )
		return std::vector<std::string>();

	std::vector<std::string> commands;
	commands.push_back( LocaliseUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_PARTICLE/EDIT_IN_PARTICLE_EDITOR") );
	return commands;
}

bool EditorChunkParticleSystem::edExecuteCommand( const std::string& path, std::vector<std::string>::size_type index )
{
	BW_GUARD;

	if ( system_ == NULL )
		return true;

	if( path.empty() && index == 0 )
	{
		std::string commandLine = "-o ";
		commandLine += '\"' + BWResource::resolveFilename( resourceName_ ) + '\"';
		std::replace( commandLine.begin(), commandLine.end(), '/', '\\' );
		commandLine += ' ';
		commandLine += BWResource::getPathAsCommandLine( true );

		PyObject * function = NULL;
		PyObject* pModule = PyImport_ImportModule( "WorldEditor" );
		if (pModule != NULL)
		{
			function = PyObject_GetAttrString( pModule, "launchTool" );
		}

		return Script::call( function,
							Py_BuildValue( "(ss)", "particleeditor",
											commandLine.c_str() ), 
							"WorldEditor" );
	}
	return false;
}

void EditorChunkParticleSystem::drawBoundingBoxes( const BoundingBox &bb, const BoundingBox &vbb, const Matrix &spaceTrans ) const
{
	BW_GUARD;

	if ( system_ == NULL )
		return;

	if (!WorldManager::instance().drawSelection())
		EditorChunkSubstance<ChunkParticles>::drawBoundingBoxes( bb, vbb, spaceTrans );
}


bool EditorChunkParticleSystem::addYBounds( BoundingBox& bb ) const
{
	BW_GUARD;
	ChunkParticles::addYBounds( bb );
	return true;
}


/**
 *	Return a modelptr that is the representation of this chunk item
 */
ModelPtr EditorChunkParticleSystem::reprModel() const
{
	return currentModel_;
}


/// Write the factory statics stuff
#undef IMPLEMENT_CHUNK_ITEM_ARGS
#define IMPLEMENT_CHUNK_ITEM_ARGS (pSection, pChunk, &errorString)
IMPLEMENT_CHUNK_ITEM( EditorChunkParticleSystem, particles, 1 )
