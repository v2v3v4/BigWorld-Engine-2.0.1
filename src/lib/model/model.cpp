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

#include "model.hpp"

#include "model_actions_iterator.hpp"
#include "model_animation.hpp"
#include "model_common.hpp"
#include "model_factory.hpp"
#include "model_map.hpp"
#include "model_static_lighting.hpp"
#include "nodefull_model.hpp"
#include "nodeless_model.hpp"
#include "tint.hpp"
#include "moo/resource_load_context.hpp"

#include "resmgr/auto_config.hpp"

DECLARE_DEBUG_COMPONENT2( "Model", 0 )
PROFILER_DECLARE( Model_get, "Model get" );


Model::PropCatalogue Model::s_propCatalogue_;
SimpleMutex Model::s_propCatalogueLock_;
int Model::s_blendCookie_ = 0x08000000;
/*static*/ std::vector< std::string > Model::s_warned_;

#ifdef EDITOR_ENABLED
/*static*/ SmartPointer< AnimLoadCallback > Model::s_pAnimLoadCallback_ = NULL;
static AutoConfigString s_modelMetaDataConfig( "editor/metaDataConfig/model", "helpers/meta_data/model.xml" );
#endif


template <class TYPE>
uint32 contentSize( const std::vector< TYPE >& vector )
{
	return vector.size() * sizeof( TYPE );
}

template <class TYPE>
uint32 contentSize( const StringMap< TYPE >& vector )
{
	return vector.size() * sizeof( TYPE );
}


/**
 *	This class lets us use a singleton of another class in a safe way
 */
template <class C> class OneClassPtr
{
public:
	C * operator->()
		{ if (pC == NULL) pC = new C(); return pC; }
	C & operator*()
		{ return *this->operator->(); }
	~OneClassPtr()
		{ if (pC != NULL) { delete pC; pC = NULL; } }
private:
	static C	* pC;
};

typedef OneClassPtr<ModelMap> OneModelMapPtr;

template <> ModelMap * OneClassPtr<ModelMap>::pC = NULL;
//ModelMap * OneModelMapPtr::pC = NULL;

static OneModelMapPtr s_pModelMap;




/**
 *	This class defines and manages callbacks which occur at various
 *	times in the model loading process.
 */
class PostLoadCallBack
{
public:
	virtual ~PostLoadCallBack() {};

	virtual void operator()() = 0;

	static void add( PostLoadCallBack * plcb, bool global = false );
	static void run( bool global = false );

	static void enter();
	static void leave();

private:

	/**
	 *	Helper struct to store the callbacks for one thread
	 */
	struct PLCBs
	{
		PLCBs() : enterCount( 0 ) { }

		int									enterCount;
		std::vector< PostLoadCallBack * >	globals;
		std::vector< PostLoadCallBack * >	locals;
	};

	typedef std::map< uint32, PLCBs > PLCBmap;
	static PLCBmap		threads_;
	static SimpleMutex	threadsLock_;
};

PostLoadCallBack::PLCBmap PostLoadCallBack::threads_;
SimpleMutex PostLoadCallBack::threadsLock_;

/**
 *	This method adds a callback object to either the global or local list
 */
void PostLoadCallBack::add( PostLoadCallBack * plcb, bool global )
{
	BW_GUARD;
	SimpleMutexHolder smh( threadsLock_ );

	PLCBs & plcbs = threads_[OurThreadID()];
	if (!global)	plcbs.locals.push_back ( plcb );
	else			plcbs.globals.push_back( plcb );
}

/**
 *	This method runs all callbacks from one of the lists
 */
void PostLoadCallBack::run( bool global )
{
	BW_GUARD;
	SimpleMutexHolder smh( threadsLock_ );
	std::vector< PostLoadCallBack * > * cbs;

	PLCBs & plcbs = threads_[OurThreadID()];
	cbs = global ? &plcbs.globals : &plcbs.locals;

	while (!cbs->empty())
	{
		PostLoadCallBack * lf = cbs->front();
		cbs->erase( cbs->begin() );

		threadsLock_.give();
		(*lf)();
		delete lf;
		threadsLock_.grab();

		PLCBs & plcbs = threads_[OurThreadID()];
		cbs = global ? &plcbs.globals : &plcbs.locals;
	}
}


