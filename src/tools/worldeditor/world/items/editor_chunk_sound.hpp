/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef EDITOR_CHUNK_SOUND_HPP
#define EDITOR_CHUNK_SOUND_HPP


#include "worldeditor/config.hpp"
#include "worldeditor/forward.hpp"
#include "worldeditor/world/items/editor_chunk_substance.hpp"
#include "chunk/chunk_sound.hpp"
	

/**
 *	This class is the editor version of a sound item in a chunk.
 */
class EditorChunkSound : public EditorChunkSubstance<ChunkSound>, public Aligned
{
	DECLARE_EDITOR_CHUNK_ITEM( EditorChunkSound )
public:
	EditorChunkSound();
	~EditorChunkSound();

	bool load( DataSectionPtr pSection, Chunk * pChunk );

	virtual bool edSave( DataSectionPtr pSection );

	virtual const Matrix & edTransform()	{ return transform_; }
	virtual bool edTransform( const Matrix & m, bool transient );

	virtual bool edEdit( class GeneralEditor & editor );

	bool reload();


	bool is3dGet() const					{ return is3d_; }
	void is3dSet( const bool & v )			{ is3d_ = v; }

	std::string resGet() const				{ return soundRes_; }
	void resSet( const std::string & v )	{ soundRes_ = v; }

	#define DECLARE_ECS_FLOAT_PROP( NAME )	\
		float NAME##Get() const;			\
		bool NAME##Set( const float & f);	\

	DECLARE_ECS_FLOAT_PROP( innerRadius )
	DECLARE_ECS_FLOAT_PROP( outerRadius )

	DECLARE_ECS_FLOAT_PROP( attenuation )
	DECLARE_ECS_FLOAT_PROP( outsideAtten )
	DECLARE_ECS_FLOAT_PROP( insideAtten )

	DECLARE_ECS_FLOAT_PROP( centreHour )
	DECLARE_ECS_FLOAT_PROP( innerHours )
	DECLARE_ECS_FLOAT_PROP( outerHours )

	bool loopGet() const					{ return loop_; }
	void loopSet( const bool & v )			{ loop_ = v; }

	float loopDelayGet() const				{ return loopDelay_; }
	void loopDelaySet( const float & v )	{ loopDelay_ = v; }

	float loopDVGet() const					{ return loopDelayVariance_; }
	void loopDVSet( const float & v )		{ loopDelayVariance_ = v; }

	static void fini();

private:
	EditorChunkSound( const EditorChunkSound& );
	EditorChunkSound& operator=( const EditorChunkSound& );

	virtual const char * sectName() const	{ return "sound"; }
	virtual const char * drawFlag() const	{ return "render/drawChunkSounds"; }
	virtual ModelPtr reprModel() const;

	void notifySoundMgr();

	bool			is3d_;
	std::string		soundRes_;

	Matrix			transform_;

	float			origAtten_;

	bool			loop_;
	float			loopDelay_;
	float			loopDelayVariance_;
};


#endif // EDITOR_CHUNK_SOUND_HPP
