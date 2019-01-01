/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef ENTITY_ARRAY_UNDO_HPP
#define ENTITY_ARRAY_UNDO_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/undoredo.hpp"


/**
 *	This class handles undoing array operations
 */
class EntityArrayUndo : public UndoRedo::Operation
{
public:
    explicit EntityArrayUndo( BasePropertiesHelper* props, int index );

	/*virtual*/ void undo();

    /*virtual*/ bool iseq( UndoRedo::Operation const &other ) const;

protected:
    BasePropertiesHelper* props_;
	int index_;
	DataSectionPtr undoData_;

private:
	EntityArrayUndo( const EntityArrayUndo& );
	EntityArrayUndo &operator=( const EntityArrayUndo& );
};


#endif // ENTITY_ARRAY_UNDO_HPP
