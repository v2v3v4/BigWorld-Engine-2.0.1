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

//#include <algorithm>

#include "cstdmf/debug.hpp"
//#include "cstdmf/watcher.hpp"
#include "resmgr/bwresource.hpp"
//#include "cstdmf/dogwatch.hpp"

#include <xact.h>
#include <xactwb.h>

#include "dspmanager.hpp"
#if defined(_INSTRUMENTED) || defined(_DEBUG)
#include "dspserver.h"
#endif
#include "../../../cz/src/client/voicefx.h"

#ifndef CODE_INLINE
#include "dspmanager.ipp"
#endif


DECLARE_DEBUG_COMPONENT2( "DSPManager", 0 )	// debugLevel for this file

DSPManager gDSPManager;	// global instance

// -----------------------------------------------------------------------------
// Section: DSPManager
// -----------------------------------------------------------------------------

// class static defs
//CDSPServer*			DSPManager::dspServer_;
//DSEFFECTIMAGEDESC*	DSPManager::pEffectImageDesc_;
//DataSectionPtr		DSPManager::dspImageDsp_;


/**
 *	Constructor.
 */
DSPManager::DSPManager()
{
	TRACE_MSG("DSPManager::DSPManager\n");
}


/**
 *	Destructor.
 */
DSPManager::~DSPManager()
{
#if defined(_INSTRUMENTED) || defined(_DEBUG)
	MF_ASSERT(dspServer_);
	dspServer_->Stop();
	delete dspServer_;
	dspServer_ = NULL;
#endif
}



bool DSPManager::load(const char* dspImageName, IXACTEngine* pXACT)
{
	dspImageDsp_ = BWResource::openSection(std::string(dspImageName));
	if (!dspImageDsp_) {
		ERROR_MSG("DSPManager::load: cannot find DSP image '%s'\n", dspImageName);
		return false;
	}

	DSEFFECTIMAGELOC dseil;
	dseil.dwI3DL2ReverbIndex = GraphI3DL2_I3DL2Reverb; //I3DL2_CHAIN_I3DL2_REVERB;
	dseil.dwCrosstalkIndex   = GraphXTalk_XTalk; //I3DL2_CHAIN_XTALK;
	if (FAILED(pXACT->DownloadEffectsImage(
			const_cast<void*>(dspImageDsp_->asBinary()->data()),
			dspImageDsp_->asBinary()->len(), &dseil, &pEffectImageDesc_))
	) {
		ERROR_MSG("DSPManager::load: DownloadEffectsImage() failed\n");
		return false;
	}

	DSI3DL2LISTENER r = {DSI3DL2_ENVIRONMENT_PRESET_NOREVERB};
	pXACT->SetI3dl2Listener(&r, DS3D_IMMEDIATE);

	return true;
}



//void* DSPManager::imageData()
//{
//	MF_ASSERT(dspImageDsp_);
//	return const_cast<void*>(dspImageDsp_->asBinary()->data());
//}

//size_t DSPManager::imageDataLen()
//{
	//MF_ASSERT(dspImageDsp_);
	//return dspImageDsp_->asBinary()->len();
//}



DSEFFECTIMAGEDESC* DSPManager::effectImageDesc()
{
	return pEffectImageDesc_;
}

void DSPManager::startDSPServer(IXACTEngine* pXACT)
{
#if defined(_INSTRUMENTED) || defined(_DEBUG)
	dspServer_ = new CDSPServer();
	dspServer_->Run(pXACT);
#endif
}

// dspmanager.cpp
