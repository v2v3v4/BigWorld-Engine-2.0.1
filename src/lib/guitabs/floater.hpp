/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef GUITABS_FLOATER_HPP
#define GUITABS_FLOATER_HPP

namespace GUITABS
{


/**
 *  This class implements a floating window, inheriting from CMiniFrameWnd.
 */
class Floater : public CMiniFrameWnd, public ReferenceCount
{
public:
	Floater( CWnd* parentWnd );
	~Floater();

	static void validatePos( int& posX, int& posY, int& width, int& height );
	bool load( DataSectionPtr section );
	bool save( DataSectionPtr section );

	CWnd* getCWnd();
	DockNodePtr getRootNode();
	void setRootNode( DockNodePtr node );
	void updateStyle();

	int getLastRollupSize();
	void setLastRollupSize( int size );

	void adjustSize( bool rollUp = false );

private:
	DockNodePtr dockTreeRoot_;
	int lastRollupSize_;

	void countVisibleNodes( DockNodePtr node, int& count );

	void PostNcDestroy();

	bool onClosePanels( DockNodePtr node );

	afx_msg void OnClose();
	afx_msg void OnSizing( UINT, LPRECT );
	DECLARE_MESSAGE_MAP()
};


} // namespace

#endif // GUITABS_FLOATER_HPP