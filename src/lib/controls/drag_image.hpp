/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DRAG_IMAGE_HPP
#define DRAG_IMAGE_HPP


namespace controls
{

class DragImage
{
public:
	DragImage( CImageList & imgList, const CPoint & pos, const CPoint & offset );
	~DragImage();

	void update( const CPoint & pos, BYTE alpha = 160 );

private:
	CImageList & imgList_;
	HWND hwnd_;
	CRect rect_;
	CPoint offset_;
};

} // namespace controls


#endif // DRAG_IMAGE_HPP