/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_ATTACHMENT_HPP
#define GUI_ATTACHMENT_HPP

#pragma warning( disable:4786 )

#include "simple_gui_component.hpp"
#include "duplo/py_attachment.hpp"

	/*~ class GUI.GuiAttachment
	 *	@components{ client, tools }
	 *
	 *	This class is a basic adaptor between PyAttachment and SimpleGUIComponent.
	 *
	 *	If you wish to display gui elements in the 3D scene ( and sorted into the scene )
	 *	then use this class as a container for your gui hierarchy, and attach this object
	 *	onto a PyModel or other PyAttachment object.
	 *
	 *	Note that if you wish to receive input like most gui components, then you can still
	 *	use the attached gui component's focus property as per normal.
	 *
	 *		eg,	guiSomeImage = GUI.Simple(imageFile)
	 *			guiAttach = GUI.Attachment()
	 *			guiAttach.component = guiSomeImage
	 *			self.model.rightHand = guiAttach	# Where rightHand is PyModelNode (model hard-point)
	 *			GUI.addRoot(guiSomeImage)
	 *
	 *		Current Model will now run around with the image showing in its right hand...
	 */
/**
 *	This class is a basic adaptor between PyAttachment and SimpleGUIComponent.
 *
 *	If you wish to display gui elements in the 3D scene ( and sorted into the scene )
 *	then use this class as a container for your gui hierarchy, and attach this object
 *	onto a PyModel or other PyAttachment object.
 *
 *	Note that if you wish to receive input like most gui components, then you can still
 *	use the attached gui component's focus property as per normal.
 */
class GuiAttachment : public PyAttachment
{
	Py_Header( GuiAttachment, PyAttachment )
	
public:
	GuiAttachment( PyTypePlus * pType = &s_type_ );
	virtual ~GuiAttachment();

	//These are PyAttachment methods
	virtual void tick( float dTime );
	virtual void draw( const Matrix & worldTransform, float lod );
	virtual void localBoundingBox( BoundingBox & bb, bool skinny = false );
	virtual void localVisibilityBox( BoundingBox & bb, bool skinny = false );
	virtual void worldBoundingBox( BoundingBox & bb, const Matrix& world, bool skinny = false );

	//C++ set the component
	void component( SmartPointer<SimpleGUIComponent> component );
	SmartPointer<SimpleGUIComponent> component() const;

	//This is the python interface
	PyObject *	pyGetAttribute( const char * attr );
	int			pySetAttribute( const char * attr, PyObject * value );

	PY_FACTORY_DECLARE()

	PY_RW_ATTRIBUTE_DECLARE( gui_, component )
	PY_RW_ATTRIBUTE_DECLARE( faceCamera_, faceCamera )
	PY_RW_ATTRIBUTE_DECLARE( reflectionVisible_, reflectionVisible )

private:
	SmartPointer<SimpleGUIComponent>	gui_;
	bool	faceCamera_;
	float	dTime_;
	uint32	tickCookie_;
	uint32	drawCookie_;
	bool reflectionVisible_;
};

#ifdef CODE_INLINE
#include "gui_attachment.ipp"
#endif

#endif