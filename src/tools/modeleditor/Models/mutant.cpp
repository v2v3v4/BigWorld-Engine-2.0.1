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
#include "mutant.hpp"
#include "me_consts.hpp"

#include "moo/managed_texture.hpp"
#include "moo/software_skinner.hpp"
#include "moo/vertices_manager.hpp"
#include "moo/visual_channels.hpp"
#include "resmgr/string_provider.hpp"
#include "resmgr/xml_section.hpp"
#include "romp/geometrics.hpp"
#include "model/super_model.hpp"
#include "model/super_model_animation.hpp"
#include "model/super_model_dye.hpp"
#include "model/tint.hpp"
#include "moo/resource_load_context.hpp"
#include "undo_redo.hpp"
#include "utilities.hpp"
#include "visual_bumper.hpp"

DECLARE_DEBUG_COMPONENT( 0 )
#include "me_error_macros.hpp"

Mutant::Mutant( bool groundModel, bool centreModel ):
	modelName_ (""),
	visualName_ (""),
	primitivesName_ (""),
	editorProxyName_ (""),
	superModel_ ( NULL ),
	editorProxySuperModel_ ( NULL ),
	matrixLI_( NULL ),
	playing_ (true),
	looping_ (true),
	virtualDist_ (-1.f),
	groundModel_(groundModel),
	centreModel_(centreModel),
	numNodes_(0),
	normalsLength_(50),
	foundRootNode_( false ),
	hasRootNode_( false ),
	initialRootNode_( 0.f, 0.f, 0.f ),
	animMode_( true ),
	clearAnim_( false ),
	visibilityBoxDirty_( false ),
	texMemDirty_( true ),
	texMem_( 0 ),
	modelBeenBumped_( false ),
	modifiedTime_( 0 ),
	isReadOnly_( false ),
	isSkyBox_( false ),
	pPortalMat_( NULL ),
	metaData_( NULL )
{
	BW_GUARD;

	extraModels_ = BWResource::openSection( "extraModels.xml", true );
}

Mutant::~Mutant()
{
	BW_GUARD;

	delete superModel_;
}

void Mutant::registerModelChangeCallback( SmartPointer <ModelChangeCallback> mcc )
{
	BW_GUARD;

	modelChangeCallbacks_.push_back( mcc );
}

void Mutant::unregisterModelChangeCallback( void* parent )
{
	BW_GUARD;

	ModelChangeCallbackVect::iterator it = modelChangeCallbacks_.begin();
	while (it != modelChangeCallbacks_.end())
	{
		if (parent == (*it)->parent())
		{
			it = modelChangeCallbacks_.erase( it );
		}
		else
		{
			++it;
		}
	}
}

bool Mutant::executeModelChangeCallbacks()
{
	BW_GUARD;

	bool ok = true;
	for (unsigned i=0; i<modelChangeCallbacks_.size(); i++)
	{
		ok = modelChangeCallbacks_[i]->execute() && ok;
	}
	return ok;
}

void Mutant::groundModel( bool lock )
{
	groundModel_ = lock;
}

void Mutant::centreModel( bool lock )
{
	centreModel_ = lock;
}

bool Mutant::hasVisibilityBox( std::string modelPath /* = "" */ )
{
	BW_GUARD;

	if (modelPath == "")
		modelPath = modelName_;

	if (modelPath == "") return 0;

	DataSectionPtr model = BWResource::openSection( modelPath, false );

	return ((model) && (model->openSection("visibilityBox/min")) &&
		(model->openSection("visibilityBox/max")));
}

int Mutant::animCount( std::string modelPath /* = "" */ )
{
	BW_GUARD;

	if (modelPath == "")
		modelPath = modelName_;

	if (modelPath == "") return 0;

	int animCount = 0;
	do
	{
		DataSectionPtr model = BWResource::openSection( modelPath, false );

		if (!model) break;
		
		std::vector< DataSectionPtr > pAnimations;
		model->openSections( "animation", pAnimations );
		animCount += pAnimations.size();
		modelPath = model->readString("parent","") + ".model";
	}
	while (modelPath != ".model");
	return animCount;
}

bool Mutant::nodefull( std::string modelPath /* = "" */ )
{
	BW_GUARD;

	if (modelPath == "")
		modelPath = modelName_;

	if (modelPath == "") return false;

	DataSectionPtr model = BWResource::openSection( modelPath, false );

	std::string visualPath = model->readString( "nodefullVisual", "" );

	return (visualPath != "");
}


/**
 *	This method returns the number of BlendBone nodes the current Visual has.
 *
 *	@return The number of BlendBone nodes.
 */
size_t Mutant::blendBoneCount() const
{
	BW_GUARD;

	std::set< std::string > blendBonesSet;
	
	std::vector< DataSectionPtr > renderSets;
	std::vector< DataSectionPtr >::iterator renderSetsIt;
	
	if (currVisual_)
	{
		currVisual_->openSections( "renderSet", renderSets );
	}
	else
	{
		return 0;
	}

	for (
		renderSetsIt = renderSets.begin();
		renderSetsIt != renderSets.end();
		++renderSetsIt
		)
	{
		std::vector< std::string > blendBonesVector;
		std::vector< std::string >::iterator blendBonesVectorIt;

		(*renderSetsIt)->readStrings( "node", blendBonesVector, DS_TrimWhitespace );

		for (
			blendBonesVectorIt = blendBonesVector.begin();
			blendBonesVectorIt != blendBonesVector.end();
			++blendBonesVectorIt
			)
		{
			blendBonesSet.insert( *blendBonesVectorIt );
		}
	}

	return blendBonesSet.size();
}


/**
 *	This method checks to see if the model can be batch rendered.
 *	Models with tints or with alpha blending cannot be batched.
 *
 *	@return True if the model can be batched, false otherwise.
 */
bool Mutant::canBatch() const
{
	BW_GUARD;

	// If the model is tinted then we cannot batch
	if (tints_.size() > 0)
	{
		return false;
	}

	// If the model is alpha blended then we cannot batch
	std::map< std::string, MaterialInfo >::const_iterator matIt = materials_.begin();
	std::map< std::string, MaterialInfo >::const_iterator matEnd = materials_.end();
	for (; matIt != matEnd; ++matIt)
	{
		EffectMaterialSet::const_iterator sectIt = matIt->second.effect.begin();
		EffectMaterialSet::const_iterator sectEnd = matIt->second.effect.end();
		for (; sectIt != sectEnd; ++sectIt)
		{
			Moo::EffectMaterialPtr effect = *sectIt;

			if ((effect != NULL) && (effect->channel() != NULL))
			{
				return false;
			}
		}
	}

	return true;
}


/**
 *	This method sets whether the model can be batched.
 *
 *	@param val A value specifying whether to batch.
 */
void Mutant::batched( bool val, bool undo /*=true*/ )
{
	BW_GUARD;

	if (!currModel_) 
	{
		return;
	}

	DataSectionPtr data = currModel_->openSection( "batched" );
	if (data == NULL) 
	{
		data = currModel_;
	}

	if (undo)
	{
		UndoRedo::instance().add( new UndoRedoOp( 0, data, currModel_ ));
		UndoRedo::instance().barrier( LocaliseUTF8(L"MODELEDITOR/MODELS/MUTANT/CHANGING_BATCH"), false );
	}

	currModel_->writeBool( "batched", val );
}


