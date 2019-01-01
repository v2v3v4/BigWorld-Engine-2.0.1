/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef __CHUNK_PACKER_HPP__
#define __CHUNK_PACKER_HPP__


#include "base_packer.hpp"
#include "packers.hpp"

#include <string>

/**
 *	This class strips the .chunk files of unwanted data. In the client, it
 *	removes server entities, and in the server it removes client-only entities.
 */
class ChunkPacker : public BasePacker
{
public:
	virtual bool prepare( const std::string & src, const std::string & dst );
	virtual bool print();
	virtual bool pack();

private:
	DECLARE_PACKER()
	std::string src_;
	std::string dst_;
};

#endif // __CHUNK_PACKER_HPP__
