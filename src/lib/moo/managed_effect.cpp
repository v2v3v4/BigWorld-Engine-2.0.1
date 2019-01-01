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
#include "managed_effect.hpp"

#include <d3dx9.h>

#include "resmgr/multi_file_system.hpp"
#include "resmgr/zip_section.hpp"

#include "texture_manager.hpp"
#include "effect_state_manager.hpp"
#include "effect_material.hpp"
#include "visual_channels.hpp"

#include "cstdmf/diary.hpp"
#ifdef EDITOR_ENABLED
#include "cstdmf/slow_task.hpp"
#endif//EDITOR_ENABLED

#ifndef CODE_INLINE
#include "managed_effect.ipp"
#endif

DECLARE_DEBUG_COMPONENT2( "Moo", 0 );

#ifdef D3DXSHADER_USE_LEGACY_D3DX9_31_DLL
	#define USE_LEGACY_D3DX9_DLL D3DXSHADER_USE_LEGACY_D3DX9_31_DLL
#else  // D3DXSHADER_USE_LEGACY_D3DX9_31_DLL
	#define USE_LEGACY_D3DX9_DLL 0
#endif // D3DXSHADER_USE_LEGACY_D3DX9_31_DLL

PROFILER_DECLARE( ME_getFirstValidTechnique,	"ME_getFirstValidTechnique" );
PROFILER_DECLARE( ME_findTechniqueInfo,			"ME_findTechniqueInfo"		);

namespace Moo
{

namespace { // anonymous

// Helper functions
static EffectMacroSetting::MacroSettingVector * s_pSettings = NULL;
EffectMacroSetting::MacroSettingVector & static_settings();


/**
 * This function dumps all techniques of an effect, listing their valid state
 * and all available passes. 
 */
void spamEffectInfo( ManagedEffectPtr effect )
{
	BW_GUARD;
	ID3DXEffect*		e = effect->pEffect();
	const char*			en= effect->resourceID().c_str();
	D3DXEFFECT_DESC		ed;

	TRACE_MSG("Effect:%s\n", en );

	if ( SUCCEEDED(e->GetDesc( &ed ) ) )
	{
		for ( uint i = 0; i < ed.Techniques; i++ )
		{
			TRACE_MSG( " Technique %d/%d:", i+1, ed.Techniques );

			D3DXHANDLE th = e->GetTechnique(i);
			if ( th )
			{
				D3DXTECHNIQUE_DESC td;

				if ( SUCCEEDED( e->GetTechniqueDesc( th, &td )) )
				{
					bool v = SUCCEEDED( e->ValidateTechnique( th ) );

					TRACE_MSG( "%s (%s)\n", td.Name,
						v ? "valid" : "invalid" );

					for ( uint j = 0; j < td.Passes; j++ )
					{
						TRACE_MSG("  Pass %d/%d:", j+1, td.Passes );

						D3DXHANDLE ph = e->GetPass( th, j );
						if ( ph )
						{
							D3DXPASS_DESC pd;

							if ( SUCCEEDED( e->GetPassDesc( ph, &pd )))
							{
								TRACE_MSG("%s\n", pd.Name );
							}
						}
						else
						{
							TRACE_MSG("Can't get pass handle.\n");
						}
					}
				}
				else
				{
					TRACE_MSG( "Can't get technique description.\n" );
				}
			}
			else
			{
				TRACE_MSG( "Can't get technique handle.\n" );
			}
		}
	}
	else
	{
		TRACE_MSG("Can't get effect description.\n" );
	}
}

} // namespace anonymous


/**
 *	Vector4 effect property wrapper.
 */
class Vector4Property : public EffectProperty
{
public:
	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty )
	{
		return SUCCEEDED( pEffect->SetVector( hProperty, &value_ ) );
	}
	bool be( const Vector4 & v ) { value_ = v; return true; }

	bool getVector( Vector4 & v ) const { v = value_; return true; };

	void value( const Vector4& value ) { value_ = value; }
	const Vector4& value() const { return value_; }
	virtual void save( DataSectionPtr pDS )
	{
		pDS->writeVector4( "Vector4", value_ );
	}
	EffectProperty* clone() const
	{
		Vector4Property* pClone = new Vector4Property;
		pClone->value_ = value_;
		return pClone;
	}
protected:
	Vector4 value_;
};

/**
 *	Vector4 property functor.
 */
class Vector4PropertyFunctor : public EffectPropertyFunctor
{
public:
	virtual EffectPropertyPtr create( DataSectionPtr pSection )
	{
		Vector4Property* prop = new Vector4Property;
		prop->value( pSection->asVector4() );
		return prop;
	}
	virtual EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		Vector4Property* prop = new Vector4Property;
		Vector4 v;
        pEffect->GetVector( hProperty, &v );
		prop->value( v );
		return prop;
	}
	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		return (propertyDesc->Class == D3DXPC_VECTOR &&
			propertyDesc->Type == D3DXPT_FLOAT);
	}	
private:
};


/**
 *	Matrix effect property wrapper.
 */
class MatrixProperty : public EffectProperty, public Aligned
{
public:
	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty )
	{
		return SUCCEEDED( pEffect->SetMatrix( hProperty, &value_ ) );
	}
	bool be( const Vector4 & v )
		{ value_.translation( Vector3( v.x, v.y, v.z ) ); /*hmmm*/ return true; }
	bool be( const Matrix & m )
		{ value_ = m; return true; }

	bool getMatrix( Matrix & m ) const { m = value_; return true; };

	void value( Matrix& value ) { value_ = value; };
	const Matrix& value() const { return value_; }
	virtual void save( DataSectionPtr pDS )
	{
		pDS->writeString( "Matrix", "NOT YET IMPLEMENTED" );
	}
	EffectProperty* clone() const
	{
		MatrixProperty* pClone = new MatrixProperty;
		pClone->value_ = value_;
		return pClone;
	}
protected:
	Matrix value_;
};


/**
 *	Matrix property functor.
 */
class MatrixPropertyFunctor : public EffectPropertyFunctor
{
public:
	virtual EffectPropertyPtr create( DataSectionPtr pSection )
	{
		MatrixProperty* prop = new MatrixProperty;
		Matrix m;
		m.row(0, pSection->readVector4( "row0" ) );
		m.row(1, pSection->readVector4( "row1" ) );
		m.row(2, pSection->readVector4( "row2" ) );
		m.row(3, pSection->readVector4( "row3" ) );
		prop->value( m );
		return prop;
	}
	virtual EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		MatrixProperty* prop = new MatrixProperty;
		Matrix m;
        pEffect->GetMatrix( hProperty, &m );
		prop->value( m );
		return prop;
	}
	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		return ((propertyDesc->Class == D3DXPC_MATRIX_ROWS ||
            propertyDesc->Class == D3DXPC_MATRIX_COLUMNS) &&
			propertyDesc->Type == D3DXPT_FLOAT);
	}
private:
};

/**
 *	Texture effect property wrapper.
 */
