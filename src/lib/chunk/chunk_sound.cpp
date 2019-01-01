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
#include "chunk_sound.hpp"
#include "chunk.hpp"

//#include "xactsnd/soundmgr.hpp"

#include "cstdmf/debug.hpp"
#include "cstdmf/guard.hpp"


DECLARE_DEBUG_COMPONENT2( "Chunk", 0 )


// -----------------------------------------------------------------------------
// Section: ChunkSound
// -----------------------------------------------------------------------------

int ChunkSound_token;

/**
 *	Constructor.
 */
ChunkSound::ChunkSound() :
	pSound_( NULL )
{
}


/**
 *	Destructor.
 */
ChunkSound::~ChunkSound()
{
	if (pSound_ != NULL)
	{
//		delete pSound_;
	}
}

/**
 *	A helper struct to call CoInitialise for our thread
 */
struct CoInitter
{
	CoInitter()		{ CoInitialize( NULL ); }
	//~CoInitter()	{ CoUninitialize(); }
	// have to find some way to uninit from the same thread
};

/**
 *	The load method.
 */
bool ChunkSound::load( DataSectionPtr pSection, Chunk * pChunk, std::string* errorString )
{
	BW_GUARD;
	static CoInitter	coinitter_;

	if (pSound_ != NULL)
	{
//		delete pSound_;
		pSound_ = NULL;
	}

	if (!pSection->openSection( "3d" ))
	{
		std::string err = "No section specifying ambient or 3d";
		if ( errorString )
		{
			*errorString = "Failed loading sound " + pSection->readString( "resource" ) + " (" + err + ")";
		}
		else
		{
			ERROR_MSG( "ChunkSound::load: %s\n", err.c_str() );
		}
		return false;
	}

	if (pSection->readBool( "3d" ))
	{
/*		pSound_ = soundMgr().loadStatic3D(
			pSection->readString( "resource" ).c_str(),
			pChunk->transform().applyPoint( pSection->readVector3("position") ),
			pSection->readFloat(	"min", 5),
			pSection->readFloat(	"max", 10),
			pSection->readFloat(	"attenuation", 0.0f),
			pSection->readFloat( "outsideAtten", 0.0f),
			pSection->readFloat( "insideAtten", 0.0f),
			pSection->readFloat(	"centreHour", 0.0f),
			pSection->readFloat(	"minHour", 0.0f),
			pSection->readFloat(	"maxHour", 0.0f),
			pSection->readBool( "loop", true),
			pSection->readFloat( "loopDelay", 0),
			pSection->readFloat( "loopDelayVariance", 0) );*/
	}
	else
	{
/*		pSound_ = soundMgr().loadAmbient(
			pSection->readString( "resource" ).c_str(),
			pChunk->transform().applyPoint( pSection->readVector3("position") ),
			pSection->readFloat(	"min", 5),
			pSection->readFloat(	"max", 10),
			pSection->readFloat(	"attenuation", 0.0f),
			pSection->readFloat( "outsideAtten", 0.0f),
			pSection->readFloat( "insideAtten", 0.0f),
			pSection->readFloat(	"centreHour", 0.0f),
			pSection->readFloat(	"minHour", 0.0f),
			pSection->readFloat(	"maxHour", 0.0f),
			pSection->readBool( "loop", true),
			pSection->readFloat( "loopDelay", 0),
			pSection->readFloat( "loopDelayVariance", 0) );*/
	}

	if (pSound_ == NULL)
	{
		std::string err = "SoundManager could not load sound " + pSection->readString( "resource" );
		if ( errorString )
		{
			*errorString = err;
		}
		else
		{
			ERROR_MSG( "ChunkSound::load: %s\n", err.c_str() );
		}
		return false;
	}

	return true;
}


/**
 *	The tick method.
 */
void ChunkSound::tick( float )
{
	//if (pSound_ != NULL) pSound_->tick();
}

/// Static initialiser
#undef IMPLEMENT_CHUNK_ITEM_ARGS
#define IMPLEMENT_CHUNK_ITEM_ARGS (pSection, pChunk, &errorString)
IMPLEMENT_CHUNK_ITEM( ChunkSound, sound, 0 )

// chunk_sound.cpp
