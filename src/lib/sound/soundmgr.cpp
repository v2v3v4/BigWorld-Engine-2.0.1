/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/* TODO
 *
 * - record some daytime sounds
 *
 * - efx: parametric (cut treble) + reverb (heavy, plus more treble atten) to
 *		get distance muffling
 *
 * - birds (noise): needs Dymanic3DSound
 *
 * - envelopes
 *
 * - doppler shift
 *
 * - change fade curve for ambient sounds (log10)
 *
 * - make sound spheres elipses
 */


#include "pch.hpp"

#include <algorithm>
#define INITGUID
#define __IReferenceClock_INTERFACE_DEFINED__	// now in strmiids.lib
#define _IKsPropertySet_						// now in strmiids.lib

#include <dmusici.h>
#include <ks.h>		// for the definition of GUID_NULL

// WARNING: If these symbols aren't undefined you get multiple
// definitions of DX8 D3D guids when linking
#undef INITGUID
#undef DEFINE_GUID
#define DEFINE_GUID(a,b,c,d,e,f,g,h,i,j,k,l)

#include "cstdmf/debug.hpp"
#include "cstdmf/watcher.hpp"
#include "resmgr/bwresource.hpp"
#include "cstdmf/dogwatch.hpp"

#ifndef _RELEASE
#include "moo/moo_math.hpp"
#include "moo/render_context.hpp"
#include "romp/geometrics.hpp"
#include "romp/font.hpp"
#include "ashes/gui_shader.hpp"
#include "ashes/simple_gui.hpp"
#include "ashes/text_gui_component.hpp"
#endif

#include "romp/time_of_day.hpp"

#include "soundmgr.hpp"
#ifndef CODE_INLINE
#include "soundmgr.ipp"
#endif


typedef std::string string;
#define vector std::vector


DECLARE_DEBUG_COMPONENT2( "Sound", 0 )	// debugLevel for this file

// Have to do this 'coz there's no easy way to just turn of trace msgs
//  (as far as I can see)
#undef TRACE_MSG
#define TRACE_MSG nullTraceMsg
inline void nullTraceMsg( const char * format, ... ) { }

static const float MAX_ATTENUATION = 80.0f;	// measured in db

const int TIMER_SCALE = 100;	/* this is the resolution of the delay timer 1sec/TIMER_SCALE) */
						/* 10 == 0.1sec resolution */

struct TimerInfo {
	enum {LOOPDELAY, PLAYDELAY} type;
	Vector3		pos;
	float		attenuation;
};

static int MAX_EFX_PATHS		= 32;
static int MAX_AMBIENT_PATHS	= 5;
static int MAX_STATIC3D_PATHS	= 5;
static int MAX_SIMPLE_PATHS		= 8;

static bool showAmbientSpheres = false;
static bool showStatic3DSpheres = false;

DogWatch g_watchSound("Sound");




static void setMidiVolume(IDirectMusicPerformance8* pPerformance, int volume)

{
	IDirectMusicGraph* pGraph;

	// Get the graph pointer from the performance. If you wanted the
	// message to go through a segment graph, you would
	// QueryInterface a segment object instead.

	if (SUCCEEDED(pPerformance->QueryInterface(IID_IDirectMusicGraph, (void**)&pGraph)))
	{
		// Allocate a DMUS_MIDI_PMSG of the appropriate size,
		// and read the system exclusive data into it.

		int i;
		for (i=0; i <= 0xf; i++) {
			DMUS_MIDI_PMSG* pMsg;

			if ( SUCCEEDED( pPerformance->AllocPMsg(
							sizeof(DMUS_MIDI_PMSG), (DMUS_PMSG**)&pMsg )))
			{
				ZeroMemory(pMsg, sizeof(DMUS_MIDI_PMSG));
				pMsg->dwSize = sizeof(DMUS_MIDI_PMSG);
				pMsg->mtTime = 0;
				pMsg->dwFlags = DMUS_PMSGF_REFTIME;
				pMsg->dwType = DMUS_PMSGT_MIDI;
									// PMsg is sent on all PChannels of the performance.
				pMsg->dwPChannel = DMUS_PCHANNEL_BROADCAST_PERFORMANCE;
				pMsg->bStatus = 0xb0 | i;	// controller cmd/chan0
				pMsg->bByte1 = 0x07;		// command to set channel volume
				pMsg->bByte2 = volume;		// volume

				pGraph->StampPMsg((DMUS_PMSG*)pMsg);
				if (FAILED(pPerformance->SendPMsg( (DMUS_PMSG*)pMsg ))) {
					pPerformance->FreePMsg( (DMUS_PMSG*)pMsg );
				} else {
					//HACK_MSG("SoundMgr::setMidiVolume: SendPMsg success\n");
				}
			}
		}

		pGraph->Release();
	}
}



// -----------------------------------------------------------------------------
//                       A u d i o P a t h P o o l
// -----------------------------------------------------------------------------


/* This class is used to manage a single dx audiopath.
 */
class AudioPath
{
public:
	AudioPath() :
		inUse_(false),
		priority_(FLT_MAX),
		snd_(NULL),
		instance_(-1),
		hasFx_(false),
		pDSPath_(NULL),
		pDSB_(NULL),
		pDS3DB_(NULL),
		pDSSS_( NULL )
	{
		TRACE_MSG("AudioPath::AudioPath:\n");
	};

	~AudioPath()
	{
		TRACE_MSG("AudioPath::~AudioPath: path=%p\n", pDSPath_);
		if (pDSPath_) pDSPath_->Release();
		pDSPath_ = NULL;
	};

	void releasePath();
	void activatePath(BaseSound& snd);
	void addReverb(float shellSize);
	void addEcho();

	AudioPath(const AudioPath&);			// NULL copy constructor
	AudioPath& operator=(const AudioPath&);	// NULL assignment operator

	bool		inUse_;			// being used?
	float		priority_;		// bigger number == lower priority
	BaseSound*	snd_;			// sound being played on this path
	uint		instance_;		// which instance of this sound are we?
	bool		hasFx_;			// any effects on this path?

	IDirectMusicAudioPath*		pDSPath_;	// we create this
    IDirectSoundBuffer8*		pDSB_;		// this is extracted from pDSPath and cached
	IDirectSound3DBuffer8*		pDS3DB_;	// this is extracted from pDSPath and cached
	IDirectMusicSegmentState*	pDSSS_;		// ptr received when sound started
};




/**
 *	This method marks a path as not in use and releases its DX resources.
 */
void AudioPath::releasePath()
{
	//TRACE_MSG("AudioPath::releasePath:\n");

	inUse_ = false;

	// deactivate the path. This ensures that all audio in the buffer is cleared.
	pDSPath_->Activate(false);

	MF_ASSERT(pDSB_);	// TODO: I suspect that DX returns the same DSB for this audioPath
	if (pDSB_) {		// so maybe I don't have to release and fetch these pts each time
		// turn off any effects that might have been selected
		if (hasFx_) {
			pDSB_->SetFX(0, NULL, NULL);
			hasFx_ = false;
		}
		pDSB_->Release();
		pDSB_ = NULL;
	}

	if (pDS3DB_) {		// won't be set for AmbientSounds
		pDS3DB_->Release();
		pDS3DB_ = NULL;
	}
}


/**
 *	This method prepares a path for use.
 */
void AudioPath::activatePath(BaseSound& snd)
{
	//TRACE_MSG("AudioPath::activatePath: %s\n", snd.tag_.c_str());
	MF_ASSERT(pDSPath_);

	inUse_ = true;
	snd_ = &snd;

	pDSPath_->Activate(true);

	// get an access ptr to the dx sound buffer so we can change settings
	snd.getDXObjects(*this);
}



void AudioPath::addReverb(float shellSize)
{
	//DEBUG_MSG("AudioPath::addReverb:\n");

	pDSPath_->Activate(false);

	// Describe the effect.

	DSEFFECTDESC dsEffect;
	dsEffect.dwSize = sizeof(DSEFFECTDESC);
	dsEffect.dwFlags = 0;
	dsEffect.guidDSFXClass = GUID_DSFX_WAVES_REVERB;//GUID_DSFX_STANDARD_I3DL2REVERB;//GUID_DSFX_STANDARD_ECHO;
	dsEffect.dwReserved1 = 0;
	dsEffect.dwReserved2 = 0;

	DWORD dwResults;
	HRESULT hr;

	// Set the effect
	if (FAILED(hr = pDSB_->SetFX(1, &dsEffect, &dwResults))) {
		ERROR_MSG("AudioPath::addReverb: SetFx failed\n");
		pDSPath_->Activate(true);
		return;
	}

	hasFx_ = true;

	// see if and how the effect was allocated.
/*	switch (dwResults) {
	  case DSFXR_LOCHARDWARE: DEBUG_MSG("AudioPath::addReverb: DSFXR_LOCHARDWARE\n"); break;
	  case DSFXR_LOCSOFTWARE: DEBUG_MSG("AudioPath::addReverb: DSFXR_LOCSOFTWARE\n"); break;
	  case DSFXR_UNALLOCATED: DEBUG_MSG("AudioPath::addReverb: DSFXR_UNALLOCATED\n"); break;
	  case DSFXR_FAILED:	  DEBUG_MSG("AudioPath::addReverb: DSFXR_FAILED\n"); break;
	  case DSFXR_PRESENT:	  DEBUG_MSG("AudioPath::addReverb: DSFXR_PRESENT\n"); break;
	  case DSFXR_UNKNOWN:	  DEBUG_MSG("AudioPath::addReverb: DSFXR_UNKNOWN\n"); break;
	  default:				  DEBUG_MSG("AudioPath::addReverb: dwResults=%lx\n", dwResults); break;
	}
*/

/*
	IDirectSoundFXI3DL2Reverb8 *pReverb;
	if (pDSPath_->GetObjectInPath(
		DMUS_PCHANNEL_ALL,					// Performance channel
		DMUS_PATH_BUFFER_DMO,				// Stage in the path
		0,									// Index of buffer in chain
		GUID_All_Objects,					// Class of object
		0,									// Index of object in buffer: ignored
		IID_IDirectSoundFXI3DL2Reverb8,		// GUID of desired interface
		(LPVOID*)&pReverb					// Pointer that receives interface
	) != S_OK) {
		DEBUG_MSG("AudioPath::addReverb: get reverb buffer failed\n");
	}
	else {

		pReverb->SetPreset(DSFX_I3DL2_ENVIRONMENT_PRESET_CAVE);
		//pReverb->SetPreset(DSFX_I3DL2_ENVIRONMENT_PRESET_CONCERTHALL);
		//pReverb->SetPreset(DSFX_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR);
		//pReverb->SetPreset(DSFX_I3DL2_ENVIRONMENT_PRESET_MOUNTAINS);
		//pReverb->SetPreset(DSFX_I3DL2_ENVIRONMENT_PRESET_UNDERWATER);

		DSFXI3DL2Reverb r;
		pReverb->GetAllParameters(&r);
		r.lRoom = DSFX_I3DL2REVERB_ROOM_MAX;
		pReverb->SetAllParameters(&r);

		//LONG    lRoom;                  // [-10000, 0]      default: -1000 mB
		//LONG    lRoomHF;                // [-10000, 0]      default: 0 mB
		//FLOAT   flRoomRolloffFactor;    // [0.0, 10.0]      default: 0.0
		//FLOAT   flDecayTime;            // [0.1, 20.0]      default: 1.49s
		//FLOAT   flDecayHFRatio;         // [0.1, 2.0]       default: 0.83
		//LONG    lReflections;           // [-10000, 1000]   default: -2602 mB
		//FLOAT   flReflectionsDelay;     // [0.0, 0.3]       default: 0.007 s
		//LONG    lReverb;                // [-10000, 2000]   default: 200 mB
		//FLOAT   flReverbDelay;          // [0.0, 0.1]       default: 0.011 s
		//FLOAT   flDiffusion;            // [0.0, 100.0]     default: 100.0 %
		//FLOAT   flDensity;              // [0.0, 100.0]     default: 100.0 %
		//FLOAT   flHFReference;          // [20.0, 20000.0]  default: 5000.0 Hz
	}
	*/

	IDirectSoundFXWavesReverb *pReverb;
	if (pDSPath_->GetObjectInPath(
		DMUS_PCHANNEL_ALL,					// Performance channel
		DMUS_PATH_BUFFER_DMO,				// Stage in the path
		0,									// Index of buffer in chain
		GUID_All_Objects,					// Class of object
		0,									// Index of object in buffer: ignored
		IID_IDirectSoundFXWavesReverb,		// GUID of desired interface
		(LPVOID*)&pReverb					// Pointer that receives interface
	) != S_OK) {
		ERROR_MSG("AudioPath::addReverb: get reverb buffer failed\n");
	}
	else {

		DSFXWavesReverb r;
		pReverb->GetAllParameters(&r);

		const float SMALLRM_REVERB_TIME	 = 500.0f;
		const float BIGRM_REVERB_TIME	 = 3000.0f;
		const float SMALLRM_REVERB_ATTEN = -16.0f;
		const float BIGRM_REVERB_ATTEN	 = -10.0f;

		const float MIN_SHELL_SZ =  4.0f;
		const float MAX_SHELL_SZ = 20.0f;

		const float REVERB_TIME_FACTOR =
			((BIGRM_REVERB_TIME - SMALLRM_REVERB_TIME) / (MAX_SHELL_SZ - MIN_SHELL_SZ));

		const float REVERB_ATTEN_FACTOR	=
			((BIGRM_REVERB_ATTEN - SMALLRM_REVERB_ATTEN) / (MAX_SHELL_SZ - MIN_SHELL_SZ));

		float clampedShellSize = Math::clamp(MIN_SHELL_SZ, shellSize, MAX_SHELL_SZ);

		r.fReverbMix = ((clampedShellSize - MIN_SHELL_SZ) * REVERB_ATTEN_FACTOR)
						+ SMALLRM_REVERB_ATTEN;
		r.fReverbTime = ((clampedShellSize - MIN_SHELL_SZ) * REVERB_TIME_FACTOR)
						+ SMALLRM_REVERB_TIME;

		//DEBUG_MSG("AudioPath::addReverb: shellSz=%1.1f reverbMix=%1.1f reverbTime=%1.0f\n",
		//	shellSize, r.fReverbMix, r.fReverbTime);

		r.fInGain =	DSFX_WAVESREVERB_INGAIN_DEFAULT;		// [-96.0,0.0]    default: 0.0 dB
		r.fHighFreqRTRatio =
					DSFX_WAVESREVERB_HIGHFREQRTRATIO_MIN;	// [0.001,0.999]  default: 0.001

		pReverb->SetAllParameters(&r);

		pReverb->Release();
	}

	pDSPath_->Activate(true);	// reactivate the path
}


