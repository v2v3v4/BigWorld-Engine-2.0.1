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

/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

	dspserver.cpp

Abstract:

	DSP Server code. This module will run as a thread, and allow
	connections from dspbuilder running on a PC to modify the GP DSP image

Revision History:

	19-Jun-2001 Initial Version

--*/

//------------------------------------------------------------------------------
//	Includes:
//------------------------------------------------------------------------------
#include "dspserver.h"
#include <assert.h>

//------------------------------------------------------------------------------
//	Static Member Variables
//------------------------------------------------------------------------------
BOOL			CDSPServer::m_threadAlive	= FALSE;
SOCKET			CDSPServer::m_socket;
USHORT			CDSPServer::m_port;
unsigned		CDSPServer::m_uTimeout;
LPXACTENGINE	CDSPServer::m_pXACT;
HANDLE			CDSPServer::m_thread;
void (__cdecl *CDSPServer::m_pPreDownloadCallback)(void) = NULL;
void (__cdecl *CDSPServer::m_pDownloadCallback)(LPDSEFFECTIMAGEDESC pImageDes,
										       int count, char** pEffectNames, 
											   int* pEffectIndices) = NULL;
void (__cdecl *CDSPServer::m_pSetEffectData)(DWORD dwIndex, DWORD dwOffset,
										     LPCVOID pData, DWORD dwSize, 
											 DWORD dwFlags) = NULL;
void (__cdecl *CDSPServer::m_pSetI3DL2Listener)(LPDSI3DL2LISTENER pData) = NULL;
void (__cdecl *CDSPServer::m_pConnectCallback)(void) = NULL;
void (__cdecl *CDSPServer::m_pDisconnectCallback)(void) = NULL;

//------------------------------------------------------------------------------
//	Static Local Functions:
//------------------------------------------------------------------------------
static DWORD WINAPI ThreadFunc(LPVOID pParam);
static BOOL HandleRequest(SOCKET);
static int IsDataAvailable(SOCKET);
static DWORD GetCRC(const VOID* pBuffer, ULONG size);



//------------------------------------------------------------------------------
//	CDSPServer::Run
//	Routine Description:
//	
//		Constructor
//	
//	Arguments:
//
//	    IN pDirectSound -	Direct sound object to use for DSP changes. If NULL,
//							One will be created.
//		IN usPort -			Port to use for connection to dspbuilder. If a value 
//							other than the default is used, you will need to set
//							that value in dspbuilder as well
//		IN uTimeout -		Timeout in milleseconds
//	    IN pPreCallback -	If not NULL, called before a new effects image is
//							downloaded
//	    IN pCallback -		If not NULL, called after a new effects image is
//							dowloaded
//						OUT pImageDes -			Pointer to the DSEFFECTIMAGEDESC 
//												structure that is returned by the 
//												method, which describes the DSP 
//												scratch image. 
//						OUT count -				Number of effects in image
//						OUT pEffectNames -		Array of effect name strings 
//												(each null terminated)
//						OUT pEffectIndices -	Array of effect indices
//		IN pSetEffectData -	Called after an effect parameter is changed. This
//							callback is made with the same paramaters made to 
//							the call to DSound::SetEffectData()
//						IN dwIndex -	Effect index
//						IN dwOffset -	Offset into pData
//						IN pData -		Effect data 
//						IN dwSize -		Size of pData 
//						IN dwFlags -	Flags
//		IN pSetI3DL2Listener -	Called after a call to DSound::SetI3DL2Listener
//								with the same parameters
//		IN pConnectCallback - Called when dspbuilder connects
//		IN pDisconnectCallback - Called when dspbuilder disconnects
//	
//	Return Value:
//	
//		None
//------------------------------------------------------------------------------

