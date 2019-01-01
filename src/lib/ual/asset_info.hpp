/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	AssetInfo: generic asset info class
 */

#ifndef ASSET_INFO_HPP
#define ASSET_INFO_HPP

#include "cstdmf/smartpointer.hpp"
#include "resmgr/datasection.hpp"

//XmlItem
class AssetInfo : public ReferenceCount
{
public:
	AssetInfo()
	{};
	AssetInfo(
		const std::wstring& type,
		const std::wstring& text,
		const std::wstring& longText,
		const std::wstring& thumbnail = L"",
		const std::wstring& description = L"" ) :
		type_( type ),
		text_( text ),
		longText_( longText ),
		thumbnail_( thumbnail ),
		description_( description )
	{};
	AssetInfo( DataSectionPtr sec )
	{
		if ( sec )
		{
			type_ = sec->readWideString( "type" );
			text_ = sec->asWideString();
			longText_ = sec->readWideString( "longText" );
			thumbnail_ = sec->readWideString( "thumbnail" );
			description_ = sec->readWideString( "description" );
		}
	}

	AssetInfo operator=( const AssetInfo& other )
	{
		type_ = other.type_;
		text_ = other.text_;
		longText_ = other.longText_;
		thumbnail_ = other.thumbnail_;
		description_ = other.description_;
		return *this;
	};

	bool empty() const { return text_.empty(); };
	bool equalTo( const AssetInfo& other ) const
	{
		return
			type_ == other.type_ &&
			text_ == other.text_ &&
			longText_ == other.longText_;
	};
	const std::wstring& type() const { return type_; };
	const std::wstring& text() const { return text_; };
	const std::wstring& longText() const { return longText_; };
	const std::wstring& thumbnail() const { return thumbnail_; };
	const std::wstring& description() const { return description_; };

	void type( const std::wstring& val ) { type_ = val; };
	void text( const std::wstring& val ) { text_ = val; };
	void longText( const std::wstring& val ) { longText_ = val; };
	void thumbnail( const std::wstring& val ) { thumbnail_ = val; };
	void description( const std::wstring& val ) { description_ = val; };
private:
	std::wstring type_;
	std::wstring text_;
	std::wstring longText_;
	std::wstring thumbnail_;
	std::wstring description_;
};
typedef SmartPointer<AssetInfo> AssetInfoPtr;

#endif // ASSET_INFO
