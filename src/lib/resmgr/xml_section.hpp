/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/*
 * A class that encapsulates the document into data sections. The class allows
 * manipulation and access to elements by hierarchical data sections.
 */

#ifndef XML_SECTION_HPP
#define XML_SECTION_HPP

#include "datasection.hpp"
#include "dataresource.hpp"

#include "math/vector2.hpp"
#include "math/vector3.hpp"

/**
 * We now refer to XMLSection objects using smart pointers.
 */

class XMLSection;
class IFileSystem;
typedef SmartPointer<XMLSection> XMLSectionPtr;

/**
 *	A specialisation of DataSection that deals with XML files.
 *
 *	Each XMLSection is tied to an XMLResource. By this, all instances of
 *	XMLSection are linked that way - change the values of one XML element and it
 *	will affect all the XMLSection instances linked to the XMLResource.
 *
 *	@see DataSection, XMLResource
 */
class XMLSection : public DataSection
{
public:
	///	@name Constructor(s).
	//@{
	///	Constructor for XMLSection.
	XMLSection( const std::string & tag );
	XMLSection( const char * tag, bool cstrToken );
	//@}

	///	@name Destructor.
	//@{
	///	Destructor for XMLSection.
	virtual ~XMLSection();
	//@}

	static XMLSectionPtr createFromFile( const char * filename,
			bool returnEmptySection = true );

	static XMLSectionPtr createFromStream(
		const std::string & tag, std::istream& astream );
	static XMLSectionPtr createFromBinary(
		const std::string & tag, BinaryPtr pBinary );

	virtual void setParent( DataSectionPtr pParent );

	uint32 sizeInBytes() const;

	/// This method makes a string with spaces xml-tag friend
	std::string sanitise( const std::string & val ) const;
	/// This method reverses what a sanitise does
	std::string unsanitise( const std::string & val ) const;
	/// Santise the sectionName.
	bool santiseSectionName();
	/// Is the sectionName valid?
	bool isValidSectionName() const;

	/// This method reads a wide string from our xml-friendly representation
	static std::wstring decodeWideString( const std::string& val );
	/// This method encodes a wide string as our xml-friendly representation
	static std::string encodeWideString( const std::wstring& val );

	// creator
	static DataSectionCreator* creator();
protected:


	/// @name General virtual methods from DataSection
	//@{
	virtual DataSectionPtr findChild( const std::string & tag,
		DataSectionCreator* creator=NULL );
	virtual int countChildren();
	virtual DataSectionPtr openChild( int index );
	virtual DataSectionPtr newSection( const std::string &tag,
		DataSectionCreator* creator=NULL );
	virtual DataSectionPtr insertSection( const std::string &tag, int index /* = -1 */ );
	virtual void delChild( const std::string & tag );
	virtual void delChild( DataSectionPtr pSection );
	virtual void delChildren();
	virtual std::string sectionName() const;
	virtual int bytes() const;
	virtual bool save( const std::string & saveAsFileName = std::string() );
	//@}

	///	@name Virtual methods for reading this DataSection
	//@{
	virtual bool	asBool(	bool defaultVal = false );
	virtual int		asInt( int defaultVal = 0 );
	virtual unsigned int asUInt( unsigned int defaultVal = 0 );
	virtual long	asLong( long defaultVal = 0 );
	virtual int64	asInt64( int64 defaultVal = 0 );
	virtual uint64	asUInt64( uint64 defaultVal = 0 );
	virtual float	asFloat( float  defaultVal = 0.f );
	virtual double	asDouble( double defaultVal = 0.0 );
	virtual std::string asString( const std::string& defaultVal = "",
			int flags = DS_IncludeWhitespace );
	virtual std::wstring asWideString( const std::wstring& defaultVal = L"",
			int flags = DS_IncludeWhitespace );
	virtual Vector2	asVector2( const Vector2& defaultVal = Vector2( 0, 0 ) );
	virtual Vector3	asVector3( const Vector3& defaultVal = Vector3( 0, 0, 0 ) );
	virtual Vector4	asVector4( const Vector4& defaultVal = Vector4( 0, 0, 0, 0 ) );
	virtual Matrix	asMatrix34( const Matrix& defaultVal = Matrix() );
	virtual BinaryPtr	asBinary();
	virtual std::string asBlob( const std::string& defaultVal = "" );
	//@}

	///	@name Methods for setting the value of this DataSection.
	//@{
	virtual bool setBool( bool value );
	virtual bool setInt( int value );
	virtual bool setUInt( unsigned int value );
	virtual bool setLong( long value );
	virtual bool setInt64( int64 value );
	virtual bool setUInt64( uint64 value );
	virtual bool setFloat( float value );
	virtual bool setDouble( double value );
	virtual bool setString( const std::string &value );
	virtual bool setWideString( const std::wstring &value );
	virtual bool setVector2( const Vector2 &value );
	virtual bool setVector3( const Vector3 &value );
	virtual bool setVector4( const Vector4 &value );
	virtual bool setMatrix34( const Matrix &value );
	virtual bool setBlob( const std::string &value );
	//@}


	bool writeToStream( std::ostream & stream, int identation = 0 ) const;

	friend class XMLHandle;


private:
	template <typename T>
	bool checkConversion( T t, const char * str, const char * methodName );

	///	@name Private Helper Methods.
	//@{
	/// This method strips the whitespace from a string.
	char *stripWhitespace( char *inputString );

	/// This method adds a new XMLSection to the current node.
	XMLSection * addChild( const char * tag, int index = -1 );
	//@}

	static bool processBang( class WrapperStream & stream,
			XMLSection * pCurrNode );
	static bool processQuestionMark( class WrapperStream & stream );

	typedef std::vector< XMLSectionPtr > Children;

	const char		* ctag_;
	const char		* cval_;
	std::string		tag_;
	std::string		value_;

	Children		children_;
	DataSectionPtr	parent_;

	BinaryPtr		block_;
};

#endif

/* xml_section.hpp */
