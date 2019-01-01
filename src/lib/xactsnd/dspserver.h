/*++

Copyright (c) Microsoft Corporation. All rights reserved.

Module Name:

	dspserver.h

Abstract:

	DSP Server code. This module will run as a thread, and allow
	connections from dspbuilder running on a PC to modify the GP DSP image

	If another call to XNetStartup is made before this class is 
	instantiated, you must bypass security by setting the flag
	XNET_STARTUP_BYPASS_SECURITY in the call.

	To use this class:

	1. Instantiate an instance of CDSPServer
	2. Call CDSPServer::Run()

	When you are done, just destroy it.

Revision History:

	20-Jun-2001 Extensive modifications for dsp server 

--*/

#ifndef __DSPSERVER_H__
#define __DSPSERVER_H__

//------------------------------------------------------------------------------
//	Includes:
//------------------------------------------------------------------------------
//#include <xtl.h>
#include <process.h>
#include <xact.h>

//------------------------------------------------------------------------------
//	CDSPServer
//------------------------------------------------------------------------------
class CDSPServer
{
public:

	static BOOL Run(IN LPXACTENGINE	pXACTEngine				= NULL, 
					IN USHORT			usPort				= 80, 
					IN unsigned			uTimeout			= 1000,
					IN void (__cdecl *pPreCallback)(void)	= NULL,
					IN void (__cdecl *pCallback)(LPDSEFFECTIMAGEDESC pImageDes,
												 int count, char** pEffectNames, 
												 int* pEffectIndices) = NULL,
					IN void (__cdecl *pSetEffectData)(DWORD dwIndex, 
													  DWORD dwOffset,
													  LPCVOID pData, 
													  DWORD dwSize, 
													  DWORD dwFlags) = NULL,
					IN void (__cdecl *pSetI3DL2Listener)(LPDSI3DL2LISTENER pData) = NULL,
					IN void (__cdecl *pConnectCallback)(void) = NULL,
					IN void (__cdecl *pDisconnectCallback)(void) = NULL);
    /*--------------------------------------------------------------------------
	Routine Description:

        Creates the socket, binds it and runs. Also creates a Direct Sound 
		object if it needs to.

    Arguments:

        IN pDirectSound -	Direct sound object to use for DSP changes. If NULL,
							One will be created.
		IN usPort -			Port to use for connection to dspbuilder. If a value 
							other than the default is used, you will need to set
							that value in dspbuilder as well
		IN uTimeout -		Timeout in millesconds
        IN pPreCallback -	If not NULL, called before a new effects image is
							downloaded
        IN pCallback -		If not NULL, called after a new effects image is
							dowloaded
						IN pImageDes -		Pointer to the DSEFFECTIMAGEDESC 
											structure that is returned by the 
											method, which describes the DSP 
											scratch image. 
						IN count -			Number of effects in image
						IN pEffectNames -	Array of effect name strings 
											(each null terminated)
						IN pEffectIndices -	Array of effect indices
		IN pSetEffectData -	Called after an effect parameter is changed. This
							callback is made with the same paramaters made to 
							the call to DSound::SetEffectData()
						IN dwIndex -	Effect index
						IN dwOffset -	Offset into pData
						IN pData -		Effect data 
						IN dwSize -		Size of pData 
						IN dwFlags -	Flags
		IN pSetI3DL2Listener -	Called after a call to DSound::SetI3DL2Listener
								with the same parameters
		IN pConnectCallback - Called when dspbuilder connects
		IN pDisconnectCallback - Called when dspbuilder disconnects

    Return Value:

        TRUE on success, FALSE on failure.

    --------------------------------------------------------------------------*/
	
	static void Stop(void);
    /*--------------------------------------------------------------------------
	Routine Description:

        Stops the thread, and closes the socket.

    Arguments:

        None

    Return Value:

        None

    --------------------------------------------------------------------------*/

public:

	static BOOL				m_threadAlive;	// FALSE to kill thread
	static SOCKET			m_socket;		// Socket used to "listen" for connections
	static USHORT			m_port;			// Port of our current server
	static unsigned			m_uTimeout;		// idle timeout length
	static LPXACTENGINE		m_pXACT;		// XACT engine object
	static HANDLE			m_thread;		// Thread handle
	static void (__cdecl *m_pPreDownloadCallback)(void);
	static void (__cdecl *m_pDownloadCallback)(LPDSEFFECTIMAGEDESC pImageDes,
										       int count, char** pEffectNames, 
											   int* pEffectIndices);
	static void (__cdecl *m_pSetEffectData)(DWORD dwIndex, DWORD dwOffset,
										    LPCVOID pData, DWORD dwSize, 
											DWORD dwFlags);
	static void (__cdecl *m_pSetI3DL2Listener)(LPDSI3DL2LISTENER pData);
	static void (__cdecl *m_pConnectCallback)(void);
	static void (__cdecl *m_pDisconnectCallback)(void);
};

#endif
