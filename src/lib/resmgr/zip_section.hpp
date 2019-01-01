/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _ZIP_SECTION_HPP
#define _ZIP_SECTION_HPP

#include "file_system.hpp"
#include "dir_section.hpp"

class DataSectionCreator;
class ZipFileSystem;
typedef SmartPointer<ZipFileSystem> ZipFileSystemPtr;

/**
 * Data section representing a zip file archive.
 */
class ZipSection : public DataSection
{
public:
	ZipSection( const std::string& tag, FileSystemPtr parentSystem,
		std::vector<DataSectionPtr>* pChildren = NULL);
	ZipSection( ZipFileSystemPtr parent, const std::string& zipPath,
		const std::string& tag );
	~ZipSection();

	///	@name Section access functions.
	//@{
	int countChildren();
	DataSectionPtr openChild( int index );
	std::string childSectionName( int index );
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
					std::vector<DataSectionPtr>* children=NULL);

	virtual bool saveChild( DataSectionPtr pChild, bool isBinary );	

	void tag(const std::string& tag) { tag_ = tag; }
	const std::string& tag() const { return tag_; }

	// creator
	static DataSectionCreator* creator();
private:
	typedef std::vector< DataSectionPtr > Children;

	void introspect();
	BinaryPtr recollect();
	
	void init( const std::string& zipPath,
				const std::string& tag, 
				ZipFileSystemPtr parentZip );

	std::string			tag_;
	std::string			fullPath_;	
	bool				introspected_;
	mutable Children	children_;

	DataSectionPtr		parent_;
	ZipFileSystemPtr	pFileSystem_;
};
typedef SmartPointer<ZipSection> ZipSectionPtr;

#endif //_ZIP_SECTION_HPP