void AudioPath::addEcho()
{
	//TRACE_MSG("AudioPath::addEcho:\n");

	pDSPath_->Activate(false);

	// Describe the effect.

	DSEFFECTDESC dsEffect;
	dsEffect.dwSize = sizeof(DSEFFECTDESC);
	dsEffect.dwFlags = 0;
	dsEffect.guidDSFXClass = GUID_DSFX_STANDARD_ECHO;
	dsEffect.dwReserved1 = 0;
	dsEffect.dwReserved2 = 0;

	DWORD dwResults;
	HRESULT hr;

	// Set the effect
	if (FAILED(hr = pDSB_->SetFX(1, &dsEffect, &dwResults))) {
		ERROR_MSG("AudioPath::addEcho: SetFx failed\n");
		pDSPath_->Activate(true);
		return;
	}

	hasFx_ = true;


	IDirectSoundFXEcho *pEcho;
	if (pDSPath_->GetObjectInPath(
		DMUS_PCHANNEL_ALL,					// Performance channel
		DMUS_PATH_BUFFER_DMO,				// Stage in the path
		0,									// Index of buffer in chain
		GUID_All_Objects,					// Class of object
		0,									// Index of object in buffer: ignored
		IID_IDirectSoundFXEcho,				// GUID of desired interface
		(LPVOID*)&pEcho						// Pointer that receives interface
	) != S_OK) {
		ERROR_MSG("AudioPath::addEcho: get DMO failed\n");
	}
	else {
		DSFXEcho e;
		pEcho->GetAllParameters(&e);

		e.fWetDryMix = soundMgr().echoMix_;
		e.fFeedback = soundMgr().echoFeedback_;
		e.fLeftDelay = soundMgr().echoDelay_;
		e.fRightDelay = soundMgr().echoDelay_;
		e.lPanDelay = DSFXECHO_PANDELAY_MIN;

		pEcho->SetAllParameters(&e);

		pEcho->Release();
	}

	pDSPath_->Activate(true);	// reactivate the path
}



/* Class to manage a pool of audio paths.
 */
class AudioPathPool
{
public:
	AudioPathPool(int nPaths, int pathType, IDirectMusicPerformance8* pPerformance);
	~AudioPathPool();

	AudioPath* allocPath(BaseSound& snd);

	int pathsInUse() const;

private:
	AudioPathPool(const AudioPathPool&);			// NULL copy constructor
	AudioPathPool& operator=(const AudioPathPool&);	// NULL assignment operator

	vector<AudioPath*> pAudioPaths_;
	IDirectMusicPerformance8* pPerformance_;
};



/** Constructor
 */
AudioPathPool::AudioPathPool(int nPaths, int type, IDirectMusicPerformance8* pPerformance)
	: pPerformance_(pPerformance)
{
	TRACE_MSG("AudioPathPool::AudioPathPool: %d\n", nPaths);
	MF_ASSERT(nPaths > 0 && nPaths < 64);

	pAudioPaths_.reserve(nPaths);	// more efficient adding

	while (nPaths--) {
		AudioPath *path = new AudioPath;

		if (pPerformance->CreateStandardAudioPath(
			type,				// DMUS_APATH_DYNAMIC_STEREO | DMUS_APATH_DYNAMIC_3D etc
			1,					// number of performance channels.
			FALSE,				// activate now.
			&path->pDSPath_		// pointer that receives audiopath.
		) != S_OK) {
			ERROR_MSG("AudioPathPool::AudioPathPool: CreateStandardAudioPath() failed\n");
			return;
		}
		pAudioPaths_.push_back(path);
		setMidiVolume(pPerformance, 127);	// set to max volume
	}
}



/** Destructor
 */
AudioPathPool::~AudioPathPool()
{
	TRACE_MSG("AudioPathPool::~AudioPathPool:\n");

	vector<AudioPath*>::iterator ipath;
	for (ipath = pAudioPaths_.begin();  ipath != pAudioPaths_.end();  ipath++) {
		MF_ASSERT(*ipath);
		delete *ipath;
	}
}


/**
 *	Find and return a path for use.
 *
 *	Returns the first not in use path it comes across.
 *	If all paths are in use the least necessary path (based on priority) is
 *	returned.
 *	Must call releasePath on the returned object when finished.
 *
 *	NB: for priority, 0=highest pri, FLT_MAX=lowest
 *	NB: the returned path has its inUse flag set
 *
 *	@param snd The sound that a path is being activated for
 *	@return The audio path to use
 */
AudioPath* AudioPathPool::allocPath(BaseSound& snd)
{
	//TRACE_MSG("AudioPathPool::allocPath: [%s] paths in use: %d/%d\n",
	//	snd.tag().c_str(), pathsInUse(), pAudioPaths_.size());

	MF_ASSERT(pAudioPaths_.size() > 0);

	AudioPath* bestPath = NULL;
	float bestPri = 0;
	vector<AudioPath*>::iterator ipath;
	MUSIC_TIME now;

	pPerformance_->GetTime(NULL, &now);

	for (ipath = pAudioPaths_.begin();  ipath != pAudioPaths_.end();  ipath++) {

		AudioPath* path = *ipath;

		if (path->inUse_) {
			MF_ASSERT(path->snd_);
			if (!path->snd_->isPlaying(path->instance_)) {
				TRACE_MSG( "AudioPathPool::allocPath: Reusing path %d (was playing instance %d)\n",
					ipath - pAudioPaths_.begin(), path->instance_ );
				//DEBUG_MSG("AudioPathPool::allocPath: releasing a path !isPlaying (path=%p instance=%d)\n", path, path->instance_);
				path->snd_->stop(path->instance_);	// might have finished playing by itself, so stop it properly
				// NB: stop does a releasePath()
				//MF_ASSERT( !path->inUse_ );	TODO: Uncomment this assert
			}
		}

		if (!path->inUse_) {
			path->activatePath(snd);
			//TRACE_MSG("AudioPathPool::allocPath: returning path: %p\n", path);
			return path;
		}
		else {
			MUSIC_TIME tm;
			path->pDSSS_->GetStartTime(&tm);
			//DEBUG_MSG("AudioPathPool::allocPath: time diff: %ld\n", now - tm);
			float priority = (path->priority_ + 1) * ((abs(now - tm) / 5000) + 1);
			if (priority >= bestPri) {
				bestPri = priority;
				bestPath = path;
			}
		}
	}

	// all paths are in use.  The best path to use is in 'bestPath'
	// we must stop the sound playing here before returning the path for use

	MF_ASSERT(bestPath);

	//DEBUG_MSG("AudioPathPool::allocPath: paths all allocated: bestPri=%1.2f\n", bestPri);

	bestPath->snd_->stop(bestPath->instance_);		// NB: stop does a releasePath()
	bestPath->activatePath(snd);
	return bestPath;
}


/**
 *	This method returns the number of paths currently in use.
 */
int AudioPathPool::pathsInUse() const
{
	int inUse = 0;
	vector<AudioPath*>::const_iterator ipath;

	for (ipath = pAudioPaths_.begin();  ipath != pAudioPaths_.end();  ipath++) {
		if ((*ipath)->inUse_)
			inUse++;
	}

	return inUse;
}







// -------------------------------------------------------------------------------
//                              S o u n d M g r
// -------------------------------------------------------------------------------



SoundMgr SoundMgr::instance_;



/** Constructor
 */
SoundMgr::SoundMgr() :
	listenerPos_( 0.f, 0.f, 0.f )
{
	TRACE_MSG("SoundMgr::SoundMgr:\n");

	// initialise the watchers

	Watcher::rootWatcher().addChild("SoundMgr/showAmbientSpheres",
		new DataWatcher<bool>(showAmbientSpheres, Watcher::WT_READ_WRITE));

	Watcher::rootWatcher().addChild("SoundMgr/showStatic3DSpheres",
		new DataWatcher<bool>(showStatic3DSpheres, Watcher::WT_READ_WRITE));

	SequenceWatcher<AmbientSoundVec>* ambientSeq =
		new SequenceWatcher<AmbientSoundVec>(ambientList_);
	ambientSeq->addChild("*", new BaseDereferenceWatcher(&AmbientSound::watcher()));
	ambientSeq->setLabelSubPath("tag");
	Watcher::rootWatcher().addChild("SoundMgr/ambients", ambientSeq);

	SequenceWatcher<Static3DSoundVec>* static3DSeq =
		new SequenceWatcher<Static3DSoundVec>(static3DList_);
	static3DSeq->addChild("*", new BaseDereferenceWatcher(&Static3DSound::watcher()));
	static3DSeq->setLabelSubPath("tag");
	Watcher::rootWatcher().addChild("SoundMgr/statics", static3DSeq);

	SequenceWatcher<FxSoundVec>* fxSeq =
		new SequenceWatcher<FxSoundVec>(fxList_);
	fxSeq->addChild("*", new BaseDereferenceWatcher(&FxSound::watcher()));
	fxSeq->setLabelSubPath("tag");
	Watcher::rootWatcher().addChild("SoundMgr/fx", fxSeq);

	SequenceWatcher<BaseSoundVec>* baseSeq =
		new SequenceWatcher<BaseSoundVec>(baseList_);
	baseSeq->addChild("*", new BaseDereferenceWatcher(&BaseSound::watcher()));
	baseSeq->setLabelSubPath("tag");
	Watcher::rootWatcher().addChild("SoundMgr/all", baseSeq);

	Watcher::rootWatcher().addChild("SoundMgr/memory (k)",
		new DataWatcher<uint32>(memoryUsedK_, Watcher::WT_READ_ONLY));

	Watcher::rootWatcher().addChild("SoundMgr/echoMix",
		new DataWatcher<float>(echoMix_, Watcher::WT_READ_WRITE));
	Watcher::rootWatcher().addChild("SoundMgr/echoFeedback",
		new DataWatcher<float>(echoFeedback_, Watcher::WT_READ_WRITE));
	Watcher::rootWatcher().addChild("SoundMgr/echoDelay",
		new DataWatcher<float>(echoDelay_, Watcher::WT_READ_WRITE));
}


/**
 * Initialise the sound manager
 * Note: only the first argument, mainWindowHandle needs to be specified, all the rest can
 * be left out, or passed through as zero to get a default value.
 *
 * @param mainWindowHandle		This handle is needed for the DX8 audio initialisation only
 *								(so the sound can be disabled when window loses focus etc)
 * @param performanceChannels	maximum number of channels at any one time
 * @param rolloffFactor			rolloff factor for 3D sounds.  A setting 1 is realworld,
 *								2 is twice as steep as realworld
 * @param dopplerFactor			doppler factor for 3D sounds. Like rolloff 1 is realworld
 *								and 2 is twice as exaggerated
 * @param maxFxPaths			maximum number of fx sounds that can play simultaneously
 * @param maxAmbientPaths		maximum number of ambient sounds that can play simultaneously
 * @param maxStatic3DPaths		maximum number of static sounds that can play simultaneously
 * @param maxSimplePaths		maximum number of simple sounds that can play simultaneously
 *
 * @return True if initialisation succeeds, else false
 *
 * TODO: return a Status value
 */
bool SoundMgr::init(
	void* mainWindowHandle,
	int performanceChannels,
	float rolloffFactor,
	float dopplerFactor,
	int maxFxPaths,
	int maxAmbientPaths,
	int maxStatic3DPaths,
	int maxSimplePaths)
{
	TRACE_MSG("SoundMgr::init: doppler:%f\n", dopplerFactor);

	MF_ASSERT(!inited_);


	if (!rolloffFactor)
		rolloffFactor = 2;
	if (!performanceChannels)
		performanceChannels = 32;
	if (!dopplerFactor)
		dopplerFactor = 1;

	if (maxFxPaths)			MAX_EFX_PATHS =		maxFxPaths;
	if (maxAmbientPaths)	MAX_AMBIENT_PATHS =	maxAmbientPaths;
	if (maxStatic3DPaths)	MAX_STATIC3D_PATHS= maxStatic3DPaths;
	if (maxSimplePaths)		MAX_SIMPLE_PATHS =	maxSimplePaths;
	CoInitialize(NULL);

	if (CoCreateInstance(CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC,
		IID_IDirectMusicLoader8, (void**)&pLoader_) != S_OK
	){
		CRITICAL_MSG("Cannot get interface to IID_IDirectMusicLoader8 (make sure DX8 is installed)\n");
		return false;
	}

	if (CoCreateInstance(CLSID_DirectMusicPerformance, NULL, CLSCTX_INPROC,
		IID_IDirectMusicPerformance8, (void**)&pPerformance_) != S_OK
	){
		CRITICAL_MSG("Cannot get interface to IID_IDirectMusicPerformance8 "
					"(make sure DX8 is installed)\n");
		return false;
	}

	DMUS_AUDIOPARAMS audioParams;
	audioParams.dwSize = sizeof(audioParams);
	audioParams.fInitNow = true;
	audioParams.dwValidData =	DMUS_AUDIOPARAMS_FEATURES |
								DMUS_AUDIOPARAMS_VOICES |
								DMUS_AUDIOPARAMS_SAMPLERATE;
	audioParams.dwFeatures =	DMUS_AUDIOF_3D |
								DMUS_AUDIOF_BUFFERS |
								DMUS_AUDIOF_DMOS;
	audioParams.dwVoices = performanceChannels;
	audioParams.dwSampleRate = 22050;

	if (pPerformance_->InitAudio(
		NULL,								// IDirectMusic interface not needed
		NULL,								// IDirectSound interface not needed
		static_cast<HWND>(mainWindowHandle),
		DMUS_APATH_SHARED_STEREOPLUSREVERB,	// Default audiopath type
		performanceChannels,				// Number of performance channels
		DMUS_AUDIOF_ALL,					// Synthesizer features (ignored if audioParams used)
		&audioParams						// Audio parameters (NULL=default)
    ) != S_OK)
	{
		// TODO: do we need to fallback to a simpler request?
		CRITICAL_MSG("Cannot InitAudio\n");
		return false;
	}

	if (pPerformance_->CreateStandardAudioPath(
			DMUS_APATH_DYNAMIC_3D,
			1,						// number of performance channels.
			TRUE,					// activate now.
			&pDefAudioPath_			// pointer that receives audiopath.
	) != S_OK) {
		ERROR_MSG("SoundMgr::init: CreateStandardAudioPath() failed\n");
		return false;
	}

	if (pDefAudioPath_->GetObjectInPath(
		0,
		DMUS_PATH_PRIMARY_BUFFER,
        0,
		GUID_All_Objects,
		0,
		IID_IDirectSound3DListener,
        (LPVOID*)&pListener_
	) != S_OK) {
		ERROR_MSG("SoundMgr::init: GetObjectInPath(Listener) failed\n");
		return false;
	}

	// rolloffFactor of 1 is realworld, 2 is twice as steep as realworld etc
	pListener_->SetRolloffFactor(rolloffFactor, DS3D_DEFERRED);
	pListener_->SetDopplerFactor(dopplerFactor, DS3D_DEFERRED);
	pListener_->CommitDeferredSettings();

	// set the global volume to maximum & start mixer going continuously
	{
		IDirectSoundBuffer8 *pDSB;
		if (pDefAudioPath_->GetObjectInPath(
			0,							// Performance channel
			DMUS_PATH_PRIMARY_BUFFER,	// Stage in the path
			0,							// Index of buffer in chain
			GUID_All_Objects,			// Class of object
			0,							// Index of object in buffer: ignored
			IID_IDirectSoundBuffer,		// GUID of desired interface
			(LPVOID*)&pDSB				// Pointer that receives interface
		) != S_OK) {
			ERROR_MSG("SoundMgr::init: GetObjectInPath(DSB) failed\n");
			return false;
		}

		pDSB->SetVolume(DSBVOLUME_MAX);

		// From DX8 Help:
		//
		// When there are no sounds playing, DirectSound stops the mixer engine and halts DMA
		// activity. If your application has frequent short intervals of silence, the overhead
		// of starting and stopping the mixer each time a sound is played may be worse than
		// the DMA overhead if you kept the mixer active. Also, some sound hardware or drivers
		// may produce unwanted audible artifacts from frequent starting and stopping of
		// playback.
		//
		// If your application is playing audio almost continuously with only short breaks of
		// silence, you can force the mixer engine to remain active by calling the
		// IDirectSoundBuffer8::Play method for the primary buffer.
		//
		// To resume the default behavior of stopping the mixer engine when there are no
		// sounds playing, call the IDirectSoundBuffer8::Stop method for the primary buffer.

		pDSB->Play(0, 0, DSBPLAY_LOOPING);

		pDSB->Release();
	}


	{
		long vol = 0;
		if (pPerformance_->SetGlobalParam(GUID_PerfMasterVolume, (void *)&vol,
			sizeof(vol)) != S_OK)
		{
			ERROR_MSG("SoundMgr::init: SetGlobalParam() failed\n");
			return false;
		}
	}

//	HACK_MSG("SoundMgr::SoundMgr: success\n");
	inited_ = true;
	return true;
}