/**
 *	Enter a level of model loads in the current thread.
 */
void PostLoadCallBack::enter()
{
	BW_GUARD;
	SimpleMutexHolder smh( threadsLock_ );
	threads_[OurThreadID()].enterCount++;
}

/**
 *	Leave a level of model loads in the current thread.
 *	If it was the last level, call the global callbacks.
 */
void PostLoadCallBack::leave()
{
	BW_GUARD;
	threadsLock_.grab();
	bool wasLastLevel = (--threads_[OurThreadID()].enterCount == 0);
	threadsLock_.give();

	if (wasLastLevel) run( true );
}


/**
 *	This class is an object for calling a model's postLoad function
 */
class ModelPLCB : public PostLoadCallBack
{
public:
	ModelPLCB( Model & m, DataSectionPtr p ) : m_( m ), p_( p ) {};

private:
	virtual void operator()() { m_.postLoad( p_ ); }

	Model & m_;
	DataSectionPtr p_;
};




#pragma warning( disable: 4355 )

/**
 *	Constructor
 */
Model::Model( const std::string & resourceID, DataSectionPtr pFile ) :
	resourceID_( resourceID ),
	parent_( NULL ),
	extent_( 0.f ),
	pDefaultAnimation_( NULL )
#ifdef EDITOR_ENABLED
	,metaData_( this )
#endif
{
	BW_GUARD;
	// read the parent
	std::string parentResID = pFile->readString( "parent" );
	if (!parentResID.empty()) parent_ = Model::get( parentResID + ".model" );

	// read the extent
	extent_ = pFile->readFloat( "extent", 10000.f );

	// copy our parent's stuff
	if (parent_)
	{
		animations_ = parent_->animations_;

		matters_.reserve( parent_->matters_.size() );
		for (uint i=0; i < parent_->matters_.size(); i++)
		{
			Matter & pm = *parent_->matters_[i];

			matters_.push_back( new Matter( pm.name_, pm.replaces_, pm.tints_ ) );
		}
	}

	// schedule postLoad to be called
	PostLoadCallBack::add( new ModelPLCB( *this, pFile ) );

#ifdef EDITOR_ENABLED
	static MetaData::Desc dataDesc( s_modelMetaDataConfig.value() );
	metaData_.load( pFile, dataDesc );
#endif
}


/**
 *	Destructor
 */
Model::~Model()
{
	BW_GUARD;
	// subtract the memory we use from the counter
	memoryCounterSub( model );

	memoryClaim( matters_ );
	for (uint i = 0; i < matters_.size(); i++)
	{
		delete matters_[i];
		matters_[i] = NULL;
	}
	matters_.clear();

	memoryClaim( resourceID_ );
	memoryClaim( this );

	// remove ourselves from the map of loaded models
	s_pModelMap->del( resourceID_ );
}


/**
 *	This function add the current model into global
 *	model map
 */
void Model::addToMap()
{
	BW_GUARD;
	s_pModelMap->add( this, resourceID_ );
}


/**
 *	This method loads a model's dyes, if it has them. If multipleMaterials is
 *	true, then it calls the derived model to search for searches for material
 *	names in its bulk. Otherwise the dyes are read as if for a billboard model,
 *	which can have only one material (and thus dyes must each be composited).
 */
