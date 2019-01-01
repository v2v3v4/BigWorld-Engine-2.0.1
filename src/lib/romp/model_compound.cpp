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
#include "model_compound.hpp"
#include "moo/visual_compound.hpp"

DECLARE_DEBUG_COMPONENT2( "romp", 0 );
PROFILER_DECLARE( ModelCompound_del, "ModelCompound del" );

ModelCompoundInstance::ModelCompoundInstance()
{
}

ModelCompoundInstance::~ModelCompoundInstance()
{
	// Del all our visual compound lods.
	LodHolders::iterator it = lodHolders_.begin();
	while (it != lodHolders_.end())
	{
		it->second->del();
		++it;
	}

	// If we have a model compound remove ourselves from it
	if (pModelCompound_)
	{
		pModelCompound_->del( this );
	}
}

/**
 *	This method inits the compound instance from a model compound,
 *	creating transform holders for every visual compound.
 */
bool ModelCompoundInstance::init( ModelCompoundPtr pModelCompound, 
		const Matrix& transform, uint32 batchCookie )
{
	const ModelCompound::LodCompounds& lcs = pModelCompound->lodCompounds();
	ModelCompound::LodCompounds::const_iterator it = lcs.begin();

	while (it != lcs.end())
	{
		Moo::VisualCompound* pvc = Moo::VisualCompound::get(it->second);

		if (pvc)
		{
			Moo::TransformHolder* pth = pvc->addTransform( transform, batchCookie );
			if (pth)
			{
				lodHolders_.push_back( std::make_pair( it->first, pth ) );
			}
			else
			{
				return false;
			}
			++it;
		}
		else
		{
			return false;
		}
	}
	pModelCompound_ = pModelCompound;
	pModelCompound_->add( this );
	
	position_ = transform.applyToOrigin();
	yScale_ = max( 1.f, transform.applyToUnitAxisVector(1).length() );
	return true;
}

/**
 *	This method calculates the distance used for lod calculations
 *	Scale is factored into the distance calculations
 *	@return float the distance for lod calculations
 */
float ModelCompoundInstance::lodDistance()
{
	const Matrix& mooView = Moo::rc().view();
	float distance = position_.dotProduct(
		Vector3( mooView._13, mooView._23, mooView._33 ) ) + mooView._43;

	return (distance / yScale_) * Moo::rc().zoomFactor();
}

/**
 *	This method draws the model compound at the appropriate LOD
 *	@return false if the method fails
 */
bool ModelCompoundInstance::draw()
{
	bool ret = false;
	if (pModelCompound_ && lodHolders_.size() && pModelCompound_->valid())
	{
		// Find the lod to use
		float lod = lodDistance();
		LodHolders::iterator it = lodHolders_.begin();
		bool lodFound = false;
		while (it != lodHolders_.end() && !lodFound)
		{
			if (it->first > lod || it->first == -1.f)
			{
				lodFound = true;
			}
			else
			{
				++it;
			}
		}

		if (lodFound)
		{
			if (it->second->pBatch())
			{
				it->second->draw();
				ret = true;
			}
		}
		else
		{
			ret = true;
		}
	}
	if (ret == false)
	{
		if (pModelCompound_)
			pModelCompound_->invalidate();
	}
	return ret;
}

/**
 *	Destructor
 */
ModelCompound::~ModelCompound()
{
	invalidate();
}

/**
 *	This methods adds a model compound instance
 *	@param instance the model compound instance to add
 */
void ModelCompound::add( ModelCompoundInstance* instance )
{
	SimpleMutexHolder smh(mutex_);
	instances_.push_back( instance );
}

/**
 *	This method removes a model compound instance
 *	@param instance the model compound instance to remove
 */
void ModelCompound::del( ModelCompoundInstance* instance )
{	
	BW_GUARD_PROFILER( ModelCompound_del );
	SimpleMutexHolder smh(mutex_);
	Instances::iterator it = std::find( instances_.begin(), instances_.end(), instance );
	if (it != instances_.end())
		instances_.erase(it);
}

/**
 *	This method gets a model compound instance at the transform specified
 *	@param name the name of the model the model compound is created from
 *	@param transform the transform of the model
 *	@param batchCookie the batch cookie for the batch
 *
 *	@return the compound instance, NULL if a compound is not to be added
 */
ModelCompoundInstancePtr ModelCompound::get( const std::string& name, 
	const Matrix& transform, uint32 batchCookie )
{
	Moo::VisualCompound::grabDelMutex();
	compoundsMutex_.grab();

	ModelCompoundInstance* pResult = NULL;
	
	CompoundMap::iterator it = compoundMap_.find( name );

	if (it == compoundMap_.end())
	{
		compoundsMutex_.give();
		ModelCompoundPtr pSMC = new ModelCompound;
		if (!pSMC->init(name))
		{
			pSMC = NULL;
		}
		compoundsMutex_.grab();
		it = compoundMap_.insert(std::make_pair(name, pSMC)).first;
	}

	ModelCompoundPtr pModelCompound = it->second;

	if (pModelCompound && pModelCompound->valid())
	{
		ModelCompoundInstance* pSMCI = new ModelCompoundInstance;
		if (pSMCI->init( pModelCompound, transform, batchCookie ))
		{
			pResult = pSMCI;
		}
		else
		{
			delete pSMCI;
		}
	}

	compoundsMutex_.give();
	Moo::VisualCompound::giveDelMutex();
	return pResult;
}

/**
 *	This method invalides the model compound
 */
void ModelCompound::invalidate()
{
	SimpleMutexHolder smh(mutex_);
	Instances::iterator it = instances_.begin();
	Instances::iterator end = instances_.end();
	while (it != end)
	{
		(*it)->pModelCompound_ = NULL;
		++it;
	}

	instances_.clear();

	valid_ = false;
}

/**
 *	The constructor
 */
ModelCompound::ModelCompound() :
valid_( true )
{
}

/**
 *	This method inits the model compound from a model
 *	@param modelName the name of the model to init the compound from
 *	@return false if the compound could not be created
 */
bool ModelCompound::init( const std::string& modelName )
{
	valid_ = false;

	DataSectionPtr pModelSection = BWResource::openSection( modelName );
	if (pModelSection)
	{
		std::string visualName = pModelSection->readString( "nodelessVisual" );
		if (visualName.length())
		{
			Moo::VisualCompound* pVisualCompound = Moo::VisualCompound::get(
				visualName + ".visual" );
			if (pVisualCompound)
			{
				float lodDist = pModelSection->readFloat( "extent", -1.f );
				lodCompounds_.push_back( std::make_pair( lodDist, visualName + ".visual" ) );
				std::string parentName = pModelSection->readString( "parent" );
				if (parentName.length())
				{
					valid_ = init( parentName + ".model" );
				}
				else
				{
					valid_ = true;
				}
			}
		}
	}

	return valid_;
}

ModelCompound::CompoundMap ModelCompound::compoundMap_;
SimpleMutex ModelCompound::compoundsMutex_;