/**
 *	This method returns the batch parameter for the model.
 *
 *	@return True if the model is batched, false otherwise.
 */
bool Mutant::batched() const
{
	BW_GUARD;

	if (!currModel_) 
	{
		return false;
	}

	return currModel_->readBool( "batched", false );
}


void Mutant::dpvsOccluder( bool val )
{
	BW_GUARD;

	if (!currModel_) return;
	DataSectionPtr data = currModel_->openSection( "dpvsOccluder" );
	if (data == NULL) data = currModel_;
	UndoRedo::instance().add( new UndoRedoOp( 0, data, currModel_ ));
	UndoRedo::instance().barrier( LocaliseUTF8(L"MODELEDITOR/MODELS/MUTANT/CHANGING_OCCLUDER"), false );
	currModel_->writeBool( "dpvsOccluder", val );
}

bool Mutant::dpvsOccluder()
{
	BW_GUARD;

	if (!currModel_) return false;
	return currModel_->readBool( "dpvsOccluder", false );
}

std::string Mutant::editorProxyName()
{
	return editorProxyName_;
}

void Mutant::editorProxyName( std::string editorProxyName )
{
	BW_GUARD;

	if (!currModel_) return;
	DataSectionPtr data = currModel_->openSection( "editorModel" );
	if (data == NULL) data = currModel_;
	UndoRedo::instance().add( new UndoRedoOp( 0, data, currModel_ ));
	UndoRedo::instance().barrier( LocaliseUTF8(L"MODELEDITOR/MODELS/MUTANT/CHANGING_EDITOR_PROXY"), false );
	currModel_->writeString( "editorModel", editorProxyName );

	delete editorProxySuperModel_;
	editorProxySuperModel_ = NULL;
	std::vector< std::string > editorProxyVect;
	editorProxyVect.push_back( editorProxyName );
	editorProxySuperModel_ = new SuperModel( editorProxyVect );
	if ((editorProxySuperModel_ == NULL) || (!editorProxySuperModel_->isOK()))
	{
		ERROR_MSG( "An error occured while trying to load \"%s\".\n"
			"This model may need to be re-exported.\n", editorProxyName_.c_str() );
		delete editorProxySuperModel_;
		editorProxySuperModel_ = NULL;
		editorProxyName_ = "";
	}
	else
	{
		editorProxyName_ = editorProxyName;
	}
}

void Mutant::removeEditorProxy()
{
	BW_GUARD;

	if (!currModel_) return;
	DataSectionPtr data = currModel_->openSection( "editorModel" );
	if (data == NULL) return;
	UndoRedo::instance().add( new UndoRedoOp( 0, currModel_, currModel_ ));
	UndoRedo::instance().barrier( LocaliseUTF8(L"MODELEDITOR/MODELS/MUTANT/REMOVING_EDITOR_PROXY"), false );
	currModel_->delChild( data );

	delete editorProxySuperModel_;
	editorProxySuperModel_ = NULL;
	editorProxyName_ = "";
}

/*
 *	This bumps the mutant's visual.
 */
void Mutant::bumpMutant( DataSectionPtr visual, const std::string& primitivesName )
{
	BW_GUARD;

	if ( visual && !primitivesName.empty() )
	{
		// create a backup of the original primitive file
		std::string fullPath = BWResource::resolveFilename( primitivesName );
		std::rename( fullPath.c_str(), std::string(fullPath+".backup").c_str() );
		std::wstring wfullPath = bw_utf8tow( fullPath );
		CopyFile( bw_utf8tow(fullPath+".backup").c_str(), wfullPath.c_str(), FALSE );

		DWORD att = GetFileAttributes( wfullPath.c_str() );
		att &= ~FILE_ATTRIBUTE_READONLY;
		SetFileAttributes( wfullPath.c_str(), att );

		// initiate the visual bumper
		VisualBumper vb( visual, primitivesName );
		modelBeenBumped_ = vb.bumpVisual();

		modifiedTime_ = BWResource::modifiedTime( fullPath );
	}
	else
	{
		modelBeenBumped_ = false;
	}
}

/*
 * This reverts the model to it's on disk state.
 */
void Mutant::revertModel()
{
	BW_GUARD;

	loadModel( modelName_ );
}

/*
 * This reloads the entire model including the trees, animations and actions.
 */
void Mutant::reloadModel()
{
	BW_GUARD;

	//Clear the fashion to ensure the default stance is used
	fashions_.clear();
	drawModel( false, -1, false );

	//Reload the model
	loadModel( modelName_, true );
}

