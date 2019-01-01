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
 * binary_block.cpp
 */

#include "pch.hpp"

#include "binary_block.hpp"
#include "cstdmf/debug.hpp"
#include "cstdmf/memory_counter.hpp"
#include "cstdmf/resource_counters.hpp"

#include "zip/zlib.h"

#include <string.h>

DECLARE_DEBUG_COMPONENT2( "ResMgr", 0 )

memoryCounterDefineWithAlias( binaryBlock, Entity, "BinaryBlkSize" );

namespace
{
	const uint32 COMPRESSED_MAGIC1		= 0x7a697000;	// "zip\0"
	const uint32 COMPRESSED_MAGIC2		= 0x42af9021;	// random number
	const uint32 COMPRESSED_HEADER_SZ 	= 3;			// two magic uint32s plus a uint32 for size
}


/*static*/ bool BinaryBlock::s_memoryCritical_ = false;


/**
 *	Constructor.
 *
 *	@param data			Pointer to a block of data
 *	@param len			Size of the data block
 *	@param allocator	A string describing the allocator of this object.
 *	@param pOwner		A smart pointer to the owner of this data. If not NULL, a reference
 *						is kept to the owner, otherwise a copy of the data is taken.
 */
BinaryBlock::BinaryBlock( const void * data, int len, 
						  const char *allocator, BinaryPtr pOwner ) :
	len_( len ),
	pOwner_( pOwner ),
	canZip_( true )
#if ENABLE_RESOURCE_COUNTERS
	, allocator_(allocator)
#endif
{
	memoryCounterAdd( binaryBlock );
	memoryClaim( this );

	// if we're the owner of this block, then make a copy of it
	if (!pOwner)
	{
#if ENABLE_RESOURCE_COUNTERS
		// Track memory usage
		RESOURCE_COUNTER_ADD(ResourceCounters::DescriptionPool(allocator_, (uint)ResourceCounters::SYSTEM),
							 (uint)(len_ + sizeof(*this)))
#endif

		try
		{
			data_ = new char[len];
		}
		catch (...)
		{
			data_ = NULL;
		}

		memoryClaim( data_ );
		//dprintf( "Make BB of size %d at 0x%08X\n", len_, this );

		if(data_ != NULL)
		{
			if (data != NULL)
			{
				memcpy(data_, data, len);
			}
			else
			{
				memset(data_, 0, len);
			}
		}
		else
		{
			s_memoryCritical_ = true;
			WARNING_MSG( "BinaryBlock: Failed to alloc %d bytes\n", len );
			len_ = 0;
			return;
		}
	}
	else
	{
		data_ = const_cast<void*>( data );

#if ENABLE_RESOURCE_COUNTERS
		// Track memory usage
		RESOURCE_COUNTER_ADD(ResourceCounters::DescriptionPool(allocator_, (uint)ResourceCounters::SYSTEM),
							 (uint)sizeof(*this))
#endif
	}
}

/**
 *	Constructor. Initialises binary block from stream content.
 *
 *	@param stream		Stream to get data from.
 *	@param len			Size of the data block
 *	@param allocator	A string describing the allocator of this BinaryBlock.
 */
BinaryBlock::BinaryBlock( std::istream& stream, std::streamsize len, 
						 const char *allocator ) :
	len_( len ),
	pOwner_( NULL ),
	canZip_( true )
#if ENABLE_RESOURCE_COUNTERS
	, allocator_(allocator)
