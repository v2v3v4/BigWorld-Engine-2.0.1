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

#include "data_download.hpp"

#include "download_segment.hpp"


#include "cstdmf/binary_stream.hpp"
#include "cstdmf/debug.hpp"

DataDownload::~DataDownload()
{
	for (iterator it = this->begin(); it != this->end(); ++it)
	{
		delete *it;
	}

	if (pDesc_ != NULL)
	{
		delete pDesc_;
	}
}


/**
 *  Insert the segment into this record in a sorted fashion.
 */
void DataDownload::insert( DownloadSegment *pSegment, bool isLast )
{
	uint8 inseq = pSegment->seq_;

	// Make a note of holes if we're making them
	if (!this->empty() && offset( inseq, this->back()->seq_ ) > 1)
	{
		for (int hole = (this->back()->seq_ + 1) % 0xff;
			 hole != inseq;
			 hole = (hole + 1) % 0xff)
		{
			holes_.insert( hole );
		}
	}

	// An iterator pointing to the newly inserted segment
	iterator insPos;

	// Chuck the new segment at the end if that's obviously the place for it
	if (this->empty() || offset( inseq, this->back()->seq_ ) > 0)
	{
		this->push_back( pSegment );
		insPos = this->end(); --insPos;
	}

	// Otherwise, find an insertion point working backwards from the end
	else
	{
		iterator it = this->end(); --it;
		while (it != this->begin() && offset( inseq, (*it)->seq_ ) > 0)
			--it;

		insPos = std::list< DownloadSegment* >::insert( it, pSegment );
	}

	// Check if we've filled a hole
	if (!holes_.empty())
	{
		std::set< int >::iterator it = holes_.find( inseq );

		if (it != holes_.end())
			holes_.erase( it );
	}

	// If we received the expected packet, update the expected field
	if (inseq == expected_)
	{
		for (++insPos; insPos != this->end(); ++insPos)
		{
			iterator next = insPos; ++next;

			// If the iterator is pointing to the last element in the chain or
			// the next segment does not follow this one, then we are expecting
			// the packet after this one
			if (*insPos == this->back() ||
				offset( (*next)->seq_, (*insPos)->seq_ ) != 1)
			{
				expected_ = ((*insPos)->seq_ + 1) % 0xff;
				break;
			}
		}
	}

	if (isLast)
		hasLast_ = true;
}


/**
 *  Returns true if this piece of DataDownload is complete and ready to be returned
 *  back into onStreamComplete().
 */
bool DataDownload::complete()
{
	return holes_.empty() && hasLast_ && pDesc_ != NULL;
}


/**
 *  Write the contents of this DataDownload into a BinaryOStream.  This DataDownload
 *  must be complete() before calling this.
 */
void DataDownload::write( BinaryOStream &os )
{
	MF_ASSERT_DEV( this->complete() );
	for (iterator it = this->begin(); it != this->end(); ++it)
	{
		DownloadSegment &segment = **it;
		os.addBlob( segment.data(), segment.size() );
	}
}


/**
 *  Set the description for this download from the provided stream.
 */
void DataDownload::setDesc( BinaryIStream &is )
{
	pDesc_ = new std::string();
	is >> *pDesc_;
}


/**
 *  Basically returns seq1 - seq2, adjusted for the ring buffery-ness of the
 *  8-bit sequence numbers used.
 */
int DataDownload::offset( int seq1, int seq2 )
{
	seq1 = (seq1 + 0xff - expected_) % 0xff;
	seq2 = (seq2 + 0xff - expected_) % 0xff;
	return seq1 - seq2;
}


// data_download.cpp