/** Destructor
 */
SoundMgr::~SoundMgr()
{
	TRACE_MSG("SoundMgr::~SoundMgr:\n");

	if (inited_) {
		pPerformance_->Stop(
			NULL,	// Stop all segments
			NULL,	// Stop all segment states
			0,		// Do it immediately
			0);		// Flags

		pPerformance_->CloseDown();

		pLoader_->Release();
		pPerformance_->Release();
		CoUninitialize();

		inited_ = false;
	}
}


/** Load a base sound
 */
BaseSound* SoundMgr::load(const char* filename, float defaultAtten)
{
	//TRACE_MSG("SoundMgr::load: %s\n", filename);
	MF_ASSERT(filename);
	MF_ASSERT(defaultAtten <= 0);

	if (!inited_)
		return NULL;

	BaseSound* newSound = new BaseSound(filename, defaultAtten, 1);
	if (!newSound)
		return NULL;

	if (!newSound->load(filename)) {
		delete newSound;
		return NULL;
	}

	return newSound;
}


/**
 *	Load an ambient sound.  An ambient sound has a location in 3D space, but it
 *	is not a proper 3D sound.  The location is only to set the attenuation of the sound
 *	not its exact placement
 *
 *	@param filename	What file to load (*.wav in many internal formats as supported by ACM)
 *	@param pos		Where the listener should be placed in 3D space
 *	@param minDist	The min distance to hear the sound
 *	@param maxDist	The max distance to hear the sound
 *	@param defaultAtten	Default attenuation
 *	@param outsideAtten	Attenuation to use when outside
 *	@param insideAtten	Attenuation to use when inside
 *	@param centreHour	Central time of day when sound is enabled
 *	@param minHour		Earliest time of day when sound is enabled
 *	@param maxHour		Latest time of day when sound is enabled
 *	@param loop			Whether or not the sound loops
 *	@param loopDelay	Delay between loops in seconds
 *	@param loopDelayVariance	Maximum amount by which loop delay can vary
 *
 *	@return			A new ambient sound (need to call play() to hear it)
 */
AmbientSound* SoundMgr::loadAmbient(
	const char* filename, const Vector3& pos,
	float  minDist, float  maxDist,
	float defaultAtten,
	float outsideAtten,
	float insideAtten,
	float centreHour,
	float minHour, float maxHour,
	bool  loop, float loopDelay, float loopDelayVariance)
{
	TRACE_MSG("SoundMgr::loadAmbient: %s\n", filename);
	MF_ASSERT(filename);
	MF_ASSERT(defaultAtten <= 0);
	MF_ASSERT(loopDelay >= 0);
	MF_ASSERT(loopDelayVariance >= 0);
	MF_ASSERT(minDist >= 0);
	MF_ASSERT(maxDist >= minDist);

	if (!inited_)
		return NULL;

	AmbientSound* newSound = new AmbientSound(filename, pos, minDist, maxDist,
					defaultAtten, outsideAtten, insideAtten,
					centreHour, minHour, maxHour, loop, loopDelay, loopDelayVariance);
	if (!newSound)
		return NULL;

	if (!newSound->load(filename)) {
		delete newSound;
		return NULL;
	}

	return newSound;
}


/**
 *	Load a static 3D sound.  These sounds are assumed to be mono.  They are played back
 *	in stereo to mimic 3D spatial positioning.
 */
Static3DSound* SoundMgr::loadStatic3D(
	const char*		filename,
	const Vector3&	pos,
	float			minDist,
	float			maxDist,
	float			defaultAtten,
	float			outsideAtten,
	float			insideAtten,
	float			centreHour,
	float			minHour,
	float			maxHour,
	bool			loop,
	float			loopDelay,
	float			loopDelayVariance)
{
	TRACE_MSG("SoundMgr::loadStatic3D: %s\n", filename);
	MF_ASSERT(filename);
	MF_ASSERT(defaultAtten <= 0);
	MF_ASSERT(loopDelay >= 0);
	MF_ASSERT(loopDelayVariance >= 0);
	MF_ASSERT(minDist >= 0);
	MF_ASSERT(maxDist >= minDist);

	if (!inited_)
		return NULL;

	Static3DSound* newSound = new Static3DSound(filename, pos, minDist, maxDist,
										defaultAtten, outsideAtten, insideAtten,
										centreHour, minHour, maxHour,
										loop, loopDelay, loopDelayVariance);
	if (!newSound->load(filename)) {
		delete newSound;
		return NULL;
	}

	if (!newSound->audioPathPool_) {
		newSound->audioPathPool_ = new AudioPathPool(
			MAX_STATIC3D_PATHS, DMUS_APATH_DYNAMIC_3D, soundMgr().pPerformance_);
	}
	newSound->audioPathPoolCopy_ = newSound->audioPathPool_;

	// make it loop forever
	if (newSound->loop_ && newSound->loopDelay_ == 0)
		newSound->pSegment_->SetRepeats(DMUS_SEG_REPEAT_INFINITE);

	return newSound;
}


/** Load an fx sound
 */
FxSound* SoundMgr::loadFx(
	const char*	filename,
	const char*	tag,
	float		minDist,
	float		maxDist,
	float		defaultAtten,
	uint		maxInstances,
	bool		loop)
{
	TRACE_MSG("SoundMgr::loadFx: %s\n", filename);
	MF_ASSERT(filename);
	MF_ASSERT(defaultAtten <= 0);
	MF_ASSERT(minDist >= 0);
	MF_ASSERT(maxDist >= minDist);


	if (!inited_)
		return NULL;

	FxSound* newSound = new FxSound(tag, minDist, maxDist, defaultAtten, maxInstances, loop);
	if (!newSound->load(filename)) {
		delete newSound;
		return NULL;
	}

	if (!newSound->audioPathPool_) {
		newSound->audioPathPool_ = new AudioPathPool(
			MAX_EFX_PATHS, DMUS_APATH_DYNAMIC_3D, soundMgr().pPerformance_);
	}
	newSound->audioPathPoolCopy_ = newSound->audioPathPool_;

	// make it loop forever
	if (newSound->loop_ /* && newSound->loopDelay_ == 0 */)
		newSound->pSegment_->SetRepeats(DMUS_SEG_REPEAT_INFINITE);


	return newSound;
}


/** Load a simple sound
 */
SimpleSound* SoundMgr::loadSimple(
	const char*	filename,
	float		defaultAtten,
	uint		maxInstances,
	bool		loop)
{
	TRACE_MSG("SoundMgr::loadSimple: %s\n", filename);
	MF_ASSERT(filename);
	MF_ASSERT(defaultAtten <= 0);

	if (!inited_)
		return NULL;

	SimpleSound* newSound = new SimpleSound(filename, defaultAtten, maxInstances, loop);
	if (!newSound->load(filename)) {
		delete newSound;
		return NULL;
	}

	if (!newSound->audioPathPool_) {
		newSound->audioPathPool_ = new AudioPathPool(
			MAX_SIMPLE_PATHS, DMUS_APATH_DYNAMIC_MONO, soundMgr().pPerformance_);
	}
	newSound->audioPathPoolCopy_ = newSound->audioPathPool_;

	return newSound;
}




/**
 *	This method tells the sound mgr where the listener (normally the camera) is.
 */
void SoundMgr::setListenerPos(	const Matrix& listenerToWorldMatrix,
								/*bool cameraInShell,*/
								const Vector3& playerPos,
								bool playerInShell,
								float minDim,
								float hours,
								float curTime)
{
	//TRACE_MSG("SoundMgr::setListenerPos: (%1.2f %1.2f %1.2f) minDim=%1.2f\n",
	//		listenerToWorldMatrix.applyToOrigin().x,
	//		listenerToWorldMatrix.applyToOrigin().y,
	//		listenerToWorldMatrix.applyToOrigin().z, minDim);

	if (!inited_)
		return;

	currentTime_ = curTime;
	timeQueue_.process(static_cast<TimeQueue::TimeStamp>(curTime) * TIMER_SCALE);

	pListener_->SetPosition(
			listenerToWorldMatrix.applyToOrigin().x,
			listenerToWorldMatrix.applyToOrigin().y,
			listenerToWorldMatrix.applyToOrigin().z,
			DS3D_DEFERRED);
	pListener_->SetOrientation(
			listenerToWorldMatrix.applyToUnitAxisVector(Z_AXIS).x,
			listenerToWorldMatrix.applyToUnitAxisVector(Z_AXIS).y,
			listenerToWorldMatrix.applyToUnitAxisVector(Z_AXIS).z,
			listenerToWorldMatrix.applyToUnitAxisVector(Y_AXIS).x,
			listenerToWorldMatrix.applyToUnitAxisVector(Y_AXIS).y,
			listenerToWorldMatrix.applyToUnitAxisVector(Y_AXIS).z,
			DS3D_DEFERRED);
	pListener_->CommitDeferredSettings();

	// adjust the active set of 3D sounds to ones in range
	restart3DSounds(listenerToWorldMatrix.applyToOrigin(), hours);

	setListenerPosAmbient(playerPos, hours, playerInShell);

	listenerPos_ = listenerToWorldMatrix.applyToOrigin();
	inShell_ = playerInShell;
	shellSize_ = minDim;
}



/**
 *	This method draws spheres to represent ambient sounds placed in the world.
 *	Orange is maxDistance and green is minDistance.
 */
void SoundMgr::drawDebugStuff()
{
#ifndef _RELEASE
	static bool needToRedrawAmbient = false;
	static bool needToRedrawStatic3D = false;
	static FontPtr s_font = NULL;
	static vector<TextGUIComponent*> ambLabels, statLabels;

	if (!showAmbientSpheres && !needToRedrawAmbient &&
		!showStatic3DSpheres && !needToRedrawStatic3D)
	{
		return;
	}

	AmbientSoundVec::iterator iAmbSnd;
	Static3DSoundVec::iterator iStatSnd;


	if (!s_font) {
		s_font = FontManager::instance().get( "default_small.font" );
		s_font->colour(0xffffffff);
		//s_font->setFontScale(FONT_SCALE_FIXED);
		//s_font->setScale((Moo::rc().screenWidth() / 100.f) / s_font->fontWidth());

		for (iAmbSnd = ambientList_.begin();  iAmbSnd != ambientList_.end();  iAmbSnd++)
		{
			vector<AmbientSound::PositionData>::iterator pd;
			int idx = 0;
			for (pd=(*iAmbSnd)->positionData_.begin();
						pd != (*iAmbSnd)->positionData_.end();  pd++, idx++)
			{
				TextGUIComponent* txt = new TextGUIComponent();
				ambLabels.push_back(txt);
				if ((*iAmbSnd)->positionData_.size() > 1) {
					txt->slimLabel((*iAmbSnd)->tag_.c_str());
				} else {
					txt->slimLabel((*iAmbSnd)->tag_.c_str());
				}
				txt->anchor(SimpleGUIComponent::ANCHOR_H_CENTER,
							SimpleGUIComponent::ANCHOR_V_CENTER);
				txt->colour(0x80ffffff);
				txt->visible(false);
				SimpleGUI::instance().addSimpleComponent(*txt);
			}
		}

		for (iStatSnd = static3DList_.begin();  iStatSnd != static3DList_.end();  iStatSnd++)
		{
			TextGUIComponent* txt = new TextGUIComponent();
			statLabels.push_back(txt);
			txt->slimLabel((*iStatSnd)->tag_.c_str());
			txt->anchor(SimpleGUIComponent::ANCHOR_H_CENTER,
						SimpleGUIComponent::ANCHOR_V_CENTER);
			txt->colour(0x808080ff);
			SimpleGUI::instance().addSimpleComponent(*txt);
		}
	}


	vector<TextGUIComponent*>::iterator ilabel;

	if (showAmbientSpheres || needToRedrawAmbient) {

		ilabel = ambLabels.begin();
		for (iAmbSnd=ambientList_.begin();  iAmbSnd != ambientList_.end();  iAmbSnd++)
		{
			vector<AmbientSound::PositionData>::iterator pd;
			int idx = 0;
			for (pd=(*iAmbSnd)->positionData_.begin();
							pd != (*iAmbSnd)->positionData_.end();  pd++, idx++)
			{
				Geometrics::instance().drawSphere(pd->position, (float)pd->minDistance, 0x20FF8000);
				Geometrics::instance().drawSphere(pd->position, (float)pd->maxDistance, 0x20808000);

				Vector4 scrnPos;
				Moo::rc().viewProjection().applyPoint(scrnPos, Vector4(pd->position, 1));
				// test .w and ignore labels behind the screen
				if (scrnPos.w > 0.0f && showAmbientSpheres) {
					Vector3 clipPos( scrnPos.x / scrnPos.w, scrnPos.y / scrnPos.w, 1.f );
					(*ilabel)->position(clipPos);
					(*ilabel)->visible(true);
				} else {
					(*ilabel)->visible(false);
				}

				ilabel++;
			}
		}

		needToRedrawAmbient = showAmbientSpheres;
									// need one extra redraw after turning off
									// so that the labels are removed
	}


	if (showStatic3DSpheres || needToRedrawStatic3D) {

		ilabel = statLabels.begin();
		for (iStatSnd=static3DList_.begin(); iStatSnd != static3DList_.end();  iStatSnd++)
		{
			Static3DSound* snd = *iStatSnd;

			Geometrics::instance().drawSphere(snd->position_[0], (float)snd->minDistance_, 0x2000FF00);
			Geometrics::instance().drawSphere(snd->position_[0], (float)snd->maxDistance_, 0x2040FF00);

			Vector4 scrnPos;
			Moo::rc().viewProjection().applyPoint(scrnPos, Vector4(snd->position_[0], 1));
			// test .w and ignore labels behind the screen
			if (scrnPos.w > 0.0f && showStatic3DSpheres) {
				Vector3 clipPos( scrnPos.x / scrnPos.w, scrnPos.y / scrnPos.w, 1.f );
				(*ilabel)->position(clipPos);
				(*ilabel)->visible(true);
			} else {
				(*ilabel)->visible(false);
			}

			ilabel++;
		}

		needToRedrawStatic3D = showStatic3DSpheres;
								// need one extra redraw after turning off
								// so that the labels are removed
	}


#if 0

	for (isnd = ambientList_.begin();  isnd != ambientList_.end();  isnd++)
	{
		vector<AmbientSound::PositionData>::iterator pd;
		int idx = 0;
		for (pd=(*isnd)->positionData_.begin();  pd != (*isnd)->positionData_.end();  pd++, idx++)
		{
			Geometrics::instance().drawSphere(pd->position, pd->minDistance, 0x1000FF00);
			Geometrics::instance().drawSphere(pd->position, pd->maxDistance, 0x20FF8000);

			Moo::Vector3 scrnPos = Moo::rc().viewProjection().applyPoint(pd->position);
			if (scrnPos.x > -1.f && scrnPos.x < 1.f && scrnPos.y > -1.f && scrnPos.y < 1.f)
			{
				if (FontManager::instance().begin( font ))
				{
					int x = (scrnPos.x + 1) * (CONSOLE_WIDTH/2);
					int y = (-scrnPos.y + 1) * (CONSOLE_HEIGHT/2);
					if ((*isnd)->positionData_.size() > 1) {
						char tag[128];
						sprintf(tag, "%s(%d)", (*isnd)->tag_.c_str(), idx);
						s_font->drawString(string(tag), x, y);
					} else {
						s_font->drawString((*isnd)->tag_, x, y);
					}
					FontManager::instance().end();
				}
			}
		}
	}
#endif
#endif
}


