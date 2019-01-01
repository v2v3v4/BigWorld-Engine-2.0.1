/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PY_GUI_ITEM_HPP_
#define PY_GUI_ITEM_HPP_

#include "pyscript/pyobject_plus.hpp"
#include "pyscript/script.hpp"
#include "gui_manager.hpp"

/**
 *	This class is used to expose a data section.
 */
class PyItem : public PyObjectPlus
{
	Py_Header( PyItem, PyObjectPlus )

public:
	PyItem( GUI::ItemPtr pItem,
		PyTypePlus * pType = &PyItem::s_type_ );
	~PyItem();

	GUI::ItemPtr	pItem() const;

	PyObject * 		pyGetAttribute( const char * attr );
	int 			pySetAttribute( const char * attr, PyObject * value );
	PyObject * 		subscript( PyObject * entityID );
	int				length();

	PY_METHOD_DECLARE( py_has_key )
	PY_METHOD_DECLARE( py_keys )
	PY_METHOD_DECLARE( py_values )
	PY_METHOD_DECLARE( py_items )

	PY_METHOD_DECLARE( py_createItem )
	PY_METHOD_DECLARE( py_deleteItem )

	PY_METHOD_DECLARE( py_copy )

	PyObject * child( int index );
	PY_AUTO_METHOD_DECLARE( RETOWN, child, ARG( int, END ) )
	PyObject * childName( int index );
	PY_AUTO_METHOD_DECLARE( RETOWN, childName, ARG( int, END ) )

	PY_RO_ATTRIBUTE_DECLARE( pItem_->name(), name )
	PY_RO_ATTRIBUTE_DECLARE( pItem_->displayName(), displayName )

	static PyObject * 	s_subscript( PyObject * self, PyObject * entityID );
	static int			s_length( PyObject * self );

	PY_FACTORY_DECLARE()

private:
	GUI::ItemPtr pItem_;
};

typedef SmartPointer<PyItem> PyItemPtr;

PY_SCRIPT_CONVERTERS_DECLARE( PyItem )

#endif // PY_GUI_ITEM_HPP_
