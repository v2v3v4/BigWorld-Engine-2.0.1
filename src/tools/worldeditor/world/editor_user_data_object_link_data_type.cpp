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
#include "worldeditor/world/user_data_object_link_proxy.hpp"
#include "worldeditor/world/editor_chunk_item_linker.hpp"
#include "worldeditor/world/items/editor_chunk_entity.hpp"
#include "worldeditor/world/items/editor_chunk_user_data_object.hpp"
#include "chunk/user_data_object_link_data_type.hpp"
#include "gizmo/link_property.hpp"


/**
 *  This method returns the appropriate editor property for a UDO_REF link.
 *	This method is declared in "user_data_object_link_data_type" as editor only
 *  and is not implemented in the user_data_object_link_data_type.cpp file, so
 *  EDITOR_ENABLED apps must include this file to use this functionality.
 *
 *  @param name					Name of the property.
 *  @param item					Chunk item (must be a linkable object).
 *  @param editorPropertyId		Ignored.
 *	@return						Editor property for the link.
 */
GeneralProperty * UserDataObjectLinkDataType::createEditorProperty(
	const std::string& name, ChunkItem* item, int editorPropertyId )
{
	BW_GUARD;

	EditorChunkItemLinkable* linker = NULL;
	bool alwaysShow = false;
	if ( item->isEditorEntity() )
	{
		EditorChunkEntity* entity =
			static_cast<EditorChunkEntity*>( item );

		// show the gizmo always for the first link property.
		alwaysShow = !entity->firstLinkFound();
		entity->firstLinkFound( true );
		linker = entity->chunkItemLinker();
	}
	else if ( item->isEditorUserDataObject() )
	{
		EditorChunkUserDataObject* udo =
			static_cast<EditorChunkUserDataObject*>( item );

		// show the gizmo always for the first link property.
		alwaysShow = !udo->firstLinkFound();
		udo->firstLinkFound( true );
		linker = udo->chunkItemLinker();
	}
	else
	{
		// Should never get here.
		MF_ASSERT( 0 && "Creating a UserDataObjectLinkDataType from an item that "
			"it's neither a EditorChunkEntity nor a EditorChunkUserDataObject." );
		return NULL;
	}

	return new LinkProperty
		(
			name,
			new UserDataObjectLinkProxy(name, linker),
			NULL,	// use the selection's matrix
			alwaysShow
		);

	// Should never reach here
	MF_ASSERT( 0 && "Creating a UserDataObjectLinkDataType from an item that "
		"it's neither a EditorChunkEntity nor a EditorChunkUserDataObject." );
	return NULL;
}