int Model::readDyes( DataSectionPtr pFile, bool multipleMaterials )
{
	BW_GUARD;
	int dyeProduct = 1;

	// read our own dyes
	std::vector<DataSectionPtr> vDyeSects;
	pFile->openSections( "dye", vDyeSects );
	for (uint i = 0; i < vDyeSects.size(); i++)
	{
		DataSectionPtr pDyeSect = vDyeSects[i];

		std::string matterName = pDyeSect->readString( "matter", "" );
		std::string replaces = pDyeSect->readString( "replaces", "" );

		//Ignore any empty "dye" sections
		if ((matterName == "") && (replaces == ""))
			continue;

		// see if we already have a matter of that name


		Matters::iterator miter = matters_.begin();
		for (; miter != matters_.end(); ++miter)
		{
			if ((*miter)->name_ == matterName)
				break;
		}

		if (miter == matters_.end())
		{
			matters_.push_back( new Matter( matterName, replaces ) );
			miter = matters_.end() - 1;
		}

		// set it up
		Matter * m = *miter;
		m->replaces_ = replaces;

		// get all instances of replaces from the visual,
		// and set the default tint from the original material
		if (multipleMaterials)	// not for billboards
		{
			if (this->initMatter( *m ) == 0)
			{
				ERROR_MSG( "Model::readDyes: "
					"In model \"%s\", for dye matter \"%s\", "
					"the material id \"%s\" isn't in the visual\n",
					resourceID_.c_str(), matterName.c_str(), replaces.c_str() );

				continue;
			}
		}
		else					// for billboards
		{
			//miter->second.tints_["Default"]->oldMaterial_->textureFactor( 0 );
		}

		// add the other tints
		std::vector<DataSectionPtr> vTintSects;
		pDyeSect->openSections( "tint", vTintSects );
		for (uint j = 0; j < vTintSects.size(); j++)
		{
			DataSectionPtr pTintSect = vTintSects[j];

			std::string tintName = pTintSect->readString( "name", "" );
			if (tintName.empty())
			{
				ERROR_MSG( "Model::readDyes: "
					"In model %s, for dye matter %s, "
					"tint section index %d has no name.\n",
					resourceID_.c_str(), matterName.c_str(), j );
				continue;
			}

			// if it's already there then use that one
			Matter::Tints::iterator titer = m->tints_.begin();
			for (; titer != m->tints_.end(); ++titer)
			{
				if ((*titer)->name_ == tintName)
					break;
			}

			// Create if missing
			if (titer == m->tints_.end())
			{
				
				m->tints_.push_back( new Tint( tintName ) );
				titer = m->tints_.end() - 1;
			}

			TintPtr t = *titer;

			// if it's not a billboard...
			if (multipleMaterials)
			{
				// clear source dyes that we don't use
				t->sourceDyes_.clear();

				// read the material
				DataSectionPtr matSect = pTintSect->openSection( "material" );
				if (!this->initTint( *t, matSect ))
				{
					WARNING_MSG( "Model::readDyes: "
						"In model %s, for dye matter %s tint %s, "
						"there %s <material> section.\n",
						resourceID_.c_str(), matterName.c_str(), tintName.c_str(),
						(!!matSect) ? "was a problem loading the" : "is no" );
				}

				// read the properties
				std::vector<DataSectionPtr> vPropSects;
				pTintSect->openSections( "property", vPropSects );
				for (uint k = 0; k < vPropSects.size(); k++)
				{
					DataSectionPtr pPropSect = vPropSects[k];

					std::string propName = pPropSect->readString( "name", "" );
					if (propName.empty())
					{
						ERROR_MSG( "Model::readDyes: "
							"In model %s, for dye matter %s tint %s, "
							"property section index %d has no name.\n",
							resourceID_.c_str(), matterName.c_str(),
							tintName.c_str(), k );
						continue;
					}

					s_propCatalogueLock_.grab();
					std::stringstream catalogueName;
					catalogueName << matterName << "." << tintName << "." << propName;
					PropCatalogue::iterator pit =
						s_propCatalogue_.find( catalogueName.str().c_str() );
					if (pit == s_propCatalogue_.end())
					{
						{
							memoryCounterSub( modelGlobl );
							memoryClaim( s_propCatalogue_ );
						}
						pit = s_propCatalogue_.insert(
							std::make_pair( catalogueName.str().c_str(), Vector4() ) );
						{
							memoryCounterAdd( modelGlobl );
							memoryClaim( s_propCatalogue_ );
						}
					}

					DyeProperty dp;
					dp.index_ = pit - s_propCatalogue_.begin();
					s_propCatalogueLock_.give();
					dp.controls_ = pPropSect->readInt( "controls", 0 );
					dp.mask_ = pPropSect->readInt( "mask", 0 );
					dp.future_ = pPropSect->readInt( "future", 0 );
					dp.default_ = pPropSect->readVector4( "default",
						Vector4(0,0,0,0) );

					if (dp.mask_ == -1) dp.mask_ = 0;
					// look for a corresponding D3DXHANDLE
					// (if newMaterial is NULL, won't have any props anyway)
					if ( t->effectMaterial_->pEffect() )
					{
						ID3DXEffect * effect = t->effectMaterial_->pEffect()->pEffect(); 
						D3DXHANDLE propH = effect->GetParameterByName( NULL, propName.c_str() ); 
	
						if (propH != NULL)
						{
							Moo::EffectPropertyFunctorPtr factory = Moo::g_effectPropertyProcessors.find( "Vector4" )->second; 
							Moo::EffectPropertyPtr modifiableProperty = factory->create( propH, effect ); 
							modifiableProperty->be( dp.default_ ); 
							t->effectMaterial_->replaceProperty( propName, modifiableProperty ); 

							dp.controls_ = (int)propH;
							dp.mask_ = -1;
						}
					}

					uint l;
					for (l = 0; l < t->properties_.size(); l++)
					{
						if (t->properties_[l].index_ == dp.index_) break;
					}
					if (l < t->properties_.size())
						t->properties_[l] = dp;
					else
						t->properties_.push_back( dp );

					// end of properties
				}
			}
			// if it's a billboard...
			else
			{
				t->properties_.clear();
				t->sourceDyes_.clear();


				// read the source dye selections
				std::vector<DataSectionPtr> vSourceDyes;
				pTintSect->openSections( "dye", vSourceDyes );
				for (uint k = 0; k < vSourceDyes.size(); k++)
				{
					DataSectionPtr pSourceDye = vSourceDyes[k];
					DyeSelection dsel;

					if (!this->readSourceDye( pSourceDye, dsel ))
					{
						ERROR_MSG( "Model::readDyes: "
							"In model %s, for dye matter %s tint %s, "
							"source dye selection section index %d is missing "
							"either a matter ('%s') or tint ('%s') name.\n",
							resourceID_.c_str(), matterName.c_str(),
							tintName.c_str(), k,
							dsel.matterName_.c_str(), dsel.tintName_.c_str() );
					}

					t->sourceDyes_.push_back( dsel );
				}


			}

			// end of tints
		}

		// keep a running total of the dye product
		dyeProduct *= m->tints_.size();

		MF_ASSERT( dyeProduct );

		// end of matters (dyes)
	}

	// end of method
	return dyeProduct;
}



