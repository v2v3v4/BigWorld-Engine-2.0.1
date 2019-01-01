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
#include "material_properties.hpp"
#include "material_proxies.hpp"
#include "resmgr/bwresource.hpp"
#include "resmgr/xml_section.hpp"
#include "moo/texture_manager.hpp"
#include "moo/effect_material.hpp"
#include "common/material_editor.hpp"
#include "common/material_utility.hpp"
#include "material_proxies.hpp"

#define EDCALL __stdcall
#define WORLDEDITORDLL_API

DECLARE_DEBUG_COMPONENT2( "Common", 0 );


namespace
{

typedef MaterialProxy< MaterialTextureProxy, std::string > TextureProxyType;
typedef MaterialProxy< MaterialTextureFeedProxy, std::string > TextureFeedProxyType;
typedef MaterialProxy< MaterialBoolProxy, bool > BoolProxyType;
typedef MaterialProxy< MaterialIntProxy, int32 > IntProxyType;
typedef MaterialProxy< MaterialEnumProxy, uint32 > EnumProxyType;
typedef MaterialProxy< MaterialFloatProxy, float > FloatProxyType;
typedef MaterialProxy< MaterialVector4Proxy, Vector4 > Vector4ProxyType;

// These maps help in identifying and join multiple instances of the same
// material property.
std::map< std::string, SmartPointer< TextureProxyType > >		s_textureProxy;
std::map< std::string, SmartPointer< TextureFeedProxyType > >	s_textureFeedProxy;
std::map< std::string, SmartPointer< BoolProxyType > >			s_boolProxy;
std::map< std::string, SmartPointer< IntProxyType > >			s_intProxy;
std::map< std::string, SmartPointer< EnumProxyType > >			s_enumProxy;
std::map< std::string, SmartPointer< FloatProxyType > >			s_floatProxy;
std::map< std::string, SmartPointer< Vector4ProxyType > >		s_vector4Proxy;


///////////////////////////////////////////////////////////////////////////////
// Section : Proxy functors needed by MaterialProperties
///////////////////////////////////////////////////////////////////////////////

class BoolProxyFunctor : public Moo::EffectPropertyFunctor
{
public:
	virtual Moo::EffectPropertyPtr create( DataSectionPtr pSection )
	{
		BW_GUARD;

		MaterialBoolProxy* prop = new MaterialBoolProxy;
		prop->set( pSection->asBool(), false );
		return prop;
	}

	virtual Moo::EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		BW_GUARD;

		MaterialBoolProxy* prop = new MaterialBoolProxy( pEffect, hProperty );
		BOOL v;
        pEffect->GetBool( hProperty, &v );
		prop->set( !!v, false );
		return prop;
	}

	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		BW_GUARD;

		return (propertyDesc->Class == D3DXPC_SCALAR && propertyDesc->Type == D3DXPT_BOOL);
	}
};


class IntProxyFunctor : public Moo::EffectPropertyFunctor
{
public:
	virtual Moo::EffectPropertyPtr create( DataSectionPtr pSection )
	{
		BW_GUARD;

		MaterialIntProxy* prop = new MaterialIntProxy;
		prop->set( pSection->asInt(), false );
		return prop;
	}

	virtual Moo::EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		BW_GUARD;

		MaterialIntProxy* prop = new MaterialIntProxy( pEffect, hProperty );
		int32 v;
        pEffect->GetInt( hProperty, &v );
		prop->set( v, false );
		prop->attach( hProperty, pEffect );
		return prop;
	}

	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		BW_GUARD;

		return (propertyDesc->Class == D3DXPC_SCALAR && propertyDesc->Type == D3DXPT_INT );
	}
};


class FloatProxyFunctor : public Moo::EffectPropertyFunctor
{
public:
	virtual Moo::EffectPropertyPtr create( DataSectionPtr pSection )
	{
		BW_GUARD;

		MaterialFloatProxy* proxy = new MaterialFloatProxy;
		proxy->set( pSection->asFloat(), false );
		return proxy;
	}