bool Mutant::loadModel( const std::string& name, bool reload )
{
	BW_GUARD;

	if (name == "")
	{
		return false;
	}
	
	Moo::ScopedResourceLoadContext resLoadCtx( BWResource::getFilename( name ) );

	executeModelChangeCallbacks();
	
	//Backup the added models list in case the load fails
	static bool s_last_failed = false;
	static std::set< std::string > s_extra_models;
	std::vector< DataSectionPtr > extras;
	extraModels_->openSections( "model", extras );
	for (unsigned i=0; i<extras.size(); i++)
	{
		s_extra_models.insert( extras[i]->asString() );
	}
	
	// unfortunately we need to have this string here (otherwise primitive files can go missing)
	std::string primitivesName;
	bool isReadOnly = false;
	if (!reload)
	{
		//Clear the list of missing files
		Mutant::clearFilesMissingList();
		
		//Purge all datasection caches to ensure the new model is loaded
		BWResource::instance().purgeAll();
		
		DataSectionPtr visual;
		//Check the model's validity before doing anything else...
		bool loadingInvalid = false;
		if (!ensureModelValid( name, "load", NULL, &visual, NULL, &primitivesName, &isReadOnly ))
		{
			if ( !currVisual_ && primitivesName_.empty() ) return false;
			visual = currVisual_;
			primitivesName = primitivesName_;
			loadingInvalid = true;
		}
		
		// Bump the mutant
		bumpMutant( visual, primitivesName );
		
		// Now if we tried to load a bad model we want to restore the previous model's
		// bumped primitives file.
		if ( loadingInvalid ) return false;
		
		//Clear all reported Model::get messages
		Model::clearReportedErrors();
		
		//We need to delete the old supermodel to ensure that the
		//version on disk is loaded
		delete superModel_;
		superModel_ = NULL;
		
		// remove the added models
		extraModels_->delChildren();
		
		UndoRedo::instance().clear();
		UndoRedo::instance().setSavePoint();
	}

	// Save the state of the action queue incase we are reloading
	XMLSectionPtr savedActionQueue;
	if ( reload )
	{
		savedActionQueue = new XMLSection("old_state");
		actionQueue_.saveActionState( savedActionQueue );
	}
	else
	{
		actionQueue_.debugTick( -1.f );
	}
	// Make sure to always clear the action queue (load and reload)
	actionQueue_.flush();

	if (reload)
	{
		
		//Reload all the models in the lod tree
		std::map < std::string , DataSectionPtr >::iterator it = models_.begin();
		std::map < std::string , DataSectionPtr >::iterator end = models_.end();
		
		for (;it!=end;++it)
		{
			ModelPtr original = Model::get( it->first );

			if ( original )
			{
				original->reload( it->second , false );
			}
		}
	}

	std::vector< std::string > models;
	
	models.push_back( name );
	
	//If the previous load failed re-add the old added models.
	if (s_last_failed)
	{
		std::set< std::string >::iterator it = s_extra_models.begin();
		std::set< std::string >::iterator end = s_extra_models.end();
		for(;it != end;++it)
		{
			addModel( *it, false );
		}
	}
	
	if (extraModels_->countChildren())
	{
		bool alreadyIn;
		extraModels_->openSections( "model", extras );
		for (unsigned i=0; i<extras.size(); i++)
		{
			alreadyIn = false;
			for (unsigned j=0; j<models.size(); j++)
			{
				if (extras[i]->asString() == models[j])
				{
					alreadyIn = true;
					break;
				}
			}
			if (!alreadyIn)
			{
				models.push_back( extras[i]->asString() );
			}
		}
	}
	
	//Moo::ManagedTexture::accErrs( true );

	SuperModel* newSuperModel = new SuperModel( models );
	if ((newSuperModel == NULL) || (!newSuperModel->isOK()))
	{
		//Moo::ManagedTexture::accErrs( false );

		ERROR_MSG( "An error occured while trying to load \"%s\".\n"
			"This model may need to be re-exported.\n", name.c_str() );
		
		// Clean up the primitive files
		std::string backupPrimitivesName = primitivesName_;
		primitivesName_ = primitivesName;
		saveCorrectPrimitiveFile( false );
		primitivesName_ = backupPrimitivesName;
		bumpMutant( currVisual_, primitivesName_ );
		
		delete newSuperModel;
		s_last_failed = true;
		loadModel( modelName_ );
		return false;
	}
	
	s_last_failed = false;
	
	//Clear the added models list now that we have loaded successfully
	s_extra_models.clear();

	delete superModel_; // OK, lets nuke the old supermodel now...
	superModel_ = newSuperModel;
	
	if (!reload)
	{
		isReadOnly_ = isReadOnly;

		//Now get the file datasections for the model
		ensureModelValid( name, "load", &currModel_, &currVisual_, &visualName_, &primitivesName_ );

		modelHist_.erase( modelHist_.begin(), modelHist_.end() ); // Clear the model history
		
		verts_.resize( 0 );

		animations_.clear();
		currDyes_.clear();
		currAnims_.clear();
		actionQueue_.flush();

		foundRootNode_ = false;
		hasRootNode_ = false;
		initialRootNode_ = Vector3( 0.f, 0.f, 0.f );

		modelBB_ =  BoundingBox::s_insideOut_;
		visibilityBB_ = BoundingBox::s_insideOut_;
		frameBB_ = BoundingBox::s_insideOut_;

		visibilityBoxDirty_ = false;
	}

	//Let's get the model's bounding box
	//Note that we are getting the bounding box from the super model
	//directly since the bounding box may have been altered due to
	//an externally loaded BSP
	superModel_->localBoundingBox( modelBB_ );
	Vector3 minBB = modelBB_.minBounds();
	Vector3 maxBB = modelBB_.maxBounds();

	//Now let's get the visibility box
	if (currModel_->openSection("visibilityBox/min") &&
		currModel_->openSection("visibilityBox/max"))
	{
		minBB = currModel_->readVector3( "visibilityBox/min", Vector3( 0.f, 0.f, 0.f ) );
		maxBB = currModel_->readVector3( "visibilityBox/max", Vector3( 0.f, 0.f, 0.f ) );
		visibilityBB_ = BoundingBox( minBB, maxBB );
	}
	else
	{
		visibilityBB_ = modelBB_;
		visibilityBoxDirty_ = true; // The visibility box will require a recalculation
	}

	frameBB_ = modelBB_; // Make sure it is valid initialy.

	//We always want the model draw even if it is not visible
	superModel_->checkBB( false ); 

	modelName_ = name;

	//Load the editor proxy model if it has changed
	std::string newEditorProxyName = currModel_->readString( "editorModel", "" );
	if (newEditorProxyName != editorProxyName_)
	{
		delete editorProxySuperModel_;
		editorProxySuperModel_ = NULL;
		editorProxyName_ = newEditorProxyName;
		if (editorProxyName_ != "")
		{	
			std::vector< std::string > editorProxyVect;
			editorProxyVect.push_back( editorProxyName_ );
			editorProxySuperModel_ = new SuperModel( editorProxyVect );
			if ((editorProxySuperModel_ == NULL) || (!editorProxySuperModel_->isOK()))
			{
				ERROR_MSG( "An error occured while trying to load \"%s\".\n"
					"This model may need to be re-exported.\n", editorProxyName_.c_str() );
				delete editorProxySuperModel_;
				editorProxySuperModel_ = NULL;
				editorProxyName_ = "";
			}
		}
	}

	regenAnimLists();
	regenMaterialsList();
	recalcTextureMemUsage();
	calcCustomHull();

	// Here we check for special cases in the materials, for example, to find
	// if a shader is a skybox shader, so we render it appropriately.
	checkMaterials();

	if ( reload )
		actionQueue_.restoreActionState( savedActionQueue, superModel_, matrixLI_ );

	if (!reload)
	{
		reloadBSP();
		
		triggerUpdate("Anim");
		triggerUpdate("Act");
		triggerUpdate("LOD");
		triggerUpdate("Object");
		triggerUpdate("Materials");

		if (superModel_ && superModel_->nModels())
		{
			metaData_ = superModel_->topModel( 0 )->metaData();
		}
	}

	this->postLoad();

	return true;
}


/**
 *	This method is used to clean up the model after a load and also perform any
 *	post-load functions such as recreating the supermodel's functions.
 *
 *	TODO: Move more post load stuff out of loadModel.
 *	TODO: once supermodel no longer needs to reload after every change, change 
 *		  this method to syncModel().
 */
void Mutant::postLoad()
{
	BW_GUARD;

	// Fix the batching flag for the model
	// i.e. do not batch if model has tints or effects with channels
	if (!this->canBatch() && this->batched())
	{
		WARNING_MSG( "Turning off batch rendering as the model has tints "
					 "and/or effect materials which contain channels.\n" );
		this->batched( false, false );
	}

	this->recreateFashions();
}


bool Mutant::addModel( const std::string& name, bool reload /* = true */ )
{
	BW_GUARD;

	if (!ensureModelValid( name, "add" )) return false;
	
	if (name == modelName_) return false;
	
	std::vector< DataSectionPtr > extras;
	extraModels_->openSections( "model", extras );
	
	for (unsigned i=0; i<extras.size(); i++)
	{
		if (name == extras[i]->asString()) return false;
	}

	UndoRedo::instance().add( new UndoRedoOp( 0, extraModels_ ));
	UndoRedo::instance().barrier( LocaliseUTF8(L"MODELEDITOR/MODELS/MUTANT/ADDING_MODEL"), false );

	extraModels_->newSection( "model" )->setString( name );

	if (reload)
	{
		reloadAllLists();
	}

	return true;
}

bool Mutant::hasAddedModels()
{
	BW_GUARD;

	return (extraModels_->countChildren() > 0);
}

void Mutant::removeAddedModels()
{
	BW_GUARD;

	UndoRedo::instance().add( new UndoRedoOp( 0, extraModels_ ));
	UndoRedo::instance().barrier( LocaliseUTF8(L"MODELEDITOR/MODELS/MUTANT/REMOVING_ADDED"), false );
	
	extraModels_->delChildren();
	reloadModel();
}

