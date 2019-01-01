/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ITEM_FRUSTUM_LOCATOR_HPP
#define ITEM_FRUSTUM_LOCATOR_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/tool_locator.hpp"
#include "gizmo/tool_view.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "duplo/material_draw_override.hpp"


/**
 * This class is a locator that finds chunk items in a frustum
 * defined by the positions the mouse cursor has been in.
 *
 * It will generate a frustum aligned with the camera, with the same
 * near and far planes, and the corners being the extents the mouse
 * has been moved to.
 *
 * This is used to do drag selecting.
 *
 * Its matrix is determined by a supplemental locator set into the
 * class.
 */
class ChunkItemFrustumLocator : public ToolLocator
{
	Py_Header( ChunkItemFrustumLocator, ToolLocator )
public:
	ChunkItemFrustumLocator( ToolLocatorPtr pSubLoc = NULL,
		PyTypePlus * pType = &s_type_ );
	~ChunkItemFrustumLocator();

	static void fini();

	virtual void calculatePosition( const Vector3& worldRay, Tool& tool );

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_DECLARE( subLocator_, subLocator )

	PyObject * pyGet_revealer();
	PY_RO_ATTRIBUTE_SET( revealer )

	PY_RW_ATTRIBUTE_DECLARE( enabled_, enabled )

	PY_FACTORY_DECLARE()

	std::vector<ChunkItemPtr> items;
	
private:
	ChunkItemFrustumLocator( const ChunkItemFrustumLocator& );
	ChunkItemFrustumLocator& operator=( const ChunkItemFrustumLocator& );

	void enterSelectionMode();
	void leaveSelectionMode();

	Moo::Visual::DrawOverride* visualDrawOverride();

	ToolLocatorPtr	subLocator_;
	bool			enabled_;

	POINT startPosition_;
	POINT currentPosition_;

	Matrix oldView_;
	Matrix oldProjection_;
	Moo::Visual::DrawOverride* oldOverride_;

	static Moo::Visual::DrawOverride* s_override_;

	bool oldDrawReflection_;

	friend class DragBoxView;
};

typedef SmartPointer<ChunkItemFrustumLocator> ChunkItemFrustumLocatorPtr;

PY_SCRIPT_CONVERTERS_DECLARE( ChunkItemFrustumLocator )


class DragBoxView : public ToolView
{
	Py_Header( DragBoxView, ToolView )

public:
	DragBoxView( ChunkItemFrustumLocatorPtr locator, Moo::Colour colour,
		PyTypePlus * pType = &s_type_ );
	~DragBoxView();

	virtual void viewResource( const std::string& resourceID )
		{ resourceID_ = resourceID; }
	virtual void render( const class Tool& tool );

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PY_AUTO_CONSTRUCTOR_FACTORY_DECLARE( DragBoxView,
		NZARG( ChunkItemFrustumLocatorPtr, ARG( uint, END ) ) )
private:
	DragBoxView( const DragBoxView& );
	DragBoxView& operator=( const DragBoxView& );

	ChunkItemFrustumLocatorPtr locator_;
	Moo::Colour colour_;
};

#endif // ITEM_FRUSTUM_LOCATOR_HPP