	virtual Moo::EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		BW_GUARD;

		MaterialFloatProxy* proxy = new MaterialFloatProxy( pEffect, hProperty );
		float v;
        pEffect->GetFloat( hProperty, &v );
		proxy->set( v, false );
		proxy->attach( hProperty, pEffect );
		return proxy;
	}

	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		BW_GUARD;

		return (propertyDesc->Class == D3DXPC_SCALAR && propertyDesc->Type == D3DXPT_FLOAT);
	}
};


class Vector4ProxyFunctor : public Moo::EffectPropertyFunctor
{
public:
	virtual Moo::EffectPropertyPtr create( DataSectionPtr pSection )
	{
		BW_GUARD;

		MaterialVector4Proxy* proxy = new MaterialVector4Proxy;
		proxy->set( pSection->asVector4(), false );
		return proxy;
	}

	virtual Moo::EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		BW_GUARD;

		MaterialVector4Proxy* proxy = new MaterialVector4Proxy( pEffect, hProperty );
		Vector4 v;
        pEffect->GetVector( hProperty, &v );
		proxy->set( v, false );
		return proxy;
	}

	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		BW_GUARD;

		return (propertyDesc->Class == D3DXPC_VECTOR && propertyDesc->Type == D3DXPT_FLOAT);
	}
};


class MatrixProxyFunctor : public Moo::EffectPropertyFunctor
{
public:
	virtual Moo::EffectPropertyPtr create( DataSectionPtr pSection )
	{
		BW_GUARD;

		MaterialMatrixProxy* prop = new MaterialMatrixProxy;
        Matrix m;
		m.row( 0, pSection->readVector4( "row0" ) );
		m.row( 1, pSection->readVector4( "row1" ) );
		m.row( 2, pSection->readVector4( "row2" ) );
		m.row( 3, pSection->readVector4( "row3" ) );
		prop->setMatrix( m );
		return prop;
	}

	virtual Moo::EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		BW_GUARD;

		MaterialMatrixProxy* prop = new MaterialMatrixProxy( pEffect, hProperty );
		Matrix m;
        pEffect->GetMatrix( hProperty, &m );
		prop->setMatrix( m );
		return prop;
	}

	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		BW_GUARD;

		return ((propertyDesc->Class == D3DXPC_MATRIX_ROWS || propertyDesc->Class == D3DXPC_MATRIX_COLUMNS) &&
				propertyDesc->Type == D3DXPT_FLOAT);
	}
};


class ColourProxyFunctor : public Moo::EffectPropertyFunctor
{
public:
	virtual Moo::EffectPropertyPtr create( DataSectionPtr pSection )
	{
		BW_GUARD;

		MaterialColourProxy* proxy = new MaterialColourProxy;
		proxy->setVector4( pSection->asVector4(), false );
		return proxy;
	}

	virtual Moo::EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		BW_GUARD;

		MaterialColourProxy* proxy = new MaterialColourProxy( pEffect, hProperty );
		Vector4 v;
        pEffect->GetVector( hProperty, &v );
		proxy->setVector4( v, false );
		return proxy;
	}

	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		BW_GUARD;

		return (propertyDesc->Class == D3DXPC_VECTOR &&
			propertyDesc->Type == D3DXPT_FLOAT);
	}
private:
};


class TextureProxyFunctor : public Moo::EffectPropertyFunctor
{
public:
	virtual Moo::EffectPropertyPtr create( DataSectionPtr pSection )
	{
		BW_GUARD;

		MaterialTextureProxy* prop = new MaterialTextureProxy;
		prop->set( pSection->asString(), false );
		return prop;
	}

	virtual Moo::EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		BW_GUARD;

