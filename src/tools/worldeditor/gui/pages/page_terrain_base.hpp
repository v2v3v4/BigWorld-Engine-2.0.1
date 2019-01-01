/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PAGE_TERRAIN_BASE_HPP
#define PAGE_TERRAIN_BASE_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "controls/auto_tooltip.hpp"


class PageTerrainBase : public CDialog
{
public:
	PageTerrainBase(UINT nIDTemplate);

protected:
	BOOL OnInitDialog();

	DECLARE_AUTO_TOOLTIP(PageTerrainBase, CDialog);
	DECLARE_MESSAGE_MAP()
};


#endif // PAGE_TERRAIN_BASE_HPP