class TextureProperty : public EffectProperty
{
public:
	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty )
	{
		if (!value_.hasObject())
			return SUCCEEDED( pEffect->SetTexture( hProperty, NULL ) );
		return SUCCEEDED( pEffect->SetTexture( hProperty, value_->pTexture() ) );
	}

	bool be( const BaseTexturePtr pTex ) { value_ = pTex; return true; }
	bool be( const std::string& s )		 { value_ = TextureManager::instance()->get(s); return true; }

	void value( BaseTexturePtr value ) { value_ = value; };
	const BaseTexturePtr value() const { return value_; }

	bool getResourceID( std::string & s ) const { s = value_ ? value_->resourceID() : ""; return true; };
	virtual void save( DataSectionPtr pDS )
	{
		std::string s;
		if ( getResourceID(s) )
		{
			pDS->writeString( "Texture", s );
		}
	}
	EffectProperty* clone() const
	{
		TextureProperty* pClone = new TextureProperty;
		pClone->value_ = value_;
		return pClone;
	}
protected:
	BaseTexturePtr value_;
};

/**
 *	Texture property functor.
 */
class TexturePropertyFunctor : public EffectPropertyFunctor
{
public:
	virtual EffectPropertyPtr create( DataSectionPtr pSection )
	{
		TextureProperty* prop = new TextureProperty;
		BaseTexturePtr pTexture = TextureManager::instance()->get( pSection->asString() );
		prop->value( pTexture );
		return prop;
	}
	virtual EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		TextureProperty* prop = new TextureProperty;
		return prop;
	}
	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		return (propertyDesc->Class == D3DXPC_OBJECT &&
			(propertyDesc->Type == D3DXPT_TEXTURE ||
			propertyDesc->Type == D3DXPT_TEXTURE1D ||
			propertyDesc->Type == D3DXPT_TEXTURE2D ||
			propertyDesc->Type == D3DXPT_TEXTURE3D ||
			propertyDesc->Type == D3DXPT_TEXTURECUBE ));
	}
private:
};


/**
 *	Float effect property wrapper.
 */
class FloatProperty : public EffectProperty
{
public:
	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty )
	{
		return SUCCEEDED( pEffect->SetFloat( hProperty, value_ ) );
	}
	bool be( const Vector4 & v ) { value_ = v.x; return true; }
	bool be( const float & f ) { value_ = f; return true; }

	bool getFloat( float & f ) const { f = value_; return true; };

	void value( float value ) { value_ = value; };
	float value() const { return value_; }
	virtual void save( DataSectionPtr pDS )
	{
		pDS->writeFloat( "Float", value_ );
	}
	EffectProperty* clone() const
	{
		FloatProperty* pClone = new FloatProperty;
		pClone->value_ = value_;
		return pClone;
	}
protected:
	float value_;
};


/**
 * Float property functor
 */
class FloatPropertyFunctor : public EffectPropertyFunctor
{
public:
	virtual EffectPropertyPtr create( DataSectionPtr pSection )
	{
		FloatProperty* prop = new FloatProperty;
		prop->value( pSection->asFloat() );
		return prop;
	}
	virtual EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		FloatProperty* prop = new FloatProperty;
		float v;
        pEffect->GetFloat( hProperty, &v );
		prop->value( v );
		return prop;
	}
	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		return (propertyDesc->Class == D3DXPC_SCALAR &&
			propertyDesc->Type == D3DXPT_FLOAT);
	}
private:
};

/**
 * Bool effect property wrapper.
 */
class BoolProperty : public EffectProperty
{
public:
	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty )
	{
		return SUCCEEDED( pEffect->SetBool( hProperty, value_ ) );
	}
	bool be( const Vector4 & v ) { value_ = (v.x != 0); return true; }
	bool be( const bool & b ) { value_ = b; return true; }

	bool getBool( bool & b ) const { b = ( value_ != 0 ); return true; };

	void value( BOOL value ) { value_ = value; };
	BOOL value() const { return value_; }
	virtual void save( DataSectionPtr pDS )
	{
		pDS->writeBool( "Bool", value_ != 0 ? true : false );
	}
	EffectProperty* clone() const
	{
		BoolProperty* pClone = new BoolProperty;
		pClone->value_ = value_;
		return pClone;
	}
protected:
	BOOL value_;
};

/**
 *	Bool property functor.
 */
class BoolPropertyFunctor : public EffectPropertyFunctor
{
public:
	virtual EffectPropertyPtr create( DataSectionPtr pSection )
	{
		BoolProperty* prop = new BoolProperty;
		prop->value( pSection->asBool() );
		return prop;
	}
	virtual EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		BoolProperty* prop = new BoolProperty;
		BOOL v;
        pEffect->GetBool( hProperty, &v );
		prop->value( v );
		return prop;
	}
	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		return (propertyDesc->Class == D3DXPC_SCALAR &&
			propertyDesc->Type == D3DXPT_BOOL);
	}
private:
};


/**
 *	Int effect property wrapper.
 */
class IntProperty : public EffectProperty
{
public:
	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty )
	{
		return SUCCEEDED( pEffect->SetInt( hProperty, value_ ) );
	}
	bool be( const Vector4 & v ) { value_ = int(v.x); return true; }
	bool be( const int & i ) { value_ = i; return true; }

	bool getInt( int & i ) const { i = value_; return true; };

	void value( int value ) { value_ = value; };
	int value() const { return value_; }
	virtual void save( DataSectionPtr pDS )
	{
		pDS->writeInt( "Int", value_ );
	}
	EffectProperty* clone() const
	{
		IntProperty* pClone = new IntProperty;
		pClone->value_ = value_;
		return pClone;
	}
protected:
	int value_;
};


/**
 *	Int property functor.
 */
class IntPropertyFunctor : public EffectPropertyFunctor
{
public:
	virtual EffectPropertyPtr create( DataSectionPtr pSection )
	{
		IntProperty* prop = new IntProperty;
		prop->value( pSection->asInt() );
		return prop;
	}
	virtual EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		IntProperty* prop = new IntProperty;
		int v;
        pEffect->GetInt( hProperty, &v );
		prop->value( v );
		return prop;
	}
	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		return (propertyDesc->Class == D3DXPC_SCALAR &&
			propertyDesc->Type == D3DXPT_INT );
	}
private:
};


PropertyFunctors g_effectPropertyProcessors;

bool setupTheseThings()
{
	BW_GUARD;	
#define EP(x) g_effectPropertyProcessors[#x] = new x##PropertyFunctor
	EP(Vector4);
	EP(Matrix);
	EP(Float);
	EP(Bool);
	EP(Texture);
	EP(Int);

	return true;
}

static bool blah = setupTheseThings();

/*
 *	Helper function to see if a parameter is artist editable
 */
static bool artistEditable( ID3DXEffect* pEffect, D3DXHANDLE hProperty )
{
	BW_GUARD;
	BOOL artistEditable = FALSE;
	D3DXHANDLE hAnnot = pEffect->GetAnnotationByName( hProperty, "artistEditable" );
	if (!hAnnot)
		hAnnot = pEffect->GetAnnotationByName( hProperty, "worldBuilderEditable" );
	if (hAnnot)
		pEffect->GetBool( hAnnot, &artistEditable );	

	hAnnot = pEffect->GetAnnotationByName( hProperty, "UIName" );
	return (artistEditable == TRUE) || (hAnnot != 0);
}


// -----------------------------------------------------------------------------
// Section: ManagedEffect
// -----------------------------------------------------------------------------

