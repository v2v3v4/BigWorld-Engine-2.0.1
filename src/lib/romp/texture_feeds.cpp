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
#include "texture_feeds.hpp"
#include "moo/material_loader.hpp"
#include "moo/effect_material.hpp"
#include "moo/managed_effect.hpp"
#include "moo/material.hpp"
#include "cstdmf/debug.hpp"
#include "resmgr/auto_config.hpp"

DECLARE_DEBUG_COMPONENT2( "romp", 0 )

int TextureFeeds_token = 0;


BW_INIT_SINGLETON_STORAGE( TextureFeeds )


/**
 *	The texture feed property implements a Moo::EffectProperty
 *	for TextureFeeds.  It is exactly the same as TextureProperty,
 *	which is not exposed from src/lib/moo and thus must be duplicated.
 */
class TextureFeedProperty : public Moo::EffectProperty
{
public:
	bool apply( ID3DXEffect* pEffect, D3DXHANDLE hProperty )
	{
		if (!value_.hasObject())
			return SUCCEEDED( pEffect->SetTexture( hProperty, NULL ) );
		return SUCCEEDED( pEffect->SetTexture( hProperty, value_->pTexture() ) );
	}
	
	bool be( const Moo::BaseTexturePtr pTex ) { value_ = pTex; return true; }

	void value( Moo::BaseTexturePtr value ) { value_ = value; };
	const Moo::BaseTexturePtr value() const { return value_; }
	virtual void save( DataSectionPtr pDS )
	{
		pDS->writeString( "TextureFeed", value_->resourceID() );
	}
	EffectProperty* clone() const
	{
		TextureFeedProperty* pClone = new TextureFeedProperty;
		pClone->value_ = value_;
		return pClone;
	}
protected:
	Moo::BaseTexturePtr value_;
};


/**
 *	The TextureFeedPropertyFunctor is registered with the
 *	Moo::g_effectPropertyProcessors map, and creates texture properties
 *	that use TextureFeeds by name.  They must have a default texture
 *	for when the texture feed is not available. 
 */
class TextureFeedPropertyFunctor : public Moo::EffectPropertyFunctor
{
public:
	virtual Moo::EffectPropertyPtr create( DataSectionPtr pSection )
	{
		static AutoConfigString s_notFoundBmp( "system/notFoundBmp" );
		TextureFeedProperty* prop = new TextureFeedProperty;
		Moo::BaseTexturePtr pDefaultTexture = Moo::TextureManager::instance()->get(
			pSection->readString( "default", s_notFoundBmp ) );
		Moo::BaseTexturePtr pTexture = new TextureFeed(
			pSection->asString(), pDefaultTexture );
		prop->value( pTexture );
		return prop;
	}
	virtual Moo::EffectPropertyPtr create( D3DXHANDLE hProperty, ID3DXEffect* pEffect )
	{
		TextureFeedProperty* prop = new TextureFeedProperty;
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
 *	This method reads a texture feed section in an old-style
 *	Moo::Material section.
 */
bool readTextureFeed( Moo::Material& mat, DataSectionPtr pSect )
{
	int numStages = mat.numTextureStages();
	if ( numStages > 0 )
	{
		Moo::TextureStage & ts = mat.textureStage( numStages-1 );
		ts.pTexture( new TextureFeed( pSect->asString(), ts.pTexture() ) );
	}
	return true;
}

/**
 *	This method registers the TextureFeedPropertyFunctor for use
 *	with Moo::EffectMaterials.
 */
void setupTextureFeedPropertyProcessors()
{
	Moo::SectionProcessors::instance().insert( Moo::SectProcEntry(
		"TextureFeed", &readTextureFeed ) );
	// if it is initialised, don't overwrite it
	if( !Moo::g_effectPropertyProcessors[ "TextureFeed" ] )
		Moo::g_effectPropertyProcessors[ "TextureFeed" ] = new TextureFeedPropertyFunctor;
}


/*static*/ void TextureFeeds::addTextureFeed( const std::string & identifier, PyTextureProviderPtr texture )
{
	instance().feeds_[ identifier ] = texture;
}


/*~ function BigWorld.addTextureFeed
 *	@components{ client, tools }
 *
 *	This adds a new TextureProvider to BigWorld, identified by a given label.  This label can then be used to refer 
 *	to the TextureProvider in other parts of BigWorld.
 *
 *	@param	identifier	A string label to identify this TextureProvider
 *	@param	texture		A PyTextureProviderPtr that is the TextureProvider to link to this label
*/
PY_MODULE_STATIC_METHOD( TextureFeeds, addTextureFeed, BigWorld )


/*static*/ void TextureFeeds::delTextureFeed( const std::string & identifier )
{
	if (pInstance() && !instance().feeds_.empty())
	{
		Feeds::iterator it = instance().feeds_.find( identifier );
		if (it != instance().feeds_.end())
		{
			instance().feeds_.erase( it );
		}
	}
}

/*~ function BigWorld.delTextureFeed
 *	@components{ client, tools }
 *
 *	This function removes a previously added TextureProvider.  The TextureProvider can still be used in script, but 
 *	cannot be referred to by its previously associated label anymore.
 *
 *	@param	identifier	A string label to identity the TextureProvider to remove
 */
PY_MODULE_STATIC_METHOD( TextureFeeds, delTextureFeed, BigWorld )


PyTextureProviderPtr TextureFeeds::getTextureFeed( const std::string& identifier )
{
	if (!instance().feeds_.empty())
	{
		Feeds::iterator it = instance().feeds_.find( identifier );
		if (it != instance().feeds_.end())
		{
			return it->second;
		}
	}
	return NULL;
}

/*~ function BigWorld.getTextureFeed
 *	@components{ client, tools }
 *
 *	This function returns a previously added TextureProvider.
 *
 *	@param	identifier	A string label to identity the TextureProvider to return
 */
PY_MODULE_STATIC_METHOD( TextureFeeds, getTextureFeed, BigWorld )


Moo::BaseTexturePtr TextureFeeds::get( const std::string& identifier ) 
{
	if (!instance().feeds_.empty())
	{
		Feeds::iterator it = instance().feeds_.find( identifier );
		if (it != instance().feeds_.end() && it->second)
		{
			return it->second->texture();
		}
	}
	return NULL;
}



TextureFeed::TextureFeed( const std::string& resourceID, Moo::BaseTexturePtr defaultTexture ):
	resourceID_( resourceID ),
	default_( defaultTexture )
{
	MF_ASSERT( default_ );
}


DX::BaseTexture* TextureFeed::pTexture( )
{
	return current()->pTexture();
}


uint32 TextureFeed::width( ) const
{
	return current()->width();
}


uint32 TextureFeed::height( ) const
{
	return current()->height();
}


D3DFORMAT TextureFeed::format( ) const
{
	return current()->format();
}


uint32 TextureFeed::textureMemoryUsed()
{
	return current()->textureMemoryUsed();
}