/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SOUNDMGR_HPP
#define SOUNDMGR_HPP

#error "This lib is deprecated. Use xactsnd lib instead."

#include <vector>
#include <map>
#include <float.h>

#include "moo/moo_math.hpp"
//#include "math/matrix34.hpp"
#include "resmgr/bin_section.hpp"
#include "cstdmf/time_queue.hpp"



class AmbientSound;
class AudioPath;
class AudioPathPool;
class Base3DSound;
class BaseSound;
class DX3DSound;
class FxSound;
class SimpleSound;
class SoundMgr;
class Static3DSound;


// forward declare these so I don't have to include dmusic header files
struct IDirectMusicLoader8;
struct IDirectMusicPerformance8;
struct IDirectMusicSegment8;
struct IDirectSound3DBuffer8;
struct IDirectSound3DListener8;
struct IDirectSoundBuffer8;

struct IDirectMusicAudioPath;
struct IDirectMusicLoader;
struct IDirectMusicPerformance;
struct IDirectMusicSegment;
struct IDirectSound3DBuffer;
struct IDirectSound3DListener;
struct IDirectSoundBuffer;



/**
 *	This singleton class manages sounds.
 */
class SoundMgr
{
	friend BaseSound;
	friend Base3DSound;
	friend DX3DSound;
	friend AmbientSound;
	friend Static3DSound;
	friend FxSound;
	friend SimpleSound;

	typedef std::vector<BaseSound*> BaseSoundVec;
	typedef std::vector<AmbientSound*> AmbientSoundVec;
	typedef std::vector<Static3DSound*> Static3DSoundVec;
	typedef std::vector<FxSound*> FxSoundVec;
	typedef std::vector<SimpleSound*> SimpleSoundVec;
	typedef std::multimap<std::string, FxSound*> FxMap;
	typedef std::map<std::string, BaseSound*> SoundMap;

public:
    ~SoundMgr();

	bool init(
		void* mainWindowHandle,	// use void* here to avoid needing windows.h
		int performanceChannels = 0,
		float rolloffFactor = 0,
		float dopplerFactor = 0,
		int maxFxPaths = 0,
		int maxAmbientPaths = 0,
		int maxStatic3DPaths = 0,
		int maxSimplePaths = 0);

	bool isInited() const { return inited_; }

	AmbientSound* loadAmbient(
			const char* filename,		/// filename here is also used as tag
			const Vector3& position,
			float	minDistance,
			float	maxDistance,
			float	defaultAtten=0.0f,
			float	outsideAtten=0,		/// extra attenuation when outside
			float	insideAtten=0,		/// extra attenuation when inside a shell
			float	centreHour=0.0f,	/// time used as 4th dimension to attenuate sound
			float	minHour=0.0f,
			float	maxHour=0.0f,
			bool	loop=true,			/// is the sound to be looped during playback?
			float	loopDelay=0,		/// how long to wait between retriggers (in secs)
			float	loopDelayVariance=0);/// maximum random variance added to loopDelay


	Static3DSound* loadStatic3D(
			const char* filename,	// filename here is also used as tag
			const Vector3& position,
			float	minDistance,
			float	maxDistance,
			float	defaultAtten=0.0f,
			float	outsideAtten=0.0f,	// extra attenuation when outside
			float	insideAtten=0.0f,	// extra attenuation when inside a shell
			float	centreHour=0.f,		// time used as 4th dimension to attenuate sound
			float	minHour=0.0f,
			float	maxHour=0.0f,
			bool	loop=true,			// is the sound to be looped during playback?
			float	loopDelay=0,		// how long to wait between retriggers (in secs)
			float	loopDelayVariance=0);// maximum random variance added to loopDelay



	FxSound* loadFx(
			const char*	filename,
			const char*	tag,		// these use a simpler explicit tag for script usage
			float		minDistance,
			float		maxDistance,
			float		defaultAtten=0.0f,
			uint		maxInstances=2,
			bool		loop=false);