TechniqueInfo::TechniqueInfo()
	:	handle_( NULL ),
	settingLabel_( "Invalid Technique" ),
	settingDesc_( "Invalid Technique" ),
	psVersion_( -1 ),
	supported_( false ),
	skinned_( false ),
	bumpMapped_( false ),
	dualUV_( false )
{

}

/**
 *	Constructor.
 */
ManagedEffect::ManagedEffect():
	hCurrentTechnique_( 0 ),
	techniqueExplicitlySet_(false),
	initComplete_(false),
	settingsListenerId_(-1),
	resourceID_( "Not loaded" ),
	settingName_(),
	settingEntry_(0),
	validated_( false ),
	settingsAdded_(false),
	firstValidTechniqueIndex_( (uint32)-1 )
{
	BW_GUARD;
}

/**
 *	Destructor.
 */
ManagedEffect::~ManagedEffect()
{
	BW_GUARD;
	if ( EffectManager::pInstance() )
	{
		EffectManager::pInstance()->deleteEffect( this );
	}

	// unregister as feature listener from all effects flagged as engine features
	if (graphicsSettingEntry()	!= 0 && 
		settingsListenerId_		!= -1)
	{
		graphicsSettingEntry()->delListener( settingsListenerId_ );
	}
}

/**	
 *	This method sets the automatic constants 
 */
void ManagedEffect::setAutoConstants()
{
	BW_GUARD;
	MappedConstants::iterator it = mappedConstants_.begin();
	MappedConstants::iterator end = mappedConstants_.end();
	while (it != end)
	{
		EffectConstantValue* pValue = (*it->second).getObject();
		if (pValue)
			(*pValue)( pEffect_.pComObject(), it->first );
		it++;
	}
}

void ManagedEffect::recordAutoConstants( RecordedEffectConstants& recordedList )
{
	BW_GUARD;
	MappedConstants::iterator it = mappedConstants_.begin();
	MappedConstants::iterator end = mappedConstants_.end();
	while (it != end)
	{
		EffectConstantValue* pValue = (*it->second).getObject();
		if (pValue)
		{
			RecordedEffectConstant* pRecorded = 
				pValue->record( pEffect_.pComObject(), it->first );
			if (pRecorded)
				recordedList.push_back( pRecorded );
		}
		it++;
	}
}

/**
 *	Returns the graphics setting entry for this effect, if it has 
 *	been registered as such. Otherwise, returns an empty pointer.
 */
EffectTechniqueSetting * ManagedEffect::graphicsSettingEntry()
{
	BW_GUARD;
	return settingEntry_.getObject(); 
}

/**
 *	Retrieves the maximum pixel shader version used by 
 *	all passes in the provided technique.
 *
 *	@param	hTechnique	technique to be queried.
 *	@return				major shader version number.
 */
int ManagedEffect::maxPSVersion(D3DXHANDLE hTechnique) const
{
	BW_GUARD;
	return ManagedEffect::maxPSVersion(pEffect_.pComObject(), hTechnique);
}

/**
 *	Retrieves the maximum pixel shader version used by all passes 
 *	in the provided technique in the given effect object. (static)
 *
 *	@param	d3dxeffect	ID3DEffect object which contains the technique.
 *	@param	hTechnique	technique to be queried.
 *	@return				major shader version number.
 */
int ManagedEffect::maxPSVersion(
	ID3DXEffect * d3dxeffect, 
	D3DXHANDLE    hTechnique)
{
	BW_GUARD;
	int maxPSVersion = 10;	
	D3DXTECHNIQUE_DESC techniqueDesc;
	if (d3dxeffect->GetTechniqueDesc(hTechnique, &techniqueDesc) == D3D_OK)
	{
		maxPSVersion = 0;
		for (uint i=0; i<techniqueDesc.Passes; ++i)
		{
			D3DXPASS_DESC passDesc;
			D3DXHANDLE hPass = d3dxeffect->GetPass(hTechnique, i);
			if (d3dxeffect->GetPassDesc(hPass, &passDesc) == D3D_OK)
			{
				const DWORD * psFunction = passDesc.pPixelShaderFunction;
				int psVersion = (D3DXGetShaderVersion(psFunction) & 0xff00) >> 8;
				maxPSVersion  = std::max(maxPSVersion, psVersion);
			}
		}
	}
	return maxPSVersion;
}

/**
 * This method attempts to validate all techniques for this effect and store
 * their handles. If the techniques are already valid the method returns
 * true immediately.
 *
 * If the validation succeeds for all handles it returns true. If this fails
 * (possibly because conflicting D3D calls were being made in loading thread)
 * it does not block, but returns false immediately. The caller must handle
 * this case.
 * 
 * This method should only be called from the main (rendering) thread.
 */
bool ManagedEffect::validateAllTechniques()
{
	BW_GUARD;
	// Ensure we're only in the main thread.
	IF_NOT_MF_ASSERT_DEV( g_renderThread == true )
	{
		return false;
	}

	// If we're already done, return success.
	if ( validated_ )
		return true;

	// Not validated, so try it
	if ( Moo::rc().getD3DXCreateMutex().grabTry() )
	{
		// Get effect description
		D3DXEFFECT_DESC effectDesc;
		pEffect_->GetDesc( &effectDesc );
	
		// Reserve enough assuming all techniques are supported.
		techniques_.reserve( effectDesc.Techniques );

		if (Moo::rc().mixedVertexProcessing())
			Moo::rc().device()->SetSoftwareVertexProcessing( TRUE );

		// For each technique in the effect, add it if returns a valid handle.
		// Also record maximum pixel shader version and "validated" state.
		for (uint32 i = 0; i < effectDesc.Techniques; ++i )
		{
			// get handle
			TechniqueInfo technique;
			technique.handle_=	pEffect_->GetTechnique(i);
		
			if ( technique.handle_ != 0 )
			{
				// get name
				D3DXTECHNIQUE_DESC techniqueDesc;
				pEffect_->GetTechniqueDesc( technique.handle_ , &techniqueDesc );
				technique.name_	= techniqueDesc.Name;

				// get channel
				D3DXHANDLE hChannelName	= 
					pEffect_->GetAnnotationByName( technique.handle_, "channel" );
		
				if ( hChannelName )
				{
					const char* channelName = NULL;
					pEffect_->GetString( hChannelName, &channelName );
					if ( channelName )
					{
						technique.channel_ = VisualChannel::get( channelName );
					}
				}

				technique.psVersion_ = this->maxPSVersion( technique.handle_ );

				HRESULT validateHR = pEffect_->ValidateTechnique( technique.handle_ );
				if (SUCCEEDED( validateHR ))
				{
					technique.supported_ = true;
				}
				else
				{
					bool deviceReset = false;
					bool deviceOK = Moo::rc().checkDevice( &deviceReset);
					if (!deviceOK || deviceReset)
					{
						// We lost the device, so we'll validate again 
						// once the device has been restored
						techniques_.clear();
						Moo::rc().getD3DXCreateMutex().give();
						return false;
					}
					technique.supported_ = false;
				}

				// get skinned annotation
				D3DXHANDLE h = pEffect_->GetAnnotationByName( technique.handle_, 
															"skinned" );

				if (h)
				{
					BOOL b;
					if (SUCCEEDED(pEffect_->GetBool( h, &b )))
					{
						technique.skinned_ = b == TRUE;
					}
				}

				// get bumped annotation
				 h = pEffect_->GetAnnotationByName( technique.handle_, 
															"bumpMapped" );
				if (h)
				{
					BOOL b;
					if (SUCCEEDED(pEffect_->GetBool( h, &b )))
					{
						technique.bumpMapped_ = b == TRUE;
					}
				}
				
				// get dualUV annotation
				 h = pEffect_->GetAnnotationByName( technique.handle_, 
															"dualUV" );
				if (h)
				{
					BOOL b;
					if (SUCCEEDED(pEffect_->GetBool( h, &b )))
					{
						technique.dualUV_ = b == TRUE;
					}
				}
				
				techniques_.push_back( technique );
			}
		}

		if (Moo::rc().mixedVertexProcessing())
			Moo::rc().device()->SetSoftwareVertexProcessing( FALSE );

		// flag done, then return mutex.
		validated_ = true;
		Moo::rc().getD3DXCreateMutex().give();
	}

	// Return valid state
	return validated_;
}

