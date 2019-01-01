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
#include "worldeditor/world/items/editor_chunk_sound.hpp"
#include "worldeditor/world/items/editor_chunk_substance.ipp"
#include "worldeditor/world/world_manager.hpp"
#include "worldeditor/world/editor_chunk.hpp"
#include "worldeditor/editor/item_editor.hpp"
#include "worldeditor/editor/item_properties.hpp"
#include "xactsnd/soundmgr.hpp"
#include "appmgr/options.hpp"
#include "romp/super_model.hpp"
#include "chunk/chunk_model.hpp"
#include "chunk/chunk_manager.hpp"
#include "resmgr/string_provider.hpp"


namespace 
{
	ModelPtr	s_model;
	bool		s_triedLoadOnce = true;
}


// -----------------------------------------------------------------------------
// Section: EditorChunkSound
// -----------------------------------------------------------------------------

int EditorChunkSound_token = 0;

/**
 *	Constructor.
 */
EditorChunkSound::EditorChunkSound() :
	is3d_( false )
{
	transform_.setScale( 5.f, 5.f, 5.f );
}


/**
 *	Destructor.
 */
EditorChunkSound::~EditorChunkSound()
{
}



/**
 *	This method saves the data section pointer before calling its
 *	base class's load method
 */
bool EditorChunkSound::load( DataSectionPtr pSection, Chunk * pChunk )
{
	BW_GUARD;

	bool ok = this->EditorChunkSubstance<ChunkSound>::load( pSection, pChunk );
	if (ok)
	{
		is3d_ = pSection->readBool( "3d" );
		soundRes_ = "sounds/" + pSection->readString( "resource" );
		transform_.translation( pSection->readVector3( "position" ) );

		loop_ = pSection->readBool( "loop", true );
		loopDelay_ = pSection->readFloat( "loopDelay", 0.f );
		loopDelayVariance_ = pSection->readFloat( "loopDelayVariance", 0.f );

		this->notifySoundMgr();
	}
	return ok;
}



/**
 *	Save any property changes to this data section
 */
bool EditorChunkSound::edSave( DataSectionPtr pSection )
{
	BW_GUARD;

	if (!edCommonSave( pSection ))
		return false;

	pSection->writeBool( "3d", is3d_ );
	pSection->writeString( "resource", soundRes_.substr(7) );
	pSection->writeVector3( "position", transform_.applyToOrigin() );

	// TODO: if the current values for the properties below are the same
	// as the defaults, delete their subsections from the data section.

	pSection->writeFloat( "min", pSound_->minDistance() );
	pSection->writeFloat( "max", pSound_->maxDistance() );

	pSection->writeFloat( "attenuation", pSound_->defaultAttenuation() );
	pSection->writeFloat( "outsideAtten", pSound_->outsideAtten() );
	pSection->writeFloat( "insideAtten", pSound_->insideAtten() );

	pSection->writeFloat( "centreHour", pSound_->centreHour() );
	pSection->writeFloat( "minHour", pSound_->minHour() );
	pSection->writeFloat( "maxHour", pSound_->maxHour() );

	pSection->writeBool( "loop", loop_ );
	pSection->writeFloat( "loopDelay", loopDelay_ );
	pSection->writeFloat( "loopDelayVariance", loopDelayVariance_ );

	return true;
}



/**
 *	Change our transform, temporarily or permanently
 */
bool EditorChunkSound::edTransform( const Matrix & m, bool transient )
{
	BW_GUARD;

	// if this is only a temporary change, keep it in the same chunk
	if (transient)
	{
		transform_ = m;
		pSound_->position( pChunk_->transform().applyPoint(
			transform_.applyToOrigin() ), 0 );
		this->notifySoundMgr();
		return true;
	}

	// it's permanent, so find out where we belong now
	Chunk * pOldChunk = pChunk_;
	Chunk * pNewChunk = this->edDropChunk( m.applyToOrigin() );
	if (pNewChunk == NULL) return false;

	// make sure the chunks aren't readonly
	if (!EditorChunkCache::instance( *pOldChunk ).edIsWriteable() 
		|| !EditorChunkCache::instance( *pNewChunk ).edIsWriteable())
		return false;

	// ok, accept the transform change then
	transform_.multiply( m, pOldChunk->transform() );
	transform_.postMultiply( pNewChunk->transformInverse() );
	pSound_->position( pChunk_->transform().applyPoint(
		transform_.applyToOrigin() ), 0 );
	this->notifySoundMgr();

	// note that both affected chunks have seen changes
	WorldManager::instance().changedChunk( pOldChunk );
	WorldManager::instance().changedChunk( pNewChunk );

	edMove( pOldChunk, pNewChunk );

	return true;
}


