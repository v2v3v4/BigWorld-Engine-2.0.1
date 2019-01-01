/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _DIR_SECTION_HPP
#define _DIR_SECTION_HPP

#include "file_system.hpp"
class DataSectionCache;

#include <map>

/**
 * An implementation of DataSection that provides a view onto a directory tree node.
 */
class DirSection : public DataSection
{
public:
	DirSection(const std::string& path, FileSystemPtr pFileSystem);
	~DirSection();

	///	@name virtual section access functions
	//@{
	/// This methods returns the number of sections under this one.
	int countChildren();

	/// This method opens a section with the given index.
	DataSectionPtr openChild( int index );

	/// This method returns the name of the child with the given index.
	std::string childSectionName( int index );

	///	This method does nothing for DirSection
	DataSectionPtr newSection( const std::string &tag,
		DataSectionCreator* creator=NULL );

	/// This method searches for a new section directly under the current section.
	DataSectionPtr findChild( const std::string &tag,
		DataSectionCreator* creator=NULL  );

	/// These methods currently do nothing.
	void delChild (const std::string &tag );
	void delChild ( DataSectionPtr pSection );

	void delChildren();

	///	This method returns the section's name.
	virtual std::string sectionName() const;

	/// This method returns the number of bytes used by this section.
	virtual int bytes() const;

	/// This method saves the section to the given filename
	virtual bool save(const std::string& filename = "");

	/// This method saves the given section (as if it were) our child
	virtual bool saveChild( DataSectionPtr pChild, bool isBinary );


	virtual DataSectionPtr convertToZip(const std::string& saveAs="",
		DataSectionPtr pRoot=NULL, std::vector<DataSectionPtr>* children=NULL );
	//@}

	void tag(const std::string& tag) { tag_ = tag; }
	const std::string& tag() const { return tag_; }
private:	

	std::string					fullPath_;
	std::string					tag_;
	std::vector<std::string>	children_;
	FileSystemPtr				pFileSystem_;

	void						addChildren();
};

#endif