bool ManagedEffect::validateShaderVersion(
	ID3DXEffect * d3dxeffect, 
	D3DXHANDLE    hTechnique) const
{
	int maxPSVersion = this->maxPSVersion(hTechnique);
	return maxPSVersion <= EffectManager::instance().PSVersionCap();
}


/**
 *	This method gets the first valid technique, and caches it for
 *	future calls to this method.  It may fail silently the first
 *	few times; it checks the compileValidateMutex and if somebody
 *	else has grabbed it, we simply return false and hope the caller
 *	tries again later.
 */
bool ManagedEffect::getFirstValidTechnique( D3DXHANDLE & hT )
{
	BW_GUARD_PROFILER( ME_getFirstValidTechnique );

	// Use cached technique index if available.
	if ( firstValidTechniqueIndex_ != (uint32)-1 )
	{
		hT = techniques_[firstValidTechniqueIndex_].handle_;
		return true;
	}

	// Ensure all techniques are validated, this may fail but will not block
	// this thread.
	if ( !validateAllTechniques() )
		return false;

	// Search for the first supported (valid) technique. If the pixel shader
	// is not suitable we can still use a support technique as a fallback.
	bool	found	= false;
	uint32	i		= 0;
	uint32	fallback= 0;

	for ( ; i < techniques_.size(); i++ )
	{
		if ( techniques_[i].supported_ == true )
		{
			if ( validateShaderVersion( &*pEffect_, techniques_[i].handle_ ) )
			{
				found = true;
				break;
			}
			else
			{
				// remember this one - if nothing is supported this is fallback
				fallback = i;				
			}
		}
	}

	// Output an error if nothing was suitable
	if ( !found )
	{
		ERROR_MSG(	"Could not find a technique matching "
			"shader version cap ( %s )\n",
			resourceID_.c_str() );
	
		// use the fallback index, not the last thing we looked at.
		i = fallback;
	}
	
	// return good handle, and remember index for next time
	hT							= techniques_[i].handle_;
	firstValidTechniqueIndex_	= i;

	// Return success state
	return found;
};

/**
 *	Generate the array of preprocessor definitions.
 */
D3DXMACRO* ManagedEffect::generatePreprocessors()
{
	BW_GUARD;
	D3DXMACRO * preProcess = NULL;

	// sets up macro definitions
	const EffectManager::StringStringMap & macros = EffectManager::instance().macros();
	preProcess = new D3DXMACRO[macros.size()+2];

	int idx = 0;	
	EffectManager::StringStringMap::const_iterator macroIt  = macros.begin();
	EffectManager::StringStringMap::const_iterator macroEnd = macros.end();
	while (macroIt != macroEnd)
	{	
		preProcess[idx].Name       = macroIt->first.c_str();
		preProcess[idx].Definition = macroIt->second.c_str();
		++macroIt;
		++idx;
	}
	preProcess[idx].Name         = "IN_GAME";
	preProcess[idx].Definition   = "true";
	preProcess[idx+1].Name       = NULL;
	preProcess[idx+1].Definition = NULL;

	return preProcess;
}

/**
 *	This method loads the managed effect into D3D from the passed binary section
 *	@param bin the binary data for the effect.
  *	@param preProcessDefinition preprocessor definitions.
 *	@return true if successful
 */
bool ManagedEffect::cache( BinaryPtr bin, D3DXMACRO * preProcessDefinition )
{
	BW_GUARD;
	#define SAFE_RELEASE(pointer)\
		if (pointer != NULL)     \
		{                        \
			pointer->Release();  \
			pointer = NULL;      \
		}

	ID3DXEffect* pEffect = NULL;
	ID3DXBuffer* pBuffer = NULL;
	D3DXMACRO * preProcess = NULL;

	if (preProcessDefinition == NULL)
		preProcess = preProcessDefinition = generatePreprocessors();

	// create the effect
	HRESULT hr = E_FAIL;
	if ( bin )
	{		
#ifndef EDITOR_ENABLED
		//This is so we don't create effects while other d3dx functions
		//are creating other resources.
		SimpleMutexHolder smh2( rc().getD3DXCreateMutex() );
#endif
		hr = D3DXCreateEffect( rc().device(), bin->cdata(), bin->len(), 
			preProcessDefinition,
			EffectManager::instance().pEffectIncludes(), 
			USE_LEGACY_D3DX9_DLL, NULL, &pEffect, &pBuffer );	
	}

	if (preProcess)
	{
		delete [] preProcess;
		preProcess = NULL;
	}

	SAFE_RELEASE(pBuffer);

	// Set up the effect and our handled constants.
	if ( bin && SUCCEEDED(hr) )
	{
		// Associate our state manager with the effect
		pEffect->SetStateManager( EffectManager::instance().pStateManager() );
		D3DXEFFECT_DESC effectDesc;
        pEffect->GetDesc( &effectDesc );
		// Iterate over the effect parameters.
		defaultProperties_.clear();
		mappedConstants_.clear();
		for (uint32 i = 0; i < effectDesc.Parameters; i++)
		{
			D3DXHANDLE param = pEffect->GetParameter( NULL, i );
			D3DXPARAMETER_DESC paramDesc;
			pEffect->GetParameterDesc( param, &paramDesc );
			if (artistEditable( pEffect, param ))
			{
				bool found = false;
				// if the parameter is artist editable find the right effectproperty 
				// processor and create the default property for it
				PropertyFunctors::iterator it = g_effectPropertyProcessors.begin();
				PropertyFunctors::iterator end = g_effectPropertyProcessors.end();
				while (it != end)
				{
					if (it->second->check( &paramDesc ))
					{
						defaultProperties_[ paramDesc.Name ] = it->second->create( param, pEffect );
						found = true;
						it = end;
					}
					else
					{
						it++;
					}
				}

				if ( !found )
				{
					ERROR_MSG( "Could not find property processor for %s\n", paramDesc.Name );
				}
			}
			else if (paramDesc.Semantic )
			{
				// If the parameter has a Semantic, set it up as an automatic constant.
				mappedConstants_.push_back( std::make_pair( param, EffectConstantValue::get( paramDesc.Semantic ) ) );
			}
		}

		pEffect_ = Moo::rc().usingWrapper() ? DX::createEffectWrapper( pEffect, resourceID_.c_str() ) : pEffect;
		pEffect_->Release();
		pEffect = NULL;

		EffectManager::instance().addListener( this );

		return true;
	}
	else
	{
		if (bin)
			ERROR_MSG("ManagedEffect::cache - Error creating the effect %s : %s\n", resourceID_.c_str(), DX::errorAsString( hr ).c_str() );	
		else
			ERROR_MSG("ManagedEffect::cache - Error creating the effect %s\n", resourceID_.c_str());
	}
	return false;

	#undef SAFE_RELEASE
}