void Mutant::buildNodeList( DataSectionPtr data, std::set< std::string >& nodes )
{
	BW_GUARD;

	std::vector< DataSectionPtr > nodeData;
	data->openSections( "node", nodeData );
	for (unsigned i=0; i<nodeData.size(); i++)
	{
		nodes.insert( nodeData[i]->readString( "identifier", "" ));
		buildNodeList( nodeData[i], nodes );
	}
}

bool Mutant::canAddModel( std::string modelPath )
{
	BW_GUARD;

	std::set< std::string > newNodeNames;
	std::set< std::string > oldNodeNames;

	DataSectionPtr visual;
	ensureModelValid( modelPath, "read", NULL, &visual );
	buildNodeList( visual, newNodeNames );

	buildNodeList( currVisual_, oldNodeNames );

	std::set< std::string >::iterator nodeIt = newNodeNames.begin();
	std::set< std::string >::iterator nodeEnd = newNodeNames.end();

	for( ; nodeIt != nodeEnd; ++nodeIt )
	{
		if (oldNodeNames.find( *nodeIt ) != oldNodeNames.end())
		{
			if ((*nodeIt != "Scene Root") &&
				(nodeIt->find("HP_") == std::string::npos))
					return true;
		}
	}

	return false;
}

void Mutant::reloadAllLists()
{
	BW_GUARD;

	if (!superModel_) return;

	reloadModel();
	triggerUpdate("Anim");
	triggerUpdate("Act");
	triggerUpdate("LOD");
	triggerUpdate("Object");
	triggerUpdate("Materials");
}

void Mutant::regenAnimLists()
{
	BW_GUARD;

	if (!superModel_) return;

	static int s_animCounter = 0;

	// Make a backup of the  model datasections and animation data
	// and clear the current maps.
	
	std::map< std::string, DataSectionPtr> oldModels;
	std::swap( oldModels, models_ );
	
	std::map< StringPair, AnimationInfo > oldAnims;
	std::swap( oldAnims, animations_ );
	
	actions_.clear();
	dataFiles_.clear();
	animList_.clear();
	actList_.clear();
	lodList_.clear();
	
	std::string modelPath = modelName_;

	DataSectionPtr model = currModel_;

	dataFiles_[currVisual_] = visualName_;

	std::string modelName;
	
	// This code gets data sections of all the models, animations and actions
	// of the supermodel and stores them in maps for later reference.
	do
	{
		if (!model) break;

		std::string::size_type first = modelPath.rfind("/") + 1;
		std::string::size_type last = modelPath.rfind(".");
		modelName = modelPath.substr( first, last-first );
		
		models_[modelPath] = model;
		dataFiles_[model] = modelPath;

		//Animations
		std::vector< std::string > animNames;
		std::vector< DataSectionPtr > pAnimations;
		model->openSections( "animation", pAnimations );
		std::vector< DataSectionPtr >::iterator animIt = pAnimations.begin();
		std::vector< DataSectionPtr >::iterator animEnd = pAnimations.end();
		while (animIt != animEnd)
		{
			DataSectionPtr pData = *animIt++;
			std::string animName = pData->readString( "name", "" );

			if (animName != "")
			{
 				std::map< std::string, float > boneWeights;
				DataSectionPtr pAlpha = pData->openSection( "alpha");
				if (pAlpha)
				{
					DataSectionIterator alphaIt = pAlpha->begin();
					DataSectionIterator alphaEnd = pAlpha->end();
					while (alphaIt != alphaEnd)
					{
						DataSectionPtr bone = *alphaIt++;
						std::string boneName = SanitiseHelper::substringReplace
							(
								bone->sectionName(),
								SanitiseHelper::SANITISING_TOKEN,
								SanitiseHelper::SPACE_TOKEN
							);
						float boneVal = bone->asFloat(0.f);
						boneWeights[ boneName ] = boneVal;
					}
				}

				StringPair animID( animName, modelPath );
				
				DataSectionPtr frameRates = NULL;
				ChannelsInfoPtr channels = NULL;
				float animTime = -1.f;
				
				if (oldAnims.find( animID ) != oldAnims.end())
				{
					//Get the initial framerate
					frameRates = oldAnims[ animID ].frameRates;
					
					//Get the current channel settings
					channels = oldAnims[ animID ].channels;

					//Get the current animation time
					animTime = oldAnims[ animID ].animation->time;
				}
				else
				{
					std::string animPath = pData->readString("nodes","");

					if ( animPath != "" )
					{
						std::map< StringPair, AnimationInfo >::iterator oldAnimIt = oldAnims.begin();
						std::map< StringPair, AnimationInfo >::iterator oldAnimEnd = oldAnims.end();
						for ( ; oldAnimIt != oldAnimEnd ; oldAnimIt++ )
						{
							std::string oldAnimPath = oldAnimIt->second.data->readString("nodes", "");
							if ( animPath == oldAnimPath )
								channels = oldAnimIt->second.channels;
						}
					}
				}

				if (frameRates == NULL)
				{
					char buf[256];
					bw_snprintf( buf, sizeof(buf), "frameRates%d.xml", s_animCounter++ );
					frameRates = BWResource::openSection( std::string( buf ), true );
					frameRates->writeFloat( "curr", pData->readFloat( "frameRate", 30.f ));
					frameRates->writeFloat( "init", pData->readFloat( "frameRate", 30.f ));
				}

				//Lets get the animation from the supermodel
				SuperModelAnimationPtr animation = superModel_->getAnimation( animName );
				if (animation)
				{
					animations_[ animID ] = AnimationInfo( pData, model, animation, boneWeights, frameRates, channels, animTime );
					animNames.push_back( animName );
				}
			}
		}
		
		animList_.push_back( TreeBranch( StringPair( modelName, modelPath ), animNames) );

		//Actions
		std::vector< std::string > actNames;
		std::vector< DataSectionPtr >	pActions;
		model->openSections( "action", pActions );
		std::vector< DataSectionPtr >::iterator actIt = pActions.begin();
		std::vector< DataSectionPtr >::iterator actEnd = pActions.end();
		while (actIt != actEnd)
		{
			DataSectionPtr data = *actIt++;
			std::string actName = data->readString("name","");
			if (actName != "")
			{
				actions_[ StringPair(actName,modelPath) ] = ActionInfo( data, model );
				actNames.push_back( actName );
			}
		}
		actList_.push_back( TreeBranch( StringPair( modelName, modelPath ), actNames) );

		float dist = model->readFloat( "extent", LOD_HIDDEN );

		//The LOD list
		lodList_.push_back( LODEntry( StringPair( modelName, modelPath ), model->readFloat( "extent", LOD_HIDDEN )));

		//Move to the next LOD
		modelPath = model->readString("parent","") + ".model";
		if (modelPath != ".model")
		{
			//Make sure we use the same model DataSectionPtr as previously used if it exists.
			if (modelHist_.find( modelPath ) != modelHist_.end())
			{
				model = modelHist_[ modelPath ];
			}
			else // Otherwise load it up...
			{
				model = BWResource::openSection( modelPath );
			}
		}

	}
	while (modelPath != ".model");

	//The LOD list
	if (modelPath != ".model")
	{
		//This is a special case where the model cannot be loaded...
		modelName = BWResource::getFilename( modelPath );
		modelName = BWResource::removeExtension( modelName );
		
		lodList_.push_back( LODEntry( StringPair( modelName, modelPath ), LOD_MISSING ));
	}
}

