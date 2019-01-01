/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LINK_VIEW_HPP
#define LINK_VIEW_HPP


#include "gizmo/tool_view.hpp"
#include "gizmo/solid_shape_mesh.hpp"
#include "gizmo/link_proxy.hpp"


/**
 *  This class provides a view a draggable object when trying to link two
 *  objects.
 */
class LinkView : public ToolView
{
public:
    LinkView(LinkProxyPtr linkProxy, Matrix const &startPos);

    /*virtual*/ void render(Tool const &tool);

protected:
    void buildMesh();

private:
    LinkView(LinkView const &);
    LinkView &operator=(LinkView const &);	

protected:
    LinkProxyPtr        linkProxy_;
    SolidShapeMesh      linkMesh_;
	Moo::VisualPtr		linkDrawMesh_;    
    Matrix              startPos_;
};


#endif // LINK_VIEW_HPP