	SimpleSound* loadSimple(
			const char*	filename,	// the basename is used as tag (no path or ext)
			float		defaultAtten,
			uint		maxInstances=1,
			bool		loop=false);


	void setListenerPos(
			const Matrix& cameraToWorldMatrix, 
			/*bool cameraInShell,*/
			const Vector3& playerPos,
			bool playerInShell,
			float minDimension,	// min dim across shell
			float hours,
			float totalTime);

	void playFx(const char* tag);	// play a sound fx that matches the tag
	void playFx(const char* tag, const Vector3& pos);
	void playFxDelayed(const char* tag, float delay, const Vector3& pos);
	void playFxDelayedAtten(const char* tag, float delay, float atten, const Vector3& pos);
	void playSimple(const char* tag);	// the tag is the basename of filename
	void playAmbient(const char* tag, const Vector3& worldPos);

	BaseSound* SoundMgr::findSound(const char* tag);
	FxSound* SoundMgr::findFxSound(const char* tag);

	void drawDebugStuff();
	void updateASAP();


	/// This method returns the singleton instance of this class.
	static inline SoundMgr& instance() { return instance_; }


	float			echoMix_;		// params for echo
	float			echoFeedback_;
	float			echoDelay_;

private:
	SoundMgr();
	SoundMgr(const SoundMgr&);				// NULL copy constructor
	SoundMgr& operator=(const SoundMgr&);	// NULL assignment operator

	BaseSound* load(const char* filename, float defaultAtten);

	static SoundMgr instance_;


	void setListenerPosAmbient(const Vector3& position, float hours, bool inShell);
	void restart3DSounds(const Vector3& position, float hours);


	bool inited_;				// class initialised?
	IDirectMusicLoader8*		pLoader_;
	IDirectMusicPerformance8*	pPerformance_;
	IDirectSound3DListener*		pListener_;
	IDirectMusicAudioPath*		pDefAudioPath_;	// default audio path


	BaseSoundVec		baseList_;		// vector of all loaded Base sounds
	AmbientSoundVec		ambientList_;	// vector of all loaded Ambient sounds
	Static3DSoundVec	static3DList_;	// vector of all loaded Static sounds
	FxSoundVec			fxList_;		// vector of all loaded Fx sounds
	SimpleSoundVec		simpleList_;	// vector of all loaded Simple sounds

	FxMap			fxMap_;		// translates between fx tags and sounds
	SoundMap		soundMap_;		// translates between generic tags and sounds

	Vector3			listenerPos_;	// last known position of listener
	bool			inShell_;		// last known situation of listener
	float			shellSize_;		// min dimension across shell

	uint32			memoryUsed_;	// #bytes used so far in loaded sounds
	uint32			memoryUsedK_;	// #Kbytes used so far in loaded sounds

	TimeQueue		timeQueue_;
	float			currentTime_;	// in seconds
};


inline SoundMgr& soundMgr() { return SoundMgr::instance(); }




/** Semi-abstract Class to represent the simplest sound. Doesn't support 3D.
 */
class BaseSound
{
	friend SoundMgr;
	friend AudioPath;

public:
	virtual ~BaseSound();

	virtual bool load(const char* filename);
	virtual int play();
	virtual int play(float attenuation);
	void stop(uint instance);	/// stop a particular instance
	void stopAll();				/// stop all instances

	bool isPlaying() const;
	bool isPlaying(uint instance) const;

	/// Change the current volume if the sound
	float attenuation() const { return attenuation_; }
	virtual void attenuation(const float dbAttenuation, uint instance=0);

	/// change the default volume of the sound
	virtual float defaultAttenuation() const { return defaultAtten_; }
	virtual void defaultAttenuation(float dbAttenuation);

	void frequency(uint Hz, uint inst);	/// set the playback frequency
	uint frequency(uint inst) const;	/// get the playback frequency

	static Watcher& watcher();