void Mutant::regenMaterialsList()
{
	BW_GUARD;

	if (!superModel_)
		return;

	materials_.erase(materials_.begin(),materials_.end());
	dyes_.erase(dyes_.begin(),dyes_.end());
	tints_.erase(tints_.begin(),tints_.end());

	tintedMaterials_.erase( tintedMaterials_.begin(), tintedMaterials_.end() );

	materialNameDataToRemove_.clear();
	
	materialList_.clear();
		
	std::vector< DataSectionPtr >	pRenderSets;
	currVisual_->openSections( "renderSet", pRenderSets );
	std::vector< DataSectionPtr >::iterator renderSetsIt = pRenderSets.begin();
	std::vector< DataSectionPtr >::iterator renderSetsEnd = pRenderSets.end();
	while (renderSetsIt != renderSetsEnd)
	{
		DataSectionPtr renderSet = *renderSetsIt++;

		std::vector< DataSectionPtr >	pGeometries;
		renderSet->openSections( "geometry", pGeometries );
		std::vector< DataSectionPtr >::iterator geometriesIt = pGeometries.begin();
		std::vector< DataSectionPtr >::iterator geometriesEnd = pGeometries.end();
		while (geometriesIt != geometriesEnd)
		{
			DataSectionPtr geometries = *geometriesIt++;

			std::string vertGroup = geometries->readString( "vertices", "" );
			if (vertGroup.find("/") == -1)
				vertGroup = primitivesName_ + "/" + vertGroup;

			// work out if this geometry has dual uv's or vertex colours
			bool dualUV = false;
			bool hasColours = false;

			std::vector<std::string> streams;

			geometries->readStrings( "stream", streams );

			for (uint32 i = 0; i < streams.size(); i++)
			{
				std::string streamID = streams[i];
				size_t dotLocation = streamID.find_last_of( '.' );
				if ( dotLocation != std::string::npos)
				{
					streamID = streamID.substr( dotLocation + 1 );
				}
				if (streamID == "uv2")
				{
					dualUV = true;
				}
				if (streamID == "colour")
				{
					hasColours = true;
				}
			}

			std::vector< DataSectionPtr >	pPrimitiveGroups;
			geometries->openSections( "primitiveGroup", pPrimitiveGroups );
			std::vector< DataSectionPtr >::iterator primitiveGroupsIt = pPrimitiveGroups.begin();
			std::vector< DataSectionPtr >::iterator primitiveGroupsEnd = pPrimitiveGroups.end();
			while (primitiveGroupsIt != primitiveGroupsEnd)
			{
				DataSectionPtr primitiveGroups = *primitiveGroupsIt++;

				std::vector< DataSectionPtr >	pMaterials;
				primitiveGroups->openSections( "material", pMaterials );
				std::vector< DataSectionPtr >::iterator materialsIt = pMaterials.begin();
				std::vector< DataSectionPtr >::iterator materialsEnd = pMaterials.end();
				while (materialsIt != materialsEnd)
				{
					DataSectionPtr materials = *materialsIt++;

					std::string identifier = materials->readString("identifier","");

					materials_[identifier].name = identifier;
					
					materials_[identifier].data.push_back( materials );

					materials_[identifier].format = Moo::VerticesManager::instance()->get( vertGroup )->format();

					materials_[identifier].dualUV = materials_[identifier].dualUV || dualUV;

					materials_[identifier].colours = materials_[identifier].colours || dualUV;
						
					std::vector< Moo::Visual::PrimitiveGroup * > primGroups;
					
					int numMats = superModel_->topModel(0)->gatherMaterials( identifier, primGroups );
					for (int i=0; i<numMats; i++)
					{
						materials_[identifier].effect.insert( primGroups[i]->material_ );
					}

					int found = -1;
					for (unsigned i=0; i<materialList_.size(); i++)
					{
						if (materialList_[i].first.first == identifier)
						{
							found = i;
							break;
						}
					}

					if ( found == -1 )
					{
						std::vector< std::string > empty;
						materialList_.push_back( TreeBranch( StringPair( identifier, "" ), empty ));
					}
				}
			}
		}
	}
	
	// Read in the material name aliases
	DataSectionPtr data = currModel_->openSection( "materialNames", true );
	std::vector< DataSectionPtr >	pNames;
	data->openSections( "material", pNames );
	std::vector< DataSectionPtr >::iterator namesIt = pNames.begin();
	std::vector< DataSectionPtr >::iterator namesEnd = pNames.end();
	while (namesIt != namesEnd)
	{
		DataSectionPtr materialNameData = *namesIt++;

		std::string original = materialNameData->readString( "original", "" );
		std::string displayName = materialNameData->readString( "display", original );

		if (materials_.find(original) != materials_.end())
		{
			materials_[original].name = displayName;
			materials_[original].nameData = materialNameData;
		}
		else
		{
			materialNameDataToRemove_.push_back( materialNameData );
		}
	}

	std::vector< DataSectionPtr >	pDyes;
	currModel_->openSections( "dye", pDyes );
	std::vector< DataSectionPtr >::iterator dyesIt = pDyes.begin();
	std::vector< DataSectionPtr >::iterator dyesEnd = pDyes.end();
	while (dyesIt != dyesEnd)
	{
		DataSectionPtr dye = *dyesIt++;
		std::string replaces = dye->readString("replaces","");
		std::string matter = dye->readString("matter","");

		dyes_[matter] = dye;

		int found = -1;
		for (unsigned i=0; i<materialList_.size(); i++)
		{
			if (materialList_[i].first.first == replaces)
			{
				materialList_[i].first.second = matter;
				found = i;
				break;
			}
		}

		// If we have not found a matching material, try the next
		if ( found == -1 )
			continue;

		std::vector< DataSectionPtr >	pTints;
		dye->openSections( "tint", pTints );
		std::vector< DataSectionPtr >::iterator tintIt = pTints.begin();
		std::vector< DataSectionPtr >::iterator tintEnd = pTints.end();
		while (tintIt != tintEnd)
		{
			DataSectionPtr tint = *tintIt++;
			std::string name = tint->readString("name","");
			materialList_[found].second.push_back( name );

			tints_[matter][name].data = tint;

			tints_[matter][name].format = materials_[replaces].format;

			tints_[matter][name].colours = materials_[replaces].colours;
			tints_[matter][name].dualUV = materials_[replaces].dualUV;

			Tint* pTint = NULL;

			superModel_->topModel(0)->getDye( matter, name, &pTint);

			tints_[matter][name].effect = pTint->effectMaterial_;

			tints_[matter][name].effect->load( tints_[matter][name].data->openSection( "material" ));

			SuperModelDyePtr spDye = superModel_->getDye( matter , name );
			tints_[matter][name].dye = spDye;
		}

		Tint* pTint = NULL;
		superModel_->topModel(0)->getDye( matter, "Default", &pTint);
		tints_[matter]["Default"].effect = pTint->effectMaterial_;

		SuperModelDyePtr spDye = superModel_->getDye( matter , "Default" );
		tints_[matter]["Default"].dye = spDye;

		//Load the default material in case the visual has changed
		if ((materials_.find(replaces) != materials_.end()) && (materials_[replaces].data[0]))
		{
			tints_[matter]["Default"].effect->load( materials_[replaces].data[0] );
		}

		tintedMaterials_[replaces] = matter;
	}

	// Now reload any materials that are NOT tints...
	std::map< std::string, MaterialInfo >::iterator matIt = materials_.begin();
	std::map< std::string, MaterialInfo >::iterator matEnd = materials_.end();

	for ( ;matIt != matEnd; ++matIt )
	{
		DataSectionPtr data = matIt->second.data[0];

		if (data == NULL)
			continue;

		std::string id = data->readString( "identifier", "" );

		if (tintedMaterials_.find( id ) != tintedMaterials_.end())
			continue;
		
		std::set< Moo::EffectMaterialPtr >::iterator sectIt = matIt->second.effect.begin();
		std::set< Moo::EffectMaterialPtr >::iterator sectEnd = matIt->second.effect.end();

		for (;sectIt != sectEnd; ++sectIt)
		{
			Moo::EffectMaterialPtr effect = *sectIt;

			if (effect == NULL)
				continue;

			effect->load( data );
		}
	}
}

