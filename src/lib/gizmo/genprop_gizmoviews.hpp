/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GENPROP_GIZMOVIEWS_HPP
#define GENPROP_GIZMOVIEWS_HPP


#include "general_editor.hpp"

/**
 *	This class maintains the kind id used by gizmo property views.
 */
class GizmoViewKind
{
public:
	static int kindID()
	{
		static int kid = GeneralProperty::nextViewKindID();
		return kid;
	}
};



#endif // GENPROP_GIZMOVIEWS_HPP
