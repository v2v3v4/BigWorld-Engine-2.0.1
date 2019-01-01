/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef CUETRACK_HPP
#define CUETRACK_HPP

#include <Max.h>
#include <list>
#include <map>
#include <string>

#include "cstdmf/binaryfile.hpp"

class CueTrack
{
public:
	static void clear();
	static void addCue( TimeValue time, const char* name );
	static void writeFile( BinaryFile& file );

	static bool hasCues();

protected:
	static CueTrack _instance;

	// maps times the cues occur at to their identifiers
	std::map< TimeValue, std::list<std::string> > _cues;
};

#endif // CUETRACK_HPP
