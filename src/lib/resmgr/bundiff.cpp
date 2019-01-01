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

#include "bundiff.hpp"
#include "cstdmf/debug.hpp"

DECLARE_DEBUG_COMPONENT(0);

// helper function to undiff a file
bool performUndiff( FILE * pDiff, FILE * pSrc, FILE * pDest )
{
	char buffer[0x7fff];
	
	while (true)
	{
		size_t amt;

		unsigned short tag;
		amt = fread( &tag, 2, 1, pDiff );
		if (amt != 1)
			break; // eof

		FILE * pCopyFrom = NULL;
		if (tag&0x8000) // copy-block
		{
			unsigned ofs;
			amt = fread( &ofs, 4, 1, pDiff );
			if (amt != 1)
				return false;
			fseek( pSrc, ofs, SEEK_SET );
			pCopyFrom = pSrc;
		}
		else // new data
		{
			pCopyFrom = pDiff;
		}
		amt = fread( buffer, tag&0x7fff, 1, pCopyFrom );
		if (amt != 1)
			return false;
		amt = fwrite( buffer, tag&0x7fff, 1, pDest );
		if (amt != 1)
			return false;
	}

	return true;
}