		MaterialTextureProxy* prop = new MaterialTextureProxy( pEffect, hProperty );
		return prop;
	}

	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		BW_GUARD;

		return (propertyDesc->Class == D3DXPC_OBJECT &&
			(propertyDesc->Type == D3DXPT_TEXTURE ||
			propertyDesc->Type == D3DXPT_TEXTURE1D ||
			propertyDesc->Type == D3DXPT_TEXTURE2D ||
			propertyDesc->Type == D3DXPT_TEXTURE3D ||
			propertyDesc->Type == D3DXPT_TEXTURECUBE ));
	}
};


class TextureFeedProxyFunctor : public Moo::EffectPropertyFunctor
{
public:
	virtual Moo::EffectPropertyPtr create( DataSectionPtr pSection )
	{
		BW_GUARD;

		MaterialTextureFeedProxy* prop = new MaterialTextureFeedProxy;
		prop->set( pSection->readString( "default", "" ), false );
		prop->setTextureFeed( pSection->asString() );
		return prop;
	}

	virtual Moo::EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		BW_GUARD;

		MaterialTextureFeedProxy* prop = new MaterialTextureFeedProxy( pEffect, hProperty );
		return prop;
	}

	virtual bool check( D3DXPARAMETER_DESC* propertyDesc )
	{
		BW_GUARD;

		return (propertyDesc->Class == D3DXPC_OBJECT &&
			(propertyDesc->Type == D3DXPT_TEXTURE ||
			propertyDesc->Type == D3DXPT_TEXTURE1D ||
			propertyDesc->Type == D3DXPT_TEXTURE2D ||
			propertyDesc->Type == D3DXPT_TEXTURE3D ||
			propertyDesc->Type == D3DXPT_TEXTURECUBE ));
	}
};


///////////////////////////////////////////////////////////////////////////////
// Section : Functions to add properties of different types
///////////////////////////////////////////////////////////////////////////////

void addBoolProperty(	const std::string & descName,
						const std::string & uiName, const std::string & uiDesc,
						const Moo::EffectPropertyPtr & pProperty,
						bool canExposeToScript, bool canUseTextureFeeds, 
						MaterialPropertiesUser * callback, GeneralEditor * editor )
{
	BW_GUARD;

	MaterialBoolProxy* pBoolProxy = dynamic_cast<MaterialBoolProxy*>(pProperty.get());
	MF_ASSERT( pBoolProxy != NULL );

	if (s_boolProxy[ descName ])
	{
		s_boolProxy[ descName ]->addProperty( pBoolProxy );
	}
	else
	{
		s_boolProxy[ descName ] =
			new BoolProxyType( pBoolProxy, callback );

		BoolProxyPtr proxy = callback->boolProxy( s_boolProxy[ descName ],
				new BWFunctor0RC< BoolProxyType, bool >( s_boolProxy[ descName ].get(), &(BoolProxyType::get) ),
				new BWFunctor1< BoolProxyType, bool >( s_boolProxy[ descName ].get(), &(BoolProxyType::set) ),
				uiName, descName );

		std::string exposedToScriptName = callback->exposedToScriptName( descName );
		
		GenBoolProperty* pProp = new GenBoolProperty( uiName, proxy );
		pProp->descName( bw_utf8tow( descName ) );
		pProp->UIDesc( bw_utf8tow( uiDesc ) );
		pProp->exposedToScriptName( bw_utf8tow( exposedToScriptName ) );
		pProp->canExposeToScript( canExposeToScript );
		editor->addProperty( pProp );
	}
}


