/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "clean_chunk_list.hpp"
#include "world/editor_chunk.hpp"
#include "common/utilities.hpp"

namespace
{

/**
 *	This function returns the file information of the given file via parameter
 *	pFileInfo. If the given pathname is not a file, it will return false.
 */
bool getFileInfo( const std::string& pathName, IFileSystem::FileInfo* pFileInfo )
{
	BW_GUARD;

	IFileSystem::FileType ft = BWResource::getFileType( pathName, pFileInfo );

	return ft == IFileSystem::FT_FILE ||
		ft == IFileSystem::FT_ARCHIVE;
}

}


/**
 *	Constructor
 */
CleanChunkList::CleanChunkList( GeometryMapping& dirMapping )
	: dirMapping_( dirMapping )
{
	BW_GUARD;

	load();
}


/**
 *	Saves out the clean chunk list
 */
void CleanChunkList::save() const
{
	BW_GUARD;

	DataSectionPtr dsDL = getDataSection();

	dsDL->deleteSections( "chunk" );

	for (std::map<std::string, ChunkInfo>::const_iterator iter = checkedChunks_.begin();
		iter != checkedChunks_.end(); ++iter)
	{
		DataSectionPtr ds = dsDL->newSection( "chunk" );

		ds->writeUInt64( "cdataCreationTime", iter->second.cdataCreationTime_ );
		ds->writeUInt64( "cdataModificationTime", iter->second.cdataModificationTime_ );
		ds->writeUInt64( "chunkCreationTime", iter->second.chunkCreationTime_ );
		ds->writeUInt64( "chunkModificationTime", iter->second.chunkModificationTime_ );
		ds->writeString( "name", iter->first );
	}

	for (std::map<std::string, ChunkInfo>::const_iterator iter = uncheckedChunks_.begin();
		iter != uncheckedChunks_.end(); ++iter)
	{
		DataSectionPtr ds = dsDL->newSection( "chunk" );

		ds->writeUInt64( "cdataCreationTime", iter->second.cdataCreationTime_ );
		ds->writeUInt64( "cdataModificationTime", iter->second.cdataModificationTime_ );
		ds->writeUInt64( "chunkCreationTime", iter->second.chunkCreationTime_ );
		ds->writeUInt64( "chunkModificationTime", iter->second.chunkModificationTime_ );
		ds->writeString( "name", iter->first );
	}

	dsDL->save();
}


/**
 *	Check every unchecked time stamp against the time stamp of chunk and cdata
 *	files to see if it has been modified outside WE
 */
void CleanChunkList::sync( ProgressTask* task )
{
	BW_GUARD;

	std::set<std::string> chunks = Utilities::gatherChunks( &dirMapping_ );
	int chunkProcessed = 0;

	task->length( (float)chunks.size() );

	for (std::set<std::string>::iterator iter = chunks.begin();
		iter != chunks.end(); ++iter)
	{
		if (uncheckedChunks_.find( *iter ) != uncheckedChunks_.end())
		{
			std::string chunkName = dirMapping_.path() + *iter + ".chunk";
			std::string cdataName = dirMapping_.path() + *iter + ".cdata";
			IFileSystem::FileInfo fiChunk;
			IFileSystem::FileInfo fiCdata;

			if (getFileInfo( chunkName, &fiChunk ) &&
				getFileInfo( cdataName, &fiCdata ))
			{
				if (uncheckedChunks_[ *iter ].cdataCreationTime_ == fiCdata.created &&
					uncheckedChunks_[ *iter ].cdataModificationTime_ == fiCdata.modified &&
					uncheckedChunks_[ *iter ].chunkCreationTime_ == fiChunk.created &&
					uncheckedChunks_[ *iter ].chunkModificationTime_ == fiChunk.modified)
				{
					checkedChunks_[ *iter ] = uncheckedChunks_[ *iter ];
				}
			}

			uncheckedChunks_.erase( *iter );
		}

		++chunkProcessed;
		if (chunkProcessed % 100 == 0)
		{
			task->step( 100.f );
		}
	}
}


