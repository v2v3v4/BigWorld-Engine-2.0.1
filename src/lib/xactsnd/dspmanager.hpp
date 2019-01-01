/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DSPMANAGER_HPP
#define DSPMANAGER_HPP

#include "resmgr/bin_section.hpp"
#include <xact.h>

// fwd decl so the header doesn't need to be included
class CDSPServer;
//struct IXACTEngine;
//struct DSEFFECTIMAGEDESC;


/** This class manages the DSP processor
 */
class DSPManager
{
public:
	DSPManager();
	~DSPManager();

	bool load(const char* dspImageName, IXACTEngine* pXACT);

	void startDSPServer(IXACTEngine* pXACT);

	//void* imageData();
	//size_t imageDataLen();

	DSEFFECTIMAGEDESC* effectImageDesc();

private:
	DSPManager( const DSPManager& );
	DSPManager& operator=( const DSPManager& );

	CDSPServer* dspServer_;
	DataSectionPtr dspImageDsp_;
	DSEFFECTIMAGEDESC* pEffectImageDesc_;
};

extern DSPManager gDSPManager;

#ifdef CODE_INLINE
#include "dspmanager.ipp"
#endif

#endif // DSPMANAGER_HPP
