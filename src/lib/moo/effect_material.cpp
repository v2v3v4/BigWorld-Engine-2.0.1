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
#include "effect_material.hpp"
#include "managed_effect.hpp"
#include "resmgr/bwresource.hpp"
#include "effect_state_manager.hpp"

#include "resmgr/xml_section.hpp"
#include "cstdmf/profiler.hpp"
#include "visual_channels.hpp"

DECLARE_DEBUG_COMPONENT2( "Moo", 0 );

namespace Moo
{

PROFILER_DECLARE( EffectMaterial_begin, "EffectMaterial Begin" );
PROFILER_DECLARE( EffectMaterial_commit, "EffectMaterial Commit" );
PROFILER_DECLARE( EffectMaterial_end, "EffectMaterial End" );

PROFILER_DECLARE( EffectMaterial_beginPass, "EffectMaterial BeginPass" );
PROFILER_DECLARE( EffectMaterial_endPass, "EffectMaterial EndPass" );
PROFILER_DECLARE( EffectMaterial_finishEffectInits, "EffectMaterial finishEffectInits" );

PROFILER_DECLARE( EffectMaterial_hTechnique, "EffectMaterial hTechnique" );

// -----------------------------------------------------------------------------
// Section: EffectMaterial
// -----------------------------------------------------------------------------

EffectMaterialPtr EffectMaterial::s_curMaterial;

/**
 *	Constructor.
 */
EffectMaterial::EffectMaterial() :
	hOverriddenTechnique_( 0 ),
	materialKind_( 0 ),
	collisionFlags_( 0 )
#ifdef EDITOR_ENABLED
	,bspModified_( false )
#endif
{
}

/**
 *	Copy constructor.
 */
EffectMaterial::EffectMaterial( const EffectMaterial & other )
{
	*this = other;
}

/**
 *	Destructor.
 */
EffectMaterial::~EffectMaterial()
{	
}

/**
 *	This method sets up the initial constants and render states for rendering.
 *	@return true if successful.
 */
bool EffectMaterial::begin()
{
	BW_GUARD_PROFILER( EffectMaterial_begin );

	if ( s_curMaterial != NULL )
	{
		ERROR_MSG( "EffectMaterial::begin() already called.\n" );
		return false;
	}

	if ( pManagedEffect_ && !pManagedEffect_->finishInit() )
	{
		nPasses_ = 0;
		return false;
	}

	D3DXHANDLE hTechnique = this->hTechnique();

	if ( !hTechnique )
	{
		nPasses_ = 0;
		return false;
	}

	ID3DXEffect*				pEffect = pManagedEffect_->pEffect();
	Properties::const_iterator	it		= properties_.begin();
	Properties::const_iterator	end		= properties_.end();

	pEffect->SetTechnique( hTechnique );
	pManagedEffect_->setAutoConstants();

	while (it != end)
	{
		it->second->apply( pEffect, it->first );
		it++;
	}

	bool ret = SUCCEEDED(pEffect->Begin( &nPasses_, D3DXFX_DONOTSAVESTATE ));

	if (ret)
	{
		s_curMaterial = this;
	}

	return ret;
}

/**
 * This method commits state changes on the material's effect to Direct3D.
 */
bool EffectMaterial::commitChanges() const
{
	BW_GUARD_PROFILER( EffectMaterial_commit );

	bool ret = false;

	if ( pManagedEffect_ )
	{
		ret = SUCCEEDED( pManagedEffect_->pEffect()->CommitChanges() );
	}
	
	return ret;
}

/**
 *	This method cleans up after rendering with this material.
 *	@return true if successful
 */
bool EffectMaterial::end() const
{
	BW_GUARD_PROFILER( EffectMaterial_end );

	if ( s_curMaterial != this )
	{
		ERROR_MSG("EffectMaterial::end() begin/end mismatch!\n");
		return false;
	}

	s_curMaterial = NULL;

	bool ret = false;

	if ( pManagedEffect_ )
	{
		if (nPasses_ != 0)
		{
			ret = SUCCEEDED( pManagedEffect_->pEffect()->End() );
		}
	}
	return ret;
}

/**
 *	This method sets up rendering for a specific pass
 *
 *	@param pass the pass to render
 *	@return true if successful
 */
bool EffectMaterial::beginPass( uint32 pass ) const
{
	BW_GUARD_PROFILER( EffectMaterial_beginPass );
	bool ret = false;

	if ( pManagedEffect_ )
	{	
		if (nPasses_ != 0)
		{
			ret = SUCCEEDED(pManagedEffect_->pEffect()->BeginPass( 
						pass < nPasses_ ? pass : nPasses_ - 1  ) );
		}
	}
	return ret;
}

/**
 *	This method records all the states for the pass, note that the
 *	returned object is only valid until the end of the current 
 *	rendering loop.
 *
 *	@param pass the pass to record
 *	@return the recorded states
 */
StateRecorder* EffectMaterial::recordPass( uint32 pass ) const
{
	BW_GUARD;
	StateRecorder* pStateRecorder = rc().usingWrapper() ? 
		WrapperStateRecorder::get() : StateRecorder::get();

	pStateRecorder->init();
	
	if ( pManagedEffect_ )
	{
		ID3DXEffect*				pEffect		= pManagedEffect_->pEffect();
		ID3DXEffectStateManager*	pManager	= NULL;

		pEffect->GetStateManager( &pManager );
		pEffect->SetStateManager( pStateRecorder );
	
		if ( nPasses_ != 0)
		{
			pEffect->BeginPass( pass < nPasses_ ? pass : nPasses_ - 1 );
			pEffect->EndPass();
		}
		pEffect->SetStateManager( pManager );
		pManager->Release();
	}

	return pStateRecorder;

}

/**
 *	This method ends the current pass
 *
 *	@return true if successful
 */
bool EffectMaterial::endPass() const
{
	BW_GUARD_PROFILER( EffectMaterial_endPass );
	
	if ( pManagedEffect_ )
	{
		if ( nPasses_ != 0 )
		{
			pManagedEffect_->pEffect()->EndPass();	
		}
	}

	return true;
}

/**
 *	This method loads an EffectMaterial from a DataSection and sets up the tweakables.
 *
 *	@param pSection the DataSection to load the material from
 *	@param addDefault	Boolean indicating whether to add default mappings
 *                      from properties to shader parameters.
 *
 *	@return true if successful
 */
bool EffectMaterial::load( DataSectionPtr pSection, bool addDefault /*= true*/ )
{
	BW_GUARD;
	bool ret = false;
	IF_NOT_MF_ASSERT_DEV( pSection.hasObject() )
	{
		return false;
	}

	EffectPropertyMappings effectProperties;

	pManagedEffect_ = NULL;
	nPasses_		= 0;
	properties_.clear();
	
	// Do internal loading of material
	ret = this->loadInternal( pSection, effectProperties );

	if ( ret && pManagedEffect_ != NULL )
	{
		const EffectPropertyMappings& defaultProperties = pManagedEffect_->defaultProperties();
		EffectPropertyMappings::const_iterator dit = defaultProperties.begin();
		EffectPropertyMappings::const_iterator dend = defaultProperties.end();

		// Iterate over the default properties
		while (dit != dend)
		{
			EffectPropertyMappings::iterator propertyOverride = effectProperties.find( dit->first );
			
			if (propertyOverride != effectProperties.end())
			{
				// If a default property exists set it as the overridden properties parent
				propertyOverride->second->setParent( dit->second.getObject() );
			}
			else if (addDefault)
			{
				// Add the default property to this materials property list
				effectProperties.insert( *dit );
			}
			dit++;
		}
		ret = true;

		// Map the effect property names to the right d3dx handles.
		EffectPropertyMappings::iterator it = effectProperties.begin();
		EffectPropertyMappings::iterator end = effectProperties.end();
		
		while (it != end)
		{
			bool found = false;

			ID3DXEffect*	pEffect = pManagedEffect_->pEffect();
			D3DXHANDLE		h = pEffect->GetParameterByName( NULL, it->first.c_str() );

			if (h != NULL)
			{
				properties_[ h ] = it->second;
				found = true;
			}
			
			if (found == false)
			{
				if ( pManagedEffect_ )
				{
					NOTICE_MSG( 
						"EffectMaterial::load - no parameter \"%s\" found in .fx file \"%s\"\n",
						it->first.c_str(), pManagedEffect_->resourceID().c_str());
				}
			
			}
			it++;
		}
	}

	return ret;
}


/**
 *	This method saves the given material's tweakable properties
 *	to the given datasection.
 *
 *	Material saving does not support recursion / inherited properties.
 *
 *	@param	m			The material to save
 *	@param	pSection	The data section to save to.
 *
 *	@return Success or Failure.
 */
void EffectMaterial::save( DataSectionPtr pSection )
{
	ComObjectWrap<ID3DXEffect> pEffect = this->pEffect()->pEffect();
	if ( !pEffect )
		return;

    pSection->deleteSections( "property" );
    pSection->deleteSections( "fx" );

    pSection->writeString( "fx", this->pEffect()->resourceID() );

	EffectMaterial::Properties& properties = this->properties();
    EffectMaterial::Properties::iterator it = properties.begin();
    EffectMaterial::Properties::iterator end = properties.end();
    
    while ( it != end )
    {
		D3DXHANDLE hParameter = it->first;
        EffectPropertyPtr& pProperty = it->second;

        D3DXPARAMETER_DESC desc;
        HRESULT hr = pEffect->GetParameterDesc( hParameter, &desc );
        if ( SUCCEEDED(hr) )
        {
            EffectProperty* pProp = pProperty.getObject();
            //If the following assertion is hit, then
            //runtimeInitMaterialProperties() was not called before this effect
            //material was created.
            std::string name(desc.Name);
            DataSectionPtr pChild = pSection->newSection( "property" );
            pChild->setString( name );
            pProp->save( pChild );
        }
		it++;
	}

	//TODO : probably want to do save these too.
	//pSection->writeInt( "collisionFlags", this->collisionFlags() );
	//pSection->writeInt( "materialKind", this->materialKind() );
}


/*
 * This method is the internal load method from a data section.
 */
bool EffectMaterial::loadInternal(	DataSectionPtr pSection, 
									EffectPropertyMappings& outProperties )
{
	BW_GUARD;
	bool ret = true;
	
	IF_NOT_MF_ASSERT_DEV( pSection.hasObject() )
	{
		return false;
	}

	// get the identifier if we don't have one already.
	if (!identifier_.length())
		identifier_ = pSection->readString( "identifier" );

	// open the effect itself

	std::vector< std::string > fxNames;
	pSection->readStrings( "fx", fxNames );

	// complain if there are too many effects.
	if (fxNames.size() > 1)
	{
		WARNING_MSG( "EffectMaterial::loadInternal - "
					 "found multiple .fx files in %s\n", 
			pSection->sectionName().c_str() );
	}

	if ( pManagedEffect_ == NULL && fxNames.size() > 0 )
	{
		nPasses_		= 0;
		properties_.clear();

		// load the managed effect
		pManagedEffect_ = EffectManager::instance().get( fxNames[0] );
		
		if ( !pManagedEffect_ )
		{
			return false;
		}
	}
	
	// get the channel if it has been overridden in the material section
	if ( !channelOverride_ )
	{
		std::string channel = pSection->readString("channel");
		if (channel.length())
		{
			channelOverride_ = VisualChannel::get( channel );
		}
	}
	
	// load another material file if we are inheriting
	DataSectionPtr pMFMSect = pSection->openSection( "mfm" );
	if( pMFMSect )
	{
		DataSectionPtr pBaseSection = 
			BWResource::instance().openSection( pMFMSect->asString() );
		if (pBaseSection)
		{
			ret = loadInternal( pBaseSection, outProperties );
		}
	}
	
	// grab the mfm saved tweakable properties
	std::vector<DataSectionPtr> dataSections;
	pSection->openSections( "property", dataSections );
	while (dataSections.size())
	{
		DataSectionPtr pSect = dataSections.back();
		dataSections.pop_back();
		DataSectionPtr pChild = pSect->openChild( 0 );
		if (pChild)
		{
			PropertyFunctors::iterator it = 
				g_effectPropertyProcessors.find( pChild->sectionName() );
			if (it != g_effectPropertyProcessors.end())
			{
				outProperties[pSect->asString()] = it->second->create( pChild );
			}
            else
            {
                DEBUG_MSG( "Could not find property processor "
							"for mfm-specified property %s\n", 
							pChild->sectionName().c_str() );
            }
		}
	}

	// grab the editor only properties
	materialKind_ = pSection->readInt( "materialKind", materialKind_ );
	collisionFlags_ = pSection->readInt( "collisionFlags", collisionFlags_ );
	
	return ret;
}

/**
 *	This method returns the current technique handle. If a specific technique
 *	has been selected then that will take precedence over the default one.
 *	A graphics settings change can change the default technique, but a caller
 *	selected technique still overrides that.
 */
D3DXHANDLE EffectMaterial::hTechnique() const 
{ 
	BW_GUARD_PROFILER( EffectMaterial_hTechnique );

	D3DXHANDLE ret = 0;
	
	if ( pManagedEffect_ )
	{
		if ( hOverriddenTechnique_ )
		{
			ret = hOverriddenTechnique_;
		}
		else
		{
			ret = pManagedEffect_->getCurrentTechnique();
		}
	}

	return ret; 
}

/**
 *	This method sets the current technique and selects the appropriate visual 
 *	channel for the material.
 *
 *	@param hTec handle to the technique to use.
 *	@return true if successful
 */
bool EffectMaterial::hTechnique( D3DXHANDLE hTec )
{
	BW_GUARD;
	hOverriddenTechnique_ = hTec;

	bool ret = pManagedEffect_->setCurrentTechnique( hTec, true );

	return ret;
}

bool EffectMaterial::initFromEffect( const std::string& effect, 
									 const std::string& diffuseMap /* = "" */, 
									 int doubleSided /* = -1 */ )
{
	BW_GUARD;
	DataSectionPtr pSection = new XMLSection( "material" );
	if (pSection)
	{
		pSection->writeString( "fx", effect );

		if (diffuseMap != "")
		{
			DataSectionPtr pSect = pSection->newSection( "property" );
			pSect->setString( "diffuseMap" );
			pSect->writeString( "Texture", diffuseMap );
		}

		if (doubleSided != -1)
		{
			DataSectionPtr pSect = pSection->newSection( "property" );
			pSect->setString( "doubleSided" );
			pSect->writeBool( "Bool", !!doubleSided );			
		}

		return this->load( pSection );
	}

	return false;
}

/**
 *	Assignment operator.
 */
EffectMaterial & EffectMaterial::operator=( const EffectMaterial & other )
{
	BW_GUARD;
	pManagedEffect_         = other.pManagedEffect_;
	nPasses_                = other.nPasses_;
	properties_             = other.properties_;
	channelOverride_        = other.channelOverride_;
	hOverriddenTechnique_	= other.hOverriddenTechnique_;
	identifier_             = other.identifier_;
	materialKind_			= other.materialKind_;
	collisionFlags_			= other.collisionFlags_;

	return *this;
}


/**
 *	Retrieves value of the first property with the specified name.
 *	This is not the value in the DirectX effect but the  Moo::EffectProperty
 *	If the property is a integer the value is copied into result.
 *
 *	@param result The int into which the result will be placed
 *	@param name The name of the property to read.
 *	@return Returns true if the property’s value was successfully retrieved.
 */
bool EffectMaterial::boolProperty( bool & result, const std::string & name ) const
{
	BW_GUARD;
	EffectPropertyPtr effectProperty = this->getProperty( name );
	if( effectProperty )
		return effectProperty->getBool( result );
	return false;
}

/**
 *	Retrieves value of the first property with the specified name.
 *	This is not the value in the DirectX effect but the  Moo::EffectProperty
 *	If the property is a integer the value is copied into result.
 *
 *	@param result The int into which the result will be placed
 *	@param name The name of the property to read.
 *	@return Returns true if the property’s value was successfully retrieved.
 */
bool EffectMaterial::intProperty( int & result, const std::string & name ) const
{
	BW_GUARD;
	EffectPropertyPtr effectProperty = this->getProperty( name );
	if( effectProperty )
		return effectProperty->getInt( result );
	return false;
}

/**
 *	Retrieves value of the first property with the specified name.
 *	This is not the value in the DirectX effect but the  Moo::EffectProperty
 *	If the property is a float the value is copied into result.
 *
 *	@param result The float into which the result will be placed
 *	@param name The name of the property to read.
 *	@return Returns true if the property’s value was successfully retrieved.
 */
bool EffectMaterial::floatProperty( float & result, const std::string & name ) const
{
	BW_GUARD;
	EffectPropertyPtr effectProperty = this->getProperty( name );
	if( effectProperty )
		return effectProperty->getFloat( result );
	return false;
}

/**
 *	Retrieves value of the first property with the specified name.
 *	This is not the value in the DirectX effect but the  Moo::EffectProperty
 *	If the property is a vector the value is copied into result.
 *
 *	@param result The vector in which the result will be placed
 *	@param name The name of the property to read.
 *	@return Returns true if the property’s value was successfully retrieved.
 */
bool EffectMaterial::vectorProperty( Vector4 & result, const std::string & name ) const
{
	BW_GUARD;
	EffectPropertyPtr effectProperty = this->getProperty( name );
	if( effectProperty )
		return effectProperty->getVector( result );
	return false;
}

/**
 *	Retrieves the first Moo::EffectProperty with the specified name.
 *
 *	@param name The name of the property to read.
 *	@return SmartPointer to the property. NULL if not found.
 */
EffectPropertyPtr EffectMaterial::getProperty( const std::string & name )
{	
	BW_GUARD;
	if ( pManagedEffect_ )
	{
		ID3DXEffect* dxEffect = pManagedEffect_->pEffect();
		if( !dxEffect )
			return NULL;

		for( Properties::iterator iProperty = properties_.begin();
			iProperty != properties_.end(); ++iProperty )
		{
			D3DXPARAMETER_DESC description;

			if( !SUCCEEDED( dxEffect->GetParameterDesc( iProperty->first, &description ) ) )
				continue;

			if( name == description.Name )
			{
				return iProperty->second;
			}
		}
	}

	return NULL;
}

/**
 *	Retrieves the first Moo::EffectProperty with the specified name.
 *
 *	@param name The name of the property to read.
 *	@return ConstSmartPointer to the property. NULL if not found.
 */
ConstEffectPropertyPtr EffectMaterial::getProperty( const std::string & name ) const
{
	BW_GUARD;
	return const_cast<EffectMaterial*>( this )->getProperty( name );
}

/** 
 *  This function replaces an existing effect property with the one given. 
 *
 *  @param name             The name of the property to replace. 
 *  @param effectProperty   SmartPointer to the property to replace the existing. 
 *  @return                 Returns true if the replace succeeded. 
 */ 
bool EffectMaterial::replaceProperty( const std::string & name, EffectPropertyPtr effectProperty ) 
{
	BW_GUARD;
	if ( pManagedEffect_ )
	{
		ID3DXEffect* dxEffect = pManagedEffect_->pEffect();
		if( !dxEffect )
			return NULL;

		for( Properties::iterator iProperty = properties_.begin();
			iProperty != properties_.end(); ++iProperty )
		{
			D3DXPARAMETER_DESC description;

			if( !SUCCEEDED( dxEffect->GetParameterDesc( iProperty->first, &description ) ) )
				continue;

			if( name == description.Name )
			{
                iProperty->second = effectProperty; 
                return true; 
			}
		}
	}

    return false; 
} 

/** 
 * 	This method returns the physics flags for this material.
 */
WorldTriangle::Flags EffectMaterial::getFlags( int objectMaterialKind ) const
{
	return ( materialKind_ != 0 ) ?
		WorldTriangle::packFlags( collisionFlags_, materialKind_ ) :
		WorldTriangle::packFlags( collisionFlags_, objectMaterialKind );
}

/**
 * This method returns the skinned status of the current technique of the effect.
 */
bool EffectMaterial::skinned() const 
{ 
	BW_GUARD;
	return pManagedEffect_ ? 
		pManagedEffect_->skinned( hOverriddenTechnique_ ) : false; 
}

/**
 * This method returns the bumped status of the current technique of the effect.
 */
bool EffectMaterial::bumpMapped() const 
{ 
	BW_GUARD;
	return pManagedEffect_ ? 
		pManagedEffect_->bumpMapped( hOverriddenTechnique_ ) : false; 
}

/**
 * This method returns the dualUV status of the current technique of the effect.
 */
bool EffectMaterial::dualUV() const 
{ 
	BW_GUARD;
	return pManagedEffect_ ? 
		pManagedEffect_->dualUV( hOverriddenTechnique_ ) : false; 
}

/**
*	This method returns the channel for this material. If channel is overridden,
*	that is returned first. Otherwise the channel for the current technique is
*	returned.
*/
VisualChannelPtr EffectMaterial::channel() const
{
	BW_GUARD;
	VisualChannelPtr ret = NULL;

	if ( pManagedEffect_ )
	{
		ret = pManagedEffect_->getChannel( hOverriddenTechnique_ );
	}

	return ret;
}


/**
 *  This method returns true if the given property is a default property.
 */
bool EffectMaterial::isDefault( EffectPropertyPtr pProperty )
{
	BW_GUARD;
	if ( pManagedEffect_ )
	{
		const EffectPropertyMappings& defaultProperties = pManagedEffect_->defaultProperties();
		EffectPropertyMappings::const_iterator dit = defaultProperties.begin();
		EffectPropertyMappings::const_iterator dend = defaultProperties.end();
		
		while (dit != dend)
		{
			if ( dit->second == pProperty )
				return true;	
			dit++;
		}
	}

	return false;
}


/**
 *  This method finds all shared default properties in the material, and
 *  replaces them with instanced properties.  This allows editing of
 *  properties without affecting other materials that use the same effect.
 */
void EffectMaterial::replaceDefaults()
{
	BW_GUARD;
	if ( pManagedEffect_ )
	{
		std::vector< std::pair<D3DXHANDLE,EffectPropertyPtr> > props;
	
		// Iterate over properties and create property instances for them
		Properties::iterator it = properties_.begin();
		Properties::iterator end = properties_.end();
		while (it != end)
		{
			if ( isDefault( it->second ))
			{
				EffectPropertyPtr pProp = it->second->clone();
				it->second = pProp;
			}
			it++;
		}
	}
}

}

// effect_material.cpp