/**
 *	This function is called when the files of a chunk has been updated
 *	inside WE so its time stamp and clean status gets updated
 */
void CleanChunkList::update( Chunk* pChunk )
{
	MF_ASSERT( pChunk );

	ChunkInfo ci;
	EditorChunkCache& ecc = EditorChunkCache::instance( *pChunk );
	IFileSystem::FileInfo fiChunk;
	IFileSystem::FileInfo fiCdata;

	if (uncheckedChunks_.find( pChunk->identifier() ) !=
		uncheckedChunks_.end())
	{
		uncheckedChunks_.erase( pChunk->identifier() );
	}

	if (ecc.shadowUpdated() && ecc.lightingUpdated()
		&& ecc.terrainLODUpdated() && ecc.thumbnailUpdated())
	{
		if (getFileInfo( pChunk->resourceID(), &fiChunk ) &&
			getFileInfo( pChunk->binFileName(), &fiCdata ))
		{
			ci.cdataCreationTime_ = fiCdata.created;
			ci.cdataModificationTime_ = fiCdata.modified;
			ci.chunkCreationTime_ = fiChunk.created;
			ci.chunkModificationTime_ = fiChunk.modified;

			checkedChunks_[ pChunk->identifier() ] = ci;

			return;
		}
	}

	if (checkedChunks_.find( pChunk->identifier() ) !=
		checkedChunks_.end())
	{
		checkedChunks_.erase( pChunk->identifier() );
	}
}


/**
 *	This function marks the given chunk as clean. It ignores
 *	the chunk dirty status.
 */
void CleanChunkList::markAsClean( const std::string& chunk )
{
	BW_GUARD;

	ChunkInfo ci;
	IFileSystem::FileInfo fiChunk;
	IFileSystem::FileInfo fiCdata;

	if (uncheckedChunks_.find( chunk ) != uncheckedChunks_.end())
	{
		uncheckedChunks_.erase( chunk );
	}

	std::string chunkName = dirMapping_.path() + chunk + ".chunk";
	std::string cdataName = dirMapping_.path() + chunk + ".cdata";

	if (getFileInfo( chunkName, &fiChunk ) &&
		getFileInfo( cdataName, &fiCdata ))
	{
		ci.cdataCreationTime_ = fiCdata.created;
		ci.cdataModificationTime_ = fiCdata.modified;
		ci.chunkCreationTime_ = fiChunk.created;
		ci.chunkModificationTime_ = fiChunk.modified;

		checkedChunks_[ chunk ] = ci;

		return;
	}

	if (checkedChunks_.find( chunk ) != checkedChunks_.end())
	{
		checkedChunks_.erase( chunk );
	}
}


/**
 *	This function returns whether the chunk is clean
 */
bool CleanChunkList::isClean( const std::string& chunk ) const
{
	BW_GUARD;

	return checkedChunks_.find( chunk ) != checkedChunks_.end();
}


/**
 *	Load the clean list
 */
void CleanChunkList::load()
{
	BW_GUARD;

	DataSectionPtr dsDL = getDataSection();

	for (int i = 0; i < dsDL->countChildren(); ++i)
	{
		DataSectionPtr ds = dsDL->openChild( i );

		if (ds->sectionName() == "chunk")
		{
			ChunkInfo ci;

			ci.cdataCreationTime_ = ds->readUInt64( "cdataCreationTime" );
			ci.cdataModificationTime_ = ds->readUInt64( "cdataModificationTime" );
			ci.chunkCreationTime_ = ds->readUInt64( "chunkCreationTime" );
			ci.chunkModificationTime_ = ds->readUInt64( "chunkModificationTime" );

			uncheckedChunks_[ ds->readString( "name" ) ] = ci;
		}
	}
}


/**
 *	Return the clean list data section
 */
DataSectionPtr CleanChunkList::getDataSection() const
{
	BW_GUARD;

	return BWResource::openSection( dirMapping_.path() + "space.cleanlist", true );
}