BOOL CDSPServer::Run(
	IN LPXACTENGINE		pXACTEngine, 
	IN USHORT			usPort, 
	IN unsigned			uTimeout,
	IN void (__cdecl *pPreCallback)(void),
	IN void (__cdecl *pCallback)(
		LPDSEFFECTIMAGEDESC pImageDes, int count, char** pEffectNames, int* pEffectIndices),
	IN void (__cdecl *pSetEffectData)(DWORD dwIndex, DWORD dwOffset, LPCVOID pData, 
		DWORD dwSize, DWORD dwFlags),
	IN void (__cdecl *pSetI3DL2Listener)(LPDSI3DL2LISTENER pData),
	IN void (__cdecl *pConnectCallback)(void),
	IN void (__cdecl *pDisconnectCallback)(void)
)
{
	sockaddr_in			sockaddr;
    WSADATA				wsaData;
    unsigned short		version		= MAKEWORD(2, 2);
	XNetStartupParams	params		= { sizeof(XNetStartupParams), 
										XNET_STARTUP_BYPASS_SECURITY };

	// Stop if currently running
	if (CDSPServer::m_threadAlive)
		CDSPServer::Stop();

	// Initialize
	CDSPServer::m_port					= usPort;
    CDSPServer::m_socket				= NULL;
    CDSPServer::m_uTimeout				= uTimeout;
	CDSPServer::m_pXACT					= pXACTEngine;
	CDSPServer::m_pPreDownloadCallback	= pPreCallback;
	CDSPServer::m_pDownloadCallback		= pCallback;
	CDSPServer::m_pSetEffectData		= pSetEffectData;
	CDSPServer::m_pSetI3DL2Listener		= pSetI3DL2Listener;
	CDSPServer::m_pConnectCallback		= pConnectCallback;
	CDSPServer::m_pDisconnectCallback	= pDisconnectCallback;

	// Startup...
    XNetStartup(&params);
    WSAStartup(version, &wsaData);

	// Create or reference DirectSound Object?
	if (CDSPServer::m_pXACT)
		CDSPServer::m_pXACT->AddRef();
	else {
		return FALSE;
		//if (DirectSoundCreate(NULL, &CDSPServer::m_pXACT, NULL) != S_OK)
		//	return FALSE;
	}

	// Create the socket
    CDSPServer::m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (CDSPServer::m_socket == INVALID_SOCKET)
		return FALSE;

	// Initialize socket parameters
	memset(&sockaddr, 0, sizeof(SOCKADDR));
    sockaddr.sin_family			= AF_INET;
    sockaddr.sin_port			= htons(m_port);
    sockaddr.sin_addr.s_addr	= htonl(INADDR_ANY);

	// Bind the socket
    if (bind(CDSPServer::m_socket, (LPSOCKADDR)&sockaddr, sizeof(SOCKADDR)) == SOCKET_ERROR)
	{
	    shutdown(CDSPServer::m_socket, SD_BOTH);
		closesocket(CDSPServer::m_socket);
		return FALSE;
    }

	// Create the thread
	CDSPServer::m_threadAlive = TRUE;
    m_thread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, NULL);

	return TRUE;
}

//------------------------------------------------------------------------------
//	CDSPServer::Stop
//
//		Destructor.
//
// Arguments:
//
//		None
//
//Return Value:
//
//		None
//
//------------------------------------------------------------------------------
void CDSPServer::Stop(void)
{
	// Signal the thread to terminate
	m_threadAlive = FALSE;
	
	// Set thread priority to the highest so it expires quickly
    SetThreadPriority(m_thread, THREAD_PRIORITY_HIGHEST);

	// Wait for the thread to terminate
	WaitForSingleObject(m_thread, 500);
	
	// shutdown and close the socket
    shutdown(m_socket, SD_BOTH);
    closesocket(m_socket);

	// Cleanup
    WSACleanup();

	// Release the DirectSound object
	CDSPServer::m_pXACT->Release();
}


//------------------------------------------------------------------------------
//	ThreadFunc
//
//		Thread code for our Winsock Server
//
//	Arguments:
//		IN pParam -	A pointer to this
//	
//	Return Value:
//		0
//------------------------------------------------------------------------------