/*static*/ void Model::clearReportedErrors()
{
	BW_GUARD;
	s_warned_.clear();
}

/**
 *	Static function to get the model with the given resource ID
 */
ModelPtr Model::get( const std::string & resourceID, bool loadIfMissing )
{
	BW_GUARD_PROFILER( Model_get );
	ModelPtr pFound = s_pModelMap->find( resourceID );
	if (pFound)
		return pFound;

	if (!loadIfMissing)
		return NULL;

	DataSectionPtr pFile = BWResource::openSection( resourceID );
	if (!pFile)
	{
		std::vector< std::string >::iterator entry = std::find( s_warned_.begin(), s_warned_.end(), resourceID );
		if (entry == s_warned_.end())
		{
			
			ERROR_MSG( "Model::get: Could not open model resource %s\n",
				resourceID.c_str() );
			s_warned_.push_back( resourceID );
		}
		return NULL;
	}

	PostLoadCallBack::enter();

	ModelPtr mp = Model::load( resourceID, pFile );

	if (mp)
	{
		// must take reference to model before adding to map, or else
		// another thread looking for it could think it's been deleted
		s_pModelMap->add( &*mp, resourceID );
	}

	PostLoadCallBack::leave();

	return mp;
}

/**
 *	This method returns the size of this Model in bytes.
 */
