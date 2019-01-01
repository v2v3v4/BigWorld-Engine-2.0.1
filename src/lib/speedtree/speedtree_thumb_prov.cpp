/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


// Module Interface
#include "speedtree_thumb_prov.hpp"
#include "speedtree_renderer.hpp"

#if SPEEDTREE_SUPPORT // -------------------------------------------------------

#include "resmgr/bwresource.hpp"
#include "math/boundbox.hpp"
#include "moo/render_context.hpp"
#include "moo/effect_manager.hpp"
#include "moo/material.hpp"
#include "cstdmf/string_utils.hpp"

int SpeedTreeThumbProv_token;
DECLARE_DEBUG_COMPONENT2("SpeedTree", 0)

namespace speedtree {

// Thumbnail Provider
IMPLEMENT_THUMBNAIL_PROVIDER(SpeedTreeThumbProv)

/**
 *	Returns true if this is a SpeedTree (".spt") file.
 */
bool SpeedTreeThumbProv::isValid(const ThumbnailManager& manager, const std::wstring& file)
{
	BW_GUARD;
	if (file.empty())
		return false;

	std::string nfile;
	bw_wtoutf8(  file, nfile );
	std::string ext = BWResource::getExtension(nfile);
	return ext == "spt" || ext == "SPT";
}

/**
 *	Loads a speedtree for later rendering.
 */
bool SpeedTreeThumbProv::prepare(const ThumbnailManager& manager, const std::wstring& file)
{
	BW_GUARD;
	bool result = false;
	try
	{
		std::string nfile;
		bw_wtoutf8(  file, nfile );

		// load the tree
		tree_ = new SpeedTreeRenderer();
		std::string resFile = BWResource::dissolveFilename(nfile);
		tree_->load(resFile .c_str(), 1, Matrix::identity);
		
		result = true;
	}
	catch (const std::runtime_error &err)
	{
		ERROR_MSG("Error loading tree: %s\n", err.what());
		if ( tree_ )
		{
			delete tree_;
			tree_ = 0;
		}
	}
	return result;
}

/**
 *	Creates the thumbnail image. Renders speedtree previously loaded in the
 *	prepare method into render target provided.
 */
bool SpeedTreeThumbProv::render(const ThumbnailManager& manager, const std::wstring& file, Moo::RenderTarget* rt)
{
	BW_GUARD;
	if ( !tree_ )
		return false;

	bool result = false;
	try
	{
		/* Flush any events queued by prepare so they are available to 
		 * render the thumbnails
		 */
		Moo::EffectManager::instance().finishEffectInits();
		Moo::rc().device()->Clear( 0, NULL,
			D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 
			RGB(192, 192, 192), 1, 0 );

		// set view matrix
		Moo::rc().view(Matrix::identity);
		this->zoomToExtents(tree_->boundingBox(), 0.8f);
		
		// set world and 
		// projection matrices
		Matrix projection;
		projection.perspectiveProjection( DEG_TO_RAD(60.0f), 1.0f, 1.0f, 5000.0f);
		Moo::rc().device()->SetTransform( D3DTS_PROJECTION, &projection );
		Moo::rc().world(Matrix::identity);

		// force max lod level	
		float lodMode = SpeedTreeRenderer::lodMode(1.0f);
		
		// render the tree
		SpeedTreeRenderer::beginFrame(NULL);
		tree_->draw(Matrix::identity,0);
		SpeedTreeRenderer::endFrame();
		
		// restore previous lod level
		SpeedTreeRenderer::lodMode(lodMode);	
		result = true;
	}
	catch (const std::runtime_error &err)
	{
		ERROR_MSG("Error loading tree: %s\n", err.what());
	}
	delete tree_;
	tree_ = 0;
	return result;
}

} // namespace speedtree
#endif // SPEEDTREE_SUPPORT
