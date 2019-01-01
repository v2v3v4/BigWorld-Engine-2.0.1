/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EFFECT_MATERIAL_HPP
#define EFFECT_MATERIAL_HPP

#include "forward_declarations.hpp"
#include "moo_dx.hpp"
#include "managed_effect.hpp"
#include "effect_state_manager.hpp"
#include "physics2/worldtri.hpp"
#include "effect_manager.hpp"

namespace Moo
{

#define SAFE_SET( pTheEffect, type, name, value ) \
{\
	D3DXHANDLE h = pTheEffect->GetParameterByName( NULL, name );\
	if (h)\
	{\
	pTheEffect->Set##type( h, value );\
	}\
}

/**
 * This class implements the basic material used to render visuals.
 * The material uses a d3dx effect as it's basis for rendering operations.
 */
class EffectMaterial :
	public SafeReferenceCount
{
public:
	typedef std::map< D3DXHANDLE, EffectPropertyPtr > Properties;

	EffectMaterial();
	explicit EffectMaterial( const EffectMaterial & other );
	EffectMaterial & operator=( const EffectMaterial & other );

	~EffectMaterial();

	bool load( DataSectionPtr pSection, bool addDefault = true );
	void save( DataSectionPtr pSection );

	bool initFromEffect( const std::string& effect, const std::string& diffuseMap = "", int doubleSided = -1 );

	bool begin();
	bool end() const;

	bool beginPass( uint32 pass ) const;
	bool endPass() const;

	bool commitChanges() const;

	StateRecorder* recordPass( uint32 pass ) const;
	uint32 nPasses() const { return nPasses_; }

	const std::string& identifier() const { return identifier_; }
	void identifier( const std::string& identifier ) { identifier_ = identifier; }

	D3DXHANDLE hTechnique() const;
	bool hTechnique( D3DXHANDLE hTec );

	ManagedEffectPtr pEffect() { return pManagedEffect_; }
  
	bool boolProperty( bool & result, const std::string & name ) const;
	bool intProperty( int & result, const std::string & name ) const;
	bool floatProperty( float & result, const std::string & name ) const;
	bool vectorProperty( Vector4 & result, const std::string & name ) const;
	EffectPropertyPtr getProperty( const std::string & name );
	ConstEffectPropertyPtr getProperty( const std::string & name ) const;
	bool replaceProperty( const std::string & name, EffectPropertyPtr effectProperty );

	WorldTriangle::Flags getFlags( int objectMaterialKind ) const;

	Properties & properties()	{ return properties_; }

	bool skinned() const;
	bool bumpMapped() const;
	bool dualUV() const;

	bool readyToUse() const 
	{ return pManagedEffect_ ? pManagedEffect_->readyToUse() : false; }
	
	VisualChannelPtr channel() const;

	bool isDefault( EffectPropertyPtr pProp );
	void replaceDefaults();

#ifdef EDITOR_ENABLED
	int materialKind() const { return materialKind_; }
	void materialKind( int mk ) { materialKind_ = mk; }
	
	int collisionFlags() const { return collisionFlags_; }
	void collisionFlags( int cf ) { collisionFlags_ = cf; }

	bool bspModified_;
#endif
	
private:
	
	bool loadInternal( DataSectionPtr pSection, EffectPropertyMappings& outProperties );
	
	ManagedEffectPtr	pManagedEffect_;
	UINT				nPasses_;
	Properties			properties_;

	VisualChannelPtr	channelOverride_;
	D3DXHANDLE			hOverriddenTechnique_; // if 0, then no override

	std::string			identifier_;

	// physics data
	int					materialKind_;
	int					collisionFlags_;

	static EffectMaterialPtr			s_curMaterial;
};

}

#endif // EFFECT_MATERIAL_HPP
