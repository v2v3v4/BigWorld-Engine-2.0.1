/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "cuetrack.hpp"

#include "expsets.hpp"

#include <stdio.h>

CueTrack CueTrack::_instance;

void CueTrack::clear()
{
	_instance._cues.clear();
}

void CueTrack::addCue( TimeValue time, const char* name )
{
	// Check if the cue is among the frames being exported
	int cueFrame = time / GetTicksPerFrame();
	if (cueFrame < ExportSettings::instance().firstFrame() ||
		cueFrame > ExportSettings::instance().lastFrame())
	{
		return;
	}

	// Adjust the time based on the first frame
	time = time - (ExportSettings::instance().firstFrame() * GetTicksPerFrame());
	
	std::string note( name );
	std::string type, cue;

	int splitPosition = note.find( ':' );

	// string for the note track is expected to be of the form cue:name or sound:name
	if( splitPosition == std::string::npos )
		return;

	type = note.substr( 0, splitPosition );

	if( type == "sound" )
		cue = std::string( "s" ) + note.substr( splitPosition + 1, note.length() - splitPosition - 1 );
	else if( type == "cue" )
		cue = std::string( "c" ) + note.substr( splitPosition + 1, note.length() - splitPosition - 1 );
	else
		return; // only cue: and sound: are supported at the moment

	std::list<std::string>::iterator it = _instance._cues[time].begin();

	for( ; it != _instance._cues[time].end(); ++it )
		if( (*it) == cue )
			return; // already have the cue for that time

	// add the cue
	_instance._cues[time].push_back( cue );
}

void CueTrack::writeFile( BinaryFile& file )
{
	int num = 0; // number of cues

	std::map< TimeValue, std::list<std::string> >::iterator it = _instance._cues.begin();

	for( ; it != _instance._cues.end(); ++it )
	{
		std::list<std::string>::iterator jt = it->second.begin();

		for( ; jt != it->second.end(); ++jt )
			++num;
	}

	// if no cues, don't even bother writing the cue track
	if( num == 0 )
		return;

	// write channel identifier
	file << (int)6;

	// write number of cues
	file << num;

	// write cue data
	it = _instance._cues.begin();

	for( ; it != _instance._cues.end(); ++it )
	{
		std::list<std::string>::iterator jt = it->second.begin();

		for( ; jt != it->second.end(); ++jt )
		{
			file << (float)it->first / (float)GetTicksPerFrame();
			file << *jt;

			// no additional args
			file << (int)0;
		}
	}
}

bool CueTrack::hasCues()
{
	// check for a cue
	std::map< TimeValue, std::list<std::string> >::iterator it = _instance._cues.begin();

	for( ; it != _instance._cues.end(); ++it )
	{
		std::list<std::string>::iterator jt = it->second.begin();

		for( ; jt != it->second.end(); ++jt )
			return true; // found one
	}

	return false;
}
