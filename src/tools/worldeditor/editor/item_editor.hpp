/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ITEM_EDITOR_HPP
#define ITEM_EDITOR_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/general_editor.hpp"


/**
 *	This class controls and defines the editing operations which
 *	can be performed on a chunk item.
 */
class ChunkItemEditor : public GeneralEditor
{
	Py_Header( ChunkItemEditor, GeneralEditor )
public:
	ChunkItemEditor( ChunkItemPtr pItem, PyTypePlus * pType = &s_type_ );
	~ChunkItemEditor();

	PyObject *		pyGetAttribute( const char * attr );
	int				pySetAttribute( const char * attr, PyObject * value );

	PY_DEFERRED_ATTRIBUTE_DECLARE( className )
	PY_DEFERRED_ATTRIBUTE_DECLARE( description )
	PY_DEFERRED_ATTRIBUTE_DECLARE( transform )

	PY_FACTORY_DECLARE()

private:
	ChunkItemEditor( const ChunkItemEditor& );
	ChunkItemEditor& operator=( const ChunkItemEditor& );

	ChunkItemPtr	pItem_;

};


#endif // ITEM_EDITOR_HPP
