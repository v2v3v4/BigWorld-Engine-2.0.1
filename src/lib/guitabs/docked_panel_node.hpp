/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef GUITABS_DOCKED_PANEL_NODE_HPP
#define GUITABS_DOCKED_PANEL_NODE_HPP

namespace GUITABS
{


/**
 *  This class represents a docked panel in the dock tree. It mantains a
 *	pointer to the actual panel in order to be able to return it's CWnd
 *  pointer, which will permit its correct positioning it inside the
 *  appropriate splitter window. This class behaves as a leaf inside the dock
 *  tree.
 */
class DockedPanelNode : public DockNode
{
public:
	DockedPanelNode();
	DockedPanelNode( PanelPtr dockedPanel );

	void init( PanelPtr dockedPanel );

	CWnd* getCWnd();

	bool load( DataSectionPtr section, CWnd* parent, int wndID );
	bool save( DataSectionPtr section );

	void getPreferredSize( int& w, int& h );

	bool isExpanded();

private:
	PanelPtr dockedPanel_;
};


} // namespace

#endif // GUITABS_DOCKED_PANEL_NODE_HPP