static DWORD WINAPI ThreadFunc(IN LPVOID pParam)
{
	SOCKET	socket;
	linger	noTimeWait = { true, 0 };
	int		ret;

    if (listen(CDSPServer::m_socket, 5) != SOCKET_ERROR)
	{
		while (CDSPServer::m_threadAlive)
		{
			socket = accept(CDSPServer::m_socket, NULL, NULL);

			if (socket == INVALID_SOCKET)
				continue;
			else
			{
				// turn off the time wait delay after closing the socket
				setsockopt(socket, SOL_SOCKET, SO_LINGER, (char*)&noTimeWait, 
						   sizeof(noTimeWait));

				// Loop until an error occurs
				while (CDSPServer::m_threadAlive)
				{
					ret = IsDataAvailable(socket);
					if ((ret == SOCKET_ERROR) || ((ret > 0) && !HandleRequest(socket)))
						break;
					else
						Sleep(25);
				}

				// Shutdown & close the socket
				shutdown(socket, SD_BOTH);
				closesocket(socket);
			}
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
//	HandleRequest
//------------------------------------------------------------------------------
//	Does the real work. Each "packet" of data is broken into two parts, a header
//	and a body.
//
//	Header Format:		Each block will have a header
//
//	BYTE	head[10]	"DSPBUILDER"
//	BYTE	type[8]		Block type: "DSPIMAGE", "DSPINDEX", "FXPARAMS",
//									"I3DL2LIS", "XCONNECT", "XDSCNNCT"
//	DWORD	size		Body size (in bytes)
//	DWORD	crc			CRC of body
//	BYTE	tail[10]	"DSPBUILDER"
//
//	DSPIMAGE body:			DSP image: DSound::DownloadEffectsImage()
//
//	BYTE	head[8]			"DSPIMAGE"
//	BYTE	version			Version 2
//	DWORD	i3dl2Index		I3DL2 index
//	DWORD	xtalkIndex		XTalk index	
//	BYTE	image[size-25]	Image data
//	BYTE	tail[8]			"DSPIMAGE"
//
//	DSPINDEX body:			Effect indices within the DSP image
//	
//	BYTE	head[8]			"DSPINDEX"
//	BYTE	version			Version 1
//	DWORD	numEffects		Number of effects
//	CHAR*	effectName0		Null terminated effect name
//	CHAR*	effectIndex0	Null terminated effect index
//	CHAR*	effectName1		Null terminated effect name
//	CHAR*	effectIndex1	Null terminated effect index
//	...
//	CHAR*	effectNameN-1	Null terminated effect name
//	CHAR*	effectIndexN-1	Null terminated effect index
//	BYTE	tail[8]			"DSPINDEX"
//
//	FXPARAMS body:			FX parameters for one or more effects: DSound::SetEffectData()
//
//	BYTE	head[8]			"FXPARAMS"
//	BYTE	version			Version 1
//	DWORD	numParams		Number of effect params
//	DWORD	index0			Effect index
//	DWORD	offset0			Data offset
//	DWORD	size0			Size of effect data
//	BYTE*	data0			Effect data
//	DWORD	index1			Effect index
//	DWORD	offset1			Data offset
//	DWORD	size1			Size of effect data
//	BYTE*	data1			Effect data
//	...
//	DWORD	indexN-1		Effect index
//	DWORD	offsetN-1		Data offset
//	DWORD	sizeN-1			Size of effect data
//	BYTE*	dataN-1			Effect data
//	BYTE	tail[8]			"FXPARAMS"
//
//	I3DL2LIS body:				I3DL2 Listener parameters: DSound::SetI3DL2Listener()
//
//	BYTE	head[8]				"I3DL2LIS"
//	BYTE	version				Version 1
//	LONG	lRoom;				I3DL2 parameters follow...
//	LONG	lRoomHF;
//	FLOAT	flRoomRolloffFactor;
//	FLOAT	flDecayTime;
//	FLOAT	flDecayHFRatio;
//	LONG	lReflections;
//	FLOAT	flReflectionsDelay;
//	LONG	lReverb;
//	FLOAT	flReverbDelay;
//	FLOAT	flDiffusion;
//	FLOAT	flDensity;
//	FLOAT	flHFReference;
//	BYTE	tail[8]				"I3DL2LIS"
//
//	XCONNECT body: None
//	XDSCNNCT body: None
//
//Arguments:
//
//	IN socket -	Socket
//
//Return Value:
//
//	None


static BOOL HandleRequest(IN SOCKET socket)
{
	char						header[36];
	HRESULT						result;
	DSEFFECTIMAGELOC			imageLoc;
	BYTE						version;
	DWORD						size;
	DWORD						count;
	DWORD						crc;
	DWORD						i;
	DWORD						c;
	DWORD						bi;
	int							ret;
	char*						pBuffer;
	char**						pEffectName;
	int*						pEffectIndex;
	char						charBuffer[256];
	int							totalRead;
	DSI3DL2LISTENER				listener;
	static LPDSEFFECTIMAGEDESC	pDesc = NULL;

	// Receive the header
	totalRead = 0;
	while(totalRead != 36)
	{
		ret = recv(socket, &header[totalRead], 36-totalRead, 0);
		if ((ret == 0) || (ret == SOCKET_ERROR))
			return FALSE;
		else
			totalRead += ret;
	}
	
	if (totalRead != 36)
		return FALSE;

	// Is it a valid header?
	if (memcmp(header, "DSPBUILDER", 10) || memcmp(&header[26], "DSPBUILDER", 10))
		return FALSE;

	// Size
	size = *(DWORD*)&header[18];
	
	// Is there a data blocK?
	if (size != 0)
	{
		// Read the crc
		crc = *(DWORD*)&header[22];

		// Allocate the buffer
		pBuffer = new char [size];
		if (!pBuffer)
			return FALSE;

		// Receive the block
		totalRead = 0;
		while(totalRead != size)
		{
			ret = recv(socket, &pBuffer[totalRead], size-totalRead, 0);
			if (ret == SOCKET_ERROR) {
				delete [] pBuffer;
				return FALSE;
			}
			else
				totalRead += ret;
		}

		// Check the CRC
		if (GetCRC(pBuffer, size) != crc) {
			delete [] pBuffer;
			return FALSE;
		}
	}

	// DSP image packet
	if (!memcmp(&header[10], "DSPIMAGE", 8))
	{
		// Read the header & footer
		if (memcmp(pBuffer, "DSPIMAGE", 8) || memcmp(&pBuffer[size-8], "DSPIMAGE", 8)) {
			delete [] pBuffer;
			return FALSE;
		}

		// Read the version
		version = *(BYTE*)&pBuffer[8];
		if ((version != 1) && (version != 2)) {
			delete [] pBuffer;
			return FALSE;
		}

		// Read the I3DL2 Reverb Index
		imageLoc.dwI3DL2ReverbIndex = *(DWORD*)&pBuffer[9];

		// Read the XTalk Index
		imageLoc.dwCrosstalkIndex = *(DWORD*)&pBuffer[13];

		// Callback
		if (CDSPServer::m_pPreDownloadCallback)
			CDSPServer::m_pPreDownloadCallback();

		// Load the image
		pDesc	= NULL;
		result	= CDSPServer::m_pXACT->DownloadEffectsImage(&pBuffer[17], size-25, &imageLoc, &pDesc);

		// Version 1 files only have an effects image, no effect indices
		// so do the download callback now as opposed to the version 2 method
		// of making the callback after the effect indices are dowloaded
		if ((version == 1) && (CDSPServer::m_pDownloadCallback))
			CDSPServer::m_pDownloadCallback(pDesc, 0, NULL, NULL);

		// Cleanup
		delete [] pBuffer;
		return TRUE;
	}

	// Index of effects packet
	else if (!memcmp(&header[10], "DSPINDEX", 8))
	{
		// Read the header & footer
		if (memcmp(pBuffer, "DSPINDEX", 8) || memcmp(&pBuffer[size-8], "DSPINDEX", 8)) {
			delete [] pBuffer;
			return FALSE;
		}

		// Read the version (only one version currently supported)
		version = *(BYTE*)&pBuffer[8];
		if (version != 1) {
			delete [] pBuffer;
			return FALSE;
		}

		// Read the number of effects
		count = *(DWORD*)&pBuffer[9];

		pEffectName = new char* [count];
		if (!pEffectName) {
			delete [] pBuffer;
			return FALSE;
		}
		pEffectIndex = new int [count];
		if (!pEffectIndex) {
			delete [] pEffectName;
			delete [] pBuffer;
			return FALSE;
		}

		// Read the effect names and indices
		for (bi=13, c=0; c<count; ++c)
		{
			// Read the effect name
			for(i=0; pBuffer[bi] != 0; ++i, ++bi)
				charBuffer[i] = pBuffer[bi];
			charBuffer[i] = pBuffer[bi++];
			pEffectName[c] = new char [strlen(charBuffer)+1];
			strcpy(pEffectName[c], charBuffer);

			// Read the effect index
			for(i=0; pBuffer[bi] != 0; ++i, ++bi)
				charBuffer[i] = pBuffer[bi];
			charBuffer[i] = pBuffer[bi++];
			pEffectIndex[c] = atoi(charBuffer);
		}

		// Callback
		if (CDSPServer::m_pDownloadCallback)
			CDSPServer::m_pDownloadCallback(pDesc, count, pEffectName, pEffectIndex);

		// Free resources
		for(c=0; c<count; ++c)
			delete [] pEffectName[c];
		delete [] pEffectIndex;
		delete [] pEffectName;
		delete [] pBuffer;
		return TRUE;
	}

#if 0
	// NOT SUPPORTED WITH XACT

	// Effect Parameter packet
	else if (!memcmp(&header[10], "FXPARAMS", 8))
	{

		DWORD testCrc;
		LPCVOID pvData;
		DWORD dwOffset;
		DWORD dwIndex;
		DWORD dwSize;

		// Read the header & footer
		if (memcmp(pBuffer, "FXPARAMS", 8) || memcmp(&pBuffer[size-8], "FXPARAMS", 8)) {
			delete [] pBuffer;
			return FALSE;
		}

		// Read the version (only one version currently supported)
		version = *(BYTE*)&pBuffer[8];
		if (version != 1)
		{
			delete [] pBuffer;
			return FALSE;
		}

		// Read the number of parameters
		count = *(DWORD*)&pBuffer[9];

		// Set all the parameters
		for(bi=13, c=0; c<count; ++c)
		{
			dwIndex		= *(DWORD*)&pBuffer[bi];
			dwOffset	= *(DWORD*)&pBuffer[bi+4];
			dwSize		= *(DWORD*)&pBuffer[bi+8];
			pvData		= (LPCVOID)&pBuffer[bi+12];
			bi			+= dwSize + 12;

			// Set the parameter
			result = CDSPServer::m_pXACT->SetEffectData(dwIndex, dwOffset, pvData, dwSize, DSFX_DEFERRED);

			// Callback
			if (CDSPServer::m_pSetEffectData)
				CDSPServer::m_pSetEffectData(dwIndex, dwOffset, pvData, dwSize, DSFX_DEFERRED);
		}

		// Commit the effect data
		CDSPServer::m_pXACT->CommitEffectData();

		// Cleanup
		delete [] pBuffer;
		return TRUE;
	}
#endif

	// I3DL2 Listener packet
	else if (!memcmp(&header[10], "I3DL2LIS", 8))
	{
		// Read the header & footer
		if (memcmp(pBuffer, "I3DL2LIS", 8) || memcmp(&pBuffer[size-8], "I3DL2LIS", 8))
		{
			delete [] pBuffer;
			return FALSE;
		}

		// Read the version (only one version currently supported)
		version = *(BYTE*)&pBuffer[8];
		if (version != 1)
		{
			delete [] pBuffer;
			return FALSE;
		}

		// Set the data
		bi = 9;
		listener.lRoom					= *(LONG*)&pBuffer[bi];  bi += sizeof(LONG);
		listener.lRoomHF				= *(LONG*)&pBuffer[bi];  bi += sizeof(LONG);
		listener.flRoomRolloffFactor	= *(FLOAT*)&pBuffer[bi]; bi += sizeof(FLOAT);
		listener.flDecayTime			= *(FLOAT*)&pBuffer[bi]; bi += sizeof(FLOAT);
		listener.flDecayHFRatio			= *(FLOAT*)&pBuffer[bi]; bi += sizeof(FLOAT);
		listener.lReflections			= *(LONG*)&pBuffer[bi];  bi += sizeof(LONG);
		listener.flReflectionsDelay		= *(FLOAT*)&pBuffer[bi]; bi += sizeof(FLOAT);
		listener.lReverb				= *(LONG*)&pBuffer[bi];  bi += sizeof(LONG);
		listener.flReverbDelay			= *(FLOAT*)&pBuffer[bi]; bi += sizeof(FLOAT);
		listener.flDiffusion			= *(FLOAT*)&pBuffer[bi]; bi += sizeof(FLOAT);
		listener.flDensity				= *(FLOAT*)&pBuffer[bi]; bi += sizeof(FLOAT);
		listener.flHFReference			= *(FLOAT*)&pBuffer[bi]; bi += sizeof(FLOAT);

		// Apply the changes
		result = CDSPServer::m_pXACT->SetI3dl2Listener(&listener, DS3D_IMMEDIATE);

		// Callback
		if (CDSPServer::m_pSetI3DL2Listener)
			CDSPServer::m_pSetI3DL2Listener(&listener);

		// Cleanup
		delete [] pBuffer;
		return TRUE;
	}

	// Connect packet
	else if (!memcmp(&header[10], "XCONNECT", 8))
	{
		if (CDSPServer::m_pConnectCallback)
			CDSPServer::m_pConnectCallback();
		return TRUE;
	}

	// Disconnect packet
	else if (!memcmp(&header[10], "XDSCNNCT", 8))
	{
		if (CDSPServer::m_pDisconnectCallback)
			CDSPServer::m_pDisconnectCallback();
		return TRUE;
	}

	// This return indicates a failure, most likely a bad block/packet of data
	return FALSE;
}

//------------------------------------------------------------------------------
//	IsDataAvailable
/*++

Routine Description:

	Check to see if data is available

Arguments:

	IN socket -	Socket

Return Value:

	0 if the time limit expired, SOCKET_ERROR on an error or the number of 
	socket handles ready (should be 1)

--*/
//------------------------------------------------------------------------------
static int 
IsDataAvailable(
				IN SOCKET	socket
				)
{
    TIMEVAL	timeout = { (long)CDSPServer::m_uTimeout / 1000L, (long)CDSPServer::m_uTimeout % 1000L * 1000L };
    FD_SET	bucket;

    bucket.fd_count		= 1;
    bucket.fd_array[0]	= socket;

    return select(0, &bucket, NULL, NULL, &timeout);
}


//------------------------------------------------------------------------------
//	GetCRC
//
// Routine Description:
//
//	Generates a simple CRC
//
//Arguments:
//
//	IN pBuffer -	Buffer to generate crc for
//	IN bufferSize -	Size of buffer in bytes
//
//Return Value:
//
//	None
//
//------------------------------------------------------------------------------

static DWORD GetCRC( IN const VOID*	pBuffer, IN ULONG bufferSize )
{
    __asm {
        mov     ecx, pBuffer
		mov		edx, bufferSize
		xor		eax, eax
        xor     ebx, ebx
        shr     edx, 2      // count /= sizeof(ULONG)
        test    edx, edx
        jz      L2
    L1: add     eax, [ecx]  // eax += *data++
        adc     ebx, 0      // ebx += carry
        add     ecx, 4
        dec     edx
        jnz     L1          // while (--count)
    L2: add     eax, ebx    // take care of accumulated carries
        adc     eax, 0
    }
}

//dspserver.cpp