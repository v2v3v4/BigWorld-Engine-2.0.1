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
#include "worldeditor/editor/editor_group.hpp"
#include "chunk/chunk_item.hpp"



DECLARE_DEBUG_COMPONENT2( "WorldEditor", 2 );


EditorGroup EditorGroup::s_rootGroup_ = EditorGroup("");


EditorGroup::EditorGroup( const std::string& name )
	: name_( name )
	, parent_( NULL )
	, treeItemHandle_( 0 )
{
	MF_ASSERT( name.find_first_of( "/" ) == std::string::npos );
}

EditorGroup::~EditorGroup()
{
	BW_GUARD;

	std::vector<EditorGroup*>::const_iterator k = children().begin();
	for (; k != children().end(); ++k)
	{
		EditorGroup* g = *k;

		delete g;
	}
}

void EditorGroup::name( const std::string& n )
{
	BW_GUARD;

	name_ = n;

	pathChanged();
}

std::string EditorGroup::fullName() const
{
	BW_GUARD;

	if (!parent_)
		return "";

	if (parent_ == rootGroup())
		return name_;

	return parent_->fullName() + "/" + name_;
}

void EditorGroup::enterGroup( ChunkItem* item )
{
	BW_GUARD;

	items_.push_back( item );

	//TODO : put back in when page scene is working correctly
	//PageScene::instance().addItemToGroup( item, this );
}

void EditorGroup::leaveGroup( ChunkItem* item )
{
	BW_GUARD;

	std::vector<ChunkItem*>::iterator i =
		std::find( items_.begin(), items_.end(), item );

	MF_ASSERT( i != items_.end() );

	items_.erase( i );

	//TODO : put back in when page scene is working correctly
	//PageScene::instance().removeItemFromGroup( item, this );
}

EditorGroup* EditorGroup::findOrCreateGroup( const std::string& fullName )
{
	BW_GUARD;

	EditorGroup* current = &s_rootGroup_;
	std::string currentName = fullName;
	char* currentPart = strtok( (char*) currentName.c_str(), "/" );


	while (1)
	{
		if (currentPart == NULL)
			return current;

		current = current->findOrCreateChild( currentPart );
		currentPart = strtok( NULL, "/" );
	}
}

EditorGroup* EditorGroup::findOrCreateChild( const std::string& name )
{
	BW_GUARD;

	MF_ASSERT( name.find_first_of( "/" ) == std::string::npos );

	std::vector<EditorGroup*>::iterator i;
	for (i = children_.begin(); i != children_.end(); ++i)
		if ((*i)->name() == name)
			return *i;

	EditorGroup* eg = new EditorGroup( name );
	eg->parent_ = this;
	children_.push_back( eg );
	return eg;
}

void EditorGroup::moveChunkItemsTo( EditorGroup* group )
{
	BW_GUARD;

	std::vector<ChunkItem*> groupItems = items_;
	std::vector<EditorGroup*> groups = children_;

	std::vector<ChunkItem*>::const_iterator j = groupItems.begin();
	for (; j != groupItems.end(); ++j)
	{
		ChunkItem* chunkItem = *j;

		chunkItem->edGroup( group );
	}

	std::vector<EditorGroup*>::const_iterator k = groups.begin();
	for (; k != groups.end(); ++k)
	{
		EditorGroup* g = *k;

		g->moveChunkItemsTo( group );
	}
}

void EditorGroup::removeChildGroup( EditorGroup* group )
{
	BW_GUARD;

	MF_ASSERT( group != this );

	group->moveChunkItemsTo( this );

	MF_ASSERT( group->items().empty() && group->children().empty() );

	std::vector<EditorGroup*>::iterator i = std::find( children_.begin(), children_.end(), group );

	if (i == children_.end())
		return;

	children_.erase( i );

	delete group;
}

void EditorGroup::pathChanged()
{
	BW_GUARD;

	std::vector<ChunkItem*> groupItems = items_;
	std::vector<EditorGroup*> groups = children_;

	// Notify out items their group path has changed
	std::vector<ChunkItem*>::const_iterator j = groupItems.begin();
	for (; j != groupItems.end(); ++j)
	{
		ChunkItem* chunkItem = *j;

		chunkItem->edGroup( this );
	}

	// Ask our child groups to notify their items too
	std::vector<EditorGroup*>::const_iterator k = groups.begin();
	for (; k != groups.end(); ++k)
	{
		EditorGroup* g = *k;

		g->pathChanged();
	}
}