BinaryPtr ManagedEffect::compile( const std::string& resName, D3DXMACRO * preProcessDefinition, bool force, std::string* outResult )
{
	BW_GUARD;
	#define SAFE_RELEASE(pointer)\
		if (pointer != NULL)     \
		{                        \
			pointer->Release();  \
			pointer = NULL;      \
		}

	ID3DXBuffer* pBuffer = NULL;
	D3DXMACRO * preProcess = NULL;

	if (preProcessDefinition == NULL)
		preProcess = preProcessDefinition = generatePreprocessors();

	// use the current fxoInfix to compile the set of 
	// fx using the currently active macro definitions
	std::string effectPath = BWResource::getFilePath( resName );
	EffectManager::instance().pEffectIncludes()->currentPath( effectPath );

	std::string fxoInfix   = EffectManager::instance().fxoInfix();
	std::string fileName   = BWResource::removeExtension(resName);
	std::string objectName = !fxoInfix.empty() 
		? fileName + "." + fxoInfix + ".fxo"
		: fileName + ".fxo";

	EffectManager::instance().pEffectIncludes()->resetDependencies();

	// Check if the file or any of its dependents have been modified.
	MD5::Digest resDigest;
	bool recompile = true;
	
	if (force)
	{
		if (!EffectManager::instance().hashResource( resName, resDigest ))
		{
			DEBUG_MSG( "EffectManager::compile: failed to hash '%s'.\n", resName.c_str() );
			return NULL;
		}
	}
	else
	{
		recompile = EffectManager::instance().checkModified( objectName, resName, &resDigest );
	}
	

	BinaryPtr bin = 0;
	if (recompile)
	{
		DEBUG_MSG( 
			"ManagedEffect::compiling %s (%s)\n", 
			resName.c_str(), objectName.c_str() );

		ID3DXEffectCompiler* pCompiler = NULL;
		bin = BWResource::instance().fileSystem()->readFile( resName );
		HRESULT hr;
		if ( bin )
		{
			hr = D3DXCreateEffectCompiler( bin->cdata(), bin->len(),
				preProcessDefinition,
				EffectManager::instance().pEffectIncludes(), 
				USE_LEGACY_D3DX9_DLL, &pCompiler,
				&pBuffer );
		}
		if ( !bin || FAILED(hr))
		{
			if (preProcess)
			{
				delete [] preProcess;
				preProcess = NULL;
			}
		
			ERROR_MSG("ManagedEffect::compile - Unable to create effect compiler for %s: %s\n",
									resName.c_str(),
									pBuffer?pBuffer->GetBufferPointer():"" );

			if (outResult)
			{
				*outResult = pBuffer ? (char*)pBuffer->GetBufferPointer() : "Unable to create effect compiler.";
			}

			SAFE_RELEASE(pCompiler);
			SAFE_RELEASE(pBuffer);
			return NULL;
		}

		SAFE_RELEASE(pBuffer);

		IF_NOT_MF_ASSERT_DEV( pCompiler != NULL )
		{
			return NULL;
		}

		ID3DXBuffer* pEffectBuffer = NULL;
		{
#ifdef EDITOR_ENABLED
			SlowTask st;
#endif//EDITOR_ENABLED
			hr = pCompiler->CompileEffect( 0, &pEffectBuffer, &pBuffer );
		}
		SAFE_RELEASE(pCompiler);
		if (FAILED(hr))
		{
			if (preProcess)
			{
				delete [] preProcess;
				preProcess = NULL;
			}

			ERROR_MSG("ManagedEffect::compile - Unable to compile effect %s\n%s",
									resName.c_str(), 
									pBuffer?pBuffer->GetBufferPointer():"");

			if (outResult)
			{
				*outResult = pBuffer? (char*)pBuffer->GetBufferPointer():"Unable to compile effect.";
			}

			SAFE_RELEASE(pBuffer);
			return NULL;
		}
		else
		{
			if (outResult)
			{
				*outResult = "Compilation successful.";
			}
		}

		SAFE_RELEASE(pBuffer);

		bin = new BinaryBlock( pEffectBuffer->GetBufferPointer(),
			pEffectBuffer->GetBufferSize(), "BinaryBlock/ManagedEffect" );

		BWResource::instance().fileSystem()->eraseFileOrDirectory(
			objectName );

		// create the new section
		size_t lastSep = objectName.find_last_of('/');
		std::string parentName = objectName.substr(0, lastSep);
		DataSectionPtr parentSection = BWResource::openSection( parentName, true, ZipSection::creator() );
		IF_NOT_MF_ASSERT_DEV(parentSection)
		{
			return NULL;
		}

		std::string tagName = objectName.substr(lastSep + 1);

		// make it
		DataSectionPtr pSection = parentSection->openSection( tagName, true, ZipSection::creator() );
		pSection->setParent( parentSection );
		pSection = pSection->convertToZip( "", parentSection );

		IF_NOT_MF_ASSERT_DEV( pSection )
		{
			return NULL;
		}
		pSection->delChildren();
		DataSectionPtr effectSection = pSection->openSection( "effect", true );
		effectSection->setParent(pSection);
		
		DataSectionPtr hashSection = pSection->openSection( "hash", true );
		hashSection->setParent(pSection);

		std::string quotedResDigest = resDigest.quote();

		BinaryPtr resDigestBinaryBlock = 
			new BinaryBlock( quotedResDigest.data(), quotedResDigest.length(), "BinaryBlock/ManagedEffect" );
		
		pSection->writeBinary( "hash", resDigestBinaryBlock );

		bool warn = true;

		if ( pSection->writeBinary( "effect", bin ) )
		{
			// Write the dependency list			
			std::list<std::string>::iterator it;
			std::list<std::string> &src =
				EffectManager::instance().pEffectIncludes()->dependencies();
			src.unique(); // remove possible duplicates..

			for(it = src.begin(); it != src.end(); it++)
			{
				DataSectionPtr dependsSec = pSection->newSection("depends");
				DataSectionPtr dependsNameSec = dependsSec->newSection("name");

				std::string str((*it));

				BinaryPtr binaryBlockString = 
					new BinaryBlock( str.data(), str.size(), "BinaryBlock/ManagedEffect" );
				dependsNameSec->setBinary( binaryBlockString );


				// resolve the include and store its hash.
				std::string pathname = EffectManager::instance().resolveInclude( str );
				
				MD5::Digest depDigest;
				bool hashed = EffectManager::instance().hashResource( pathname, depDigest );
				IF_NOT_MF_ASSERT_DEV( hashed == true && "Failed to hash shader dependancy (should exist, since we just compiled it)." )
				{
					hashSection->setParent( NULL );
					effectSection->setParent( NULL );
					return NULL;
				}

				std::string quotedDepDigest = depDigest.quote();

				DataSectionPtr hashSection = dependsSec->newSection( "hash" );				
				BinaryPtr depDigestBinaryBlock = 
					new BinaryBlock( quotedDepDigest.data(), quotedDepDigest.length(), "BinaryBlock/ManagedEffect" );
				
				hashSection->setBinary( depDigestBinaryBlock );
			}

			// Now actually save...
			if ( !pSection->save() )
			{
				// It failed, but it might be because it's running from Zip(s)
				// file(s). If so, there should be a writable, cache directory
				// in the search paths, so try to build the folder structure in
				// it and try to save again.
				BWResource::ensurePathExists( effectPath );	
				warn = !pSection->save();
			}
			else
			{
				warn = false;
			}
		}

		if (warn)
		{
			std::string msg = "Could not save recompiled " + objectName + ".";
			WARNING_MSG( "%s\n", msg.c_str() );

			if (outResult)
			{
				*outResult = msg;
			}
		}

		hashSection->setParent( NULL );
		effectSection->setParent( NULL );
		effectSection = NULL;
		hashSection = NULL;
		pSection = NULL;
		parentSection = NULL;
		SAFE_RELEASE(pEffectBuffer);
	}
	else
	{
		DataSectionPtr pSection = BWResource::instance().rootSection()->openSection( objectName );
		if (pSection)
		{
			bin = pSection->readBinary( "effect" );
			if (!bin) // left in to load old files.
				bin = BWResource::instance().fileSystem()->readFile( objectName );
		}
		else
			bin = BWResource::instance().fileSystem()->readFile( objectName );

		if (bin && outResult)
		{
			*outResult = "Up to date.";
		}
	}

	if (preProcess)
	{
		delete [] preProcess;
		preProcess = NULL;
	}

	return bin;

	#undef SAFE_RELEASE
}

