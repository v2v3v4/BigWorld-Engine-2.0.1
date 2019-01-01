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
#include "worldeditor/undo_redo/entity_array_undo.hpp"
#include "common/base_properties_helper.hpp"


EntityArrayUndo::EntityArrayUndo( BasePropertiesHelper* props, int index ) :
	UndoRedo::Operation(int(typeid(EntityArrayUndo).name())),
    props_( props ),
	index_( index )
{
	BW_GUARD;

	undoData_ = props_->propGet( index_ );
}


/*virtual*/ void EntityArrayUndo::undo()
{
	BW_GUARD;

    UndoRedo::instance().add(new EntityArrayUndo( props_, index_ ));
	props_->propSet( index_, undoData_ );
}


/*virtual*/ bool EntityArrayUndo::iseq( const UndoRedo::Operation& other ) const
{
	return false;
}
