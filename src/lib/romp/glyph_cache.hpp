/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef GLYPH_CACHE_HPP
#define GLYPH_CACHE_HPP

#include "moo/render_target.hpp"
#include "moo/managed_texture.hpp"
#include "resmgr/datasection.hpp"
#include <gdiplus.h>
class GlyphReferenceHolder;

/**
 *	This class manages a cache of glyphs in a render target texture.
 *	
 */
class GlyphCache
{
public:
	GlyphCache();
	~GlyphCache();
	bool load( const std::string& fontFileName, DataSectionPtr pSection );
	bool save( DataSectionPtr pSection, const std::string& ddsName, const std::string& gridName );
	void refillSurface();

	bool dynamicCache() const;

	// Accessors
	Moo::BaseTexturePtr pTexture()			{ return pTexture_; }
	Vector2 mapSize() const					{ return Vector2((float)pTexture_->width(),(float)pTexture_->height()); }
	float mapWidth() const					{ return (float)pTexture_->width(); }
	float mapHeight() const					{ return (float)pTexture_->height(); }
	float heightPixels() const				{ return height_; }
	float maxWidthPixels() const			{ return maxWidth_; }
	float effectsMargin() const				{ return effectsMargin_; }
	float textureMargin() const				{ return textureMargin_; }
	float uvDivisor() const					{ return uvDivisor_; }
	
	const std::string& fontName() const;
	const std::string& fontFileName() const	{ return fontFileName_; }
	

	// Cache slot support
	struct Slot
	{
		Slot() : x_(0), y_(0) {}
		Slot( uint32 x, uint32 y ):
			x_(x),
			y_(y)
		{};

		uint32	x_;
		uint32	y_;
	};

	void	slotToUV( const Slot& slot, Vector2& ret ) const;
	void	uvToSlot( const Vector2& uv, Slot& ret ) const;
	bool	grabSlot( Slot& slot );
	void	occupied( const Slot& slot, bool state );
	bool	occupied( const Slot& slot ) const;

	// Glyph definition
	class Glyph
	{
	public:
		Vector2	uv_;
		float	uvWidth_;
		uint32	lastUsed_;
	};

	typedef std::map<wchar_t, Glyph>	Glyphs;
	const Glyphs& glyphs() const			{ return glyphs_; }
	const Glyph& glyph( wchar_t );
	GlyphReferenceHolder& refCounts()		{ return *refCounts_; }

private:
	const Glyph& addGlyphInternal( wchar_t c, const Slot& glyphRegion );
	bool calcGlyphRect( wchar_t c, Vector2& dimensions );
	bool loadLegacyGeneratedSection( DataSectionPtr pSection, const std::wstring& characterSet );
	bool loadPreGeneratedData( DataSectionPtr pSection );
	void createGridMap( Moo::RenderTarget* pDestRT );
	void preloadGlyphs( const std::wstring& characterSet );
	void loadCharacterSet( DataSectionPtr pSection, std::wstring& characterSet );
	void glyphToScreenRect( const Glyph& g, Vector2& tl, Vector2& br );
	bool createFont();
	void clearOccupancy();
	bool expand();
	Glyphs	glyphs_;
	GlyphReferenceHolder* refCounts_;
	bool* occupied_;
	Slot curr_;
	Slot slotBounds_;
	Vector2 slotSize_;

	wchar_t spaceProxyChar_;
	wchar_t widestChar_;
	float effectsMargin_;		//in pixels
	float textureMargin_;		//in pixels
	float maxWidth_;			//in pixels
	float height_;				//in pixels
	std::string fontFileName_;
	bool proportional_;
	int pointSize_;
	float uvDivisor_;
	std::wstring defaultCharacterSet_;
	int shadowAlpha_;
	
	Moo::ManagedTexturePtr	pTexture_;
	bool lastExpandFailed_;

	//GDI+ font rendering
	Gdiplus::Graphics*		graphics_;
	Gdiplus::Brush*			brush_;
	Gdiplus::Brush*			shadowBrush_;
	Gdiplus::Bitmap*		bitmap_;
	uint32					pixelFormat_;
	uint32					gdiSurfacePitch_;

private:
	class SourceFont : public ReferenceCount
	{
	public:
		SourceFont( DataSectionPtr ds, bool bold=false, bool antialias=true, bool dropShadow=false );
		~SourceFont();
		
		bool createFont( int pointSize );

		bool matchChar( wchar_t c ) const { return c >= rangeMin_ && c <= rangeMax_; }

		const std::string& fontName() const { return fontName_; }
		uint32 rangeMin() const { return rangeMin_; }
		uint32 rangeMax() const { return rangeMax_; }
		bool antialias() const { return antialias_; }
		bool bold() const { return bold_; }
		bool dropShadow() const { return dropShadow_; }

		Gdiplus::Font* font() const { return font_; }

	private:
		SourceFont( const SourceFont& o );

		std::string fontName_;
		uint32 rangeMin_;
		uint32 rangeMax_;
		bool antialias_;
		bool bold_;
		bool dropShadow_;

		Gdiplus::FontFamily* fontFamily_;
		Gdiplus::Font* font_;
	};

	typedef SmartPointer< SourceFont > SourceFontPtr;

	std::vector<SourceFontPtr>	sourceFonts_;
	const SourceFontPtr& charToFont( wchar_t c );

	class SortSourceFont
	{
	public:
		bool operator () ( const SourceFontPtr& p1, const SourceFontPtr& p2 )
		{ return p1->rangeMin() < p2->rangeMin(); }
	};
};

#endif //GLYPH_CACHE_HPP