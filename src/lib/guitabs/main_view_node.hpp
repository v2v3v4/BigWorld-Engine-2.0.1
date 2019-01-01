/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef GUITABS_MAIN_VIEW_NODE_HPP
#define GUITABS_MAIN_VIEW_NODE_HPP

namespace GUITABS
{


/**
 *  This class represents the main view window in the dock tree. It mantains a
 *	pointer to the actual main window in order to be able to return it when
 *  positioning it inside the appropriate splitter window. This class behaves
 *  as a leaf inside the dock tree.
 */
class MainViewNode : public DockNode
{
public:
	MainViewNode( CWnd* mainView );

	CWnd* getCWnd();

	bool load( DataSectionPtr section, CWnd* parent, int wndID );
	bool save( DataSectionPtr section );

private:
	CWnd* mainView_;
};


} // namespace

#endif // GUITABS_MAIN_VIEW_NODE_HPP