/**
 *	recreate the fashions of this model using the current settings
 */
void Mutant::recreateFashions( bool dyesOnly )
{
	BW_GUARD;

	if (!superModel_) return;

	fashions_.clear();
	
	bool actSet = false;
	
	if (!animMode_ && !dyesOnly)
	{
		if ( ( currAct_.first != "" ) && ( currAct_.second != "" ) && ( actions_.find(currAct_) != actions_.end() ) )
		{
			SuperModelActionPtr action = superModel_->getAction( actions_[currAct_].data->readString( "name", "" ) ); 
			if ( action )
			{
				std::vector< SuperModelActionPtr > actionsInUse; 
				actionQueue_.getCurrentActions( actionsInUse );
				if ( !alreadyLooping( actionsInUse, action ) )
				{
					// Add the action to the action queue
					actionQueue_.add( action );
					setupActionMatch( action );
					takeOverOldAction( actionsInUse, action );
					fashions_ = actionQueue_.fv();
					actSet = true;
				}
			}
		}
	}

	if (!clearAnim_ && ( animMode_ || !actSet ) && !dyesOnly )
	{
		actionQueue_.flush(); 

		AnimIt animIt = currAnims_.begin();
		AnimIt animEnd = currAnims_.end();
		
		while (animIt != animEnd)
		{
			if ((animIt->second.first != "") && (animIt->second.second != "") && (animations_.find(animIt->second) != animations_.end()))
			{
				SuperModelAnimationPtr animation = animations_[animIt->second].animation; 
				if (animation)
				{
					animation->blendRatio = 1.f;
					fashions_.push_back( animation );
				}
			}
			animIt++;
		}
	}

	Dyes::iterator dyeIt = currDyes_.begin();
	Dyes::iterator dyeEnd = currDyes_.end();

	while ( dyeIt != dyeEnd )
	{
		if (tints_.find( dyeIt->first ) != tints_.end())
		{
			if (tints_[dyeIt->first].find( dyeIt->second ) != tints_[dyeIt->first].end())
			{
				fashions_.push_back( tints_[dyeIt->first][dyeIt->second].dye );
			}
		}
		dyeIt++;
	}
}

void Mutant::recreateModelVisibilityBox( AnimLoadCallbackPtr pAnimLoadCallback, bool undoable )
{
	BW_GUARD;

	if (!superModel_) return;

	//Reset the bounding box first
	visibilityBB_ = BoundingBox::s_insideOut_;
	
	//Save the current fashion
	FashionVector oldFashion = fashions_;

	//Save the current positional settings
	bool grounded = groundModel_;
	groundModel_ = false;
	bool centred = centreModel_;
	centreModel_ = false;

	//Do an initial update in case this is a static model
	updateFrameBoundingBox();
	updateVisibilityBox();
	
	std::map < StringPair , AnimationInfo >::iterator it =  animations_.begin();
	std::map < StringPair , AnimationInfo >::iterator end =  animations_.end();

	for(; it != end; ++it)
	{
		SuperModelAnimationPtr animation = it->second.animation; 
		if ((animation) && (animation->pSource(*superModel_)))
		{
			fashions_.clear();
			animation->time = 0.f;
			animation->blendRatio = 1.f;
			fashions_.push_back( animation );
			while ( animation->time <= animation->pSource(*superModel_)->duration_ )
			{
				updateFrameBoundingBox();
				updateVisibilityBox();
				animation->time += 1.f / 60.f;
			}

			//Do a final
			animation->time = animation->pSource(*superModel_)->duration_;
			updateFrameBoundingBox();
			updateVisibilityBox();

			if (pAnimLoadCallback)
				pAnimLoadCallback->execute();
		}
	}

	DataSectionPtr data = currModel_->openSection( "visibilityBox" );
	if (data == NULL) data = currModel_->newSection( "visibilityBox" );
		
	if (undoable)
	{
		UndoRedo::instance().add( new UndoRedoOp( 0, data, currModel_ ));
	}

	//Save the new bounding box
	data->writeVector3( "min", visibilityBB_.minBounds() ); 
	data->writeVector3( "max", visibilityBB_.maxBounds() ); 
	dirty( currModel_ );

	if (undoable)
	{
		UndoRedo::instance().barrier( LocaliseUTF8(L"MODELEDITOR/MODELS/MUTANT/UPDATING_VIS_BOX"), false );
	}

	visibilityBoxDirty_ = false; // OK, visibility box all updated

	//Restore the fashion
	fashions_ = oldFashion;

	//Restore the positional settings
	groundModel_ = grounded;
	centreModel_ = centred;
}

void Mutant::updateAnimation( const StringPair& animID, float dTime )
{
	BW_GUARD;

	if (!superModel_) return;

	if ((animID.first != "") && (animID.second != "") && (animations_.find(animID) != animations_.end()))
	{
		SuperModelAnimationPtr animation = animations_[animID].animation; 

		if ((animation) && (animation->pSource(*superModel_)))
		{
			float initFrameRate = animations_[animID].frameRates->readFloat( "init", 30.f );
			float currFrameRate = animations_[animID].frameRates->readFloat( "curr", 30.f );
			
			if (playing_)
			{
				animation->time += dTime * (currFrameRate / initFrameRate);
			}

			while ( animation->time > animation->pSource(*superModel_)->duration_ )
			{
				if (looping_)
				{
					if (animation->pSource(*superModel_)->duration_ != 0.f)
					{
						animation->time -= animation->pSource(*superModel_)->duration_;
					}
					else
					{
						animation->time = 0.f;
					}
				}
				else
				{
					animation->time = 0.f;
					playing_ = false;
				}
			}
		}
	}
}

Vector3 Mutant::getCurrentRootNode()
{
	BW_GUARD;

	if (!superModel_) return Vector3( 0.f, 0.f, 0.f );
	
	Vector3 rootNode( Vector3::zero() );

	if (superModel_ != NULL)
	{
		Model * model = superModel_->curModel( 0 );
		if (model != NULL)
		{
			NodeTreeIterator it  = model->nodeTreeBegin();
			NodeTreeIterator end = model->nodeTreeEnd();

			if (it != end) it++;
			if (it != end) it++;

			if (it != end) 
			{
				Matrix nodeTrans = it->pData->pNode->worldTransform();
				rootNode = nodeTrans.applyToOrigin();
				hasRootNode_ = true;
			}
		}
	}

	return rootNode;
}