	/// get the name of the sound
	std::string tag() const { return tag_; }


protected:
	/// constructor is hidden 'cause I want people to use soundMgr::load()
	BaseSound(std::string tag, float defaultAtten, uint maxInstances);

	BaseSound(const BaseSound&);			///< NULL copy constructor
	BaseSound& operator=(const BaseSound&);	///< NULL assignment operator

	virtual void getDXObjects(AudioPath& audioPath) const;

	int selectPathIdx();

	std::string printableTag() const;

	IDirectMusicSegment8*	pSegment_;			///< the sound

	std::vector<AudioPath*>	audioPaths_;		///< current paths (if playing, one per instance)
	AudioPathPool*			audioPathPoolCopy_;	///< managed by subclasses

	BinaryPtr sndBits_;		///< the actual data from the resmgr

	float	attenuation_;	///< in db.  0 is full volume, -10.0 is half etc
	long	dxAttenuation_;	///< cache of attenuation as used for dx
	float	defaultAtten_;	///< in db, to set maximum volume of a sound (added to normal volume)
	bool	isPlaying_;		///< is the sound currently playing?
	std::string	tag_;		///< name of sound
	uint	maxInstances_;	///< max #instances of this sound that can play simultaneously
};



/**
 *	Abstract class to represent the simplest 3D sound.
 *
 *	This class is used as a base class to build the DX 3D sound class
 *	and the stereo 3D sound class.
 */
class Base3DSound : public BaseSound, public TimerHandler
{
	friend SoundMgr;

public:

	virtual ~Base3DSound();

	virtual int play() = 0;
	virtual int play(float attenuation, const Vector3& pos) = 0;

	/// Move the sound to a different location in world coordinates
	virtual const Vector3& position(uint instance) const { return position_[instance]; }
	virtual void position(const Vector3& newPosition, uint instance);

	// for watcher user
	const Vector3& position0() const	{ return position(0); }
	void position0(const Vector3& newPosition) { position(newPosition, 0); };

	/// Change the distance at which the sound is maximum volume (inner sphere)
	virtual float minDistance() const	{ return minDistance_; }
	virtual void minDistance(float minDist);

	/// Change the distance at which the sound is zero volume (outer sphere)
	virtual float maxDistance() const	{ return maxDistance_; }
	virtual void maxDistance(float maxDist);

	/// Change the time component of the sound's position
	float centreHour() const			{ return centreHour_; }
	void centreHour( float t )			{ centreHour_ = t; }

	/// Change the time 'distance' at which the sound is maximum volume
	float minHour() const				{ return minHour_; }
	void minHour( float t )				{ minHour_ = t; }

	/// Change the time 'distance' at which the sound is zero volume
	float maxHour() const				{ return maxHour_; }
	void maxHour( float t )				{ maxHour_ = t; }

	/// Change the extra outside attenuation
	float outsideAtten() const			{ return outsideAtten_; }
	void outsideAtten( float a )		{ outsideAtten_ = a; }

	/// Change the extra inside attenuation
	float insideAtten() const			{ return insideAtten_; }
	void insideAtten( float a )			{ insideAtten_ = a; }

	virtual void handleTimeout( TimeQueueId id, void * pUser );
	virtual void onRelease( TimeQueueId id, void * pUser );


protected:
	Base3DSound(
		std::string	tag,
		const Vector3& position,
		float	minDistance,
		float	maxDistance,
		float	defaultAtten,
		float	outsideAtten,
		float	insideAtten,
		float	centreHour,	// time used as 4th dimension to attenuate sound
		float	minHour,
		float	maxHour,
		int		maxInstances
	);

	Base3DSound(const Base3DSound&);			///< NULL copy constructor
	Base3DSound& operator=(const Base3DSound&);	///< NULL assignment operator

	//virtual void getDXObjects(AudioPath& audioPath);	// implementation not needed