static Vector3 updateAmbient_lastPos = Vector3::zero();
static int updateAmbient_lastMin = -1000000;

static Vector3 update3d_lastPos = Vector3::zero();
static int update3d_lastMin = -1000000;

static const float AMBIENT_MIN_DIST_MOVED_SQR = 0.5f * 0.5f;

// smaller than ambient since some sounds have a small falloff
static const float STATIC3D_MIN_DIST_MOVED_SQR = 0.25f * 0.25f;


/**
 *	This method instructs the sound manager to update all of its sounds
 *	as soon as possible, because they have changed their internal values.
 */
void SoundMgr::updateASAP()
{
	updateAmbient_lastMin = -1000000;
	update3d_lastMin = -1000000;
}

/**
 *	This method updates all the ambient sound attenuations when the listener
 *	position moves.
 */
void SoundMgr::setListenerPosAmbient(const Vector3& position, float hours, bool inShell)
{
	//TRACE_MSG("SoundMgr::setListenerPosAmbient: (%1.2f %1.2f %1.2f) hrs=%1.2f\n",
	//	position.x(), position.y(), position.z(), hours);

	// do our simple 2D/3D ambient sounds

	// if the camera hasn't moved very much and the time hasn't changed (1min) skip the
	// calculations [since these sounds cover a wide area we can wait for a fairly
	// large camera movement or time change]

	if ((int)(hours * 60.0) == updateAmbient_lastMin) {
		Vector3 distv = position - updateAmbient_lastPos;
		if (distv.lengthSquared() < AMBIENT_MIN_DIST_MOVED_SQR)
			return;
	}
	updateAmbient_lastPos = position;
	updateAmbient_lastMin = (int)(hours * 60.0);


	// recalculate the new volume on all ambient sounds

	AmbientSoundVec::iterator isnd;
	for (isnd = ambientList_.begin();  isnd != ambientList_.end();  isnd++) {
		AmbientSound* snd = *isnd;

		//TRACE_MSG("SoundMgr::setListenerPosAmbient: %s (%1.2f %1.2f %1.2f)\n",
		//	snd->tag_.c_str(), position.x(), position.y(), position.z());

		// choose the closest position to the listener position
		// (for multiple point sound sources)
		float minAttenuation = -MAX_ATTENUATION;	// keep track of loudest point
		vector<AmbientSound::PositionData>::iterator pd;
		for (pd=snd->positionData_.begin(); pd != snd->positionData_.end(); pd++) {

			float attenuation;
			Vector3 distv = position - (*pd).position;
			// the length vector is stretched vertically since
			// most sound emitters are located on a 2D plane, and they can
			// interfere with each other when placed vertically
			// IE: the stretching makes the sound bubble half as high

			distv.y = distv.y * 2;			// stretch things vertically

			//TRACE_MSG("SoundMgr::setListenerPosAmbient: %s distv.length=%1.2f ",
			//	snd->tag_.c_str(), distv.length());

			// do caculations for min/max distance so sound is loudest at minDistance
			// and quietest at minDistance

			if (distv.lengthSquared() > (*pd).maxDistanceSquared) {	// out of audible range?
				attenuation = -(MAX_ATTENUATION*2);
			}
			else {
				float dist = distv.length() - (*pd).minDistance;
				if (dist < 0)
					attenuation = 0;
				else {
					attenuation = -(dist / ((*pd).maxDistance - (*pd).minDistance))
																* MAX_ATTENUATION;
				}
			}

			//TRACE_MSG("atten=%1.1f\n", attenuation);
			// keep the lowest attenuation (loudest sound)
			if (attenuation > minAttenuation)
				minAttenuation = attenuation;
		}


		// now do calculations for min/max hrs
		// (attenuation based on time -- night/day sounds)

		if (snd->minHour_ || snd->maxHour_)	{

			float dist = (float)fabs(snd->centreHour_ - hours);
			if (dist > 12) dist = 12 - (dist-12);
			dist -= snd->minHour_;
			if (dist < 0) dist = 0;
			//TRACE_MSG("SoundMgr::setListenerPosAmbient: dist hrs=%1.2f\n", dist);
			minAttenuation += -(dist / (snd->maxHour_ - snd->minHour_)) * MAX_ATTENUATION;
			// TODO: put a log in here to make it linear
		}

		// now do extra attenuation as needed for in/outside shells
		minAttenuation += (inShell) ? snd->insideAtten_ : snd->outsideAtten_;

		// Now apply the atten (restart sound as necessary)

		if (minAttenuation > 0)
			minAttenuation = 0;

		if (minAttenuation <= -(MAX_ATTENUATION-1)) {	// out of audible range?
			if (snd->isPlaying_) {
				MF_ASSERT(snd->audioPaths_.size() == 1);
				snd->stop(0);	// ambient sounds can only have one instance (0)
			}
		} else {
			snd->attenuation(minAttenuation);
			// restart in case was out of range last time
			if (!snd->isPlaying_ && snd->tid_ == 0) {
				snd->priority_ = -minAttenuation;
				snd->play();
			}
		}
	}

	//TRACE_MSG("SoundMgr::setListenerPosAmbient: pathsInUse:%d\n",
	//	AmbientSound::audioPathPool_->pathsInUse());
}




/**
 *	This method adjusts the active set of 3D sounds to the ones in range.
 *	Sounds that are out of range are stopped.  There is a limit the number of sounds that
 *	play concurrently.
 */
void SoundMgr::restart3DSounds(const Vector3& position, float hours)
{

	// if the camera hasn't moved very much and the time hasn't changed (1min) skip the
	// calculations [since these sounds cover a wide area we can wait for a fairly
	// large camera movement or time change]

	if ((int)(hours * 60.0) == update3d_lastMin) {
		Vector3 distv = position - update3d_lastPos;
		if (distv.lengthSquared() < STATIC3D_MIN_DIST_MOVED_SQR)
			return;
	}
	update3d_lastPos = position;
	update3d_lastMin = (int)(hours * 60.0);


	// recalculate the new volume on all sounds

	Static3DSoundVec::iterator isnd;
	for (isnd = static3DList_.begin();  isnd != static3DList_.end();  isnd++) {
		bool audible = false;
		Static3DSound* snd = *isnd;

		Vector3 distv = position - snd->position_[0];
		float distSquared = distv.lengthSquared();

		if (distSquared <= snd->maxDistanceSquared_ * 1.5f) {		// in audible range?

			// now do calculations for min/max hrs
			if (snd->minHour_ || snd->maxHour_)	{
				float dist = (float)fabs(snd->centreHour_ - hours);
				if (dist > 12) dist = 12 - (dist-12);
				if (dist < snd->maxHour_)
					audible = true;
			} else
				audible = true;
		}


		if (!audible) {
			if (snd->tid_) {
				soundMgr().timeQueue_.cancel(snd->tid_);
				snd->tid_ = 0;
			}

			if (snd->isPlaying_)
			{
				//MF_ASSERT(snd->audioPaths_.size() == 1); TODO: Uncomment this assert
				snd->stop(0);	// NB: this won't work for all sounds!
			}

			snd->hasTriggered_ = false;

		}
		else if (!snd->isPlaying_ && !snd->hasTriggered_) {
			snd->hasTriggered_ = true;
			snd->priority_ = distSquared;
			snd->play();
		}
	}

	//TRACE_MSG("SoundMgr::restart3DSounds: pathsInUse:%d\n",
	//	Static3DSound::audioPathPool_->pathsInUse());
}



/**
 *	Play a sound fx that matches the tag at a specified location with extra attenuation
 *
 *	@param tag			The label for the file (not the filename)
 *	@param delay		The delay time in seconds
 *	@param atten		The attenuation in db
 *	@param pos			The position to play the sound
 *	@return void
 */
void SoundMgr::playFxDelayedAtten(const char* tag, float delay, float atten, const Vector3& pos)
{
	//TRACE_MSG("SoundMgr::playFxDelayed: '%s'\n", tag);

	if (!inited_)
		return;

	g_watchSound.start();

	typedef FxMap::const_iterator Iter;
	std::pair<Iter, Iter> snds = fxMap_.equal_range(tag);

	Iter origFirst = snds.first;
	int count = 0;
	for (; snds.first != snds.second;  snds.first++)
		count++;

	if (count) {
		// choose a sound to play randomly
		int n = rand() % count;
		while (n--)
			origFirst++;

		FxSound* snd = origFirst->second;
		if (snd) {
			Vector3 distv = soundMgr().listenerPos_ - pos;
			// don't even try to play sounds out of range...
			// TODO: this is not so good for long sounds that may come in range during
			// their life so maybe I should check the length and only do this for short
			// sounds (or maybe the sounds can have an attribute "canDistCull")
			if (distv.lengthSquared() <= snd->maxDistanceSquared_) {
				snd->priority_ = distv.lengthSquared();	// TODO: this should be done per instance
				if (delay)
					snd->playDelayedAtten(delay, atten, pos);
				else
					snd->play(atten, pos);
			}
		}
	} else {
		ERROR_MSG("SoundMgr::playFx: no tag matching '%s'\n", tag);
	}

	g_watchSound.stop();
}



/**
 *	Play a sound fx that matches the tag at a specified location
 *
 *	@param tag		The label for the file (not the filename)
 *	@param pos		The position to play the sound
 *	@param delay	The delay time in seconds
 *	@return void
 */
void SoundMgr::playFxDelayed(const char* tag, float delay, const Vector3& pos)
{
	playFxDelayedAtten(tag, delay, 0, pos);
}


/**
 *	Play a sound fx that matches the tag at a specified location.
 *
 *	@param tag		The label for the file (not the filename)
 *	@param pos		The position to play the sound
 *	@return void
 */
void SoundMgr::playFx(const char* tag, const Vector3& pos)
{
	playFxDelayed(tag, /*delay*/0.0f, pos);
}


/**
 *	Play a sound fx that matches the tag
 *
 *	@param tag	The label for the file (not the filename)
 *	@return		void
 */
void SoundMgr::playFx(const char* tag)
{
	playFxDelayed(tag, 0, listenerPos_);
}


/**
 *	This method plays a Simple sound.
 *
 *	Note: the name to pass is a shortened form of the filename loaded, with directory
 *	names and file extensions removed.
 *
 *	@param tag	The shortened name for the file
 *	@return		void
 *
 *	Code Example:
 *	@code
 *	::soundMgr().loadSimple("ui/boop.wav", -10);
 *	::soundMgr().playSimple("boop");
 *	@endcode
 */
void SoundMgr::playSimple(const char* tag)
{
	TRACE_MSG("SoundMgr::playSimple: '%s'\n", tag);

	if (!inited_)
		return;

	g_watchSound.start();

	BaseSound* snd = soundMap_[tag];
	if (snd) {
		snd->play();
	}
	else {
		ERROR_MSG("SoundMgr::playSimple: no tag matching '%s'\n", tag);
	}

	g_watchSound.stop();
}



/**
 *	Play a singleshot ambient sound.
 *
 *	@param tag		The name for the file
 *	@param worldPos	The position of the sound in world coordinates
 *
 *	@return void
 */
void SoundMgr::playAmbient(const char* tag, const Vector3& worldPos)
{
	TRACE_MSG("SoundMgr::playAmbient: '%s'\n", tag);

	if (!inited_)
		return;

	g_watchSound.start();

	BaseSound* snd = soundMap_[tag];
	if (snd) {
		snd->play();
	}
	else {
		ERROR_MSG("SoundMgr::playAmbient: no tag matching '%s'\n", tag);
	}

	g_watchSound.stop();
}


/**
 *	This method returns a pointer to any sound located via a tag
 *
 *	Note: the name to pass is a shortened form of the filename loaded, with directory
 *	names and file extensions removed.
 *
 *	@param tag	The shortened name for the file
 *	@return		Pointer to the sound
 *
 *	Code Example:
 *	@code
 *	::soundMgr().loadSimple("ui/boop.wav", -10);
 *	BaseSound& snd = ::soundMgr().findSound("boop");
 *	snd->play();
 *	@endcode
 */
BaseSound* SoundMgr::findSound(const char* tag)
{
	//TRACE_MSG("SoundMgr::findSound: '%s'\n", tag);

	BaseSound* snd = NULL;

	if (!inited_)
		return snd;

	g_watchSound.start();

	if (!(snd = soundMap_[tag])) {
		ERROR_MSG("SoundMgr::findSound: no tag matching '%s'\n", tag);
	}

	g_watchSound.stop();

	return snd;
}


/**
 *	This method returns a pointer to any sound located via a tag
 *
 *	Note: the name to pass is as specified in the XML file
 *
 *	@param tag	The name of the sound
 *	@return		Pointer to the sound
 */
FxSound* SoundMgr::findFxSound(const char* tag)
{
	//TRACE_MSG("SoundMgr::findSound: '%s'\n", tag);

	FxSound* snd = NULL;

	if (!inited_)
		return snd;

	g_watchSound.start();

	typedef FxMap::const_iterator Iter;
	std::pair<Iter, Iter> snds = fxMap_.equal_range(tag);

	Iter first = snds.first;
	snd = first->second;

	if (!snd) {
		ERROR_MSG("SoundMgr::findFxSound: no tag matching '%s'\n", tag);
	}

	g_watchSound.stop();

	return snd;
}