bool ManagedEffect::load( const std::string& effectResource )
{
	BW_GUARD;
	DiaryScribe dsLoad( Diary::instance(), 
		(std::string( "ManagedEffect Load ") + effectResource) );
	settingsListenerId_ = -1;
	resourceID_ = effectResource;

	D3DXMACRO * preProcessDefinition = generatePreprocessors();

	std::string resName = effectResource;

	// use the current fxoInfix to compile the set of 
	// fx using the currently active macro definitions
	std::string effectPath = BWResource::getFilePath( effectResource );
	EffectManager::instance().pEffectIncludes()->currentPath( effectPath );
	std::string fxoInfix   = EffectManager::instance().fxoInfix();
	std::string fileName   = BWResource::removeExtension(resName);
	std::string objectName = !fxoInfix.empty() 
		? fileName + "." + fxoInfix + ".fxo"
		: fileName + ".fxo";

	BinaryPtr bin = compile(resName, preProcessDefinition);

	bool ret = cache(bin, preProcessDefinition);
	delete [] preProcessDefinition;

	return ret;
}

/**
 *	Retrieve "graphicsSetting" label and "label" annotation from 
 *	the effect file, if any. Effects labeled as graphics settings will 
 *	be registered into moo's graphics settings registry. Each technique 
 *	tagged by a "label" annotation will be added as an option to the
 *	effect's graphic setting object. Materials and other subsystems can 
 *	then register into the setting entry and select the appropriate 
 *	technique when the selected options/technique changes.
 *
 *	Like any graphics setting, more than one effect can be registered 
 *	under the same setting entry. For this to happen, they must share 
 *	the same label, have the same number of tagged techniques, each
 *	with the same label and appearing in the same order. If two or 
 *	more effects shared the same label, but the above rules are 
 *	not respected, an assertion will fail. 
 *
 *	@param	effectResource	name of the effect file.
 *	@return	False if application was quit during processing.
 *	@see	GraphicsSetting::add
 */
bool ManagedEffect::registerGraphicsSettings(
	const std::string & effectResource)
{
	BW_GUARD;
	// Already done.
 	if ( settingsAdded_ )
 		return true;

	if (!Moo::rc().checkDeviceYield())
	{
		ERROR_MSG("ManagedEffect: device lost when registering settings\n");
		return false;
	}

	if ( !validateAllTechniques() )
		return false;

	D3DXHANDLE feature = pEffect_->GetParameterByName(0, "graphicsSetting");

	if (feature != 0)
	{
		bool labelExists = false;
		bool descExists = false;
		
		D3DXHANDLE nameHandle = 
			pEffect_->GetAnnotationByName(feature, "label");

		LPCSTR settingName;
		if (nameHandle != 0)
			labelExists = SUCCEEDED(pEffect_->GetString(nameHandle, &settingName));

		D3DXHANDLE descHandle = 
			pEffect_->GetAnnotationByName(feature, "desc");

		LPCSTR settingDesc;
		if (descHandle != 0)
			descExists = SUCCEEDED(pEffect_->GetString(descHandle, &settingDesc));

		if (labelExists && descExists)
		{
			// Set our name and description
			settingName_ = settingName;
			settingDesc_ = settingDesc;
		
			// For each existing technique, add name based on label.
			for ( uint32 i = 0; i < techniques_.size(); i++ )
			{
				labelExists = false;
				descExists = false;
		
				TechniqueInfo& technique = techniques_[i];
		
				IF_NOT_MF_ASSERT_DEV( technique.handle_ != NULL )
				{
					return false;
				}

				LPCSTR techLabel;
				D3DXHANDLE techLabelHandle = 
					pEffect_->GetAnnotationByName( technique.handle_, "label");						

				if (techLabelHandle != 0)
					labelExists = SUCCEEDED(pEffect_->GetString(techLabelHandle, &techLabel));

				LPCSTR techDesc;
				D3DXHANDLE techDescHandle = 
					pEffect_->GetAnnotationByName( technique.handle_, "desc");						

				if (techDescHandle != 0)
					descExists = SUCCEEDED(pEffect_->GetString(techDescHandle, &techDesc));

				if (labelExists && descExists)
				{
					technique.settingLabel_ = techLabel;
					technique.settingDesc_	= techDesc;
				}
				else
				{
					D3DXTECHNIQUE_DESC desc;
					pEffect_->GetTechniqueDesc(technique.handle_, &desc);
					WARNING_MSG(
						"Effect labeled as a feature but either the"
						"\"label\" or \"desc\" annotations provided for "
						"technique \"%s\" are missing (won't be listed)\n", desc.Name);
				}
			}	
		}
		else 
		{
			ERROR_MSG(
				"Effect \"%s\" labeled as feature but either "
				"the \"label\" or \"desc\" annotations are missing\n", 
				effectResource.c_str());
		}
	}

	// register engine feature if 
	// effect is labeled as such
	if (!this->settingName_.empty() && !this->techniques_.empty())
	{
		this->settingEntry_ = new EffectTechniqueSetting(this, this->settingName_, this->settingDesc_);
		typedef TechniqueInfoCache::const_iterator citerator;
		citerator techIt  = this->techniques_.begin();
		citerator techEnd = this->techniques_.end();
		while (techIt != techEnd)
		{
			this->settingEntry_->addTechnique(*techIt);
			++techIt;
		}

		// register it
		GraphicsSetting::add(this->settingEntry_);
	}

	settingsAdded_ = true;
	return true;
}