void addEnumProperty(	const std::string & descName,
						const std::string & uiName, const std::string & uiDesc,
						const Moo::EffectPropertyPtr & pProperty,
						bool canExposeToScript, bool canUseTextureFeeds, 
						MaterialPropertiesUser * callback, GeneralEditor * editor )
{
	BW_GUARD;

	MaterialIntProxy* pIntProxy = dynamic_cast<MaterialIntProxy*>(pProperty.get());
	MF_ASSERT( pIntProxy != NULL );

	std::string enumType;
	if( pIntProxy->stringAnnotation( "EnumType", enumType ) )
	{
		if( DXEnum::instance().isEnum( enumType ) )
		{
			DataSectionPtr enumSec = new XMLSection( enumType );
			for( DXEnum::size_type i = 0;
					i != DXEnum::instance().size( enumType ); ++i)
			{
					std::string name = DXEnum::instance().entry( enumType, i );
					enumSec->writeInt( name,
							DXEnum::instance().value( enumType, name ) );
			}
			
			if (s_enumProxy[ descName ])
			{
				s_enumProxy[ descName ]->addProperty( new MaterialEnumProxy( enumType, pIntProxy ) );
			}
			else
			{
				s_enumProxy[ descName ] =
					new EnumProxyType(
						new MaterialEnumProxy( enumType, pIntProxy ), callback );
		
				IntProxyPtr proxy = callback->enumProxy( s_enumProxy[ descName ],
						new BWFunctor0RC< EnumProxyType, uint32 >( s_enumProxy[ descName ].get(), &(EnumProxyType::get) ),
						new BWFunctor1< EnumProxyType, uint32 >( s_enumProxy[ descName ].get(), &(EnumProxyType::set) ),
						uiName, descName );
							
				ChoiceProperty* pProp = new ChoiceProperty( uiName, proxy, enumSec );
				pProp->descName( bw_utf8tow( descName ) );
				pProp->UIDesc( bw_utf8tow( uiDesc ) );
				editor->addProperty( pProp );
			}
		}
		else
		{
			enumType.clear();
		}
	}

	if( !enumType.size() )
	{
		if (s_intProxy[ descName ])
		{
			s_intProxy[ descName ]->addProperty( (MaterialIntProxy*)pProperty.getObject() );
		}
		else
		{
			s_intProxy[ descName ] =
				new IntProxyType( pIntProxy, callback );

			IntProxyPtr proxy = callback->intProxy( s_intProxy[descName],
					new BWFunctor0RC< IntProxyType, int >( s_intProxy[ descName ].get(), &(IntProxyType::get) ),
					new BWFunctor1< IntProxyType, int >( s_intProxy[ descName ].get(), &(IntProxyType::set) ),
					new BWFunctor2R< IntProxyType, int&, int&, bool >( s_intProxy[ descName ].get(), &(IntProxyType::getRange) ),
					uiName, descName );

			std::string exposedToScriptName = callback->exposedToScriptName( descName );
			
			GenIntProperty* pProp = new GenIntProperty( uiName, proxy );
			pProp->descName( bw_utf8tow( descName ) );
			pProp->UIDesc( bw_utf8tow( uiDesc ) );
			pProp->exposedToScriptName( bw_utf8tow( exposedToScriptName ) );
			pProp->canExposeToScript( canExposeToScript );
			editor->addProperty( pProp );
		}
	}
}


void addFloatProperty(const std::string & descName,
						const std::string & uiName, const std::string & uiDesc,
						const Moo::EffectPropertyPtr & pProperty,
						bool canExposeToScript, bool canUseTextureFeeds, 
						MaterialPropertiesUser * callback, GeneralEditor * editor )
{
	BW_GUARD;

	MaterialFloatProxy* pFloatProxy = dynamic_cast<MaterialFloatProxy*>( pProperty.get() );
	MF_ASSERT( pFloatProxy != NULL );

	if (s_floatProxy[ descName ])
	{
		s_floatProxy[ descName ]->addProperty( pFloatProxy );
	}
	else
	{
		s_floatProxy[ descName ] =
			new FloatProxyType( pFloatProxy, callback );

		FloatProxyPtr proxy = callback->floatProxy( s_floatProxy[ descName ],
				new BWFunctor0RC< FloatProxyType, float >( s_floatProxy[ descName ].get(), &(FloatProxyType::get) ),
				new BWFunctor1< FloatProxyType, float >( s_floatProxy[ descName ].get(), &(FloatProxyType::set) ),
				new BWFunctor3R< FloatProxyType, float&, float&, int&, bool >( s_floatProxy[ descName ].get(), &(FloatProxyType::getRange) ),
				uiName, descName );

		std::string exposedToScriptName = callback->exposedToScriptName( descName );
		
		GenFloatProperty* pProp = new GenFloatProperty( uiName, proxy );
		pProp->descName( bw_utf8tow( descName ) );
		pProp->UIDesc( bw_utf8tow( uiDesc ) );
		pProp->exposedToScriptName( bw_utf8tow( exposedToScriptName ) );
		pProp->canExposeToScript( canExposeToScript );
		editor->addProperty( pProp );
	}
}


