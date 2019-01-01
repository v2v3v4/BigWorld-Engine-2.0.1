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
 * An implementation of a packed, binary DataSection.
 */

#ifndef PACKED_SECTION_HPP
#define PACKED_SECTION_HPP

#include "binary_block.hpp"
#include "datasection.hpp"

#include "cstdmf/debug.hpp"

namespace PackedSectionData
{
typedef uint8 VersionType;
typedef int16 NumChildrenType;
typedef int32 DataPosType;
typedef int16 KeyPosType;
}

enum SectionType
{
	DATA_POS_MASK = 0x0fffffff,
	TYPE_MASK     = ~DATA_POS_MASK,

	TYPE_DATA_SECTION	= 0x00000000,
	TYPE_STRING			= 0x10000000,
	TYPE_INT			= 0x20000000,
	TYPE_FLOAT			= 0x30000000, // Used for Vector[234] and Matrix too.
	TYPE_BOOL			= 0x40000000,
	TYPE_BLOB			= 0x50000000,
	TYPE_ENCRYPTED_BLOB	= 0x60000000,
	TYPE_ERROR			= 0x70000000,
};



/**
 *	This class is used to represent a file containing packed, binary sections.
 */
class PackedSectionFile : public SafeReferenceCount
{
public:
	PackedSectionFile( const std::string & name, BinaryPtr pFileData ) :
		storedName_( name ),
		pFileData_( pFileData )
	{
	}

	DataSectionPtr createRoot();

	const char * getIndexedString( PackedSectionData::KeyPosType key ) const
	{
		return stringTable_.getString( key );
	}

	BinaryPtr pFileData() const { return pFileData_; }

private:

	/**
	 *	This class is used to store the string table associated with this file.
	 */
	class StringTable
	{
	public:
		int init( const char * pData, int dataLen );
		const char * getString( PackedSectionData::KeyPosType key ) const;

	private:
		std::vector< const char * > table_;
	};

	std::string storedName_;
	StringTable	stringTable_;
	BinaryPtr pFileData_;
};

typedef SmartPointer< PackedSectionFile > PackedSectionFilePtr;


/**
 *	This class is used to represent a packed, binary section that has children.
 */
class PackedSection : public DataSection
{
public:
	static DataSectionPtr create( const std::string & tag, BinaryPtr pFile );

	// Helper methods for res_packer.
	static DataSectionPtr openDataSection( const std::string & path );
	static bool convert( DataSectionPtr pDS, const std::string & path );
	static bool convert( const std::string & inPath,
			const std::string & outPath,
			std::vector< std::string > * pStripStrings = NULL,
			bool shouldEncrypt = false,
			int stripRecursionLevel = 1 );

	PackedSection( const char * name, const char * pData, int dataLen,
			SectionType type, PackedSectionFilePtr pFile );
	~PackedSection();

	///	@name Section access functions.
	//@{
	virtual int countChildren();
	virtual DataSectionPtr openChild( int index );
	virtual DataSectionPtr newSection( const std::string &tag,
		DataSectionCreator* creator=NULL );
	virtual DataSectionPtr findChild( const std::string &tag,
		DataSectionCreator* creator );
	virtual void delChild(const std::string &tag );
	virtual void delChild(DataSectionPtr pSection);
	virtual void delChildren();
	//@}

	virtual std::string sectionName() const;
	virtual int bytes() const;

	virtual bool isPacked() const	{ return true; }

	virtual bool save( const std::string& filename = "" );

	// creator
	static DataSectionCreator* creator();
protected:
	// Value access overrides from DataSection.
	virtual bool asBool( bool defaultVal );
	virtual int asInt( int defaultVal );
	virtual unsigned int asUInt( unsigned int defaultVal );
	virtual long asLong( long defaultVal );
	virtual int64	asInt64( int64 defaultVal );
	virtual uint64	asUInt64( uint64 defaultVal );
	virtual float asFloat( float defaultVal );
	virtual double asDouble( double defaultVal );
	virtual std::string asString( const std::string &defaultVal, int flags );
	virtual std::wstring asWideString( const std::wstring &defaultVal, int flags );
	virtual Vector2 asVector2( const Vector2 &defaultVal );
	virtual Vector3 asVector3( const Vector3 &defaultVal );
	virtual Vector4 asVector4( const Vector4 &defaultVal );
	virtual Matrix asMatrix34( const Matrix &defaultVal );
	virtual std::string asBlob( const std::string &defaultVal );
	virtual BinaryPtr asBinary();

public:
#pragma pack( push, 1 )
	/**
	 * TODO: to be documented.
	 */
	class ChildRecord
	{
	public:
		typedef PackedSectionData::DataPosType DataPosType;
		typedef PackedSectionData::KeyPosType KeyPosType;

		DataSectionPtr createSection( const PackedSection * pSection ) const;
		bool nameMatches( const PackedSectionFile & parentFile,
				const std::string & tag ) const;

		const char * getName( const PackedSectionFile & parentFile ) const;

		DataPosType startPos() const	{ return dataPos_ & DATA_POS_MASK; }
		DataPosType endPos() const		{ return (this + 1)->startPos(); }

		SectionType type() const
			{ return SectionType((this + 1)->dataPos_ & TYPE_MASK); }

		void setKeyPos( KeyPosType keyPos )	{ keyPos_ = keyPos; }

		void setEndPosAndType( DataPosType dataPos, SectionType type )
		{
			(this + 1)->dataPos_ = dataPos | type;
		}

	private:
		DataPosType dataPos_;
		KeyPosType keyPos_;
	};
#pragma pack( pop )
    friend class ChildRecord;
private:
	const ChildRecord * pRecords() const;
	const char * getDataBlock() const			{ return pOwnData_; }
	const PackedSectionFilePtr pFile() const	{ return pFile_; }

	bool isMatrix() const
	{
		return (ownDataType_ == TYPE_FLOAT) &&
				(ownDataLen_ == sizeof( float ) * 4 * 3);
	}


protected:
	const char *	name_;

	// Own data is the value of this section.
	int				ownDataLen_;
	const char *	pOwnData_;
	SectionType		ownDataType_;

	// Total data is used if this section has children. It represents all of the
	// data.
	int				totalDataLen_;
	const char *	pTotalData_;
	const PackedSectionFilePtr pFile_;
};

#endif // PACKED_SECTION_HPP
