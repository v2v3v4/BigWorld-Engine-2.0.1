/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/


#ifndef GUITABS_TAB_HPP
#define GUITABS_TAB_HPP

namespace GUITABS
{


/**
 *  This class encapsulates the functionality of a tab, which includes
 *  managing the layout of the content assigned to it.
 */
class Tab : public ReferenceCount
{
public:
	Tab( CWnd* parentWnd, std::wstring contentID );
	Tab( CWnd* parentWnd, ContentPtr content );
	virtual ~Tab();

	virtual bool load( DataSectionPtr section );
	virtual bool save( DataSectionPtr section );

	virtual std::wstring getDisplayString();
	virtual std::wstring getTabDisplayString();
	virtual HICON getIcon();
	virtual CWnd* getCWnd();
	virtual bool isClonable();
	virtual void getPreferredSize( int& width, int& height );
	
	virtual bool isVisible();
	virtual void setVisible( bool visible );
	virtual void show( bool show );

	virtual ContentPtr getContent();

	virtual void handleRightClick( int x, int y );

protected:
	ContentPtr content_;
	bool isVisible_;

	virtual void construct( CWnd* parentWnd );
};


} // namespace

#endif // GUITABS_TAB_HPP