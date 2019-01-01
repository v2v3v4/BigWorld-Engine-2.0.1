/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	Model Thumbnail Provider
 */

#include "pch.hpp"
#include <string>

#include "moo/render_context.hpp"
#include "moo/effect_manager.hpp"
#include "moo/light_container.hpp"
#include "moo/visual_channels.hpp"
#include "moo/visual_manager.hpp"
#include "moo/visual.hpp"

#include "math/boundbox.hpp"
#include "math/matrix.hpp"

#include "model_thumb_prov.hpp"

#include "common/string_utils.hpp"

#include "cstdmf/string_utils.hpp"


// Token so this file gets linked in by the linker
int ModelThumbProv_token;


DECLARE_DEBUG_COMPONENT( 0 )


// Implement the Model Thumbnail Provider Factory
IMPLEMENT_THUMBNAIL_PROVIDER( ModelThumbProv )


/**
 *	Constuctor.
 */
ModelThumbProv::ModelThumbProv():
	lights_(NULL),
	visual_(NULL)
{
	BW_GUARD;
}


/**
 *	Destuctor.
 */
ModelThumbProv::~ModelThumbProv()
{
	BW_GUARD;

	lights_ = NULL;
}


/**
 *	This method tells the manager if the asset specified in 'file' can be
 *	handled by the ModelThumbProv, i.e. if it's a .visual or a .model file.
 *
 *	@param manager	ThumbnailManager instance that is dealing with the asset.
 *	@param file		Asset requesting the thumbnail
 *	@return			True if the asset is a model or visual file.
 */
bool ModelThumbProv::isValid( const ThumbnailManager& manager, const std::wstring& file )
{
	BW_GUARD;

	if ( file.empty() )
		return false;

	std::wstring ext = file.substr( file.find_last_of( L'.' ) + 1 );
	return wcsicmp( ext.c_str(), L"model" ) == 0
		|| wcsicmp( ext.c_str(), L"visual" ) == 0;
}


/**
 *	This method is called to find out if the image for the asset needs to be
 *	created in the background thread.  For models and visuals, it only needs
 *	to be created if the thumbnail image file for it doesn't exist.
 *
 *	@param manager	ThumbnailManager instance that is dealing with the asset.
 *	@param file		Asset requesting the thumbnail.
 *	@param thumb	Returns the path to the thumbnail img file for this asset.
 *	@param size		Returns the desired size of this image, 128 for models.
 *	@return			True if there's no thumbnail image created for the model.
 */
bool ModelThumbProv::needsCreate( const ThumbnailManager& manager, const std::wstring& file, std::wstring& thumb, int& size )
{
	BW_GUARD;

	size = 128; // Models want 128 x 128 thumbnails
	
	std::string nfile;
	bw_wtoutf8( file, nfile );

	std::string basename = BWResource::removeExtension( nfile );
	if ( BWResource::getExtension( basename ) == "static" )
	{
		// it's a visual with two extensions, so remove the remaining extension
		basename = BWResource::removeExtension( basename );
	}

	std::string thumbName = basename + ".thumbnail.jpg";

	bw_utf8tow( thumbName, thumb );

	if ( PathFileExists( thumb.c_str() ) )
	{
		return false;
	}
	else
	{
		thumbName = basename + ".thumbnail.bmp";

		std::wstring wthumb;
		bw_utf8tow( thumbName, wthumb );
		if ( PathFileExists( wthumb.c_str() ) )
		{
			// change the thumbnail path to the legacy one
			thumb = wthumb;
			return false;
		}
	}
	return true;
}



/**
 *	This method is called from the bg thread to prepare the thumbnail, so here
 *	we just load the model and get it ready for rendering.
 *
 *	@param manager	ThumbnailManager instance that is dealing with the asset.
 *	@param file		Model file asset requesting the thumbnail.
 *	@return			True if the model was loaded correctly, false otherwise.
 */