/**
 *	Add the properties of this flare to the given editor
 */
bool EditorChunkSound::edEdit( class GeneralEditor & editor )
{
	BW_GUARD;

	if (this->edFrozen())
		return false;

	if (!edCommonEdit( editor ))
	{
		return false;
	}

	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/THREED"),
		new SlowPropReloadingProxy<EditorChunkSound,BoolProxy>(
			this, "ambient or 3d selector", 
			&EditorChunkSound::is3dGet, 
			&EditorChunkSound::is3dSet ) ) );

	editor.addProperty( new ResourceProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/RESOURCE"),
		new SlowPropReloadingProxy<EditorChunkSound,StringProxy>(
			this, "sound resource", 
			&EditorChunkSound::resGet, 
			&EditorChunkSound::resSet ),
		".wav" /* for now */ ) );

	MatrixProxy * pMP = new ChunkItemMatrix( this );
	editor.addProperty( new GenPositionProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/POSITION"), pMP ) );

	editor.addProperty( new GenRadiusProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/INNER_RADIUS"),
		new AccessorDataProxy<EditorChunkSound,FloatProxy>(
			this, "inner radius", 
			&EditorChunkSound::innerRadiusGet, 
			&EditorChunkSound::innerRadiusSet ), pMP ) );
	editor.addProperty( new GenRadiusProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/OUTER_RADIUS"),
		new AccessorDataProxy<EditorChunkSound,FloatProxy>(
			this, "outer radius", 
			&EditorChunkSound::outerRadiusGet, 
			&EditorChunkSound::outerRadiusSet ), pMP ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/ATTENUATION"),
		new AccessorDataProxy<EditorChunkSound,FloatProxy>(
			this, "attenuation", 
			&EditorChunkSound::attenuationGet, 
			&EditorChunkSound::attenuationSet ) ) );
	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/OUTSIDE_EXTRA_ATTENUATION"),
		new AccessorDataProxy<EditorChunkSound,FloatProxy>(
			this, "outside extra attenutation",
			&EditorChunkSound::outsideAttenGet, 
			&EditorChunkSound::outsideAttenSet ) ) );
	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/INSIDE_EXTRA_ATTENUATION"),
		new AccessorDataProxy<EditorChunkSound,FloatProxy>(
			this, "inside extra attenutation",
			&EditorChunkSound::insideAttenGet, 
			&EditorChunkSound::insideAttenSet ) ) );

	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/CENTRE_HOUR"),
		new AccessorDataProxy<EditorChunkSound,FloatProxy>(
			this, "centre hour", 
			&EditorChunkSound::centreHourGet, 
			&EditorChunkSound::centreHourSet ) ) );
	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/INNER_HOUR"),
		new AccessorDataProxy<EditorChunkSound,FloatProxy>(
			this, "inner hours", 
			&EditorChunkSound::innerHoursGet, 
			&EditorChunkSound::innerHoursSet ) ) );
	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/OUTER_HOUR"),
		new AccessorDataProxy<EditorChunkSound,FloatProxy>(
			this, "outer hours", 
			&EditorChunkSound::outerHoursGet, 
			&EditorChunkSound::outerHoursSet ) ) );

	// There are no interfaces to these in Base3DSound so we have to
	//  set them by reloading the sound. Not too terrible for now.
	editor.addProperty( new GenBoolProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/LOOP"),
		new SlowPropReloadingProxy<EditorChunkSound,BoolProxy>(
			this, "loop enable flag", 
			&EditorChunkSound::loopGet, 
			&EditorChunkSound::loopSet ) ) );
	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/LOOP_DELAY"),
		new SlowPropReloadingProxy<EditorChunkSound,FloatProxy>(
			this, "loop delay", 
			&EditorChunkSound::loopDelayGet, 
			&EditorChunkSound::loopDelaySet ) ) );
	editor.addProperty( new GenFloatProperty(
		LocaliseStaticUTF8(L"WORLDEDITOR/WORLDEDITOR/CHUNK/EDITOR_CHUNK_SOUND/LOOP_DELAY_VARIANCE"),
		new SlowPropReloadingProxy<EditorChunkSound,FloatProxy>(
			this, "loop delay variance", 
			&EditorChunkSound::loopDVGet, 
			&EditorChunkSound::loopDVSet ) ) );

	return true;
}


