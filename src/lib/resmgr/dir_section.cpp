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
 *	@file
 */

#include "pch.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#endif

#include "cstdmf/debug.hpp"
#include "cstdmf/memory_counter.hpp"
#include "access_monitor.hpp"
#include "data_section_cache.hpp"
#include "data_section_census.hpp"

#include "dir_section.hpp"
#include "xml_section.hpp"
#include "bin_section.hpp"
#include "zip_section.hpp"

DECLARE_DEBUG_COMPONENT2( "ResMgr", 0 )

memoryCounterDefine( dirSect, Entity );

void (*g_versionPointPass)( const std::string & path ) = NULL;
THREADLOCAL(bool) g_versionPointSafe( false );

/**
 *	Constructor for DirSection
 *
 *	@param fullPath		Full absolute of the directory.
 *	@param pFileSystem	An IFileSystem to populate the DirSection with.
 *
 *	@return 			None
 */
DirSection::DirSection(const std::string& fullPath, 
						FileSystemPtr pFileSystem) :
	fullPath_(fullPath),
	tag_("root"),
	pFileSystem_(pFileSystem)
{
	if (g_versionPointPass && !g_versionPointSafe)
		(*g_versionPointPass)( fullPath );

	children_ = pFileSystem_->readDirectory(fullPath_);

	memoryCounterAdd( dirSect );
	memoryClaim( this );
	memoryClaim( fullPath_ );
	memoryClaim( tag_ );
	memoryClaim( children_ );
	for (uint i = 0; i < children_.size(); i++)
		memoryClaim( children_[i] );
}

/**
 *	Destructor
 */
DirSection::~DirSection()
{
	memoryCounterSub( dirSect );
	memoryClaim( children_ );
	for (uint i = 0; i < children_.size(); i++)
		memoryClaim( children_[i] );
	memoryClaim( tag_ );
	memoryClaim( fullPath_ );
	memoryClaim( this );
}


/**
 *	This method returns the number of children of this node.
 *
 *	@return 	Number of children.
 */
int DirSection::countChildren()
{
	return children_.size();
}


/**
 *	This method returns the child with the given index
 *
 *	@param index	Index of the child to return.
 *
 *	@return 		Pointer to the specified child.
 */
DataSectionPtr DirSection::openChild( int index )
{
	MF_ASSERT(index < (int)children_.size());
	return this->findChild(children_[index]);
}

/**
 *	This method returns the name of the child with the given index
 */
std::string DirSection::childSectionName( int index )
{
	MF_ASSERT(index < (int)children_.size());
	return children_[index];
}

/**
 *	This method creates a new child. The child will either be
 *	a dirsection, or an xml section, depending on whether the
 *	file has an extension. Creating of binary sections is not
 *	supported.
 *
 *	@param tag		Ignored
 *  @param creator  DataSectionCreator to use when creating the new DataSection.
 *
 *	@return		A DataSectionPtr to the new DataSection.
 */
DataSectionPtr DirSection::newSection( const std::string & tag,
										DataSectionCreator* creator )
{
	int pos;
	DataSectionPtr pSection;
	std::string fullPath;

	if (fullPath_.empty() || fullPath_[ fullPath_.size() - 1 ] == '/')
	{
		fullPath = fullPath_ + tag;
	}
	else
	{
		fullPath = fullPath_ + "/" + tag;
	}

	pos = tag.find_first_of( '.' );

	if (creator)
	{
		pSection = creator->create( this, tag );
		
		pSection->setParent( this );
		DataSectionCache::instance()->add( fullPath, pSection );
		children_.push_back( tag );
		
	}
	else if (pos >= 0 && pos < (int)tag.size())
	{
		pSection = new XMLSection( tag );
		pSection->setParent( this );
		DataSectionCache::instance()->add( fullPath, pSection );
	}
	else if (pFileSystem_->makeDirectory( fullPath ))
	{
		pSection = new DirSection( fullPath, pFileSystem_ );
		DataSectionCache::instance()->add( fullPath, pSection );
		children_.push_back( tag );
	}

	return pSection;
}

//#include "cstdmf/diary.hpp"
//typedef char LongTimeArray[512];
//extern THREADLOCAL(LongTimeArray) g_longTime;

/**
 *	This method locates a child by name. It first searches the cache,
 *	and returns a cached DataSection if there was one. Otherwise,
 *	it will create a new DirSection, XmlSection, or BinSection. It will
 *	return a NULL pointer if the DataSection could not be created.
 *
 *	@param tag		Name of the child to locate
 *  @param creator	The DataSectionCreator to use if a new DataSection
 *                  is required.
 *
 *	@return 		Pointer to DataSection representing the child.
 */
