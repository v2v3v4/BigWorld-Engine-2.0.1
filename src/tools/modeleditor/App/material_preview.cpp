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
#include "material_preview.hpp"

#include "moo/render_target.hpp"
#include "moo/texture_manager.hpp"
#include "page_materials.hpp"
#include "romp/geometrics.hpp"

#include "me_app.hpp"

DECLARE_DEBUG_COMPONENT2( "MaterialPreview", 0 )


BW_SINGLETON_STORAGE( MaterialPreview )


/**
 *	Constructor
 */
MaterialPreview::MaterialPreview() :
	hasNew_( false ),
	needsUpdate_( false ),
	updating_( false )
{
}

		
/**
 *	This method checks to see if the material preview needs to be updated, and
 *	if so, it updates the render target. 
 */
void MaterialPreview::update()
{
	BW_GUARD;

	if (!needsUpdate_ || updating_ ||
		(PageMaterials::currPage() == NULL) ||
		(PageMaterials::currPage()->currMaterial() == NULL))
	{
			return;
	}

	updating_ = true;
	needsUpdate_ = false;

	std::string fileName = BWResource::resolveFilename( "resources/material_preview.bmp" );

	//Grab diffuse map file location as string
	std::string textureString = MeApp::instance().mutant()->materialPropertyVal(
		bw_wtoutf8( PageMaterials::currPage()->materialName() ),
		bw_wtoutf8( PageMaterials::currPage()->matterName() ),
		bw_wtoutf8( PageMaterials::currPage()->tintName() ),
		"diffuseMap",
		"Texture" );

	//If there is a diffuse map convert it to .bmp
	if ( textureString != "")
	{
		HRESULT hr = D3DXSaveTextureToFile( bw_utf8tow( fileName ).c_str(), D3DXIFF_BMP,
			Moo::TextureManager::instance()->get( textureString )->pTexture(),
			NULL );

		if (FAILED( hr ))
		{
			WARNING_MSG( "Could not create material preview file (D3D error = 0x%x).\n", hr);
		}
	}
	//Else try to grab its colour and render it to the .bmp, if that fails render white.
	else
	{
		std::string colourString = MeApp::instance().mutant()->materialPropertyVal(
			bw_wtoutf8( PageMaterials::currPage()->materialName() ),
			bw_wtoutf8( PageMaterials::currPage()->matterName() ),
			bw_wtoutf8( PageMaterials::currPage()->tintName() ),
			"Color",
			"Vector4" );

		if (colourString == "")
		{
			colourString = MeApp::instance().mutant()->materialPropertyVal(
				bw_wtoutf8( PageMaterials::currPage()->materialName() ),
				bw_wtoutf8( PageMaterials::currPage()->matterName() ),
				bw_wtoutf8( PageMaterials::currPage()->tintName() ),
				"Colour",
				"Vector4" );
		}

		if (colourString == "")
		{
			colourString = "1 1 1 0";
		}

		Moo::Colour col;
		sscanf( colourString.c_str(), "%f %f %f %f", &col.r, &col.g, &col.b, &col.a );
	
		if (!rt_)
		{
			rt_ = new Moo::RenderTarget( "Material Preview" );
			if (!rt_ || !rt_->create( 128,128 ))
			{
				WARNING_MSG( "Could not create render target for material preview.\n" );
				rt_ = NULL;
				updating_ = false;
				return;
			}
		}

		if (rt_->push())
		{
			Moo::rc().beginScene();
		
			Moo::rc().device()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
				RGB( 255, 255, 255), 1, 0 );
			
			Geometrics::drawRect(
				Vector2( 0, 0 ),
				Vector2( 128, 128 ),
				col );

			Moo::rc().endScene();
			
			if(!fileName.empty())
			{
				HRESULT hr = D3DXSaveTextureToFile( bw_utf8tow( fileName ).c_str(), D3DXIFF_BMP, rt_->pTexture(), NULL );
					
				if (FAILED( hr ))
				{
					WARNING_MSG( "Could not create material preview file (D3D error = 0x%x).\n", hr);
				}
			}

			rt_->pop();

		}
	}

	hasNew_ = true;
	updating_ = false;
}
