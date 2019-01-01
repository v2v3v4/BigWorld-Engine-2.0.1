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
#include "worldeditor/gui/pages/page_terrain_base.hpp"


DECLARE_DEBUG_COMPONENT2("WorldEditor", 2)


BEGIN_MESSAGE_MAP(PageTerrainBase, CDialog)
END_MESSAGE_MAP()


PageTerrainBase::PageTerrainBase(UINT nIDTemplate):
    CDialog(nIDTemplate)
{
}


BOOL PageTerrainBase::OnInitDialog()
{
	BW_GUARD;

	BOOL ret = CDialog::OnInitDialog();
	INIT_AUTO_TOOLTIP();
	return ret;
}