/**
*	This method completes the initialisation of the material. This occurs in
*	in the main (render) thread.
*
*	@returns true if successful, false if init was blocked (caller must try
*				again).
*/
bool ManagedEffect::finishInit()
{
	BW_GUARD;
	MF_ASSERT_DEBUG( g_renderThread == true );
	
	if ( !initComplete_ )
	{
		// this blocks callee functions from re-trying finishInit, and getting
		// stuck in an infinite loop.
		initComplete_ = true;

		bool done = true;

		if (Moo::rc().device()->TestCooperativeLevel() == D3D_OK)
		{
			D3DXHANDLE hT = NULL;

			registerGraphicsSettings( resourceID() );

			EffectTechniqueSetting* pSettingsEntry = graphicsSettingEntry();

			if (pSettingsEntry != 0 && 
				pSettingsEntry->activeTechniqueHandle() != 0)
			{
				// first set ourselves as a listener and add listener id
				settingsListenerId_ = pSettingsEntry->addListener( this );

				// set the active technique for our graphics settings
				hT = pSettingsEntry->activeTechniqueHandle();

				setCurrentTechnique( hT, true );
			}
			else if ( getFirstValidTechnique(hT) )
			{
				setCurrentTechnique( hT, false );
			}
			else
			{
				//We are probably just waiting for the effect
				//to validate its technique (this can happen at any
				//time when effect compilation is not happening)
				done = false;
			}
		}
		else
		{
			// device is not valid,
			// try again later.
			done = false;
		}

		// set status
		initComplete_ = done;
	}

	// return status
	return initComplete_;
}

/**
* This method returns the name of the current technique.
*/
const std::string ManagedEffect::currentTechniqueName()
{
	BW_GUARD;
	D3DXHANDLE		hTec	= this->getCurrentTechnique();
	TechniqueInfo*	entry	= this->findTechniqueInfo( hTec );

	if ( entry )
	{
		return entry->name_;
	}
		
	return "Unknown technique";
}

/**
* This method returns the channel annotation (or null) of the given technique
* on the effect.
*/
VisualChannelPtr ManagedEffect::getChannel( D3DXHANDLE techniqueOverride )
{
	BW_GUARD;
	D3DXHANDLE		hTec	= techniqueOverride == NULL ? 
		this->getCurrentTechnique() : techniqueOverride;
	TechniqueInfo*	entry	= this->findTechniqueInfo( hTec );
	
	if ( !entry )
	{
		ERROR_MSG("Unknown technique %s (%x) on effect %s.\n", hTec, hTec,
			resourceID_.c_str() );
		return NULL;
	}

	return entry->channel_; 
}

/**
 *	This method returns the skinned status for given technique.
 */
bool ManagedEffect::skinned( D3DXHANDLE techniqueOverride )
{ 
	BW_GUARD;

	if ( !initComplete_ )
	{
		return false;
	}

	D3DXHANDLE hTec	= techniqueOverride == NULL ? 
						this->getCurrentTechnique() : techniqueOverride;
	TechniqueInfo* entry = this->findTechniqueInfo( hTec );

	if ( entry )
	{
		return entry->skinned_;
	}

	ERROR_MSG("Unknown technique %s (%x) on effect %s.\n", hTec, hTec,
		resourceID_.c_str() );

	return false; 
}

/**
 * This method returns the bumped status for given technique.
 */
bool ManagedEffect::bumpMapped( D3DXHANDLE techniqueOverride ) 
{ 
	BW_GUARD;

	if ( !initComplete_ )
	{
		return false;
	}

	D3DXHANDLE hTec	= techniqueOverride == NULL ? 
						this->getCurrentTechnique() : techniqueOverride;
	TechniqueInfo* entry = this->findTechniqueInfo( hTec );

	if ( entry )
	{
		return entry->bumpMapped_;
	}

	ERROR_MSG("Unknown technique %s (%x) on effect %s.\n", hTec, hTec,
		resourceID_.c_str() );

	return false; 
}

/**
 * This method returns the bumped status for given technique.
 */
bool ManagedEffect::dualUV( D3DXHANDLE techniqueOverride ) 
{ 
	BW_GUARD;

	if ( !initComplete_ )
	{
		return false;
	}

	D3DXHANDLE hTec	= techniqueOverride == NULL ? 
						this->getCurrentTechnique() : techniqueOverride;
	TechniqueInfo* entry = this->findTechniqueInfo( hTec );

	if ( entry )
	{
		return entry->dualUV_;
	}

	ERROR_MSG("Unknown technique %s (%x) on effect %s.\n", hTec, hTec,
		resourceID_.c_str() );

	return false; 
}

/**
*	Internal method for setting current technique.
*
*	@param hTec handle to the technique to use.
*	@return true if successful
*/
bool ManagedEffect::setCurrentTechnique( D3DXHANDLE hTec, bool setExplicit )
{
	BW_GUARD;
	if ( !finishInit() )
		return false;

#ifdef _DEBUG
	TechniqueInfo* entry = this->findTechniqueInfo( hTec );
	if ( !entry )
	{
		D3DXTECHNIQUE_DESC techniqueDesc;
		if ( !SUCCEEDED(pEffect_->GetTechniqueDesc( hTec , &techniqueDesc ) ) )
		{
			techniqueDesc.Name = "<invalid>";
		}

		ERROR_MSG("Unknown technique %s on effect %s.\n", techniqueDesc.Name, 
			resourceID_.c_str() );

		return false;
	}
#endif

	techniqueExplicitlySet_ = setExplicit;
	hCurrentTechnique_		= hTec;

	return true;
}
	
/**
* This method is called when a different technique is selected (eg for a 
* graphics setting change).
*/
void ManagedEffect::onSelectTechnique(ManagedEffect * effect, D3DXHANDLE hTec)
{
	BW_GUARD;
	IF_NOT_MF_ASSERT_DEV( g_renderThread == true )
	{
		ERROR_MSG( "ManagedEffect::onSelelctTechnique - not called from "
			"render thread\n" );
		return;
	}

	if ( initComplete_ )
	{
		this->setCurrentTechnique( hTec, true );
	}
	else
	{
		ERROR_MSG("ManagedEffect::onSelelctTechnique - called for " 
			"incomplete effect: %s, new technique not selected. \n",
			resourceID().c_str() );					
	}
}

void ManagedEffect::onSelectPSVersionCap(int psVerCap)
{
	BW_GUARD;
	if ( !techniqueExplicitlySet_ )
	{			
		initComplete_ = false;
		finishInit();
	}
}

// -----------------------------------------------------------------------------
// Section: EffectTechniqueSetting
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 *
 *	@param	owner	owning ManagedEffect object.
 *	@param	label	label for setting.
 */
EffectTechniqueSetting::EffectTechniqueSetting(
		ManagedEffect * owner,
		const std::string & label,
		const std::string & desc):
	GraphicsSetting(label, desc, -1, false, false),
	owner_(owner),
	techniques_(),
	listeners_()
{}

/**
 *	Adds new technique to effect. This will add a new 
 *	option by the same label into the base GraphicsSetting.
 *
 *	@param	info	technique information.
 */