#endif
{
#if ENABLE_RESOURCE_COUNTERS
	// Track memory usage
	RESOURCE_COUNTER_ADD(ResourceCounters::DescriptionPool(allocator_, (uint)ResourceCounters::SYSTEM),
						 (uint)(len_ + sizeof(*this)))
#endif

	memoryCounterAdd( binaryBlock );
	memoryClaim( this );

	try
	{
		data_ = new char[ (size_t)len_ ];
	}
	catch (...)
	{
		data_ = NULL;
	}

	if (data_)
	{
		char * data = (char *) data_;

		std::streampos startPos = stream.tellg();
		std::streampos endPos = stream.seekg( 0, std::ios::end ).tellg();
		std::streamsize numAvail = endPos - startPos;

		stream.seekg(startPos);
		stream.read( data, std::min( numAvail, len_ ) );

		std::streamsize numRead = stream.gcount();

		if( numRead < len_ )
		{
			memset( data + numRead, 0, (size_t)(len_ - numRead) );
		}

		len_ = numRead;
	}
	else
	{
		s_memoryCritical_ = true;
		WARNING_MSG( "BinaryBlock: "
			"Failed to alloc %"PRIzd" bytes (creating from stream)\n", len );
		len_ = 0;
	}
}


/**
 * Destructor
 */
BinaryBlock::~BinaryBlock()
{
	memoryCounterSub( binaryBlock );

	// if we made a copy of this block, then delete it
	if (!pOwner_)
	{
#if ENABLE_RESOURCE_COUNTERS
		// Track memory usage
		if (ResourceCounters::isValid())
		{
			RESOURCE_COUNTER_SUB(ResourceCounters::DescriptionPool(allocator_, (uint)ResourceCounters::SYSTEM),
								 (uint)(len_ + sizeof(*this)))
		}
#endif

		memoryClaim( data_ );
		delete [] (char *)data_;
		//dprintf( "Free BB of size %d at 0x%08X\n", len_, this );
	}
	else
	{
#if ENABLE_RESOURCE_COUNTERS
		// Track memory usage
		if (ResourceCounters::isValid())
		{
			RESOURCE_COUNTER_SUB(ResourceCounters::DescriptionPool(allocator_, (uint)ResourceCounters::SYSTEM),
								 (uint)sizeof(*this))
		}
#endif
	}

	// otherwise simply let our pOwner smartpointer lapse and it'll
	// reduce the reference count on the actual owner (which may delete it)

	memoryClaim( this );
}


/**
 *	Return a compressed version of the block.
 *
 *  @param level The level of compression, 0 to 10, with 0 no compression
 *		(quickest) and 10 full compression (slowest).
 *  @returns The compressed block.
 */
BinaryPtr BinaryBlock::compress(int level /*= DEFAULT_COMPRESSION*/) const
{
	// Sanity check on the compression level:
	if (level < RAW_COMPRESSION ) level = RAW_COMPRESSION ;
	if (level > BEST_COMPRESSION) level = BEST_COMPRESSION;

	BinaryPtr resultBlock;

	// We don't know how well the data compresses.  ZLib can give us an upper
	// bound though.  We compress into a buffer of this size and then copy to
	// a smaller buffer below:
	uLongf compressedSz = compressBound((uint32)len_);
	uint8 * largeBuffer = NULL;
	try
	{
		largeBuffer = new uint8[compressedSz];
	}
	catch (...)
	{
		largeBuffer = NULL;
	}
	if (!largeBuffer)
	{
		s_memoryCritical_ = true;
		WARNING_MSG( "BinaryBlock: Failed to alloc %lu bytes (compress 1)\n", compressedSz );
	}
	else
	{
		int result =
		   compress2
		   (
			   largeBuffer,
			   &compressedSz,
			   (const Byte *)data_,
			   (uint32)len_,
			   level
		   );
		MF_ASSERT(result == Z_OK);
		// compressedSz has been set to the size actually needed.

		// Now copy to a smaller buffer.  The format of this buffer is:
		//	COMPRESSED_MAGIC1		(uint32)
		//	COMPRESSED_MAGIC2		(uint32)
		//	decompressed size		(uint32)
		//	zipped data
		uint32 realBufSz = compressedSz + COMPRESSED_HEADER_SZ*sizeof(uint32);
		uint8 * smallBuffer = NULL;
		try
		{
			smallBuffer = new uint8[realBufSz];
		}
		catch (...)
		{
			smallBuffer = NULL;
		}
		if (!smallBuffer)
		{
			s_memoryCritical_ = true;
			WARNING_MSG( "BinaryBlock: Failed to alloc %u bytes (compress 2)\n", realBufSz );
		}
		else
		{
			uint32 *magicAndSz = (uint32 *)smallBuffer;
			*magicAndSz++ = COMPRESSED_MAGIC1;
			*magicAndSz++ = COMPRESSED_MAGIC2;
			*magicAndSz++ = (uint32)len_;
			uint8 *compData = (uint8 *)magicAndSz;
			::memcpy(compData, largeBuffer, compressedSz);
		#if ENABLE_RESOURCE_COUNTERS
			std::string allocator = allocator_ + "/compressed";
			resultBlock = new BinaryBlock(smallBuffer, realBufSz, allocator.c_str());
		#else
			resultBlock = new BinaryBlock(smallBuffer, realBufSz, "");
		#endif
			delete[] smallBuffer;
		}
		delete[] largeBuffer;
	}
	return resultBlock;
}