	TimeQueueId	 tid_;		///< id of any queued sounds (if looping with delay)
	std::vector<Vector3> position_;	///< location of the sound in world coords
	float	minDistance_;	///< the point at which the sound is at min attenuation (loud)
	float	maxDistance_;	///< the point at which the sound is at max attenuation (v.soft)
	float	maxDistanceSquared_; ///< cached value to speed up some comparisons
	float	centreHour_;	///< time used as 4th dimension to attenuate sound
	float	minHour_;		///< centreHour +/-minHour is max vol
	float	maxHour_;		///< time outside maxHour is min vol
	float	priority_;		///< lower positive numbers are higher priority
	float	outsideAtten_;	///< extra attenuation when outside
	float	insideAtten_;	///< extra attenuation when inside a shell
};




/**
 *	This class is used to represent a single 3D ambient sound.
 *
 *	Sound is positioned in 3D, but is not directional (doesn't use DX8 3d
 *	routines).
 *
 *	Used for background sound (wind noises, general machinery noise, atmospheric
 *	noises).  Sound is generally playing constantly (and therefore looped), but can
 *	be singleshot if desired.
 */
class AmbientSound : public Base3DSound
{
	friend SoundMgr;

public:
	virtual ~AmbientSound();

	virtual int play();
	virtual int play(float attenuation, const Vector3& position);

	virtual bool load(const char* filename);

	// in case people call position() from the baseclass
	// (ambient sound can only have a single instance)
	virtual const Vector3& position(uint inst) const
		{ MF_ASSERT(inst==0); return position_[0]; }
	virtual void position(const Vector3& newPosition, uint inst)
		{ MF_ASSERT(inst==0); position(newPosition); }

	const Vector3& position() const { return position_[0]; }
	void position(const Vector3& newPosition);

	void addPosition(const Vector3& position, float minDistance, float maxDistance);

	virtual float minDistance() const { return positionData_[0].minDistance; }
	virtual void minDistance(float minDist) { positionData_[0].minDistance = minDist; }

	virtual float maxDistance() const { return positionData_[0].maxDistance; }
	virtual void maxDistance(float maxDist)
		{ positionData_[0].maxDistance = maxDist,
		positionData_[0].maxDistanceSquared = maxDist * maxDist; }

	static Watcher& watcher();

private:
	AmbientSound(const AmbientSound&);				// NULL copy constructor
	AmbientSound& operator=(const AmbientSound&);	// NULL assignment operator

	// constructor is hidden 'cause I want people to use soundMgr::load()
	AmbientSound(
			std::string	tag,
			const Vector3& position,
			float	minDistance,
			float	maxDistance,
			float	defaultAtten,
			float	outsideAtten,
			float	insideAtten,
			float	centreHour,	// time used as 4th dimension to attenuate sound
			float	minHour,
			float	maxHour,
			bool	loop,		// is the sound looped?
			float	loopDelay,
			float	loopDelayVariance
	);

	struct PositionData {
		Vector3 position;
		float minDistance; // the point at which the sound is at min attenuation (loud)
		float maxDistance; // the point at which the sound is at max attenuation (v.soft)
		float maxDistanceSquared;	// optimisation use of maxDistance
	};

	std::vector<PositionData> positionData_;
	bool loop_;
	float loopDelay_;
	float loopDelayVariance_;

	static AudioPathPool* audioPathPool_;
};





/** Abstract class to represent a DX 3D sound.
 */
class DX3DSound : public Base3DSound
{
	friend SoundMgr;

public:
	virtual ~DX3DSound();

	virtual int play();
	virtual int play(float attenuation, const Vector3& pos);

	virtual void playDelayed(float delay, const Vector3& pos);
	virtual void playDelayedAtten(float delay, float attenuation, const Vector3& pos);

	virtual const Vector3& position(uint instance_);
	virtual void position(const Vector3& newPosition, uint instance);
	virtual void position(const Vector3& newPosition, float dTime, uint instance);