DataSectionPtr DirSection::findChild( const std::string &tag,
										DataSectionCreator* creator )
{
	std::string fullName;
	DataSectionPtr pSection;

	bool hasSlash = (fullPath_.empty() || fullPath_[fullPath_.size() - 1] == '/');
	if (hasSlash)
	{
		fullName = fullPath_ + tag;
	}
	else
	{
		fullName = fullPath_ + "/" + tag;
	}

	const std::string & fsFullName = fullName;

	// If it's cached, return cached copy.
	// How do we handle new files in the filesystem?
	pSection = DataSectionCache::instance()->find(fullName);

	if(pSection)
		return pSection;

	// Also see if it's in the intrusive object list
	pSection = DataSectionCensus::find( fullName );
	if (pSection)
	{
		// add it back into the cache
		DataSectionCache::instance()->add(fullName, pSection);
		return pSection;
	}


	switch(pFileSystem_->getFileTypeEx(fsFullName))
	{
		case IFileSystem::FT_DIRECTORY:
		{
			DirSection* pDirSection = new DirSection(fsFullName,
				pFileSystem_);
			pDirSection->tag_ = tag;

			memoryCounterAdd( dirSect );
			memoryClaim( pDirSection->tag_ );

			pSection = pDirSection;
			break;
		}

		case IFileSystem::FT_ARCHIVE:
		{
			// Disabling this creator because generally we don't want to load a zip as a binary...
			// using the creator would force this at the moment... needs evaluation/fixing

			// ZipSection load via creator is not currently implemented:
			if (creator && creator != ZipSection::creator())
			{
				pSection = creator->load((DataSection*)this, tag, pFileSystem_->readFile(fsFullName));
			}
			else
			{
				// ZipSections avoid loading the entire file.
				ZipSection* pZipSection = new ZipSection(fsFullName, pFileSystem_);
				pZipSection->tag(tag);

				pSection = pZipSection;
				if (pSection)
					pSection->setParent( this );
				break;
			}
		}

		case IFileSystem::FT_FILE:
		{
			BinaryPtr pBinary = pFileSystem_->readFile(fsFullName);

			// Do a dump of the file if it is a file.
			AccessMonitor::instance().record( fullName );

			if (pBinary)
			{
#ifdef EDITOR_ENABLED
				static bool firstTime = true;
				static bool checkRootTag;
				if( firstTime )
				{
					checkRootTag = wcsstr( GetCommandLine(), L"-checkroottag" ) ||
						wcsstr( GetCommandLine(), L"/checkroottag" );
					firstTime = false;
				}
				if( checkRootTag )
				{
					// TODO:UNICODE: Make sure this still works with unicode (utf8) 
					// strings as expected.

					// The following chunk of code is being used to check if the root is conforming to our standard
					bool isXML = pBinary->len() > 0 && *pBinary->cdata() == '<';
					std::string s;
					static const std::string::size_type MAX_TAG_NAME_SIZE = 128;
					int offset = 1;
					while( offset < pBinary->len() && offset < MAX_TAG_NAME_SIZE && pBinary->cdata()[ offset ] != '>' )
					{
						s += pBinary->cdata()[ offset ];
						++offset;
					}
					if( offset < pBinary->len() && pBinary->cdata()[ offset ] == '>' &&
						_stricmp( s.c_str(), "root" ) != 0 )
					{
						ERROR_MSG( "DirSection::findChild: file %s uses an incorrect root tag : <%s>\n",
							fsFullName.c_str(), s.c_str() );
					}
				}
#endif//EDITOR_ENABLED
				pSection =
					DataSection::createAppropriateSection( tag,
						pBinary, true, creator );
				if (pSection)
					pSection->setParent( this );
			}
			break;
		}

		case IFileSystem::FT_NOT_FOUND:
			// pSection is NULL.
			break;
	}

	// If we have a valid section, add it to the cache.

	if(pSection)
	{
		pSection = DataSectionCensus::add( fullName, pSection );
		DataSectionCache::instance()->add(fullName, pSection);
	}

	return pSection;
}


/**
 * 	This method is not implemented for DirSection.
 *
 *	@param tag		Ignored
 *
 *	@return 		NULL
 */
void DirSection::delChild (const std::string & tag )
{
}

/**
 * 	This method is not implemented for DirSection.
 *
 *	@param pSection		Ignored
 *
 *	@return 			NULL
 */
void DirSection::delChild( DataSectionPtr pSection )
{
}

/**
 * 	This method is not implemented for DirSection.
 */
void DirSection::delChildren()
{
//	MF_ASSERT(!"DirSection::delChildren() not implemented");
}


/**
 * 	This method returns the name of the section
 *
 *	@return 		The section name
 */
std::string DirSection::sectionName() const
{
	return tag_;
}


/**
 * 	This method returns the number of bytes used by this section.
 *
 *	@return 		Number of bytes used by this DirSection.
 */
int DirSection::bytes() const
{
	// Close enough for now.
	return 1024;
}


/**
 * 	This method saves the given section to a file.
 */
bool DirSection::save(const std::string& filename)
{
	if(filename != "")
	{
		WARNING_MSG("DirSection: Saving to filename not supported.\n");
		return false;
	}

	for(int i = 0; i < this->countChildren(); i++)
	{
		DataSectionPtr pSection = this->openChild(i);
		if(!pSection->save())
			return false;
	}

	return true;
}

/**
 *	This method saves the given data section which is a child of this section.
 */
bool DirSection::saveChild( DataSectionPtr pChild, bool isBinary )
{
	std::string fullName;

	bool hasSlash = (fullPath_.empty() || fullPath_[fullPath_.size() - 1] == '/');
	if (hasSlash)
	{
		fullName = fullPath_ + pChild->sectionName();
	}
	else
	{
		fullName = fullPath_ + "/" + pChild->sectionName();
	}

	return pFileSystem_->writeFile( fullName, pChild->asBinary(), isBinary );
}

// dir_section.cpp