/**
 *	Decompress the block.
 *
 *  @returns The uncompressed block.  If the block was not compressed then
 *		NULL is returned.
 */
BinaryPtr BinaryBlock::decompress() const
{
	// Make sure that this was a compressed block:
	if (!isCompressed())
		return NULL;

	BinaryPtr resultBlock;

	uLongf uncompressSz = *((uint32 *)data_ + COMPRESSED_HEADER_SZ - 1);			// size uncompressed
	uint8 * uncompressBuffer = NULL;
	try
	{
		uncompressBuffer = new uint8[uncompressSz];	// buffer for decompression
	}
	catch (...)
	{
		uncompressBuffer = NULL;
	}
	if (!uncompressBuffer)
	{
		s_memoryCritical_ = true;
		WARNING_MSG( "BinaryBlock: Failed to alloc %lu bytes (decompress)\n", uncompressSz );
	}
	else
	{
		uint8 *rawData = (uint8 *)data_ + COMPRESSED_HEADER_SZ*sizeof(uint32);
		uint32 rawDataSz = (uint32)len_ - COMPRESSED_HEADER_SZ*sizeof(uint32);
		int result =
			uncompress
			(
				uncompressBuffer,
				&uncompressSz,
				rawData,
				rawDataSz
			);
		MF_ASSERT(result == Z_OK);
	#if ENABLE_RESOURCE_COUNTERS
		std::string allocator = allocator_;
		if (allocator.substr(allocator.length() - 11) == "/compressed")
			allocator = allocator.substr(0, allocator.length() - 11);
		resultBlock = new BinaryBlock(uncompressBuffer, uncompressSz, allocator.c_str());
	#else
		resultBlock = new BinaryBlock(uncompressBuffer, uncompressSz, "");
	#endif
		delete[] uncompressBuffer;
	}
	return resultBlock;
}


/**
 *	This checks whether the block is likely to be compressed.  We do this by
 *	checking for our zip magic header.  It's possible but unlikely that
 *	a BinaryBlock is not compressed but has the header.
 *
 *  @returns True if the block has the compressed header.
 */
bool BinaryBlock::isCompressed() const
{
	if (len_ <= int( COMPRESSED_HEADER_SZ * sizeof( uint32 ) ))
	{
		return false;
	}
	else
	{
		return
			*((uint32 *)data_) == COMPRESSED_MAGIC1
			&&
			*((uint32 *)data_ + 1) == COMPRESSED_MAGIC2;
	}
}


/**
 *	This method returns whether or not it makes sense to compress this
 *	binary block (for example, if it's compressed already, don't recompress).
 *
 *	@return		True if external compression can help, false otherwise.
 */
bool BinaryBlock::canZip() const
{
	return canZip_ && !isCompressed();
}


/**
 *	This method allows specifying that a binary block should not be
 *	recompressed.  For example, an image stored as a binary block shouldn't
 *	be recompressed externally (it would just make things slower).
 *
 *	@param newVal	True if external compression can help, false otherwise.
 */
void BinaryBlock::canZip( bool newVal )
{
	canZip_ = newVal;
}
