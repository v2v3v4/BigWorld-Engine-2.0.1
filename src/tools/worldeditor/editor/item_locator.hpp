/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ITEM_LOCATOR_HPP
#define ITEM_LOCATOR_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/tool_locator.hpp"


/**
 *	This class is a locator that finds a chunk item under the cursor.
 *	Its matrix is determined by a supplemental locator set into the
 *	class.
 */
class ChunkItemLocator : public ToolLocator
{
	Py_Header( ChunkItemLocator, ToolLocator )
public:
	ChunkItemLocator( ToolLocatorPtr pSubLoc = NULL,
		PyTypePlus * pType = &s_type_ );
	~ChunkItemLocator();

	virtual void calculatePosition( const Vector3& worldRay, Tool& tool );

	virtual bool positionValid() const { return positionValid_; }

	ChunkItemPtr	chunkItem();

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PY_RW_ATTRIBUTE_DECLARE( subLocator_, subLocator )

	PyObject * pyGet_revealer();
	PY_RO_ATTRIBUTE_SET( revealer )

	PY_RW_ATTRIBUTE_DECLARE( enabled_, enabled )

	PY_FACTORY_DECLARE()

private:
	ChunkItemLocator( const ChunkItemLocator& );
	ChunkItemLocator& operator=( const ChunkItemLocator& );

	ToolLocatorPtr	subLocator_;
	ChunkItemPtr	chunkItem_;
	bool			enabled_;
	bool			positionValid_;
};


/**
 *	This class implements a tool locator that sits on the xz plane,
 *	with the y value being set by the last item interaction.
 *
 *	Additionally, XZ snaps are taken into account.
 */
class ItemToolLocator : public ToolLocator
{
	Py_Header( ItemToolLocator, ToolLocator )
public:
	ItemToolLocator( PyTypePlus * pType = &s_type_ );
	virtual void calculatePosition( const Vector3& worldRay, Tool& tool );

	virtual bool positionValid() const { return positionValid_; }
	/** If the position can not be set
	  * because the raycast from the camera to the world
	  * does not hit anything.
	  * Keep the raycast direction, as we can use this when placing
	  * the objects relate to the camera 
	  */
	virtual Vector3 direction() const { return direction_; }

	PY_FACTORY_DECLARE()
private:
	bool			positionValid_;
	Vector3			direction_;

	LOCATOR_FACTORY_DECLARE( ItemToolLocator() )
};


#endif // ITEM_LOCATOR_HPP