/**
 *	Reload method. Note that this method hides the reload method in the
 *	template that we derive from, and causes it not to be ever generated.
 *	This is most fortunate, because that (hidden) method would not compile
 *	because it calls 'load' with the wrong arguments. More template fun!
 */
bool EditorChunkSound::reload()
{
	BW_GUARD;

	bool ok = this->load( pOwnSect_, pChunk_ );
	return ok;
}


#define NOEXEC rf
#define ECS_FLOAT_PROP( NAME, METH, EXEC, CHECK )		\
float EditorChunkSound::NAME##Get() const				\
{														\
	return pSound_->METH();								\
}														\
														\
bool EditorChunkSound::NAME##Set( const float & rf )	\
{														\
	float f = EXEC;										\
	if (!(CHECK)) return false;							\
	pSound_->METH( f );									\
	this->notifySoundMgr();								\
	return true;										\
}														\

// These restrictions on min and max are the sound manager's.
// It asserts (!) if a sound is loaded with min >= max.
ECS_FLOAT_PROP( innerRadius, minDistance,
	NOEXEC, f >= 0.f && f <= pSound_->maxDistance() )
ECS_FLOAT_PROP( outerRadius, maxDistance,
	NOEXEC, f >= 0.f && f >= pSound_->minDistance() )

// Only the default attenuation needs to be <= 0,
// the others can be either boost or a drop the volume.
ECS_FLOAT_PROP( attenuation, defaultAttenuation, NOEXEC, f <= 0.f )
ECS_FLOAT_PROP( outsideAtten, outsideAtten, NOEXEC, 1 )
ECS_FLOAT_PROP( insideAtten, insideAtten, NOEXEC, 1 )

// We check with this check condition and normalise the hour,
// but we still check to make sure it's not negative
// (not sure if fmod will always do this for us).
// SoundMgr does not do asserts on min and max hour.
ECS_FLOAT_PROP( centreHour, centreHour, fmodf( rf, 24.f ), f >= 0.f )
ECS_FLOAT_PROP( innerHours, minHour, fmodf( rf, 24.f ), f >= 0.f )
ECS_FLOAT_PROP( outerHours, maxHour, fmodf( rf, 24.f ), f >= 0.f )


/**
 *	This cleans up internally used memory and resources.
 */
/*static*/ void EditorChunkSound::fini()
{
	s_model = NULL;
}


/**
 *	Return a modelptr that is the representation of this chunk item
 */
ModelPtr EditorChunkSound::reprModel() const
{
	BW_GUARD;

	if (!s_model && !s_triedLoadOnce)
	{
		s_model = Model::get( "resources/models/sound.model" );
		s_triedLoadOnce = true;
	}
	return s_model;
}




/**
 *	This method lets the sound manager know that its sound has changed.
 *	
 *	Currently, we can only tell it to update all sounds ... but that's ok.
 */
void EditorChunkSound::notifySoundMgr()
{
	BW_GUARD;

	soundMgr().updateASAP();
}


/// Write the factory statics stuff
#undef IMPLEMENT_CHUNK_ITEM_ARGS
#define IMPLEMENT_CHUNK_ITEM_ARGS (pSection, pChunk)
IMPLEMENT_CHUNK_ITEM( EditorChunkSound, sound, 1 )

// editor_chunk_sound.cpp