// -----------------------------------------------------------------------------
//                           B a s e S o u n d
// -----------------------------------------------------------------------------

/** BaseSound constructor: Internal use only
 */
BaseSound::BaseSound(string tag, float defaultAtten, uint maxInstances) :
	tag_(tag),
	attenuation_(0),
	defaultAtten_(defaultAtten),
	isPlaying_(false),
	pSegment_(NULL),
	audioPaths_(1),
	dxAttenuation_(0),
	maxInstances_(maxInstances)
{
	TRACE_MSG("BaseSound::BaseSound:\n");
	MF_ASSERT(defaultAtten <= 0);

	audioPaths_.reserve(maxInstances_);	// more efficient adding
	audioPaths_[0] = NULL;
	soundMgr().baseList_.push_back(this);
	attenuation(0);	// reset volume
}


/** BaseSound destructor
 */
BaseSound::~BaseSound()
{
	TRACE_MSG("BaseSound::~BaseSound:\n");

	stopAll();		// this will also release the audiopath and dsb's

	// release its memory as loaded via dmusic
	{
		DMUS_OBJECTDESC objDesc;
		ZeroMemory(&objDesc, sizeof(objDesc));
		objDesc.dwSize = sizeof(objDesc);
		objDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;
		objDesc.guidClass = CLSID_DirectMusicSegment;
		objDesc.pbMemData = NULL;
		objDesc.llMemLength = 0;
		soundMgr().pLoader_->SetObject(&objDesc);
	}

	if (pSegment_) {
		pSegment_->Unload(soundMgr().pPerformance_);
		pSegment_->Release();
	}

	// delete ptr from list
	SoundMgr::BaseSoundVec* list = &soundMgr().baseList_;
	SoundMgr::BaseSoundVec::iterator isnd = std::find(list->begin(), list->end(), this);
	MF_ASSERT(isnd != list->end());
	if (isnd != list->end())
		list->erase(isnd);


	// release any paths in use
	vector<AudioPath*>::iterator i;
	for (i=audioPaths_.begin(); i != audioPaths_.end(); i++) {
		if (*i)
			(*i)->releasePath();
	}
}



/** Load a sound.
 */
bool BaseSound::load(const char* filename)
{
	TRACE_MSG("BaseSound::load: %s\n", filename);

	DataSectionPtr dsp = BWResource::openSection(string("sounds/") + string(filename));
	if (!dsp) {
		ERROR_MSG("BaseSound::load: cannot find binary res '%s'\n", filename);
		return false;
	}

	sndBits_ = dsp->asBinary();

	DMUS_OBJECTDESC objDesc;
	ZeroMemory(&objDesc, sizeof(objDesc));
	objDesc.dwSize = sizeof(objDesc);
	objDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_MEMORY;
	objDesc.guidClass = CLSID_DirectMusicSegment; //CLSID_DirectSoundWave;
	objDesc.pbMemData = (BYTE *)sndBits_->data();
	objDesc.llMemLength = sndBits_->len();

	soundMgr().memoryUsed_ += sndBits_->len();
	soundMgr().memoryUsedK_ = soundMgr().memoryUsed_ / 1024;

	//TRACE_MSG("BaseSound::load: len=%d\n", objDesc.llMemLength);

	HRESULT hr;
	if (FAILED(hr=soundMgr().pLoader_->GetObject(
			&objDesc, IID_IDirectMusicSegment8, (LPVOID*)&pSegment_))
	) {
		ERROR_MSG("BaseSound::load: GetObject() failed (%lx)\n", hr);
		return false;
	}

//	WCHAR wFilename[MAX_PATH];
//	MultiByteToWideChar(CP_ACP, 0, filename, -1, wFilename, MAX_PATH);
//
//	if (soundMgr().pLoader_->LoadObjectFromFile(
//			CLSID_DirectMusicSegment,   // Class identifier
//			IID_IDirectMusicSegment8,   // ID of desired interface
//			wFilename,
//			(LPVOID*)&pSegment_			// Pointer that receives interface
//		) != S_OK
//	) {
//		DEBUG_MSG("SoundMgr::load: LoadObjectFromFile() failed\n");
//		return false;
//  }


	if (FAILED(hr=pSegment_->Download(soundMgr().pPerformance_))) {
		ERROR_MSG("SoundMgr::load: Download() failed (hr=0x%lx)\n", hr);
		//return false;
	}

	//TRACE_MSG("BaseSound::load: ok\n");
	return true;
}


/** store an access ptr to the dx sound buffer so we can change settings later
 */
void BaseSound::getDXObjects(AudioPath& audioPath) const
{
	if (audioPath.pDSPath_->GetObjectInPath(
		DMUS_PCHANNEL_ALL,			// Performance channel
		DMUS_PATH_BUFFER,			// Stage in the path
		0,							// Index of buffer in chain
		GUID_NULL,					// Class of object
		0,							// Index of object in buffer: ignored
		IID_IDirectSoundBuffer,		// GUID of desired interface
		(LPVOID*)&audioPath.pDSB_	// Pointer that receives interface
	) != S_OK) {
		ERROR_MSG("BaseSound::getDXObjects: GetObjectInPath(DSB) failed\n");
	}

	//TRACE_MSG("BaseSound::getDXObjects: dsb=%p\n", audioPath.pDSB_);
}


/**
 *	This method starts a sound playing.
 *
 *	@return The instance of sound started (for this sound class always zero)
 */
int BaseSound::play()
{
	TRACE_MSG("BaseSound::play: [%s] (dxAtten=%ld, pathsInUse=%d)\n",
		tag_.c_str(), dxAttenuation_, audioPathPoolCopy_->pathsInUse());
	MF_ASSERT(pSegment_);


	if (audioPaths_[0])		// if it was in use release it
		audioPaths_[0]->releasePath();

	audioPaths_[0] = audioPathPoolCopy_->allocPath(*this);

	// reset the volume to cached value
	audioPaths_[0]->pDSPath_->SetVolume(dxAttenuation_, /*duration*/0);
	audioPaths_[0]->priority_ = 0;

	soundMgr().pPerformance_->PlaySegmentEx(
		pSegment_,					// Segment to play
		NULL,						// not implemented
		NULL,						// For transitions
		DMUS_SEGF_DEFAULT|DMUS_SEGF_SECONDARY,		// Flags
		0,							// Start time; 0 is immediate
		&audioPaths_[0]->pDSSS_,	// Pointer that receives segment state
		NULL,						// Object to stop
		audioPaths_[0]->pDSPath_	// Audiopath to play sound on
	);

	isPlaying_ = true;

	return 0;
}


/**
 *	This method starts a sound playing at a particular volume.
 *	The volume is not saved with the sound.
 *
 *	@param attenuation	Volume (db attenuation) to play the sound at.  -10 is half volume.
 *	@return The instance of sound started (for this sound class always zero)
 */
int BaseSound::play(float attenuation)
{
	TRACE_MSG("BaseSound::play: [%s] (dxAtten=%ld)\n", tag_.c_str(), dxAttenuation_);
	MF_ASSERT(pSegment_);


	if (audioPaths_[0])		// if it was in use release it
		audioPaths_[0]->releasePath();
	audioPaths_[0] = audioPathPoolCopy_->allocPath(*this);


	long dxAttenuation = long((attenuation + defaultAtten_) * 100.0f);
	if (dxAttenuation < DSBVOLUME_MIN)
		dxAttenuation = DSBVOLUME_MIN;
	if (dxAttenuation > DSBVOLUME_MAX)
		dxAttenuation = DSBVOLUME_MAX;

	audioPaths_[0]->pDSPath_->SetVolume(dxAttenuation, /*duration*/0);
	audioPaths_[0]->priority_ = 0;

	soundMgr().pPerformance_->PlaySegmentEx(
		pSegment_,					// Segment to play
		NULL,						// not implemented
		NULL,						// For transitions
		DMUS_SEGF_DEFAULT|DMUS_SEGF_SECONDARY,		// Flags
		0,							// Start time; 0 is immediate
		&audioPaths_[0]->pDSSS_,	// Pointer that receives segment state
		NULL,						// Object to stop
		audioPaths_[0]->pDSPath_	// Audiopath to play sound on
	);

	isPlaying_ = true;

	return 0;
}


/**
 *	Set the attenuation of a sound (playing or stopped).
 *
 *	The attenuation as specified by defaultAtten() also added to get the current
 *	attenuation.
 *
 *	@param db the attenuation in decibels.  For example, -10db will halve the volume.
 *	@param instance the instance whose attenuation should be set
 */
void BaseSound::attenuation(const float db, uint instance)
{
	//TRACE_MSG("BaseSound::attenuation: [%s,%d] %0.1f\n", tag_.c_str(), instance, db);

	MF_ASSERT(db <= 0);
	MF_ASSERT(instance < audioPaths_.size());

	if (instance >= audioPaths_.size())
		instance = 0;

	attenuation_ = db + defaultAtten_;
	dxAttenuation_ = long(attenuation_ * 100.0f);
	if (dxAttenuation_ < DSBVOLUME_MIN)
		dxAttenuation_ = DSBVOLUME_MIN;
	if (dxAttenuation_ > DSBVOLUME_MAX)
		dxAttenuation_ = DSBVOLUME_MAX;

	if (isPlaying_ && audioPaths_[instance]) {
		MF_ASSERT(audioPaths_[instance]->pDSPath_);
		audioPaths_[instance]->pDSPath_->SetVolume(dxAttenuation_, /*duration*/0);
	}
}


/**
 *	Set the maximum volume a sound will ever get to.
 *
 *	@param db the attenuation in decibels.  For example, -10db will halve the volume.
 */
void BaseSound::defaultAttenuation(float db)
{
	//TRACE_MSG("BaseSound::defaultAttenuation: [%s] %0.1f\n", tag_.c_str(), db);
	MF_ASSERT(db <= 0);

	float oldAtten = attenuation_ - defaultAtten_;
	defaultAtten_ = db;
	attenuation(oldAtten);
}



/**
 *	Stop a particular instance of a sound that is currently playing.
 *
 *	NB: it is possible to call stop on a sound that isn't playing quite safely
 */
void BaseSound::stop(uint instance)
{
	TRACE_MSG("BaseSound::stop: [%s] instance=%d\n", tag_.c_str(), instance);
	MF_ASSERT(instance < audioPaths_.size());

	if (maxInstances_ > 1) {

		//TRACE_MSG("BaseSound::stop: [%s] DSPath=%p\n", tag_.c_str(), audioPaths_[instance]->pDSPath_);

		if (audioPaths_[instance])
			soundMgr().pPerformance_->StopEx(
				/*whatToStop*/audioPaths_[instance]->pDSPath_, /*when*/0, /*flags*/0);

	} else {
		soundMgr().pPerformance_->Stop(
			pSegment_,		// Stop this segment
			NULL,			// Stop all segment states
			0,				// Do it immediately
			0);				// Flags

	}

	if (audioPaths_[instance]) {
		audioPaths_[instance]->releasePath();	// not in use anymore, allow others to reuse
		audioPaths_[instance] = NULL;
	}

	isPlaying_ = false;
}

/**
 *	Stop all instances of the sound that are currently playing.
 *
 *	NB: it is possible to call stop on a sound that isn't playing (although routine
 *	will assert at debug time)
 */
void BaseSound::stopAll()
{
	TRACE_MSG("BaseSound::stopAll [%s]\n", tag_.c_str());
	soundMgr().pPerformance_->Stop(
		pSegment_,		// Stop this segment
		NULL,			// Stop all segment states
		0,				// Do it immediately
		0);				// Flags

	for (uint i=0; i<audioPaths_.size(); i++) {
		if (audioPaths_[i]) {
			audioPaths_[i]->releasePath();	// not in use anymore, allow others to reuse
			audioPaths_[i] = NULL;
		}
	}

	isPlaying_ = false;
}


/** Return true if the sound is currently being played
 */
bool BaseSound::isPlaying() const
{
	if (!isPlaying_)		// optimisation: we know if definately not playing
		return false;

	bool isPlaying =
		soundMgr().pPerformance_->IsPlaying(pSegment_, audioPaths_[0]->pDSSS_) == S_OK;

	if (!isPlaying) {
		// this code is to work around the bug where a sound can return IsPlaying==false
		// when its just got a request to start (because of latency in the drivers)

		MUSIC_TIME m_tm_now, m_tm_snd;
		soundMgr().pPerformance_->GetTime(NULL, &m_tm_now);
		audioPaths_[0]->pDSSS_->GetStartTime(&m_tm_snd);
		//DEBUG_MSG("BaseSound::isPlaying: latency check now=%ld snd=%ld\n", m_tm_now, m_tm_snd);
		if (m_tm_now <= (m_tm_snd + (DMUS_PPQ * 4))) {
			//DEBUG_MSG("BaseSound::isPlaying: latency bug workaround\n");
			isPlaying = true;
		}
	}

	return isPlaying;
}


/** Return true if a particular instance of the sound is currently being played
 */
bool BaseSound::isPlaying(uint inst) const
{
	//TRACE_MSG("BaseSound::isPlaying: inst=%d\n", inst);

	if (inst >= audioPaths_.size() || !audioPaths_[inst] || !audioPaths_[inst]->pDSSS_) {
		//TRACE_MSG("BaseSound::isPlaying: never used\n", inst);
		return false;
	}

	bool isPlaying =
		soundMgr().pPerformance_->IsPlaying(NULL, audioPaths_[inst]->pDSSS_) == S_OK;

	//TRACE_MSG("BaseSound::isPlaying: DMusic says %d\n", isPlaying);

	if (!isPlaying) {
		// this code is to work around the bug where a sound can return IsPlaying==false
		// when its just got a request to start (because of latency in the drivers)

		MUSIC_TIME m_tm_now, m_tm_snd;
		soundMgr().pPerformance_->GetTime(NULL, &m_tm_now);
		audioPaths_[inst]->pDSSS_->GetStartTime(&m_tm_snd);
		//DEBUG_MSG("BaseSound::isPlaying: latency check now=%ld snd=%ld\n", m_tm_now, m_tm_snd);
		if (m_tm_now <= (m_tm_snd + (DMUS_PPQ * 4))) {
			//DEBUG_MSG("BaseSound::isPlaying: latency bug\n");
			isPlaying = true;
		}
	}

	return isPlaying;
}


/**
 *	Select a path to use to play the sound.
 *	Choose the best one based on whether its playing, ttl of the sound etc
 *
 *	@return -1 if no path available
 */
