/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TEXTURE_FEEDS_HPP
#define TEXTURE_FEEDS_HPP

#include "moo/base_texture.hpp"
#include "cstdmf/init_singleton.hpp"
#include "py_texture_provider.hpp"

/// This token should be referenced to link in the TextureFeeds script functions.
extern int TextureFeeds_token;

typedef SmartPointer<PyTextureProvider>	PyTextureProviderPtr;

/**
 * TODO: to be documented.
 */
class TextureFeeds : public InitSingleton< TextureFeeds >
{
	typedef TextureFeeds This;
public:
	static void addTextureFeed( const std::string& identifier, PyTextureProviderPtr texture );
	PY_AUTO_MODULE_STATIC_METHOD_DECLARE( RETVOID, addTextureFeed, ARG( std::string, ARG( PyTextureProviderPtr, END ) ) )
	static void delTextureFeed( const std::string& identifier );
	PY_AUTO_MODULE_STATIC_METHOD_DECLARE( RETVOID, delTextureFeed, ARG( std::string, END ) )
	static PyTextureProviderPtr getTextureFeed( const std::string& identifier );
	PY_AUTO_MODULE_STATIC_METHOD_DECLARE( RETDATA, getTextureFeed, ARG( std::string, END ) )
	static Moo::BaseTexturePtr get( const std::string& identifier );

private:	
	typedef StringHashMap< PyTextureProviderPtr > Feeds;
	Feeds feeds_;
};

/**
 *	This class delegates the pTexture accessor between
 *	the base texture
 */
class TextureFeed : public Moo::BaseTexture
{
public:
	TextureFeed( const std::string& resourceID, Moo::BaseTexturePtr defaultTexture );

	//methods inherited from BaseTexture
	virtual DX::BaseTexture*	pTexture( );
	virtual uint32			width( ) const;
	virtual uint32			height( ) const;
	virtual D3DFORMAT		format( ) const;
	virtual uint32			textureMemoryUsed( );
	
private:
	const Moo::BaseTexturePtr current() const
	{
		Moo::BaseTexturePtr found = TextureFeeds::instance().get( resourceID_ );
		return found ? found : default_;
	}
	Moo::BaseTexturePtr		default_;
	std::string				resourceID_;
};

#endif
