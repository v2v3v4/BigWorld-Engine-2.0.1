/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DATA_DOWNLOAD_HPP
#define DATA_DOWNLOAD_HPP

#include "cstdmf/stdmf.hpp"

#include <list>
#include <set>
#include <string>

class BinaryIStream;
class BinaryOStream;
class DownloadSegment;

/**
 *  A list of DownloadSegments used to collect chunks of data as they are
 *  downloaded from an addProxyData() or addProxyFileData() call.
 */
class DataDownload : public std::list< DownloadSegment* >
{
public:
	DataDownload( uint16 id ) :
		id_( id ),
		pDesc_( NULL ),
		expected_( 0 ),
		hasLast_( false )
	{}

	~DataDownload();

	void insert( DownloadSegment *pSegment, bool isLast );
	bool complete();
	void write( BinaryOStream &os );
	uint16 id() const { return id_; }
	const std::string *pDesc() const { return pDesc_; }
	void setDesc( BinaryIStream &stream );

protected:
	int offset( int seq1, int seq2 );

	// The id of this DataDownload transfer
	uint16 id_;

	// The description associated with this download
	std::string *pDesc_;

	// A set of holes in this record due to out-of-order deliveries
	std::set< int > holes_;

	// The seq we are expecting to receive next
	uint8 expected_;

	// Set to true when the final segment has been added.  Does not necessarily
	// mean this piece of data is complete, as there may be holes.
	bool hasLast_;
};


#endif // DATA_DOWNLOAD_HPP
