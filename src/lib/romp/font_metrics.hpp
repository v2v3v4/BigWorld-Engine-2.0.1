/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef FONT_METRICS_HPP
#define FONT_METRICS_HPP

#include "cstdmf/stdmf.hpp"
#include "resmgr/datasection.hpp"

class GlyphReferenceHolder;
class GlyphCache;


/**
 *	This class can be queried for font based information, including
 *	character width, string width and height.
 *
 *	All values are returned in texels, unless otherwise specified.
 */
class FontMetrics : public ReferenceCount
{
public:
	FontMetrics();
	~FontMetrics();

	std::vector<std::wstring> breakString( std::wstring wstr, /* IN OUT */int& w, int& h,
		int minHyphenWidth = 0, const std::wstring& wordBreak = L" ",
		const std::wstring& punctuation = L",<.>/?;:'\"[{]}\\|`~!@#$%^&*()-_=+" );

	void stringDimensions( const std::string& str, int& w, int& h, bool multiline=false, bool colourFormatting=false );
	uint stringWidth( const std::string& str, bool multiline=false, bool colourFormatting=false );
	void stringDimensions( const std::wstring& str, int& w, int& h, bool multiline=false, bool colourFormatting=false );
	uint stringWidth( const std::wstring& str, bool multiline=false, bool colourFormatting=false );
	
	float maxWidth() const;
	float clipWidth( wchar_t c );
	float clipHeight() const;
	float heightPixels() const;
//private:

	GlyphCache& cache()			{ return *cache_; }
	const GlyphCache& cache() const	{ return *cache_; }
	GlyphCache* cache_;

private:
	void stringDimensionsInternal( const std::wstring& s, bool multline, bool colourFormatting, int& w, int& h );
	void stringDimensionsInternal( const std::string& s, bool multline, bool colourFormatting, int& w, int& h );	
};

typedef SmartPointer<FontMetrics>	FontMetricsPtr;


#endif	//FONT_METRICS_HPP