uint32 Model::sizeInBytes() const
{
	BW_GUARD;
	uint32 size = sizeof(*this) + resourceID_.length();
	uint parentSize;

	AnimationsIndex::const_iterator it;

	size += animations_.capacity() * sizeof(animations_[0]);
	parentSize = parent_ ? parent_->animations_.size() : 0;
	for (it = animationsIndex_.begin(); it != animationsIndex_.end(); it++)
	{
		size += 16;	// about the size of a tree node, excluding value_type
		size += sizeof(it) + it->first.length();
	}
	for (uint i = parentSize; i < animations_.size(); i++)
		size += animations_[i]->sizeInBytes();

	size += actions_.capacity() * sizeof(actions_[0]);
	Actions::const_iterator it2;
	for (it2 = actions_.begin(); it2 != actions_.end(); it2++)
	{
		size += 16;										// map node overhead
		size += sizeof(ActionsIndex::const_iterator);	// map node size

		size += (*it2)->sizeInBytes();
	}

	return size;
}

/**
 *	This method reloads the referenced model - and returns a new model.
 *	The old model is not deleted, but future calls to the static 'get'
 *	method (and thus future SuperModel instantiations) will return
 *	the new model.
 *
 *	@note The same model can be reloaded more than once, i.e.
 *	this model needn't be the one in the model map used by 'get'
 */
ModelPtr Model::reload( DataSectionPtr pFile /* = NULL */, bool reloadChildren /* = true */ ) const
{
	BW_GUARD;
	if (!pFile) pFile = BWResource::openSection( resourceID_ );
	if (!pFile)
	{
		ERROR_MSG( "Model::reload: Could not open model resource %s\n",
			resourceID_.c_str() );
		return NULL;
	}

	PostLoadCallBack::enter();

    // must use a strong ref, or if we have children, the model will be deleted
    // before the end of this method, as at this point the children are the only
	// other thing that has a strong ref to this model.
	ModelPtr mp = Model::load( resourceID_, pFile );

	if (mp)
	{
		// also need a strong ref before we call add
		s_pModelMap->add( &*mp, resourceID_ );
		// intentionally replacing existing entry
	}

	if (reloadChildren)
	{
		// now go through and reload any models that have us as a parent
		std::vector< ModelPtr > children;
		s_pModelMap->findChildren( resourceID_, children );
		// due to the doxygen note above, we can't just see if the parent
		// pointer is the this pointer - because it could be pointing
		// to an intermediate old model. So we compare their resource IDs.

		for (uint i = 0; i < children.size(); i++)
		{
			children[i]->reload();
		}
	}

	PostLoadCallBack::leave();

	return mp;
}




/**
 *	This private static method performs operations common to
 *	both the 'get' and 'reload' methods.
 */
ModelPtr Model::load( const std::string & resourceID, DataSectionPtr pFile )
{
	BW_GUARD;
	Moo::ScopedResourceLoadContext resLoadCtx( BWResource::getFilename( resourceID ) );

	ModelFactory factory( resourceID, pFile );
	ModelPtr model = Moo::createModelFromFile( pFile, factory );

	if (!model)
	{
		ERROR_MSG( "Could not load model: '%s'\n", resourceID.c_str() );
		return NULL;
	}

	PostLoadCallBack::run();

	if (!(model && model->valid()))
	{
		ERROR_MSG( "Error in post load of model: '%s'\n", resourceID.c_str() );
		return NULL;
	}

	return model;
}



/**
 *	This method reads the source dye described in the given data section
 *	into a DyeSelection object.
 *
 *	@return true for success
 */
bool Model::readSourceDye( DataSectionPtr pSourceDye, DyeSelection & dsel )
{
	BW_GUARD;
	for (DataSectionIterator il = pSourceDye->begin();
		il != pSourceDye->end();
		il++)
	{
		const std::string & sName = (*il)->sectionName();
		if (sName == "matter")
			dsel.matterName_ = sName;
		else if (sName == "tint")
			dsel.tintName_ = sName;
		else	// general property
		{
			DyePropSetting	dps;

			s_propCatalogueLock_.grab();
			PropCatalogue::iterator pit =
				s_propCatalogue_.find( sName.c_str() );
			if (pit == s_propCatalogue_.end())
			{
				{
					memoryCounterSub( modelGlobl );
					memoryClaim( s_propCatalogue_ );
				}
				pit = s_propCatalogue_.insert(
					std::make_pair( sName.c_str(), Vector4() ) );
				{
					memoryCounterAdd( modelGlobl );
					memoryClaim( s_propCatalogue_ );
				}
			}

			dps.index_ = pit - s_propCatalogue_.begin();
			s_propCatalogueLock_.give();
			dps.value_ = (*il)->asVector4();
			dsel.properties_.push_back( dps );
		}
	}

	return !(dsel.matterName_.empty() || dsel.tintName_.empty());
}