int BaseSound::selectPathIdx()
{

//  some debug stuff
//	DEBUG_MSG("DX3DSound::play: audioPaths_={");
//	for (int i=0; i<audioPaths_.size(); i++) {
//		if (!audioPaths_[i]) {
//			DEBUG_MSG("NULL", audioPaths_[i]);
//		} else {
//			if (audioPaths_[i]->instance_ != i) {
//				DEBUG_MSG("<><>");
//			}
//			DEBUG_MSG("%d",
//					//audioPaths_[i]->pDSSS_,
//					audioPaths_[i]->snd_->isPlaying(i));
//		}
//		DEBUG_MSG(", ");
//	}
//	DEBUG_MSG("}\n");


	//DEBUG_MSG("BaseSound::selectPathIdx: nPaths=%d cap=%d\n", audioPaths_.size(), audioPaths_.capacity());
	//for (int i=0; i<audioPaths_.size(); i++) {
	//	DEBUG_MSG("BaseSound::selectPathIdx: slot %d=%p(%p)\n", i, audioPaths_[i], &audioPaths_[i]);
	//}

	int bestInst = 0;
	int useInst = -1;
	MUSIC_TIME bestTime = LONG_MAX;


	for (uint i=0; i<audioPaths_.size() && useInst == -1; i++) {

		if (!audioPaths_[i]) {
			// slot never used
			useInst = i;
			//DEBUG_MSG("BaseSound::selectPathIdx: slot empty %d\n", useInst);
		}
		else if (!audioPaths_[i]->snd_->isPlaying(i)) {
			// a sound has finished (
			useInst = i;
			//DEBUG_MSG("BaseSound::selectPathIdx: slot finished playing %d\n", useInst);
		}
		else {
			// keep track of the oldest one in case we need to reuse a live one
			MUSIC_TIME tm;
			if (audioPaths_[i]->pDSSS_->GetStartTime(&tm) == S_OK && tm < bestTime) {
				bestTime = tm;
				bestInst = i;
			}
		}
	}

	if (useInst == -1) {			// nothing free to use, reuse one or allocate a new slot

		if (audioPaths_.size() < maxInstances_) {
			audioPaths_.push_back(NULL);
			useInst = audioPaths_.size() - 1;
			//DEBUG_MSG("BaseSound::selectPathIdx: slot newly added %d\n", useInst);
		}
		else {
			useInst = bestInst;		// reuse the oldest one
			//DEBUG_MSG("BaseSound::selectPathIdx: slot reused %d\n", useInst);
		}
	}

	MF_ASSERT(useInst != -1);
	return useInst;
}



/**
 *	Set the playback frequency
 *
 *	@param	Hz		The new frequency to play the sound at (100-100,000)
 *	@param	inst	Instance of the sound to change the frequency for (0 if the sound
 *					class doesn't handle multiple instances)
 */
void BaseSound::frequency(uint Hz, uint inst)
{
	TRACE_MSG("BaseSound::frequency(set): Hz:%d inst:%d\n", Hz, inst);

	if (inst >= audioPaths_.size() || !audioPaths_[inst] || !audioPaths_[inst]->pDSB_) {
		MF_ASSERT(0);
		return;
	}

	//HACK_MSG("BaseSound::frequency(set): DSB=%p\n", audioPaths_[inst]->pDSB_);

	HRESULT hr;
	if ((hr = audioPaths_[inst]->pDSB_->SetFrequency(Hz)) != DS_OK) {
		ERROR_MSG("BaseSound::frequency: SetFrequency error %ld\n", hr);
	}



#if 0
	// src as taken from Donuts
	IDirectSoundBuffer *pDSB;
	hr = audioPaths_[inst]->pDSPath_->GetObjectInPath(DMUS_PCHANNEL_ALL,DMUS_PATH_BUFFER,
						0, GUID_NULL, 0, IID_IDirectSoundBuffer, (void **) &pDSB);

	if (hr != DS_OK) {
		ERROR_MSG("BaseSound::frequency: GetObjectInPath error %ld\n", hr);
	}
	else {
		//HACK_MSG("BaseSound::frequency(set): pDSB=%p\n", pDSB);

		if (pDSB->SetFrequency(Hz) != DS_OK) {
			ERROR_MSG("BaseSound::frequency: setFrequency error %ld\n", hr);
		}
		pDSB->Release();
	}
#endif

#if 0
	// to test buffer capabilities

	// On a 3D buffer currently have:
		//DSBCAPS_CTRL3D
		//DSBCAPS_CTRLFREQUENCY
		//DSBCAPS_CTRLFX
		//DSBCAPS_CTRLVOLUME
		//DSBCAPS_GLOBALFOCUS
		//DSBCAPS_LOCDEFER
		//DSBCAPS_MUTE3DATMAXDISTANCE


	DSBCAPS dsBufferCaps;
	dsBufferCaps.dwSize = sizeof(dsBufferCaps);
	if (audioPaths_[inst]->pDSB_->GetCaps(&dsBufferCaps) != DS_OK) {
		ERROR_MSG("BaseSound::frequency: GetCaps error %ld\n", hr);
	} else {
/*		HACK_MSG("Caps: \n");
		if (dsBufferCaps.dwFlags & DSBCAPS_CTRL3D)				{HACK_MSG("DSBCAPS_CTRL3D ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_CTRLFREQUENCY)		{HACK_MSG("DSBCAPS_CTRLFREQUENCY ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_CTRLFX)				{HACK_MSG("DSBCAPS_CTRLFX ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_CTRLPAN)				{HACK_MSG("DSBCAPS_CTRLPAN ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_CTRLVOLUME)			{HACK_MSG("DSBCAPS_CTRLVOLUME ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY)	{HACK_MSG("DSBCAPS_CTRLPOSITIONNOTIFY ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_GETCURRENTPOSITION2)	{HACK_MSG("DSBCAPS_GETCURRENTPOSITION2 ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_GLOBALFOCUS)			{HACK_MSG("DSBCAPS_GLOBALFOCUS ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_LOCDEFER)			{HACK_MSG("DSBCAPS_LOCDEFER ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_LOCHARDWARE)			{HACK_MSG("DSBCAPS_LOCHARDWARE ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_LOCSOFTWARE)			{HACK_MSG("DSBCAPS_LOCSOFTWARE ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_MUTE3DATMAXDISTANCE)	{HACK_MSG("DSBCAPS_MUTE3DATMAXDISTANCE ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_PRIMARYBUFFER)		{HACK_MSG("DSBCAPS_PRIMARYBUFFER ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_STATIC)				{HACK_MSG("DSBCAPS_STATIC ");}
		if (dsBufferCaps.dwFlags & DSBCAPS_STICKYFOCUS)			{HACK_MSG("DSBCAPS_STICKYFOCUS ");}
		HACK_MSG("\n");*/
	}
#endif
}


/** Get the playback frequency
 */
uint BaseSound::frequency(uint inst) const
{
	TRACE_MSG("BaseSound::frequency(get): inst:%d\n", inst);

	if (inst >= audioPaths_.size() || !audioPaths_[inst] || !audioPaths_[inst]->pDSB_) {
		MF_ASSERT(0);
		return 0;
	}

	DWORD freq;
	audioPaths_[inst]->pDSB_->GetFrequency(&freq);
	return freq;
}


/** This method returns the watcher associated with this class.
 */
Watcher& BaseSound::watcher()
{
	static DirectoryWatcherPtr dw = NULL;

	if (dw == NULL)	{
		dw = new DirectoryWatcher();

		BaseSound* pNull = NULL;

		dw->addChild("tag",
			new MemberWatcher<string, BaseSound>(*pNull, &BaseSound::printableTag) );
		dw->addChild("isPlaying",  new DataWatcher<bool>(pNull->isPlaying_,	Watcher::WT_READ_ONLY) );
		dw->addChild("defaultAtten",  new DataWatcher<float>(pNull->defaultAtten_,Watcher::WT_READ_WRITE) );
		dw->addChild("attenuation",new DataWatcher<float>(pNull->attenuation_,Watcher::WT_READ_ONLY) );
	}

	return *dw;
}



/** This method returns a tag without / so it can be used by watcher
 */
string BaseSound::printableTag() const
{
	string tmp = tag_;
	string::iterator i;

	for (i = tmp.begin(); i != tmp.end(); i++)
	{
		if (*i == '/')
			*i = '\\';
	}

	return tmp;
}



// -----------------------------------------------------------------------------
//                            B a s e 3 D S o u n d
// -----------------------------------------------------------------------------

/** Base3DSound constructor: internal use only
 */
Base3DSound::Base3DSound(string tag,
						const Vector3& pos,
						float minDist, float maxDist,
						float defaultAtten, float outsideAtten, float insideAtten,
						float centreHour, float minHour, float maxHour,
						int maxInstances) :
	BaseSound(tag, defaultAtten, maxInstances),
	minDistance_(minDist),
	maxDistance_(maxDist),
	centreHour_(centreHour),
	minHour_(minHour),
	maxHour_(maxHour),
	tid_(0),
	outsideAtten_(outsideAtten),
	insideAtten_(insideAtten)
{
	TRACE_MSG("Base3DSound::Base3DSound: min:%f max:%f defaultAtten:%f\n", minDist, maxDist, defaultAtten);
	MF_ASSERT(defaultAtten <= 0);
	MF_ASSERT(minHour >= 0 && minHour < 24);
	MF_ASSERT(maxHour >= 0 && maxHour < 24);
	MF_ASSERT(centreHour >= 0 && centreHour < 24);

	position_.resize(maxInstances);
	for (int i=0; i<maxInstances; i++)
		position_[i] = pos;
	if (minDistance_ < 0)
		minDistance_ = 0;
	if (maxDistance_ <= minDistance_)
		maxDistance_ = minDistance_ + 1;
	maxDistanceSquared_ = maxDist * maxDist;
}


/** Base3DSound destructor
 */
Base3DSound::~Base3DSound()
{
	TRACE_MSG("Base3DSound::~Base3DSound:\n");
}


/**
 *	This method sets the position that is associated with this sound.
 */
void Base3DSound::position(const Vector3& newPosition, uint instance)
{
	//TRACE_MSG("Base3DSound::position(write):\n");
	MF_ASSERT(instance < maxInstances_);

	position_[instance] = newPosition;
}


/**
 *	This method sets the maximum distance for the minimum volume of the sound.
 *	If the sound is outside this range it will be inaudible.
 */
void Base3DSound::maxDistance(float maxDist)
{
	TRACE_MSG("Base3DSound::maxDistance(write): [%s] %f\n", tag_.c_str(), maxDist);

	if (maxDist < 1)
		maxDist = 1;
	if (minDistance_ >= maxDist)
		minDistance_ = maxDist - 1;

	maxDistance_ = maxDist;
	maxDistanceSquared_ = maxDist * maxDist;
}


/**
 *	This method sets the maximum distance for the maximum volume of the sound.
 *	If the sound is <= this range it will be at maximum volume.
 */
void Base3DSound::minDistance(float minDist)
{
	TRACE_MSG("Base3DSound::minDistance(write): [%s] %f\n", tag_.c_str(), minDist);

	if (minDist < 0)
		minDist = 0;
	if (maxDistance_ <= minDist)
		maxDistance_ = minDist + 1;
	minDistance_ = minDist;
}



/* This method is called from the time queue to handle time based events.
 * It is used for restarting a looped sound which has a variable delay (ie not
 * continuously looped) and also for starting a delayed sound
 */
void Base3DSound::handleTimeout(TimeQueueId id, void *user)
{
	TRACE_MSG("DX3DSound::handleTimeout: %s\n", tag_.c_str());

	TimerInfo* ti = (TimerInfo*)user;

	if (ti->type == TimerInfo::LOOPDELAY) {
		MF_ASSERT(id == tid_);
		tid_ = 0;
		stopAll();
		play();
	}
	else {
		stopAll();
		play(ti->attenuation, ti->pos);
	}

	delete ti;
}


/**
 *	Note : don't clean up ti, because as far as Base3DSounds are concerned,
 *	they never get cancelled externally; i.e. handleTimeout is always called
 *	if onRelease is called.  We clean up in handleTimeout.
 *	The reason we know this, is that nobody takes a handle when the timer
 *	is added.
 */
void Base3DSound::onRelease( TimeQueueId id, void * pUser )
{
	//stub out pure virtual method.
}




// -----------------------------------------------------------------------------
//                            D X 3 D S o u n d
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
DX3DSound::DX3DSound(string tag, const Vector3& pos,
					float minDist, float maxDist,
					float defaultAtten, float outsideAtten, float insideAtten,
					float centreHour, float minHour, float maxHour,
					uint maxInstances, bool loop) :
	Base3DSound(tag, pos, minDist, maxDist, defaultAtten, outsideAtten, insideAtten,
				centreHour, minHour, maxHour, maxInstances),
	loop_(loop)
{
	TRACE_MSG("DX3DSound::DX3DSound: min:%f max:%f defaultAtten:%f\n", minDist, maxDist, defaultAtten);
	MF_ASSERT(defaultAtten <= 0);

	velocity_.resize(maxInstances);

	for (uint i = 0; i < velocity_.size(); i++)
		velocity_[i].setZero();
}


/**
 *	Destructor.
 */
DX3DSound::~DX3DSound()
{
	TRACE_MSG("DX3DSound::~DX3DSound:\n");
}


/**
 *	This method stores an access ptr to the dx sound buffer so we can change
 *	settings later.
 */
void DX3DSound::getDXObjects(AudioPath& audioPath) const
{
	Base3DSound::getDXObjects(audioPath);

	HRESULT hr;
	if ((hr=audioPath.pDSPath_->GetObjectInPath(
		DMUS_PCHANNEL_ALL,			// Performance channel
		DMUS_PATH_BUFFER,			// Stage in the path
		0,							// Index of buffer in chain
        GUID_NULL,					// Class of object
		0,							// Index of object in buffer: ignored
		IID_IDirectSound3DBuffer8,	// GUID of desired interface
		(LPVOID*)&audioPath.pDS3DB_	// Pointer that receives interface
	)) != S_OK) {
		ERROR_MSG("Base3DSound::getDXObjects: GetObjectInPath(DS3DB) failed (0x%lx)\n", hr);
	}
}



/**
 *	This method sets the position of this sound.
 */
void DX3DSound::position(const Vector3& newPosition, uint instance)
{
	//TRACE_MSG("DX3DSound::position(write):\n");
	MF_ASSERT(instance < audioPaths_.size());

	position_[instance] = newPosition;

	// NB: audioPath/pDS3DB will be valid iff sound is playing
	if (audioPaths_[instance]) {
		MF_ASSERT(audioPaths_[instance]->pDS3DB_);
		audioPaths_[instance]->pDS3DB_->SetPosition(position_[instance].x,
			position_[instance].y, position_[instance].z, DS3D_IMMEDIATE);
	}
}


/**
 *	This method sets the position of this sound.
 *	@param	newPosition	The position of the sound in world coordinates (in metres)
 *	@param	dTime		Time in seconds since the last position() call
 *	@param	instance	The instances whose position is set
 */
