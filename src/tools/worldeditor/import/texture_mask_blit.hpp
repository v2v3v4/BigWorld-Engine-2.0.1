/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef TEXTURE_MASK_BLIT_HPP
#define TEXTURE_MASK_BLIT_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"


namespace TextureMaskBlit
{
	size_t numChunksAffected( TerrainPaintBrushPtr pBrush );

    bool saveUndoForImport(
        TerrainPaintBrushPtr pBrush,
        const std::string & barrierDesc,
		const std::string & progressDesc,
        bool                showProgress );

	bool import(
		TerrainPaintBrushPtr pBrush,
		const std::string &	texture,
		const Vector4 &		uProj,
		const Vector4 &		vProj,
		bool				showProgress,
		const std::string &	progDesc,
		bool				forceSave );
}


#endif // TEXTURE_MASK_BLIT_HPP
