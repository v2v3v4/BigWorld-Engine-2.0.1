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
#include "worldeditor/framework/world_editor_doc.hpp"
#include "worldeditor/framework/world_editor_app.hpp"


namespace
{
	WorldEditorDoc	*s_instance = NULL;
}


IMPLEMENT_DYNCREATE(WorldEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(WorldEditorDoc, CDocument)
END_MESSAGE_MAP()



WorldEditorDoc::WorldEditorDoc()
{
}


/*static*/ WorldEditorDoc& WorldEditorDoc::instance()
{
	return *s_instance;
}


WorldEditorDoc::~WorldEditorDoc()
{
	s_instance = NULL;
}


BOOL WorldEditorDoc::OnNewDocument()
{
	BW_GUARD;

	ASSERT(!s_instance);
	s_instance = this;

	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}


BOOL WorldEditorDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	BW_GUARD;

	ASSERT(!s_instance);
	s_instance = this;
	return TRUE;
}


void WorldEditorDoc::OnCloseDocument()
{
	BW_GUARD;

	CDocument::OnCloseDocument();
}