Matrix Mutant::transform( bool grounded, bool centred /* = true */ )
{
	BW_GUARD;

	if (!superModel_) return Matrix::identity;

	//TODO: Fix the centering behaviour better.
	
	Matrix transform = Matrix::identity;
	if (grounded)
	{
		transform[3][1] = - modelBB_.minBounds().y;
	}
	if (centred)
	{
		BoundingBox bb = modelBB_;

		/*
		if (hasRootNode_)
		{
			Matrix m = Matrix::identity;
			Vector3 t( getCurrentRootNode() - initialRootNode_ );
			t[1] = 0.f;
			m.setTranslate( t );
			bb.transformBy( m );
		}
		//*/

		Vector3 centre = bb.centre();
		transform[3][0] = -centre.x;
		transform[3][2] = -centre.z;
	}
	return transform;
}

BoundingBox Mutant::zoomBoundingBox()
{
	BW_GUARD;

	if (!superModel_) return BoundingBox( Vector3( 0.f, 0.f, 0.f ), Vector3( 0.f, 0.f, 0.f ));

	BoundingBox bb( modelBB_ );
	Matrix m;
	if (hasRootNode_)
	{
		m = Matrix::identity;
		Vector3 t( getCurrentRootNode() - initialRootNode_ );
		t[0] = 0.f;
		t[2] = 0.f;
		m.setTranslate( t );
	}
	else
	{
		m = transform( groundModel_, centreModel_ );
	}
	bb.transformBy( m );
	return bb;
}

BoundingBox Mutant::modelBoundingBox()
{
	BW_GUARD;

	if (!superModel_) return BoundingBox( Vector3( 0.f, 0.f, 0.f ), Vector3( 0.f, 0.f, 0.f ));

	BoundingBox bb( modelBB_ );
	Matrix m;
	if (hasRootNode_)
	{
		m = Matrix::identity;
		Vector3 t( getCurrentRootNode() - initialRootNode_ );
		m.setTranslate( t );
	}
	else
	{
		m = transform( groundModel_, centreModel_ );
	}
	bb.transformBy( m );
	return bb;
}

/**
 *	Transforms the bounding box relative to the root node if one exists.
 *	If there is no root node the bounding box will be transformed using
 *	the groundModel_ and centreModel_ member variables as rules.
 *
 *	@param inout	The bounding box to be transformed.
 *	@return			The transformed bounding box, same object as inout.
 */
BoundingBox& Mutant::transformBoundingBox( BoundingBox& inout )
{
	BW_GUARD;

	Matrix m;
	if (hasRootNode_)
	{
		m = Matrix::identity;
		Vector3 t( getCurrentRootNode() - initialRootNode_ );
		m.setTranslate( t );
	}
	else
	{
		m = transform( groundModel_, centreModel_ );
	}
	inout.transformBy( m );
	return inout;
}

/**
 *	Returns the visibility box for the mutant.
 *
 *	@return		The visibility bounding box.
 */
BoundingBox Mutant::visibililtyBox()
{
	BW_GUARD;

	if (!superModel_) return BoundingBox( Vector3( 0.f, 0.f, 0.f ), Vector3( 0.f, 0.f, 0.f ));

	BoundingBox bb( visibilityBB_ );

	return this->transformBoundingBox( bb );
}

/**
 *	Returns the shadow visibility box for the mutant.  This differs from that
 *	of the visibility box in that all additional objects added to supermodel
 *	contribute towards the size of the shadow visibility box.
 *
 *	@return		The shadow visibility bounding box.
 */
BoundingBox Mutant::shadowVisibililtyBox()
{
	BW_GUARD;

	if (!superModel_) return BoundingBox( Vector3( 0.f, 0.f, 0.f ), Vector3( 0.f, 0.f, 0.f ));

	BoundingBox bb( visibilityBB_ );
	for (int i = 0; i < superModel_->nModels(); ++i)
	{
		Model* curModel = superModel_->curModel( i );
		if (curModel != NULL)
		{
			bb.addBounds( curModel->visibilityBox() );
		}
	}

	return this->transformBoundingBox( bb );
}

/*
 *	This class calculates the bounding box of an object from the
 *	transformed positions of its vertices.
 *
 */
class BoundingBoxDrawOverride : public Moo::Visual::DrawOverride
{
public:
	BoundingBoxDrawOverride() :
	  bb_( BoundingBox::s_insideOut_ )
	{
	}

	// This is the amount of vertices each model piece are allowed to use
	// for calculating their bounding boxes
	static const uint32 NUM_BB_VERTICES = 1000;

	HRESULT draw( Moo::Visual* pVisual, bool ignoreBoundingBox )
	{
		BW_GUARD;

		typedef Moo::Visual::RenderSetVector RSV;
		RSV& renderSets = pVisual->renderSets();
		RSV::iterator rsit = renderSets.begin();
		RSV::iterator rsend = renderSets.end();

		while (rsit != rsend)
		{
			Moo::Visual::RenderSet& rs = *(rsit++);
			Moo::Visual::GeometryVector::iterator git = rs.geometry_.begin();
			Moo::Visual::GeometryVector::iterator gend = rs.geometry_.end();
			while (git != gend)
			{
				Moo::Visual::Geometry& geom = *(git++);
				Moo::VerticesPtr pVerts = geom.vertices_;
				if (rs.treatAsWorldSpaceObject_)
				{
					// Use the software skinner
					Moo::BaseSoftwareSkinner* pSkinner = pVerts->softwareSkinner();

					if (pSkinner)
					{
						static VectorNoDestructor< Vector3 > positions;
						positions.resize( pVerts->nVertices() );

						uint32 skip = pVerts->nVertices() / NUM_BB_VERTICES;

						pSkinner->transformPositions( &positions.front(),
							0, pVerts->nVertices(), rs.transformNodes_, true,
							skip );

						uint32 nVertices = pVerts->nVertices() / (skip + 1);

						for (uint32 i = 0; i < nVertices; i++)
						{
							bb_.addBounds( positions[i] );
						}
					}
				}
				else
				{
					const Moo::Vertices::VertexPositions& vps = geom.vertices_->vertexPositions();
					const Matrix& wt = rs.transformNodes_[0]->worldTransform();

					uint32 skip = (vps.size() / NUM_BB_VERTICES) + 1;

					for (uint32 i = 0; i < vps.size(); i += skip)
					{
						bb_.addBounds( wt.applyPoint( vps[i] ) );
					}
				}
			}
		}

		return S_OK;
	}

	BoundingBox bb_;
};


void Mutant::updateFrameBoundingBox()
{
	BW_GUARD;

	if (!superModel_) return;

	static DogWatch s_bbTimer("BoundingBox");
	s_bbTimer.start();

	BoundingBoxDrawOverride bbdo;
	Moo::Visual::s_pDrawOverride = &bbdo;
	drawModel( true );
	Moo::Visual::s_pDrawOverride = NULL;
	frameBB_ = bbdo.bb_;

	if ( frameBB_ == BoundingBox::s_insideOut_ )
	{
		Vector3 root = getCurrentRootNode();
		frameBB_ = BoundingBox( root, root );
	}

	s_bbTimer.stop();
}

