/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TEXTURE_RENDERER_HPP
#define TEXTURE_RENDERER_HPP

#include <vector>
#include "cstdmf/smartpointer.hpp"
#include "moo/render_target.hpp"


class TextureRenderer;
typedef SmartPointer<TextureRenderer> TextureRendererPtr;

/**
 * TODO: to be documented.
 */
class TextureSceneInterface : public ReferenceCount
{
public:
	TextureSceneInterface();
	~TextureSceneInterface();

	virtual bool shouldDraw() const = 0;
	virtual void render( float dTime ) = 0;
	virtual bool cachable() const = 0;

	virtual uint skipFrames() const { return skipFrames_; }
	virtual void skipFrames( int value );
protected:
	uint					skipFrames_;
};

/**
 *	This class manages the rendering of things into a texture.
 *	They can be static or dynamic; if dynamic they are re-rendered
 *	every frame.
 */
class TextureRenderer : public TextureSceneInterface
{
public:
	TextureRenderer( int width, int height );
	~TextureRenderer();

	virtual void render( float dTime = 0.f );
	virtual void renderSelf( float dTime ) = 0;

	static void addDynamic( TextureSceneInterface * pTR );
	static void delDynamic( TextureSceneInterface * pTR );
	static void addStaggeredDynamic( TextureSceneInterface * pTR );
	static void delStaggeredDynamic( TextureSceneInterface * pTR );
	static void updateDynamics( float dTime );

	static void clearCachable( );
	static void updateCachableDynamics( float dTime );

	virtual void incRef() const		{ this->ReferenceCount::incRef(); }
	virtual void decRef() const		{ this->ReferenceCount::decRef(); }

	virtual bool shouldDraw() const	{ return true; }
	virtual bool cachable() const { return false; }

	void touchRenderTarget();

private:
	TextureRenderer( const TextureRenderer& );
	TextureRenderer& operator=( const TextureRenderer& );

	typedef std::vector<TextureSceneInterface*> TextureRenderers;
	static TextureRenderers s_dynamicRenderers_;
	static TextureRenderers s_staggeredRenderers_;
	static TextureRenderers s_cachableRenderers_;
	static TextureRenderers s_currentFrame_;
	static int staggerIdx_;
	static int stagger_cache_Idx_;

protected:
	static void addCachable( TextureSceneInterface * pTR );
	Moo::RenderTargetPtr	pRT_;
	int						width_;
	int						height_;
	bool					clearTargetEachFrame_;
};



#endif // TEXTURE_RENDERER_HPP
