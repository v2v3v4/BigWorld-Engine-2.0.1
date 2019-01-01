/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// ModelEditorDoc.cpp : implementation of the CModelEditorDoc class
//

#include "pch.hpp"
#include "model_editor.h"

#include "model_editor_doc.h"


CModelEditorDoc * CModelEditorDoc::s_instance_ = NULL;

DECLARE_DEBUG_COMPONENT2( "ModelEditor", 0 )


// CModelEditorDoc

IMPLEMENT_DYNCREATE(CModelEditorDoc, CDocument)

BEGIN_MESSAGE_MAP(CModelEditorDoc, CDocument)
END_MESSAGE_MAP()


// CModelEditorDoc construction/destruction

CModelEditorDoc::CModelEditorDoc()
{
	// TODO: add one-time construction code here

}

CModelEditorDoc::~CModelEditorDoc()
{
	s_instance_ = NULL;
}

BOOL CModelEditorDoc::OnNewDocument()
{
	BW_GUARD;

	ASSERT(!s_instance_);
	s_instance_ = this;

	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialisation code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CModelEditorDoc serialization

void CModelEditorDoc::Serialize(CArchive& ar)
{
	BW_GUARD;

	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CModelEditorDoc diagnostics

#ifdef _DEBUG
void CModelEditorDoc::AssertValid() const
{
	BW_GUARD;

	CDocument::AssertValid();
}

void CModelEditorDoc::Dump(CDumpContext& dc) const
{
	BW_GUARD;

	CDocument::Dump(dc);
}
#endif //_DEBUG


// CModelEditorDoc commands