void addVector4Property(const std::string & descName,
						const std::string & uiName, const std::string & uiDesc,
						const Moo::EffectPropertyPtr & pProperty,
						bool canExposeToScript, bool canUseTextureFeeds, 
						MaterialPropertiesUser * callback, GeneralEditor * editor )
{
	BW_GUARD;

	MaterialVector4Proxy* pVector4Proxy = dynamic_cast<MaterialVector4Proxy*>( pProperty.get() );
	MF_ASSERT( pVector4Proxy != NULL );

	if (s_vector4Proxy[ descName ])
	{
		s_vector4Proxy[ descName ]->addProperty( pVector4Proxy );
	}
	else
	{
		s_vector4Proxy[ descName ] =
			new Vector4ProxyType( pVector4Proxy, callback );

		Vector4ProxyPtr proxy = callback->vector4Proxy( s_vector4Proxy[ descName ],
				new BWFunctor0RC< Vector4ProxyType, Vector4 >( s_vector4Proxy[ descName ].get(), &(Vector4ProxyType::get) ),
				new BWFunctor1< Vector4ProxyType, Vector4 >( s_vector4Proxy[ descName ].get(), &(Vector4ProxyType::set) ),
				uiName, descName );

		std::string exposedToScriptName = callback->exposedToScriptName( descName );
		
		std::string UIWidget = MaterialUtility::UIWidget( pVector4Proxy );

		if ((UIWidget == "Color") || (UIWidget == "Colour"))
		{
			ColourProperty* pColourProp = new ColourProperty( uiName, (Vector4ProxyPtr)proxy );
			pColourProp->exposedToScriptName( bw_utf8tow( exposedToScriptName  ));
			pColourProp->descName( bw_utf8tow( descName ) );
			pColourProp->UIDesc( bw_utf8tow( uiDesc ) );
			pColourProp->exposedToScriptName( bw_utf8tow( exposedToScriptName ) );
			pColourProp->canExposeToScript( canExposeToScript );	
			editor->addProperty( pColourProp );
		}
		else // Must be a vector
		{
			Vector4Property* pVectorProp = new Vector4Property( uiName, proxy );
			pVectorProp->exposedToScriptName( bw_utf8tow( exposedToScriptName ) );
			pVectorProp->descName( bw_utf8tow( descName ) );
			pVectorProp->UIDesc( bw_utf8tow( uiDesc ) );	
			pVectorProp->exposedToScriptName( bw_utf8tow( exposedToScriptName ) );
			pVectorProp->canExposeToScript( canExposeToScript );
			editor->addProperty( pVectorProp );
		}
	}
}


