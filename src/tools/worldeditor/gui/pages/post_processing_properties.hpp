/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef POST_PROCESSING_PROPERTIES_HPP
#define POST_PROCESSING_PROPERTIES_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/resource.h"
#include "controls/auto_tooltip.hpp"
#include "common/cdialog_property_table.hpp"
#include <afxwin.h>
#include <afxcmn.h>


// Forward declarations.
class BasePostProcessingNode;
typedef SmartPointer< BasePostProcessingNode > BasePostProcessingNodePtr;


class PostProcPropertyEditor;
typedef SmartPointer< PostProcPropertyEditor > PostProcPropertyEditorPtr;


/**
 *	This class implements the properties subpanel of the post processing panel.
 */
class PostProcessingProperties : public CDialogPropertyTable
{
public:
	enum { IDD = IDD_POST_PROC_PROPERTIES };

	PostProcessingProperties();
	virtual ~PostProcessingProperties();

	void editNode( BasePostProcessingNodePtr node );

protected:
	DECLARE_AUTO_TOOLTIP( PostProcessingProperties, CDialogPropertyTable );
	virtual void DoDataExchange( CDataExchange * pDX );    // DDX/DDV support

	virtual BOOL OnInitDialog();

	void OnOK() {}
	void OnCancel() {}

	afx_msg void OnSize( UINT nType, int cx, int cy );
	afx_msg LRESULT OnSelectPropertyItem( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnChangePropertyItem( WPARAM wParam, LPARAM lParam );
	afx_msg LRESULT OnDblClkPropertyItem( WPARAM wParam, LPARAM lParam );
	DECLARE_MESSAGE_MAP()

private:

	CStatic label_;
	int listRightPadding_;
	int listBottomPadding_;
	PostProcPropertyEditorPtr editor_;
};


#endif // POST_PROCESSING_PROPERTIES_HPP
