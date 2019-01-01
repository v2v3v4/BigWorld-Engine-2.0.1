/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef LATENCY_GUI_COMPONENT_HPP
#define LATENCY_GUI_COMPONENT_HPP

#include "ashes/simple_gui_component.hpp"

class TextGUIComponent;


/*~ class BigWorld.LatencyGUIComponent
 *
 *	This class is used to display the word "Offline" on the screen if the
 *  client is not connected to the server.  If the client is connected to the
 *	server, then the visible attribute of the LatencyGUIObject is automatically
 *	set to false.
 *
 *	This class inherits functionality from SimpleGUIComponent.
 */
/**
 *	This class is a GUI component to draw the latency meter
 */
class LatencyGUIComponent : public SimpleGUIComponent
{
	Py_Header( LatencyGUIComponent, SimpleGUIComponent )

public:
	LatencyGUIComponent( PyTypePlus* pType = &s_type_ );
	~LatencyGUIComponent();

	PyObject*	pyGetAttribute( const char* attr );
	int			pySetAttribute( const char* attr, PyObject* value );

	PY_FACTORY_DECLARE()

	void		draw( bool reallyDraw, bool overlay );

private:
	TextGUIComponent* txt_;

	LatencyGUIComponent(const LatencyGUIComponent&);
	LatencyGUIComponent& operator=(const LatencyGUIComponent&);
};



#ifdef CODE_INLINE
#include "latency_gui_component.ipp"
#endif

#endif
/*latency_gui_component.hpp*/