/**
 *	This method loads stuff that is common to all model types but must
 *	be done after the rest of the model has been loaded - i.e. actions.
 */
void Model::postLoad( DataSectionPtr pFile )
{
	BW_GUARD;
	// load the actions
	std::vector<DataSectionPtr> vActionSects;
	pFile->openSections( "action", vActionSects );
	for (uint i = 0; i < vActionSects.size(); i++)
	{
		DataSectionPtr pActionSect = vActionSects[i];

		ModelAction * pAction = new ModelAction( pActionSect );

		if (!pAction->valid(*this))
		{
			ERROR_MSG( "Invalid Action: '%s' in model '%s'\n",
						pActionSect->readString("name", "<no name>").c_str(),
						this->resourceID().c_str());
			delete pAction;
			continue;
		}

		actionsIndex_.insert(
			std::make_pair( &pAction->name_, actions_.size() ) );
		actions_.push_back( pAction );
	}

	memoryCounterAdd( model );
	memoryClaim( this );
	memoryClaim( resourceID_ );

	memoryClaim( animations_ );
	memoryClaim( actions_ );

	memoryClaim( matters_ );
	for (uint i = 0; i < matters_.size(); i++)
	{
		Matter & m = *matters_[i];
		memoryClaim( m.tints_ );
		for (uint j = 0; j < m.tints_.size(); j++)
		{
			TintPtr t = m.tints_[j];
			memoryClaim( t->properties_ );
			memoryClaim( t->sourceDyes_ );
			for (uint k = 0; k < t->sourceDyes_.size(); k++)
			{
				DyeSelection & d = t->sourceDyes_[k];
				memoryClaim( d.matterName_ );
				memoryClaim( d.tintName_ );
				memoryClaim( d.properties_ );
			}
		}
		memoryClaim( m.replaces_ );
		memoryClaim( m.primitiveGroups_ );
	}
}


/**
 *	@todo
 */
const int Model::blendCookie()
{
	return Model::s_blendCookie_;
}


/**
 *	@todo
 */
int Model::getUnusedBlendCookie()
{
	return ((Model::s_blendCookie_ - 16) & 0x0fffffff);
}


/**
 *	@todo
 */
int Model::incrementBlendCookie()
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV(MainThreadTracker::isCurrentThreadMain())
	{
		MF_EXIT( "incrementBlendCookie called, but not from the main thread" );
	}

	Model::s_blendCookie_ = ((Model::s_blendCookie_ + 1) & 0x0fffffff);
	return Model::s_blendCookie_;
}


/**
 *	This method soaks the Model's materials in the currently emulsified dyes,
 *	or the default for each matter if it has no emulsion. It is a utility
 *	method called by models that use multiple materials to implement their dyes.
 */
void Model::soakDyes()
{
	BW_GUARD;
	for (Matters::iterator it = matters_.begin(); it != matters_.end(); it++)
	{
		(*it)->soak();
	}
}


/**
 *	This function returns true if the Model is in a valid
 *	state.
 *	This function will not raise a critical error or asserts if called but may
 *	commit ERROR_MSGs to the log.
 *
 *	@return		Returns true if the animation is in a valid state, otherwise
 *				false.
 */
