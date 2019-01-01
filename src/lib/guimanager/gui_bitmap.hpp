/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GUI_BITMAP_HPP__
#define GUI_BITMAP_HPP__

#include "gui_forward.hpp"
#include "cstdmf/smartpointer.hpp"
#include <map>
#include <vector>
#include <vector>

/**************************************************
Some words about bitmap type
As we know, any status bitmap could be normal/disabled/enabled/hover/pressed/.......
It is a bit hard to give a full enumeration of is, so we use a string as BITMAP type

Currently we are using the following types:

NORMAL		normal, and pressed in toolbar
HOVER		when mouse put on
DISABLED	a grayed one

You may define your own freely since their are no real panelty except using a bit more resources.

TODO: for now, we doesn't support any register + post processing, will be finished when
	everything else is ok
**************************************************/

BEGIN_GUI_NAMESPACE

static const unsigned int SIZE_DEFAULT = 0xffffffff;

class Bitmap : public SafeReferenceCount
{
protected:
	std::string type_;
	HBITMAP bitmap_;
	bool defaultSize_;
public:
	Bitmap( const std::wstring& name, COLORREF transparent, const std::string& type = "NORMAL", unsigned int width = SIZE_DEFAULT, unsigned int height = SIZE_DEFAULT );
	~Bitmap();
	const std::string& type() const;
	unsigned int width() const;
	unsigned int height() const;
	bool defaultSize() const;
	operator HBITMAP();
};

typedef SmartPointer<Bitmap> BitmapPtr;

class BitmapManager
{
	std::map<std::string, std::vector<BitmapPtr> > bitmaps_;
public:
	BitmapPtr find( const std::string& name, const std::string& type = "NORMAL",
		unsigned int width = SIZE_DEFAULT, unsigned int height = SIZE_DEFAULT );
	BitmapPtr get( const std::string& name, COLORREF transparent,
		const std::string& type = "NORMAL", unsigned int width = SIZE_DEFAULT, unsigned int height = SIZE_DEFAULT );
	void free( BitmapPtr& bitmap );
	void clear();
};

END_GUI_NAMESPACE

#endif//GUI_BITMAP_HPP__
