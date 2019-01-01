/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WINDOW_GUI_COMPONENT_HPP
#define WINDOW_GUI_COMPONENT_HPP

#include "simple_gui_component.hpp"

/*~ class GUI.WindowGUIComponent
 *	@components{ client, tools }
 *
 *	This class forms the basis of windowing functionality for the GUI.
 *	Children added to a window are clipped and scrolled inside it.
 *	Children of Window components inherit the translation of the window.
 *
 *  If a child has either of its position modes set to "CLIP" then 
 *  then this window is used as the basis for that child's clip space.
 *  In other words (-1,-1) is the bottom left of this window, and (1,1)
 *  is the top right of this window, and (0,0) is the centre of this
 *  window.
 *
 *  If a child has either of its position modes set to "PIXEL" then 
 *  the position coordinate will be relative to the top left of this
 *  window.
 */
/**
 *	This class implements a window that supports hierarchical
 *	transforms, a clipping area and has scrollable contents.
 */
class WindowGUIComponent : public SimpleGUIComponent
{
	Py_Header( WindowGUIComponent, SimpleGUIComponent )
	
public:
	WindowGUIComponent( const std::string& textureName = "",
		PyTypePlus * pType = &s_type_ );
	~WindowGUIComponent();

	void update( float dTime, float relParentWidth, float relParentHeight );
	void draw( bool reallyDraw, bool overlay = true );
	
	PyObject *			pyGetAttribute( const char * attr );
	int					pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_REF_DECLARE( scroll_, scroll );
	PY_RW_ATTRIBUTE_REF_DECLARE( scrollMin_, minScroll );
	PY_RW_ATTRIBUTE_REF_DECLARE( scrollMax_, maxScroll );

	PY_FACTORY_DECLARE()
protected:
	WindowGUIComponent(const WindowGUIComponent&);
	WindowGUIComponent& operator=(const WindowGUIComponent&);

	virtual void updateChildren( float dTime, float relParentWidth, float relParentHeight );
	virtual ConstSimpleGUIComponentPtr nearestRelativeParent(int depth=0) const;

	virtual Vector2 localToScreenInternalOffset( bool isThis ) const;

	bool load( DataSectionPtr pSect, const std::string& ownerName,  LoadBindings & bindings );
	void save( DataSectionPtr pSect, SaveBindings & bindings );

	Matrix scrollTransform_;
	Matrix anchorTransform_;
	Vector2 scroll_;
	Vector2 scrollMin_;
	Vector2 scrollMax_;
	
	COMPONENT_FACTORY_DECLARE( WindowGUIComponent() )
};

#ifdef CODE_INLINE
#include "window_gui_component.ipp"
#endif

#endif
