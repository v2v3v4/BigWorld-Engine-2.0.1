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

#include "nodefull_model.hpp"

#include "moo/animation.hpp"
#include "moo/animation_manager.hpp"
#include "moo/streamed_animation_channel.hpp"
#include "moo/streamed_data_cache.hpp"
#include "moo/visual_manager.hpp"

#include "nodefull_model_animation.hpp"
#include "nodefull_model_default_animation.hpp"

#include "super_model.hpp"


DECLARE_DEBUG_COMPONENT2( "Model", 0 )


/**
 *	Constructor
 */
NodefullModel::NodefullModel( const std::string & resourceID, DataSectionPtr pFile ) :
	Model( resourceID, pFile ),
	pAnimCache_( NULL ),
	batched_( false ) 
{
	BW_GUARD;
	static std::vector < std::string > s_animWarned;

    // load our bulk
	std::string bulkName = pFile->readString( "nodefullVisual" ) + ".visual";

	// see if we're batching it
#ifndef EDITOR_ENABLED
	batched_ = pFile->readBool( "batched", false );
#endif

	bulk_ = Moo::VisualManager::instance()->get( bulkName );
	if (!bulk_)
	{
		ERROR_MSG( "NodefullModel::NodefullModel: "
			"Could not load visual resource '%s' as main bulk of model %s\n",
			bulkName.c_str(), resourceID_.c_str() );
		return;		// maybe we should load a placeholder
	}

	// Get the visibilty box for the model if it exists,
	// if it doesn't then use the visual's bounding box instead.
	DataSectionPtr bb = pFile->openSection("visibilityBox");
	if (bb)
	{
		Vector3 minBB = bb->readVector3("min", Vector3(0.f,0.f,0.f));
		Vector3 maxBB = bb->readVector3("max", Vector3(0.f,0.f,0.f));
		visibilityBox_ = BoundingBox( minBB, maxBB );
	}
	else
	{
		visibilityBox_ = bulk_->boundingBox();
	}

	std::avector<Matrix>	initialPose;

	// add any nodes found in our visual to the node table,
	// and build the node index tree at the same time
	std::vector<Moo::NodePtr>	stack;
	stack.push_back( bulk_->rootNode() );
	while (!stack.empty())
	{
		Moo::NodePtr pNode = stack.back();
		stack.pop_back();

		// save this node's initial transform
		initialPose.push_back( pNode->transform() );

		// add this node to our table, if it's not already there
		Moo::Node * pStorageNode =
			Moo::NodeCatalogue::findOrAdd( pNode );

		// do the same for the children
		for (int i = pNode->nChildren() - 1; i >= 0 ; i--)
		{
			stack.push_back( pNode->child( i ) );
		}

		// and fill in the tree
		nodeTree_.push_back( NodeTreeData( pStorageNode, pNode->nChildren() ) );
	}	

	// load dyes
	this->readDyes( pFile, true );

	// add the default 'initial pose' animation
	pDefaultAnimation_ = new NodefullModelDefaultAnimation( nodeTree_, 1, initialPose );
	if (!pDefaultAnimation_->valid())
	{
		ERROR_MSG(	"Unable to create the default pose for model '%s'\n",
						this->resourceID().c_str() );
		pDefaultAnimation_ = NULL;
		return;
	}

	// TODO: Figure out what the itinerant node index really is instead
	//  of just assuming that it's 1 (maybe get from an anim?). It is also
	//  possible that some visuals may not have an itinerant node at all.

	// read in the sections describing our animations
	std::vector<DataSectionPtr>	vAnimSects;
	pFile->openSections( "animation", vAnimSects );

	// set up an animation cache for this model, if we have any animations
	if (!vAnimSects.empty())
	{
		std::string animCacheName = resourceID;
		if (animCacheName.length() > 5)
			animCacheName.replace( animCacheName.length()-5, 5, "anca" );
		else
			animCacheName.append( ".anca" );
		pAnimCache_ = new Moo::StreamedDataCache( animCacheName, true );

		for( uint i = 0; i < vAnimSects.size(); ++i )
		{
			DataSectionPtr pAnimData;
			if( !( pAnimData = vAnimSects[ i ]->openSection( "nodes" ) ) )
				continue;// we'll revenge later

			std::string nodesRes = pAnimData->asString() + ".animation";
			uint64 modifiedTime = BWResource::modifiedTime( nodesRes );

			if( pAnimCache_->findEntryData( nodesRes, Moo::StreamedDataCache::CACHE_VERSION, modifiedTime ) == NULL )
			{
				pAnimCache_->deleteOnClose( true );
				delete pAnimCache_;
				pAnimCache_ = new Moo::StreamedDataCache( animCacheName, true );
				break;
			}
		}
#ifndef EDITOR_ENABLED
		Moo::StreamedAnimation::s_loadingAnimCache_ = pAnimCache_;
		Moo::StreamedAnimation::s_savingAnimCache_ = pAnimCache_;
#endif
	}

	// We keep a temporary map of the animations for this model in 
	// case multiple sub-animations using the same resource are used
	typedef std::map< std::string, Moo::AnimationPtr > AnimMap;
	AnimMap modelAnims;

	// load our animations in the context of the global node table.
	for (uint i = 0; i < vAnimSects.size(); i++)
	{
		DataSectionPtr pAnimSect = vAnimSects[i];

		// read the name
		std::string animName = pAnimSect->readString( "name" );

		// now read the node stuff
		DataSectionPtr pAnimData;
		if (!(pAnimData = pAnimSect->openSection( "nodes" )))
		{
			ERROR_MSG( "Animation %s (section %d) in model %s has no 'nodes' section\n",
				animName.c_str(), i, resourceID.c_str() );
			continue;
		}

		std::string nodesRes = pAnimData->asString();

		if (animName.empty())
		{
        	const char * nameBeg = nodesRes.data() +
				nodesRes.find_last_of( '/' )+1;
        	const char * nameEnd = nodesRes.data() +
				nodesRes.length();
                animName.assign( nameBeg, nameEnd );
		}

		if (animName.empty())
		{
			ERROR_MSG( "Animation section %d in model %s has no name\n",
				i, resourceID.c_str() );
			continue;
		}

		// ok, now we know where to get it and what to call it,
		//  let's load the actual animation!
		nodesRes += ".animation";

#ifdef EDITOR_ENABLED
		// We don't use the .anca files in the editor so just grab our animation
		// fromthe animation manager
		Moo::AnimationPtr pAnim = Moo::AnimationManager::instance().get( nodesRes );
#else
		// Since we are using .anca files we do not want to get animations
		// from the anim manager. 
		Moo::AnimationPtr pAnim = NULL;

		// See if we have already loaded the animation for this model
		AnimMap::iterator loadedAnim = modelAnims.find( nodesRes );
		if (loadedAnim != modelAnims.end())
		{
			pAnim = loadedAnim->second;
		}
		else
		{
			// If the animation has not already been loaded, load it up,
			// don't use the animation manager as this may caus us to use
			// an the animation from a different .anca file, which will
			// be invalid if the owning model is removed from the scene.
			pAnim = new Moo::Animation;
			if (!pAnim->load(nodesRes))
			{
				pAnim = NULL;
			}
			else
			{
				// This line makes sure the node catalogue is set up
				// TODO: Fix this so it makes a bit more sense
				pAnim = new Moo::Animation( pAnim.get() );
			}

			modelAnims.insert( std::make_pair( nodesRes, pAnim ) );
		}
#endif //EDITOR_ENABLED
		//static int omu = memUsed() - memoryAccountedFor();
		//int nmu = memUsed() - memoryAccountedFor();
		//dprintf( "Using %d more, %d total after %s\n", nmu-omu, nmu, nodesRes.c_str() );
		//omu = nmu;

#ifdef EDITOR_ENABLED
		if (s_pAnimLoadCallback_)
			s_pAnimLoadCallback_->execute();	
#endif

        if (!pAnim)
		{
			if (std::find( s_animWarned.begin(), s_animWarned.end(), nodesRes + animName + resourceID ) ==  s_animWarned.end())
            {
				std::string modelName = BWResource::getFilename( resourceID );
				ERROR_MSG( "Could not load the animation:\n\n"
					"  \"%s\"\n\n"
					"for \"%s\" of \"%s\".\n\n"
					"Please make sure this file exists or set a different one using ModelEditor.",
					nodesRes.c_str(), animName.c_str(), modelName.c_str() );
				s_animWarned.push_back( nodesRes + animName + resourceID );
            }
			continue;
		}

		NodefullModelAnimation * pNewAnim = new NodefullModelAnimation( *this, pAnim, pAnimSect );
		int existingIndex = this->getAnimation( animName );
		if (existingIndex == -1)
		{
			animationsIndex_.insert(
				std::make_pair( animName, animations_.size() ) );
			animations_.push_back( pNewAnim );
		}
		else
		{
			animations_[ existingIndex ] = pNewAnim;
		}
	}

	// reset the static animation cache pointers
	Moo::StreamedAnimation::s_loadingAnimCache_ = NULL;
	Moo::StreamedAnimation::s_savingAnimCache_ = NULL;

	// delete it if it's empty
	if (pAnimCache_ && pAnimCache_->numEntries() == 0)
	{
		pAnimCache_->deleteOnClose( true );
		delete pAnimCache_;
		pAnimCache_ = NULL;
	}

	postLoad( pFile );
}

