/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LINK_GIZMO_HPP
#define LINK_GIZMO_HPP


#include "gizmo/gizmo_manager.hpp"
#include "gizmo/solid_shape_mesh.hpp"
#include "gizmo/link_proxy.hpp"
#include "input/input.hpp"


struct  LinkGizmoColourPart;


/**
 *  This class represents a Gizmo that allows items to be interactively
 *  linked together.
 */
class LinkGizmo : public Gizmo, public Aligned
{
public:
    LinkGizmo
    (
        LinkProxyPtr    linkProxy,
        MatrixProxyPtr  center, 
        uint32          enablerModifier     = MODIFIER_ALT
    );

    /*virtual*/ ~LinkGizmo();

	void update( LinkProxyPtr linkProxy, MatrixProxyPtr center );

    /*virtual*/ bool draw(bool force);

    /*virtual*/ bool intersects( Vector3 const &origin,
							Vector3 const &direction, float &t, bool force );

    void click(Vector3 const &origin, Vector3 const &direction);

    void rollOver(Vector3 const &origin, Vector3 const &direction);

    static Vector3  rightLinkCenter;
    static Vector3  leftLinkCenter;
    static float    linkRadius;
    static DWORD    linkColour;

    static Vector3  centerAddUL;
    static Vector3  centerAddLR;
    static float    heightAdd;
    static float    widthAdd;
    static float    breadthAdd;
    static DWORD    addColour;

    static DWORD    unlitColour;

protected:
    void rebuildMeshes(bool force);

    Matrix objectTransform() const;
	Matrix gizmoTransform() const;

    void clickAdd(Vector3 const &origin, Vector3 const &direction, bool upper);

    void clickLink(Vector3 const &origin, Vector3 const &direction, bool left);

private:
    LinkGizmo(LinkGizmo const &);               // not permitted
    LinkGizmo &operator=(LinkGizmo const &);    // not permitted

protected:
	void init();

    LinkProxyPtr        linkProxy_;
    MatrixProxyPtr      center_;
	bool				active_;
	bool				inited_;
	uint32              enablerModifier_;
    SolidShapeMesh      linkSelectionMesh_;
	Moo::VisualPtr		linkDrawMesh_;
    SolidShapeMesh      addSelectionMesh_;
	Moo::VisualPtr		addDrawMesh_;
    Moo::Colour         hilightColour_;
    LinkGizmoColourPart *currentPart_;
    Vector3             currentPos_;
};


typedef SmartPointer< LinkGizmo > LinkGizmoPtr;

#endif // LINK_GIZMO_HPP
