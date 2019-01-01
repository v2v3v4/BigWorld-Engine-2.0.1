/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef USER_DATA_OBJECT_LINK_OPERATION_HPP
#define USER_DATA_OBJECT_LINK_OPERATION_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_user_data_object.hpp"
#include "gizmo/undoredo.hpp"
#include <string>


/**
 *  This class saves the link information for a User Data Object with this
 *  property and undoes changes to this link information.
 */
class UserDataObjectLinkOperation : public UndoRedo::Operation
{
public:
    explicit UserDataObjectLinkOperation
    (
        EditorChunkUserDataObjectPtr        udo,
		const std::string&					linkName
    );

	/*virtual*/ void undo();

    /*virtual*/ bool iseq(UndoRedo::Operation const &other) const;

private:
    UserDataObjectLinkOperation( UserDataObjectLinkOperation const & );
    UserDataObjectLinkOperation &operator=( UserDataObjectLinkOperation const & );

protected:
    std::string						linkName_;
    std::string						udoLink_;
    EditorChunkUserDataObjectPtr	udo_;
};


#endif // USER_DATA_OBJECT_LINK_OPERATION_HPP