void addTextureProperty( const std::string & descName,
						const std::string & uiName, const std::string & uiDesc,
						const Moo::EffectPropertyPtr & pProperty,
						bool canExposeToScript, bool canUseTextureFeeds, 
						MaterialPropertiesUser * callback, GeneralEditor * editor )
{
	BW_GUARD;

	MaterialTextureProxy* pTextureProxy = dynamic_cast<MaterialTextureProxy*>( pProperty.get() );
	MaterialTextureFeedProxy* pTextureFeedProxy = dynamic_cast<MaterialTextureFeedProxy*>( pProperty.get() );
	MF_ASSERT( pTextureProxy != NULL || pTextureFeedProxy != NULL );

	if (s_textureProxy[ descName ] && pTextureProxy)
	{
		s_textureProxy[ descName ]->addProperty( pTextureProxy );
	}
	else if (s_textureFeedProxy[descName] && pTextureFeedProxy)
	{
		s_textureFeedProxy[ descName ]->addProperty( pTextureFeedProxy );
	}
	else
	{
		TextProperty* pProp = NULL;
		
		std::string UIWidget;
		if (pTextureProxy)
		{
			s_textureProxy[ descName ] =
				new TextureProxyType( pTextureProxy, callback );
	
			StringProxyPtr proxy = callback->textureProxy( s_textureProxy[ descName ],
					new BWFunctor0RC< TextureProxyType, std::string >( s_textureProxy[ descName ].get(), &(TextureProxyType::get) ),
					new BWFunctor1< TextureProxyType, std::string >( s_textureProxy[ descName ].get(), &(TextureProxyType::set) ),
					uiName, descName );

			pProp = new TextProperty( uiName, proxy );
			UIWidget = MaterialUtility::UIWidget( pTextureProxy );
		}
		else
		{
			s_textureFeedProxy[ descName ] =
				new TextureFeedProxyType( pTextureFeedProxy, callback );
	
			StringProxyPtr proxy = callback->textureProxy( s_textureFeedProxy[ descName ],
					new BWFunctor0RC< TextureFeedProxyType, std::string >( s_textureFeedProxy[ descName ].get(), &(TextureFeedProxyType::get) ),
					new BWFunctor1< TextureFeedProxyType, std::string >( s_textureFeedProxy[ descName ].get(), &(TextureFeedProxyType::set) ),
					uiName, descName );

			pProp = new TextProperty( uiName, proxy );
			UIWidget = MaterialUtility::UIWidget( pTextureFeedProxy );
		}
	
		pProp->descName( bw_utf8tow( descName ) );
		if (UIWidget == "RenderTarget")
		{
			pProp->fileFilter( L"Render Targets (*.rt)|*.rt|"
				L"Render Targets (*.rt)|*.rt||" );
		}
		else if (UIWidget == "CubeMap")
		{
			pProp->fileFilter( L"Cube maps (*.dds;*.texanim)|*.dds;*.texanim|"
				L"DDS files(*.dds)|*.dds|"
				L"Animated Textures (*.texanim)|*.texanim||" );
		}
		else
		{
			pProp->fileFilter( L"Texture files(*.bmp;*.tga;*.jpg;*.png;*.dds;*.texanim;*.rt)|*.bmp;*.tga;*.jpg;*.png;*.dds;*.texanim|"
				L"Bitmap files(*.bmp)|*.bmp|"
				L"Targa files(*.tga)|*.tga|"
				L"Jpeg files(*.jpg)|*.jpg|"
				L"Png files(*.png)|*.png|"
				L"DDS files(*.dds)|*.dds|"
				L"Animated Textures (*.texanim)|*.texanim|"
				L"Render Target (*.rt)|*.rt||" );
		}
		pProp->defaultDir( bw_utf8tow( callback->defaultTextureDir() ) );
		pProp->UIDesc( bw_utf8tow( uiDesc ) );
		if (pTextureFeedProxy)
		{
			std::string textureFeed = callback->textureFeed( descName );
			pProp->canTextureFeed( true );
			pProp->textureFeed( bw_utf8tow( textureFeed ) );
		}
		editor->addProperty( pProp );
	}
}


} // anonymous namespace


///////////////////////////////////////////////////////////////////////////////
// Section : MaterialProperties
///////////////////////////////////////////////////////////////////////////////

/**
 *	Important - this must be called at runtime, before you begin
 *	editing material properties.  The reason is that in
 *	moo/managed_effect the property processors are set up in the
 *	g_effectPropertyProcessors map at static initialisation time;
 *	our own processors are meant to override the default ones.
 */
