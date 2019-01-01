/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef WORLD_EDITOR_DOC_HPP
#define WORLD_EDITOR_DOC_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"


class WorldEditorDoc : public CDocument
{
protected: 
	DECLARE_DYNCREATE(WorldEditorDoc)

	WorldEditorDoc();
	
public:
	static WorldEditorDoc& instance();

	virtual ~WorldEditorDoc();

	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();

protected:
	DECLARE_MESSAGE_MAP()

private:
	static WorldEditorDoc	*s_instance_;
};


#endif // WORLD_EDITOR_DOC_HPP