void DX3DSound::position(const Vector3& newPosition, float dTime, uint instance)
{
	TRACE_MSG("DX3DSound::position(write):\n");
	MF_ASSERT(instance < audioPaths_.size());

	if (!dTime)		// shouldn't really get zero, but we do sometime...
		dTime = 1;
	Vector3 oldPos = position_[instance];
	velocity_[instance] = (oldPos - newPosition) / dTime;

	position_[instance] = newPosition;

	// NB: audioPath/pDS3DB will be valid iff sound is playing
	if (audioPaths_[instance]) {
		//HACK_MSG("DX3DSound::position(write): vel=%1.1f,%1.1f,%1.1f\n",
		//	velocity_.x, velocity_.y, velocity_.z);

		MF_ASSERT(audioPaths_[instance]->pDS3DB_);
		audioPaths_[instance]->pDS3DB_->SetPosition(
			position_[instance].x, position_[instance].y, position_[instance].z, DS3D_IMMEDIATE);
		audioPaths_[instance]->pDS3DB_->SetVelocity(
			velocity_[instance].x, velocity_[instance].y, velocity_[instance].z, DS3D_IMMEDIATE);
	}
}


const Vector3& DX3DSound::position(uint instance)
{
	MF_ASSERT(instance < audioPaths_.size());

	if (audioPaths_[instance]) {
		MF_ASSERT(audioPaths_[instance]->pDS3DB_);

		D3DVECTOR pos;
		audioPaths_[instance]->pDS3DB_->GetPosition(&pos);
		position_[instance].x = pos.x;
		position_[instance].y = pos.y;
		position_[instance].z = pos.x;
	}

	return position_[instance];
}


/**
 *	Set the maxDistance for a 3D sound
 *
 *	@param maxDist	The distance in metres
 *
 *	NB: only changes distance for first instance of multiple concurrent active sounds
 *	(all subsequently started sounds are ok)
 */
void DX3DSound::maxDistance(float maxDist)
{
	TRACE_MSG("DX3DSound::maxDistance(write): [%s] %f\n", tag_.c_str(), maxDist);

	maxDistance_ = maxDist;
	maxDistanceSquared_ = maxDist * maxDist;

	// NB: pDS3DB will be valid iff sound is playing
	if (audioPaths_[0]) {
		MF_ASSERT(audioPaths_[0]->pDS3DB_);
		audioPaths_[0]->pDS3DB_->SetMaxDistance(maxDistance_, DS3D_IMMEDIATE);
	}
}


/**
 *	Set the minDistance for a 3D sound
 *
 *	@param minDist	The distance in metres
 *
 *	NB: only changes distance for first instance of multiple concurrent active sounds
 *	(all subsequently started sounds are ok)
 */
void DX3DSound::minDistance(float minDist)
{
	TRACE_MSG("DX3DSound::minDistance(write): [%s] %f\n", tag_.c_str(), minDist);

	if (minDist < 0)
		minDist = 0;

	minDistance_ = minDist;
							// NB: pDS3DB will be valid iff sound is playing
	if (audioPaths_[0]) {
		MF_ASSERT(audioPaths_[0]->pDS3DB_);
		audioPaths_[0]->pDS3DB_->SetMinDistance(minDistance_, DS3D_IMMEDIATE);
	}

	if (maxDistance_ <= minDist) {
		maxDistance_ = minDist + 1;
		maxDistanceSquared_ = maxDistance_ * maxDistance_;
		if (audioPaths_[0]) {
			MF_ASSERT(audioPaths_[0]->pDS3DB_);
			audioPaths_[0]->pDS3DB_->SetMaxDistance(maxDistance_, DS3D_IMMEDIATE);
		}
	}
}


/**
 *	This method sets the velocity associated with this sound.
 *	This is only used for calculating doppler shifts and has no other use.
 *
 *	@param	newVelocity	The velocity of the sound in metres/sec
 *	@param	instance	The instance of the sound to modify
 */
void DX3DSound::velocity(const Vector3& newVelocity, uint instance)
{
	//TRACE_MSG("Base3DSound::position(write):\n");

	velocity_[instance] = newVelocity;
}



/**
 *	Play this sound immediately
 *
 *	@return The instance number of the sound that started
 */
int DX3DSound::play()
{
	// should really be passing the position in!
	return play(/*atten*/0, position_[0]);
}


/**
 *	Play this sound immediately
 *
 *	@return The instance number of the sound that started
 */
int DX3DSound::play(float attenuation, const Vector3& pos)
{
	TRACE_MSG("       DX3DSound::play: [%s] dxAtten=%ld 0x%08x\n",
		tag_.c_str(), dxAttenuation_, (void*)this);
	MF_ASSERT(pSegment_);

	AudioPath* path = NULL;

	int useInst = selectPathIdx();
	TRACE_MSG("DX3DSound::play: selected instance=%d\n", useInst);

	if (audioPaths_[useInst])		// if it was in use release it
		audioPaths_[useInst]->releasePath();

	audioPaths_[useInst] = path = audioPathPoolCopy_->allocPath(*this);
	audioPaths_[useInst]->instance_ = useInst;

	position_[useInst] = pos;
	//TRACE_MSG("DX3DSound::play: [%s] instance=%d, pos=(%1.1f,%1.1f,%1.1f)\n",
	//	tag_.c_str(), useInst, position_[useInst].x, position_[useInst].y, position_[useInst].z);

	if (soundMgr().inShell_)
		audioPaths_[useInst]->addReverb(soundMgr().shellSize_);

	if (soundMgr().echoMix_)
		audioPaths_[useInst]->addEcho();

	HRESULT hr;

	//hr = path->pDSB_->SetFrequency(8000); //TESTING ONLY
	//if (hr != DS_OK) {ERROR_MSG("freakout1");}
	hr = path->pDS3DB_->SetMinDistance(minDistance_, DS3D_DEFERRED);
	if (hr != DS_OK)
		{ERROR_MSG("DX3DSound::play: setMinDistance\n");}
	hr = path->pDS3DB_->SetMaxDistance(maxDistance_, DS3D_DEFERRED);
	if (hr != DS_OK)
		{ERROR_MSG("DX3DSound::play: setMaxDistance\n");}
	hr = path->pDS3DB_->SetPosition(position_[useInst].x, position_[useInst].y,
		position_[useInst].z, DS3D_DEFERRED);
	if (hr != DS_OK)
		{ERROR_MSG("DX3DSound::play: setPosition\n");}
	hr = path->pDS3DB_->SetVelocity(velocity_[useInst].x, velocity_[useInst].y,
		velocity_[useInst].z, DS3D_DEFERRED);
	if (hr != DS_OK)
		{ERROR_MSG("DX3DSound::play: setVelocity\n");}

	soundMgr().pListener_->CommitDeferredSettings();

	path->priority_ = priority_;

	// reset the volume to cached value
	long dxAttenuation_ = long(100.0 * (defaultAtten_ + attenuation +
							(soundMgr().inShell_ ? insideAtten_ : outsideAtten_)));
	if (dxAttenuation_ < DSBVOLUME_MIN)
		dxAttenuation_ = DSBVOLUME_MIN;
	if (dxAttenuation_ > DSBVOLUME_MAX)
		dxAttenuation_ = DSBVOLUME_MAX;

	path->pDSPath_->SetVolume(dxAttenuation_, /*duration*/0);
	//path->pDSPath_->SetVolume(DSBVOLUME_MIN, /*duration*/0); // TEST CODE



	//pDSB_->SetVolume(dxAttenuation_); // (this one doesn't always work) ?

	hr = soundMgr().pPerformance_->PlaySegmentEx(
		pSegment_,						// Segment to play
		NULL,							// not implemented
		NULL,							// For transitions
		DMUS_SEGF_DEFAULT|DMUS_SEGF_SECONDARY,	// Flags
		0,								// Start time; 0 is immediate
		&path->pDSSS_,					// Pointer that receives segment state
		NULL,							// Object to stop
		path->pDSPath_					// Audiopath to play sound on
	);
	if (hr != DS_OK)
		{ERROR_MSG("DX3DSound::play: PlaySegmentEx returned hr 0x%08X\n", hr );}

	//TRACE_MSG("DX3DSound::play: [%s] DSPath=%p, audioPaths_[%d]=%p DSSS=%p pDS3DB=%p\n",
	//	tag_.c_str(), path->pDSPath_, useInst, path, path->pDSSS_, path->pDS3DB_);

	//hr = path->pDSB_->SetFrequency(8000); //TESTING ONLY
	//if (hr != DS_OK) {ERROR_MSG("freakout2");}

	isPlaying_ = true;

	return useInst;

	// NB: we need to watch for eof on the sound and clear the path
}



/** Play this sound after a delay
 *
 *	@param delay	delay specified in seconds
 *	@param pos		the position in world space to place sound
 */
void DX3DSound::playDelayed(float delay, const Vector3& pos)
{
	TRACE_MSG("DX3DSound::playDelayed: [%s] %f\n", tag_.c_str(), delay);

	TimerInfo* ti = new TimerInfo;

	ti->type = TimerInfo::PLAYDELAY;
	ti->pos = pos;
	ti->attenuation = 0;

	soundMgr().timeQueue_.add(
		static_cast<TimeQueue::TimeStamp>((soundMgr().currentTime_ + delay) * TIMER_SCALE),
		/*repeat*/0, this, (void*)ti);

}


/** Play this sound after a delay
 *
 *	@param delay		delay specified in seconds
 *	@param pos			the position in world space to place sound
 *	@param attenuation	volume of sound
 */
void DX3DSound::playDelayedAtten(float delay, float attenuation, const Vector3& pos)
{
	TRACE_MSG("DX3DSound::playDelayed: [%s] %f\n", tag_.c_str(), delay);

	TimerInfo* ti = new TimerInfo;

	ti->type = TimerInfo::PLAYDELAY;
	ti->pos = pos;
	ti->attenuation = attenuation;

	soundMgr().timeQueue_.add(
		static_cast<TimeQueue::TimeStamp>((soundMgr().currentTime_ + delay) * TIMER_SCALE),
		/*repeat*/0, this, (void*)ti);

}







// -----------------------------------------------------------------------------
//                          A m b i e n t S o u n d
// -----------------------------------------------------------------------------


AudioPathPool* AmbientSound::audioPathPool_ = NULL;


/**
 *	Constructor.
 */
AmbientSound::AmbientSound(string tag,
							const Vector3 &pos,
							float minDist, float maxDist,
							float defaultAtten, float outsideAtten, float insideAtten,
							float centreHour, float minHour, float maxHour,
							bool loop, float loopDelay, float loopDelayVariance) :
	Base3DSound(tag, pos, minDist, maxDist, defaultAtten, outsideAtten, insideAtten,
				centreHour, minHour, maxHour, /*maxInstances*/1),
	positionData_(1)
{
	TRACE_MSG("AmbientSound::AmbientSound: min:%f max:%f defaultAtten:%f\n",
		minDist, maxDist, defaultAtten);
	MF_ASSERT(defaultAtten <= 0);

	// because we can have multiple positions we'll put the data in a vector for easier use

	if (minDist < 0)
		minDist = 0;
	if (maxDist <= minDist)
		maxDist = minDist + 1;

	positionData_[0].minDistance = minDist;
	positionData_[0].maxDistance = maxDist;
	positionData_[0].maxDistanceSquared = maxDist * maxDist;
	positionData_[0].position = pos;
	loop_ = loop;
	loopDelay_ = loopDelay;
	loopDelayVariance_ = loopDelayVariance;

	soundMgr().ambientList_.push_back(this);
}


/**
 *	Destructor.
 */
AmbientSound::~AmbientSound()
{
	TRACE_MSG("AmbientSound::~AmbientSound:\n");

	// delete ptr from list
	SoundMgr::AmbientSoundVec* list = &soundMgr().ambientList_;
	SoundMgr::AmbientSoundVec::iterator isnd = std::find(list->begin(), list->end(), this);
	MF_ASSERT(isnd != list->end());
	if (isnd != list->end())
		list->erase(isnd);
}


/** Load an ambient sound
 */
bool AmbientSound::load(const char* filename)
{
	TRACE_MSG("AmbientSound::load: %s\n", filename);

	if (!BaseSound::load(filename))
		return false;

	if (!audioPathPool_) {
		audioPathPool_ = new AudioPathPool(
			MAX_AMBIENT_PATHS,
			DMUS_APATH_DYNAMIC_STEREO,
			soundMgr().pPerformance_);
	}
	audioPathPoolCopy_ = audioPathPool_;

	// make it loop forever
	if (loop_ && loopDelay_ == 0)
		pSegment_->SetRepeats(DMUS_SEG_REPEAT_INFINITE);

	attenuation(-MAX_ATTENUATION);	// set volume to min

	return true;
}


/**
 *	This method sets the position of this ambient sound.
 *
 *	@param newPosition	The position to place this ambient sound.
 */
void AmbientSound::position(const Vector3& newPosition)
{
	positionData_[0].position = newPosition;
}



/**
 *	This method adds a new emitter to the list of emitters for the ambient sound.
 *	The volume of the ambient sound is based on the position of the closest emitter.
 *	NB: all emitters have the same attenuation.
 */
void AmbientSound::addPosition(const Vector3& position, float minDist, float maxDist)
{
	TRACE_MSG("AmbientSound::addPosition:\n");

	if (minDist < 0)
		minDist = 0;
	if (maxDist <= minDist)
		maxDist = minDist + 1;

	struct PositionData pd;
	pd.minDistance = minDist;
	pd.maxDistance = maxDist;
	pd.maxDistanceSquared = maxDist * maxDist;
	pd.position = position;
	positionData_.push_back(pd);
}


/**
 *	This method starts playback of an ambient sound.
 *	Note that this should not be called normally by client: looped ambient
 *	sounds run automatically and non looped ones normally use
 *	play(Vector3& position).
 *
 *	@return The instance of sound started (for this sound class always zero)
 *
 */
int AmbientSound::play()
{
	TRACE_MSG("\n       AmbientSound::play: [%s]\n", tag_.c_str());
	MF_ASSERT(pSegment_);

	MF_ASSERT(!audioPaths_[0]);
	// allocate a path to play the sound on
	audioPaths_[0] = audioPathPoolCopy_->allocPath(*this);
	audioPaths_[0]->instance_ = 0;


	// reset the volume to cached value
	audioPaths_[0]->pDSPath_->SetVolume(dxAttenuation_, /*duration*/0);
	audioPaths_[0]->priority_ = priority_;

	soundMgr().pPerformance_->PlaySegmentEx(
		pSegment_,					// Segment to play
		NULL,						// not implemented
		NULL,						// For transitions
		DMUS_SEGF_DEFAULT|DMUS_SEGF_SECONDARY,		// Flags
		0,							// Start time; 0 is immediate
		&audioPaths_[0]->pDSSS_,	// Pointer that receives segment state
		NULL,						// Object to stop
		audioPaths_[0]->pDSPath_	// Audiopath to play sound on
	);

	isPlaying_ = true;

	//DEBUG_MSG("AmbientSound::play: [%s] DSPath:%p pDSSS=%p\n", tag_.c_str(),
	//	audioPaths_[0]->pDSPath_, audioPaths_[0]->pDSSS_);


	if (loop_ && loopDelay_ != 0) {
		float delay = loopDelay_;
		if (loopDelayVariance_)
			delay += (rand() % int(loopDelayVariance_ * 10.0f)) / 10.0f;

		TimerInfo* ti = new TimerInfo;
		ti->type = TimerInfo::LOOPDELAY;
		ti->attenuation = 0;
		ti->pos = Vector3::zero();

		tid_ = soundMgr().timeQueue_.add(
			static_cast<TimeQueue::TimeStamp>((soundMgr().currentTime_ + delay) * TIMER_SCALE),
			/*repeat*/0, this, (void*)ti);
	}

	// TODO: ambient sounds don't need to be looped now.
	// NB: an ambient sound is looped, so the only way to stop an ambient sound playing is via
	// stop().  Therefore we don't need to watch for eof on the sound and do anything special.
	return 0;
}