/**
 *	Destructor
 */
NodefullModel::~NodefullModel()
{
	BW_GUARD;
	if (pAnimCache_ != NULL)
		delete pAnimCache_;
}


#ifdef EDITOR_ENABLED
/**
 *	This function reload the current model from the disk.
 *	This function isn't thread safe. It will only be called
 *	For reload an externally-modified model in WorldEditor
 */
void NodefullModel::reload()
{
	BW_GUARD;
	std::string res = resourceID_;

	long rc = refCount();
	this->~NodefullModel();
	new (this) NodefullModel( res, BWResource::openSection( res ) );
	addToMap();
	incRef( rc );
}
#endif//EDITOR_ENABLED

/**
 *	This function returns true if the NodefullModel is in a valid state.
 *	This function will not raise a critical error or assert if called but may
 *	commit ERROR_MSGs to the log.
 *
 *	@return		Returns true if the model is in a valid state, otherwise false.
 */
bool NodefullModel::valid() const
{
	BW_GUARD;
	if (!this->Model::valid())
		return false;

	if (!(bulk_ && bulk_->isOK()))
		return false;

	return true;
}


/**
 *	This method traverses this nodefull model, and soaks its dyes
 */
void NodefullModel::dress()
{
	BW_GUARD;
	const std::avector<Matrix> & initialPose = static_cast<NodefullModelDefaultAnimation&>
		(*pDefaultAnimation_).cTransforms();

	world_ = Moo::rc().world();

	static struct TravInfo
	{
		const Matrix	* trans;
		int					pop;
	} stack [NODE_STACK_SIZE ];
	int			sat = 0;

	// start with the root node
	stack[sat].trans = &world_;
	stack[sat].pop = nodeTree_.size();
	for (uint i = 0; i < nodeTree_.size(); i++)
	{
		// pop off any nodes we're through with
		while (stack[sat].pop-- <= 0) --sat;

		NodeTreeData & ntd = nodeTree_[i];

		// make sure it's valid for this supermodel
		if (ntd.pNode->blend( Model::blendCookie() ) == 0)
			ntd.pNode->blendClobber( Model::blendCookie(), initialPose[i] );

		// calculate this one
		ntd.pNode->visitSelf( *stack[sat].trans );

		// and visit its children if it has any
		if (ntd.nChildren > 0)
		{
			++sat;

			if (sat >= NODE_STACK_SIZE)
			{
				--sat;
				ERROR_MSG( "Node Hierarchy too deep, unable to dress nodes\n" );
				/* Set this equal to the end to prevent infinite loops 
				*/
				stack[sat].trans = &ntd.pNode->worldTransform();
				stack[sat].pop = ntd.nChildren;
				return;
			}

			stack[sat].trans = &ntd.pNode->worldTransform();
			stack[sat].pop = ntd.nChildren;
		}
	}
}