	virtual float minDistance() const  { return minDistance_; };
	virtual void minDistance(float minDist);

	virtual float maxDistance() const  { return maxDistance_; };
	virtual void maxDistance(float maxDist);

	virtual const Vector3& velocity(uint instance) const  { return velocity_[instance]; };
	virtual void velocity(const Vector3& newVelocity, uint instance);


protected:
	DX3DSound(const DX3DSound&);			///< NULL copy constructor
	DX3DSound& operator=(const DX3DSound&);	///< NULL assignment operator

	// constructor is hidden 'cause I want people to use soundMgr::load()
	DX3DSound(std::string tag,
			const Vector3& position,
			float	minDistance,
			float	maxDistance,
			float	defaultAtten,
			float	outsideAtten,
			float	insideAtten,
			float	centreHour,	// time used as 4th dimension to attenuate sound
			float	minHour,
			float	maxHour,
			uint	maxInstances,
			bool	loop);

	virtual void getDXObjects(AudioPath& audioPath) const;

	bool			loop_;		// does the sound loop?
	std::vector<Vector3> velocity_;	// velocity of the sound (only used for doppler calcs)
};



/**
 *	Class to represent a single 3D sound.
 *
 *	Sound is 3D, but never moves
 *	Used for crickets, windmills etc
 */
class Static3DSound : public DX3DSound
{
	friend SoundMgr;

public:
	virtual ~Static3DSound();

	virtual int play();
	virtual int play(float attenuation, const Vector3& pos);

	static Watcher& watcher();

private:
	Static3DSound(const Static3DSound&);			// NULL copy constructor
	Static3DSound& operator=(const Static3DSound&);	// NULL assignment operator

	// constructor is hidden 'cause I want people to use soundMgr::load()
	Static3DSound(std::string tag,
			const Vector3& position,
			float	minDistance,
			float	maxDistance,
			float	defaultAtten,
			float	outsideAtten,
			float	insideAtten,
			float	centreHour,	// time used as 4th dimension to attenuate sound
			float	minHour,
			float	maxHour,
			bool	loop,
			float	loopDelay,
			float	loopDelayVariance);

	static AudioPathPool* audioPathPool_;

	float loopDelay_;
	float loopDelayVariance_;
	bool hasTriggered_;	// has the sound been started (in range)
};


/**
 *	Class to represent a single Fx sound.
 *
 *	A sound referenced by a single label, but representing multiple sounds
 *	Used for gun noises etc
 */
class FxSound : public DX3DSound
{
	friend SoundMgr;

public:
	virtual ~FxSound();

	static Watcher& watcher();

private:
	FxSound(const FxSound&);			// NULL copy constructor
	FxSound& operator=(const FxSound&);	// NULL assignment operator

	// constructor is hidden 'cause I want people to use soundMgr::load()
	FxSound(std::string tag, float minDistance, float maxDistance, float defaultAtten,
		uint maxInstances, bool loop);

	static AudioPathPool* audioPathPool_;
};


/**
 *	Class to represent a single Simple sound.
 *
 *	A sound referenced by a label which is derived from the filename. Sound is
 *	not 3D.  Can be stereo or mono. Used for user interface feedback noises and
 *	some simple environment noises that are not 3D (eg thunder & rain)
 */
class SimpleSound : public BaseSound
{
	friend SoundMgr;

public:
	virtual ~SimpleSound();

	virtual int play();
	virtual int play(float attenuation);

	static Watcher& watcher();

private:
	SimpleSound(const SimpleSound&);			// NULL copy constructor
	SimpleSound& operator=(const SimpleSound&);	// NULL assignment operator

	// constructor is hidden 'cause I want people to use soundMgr::load*()
	SimpleSound(std::string tag, float defaultAtten, uint maxInstances, bool loop);

	static AudioPathPool* audioPathPool_;

	bool loop_;
};





#ifdef CODE_INLINE
#include "soundmgr.ipp"
#endif


#endif // SOUND_HPP
