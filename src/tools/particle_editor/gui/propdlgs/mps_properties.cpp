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
#include "particle_editor.hpp"
#include "main_frame.hpp"
#include "gui/propdlgs/mps_properties.hpp"
#include "particle/meta_particle_system.hpp"
#include "common/editor_views.hpp"
#include "common/user_messages.hpp"

DECLARE_DEBUG_COMPONENT2( "GUI", 0 )

BEGIN_MESSAGE_MAP(MpsProperties, PropertyTable)
	ON_MESSAGE(WM_CHANGE_PROPERTYITEM, OnChangePropertyItem)
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

IMPLEMENT_DYNCREATE(MpsProperties, PropertyTable)

MpsProperties::MpsProperties(): 
	PropertyTable(MpsProperties::IDD),
	elected_( false )
{
}

MpsProperties::~MpsProperties()
{
	BW_GUARD;

	GeneralEditor::currentEditors( GeneralEditor::Editors() );

	PropTable::table( NULL );
}

void MpsProperties::DoDataExchange(CDataExchange* pDX)
{
    PropertyTable::DoDataExchange(pDX);
}

MetaParticleSystemPtr MpsProperties::metaPS()
{
	BW_GUARD;

    return MainFrame::instance()->GetMetaParticleSystem();
}

void MpsProperties::OnInitialUpdate()
{
	BW_GUARD;

    PropertyTable::OnInitialUpdate();
}

afx_msg LRESULT MpsProperties::OnChangePropertyItem(WPARAM wParam, LPARAM lParam)
{
	BW_GUARD;

	if (lParam)
	{
		BaseView* relevantView = (BaseView*)lParam;
		bool transient = !!wParam;

		relevantView->onChange( transient );
	}

	return 0;
}

afx_msg LRESULT MpsProperties::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	BW_GUARD;

	PropTable::table( this );

	if (metaPS())
	{
		if (!elected_)
		{
			editor_ = new GeneralEditor;
			GeneralEditor::Editors newEds;
			newEds.push_back( editor_ );
			GeneralEditor::currentEditors( newEds );

			metaPS()->metaData().edit( *editor_, L"", false );
			editor_->elect();
			elected_ = true;
		}
	}
	else
	{
		if (elected_)
		{
			editor_ = NULL;
			GeneralEditor::Editors newEds;
			GeneralEditor::currentEditors( newEds );
			elected_ = false;
		}
	}

	if (elected_)
	{
		PropertyTable::update();
	}

	return 0;
}
