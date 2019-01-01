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
#include "gizmo/link_view.hpp"
#include "gizmo/tool.hpp"
#include "gizmo/link_gizmo.hpp"
#include "moo/effect_visual_context.hpp"
#include "resmgr/auto_config.hpp"
#include "romp/geometrics.hpp"
#include "moo/visual_manager.hpp"

static AutoConfigString s_glVisual( "editor/graphLinkDragGizmo" );

namespace
{
    Moo::Colour     noTargetClr     = Moo::Colour(0.0f, 0.0f, 1.0f, 1.0f);
    Moo::Colour     canLinkClr      = Moo::Colour(0.0f, 1.0f, 0.0f, 1.0f);
    Moo::Colour     cannotLinkClr   = Moo::Colour(1.0f, 0.0f, 0.0f, 1.0f);
}


/**
 *  LinkView constructor.
 *
 *  @param linkProxy        A proxy used to help linking.
 *  @param startPos         The start position/orientation of the view.
 */
LinkView::LinkView(LinkProxyPtr linkProxy, Matrix const &startPos) :
    startPos_(startPos),
    linkProxy_(linkProxy),
    linkDrawMesh_(NULL)
{
	BW_GUARD;

	if ( !s_glVisual.value().empty() )
	{
		linkDrawMesh_ = Moo::VisualManager::instance()->get( s_glVisual );
	}
    buildMesh();
}


/**
 *  Draw the link view.
 *
 *  @param tool             The tool used to move the mouse.
 */
/*virtual*/ void LinkView::render(Tool const &tool)
{
	BW_GUARD;

    Matrix transform = tool.locator()->transform();
    LinkProxy::TargetState canLink = linkProxy_->canLinkAtPos(tool.locator());

    Moo::RenderContext *rc     = &Moo::rc();
    DX::Device         *device = rc->device();

	rc->push();

	Moo::Colour linkColour = noTargetClr;
	if ( canLink == LinkProxy::TS_CAN_LINK )
		linkColour = canLinkClr;
	else if ( canLink == LinkProxy::TS_CANT_LINK )
		linkColour = cannotLinkClr;


	if (linkDrawMesh_)
	{
		rc->fogEnabled( false );
		Moo::LightContainerPtr pOldLC = rc->lightContainer();
		Moo::LightContainerPtr pLC = new Moo::LightContainer;		
		pLC->ambientColour( linkColour );
		rc->lightContainer( pLC );
		rc->setPixelShader( NULL );		

		rc->push();		
		rc->world( transform );
		linkDrawMesh_->draw();
		rc->pop();

		rc->lightContainer( pOldLC );
	}


    rc->setRenderState(D3DRS_ALPHABLENDENABLE, TRUE                );
	rc->setRenderState(D3DRS_SRCBLEND        , D3DBLEND_SRCALPHA   );
	rc->setRenderState(D3DRS_DESTBLEND       , D3DBLEND_INVSRCALPHA);
    rc->fogEnabled( false );
	rc->setRenderState(D3DRS_LIGHTING        , FALSE               );

	rc->setTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );
	rc->setTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
	rc->setTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TFACTOR );
	rc->setTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
	rc->setTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    
	device->SetTransform(D3DTS_WORLD     , &transform       );
	device->SetTransform(D3DTS_VIEW      , &rc->view()      );
	device->SetTransform(D3DTS_PROJECTION, &rc->projection());

	rc->setPixelShader(NULL);
	rc->setVertexShader(NULL);
	rc->setFVF(Moo::VertexXYZND::fvf());

	rc->setRenderState( D3DRS_TEXTUREFACTOR, linkColour );

	if (!linkDrawMesh_)
		linkMesh_.draw(*rc);    

    Geometrics::drawLine
    (
        startPos_.applyToOrigin(),
        transform.applyToOrigin(),
        linkColour
    );

	rc->pop();
}


/**
 *  Build the mesh used to draw the view.
 */
void LinkView::buildMesh()
{
	BW_GUARD;

    linkMesh_.addSphere
    (
        Vector3(0.0f, 0.0f, 0.0f), 
        LinkGizmo::linkRadius, 
        (DWORD)canLinkClr
    );
}