/**
 *	This method draws this nodefull model.
 */
void NodefullModel::draw( bool checkBB )
{
	BW_GUARD;
	Moo::rc().push();
	Moo::rc().world( world_ );
#ifndef EDITOR_ENABLED
	if (!batched_)
		bulk_->draw( !checkBB, false );
	else
		bulk_->batch( !checkBB, false );
#else
	bulk_->draw( !checkBB, false );
#endif
	Moo::rc().pop();
}


/**
 *	This method returns a BSPTree of this model when decomposed
 */
const BSPTree * NodefullModel::decompose() const
{
	BW_GUARD;
	return bulk_ ? bulk_->pBSPTree() : NULL;
}


/**
 *	This method returns the bounding box of this model
 */
const BoundingBox & NodefullModel::boundingBox() const
{
	BW_GUARD;
	if (!bulk_)
		return BoundingBox::s_insideOut_;
	return bulk_->boundingBox();
}

/**
 *	This method returns the bounding box of this model
 */
const BoundingBox & NodefullModel::visibilityBox() const
{
	BW_GUARD;
	return visibilityBox_;
}

/**
 *  Returns note tree iterator pointing to first node in tree.
 */
NodeTreeIterator NodefullModel::nodeTreeBegin() const
{
	BW_GUARD;
	const int size = nodeTree_.size();
	return size > 0
		? NodeTreeIterator( &nodeTree_[0], (&nodeTree_[size-1])+1, &world_ )
		: NodeTreeIterator( 0, 0, 0 );
}

