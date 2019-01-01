/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "blobbed_sequence_mapping.hpp"

#include "../utils.hpp"

#include "cstdmf/memory_stream.hpp"

BlobbedSequenceMapping::BlobbedSequenceMapping( const Namer & namer,
								const std::string & propName,
								PropertyMappingPtr child,
								int size,
								int dbLen ) :
						StringLikeMapping( namer, propName, false, dbLen )
{
	// Set the default value
	MemoryOStream strm;
	defaultSequenceToStream( strm, size, child );
	int len = strm.remainingLength();
	defaultValue_.assign( (char *) strm.retrieve(len), len );
}

// blobbed_sequence_mapping.cpp