void Mutant::updateVisibilityBox()
{
	BW_GUARD;

	if (!superModel_) return;

	BoundingBox oldBB = visibilityBB_;
	Matrix t( Matrix::identity );
	Vector3 trans( initialRootNode_ - getCurrentRootNode() );
	t.setTranslate( trans );
	BoundingBox bb( frameBB_ );
	bb.transformBy( t );
	visibilityBB_.addBounds( bb );

	if ( visibilityBB_ == BoundingBox::s_insideOut_ )
	{
		visibilityBB_ = oldBB;
	}
}

void Mutant::updateActions( float dTime )
{
	BW_GUARD;

	if (!superModel_) return;

	actionQueue_.tick( dTime, superModel_, matrixLI_ );

	// Need to clear any cues after the tick
	Moo::CueShot::s_cueShots_.clear();

	//Update any animations that need updating.
	std::set< StringPair > done;
	AnimIt animIt = currAnims_.begin();
	AnimIt animEnd = currAnims_.end();
	while (animIt != animEnd)
	{
		StringPair id = animIt->second;
		if (done.find( id ) == done.end())
		{
			updateAnimation( id, dTime );
			done.insert( id );
		}
		animIt++;
	}
}

void Mutant::dirty( DataSectionPtr data )
{
	BW_GUARD;

	dirty_.insert( data );
}

/**
 * This method can set the change state of Mutant to clean
 */
void Mutant::forceClean()
{
	BW_GUARD;

	// make undo/redo think we have saved
	UndoRedo::instance().setSavePoint();
}


/**
 *	This method checks to see if the model is currently dirty.
 *	Read-Only models can never be dirty.
 */
bool Mutant::dirty()
{
	BW_GUARD;

	if (isReadOnly())
	{
		return false;
	}

	return UndoRedo::instance().needsSave();
}


void Mutant::saveCorrectPrimitiveFile( bool isSaving, bool isForcedSave )
{
	BW_GUARD;

	if ( primitivesName_ == "" )
		return;

	std::string primPath = BWResource::resolveFilename( primitivesName_ );
	std::string backupPath = primPath+".backup";
	std::wstring wbackupPath = bw_utf8tow( backupPath );

	// If the primitives file has
	uint64 currentModifiedTime = BWResource::modifiedTime( primPath );
	if ( modifiedTime_ != currentModifiedTime )
	{
		DWORD att = GetFileAttributes( wbackupPath.c_str() );
		if ( att & FILE_ATTRIBUTE_READONLY )
		{
			att &= ~FILE_ATTRIBUTE_READONLY;
			SetFileAttributes( wbackupPath.c_str(), att );
		}

		::DeleteFile( wbackupPath.c_str() );
		modifiedTime_ = currentModifiedTime;
	}

	// So we do not lose our primitive file
	if ( !BWResource::fileExists( backupPath ) ) return;

	// if we are not saving the model or it hasnt been bumped then restore the original file
	if ( !modelBeenBumped_ || !isSaving || !doAnyEffectsHaveNormalMap() )
	{
		if ( !isForcedSave )
		{
			::DeleteFile( bw_utf8tow( primPath ).c_str() );
			std::rename( backupPath.c_str(), primPath.c_str() );
		}
	}
	else if ( isForcedSave )
	{
		// only called when explicitly saving, this makes the backup
		// the same as the primitives file
		std::string fileSave;
		CopyFile( bw_utf8tow( primPath ).c_str(), wbackupPath.c_str(), false );
		fileSave = "Saving file: "+primitivesName_+"\n";
		ME_INFO_MSGW( bw_utf8tow( fileSave ).c_str() );
		modelBeenBumped_ = false;
	}
	else
	{
		// if we want the bumped information then get rid of backup
		std::string fileSave;
		::DeleteFile( wbackupPath.c_str() );
		fileSave = "Saving file: "+primitivesName_+"\n";
		ME_INFO_MSGW( bw_utf8tow( fileSave ).c_str() );
		modelBeenBumped_ = false;
	}
}


/**
 *	This method saves all dirty datasections.
 */
void Mutant::save()
{
	BW_GUARD;

	// Check to see if we have any undo points or operations done
	if (!dirty())
	{
		return;
	}

	// Do we even have a supermodel?
	if (!superModel_)
	{
		return;
	}

	saveCorrectPrimitiveFile( true, true );

	// Doing this has the interesting effect of saving the datasections of the 
	// model. Some undo operations do not add anything to the dirty list,
	// but doing these operations make up for that mistake.
	cleanMaterials();
	cleanTints();
	cleanMaterialNames();

	for ( std::set<DataSectionPtr>::iterator i = dirty_.begin(); i != dirty_.end(); i++ )
	{
		if (dataFiles_.find(*i) != dataFiles_.end())
		{
			ME_INFO_MSGW( Localise(L"MODELEDITOR/MODELS/MUTANT/SAVING", dataFiles_[*i]) );
		}

		if (*i == currModel_)
			continue;

		(*i)->save();
	}

	metaData_.save( currModel_ );
	currModel_->save();

	dirty_.clear();

	UndoRedo::instance().setSavePoint();

	//Let WorldEditor know that the model has changed
	if( modelName_ != "" )
	{
		HANDLE mailSlot = CreateFile( L"\\\\.\\mailslot\\WorldEditorUpdate",
			GENERIC_WRITE, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
		if( mailSlot != INVALID_HANDLE_VALUE )
		{
			DWORD bytesWritten;
			WriteFile( mailSlot, modelName_.c_str(), modelName_.size() + 1, &bytesWritten, 0 );
			CloseHandle( mailSlot );
		}
	}
}

bool Mutant::saveAs( const std::string& newName )
{
	BW_GUARD;

	cleanMaterials();
	cleanTints();
	cleanMaterialNames();

	std::string newRootName = newName.substr( 0, newName.find_last_of( "." ));
	std::string oldRootName = currModel_->readString( "nodefullVisual", "" );
	
	std::string type;
	if (oldRootName != "")
	{
		type = "nodefullVisual";
	}
	else
	{
		oldRootName = currModel_->readString( "nodelessVisual", "" );
		if (oldRootName != "")
		{
			type = "nodelessVisual";
		}
		else
		{
			oldRootName = currModel_->readString( "billboardVisual", "" ); 
			if (oldRootName != "")
			{
				type = "billboardVisual";
			}
			else
			{
				return false;
			}
		}
	}
	currModel_->writeString( type, newRootName );
	metaData_.save( currModel_ );

	currModel_->save( newName );
	currVisual_->save( newRootName + ".visual" );

	std::string oldPrimitivesName = oldRootName + ".primitives";
	oldPrimitivesName = BWResource::resolveFilename( oldPrimitivesName );
	std::string newPrimitivesName = newRootName + ".primitives";
	newPrimitivesName = BWResource::resolveFilename( newPrimitivesName );
	CopyFile( bw_utf8tow( oldPrimitivesName ).c_str(), bw_utf8tow( newPrimitivesName ).c_str(), FALSE );

	return loadModel( newName );
}

void Mutant::edit( GeneralEditor& editor )
{
	BW_GUARD;

	if (superModel_ && superModel_->nModels())
	{
		metaData_.edit( editor, L"", false );
	}
}

void Mutant::updateModificationInfo()
{
	BW_GUARD;

	if (superModel_ && superModel_->nModels()
		&& !UndoRedo::instance().isUndoing())
	{
		MetaData::updateModificationInfo( metaData_ );
	}
}
