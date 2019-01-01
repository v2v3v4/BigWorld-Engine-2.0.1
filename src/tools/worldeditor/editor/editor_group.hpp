/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_GROUP_HPP
#define EDITOR_GROUP_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"


class EditorGroup
{
public:
	EditorGroup( const std::string& name );
	~EditorGroup();

	const std::string& name() const { return name_; }
	void name( const std::string& n );
	std::string fullName() const;
	EditorGroup* parent() const { return parent_; }

	const std::vector<EditorGroup*>& children() const { return children_; }
	const std::vector<ChunkItem*>& items() const { return items_; }

	void enterGroup( ChunkItem* item );
	void leaveGroup( ChunkItem* item );

	void treeItemHandle( uint32 hItem ) { treeItemHandle_ = hItem; }
	uint32 treeItemHandle() const { return treeItemHandle_; }

	/**
	 *	Return the child group with the given name, slashes aren't allowed
	 */
	EditorGroup* findOrCreateChild( const std::string& name );

	/** Remove the given group from us, and delete it for good measure */
	void removeChildGroup( EditorGroup* group );

	/**
	 *	Return the group with the given name, in the form of a/b/c
	 */
	static EditorGroup* findOrCreateGroup( const std::string& fullName );

	static EditorGroup* rootGroup() { return &s_rootGroup_; }
private:
	std::string name_;
	EditorGroup* parent_;
	std::vector<ChunkItem*> items_;
	std::vector<EditorGroup*> children_;

	uint32 treeItemHandle_;

	/** Call when the parent or name of this group has changed */
	void pathChanged();

	/** Recursively move all our items to the given group */
	void moveChunkItemsTo( EditorGroup* group );

	static EditorGroup s_rootGroup_;
};

#endif // EDITOR_GROUP_HPP
