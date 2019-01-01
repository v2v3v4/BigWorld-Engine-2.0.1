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
 * An implementation of DataSection that provides a view onto a binary file.
 */

#ifndef _BIN_SECTION_HPP
#define _BIN_SECTION_HPP

#include <map>

#include "binary_block.hpp"
#include "datasection.hpp"

#include "cstdmf/debug.hpp"

class IFileSystem;

/**
 * TODO: to be documented.
 */
class BinSection : public DataSection
{
public:
	BinSection( const std::string& tag, BinaryPtr pBinary );
	~BinSection();

	///	@name Section access functions.
	//@{
	int countChildren();
	DataSectionPtr openChild( int index );
	DataSectionPtr newSection( const std::string &tag,
		DataSectionCreator* creator=NULL );
	DataSectionPtr findChild( const std::string &tag,
		DataSectionCreator* creator=NULL );
	void delChild(const std::string &tag );
	void delChild(DataSectionPtr pSection);
	void delChildren();
	//@}

	/// This method returns the binary data owned by this section.
	BinaryPtr asBinary();

	/// This method sets the binary data of this section
	bool setBinary(BinaryPtr pBinary);

	///	This method returns the section's name.
	virtual std::string sectionName() const;

	/// This method returns the number of bytes used by this section.
	virtual int bytes() const;

	/// This method saves the section
	virtual bool save( const std::string& filename = "" );

	/// This method sets the parent section associated with this section
	void setParent( DataSectionPtr pParent );

	virtual bool canPack() const	{ return false; }

	virtual DataSectionPtr convertToZip( const std::string& saveAs="", 
								DataSectionPtr pRoot=NULL,
								std::vector<DataSectionPtr>* children=NULL );

	// creator
	static DataSectionCreator* creator();
private:
	void introspect();
	BinaryPtr recollect();

	std::string		tag_;
	BinaryPtr		binaryData_;

	typedef std::vector< DataSectionPtr > Children;

	bool			introspected_;
	Children		children_;

	DataSectionPtr	parent_;
};

typedef SmartPointer<BinSection> BinSectionPtr;

#endif