bool ModelThumbProv::prepare( const ThumbnailManager& manager, const std::wstring& file )
{
	BW_GUARD;

	std::wstring visualName = file;
	std::string nfile;
	bw_wtoutf8( file, nfile );
	std::string modelName = BWResource::dissolveFilename( nfile );
	bool errored = (errorModels_.find( modelName ) != errorModels_.end());

	if ( BWResource::getExtension( nfile ) != "visual" )
	{
		DataSectionPtr model = BWResource::openSection( modelName, false );

		if (model == NULL) 
		{
			if (errored) return false;
			ERROR_MSG( "ModelThumbProv::create: Could not open model file"
				" \"%s\"\n", modelName.c_str() );
			errorModels_.insert( modelName );
			return false;
		}

		static const char * visuals [] = { "nodefullVisual", "nodelessVisual", "billboardVisual" };
		for ( int idx = 0 ; idx < ARRAY_SIZE( visuals ) ; ++idx )
		{
			visualName = model->readWideString( visuals[ idx ] );
			if ( visualName != L"" )
				break;
		}
		if ( visualName == L"" )
		{
			if (errored) return false;
			ERROR_MSG( "ModelThumbProv::create: Could not determine type of model"
				" in file \"%s\"\n", modelName.c_str() );
			errorModels_.insert( modelName );
			return false;
		}
		visualName += L".visual";
	}

	std::string nvisualName;
	bw_wtoutf8( visualName, nvisualName );
	visual_ = Moo::VisualManager::instance()->get( nvisualName );
	if (visual_ == NULL)
	{
		if (errored) 
			return false;
		ERROR_MSG( "ModelThumbProv::create: Couldn't load visual \"%s\"\n", nvisualName.c_str() );
		errorModels_.insert( modelName );
		return false;
	}

	return true;
}


/**
 *	This method is called from the main thread to render the prepared model
 *	into the passed-in render target of the thumbnail manager.
 *
 *	@param manager	ThumbnailManager instance that is dealing with the asset.
 *	@param file		Model file asset requesting the thumbnail.
 *	@param rt		Render target to render the prepared model to.
 *	@return			True if the thumbnail was rendered, false otherwise.
 */
bool ModelThumbProv::render( const ThumbnailManager& manager, const std::wstring& file, Moo::RenderTarget* rt )
{
	BW_GUARD;

	if ( !visual_ )
		return false;

	Matrix rotation;
	
	if (lights_ == NULL)
	{
		lights_ = new Moo::LightContainer;

		lights_->ambientColour( Moo::Colour( 0.75f, 0.75f, 0.75f, 1.f ));

		Matrix dir (Matrix::identity);
		rotation.setRotateX( - MATH_PI / 4.f );
		dir.preMultiply( rotation );
		rotation.setRotateY( + MATH_PI / 4.f );
		dir.preMultiply( rotation );

		Moo::DirectionalLightPtr pDir = new Moo::DirectionalLight( Moo::Colour( 0.75f, 0.75f, 0.5f, 1.f ), dir[2] );
		pDir->worldTransform( Matrix::identity );
		lights_->addDirectional( pDir );

		dir = Matrix::identity;
		rotation.setRotateX( + MATH_PI / 8.f );
		dir.preMultiply( rotation );
		rotation.setRotateY( - MATH_PI / 4.f );
		dir.preMultiply( rotation );

		pDir = new Moo::DirectionalLight( Moo::Colour( 0.75f, 0.75f, 0.75f, 1.f ), dir[2] );
		pDir->worldTransform( Matrix::identity );
		lights_->addDirectional( pDir );
	}
	/* Flush any events queued by prepare so they are available to 
		 * render the thumbnails
		 */
	Moo::EffectManager::instance().finishEffectInits();
	//Make sure we set this before we try to draw
	Moo::rc().setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	Moo::rc().device()->Clear( 0, NULL,
		D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, RGB( 192, 192, 192 ), 1, 0 );

	// Set the projection matrix
	Moo::Camera cam = Moo::rc().camera();
	cam.aspectRatio( 1.f );
	Moo::rc().camera( cam );
	Moo::rc().updateProjectionMatrix();

	// Set a standard view
	Matrix view (Matrix::identity);
	Moo::rc().world( view );
	rotation.setRotateX( - MATH_PI / 8.f );
	view.preMultiply( rotation );
	rotation.setRotateY( + MATH_PI / 8.f );
	view.preMultiply( rotation );
	Moo::rc().view( view );

	// Zoom to the models bounding box
	zoomToExtents( visual_->boundingBox() );

	// Set up the lighting
	Moo::LightContainerPtr oldLights = Moo::rc().lightContainer();
	Moo::rc().lightContainer( lights_ );
	
	// Draw the model
	visual_->draw();

	// Draw any sorted channels
	Moo::SortedChannel::draw();

	Moo::rc().lightContainer( oldLights );

	//Make sure we restore this after we are done
	Moo::rc().setRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );

	visual_ = NULL;

	return true;
}