bool Model::valid() const
{
	BW_GUARD;
	if (!pDefaultAnimation_)
	{
		ERROR_MSG("Model '%s' has no default pose.\n", resourceID().c_str());
		return false;
	}

	for (	AnimationsIndex::const_iterator i = animationsIndex_.begin();
			i != animationsIndex_.end();
			++i)
	{
		if (i->second < 0 || size_t(i->second) >= animations_.size())
		{
			return false;
		}

		if (i->first.empty())
		{
			return false;
		}
	}

	for (uint i=0; i < animations_.size(); i++)
	{
		if (!animations_[i]->valid())
		{
			std::string animationName = "<unknown animation>";

			for (	AnimationsIndex::const_iterator j = animationsIndex_.begin();
					j != animationsIndex_.end();
					++j)
			{
				if (i == j->second)
				{
					animationName = j->first;
					break;
				}
			}

			ERROR_MSG(	"Invalid animation '%s' in model '%s'\n",
						animationName.c_str(),
						resourceID().c_str());
			return false;
		}
	}

	for (uint i=0; i < actions_.size(); i++)
	{
		if (!actions_[i]->valid(*this))
		{
			ERROR_MSG(	"Invalid action '%s' in model '%s'\n",
						actions_[i]->name_.c_str(),
						resourceID().c_str());
			return false;
		}
	}

	return true;
}


/**
 *	This method gets the index of the input animation name in this model's
 *	table. The index is also valid for all the ancestors and descendants
 *	of this model.
 *
 *	@return the index of the animation, or -1 if not found
 */
int Model::getAnimation( const std::string & name ) const
{
	BW_GUARD;
	for (const Model * pModel = this; pModel != NULL; pModel = &*pModel->parent_)
	{
		AnimationsIndex::const_iterator found =
			pModel->animationsIndex_.find( name );
		if (found != pModel->animationsIndex_.end())
			return found->second;
	}
	return -1;
}

/**
 *	This method ticks the animation with the given index if it exists.
 */
void Model::tickAnimation( int index, float dtime, float otime, float ntime )
{
	BW_GUARD;
	if (uint(index) >= animations_.size())
		return;
	animations_[ index ]->tick( dtime, otime, ntime );
}

/**
 *	This method plays the animation with the given index. If the animation
 *	doesn't exist for this model, then the default animation is played instead.
 */
void Model::playAnimation( int index, float time, float blendRatio, int flags )
{
	BW_GUARD;
	ModelAnimationPtr & anim = (uint(index) < animations_.size()) ?
			animations_[ index ] : pDefaultAnimation_;

	if (time > anim->duration_) time = anim->looped_ ?
		fmodf( time, anim->duration_ ) : anim->duration_;
	// only not doing this in Animation::play because its virtual.

	anim->play( time, blendRatio, flags );
}


/**
 *	This method gets the animation that is most local to this model
 *	 for the given index. It does not return the default animation
 *	 if the index cannot be found - instead it returns NULL.
 */
ModelAnimation * Model::lookupLocalAnimation( int index )
{
	BW_GUARD;
	if (index >= 0 && (uint)index < animations_.size())
	{
		return animations_[ index ].getObject();
	}

	return NULL;
}


/**
 *	This method gets the pointer to the input action name in this model's
 *	table. Actions only make sense in the context of the most derived model
 *	in a SuperModel (i.e. the top lod one), so we don't have to do any
 *	tricks with indices here. Of course, the animations referenced by these
 *	actions still do this.
 *
 *	@return the action, or NULL if not found
 */
const ModelAction * Model::getAction( const std::string & name ) const
{
	BW_GUARD;
	for (const Model * pModel = this;
		pModel != NULL;
		pModel = &*pModel->parent_)
	{
		ActionsIndex::const_iterator found =
			pModel->actionsIndex_.find( &name );
		if (found != pModel->actionsIndex_.end())
			return pModel->actions_[ found->second ].getObject();
	}
	return NULL;
}





/**
 *	This method looks up the local action of the given index
 */
ModelActionsIterator Model::lookupLocalActionsBegin() const
{
	BW_GUARD;
	return ModelActionsIterator( this );
}

ModelActionsIterator Model::lookupLocalActionsEnd() const
{
	BW_GUARD;
	return ModelActionsIterator( NULL );
}




/**
 *	This static method returns the index of the property with the given
 *	name in the global property catalogue. It returns -1 if a property
 *	of that name does not exist.
 */