/*static*/ bool MaterialProperties::runtimeInitMaterialProperties()
{
	BW_GUARD;

#define EP(x) Moo::g_effectPropertyProcessors[#x] = new x##ProxyFunctor
	EP(Vector4);
	EP(Matrix);
	EP(Float);
	EP(Bool);
	EP(Texture);
	EP(Int);
	EP(TextureFeed);

	return true;
}


/*static*/ void MaterialProperties::beginProperties()
{
	BW_GUARD;

	s_textureProxy.clear();
	s_textureFeedProxy.clear();
	s_boolProxy.clear();
	s_intProxy.clear();
	s_enumProxy.clear();
	s_floatProxy.clear();
	s_vector4Proxy.clear();
}


/*static*/ void MaterialProperties::endProperties()
{
	BW_GUARD;

	// Just clear maps again to be tidy.
	beginProperties();
}


/*static*/
bool MaterialProperties::addMaterialProperties( Moo::EffectMaterialPtr material, GeneralEditor * editor,
	bool canExposeToScript, bool canUseTextureFeeds, MaterialPropertiesUser * callback )
{
	BW_GUARD;

	if (!callback)
	{
		return false;
	}

	material->replaceDefaults();

	std::vector<Moo::EffectPropertyPtr> existingProps;

	if ( material->pEffect() )
	{
		Moo::EffectMaterial::Properties& properties = material->properties();
		Moo::EffectMaterial::Properties::reverse_iterator it = properties.rbegin();
		Moo::EffectMaterial::Properties::reverse_iterator end = properties.rend();

		for (; it != end; ++it )
		{
			MF_ASSERT( it->second );
			D3DXHANDLE hParameter = it->first;
			EditorEffectProperty* pProperty = dynamic_cast<EditorEffectProperty*>(it->second.get());

			if (pProperty && 
				MaterialUtility::artistEditable( pProperty ))
			{
				std::string uiName = MaterialUtility::UIName( pProperty );
				std::string uiDesc = MaterialUtility::UIDesc( pProperty );
				
				// Use descName for the UI if uiName doesn't exist... (bug 6940)
				if (uiName.empty()) uiName = pProperty->name();
				
				if (MaterialTextureProxy* pTextureProperty = dynamic_cast<MaterialTextureProxy*>( pProperty ))
				{
					addTextureProperty( pProperty->name(), uiName, uiDesc,
						pProperty, canExposeToScript, canUseTextureFeeds, callback, editor );
				}
				else if (MaterialTextureFeedProxy* pTextureFeedProperty = dynamic_cast<MaterialTextureFeedProxy*>( pProperty ))
				{
					addTextureProperty( pProperty->name(), uiName, uiDesc,
						pProperty, canExposeToScript, canUseTextureFeeds, callback, editor );
				}
				else if (MaterialBoolProxy* pBoolProperty = dynamic_cast<MaterialBoolProxy*>( pProperty ))
				{
					addBoolProperty( pProperty->name(), uiName, uiDesc,
						pProperty, canExposeToScript, canUseTextureFeeds, callback, editor );
				}
				else if (MaterialIntProxy* pIntProperty = dynamic_cast<MaterialIntProxy*>( pProperty ))
				{
					addEnumProperty( pProperty->name(), uiName, uiDesc,
						pProperty, canExposeToScript, canUseTextureFeeds, callback, editor );
				}
				else if (MaterialFloatProxy* pFloatProperty = dynamic_cast<MaterialFloatProxy*>( pProperty ))
				{
					addFloatProperty( pProperty->name(), uiName, uiDesc,
						pProperty, canExposeToScript, canUseTextureFeeds, callback, editor );
				}
				else if (MaterialVector4Proxy* pVector4Property = dynamic_cast<MaterialVector4Proxy*>( pProperty ))
				{
					addVector4Property( pProperty->name(), uiName, uiDesc,
						pProperty, canExposeToScript, canUseTextureFeeds, callback, editor );
				}
			}
		}
	}

	return true;
}
