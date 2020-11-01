///////////////////////////////////////////////////////////////////////  
//  SpeedTreeAllocator.h
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) CONFIDENTIAL AND PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization, Inc. and
//  may not be copied, disclosed, or exploited except in accordance with 
//  the terms of that agreement.
//
//      Copyright (c) 2003-2008 IDV, Inc.
//      All rights reserved in all media.
//
//      IDV, Inc.
//      http://www.idvinc.com
//
//  *** Release version 4.2 ***

#pragma once
#ifdef WIN32
#pragma warning (disable : 4786)
#endif

#ifdef SPEEDTREE_MEMORY_STATS
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#endif
#include <cstdio>

// storage-class specification
#if (defined(WIN32) || defined(_XBOX)) && defined(SPEEDTREE_DLL_EXPORTS)
#define ST_STORAGE_CLASS __declspec(dllexport)
#else
#define ST_STORAGE_CLASS
#endif


///////////////////////////////////////////////////////////////////////  
//  class CSpeedTreeAllocator

class ST_STORAGE_CLASS CSpeedTreeAllocator
{
public:
virtual                                     ~CSpeedTreeAllocator( ) { }

virtual void*                               Alloc(size_t BlockSize, size_t Alignment = 16) = 0;
virtual void                                Free(void* pBlock) = 0;

#ifdef SPEEDTREE_MEMORY_STATS
static  void                                TrackAlloc(const char* pDescription, void* pBlock, size_t sAmount);
static  void                                TrackFree(const char* pDescription, void* pBlock, size_t sAmount);
static  bool                                Report(const char* pFilename = NULL, bool bFreeTrackingData = true);

        struct ST_STORAGE_CLASS SAllocStats
        {
            SAllocStats( ) :
                m_siNumAllocates(0),
                m_siAmountAllocated(0),
                m_siNumFrees(0),
                m_siAmountFreed(0)
            {
            }

            bool operator<(const SAllocStats& sRight)
            {
                return sRight.m_siNumAllocates < m_siNumAllocates;
            }

            SAllocStats& operator+=(const SAllocStats& sRight)
            {
                m_siNumAllocates += sRight.m_siNumAllocates;
                m_siAmountAllocated += sRight.m_siAmountAllocated;
                m_siNumFrees += sRight.m_siNumFrees;
                m_siAmountFreed += sRight.m_siAmountFreed;

                return *this;
            }

            std::string     m_strDesc;
            size_t          m_siNumAllocates;
            size_t          m_siAmountAllocated;
            size_t          m_siNumFrees;
            size_t          m_siAmountFreed;
        };

        struct ST_STORAGE_CLASS SLeakStats
        {
            SLeakStats( ) :
                m_nCount(0),
				m_nAmount(0)
            {
            }
			
			bool			operator<(const SLeakStats& cRight)
			{
				return m_nAmount > cRight.m_nAmount; // backwards on purpose
			}

            std::string     m_strDesc;
            int             m_nCount;
			int				m_nAmount;
        };

        typedef std::map<std::string, SAllocStats> stats_map;
		typedef std::map<void*, SLeakStats> leak_map;
#endif // SPEEDTREE_MEMORY_STATS
};
