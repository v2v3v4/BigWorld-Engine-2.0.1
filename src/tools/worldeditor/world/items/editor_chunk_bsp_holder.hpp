/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_BSP_HOLDER__
#define EDITOR_CHUNK_BSP_HOLDER__

#include "moo/vertex_formats.hpp"
#include <map>
#include <string>
#include <vector>

class EditorChunkBspHolder
{
	struct Info
	{
		UINT primitiveCount_;
		Moo::VertexBuffer vb_;
	};

	typedef std::map<std::string, EditorChunkBspHolder::Info> Infos;
	static Infos infos_;
	static volatile LONG count_;

	Moo::Colour colour_;
public:
	EditorChunkBspHolder();
	~EditorChunkBspHolder();

	bool bspCreated( const std::string& name ) const;
	void drawBsp( const std::string& name ) const;
	void addBsp( const std::vector<Moo::VertexXYZL>& verts, const std::string& name );
	void postClone();
};

#endif//EDITOR_CHUNK_BSP_HOLDER__
