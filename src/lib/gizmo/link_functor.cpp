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
#include "gizmo/link_functor.hpp"
#include "gizmo/tool_manager.hpp"
#include "gizmo/current_general_properties.hpp"
#include "gizmo/link_gizmo.hpp"


/**
 *  LinkFunctor constructor.
 *
 *  @param linkProxy    A proxy to consult regarding linking.
 */
LinkFunctor::LinkFunctor(LinkProxyPtr linkProxy) :
    linkProxy_(linkProxy)
{
}


/**
 *  This is called every frame to update linking status.  Here we figure out
 *  movement by snapping movement and handling the case where the user releases
 *  the left-mouse button.
 *
 *  @param time     The current time.
 *  @param tool     The tool being used to enter the coordinates.
 */
/*virtual*/ void LinkFunctor::update(float /*time*/, Tool &tool)
{
	BW_GUARD;

    // Done yet?
    if (!InputDevices::isKeyDown(KeyCode::KEY_LEFTMOUSE))
    {
        Vector3 pos = tool.locator()->transform().applyToOrigin();
		LinkProxy::TargetState canLink = linkProxy_->canLinkAtPos(tool.locator());
        if ( canLink == LinkProxy::TS_CAN_LINK )
            linkProxy_->createLinkAtPos(tool.locator());
        ToolManager::instance().popTool();
    }
}


/**
 *  This is called to allow the user to cancel linking by pressing escape.
 *
 *  @param event    The key event.
 *  @param tool     The tool being used to enter coordinates.
 *  @returns        True if the event is process (if the user presses ESC),
 *                  false otherwise.
 */
/*virtual*/ bool 
LinkFunctor::handleKeyEvent
(
    KeyEvent        const &event, 
    Tool            &/*tool*/
)
{
	BW_GUARD;

    if (event.key() == KeyCode::KEY_ESCAPE)
    {
        ToolManager::instance().popTool();
		return true;
    }

    return false;
}


/**
 *  Called on every mouse event, which we ignore.
 *
 *  @param event    The mouse event.
 *  @param tool     The tool being used to enter coordinates.
 *  @returns        False, we don't process it.   
 */
/*virtual*/ bool 
LinkFunctor::handleMouseEvent
(
    MouseEvent      const &/*event*/, 
    Tool            &/*tool*/
)
{
    return false;
}


/**
 *  This is always true as the functor is always applying.
 *
 *  @returns        True.
 */
/*virtual*/ bool LinkFunctor::applying() const
{
    return true;
}
