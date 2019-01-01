/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DOWNLOAD_SEGMENT_HPP
#define DOWNLOAD_SEGMENT_HPP

#include <string>

#include "cstdmf/stdmf.hpp"

/**
 *  A single chunk of a DataDownload sent from the server.
 */
class DownloadSegment
{
public:
	DownloadSegment( const char *data, int len, int seq ) :
		seq_( seq ),
		data_( data, len )
	{}

	const char *data() { return data_.c_str(); }
	unsigned int size() { return data_.size(); }

	bool operator< (const DownloadSegment &other)
	{
		return seq_ < other.seq_;
	}

	uint8 seq_;

protected:
	std::string data_;
};


#endif // DOWNLOAD_SEGMENT_HPP
