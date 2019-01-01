/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LINK_FUNCTOR_HPP
#define LINK_FUNCTOR_HPP


#include "gizmo/tool_functor.hpp"
#include "gizmo/link_proxy.hpp"


/**
 *  This functor helps to link items.
 */
class LinkFunctor : public ToolFunctor
{
public:
    LinkFunctor(LinkProxyPtr linkProxy);

    /*virtual*/ void update(float time, Tool &tool);
    
    /*virtual*/ bool handleKeyEvent(KeyEvent const &event, Tool &tool);

    /*virtual*/ bool handleMouseEvent(MouseEvent const &event, Tool &tool);

    /*virtual*/ bool applying() const;

private:
    LinkFunctor(LinkFunctor const &);
    LinkFunctor &operator=(LinkFunctor const &);

protected:
    LinkProxyPtr            linkProxy_;
};


#endif // LINK_FUNCTOR_HPP