/**
 *  Returns note tree iterator pointing to one past last node in tree.
 */
NodeTreeIterator NodefullModel::nodeTreeEnd() const
{
	BW_GUARD;
	const int size = nodeTree_.size();
	return size > 0
		? NodeTreeIterator( &nodeTree_[size-1]+1, (&nodeTree_[size-1])+1, &world_ )
		: NodeTreeIterator( 0, 0, 0 );
}

/**
 *  override all materials with the same identifier, then return a MaterialOverride to make the action recoverable
 */
MaterialOverride NodefullModel::overrideMaterial( const std::string& identifier, Moo::EffectMaterialPtr material )
{
	BW_GUARD;
	if( materialOverrides_.find( identifier ) == materialOverrides_.end() )
	{// create a new one
		MaterialOverride mo;
		mo.identifier_ = identifier;
		bulk_->gatherMaterials( identifier, mo.effectiveMaterials_, NULL );
		materialOverrides_[ identifier ] = mo;
	}
	materialOverrides_[ identifier ].update( material );
//	bulk_->overrideMaterial( identifier, *material );
	return materialOverrides_[ identifier ];
}


/**
 *	This method returns whether or not the model has this node
 */
bool NodefullModel::hasNode( Moo::Node * pNode,
	MooNodeChain * pParentChain ) const
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( pNode != NULL )
	{
		return false;
	}

	if (pParentChain == NULL || nodeTree_.empty())
	{
		for (NodeTree::const_iterator it = nodeTree_.begin();
			it != nodeTree_.end();
			it++)
		{
			if (it->pNode == pNode) return true;
		}
	}
	else
	{
		static VectorNoDestructor<NodeTreeData> stack;
		stack.clear();

		stack.push_back( NodeTreeData( NULL, 1 ) );	// dummy parent for root
		for (uint i = 0; i < nodeTree_.size(); i++)
		{
			// pop off any nodes we're through with
			while (stack.back().nChildren-- <= 0) stack.pop_back();

			// see if this is the one
			const NodeTreeData & ntd = nodeTree_[i];
			if (ntd.pNode == pNode)
			{
				pParentChain->clear();
				for (uint j = 0; j < stack.size(); j++)
					pParentChain->push_back( stack[i].pNode );
				return true;
			}

			// and visit its children if it has any
			if (ntd.nChildren > 0) stack.push_back( ntd );
		}
	}

	return false;
}


int NodefullModel::initMatter( Matter & m )
{
	BW_GUARD;
	return initMatter_NewVisual( m, *bulk_ );
}


bool NodefullModel::initTint( Tint & t, DataSectionPtr matSect )
{
	BW_GUARD;
	return initTint_NewVisual( t, matSect );
}



// nodefull_model.cpp
