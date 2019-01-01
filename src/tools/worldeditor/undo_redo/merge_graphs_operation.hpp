/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef MERGE_GRAPHS_OPERATION_HPP
#define MERGE_GRAPHS_OPERATION_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "gizmo/undoredo.hpp"
#include "cstdmf/unique_id.hpp"


/**
 *  This class undos/redos the merge of two graphs.
 */
class MergeGraphsOperation : public UndoRedo::Operation
{
public:
    MergeGraphsOperation
    (
        UniqueID                const &graph1Id,
        UniqueID                const &graph2Id
    );

	/*virtual*/ void undo();

	/*virtual*/ bool iseq( const UndoRedo::Operation & oth ) const;

protected:
    MergeGraphsOperation
    (
        UniqueID                const &graph1Id,
        UniqueID                const &graph2Id,
        std::vector<UniqueID>   const &nodes
    );

protected:
    UniqueID                    graph1Id_;
    UniqueID                    graph2Id_;
    std::vector<UniqueID>       nodes_;
};


#endif // MERGE_GRAPHS_OPERATION_HPP
