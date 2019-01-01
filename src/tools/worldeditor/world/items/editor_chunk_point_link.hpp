/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_POINT_LINK
#define EDITOR_CHUNK_POINT_LINK


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_link.hpp"


/**
 *	This class implements a link from an item to an arbitrary point.
 */
class EditorChunkPointLink : public EditorChunkLink
{
	DECLARE_EDITOR_CHUNK_ITEM(EditorChunkPointLink)

public:
	EditorChunkPointLink();

	virtual void endPoint( const Vector3& endPoint, const std::string& chunkId );

	virtual bool getEndPoints(
		Vector3 &s, Vector3 &e, bool absoluteCoords ) const;

	virtual void draw();

	virtual void drawInternal();

	virtual float collide(
		const Vector3& source, const Vector3& dir, WorldTriangle& wt ) const;

private:
	Vector3 endPoint_;
	std::string chunkId_;
    ComObjectWrap<DX::BaseTexture> texture_;
};

typedef SmartPointer<EditorChunkPointLink>   EditorChunkPointLinkPtr;

#endif // _EDITOR_CHUNK_POINT_LINK_