int Model::getPropInCatalogue( const char * name )
{
	BW_GUARD;
	SimpleMutexHolder smh( s_propCatalogueLock_ );
	PropCatalogue::iterator found = s_propCatalogue_.find( name );
	if (found == s_propCatalogue_.end()) return -1;
	return found - s_propCatalogue_.begin();
}

/**
 *	This static method looks up the name of the given property in the
 *	global property catalogue. There is no method to look up the value
 *	and return a pointer to it as that would not be thread safe.
 *	Returns NULL if the index is out of range.
 */
const char * Model::lookupPropName( int index )
{
	BW_GUARD;
	SimpleMutexHolder smh( s_propCatalogueLock_ );
	if (uint(index) >= s_propCatalogue_.size()) return NULL;
	return (s_propCatalogue_.begin() + index)->first;
	// safe to return this pointer as props are never deleted from
	// the catalogue and even if they move around in the tables or
	// vector the string memory stays put.
}


/**
 *	This method gets a (packed) index of the dye formed by the
 *	given matter and tint names. If ppTint is not NULL, and the
 *	dye is found, then the pointer to the tint in the local model
 *	is written into ppTint.
 */
ModelDye Model::getDye(	const std::string & matterName,
						const std::string & tintName,
						Tint ** ppTint )
{
	BW_GUARD;
	int matterIndex = -1;
	int tintIndex = -1;

	Matters::iterator miter = matters_.begin();
	for (; miter != matters_.end(); ++miter)
	{
		if ((*miter)->name_ == matterName)
			break;
	}

	if (miter != matters_.end())
	{
		matterIndex = miter - matters_.begin();

		Matter::Tints & tints = (*miter)->tints_;
		Matter::Tints::iterator titer = tints.begin();
		for (; titer != tints.end(); ++titer)
		{
			if ((*titer)->name_ == tintName)
				break;
		}

		if (titer != tints.end())
		{
			tintIndex = titer - tints.begin();
			if (ppTint != NULL)
				*ppTint = &*(*titer);
		}
	}

	return ModelDye( *this, matterIndex, int16(tintIndex) );
}


/**
 *	This method soaks this model in the given (packed) dye index.
 *	It is a default method for model classes that use multiple materials.
 */
void Model::soak( const ModelDye & dye )
{
	BW_GUARD;
	// look up the matter
	if (uint(dye.matterIndex()) >= matters_.size())
		return;

	// tell it to tint
	matters_[ dye.matterIndex() ]->emulsify( dye.tintIndex() );
}


/**
 *	This method looks up the local matter of the given index
 */
const Matter * Model::lookupLocalMatter( int index ) const
{
	BW_GUARD;
	if (index >= 0 && (uint)index < matters_.size())
		return matters_[ index ];
	else
		return NULL;
}


/**
 *	@todo
 */
MaterialOverride Model::overrideMaterial(	const std::string & identifier,
											Moo::EffectMaterialPtr material )
{
	BW_GUARD;
	return MaterialOverride();
}


/**
 *	@todo
 */
int Model::gatherMaterials(	const std::string & materialIdentifier,
							std::vector< Moo::Visual::PrimitiveGroup * > & primGroups,
							Moo::ConstEffectMaterialPtr * ppOriginal )
{
	BW_GUARD;
	return 0;
}


/**
 *	This is the base class method to get static lighting.
 */
ModelStaticLightingPtr Model::getStaticLighting( StaticLightValueCachePtr, const DataSectionPtr )
{
	BW_GUARD;
	return NULL;
}

/**
 *	This is the base class method to set static lighting.
 */
void Model::setStaticLighting( ModelStaticLightingPtr pLighting )
{
	BW_GUARD;
	if (pLighting) pLighting->set();
}



/**
 *	Default implementation. Returns empty node tree iterator.
 */
NodeTreeIterator Model::nodeTreeBegin() const
{
	return NodeTreeIterator( NULL, NULL, NULL );
}

/**
 *	Default implementation. Returns empty node tree iterator.
 */
NodeTreeIterator Model::nodeTreeEnd() const
{
	BW_GUARD;
	return NodeTreeIterator( NULL, NULL, NULL );
}




// model.cpp