void EffectTechniqueSetting::addTechnique(const TechniqueInfo &info)
{
	BW_GUARD;
	this->GraphicsSetting::addOption(info.settingLabel_, info.settingDesc_, info.supported_);
	this->techniques_.push_back(std::make_pair(info.handle_, info.psVersion_));
}

/**
 *	Virtual functions called by the base class whenever the current 
 *	option changes. Will notify all registered listeners and slaves.
 *
 *	@param	optionIndex	index of new option selected.
 */
void EffectTechniqueSetting::onOptionSelected(int optionIndex)
{
	BW_GUARD;
	this->listenersLock_.grab();
	ListenersMap::iterator listIt  = this->listeners_.begin();
	ListenersMap::iterator listEnd = this->listeners_.end();
	while (listIt != listEnd)
	{
		listIt->second->onSelectTechnique(
			this->owner_, this->techniques_[optionIndex].first);
		++listIt;
	}
	this->listenersLock_.give();
}		

/**
 *	Sets the pixel shader version cap. Updates effect to use a supported 
 *	technique that is still within the new cap. If none is found, use the
 *	last one available (should be the less capable).
 *
*	@param	psVersionCap	maximum major version number.
 */
void EffectTechniqueSetting::setPSCapOption(int psVersionCap)
{
	BW_GUARD;
	typedef D3DXHandlesPSVerVector::const_iterator citerator;
	citerator techBegin = this->techniques_.begin();
	citerator techEnd   = this->techniques_.end();
	citerator techIt    = techBegin;
	while (techIt != techEnd)
	{
		if (techIt->second <= psVersionCap)
		{
			int optionIndex = std::distance(techBegin, techIt);
			if (this->GraphicsSetting::selectOption(optionIndex))
				break;
		}
		++techIt;
	}
	if (techIt == techEnd)
	{
		this->GraphicsSetting::selectOption(this->techniques_.size()-1);
	}
}

/**
 *	Returns handle to currently active technique.
 *
 *	@return	handle to active technique.
 */
D3DXHANDLE EffectTechniqueSetting::activeTechniqueHandle() const
{
	BW_GUARD;
	int activeOption = this->GraphicsSetting::activeOption();
	if (activeOption == -1) return 0;
	if (activeOption >= (int)(this->techniques_.size())) return 0;
	return this->techniques_[activeOption].first;
}

/**
 *	Registers an EffectTechniqueSetting listener instance.
 *
 *	@param	listener	listener to register.
 *	@return				id of listener (use this to unregister).
 */
int EffectTechniqueSetting::addListener(IListener * listener)
{
	BW_GUARD;
	++EffectTechniqueSetting::listenersId;

	this->listenersLock_.grab();
	this->listeners_.insert(std::make_pair(EffectTechniqueSetting::listenersId, listener));
	this->listenersLock_.give();

	return EffectTechniqueSetting::listenersId;
}

/**
 *	Unregisters EffectTechniqueSetting listener identified by given id.
 *
 *	@param	listenerId	id of listener to be unregistered.
 */
void EffectTechniqueSetting::delListener(int listenerId)
{
	BW_GUARD;
	this->listenersLock_.grab();
	ListenersMap::iterator listIt = this->listeners_.find(listenerId);
	MF_ASSERT_DEV(listIt !=  this->listeners_.end());
	if( listIt != this->listeners_.end() )
		this->listeners_.erase(listIt);
	this->listenersLock_.give();
}

int EffectTechniqueSetting::listenersId = 0;

// -----------------------------------------------------------------------------
// Section: EffectTechniqueSetting
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 *
 *	@param	label		label for setting.
 *	@param	macro		macro to be defined.
 *	@param	setupFunc	callback that will setup this EffectMacroSetting.
 */
EffectMacroSetting::EffectMacroSetting(
		const std::string & label, 
		const std::string & desc,
		const std::string & macro, 
		SetupFunc setupFunc) :
	GraphicsSetting(label, desc, -1, false, true),
	macro_(macro),
	setupFunc_(setupFunc),
	values_()
{
	BW_GUARD;
	EffectMacroSetting::add(this);
}

/**
 *	Adds option to this EffectMacroSetting.
 *
 *	@param	label		label for option.
 *	@param	desc		description for option.
 *	@param	isSupported	true if option is supported in this system.
 *	@param	value		value the macro should be defined to when is 
 *						option is active.
 */
void EffectMacroSetting::addOption(
	const std::string & label, 
	const std::string & desc, 
	bool                isSupported, 
	const std::string & value)
{
	BW_GUARD;
	this->GraphicsSetting::addOption(label, desc, isSupported);
	this->values_.push_back(value);
}

/**
 *	Defines the macro according to the option selected. Implicitly 
 *	called whenever the user changes the current active option.
 */		
void EffectMacroSetting::onOptionSelected(int optionIndex)
{
	BW_GUARD;
	EffectManager::instance().setMacroDefinition(
		this->macro_, 
		this->values_[optionIndex]);
}

/**
 *	REgisters setting to static list of EffectMacroSetting instances.
 */
void EffectMacroSetting::add(EffectMacroSettingPtr setting)
{
	BW_GUARD;
	static_settings().push_back(setting);
}

/**
 *	Update the EffectManager's fxo infix according to the currently 
 *	selected macro effect options.
 */
void EffectMacroSetting::updateManagerInfix()
{
	BW_GUARD;
	std::stringstream infix;
	MacroSettingVector::const_iterator setIt  = static_settings().begin();
	MacroSettingVector::const_iterator setEnd = static_settings().end();
	while (setIt != setEnd)
	{
		infix << (*setIt)->activeOption();
		++setIt;
	}
	EffectManager::instance().fxoInfix(infix.str());
}

/**
 *	Sets up registered macro effect instances.
 */	
bool SettingsPred(EffectMacroSetting::EffectMacroSettingPtr a, 
				 EffectMacroSetting::EffectMacroSettingPtr b)
{
	BW_GUARD;
	return strcmp(a->macroName().c_str(), b->macroName().c_str()) < 0;
}

void EffectMacroSetting::setupSettings()
{
	BW_GUARD;
	MacroSettingVector::iterator setIt  = static_settings().begin();
	MacroSettingVector::iterator setEnd = static_settings().end();
	while (setIt != setEnd)
	{
		(*setIt)->setupFunc_(**setIt);
		GraphicsSetting::add(*setIt);
		++setIt;
	}

	// Sort by name, so that the order is deterministic rather than 
	// (un)defined by static init order.
	EffectMacroSetting::MacroSettingVector& settings = static_settings();
	std::sort(settings.begin(), settings.end(), SettingsPred);
}


void EffectMacroSetting::finiSettings()
{
	delete s_pSettings;
	s_pSettings = NULL;
}


void EffectMacroSetting::getMacroSettings(MacroSettingVector& settings)
{
	BW_GUARD;
	settings = static_settings();
}

namespace { // anonymous

/**
 *	Helper function to work around static-initialisation-order issues.
 */
EffectMacroSetting::MacroSettingVector & static_settings()
{
	if (!s_pSettings)
	{
		s_pSettings = new EffectMacroSetting::MacroSettingVector;
	}
	return *s_pSettings;
}

} // namespace anonymous

} // namespace moo

// managed_effect.cpp