/**
 *	This method sets the position then starts playback of an ambient sound.
 *	Warning: attenuation is not currently used.
 *
 *	Can only be used on single shot sounds (non-looped ones).
 *
 *	@return The instance of sound started (for this sound class always zero)
 */
int AmbientSound::play(float attenuation, const Vector3& pos)
{
	TRACE_MSG("AmbientSound::play: %f,%f,%f [%s]\n",
		pos.x, pos.y, pos.z, tag_.c_str());

	MF_ASSERT(!loop_);

	position(pos);
	return play();
}


/**
 *	This method returns the watcher associated with this class.
 */
Watcher& AmbientSound::watcher()
{
	static DirectoryWatcherPtr dw = NULL;

	if (dw == NULL) {
		dw = new DirectoryWatcher();

		AmbientSound* pNull = NULL;

		dw->addChild("tag",
			new MemberWatcher<string, BaseSound>(*pNull, &BaseSound::printableTag) );

		dw->addChild("isPlaying",new DataWatcher<bool>(pNull->isPlaying_, Watcher::WT_READ_ONLY) );
		dw->addChild("defaultAtten",new DataWatcher<float>(pNull->defaultAtten_,Watcher::WT_READ_WRITE) );
		dw->addChild("outsideAtten",new DataWatcher<float>(pNull->outsideAtten_,Watcher::WT_READ_WRITE) );
		dw->addChild("insideAtten", new DataWatcher<float>(pNull->insideAtten_,Watcher::WT_READ_WRITE) );
		dw->addChild("attenuation",new DataWatcher<float>(pNull->attenuation_,Watcher::WT_READ_ONLY) );
		dw->addChild("loop",new DataWatcher<bool>(pNull->loop_, Watcher::WT_READ_WRITE) );

		dw->addChild("minDist",  new MemberWatcher<float, AmbientSound>(
			*pNull, MF_ACCESSORS(float, AmbientSound, minDistance)));
		dw->addChild("maxDist",  new MemberWatcher<float, AmbientSound>(
			*pNull, MF_ACCESSORS(float, AmbientSound, maxDistance)));

		dw->addChild("position", new MemberWatcher<const Vector3&, AmbientSound, Vector3>(
			*pNull, MF_ACCESSORS(const Vector3&, AmbientSound, position0)) );

	}

	return *dw;
}





// -----------------------------------------------------------------------------
//                      S t a t i c 3 D S o u n d
// -----------------------------------------------------------------------------


AudioPathPool* Static3DSound::audioPathPool_ = NULL;


/** Constructor.
 */
Static3DSound::Static3DSound(string tag,
							const Vector3 &pos,
							float minDist, float maxDist,
							float defaultAtten, float outsideAtten, float insideAtten,
							float centreHour, float minHour, float maxHour,
							bool loop, float loopDelay, float loopDelayVariance) :
	DX3DSound(tag, pos, minDist, maxDist, defaultAtten, outsideAtten, insideAtten,
			centreHour, minHour, maxHour, /*maxInstances*/1, loop),
	loopDelay_(loopDelay),
	loopDelayVariance_(loopDelayVariance)
{
	TRACE_MSG("Static3DSound::Static3DSound: min:%f max:%f defaultAtten:%f\n", minDist, maxDist, defaultAtten);

	soundMgr().static3DList_.push_back(this);
}


/** Destructor
 */
Static3DSound::~Static3DSound()
{
	TRACE_MSG("Static3DSound::~Static3DSound:\n");

	// delete ptr from list
	SoundMgr::Static3DSoundVec* list = &soundMgr().static3DList_;
	SoundMgr::Static3DSoundVec::iterator isnd = std::find(list->begin(), list->end(), this);
	MF_ASSERT(isnd != list->end());
	if (isnd != list->end())
		list->erase(isnd);
}


/**
 *	Start the sound playing
 *
 *	@return The instance of sound started
 */
int Static3DSound::play(float attenuation, const Vector3& pos)
{
	int useInst = this->DX3DSound::play(attenuation, pos);

	if (loop_ && loopDelay_ != 0) {
		float delay = loopDelay_;
		if (loopDelayVariance_)
			delay += (rand() % int(loopDelayVariance_ * 10.0f)) / 10.0f;

		TimerInfo* ti = new TimerInfo;
		ti->type = TimerInfo::LOOPDELAY;
		ti->attenuation = attenuation;
		ti->pos = Vector3::zero();

		tid_ = soundMgr().timeQueue_.add(
			static_cast<TimeQueue::TimeStamp>((soundMgr().currentTime_ + delay) * TIMER_SCALE),
			/*repeat*/0, this, (void*)ti);
	}

	return useInst;
}


/**
 *	Start the sound playing
 *
 *	@return The instance of sound started
 */
int Static3DSound::play()
{
	return play(/*atten*/0, position0());
}



/**
 *	This method returns the watcher associated with this class.
 */
Watcher& Static3DSound::watcher()
{
	static DirectoryWatcherPtr dw = NULL;

	if (dw == NULL)	{
		dw = new DirectoryWatcher();

		Static3DSound * pNull = NULL;


		dw->addChild("tag",
			new MemberWatcher<string, BaseSound>(*pNull, &BaseSound::printableTag) );

		dw->addChild("isPlaying",  new DataWatcher<bool>(pNull->isPlaying_,	Watcher::WT_READ_ONLY) );
		//dw->addChild("defaultAtten",  new DataWatcher<float>(pNull->defaultAtten_,Watcher::WT_READ_WRITE) );
		dw->addChild("attenuation", new DataWatcher<float>(pNull->attenuation_,   Watcher::WT_READ_ONLY) );
		dw->addChild("outsideAtten", new DataWatcher<float>(pNull->outsideAtten_,   Watcher::WT_READ_WRITE) );
		dw->addChild("insideAtten",  new DataWatcher<float>(pNull->insideAtten_,   Watcher::WT_READ_WRITE) );

		dw->addChild("defaultAtten",
			new MemberWatcher<float, Static3DSound>(
				*pNull, MF_ACCESSORS(float, Static3DSound, defaultAttenuation)));

		dw->addChild("minDistance",
			new MemberWatcher<float, Static3DSound>(
				*pNull, MF_ACCESSORS(float, Static3DSound, minDistance)));

		dw->addChild("maxDistance",
			new MemberWatcher<float, Static3DSound>(
				*pNull, MF_ACCESSORS(float, Static3DSound, maxDistance)));

		dw->addChild("position",
			new MemberWatcher<const Vector3&, Static3DSound, Vector3>(
				*pNull, MF_ACCESSORS(const Vector3&, Static3DSound, position0) ) );
	}

	return *dw;
}



// -----------------------------------------------------------------------------
//                             F x S o u n d
// -----------------------------------------------------------------------------


AudioPathPool* FxSound::audioPathPool_ = NULL;


/** Constructor
 */
FxSound::FxSound(string tag, float minDist, float maxDist, float defaultAtten,
				 uint maxInstances, bool loop) :
	DX3DSound(tag, Vector3(0,0,0), minDist, maxDist, defaultAtten,
			/*outsideAtten*/0, /*insideAtten*/0, /*centerHour*/0, /*minHour*/0,
			/*maxHour*/0, maxInstances, loop)
{
	TRACE_MSG("FxSound::FxSound: %s min:%f max:%f defaultAtten:%f\n",
		tag.c_str(), minDist, maxDist, defaultAtten);
	MF_ASSERT(defaultAtten <= 0);

	soundMgr().fxList_.push_back(this);

	soundMgr().fxMap_.insert(std::make_pair(tag, this));
}


/** Destructor
 */
FxSound::~FxSound()
{
	TRACE_MSG("FxSound::~FxSound: %s\n", tag_.c_str());

	// delete ptr from list
	SoundMgr::FxSoundVec* list = &soundMgr().fxList_;
	SoundMgr::FxSoundVec::iterator isnd = std::find(list->begin(), list->end(), this);
	MF_ASSERT(isnd != list->end());
	if (isnd != list->end())
		list->erase(isnd);

	soundMgr().fxMap_.erase(tag_);
}


/**
 *	This method returns the watcher associated with this class.
 */
Watcher& FxSound::watcher()
{
	static DirectoryWatcherPtr dw = NULL;

	if (dw == NULL)	{
		dw = new DirectoryWatcher();

		FxSound * pNull = NULL;

		dw->addChild("tag",
			new MemberWatcher<string, BaseSound>(*pNull, &BaseSound::printableTag) );

		dw->addChild("isPlaying",  new DataWatcher<bool>(pNull->isPlaying_,	  Watcher::WT_READ_ONLY) );
		dw->addChild("attenuation",new DataWatcher<float>(pNull->attenuation_,Watcher::WT_READ_ONLY) );

		dw->addChild("defaultAtten",
			new MemberWatcher<float, FxSound>(
				*pNull, MF_ACCESSORS(float, FxSound, defaultAttenuation)));

		dw->addChild("minDistance",
			new MemberWatcher<float, FxSound>(
				*pNull, MF_ACCESSORS(float, FxSound, minDistance)));

		dw->addChild("maxDistance",
			new MemberWatcher<float, FxSound>(
				*pNull, MF_ACCESSORS(float, FxSound, maxDistance)));
	}

	return *dw;
}


// -----------------------------------------------------------------------------
//                         S i m p l e  S o u n d
// -----------------------------------------------------------------------------


AudioPathPool* SimpleSound::audioPathPool_ = NULL;

/** Constructor
 */
SimpleSound::SimpleSound(string tag, float defaultAtten, uint maxInstances, bool loop) :
	BaseSound(tag, defaultAtten, maxInstances),
	loop_(loop)
{
	TRACE_MSG("SimpleSound::SimpleSound: %s defaultAtten:%f maxInst:%d\n",
		tag.c_str(), defaultAtten, maxInstances);
	MF_ASSERT(defaultAtten <= 0);

	soundMgr().simpleList_.push_back(this);

	// strip the basename out of the sound (no path or extension) for use as tag
	char fname[128];
	_splitpath(tag.c_str(), NULL, NULL, fname, NULL);
	tag_ = string(fname);
	soundMgr().soundMap_.insert(std::make_pair(tag_, this));
}


/** Destructor
 */
SimpleSound::~SimpleSound()
{
	TRACE_MSG("SimpleSound::~SimpleSound: %s\n", tag_.c_str());

	// delete ptr from list
	SoundMgr::SimpleSoundVec* list = &soundMgr().simpleList_;
	SoundMgr::SimpleSoundVec::iterator isnd = std::find(list->begin(), list->end(), this);
	MF_ASSERT(isnd != list->end());
	if (isnd != list->end())
		list->erase(isnd);

	soundMgr().soundMap_.erase(tag_);
}


/**
 *	Start the sound playing
 *
 *	@param attenuation	extra attenuation added to default attenuation
 *
 *	@return The instance of sound started
 */
int SimpleSound::play(float attenuation)
{
	TRACE_MSG("SimpleSound::play: [%s]\n", tag_.c_str());

	MF_ASSERT(pSegment_);

	AudioPath* path = NULL;

	int useInst = selectPathIdx();

	if (audioPaths_[useInst])		// if it was in use release it
		audioPaths_[useInst]->releasePath();

	audioPaths_[useInst] = path = audioPathPoolCopy_->allocPath(*this);
	audioPaths_[useInst]->instance_ = useInst;

	long dxAttenuation = long((attenuation + defaultAtten_) * 100.0f);
	if (dxAttenuation < DSBVOLUME_MIN)
		dxAttenuation = DSBVOLUME_MIN;
	if (dxAttenuation > DSBVOLUME_MAX)
		dxAttenuation = DSBVOLUME_MAX;
	path->pDSPath_->SetVolume(dxAttenuation, /*duration*/0);

	path->priority_ = float(abs(dxAttenuation));

	if (loop_)
		pSegment_->SetRepeats(DMUS_SEG_REPEAT_INFINITE);

	soundMgr().pPerformance_->PlaySegmentEx(
		pSegment_,						// Segment to play
		NULL,							// not implemented
		NULL,							// For transitions
		DMUS_SEGF_DEFAULT|DMUS_SEGF_SECONDARY,	// Flags
		0,								// Start time; 0 is immediate
		&path->pDSSS_,					// Pointer that receives segment state
		NULL,							// Object to stop
		path->pDSPath_					// Audiopath to play sound on
	);

	isPlaying_ = true;

	// NB: we need to watch for eof on the sound and clear the path
	return useInst;
}


/**
 *	Start the sound playing
 *
 *	@return The instance of sound started
 */
int SimpleSound::play()
{
	TRACE_MSG("SimpleSound::play: [%s]\n", tag_.c_str());

	MF_ASSERT(pSegment_);

	AudioPath* path = NULL;

	int useInst = selectPathIdx();

	if (audioPaths_[useInst])		// if it was in use release it
		audioPaths_[useInst]->releasePath();

	audioPaths_[useInst] = path = audioPathPoolCopy_->allocPath(*this);
	audioPaths_[useInst]->instance_ = useInst;

	// reset the volume to cached value
	path->pDSPath_->SetVolume(dxAttenuation_, /*duration*/0);

	path->priority_ = float(abs(dxAttenuation_));

	//pDSB_->SetVolume(dxAttenuation_); // (this one doesn't always work) ?

	if (loop_)
		pSegment_->SetRepeats(DMUS_SEG_REPEAT_INFINITE);

	soundMgr().pPerformance_->PlaySegmentEx(
		pSegment_,						// Segment to play
		NULL,							// not implemented
		NULL,							// For transitions
		DMUS_SEGF_DEFAULT|DMUS_SEGF_SECONDARY,	// Flags
		0,								// Start time; 0 is immediate
		&path->pDSSS_,					// Pointer that receives segment state
		NULL,							// Object to stop
		path->pDSPath_					// Audiopath to play sound on
	);

	isPlaying_ = true;

	// NB: we need to watch for eof on the sound and clear the path
	// (it'll work without it, its just for efficiency)

	return useInst;
}


/**
 *	This method returns the watcher associated with this class.
 */
Watcher& SimpleSound::watcher()
{
	static DirectoryWatcherPtr dw = NULL;

	if (dw == NULL)	{
		dw = new DirectoryWatcher();

		SimpleSound * pNull = NULL;

		dw->addChild("tag",
			new MemberWatcher<string, BaseSound>(*pNull, &BaseSound::printableTag) );

		dw->addChild("defaultAtten",
			new MemberWatcher<float, SimpleSound>(
				*pNull, MF_ACCESSORS(float, SimpleSound, defaultAttenuation)));
	}

	return *dw;
}

// soundmgr.cpp
