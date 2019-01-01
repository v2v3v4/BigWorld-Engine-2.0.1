/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_USER_DATA_OBJECT_LINK
#define EDITOR_CHUNK_USER_DATA_OBJECT_LINK


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_link.hpp"


class EditorChunkUserDataObjectLink : public EditorChunkLink
{
	DECLARE_EDITOR_CHUNK_ITEM(EditorChunkUserDataObjectLink)

public:
	virtual std::vector<std::string> edCommand(
		std::string const &path ) const;

    virtual bool edExecuteCommand(
		std::string const &path, std::vector<std::string>::size_type index );

	virtual float collide(
		const Vector3& source, const Vector3& dir, WorldTriangle& wt ) const;

private:
	void deleteCommand();
};


typedef SmartPointer<EditorChunkUserDataObjectLink>   EditorChunkUserDataObjectLinkPtr;


#endif // EDITOR_CHUNK_USER_DATA_OBJECT_LINK
