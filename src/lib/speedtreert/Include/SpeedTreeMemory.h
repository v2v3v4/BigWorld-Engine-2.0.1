///////////////////////////////////////////////////////////////////////  
//  SpeedTreeMemory.h
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

#if defined(STRIP_EXCEPTION_HANDLING) && defined(WIN32)
#pragma warning (disable : 4530)
#endif

#include "SpeedTreeAllocator.h"
#include <functional>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#ifdef PS3
#include <stdlib.h>
#include <stddef.h>
#endif

#ifndef SPEEDTREE_SOURCE_FILE	   // any file that's not the one holding the actual definitions of these variables
	#ifdef USING_SPEEDTREE_AS_DLL // should only be used in Windows applications using the DLL version, not static lib
		__declspec(dllimport) CSpeedTreeAllocator* g_pAllocator;
		__declspec(dllimport) size_t g_sHeapMemoryUsed;
	#else
		extern CSpeedTreeAllocator* g_pAllocator;
		extern size_t g_sHeapMemoryUsed;
	#endif
#else // SPEEDTREE_SOURCE_FILE
	ST_STORAGE_CLASS CSpeedTreeAllocator *g_pAllocator = NULL;
	ST_STORAGE_CLASS size_t g_sHeapMemoryUsed = 0;
#endif // SPEEDTREE_SOURCE_FILE


#ifdef STRIP_EXCEPTION_HANDLING
#include <stdio.h>
#define throw(a) printf("ERROR - throw not sent because exceptions are disabled\n")
#endif


///////////////////////////////////////////////////////////////////////
//  struct SHeapHandle
//
//  Necessary overhead for allocating arrays manually.

struct SHeapHandle
{
    size_t  m_sNumElements;
};


///////////////////////////////////////////////////////////////////////
//  placement new

inline void* operator new(size_t sBlockSize, const char* pDescription)
{
    void* pBlock = g_pAllocator ? g_pAllocator->Alloc(sBlockSize) : malloc(sBlockSize);
    if (!pBlock)
      throw(std::bad_alloc( )); // ANSI/ISO compliant behavior

	// track all SpeedTree memory allocation
	g_sHeapMemoryUsed += sBlockSize;
#ifdef SPEEDTREE_MEMORY_STATS
    CSpeedTreeAllocator::TrackAlloc(pDescription ? pDescription : "Unknown", pBlock, sBlockSize);
#else
    pDescription = pDescription; // avoid unreferenced parameter warning
#endif

    return pBlock;
}


///////////////////////////////////////////////////////////////////////
//  must have a delete call to match the placement new

inline void operator delete(void* pBlock, const char*)
{
    if (pBlock)
    {
		if (g_pAllocator)
            g_pAllocator->Free(pBlock);
        else
            free(pBlock);
    }
}


///////////////////////////////////////////////////////////////////////
//  st_new_array (equivalent of C++ new[])

//lint -save -e429
template <typename TYPE> inline TYPE* st_new_array(size_t sNumElements, const char* pDescription)
{
    size_t sTotalSize = sizeof(SHeapHandle) + sNumElements * sizeof(TYPE);

	char* pRawBlock = (char*) (g_pAllocator ? g_pAllocator->Alloc(sTotalSize) : malloc(sTotalSize));
    if (pRawBlock)
    {
        // get the main part of the data, after the heap handle
        TYPE* pBlock = (TYPE*) (pRawBlock + sizeof(SHeapHandle));

        // record the number of elements
        *((size_t*) pRawBlock) = sNumElements;

        // manually call the constructors using placement new
        TYPE* pElements = pBlock;
        for (size_t i = 0; i < sNumElements; ++i)
			(void) new ((void*) (pElements + i)) TYPE;

		// track all SpeedTree memory allocation
		g_sHeapMemoryUsed += sTotalSize;
#ifdef SPEEDTREE_MEMORY_STATS
        CSpeedTreeAllocator::TrackAlloc(pDescription ? pDescription : "Unknown", pRawBlock, sTotalSize);
#else
        pDescription = pDescription; // avoid unreferenced parameter warning
#endif

        return pBlock;
    }
    else
	    throw(std::bad_alloc( )); // ANSI/ISO compliant behavior
	
#ifdef STRIP_EXCEPTION_HANDLING
	return NULL;
#endif
}
//lint -restore


///////////////////////////////////////////////////////////////////////
//  st_delete (equivalent of C++ delete)

//lint -save -e530
template <typename TYPE> inline void st_delete(TYPE*& pBlock, const char* pDescription)
{
#ifdef SPEEDTREE_MEMORY_STATS
	if (pBlock)
		CSpeedTreeAllocator::TrackFree(pDescription ? pDescription : "Unknown", pBlock, pBlock ? sizeof(TYPE) : 0);
#else
    pDescription = pDescription; // avoid unreferenced parameter warning
#endif

    if (pBlock)
    {
        pBlock->~TYPE( );
        if (g_pAllocator)
            g_pAllocator->Free(pBlock);
        else
            free(pBlock);
        pBlock = NULL;

		// track all SpeedTree memory allocation
		g_sHeapMemoryUsed -= sizeof(TYPE);
    }
}
//lint -restore


///////////////////////////////////////////////////////////////////////
//  st_delete_array (equivalent of C++ delete[])

template <typename TYPE> inline void st_delete_array(TYPE*& pRawBlock, const char* pDescription)
{
    if (pRawBlock)
    {
        // extract the array size
        SHeapHandle* pHeapHandle = (SHeapHandle*) ( ((char*) pRawBlock) - sizeof(SHeapHandle) );

        if (pHeapHandle)
        {
            // point to the elements
            TYPE* pElements = pRawBlock; 

            if (pElements)
            {
				// track all SpeedTree memory allocation
				g_sHeapMemoryUsed -= sizeof(SHeapHandle) + pHeapHandle->m_sNumElements * sizeof(TYPE);

				for (size_t i = 0; i < pHeapHandle->m_sNumElements; ++i)
                    (pElements + i)->~TYPE( );

#ifdef SPEEDTREE_MEMORY_STATS
                CSpeedTreeAllocator::TrackFree(pDescription ? pDescription : "Unknown", pHeapHandle, pHeapHandle->m_sNumElements * sizeof(TYPE) + sizeof(SHeapHandle));
#else
                pDescription = pDescription; // avoid unreferenced parameter warning
#endif
                if (g_pAllocator)
                    g_pAllocator->Free(pHeapHandle);
                else
                    free(pHeapHandle);
                pRawBlock = NULL;
            }
        }
    }
}


#ifndef IDV_VISUALC_6

///////////////////////////////////////////////////////////////////////
//  STL allocator that uses our heap allocator
//
//	Nearly 100% of this code was taken directly from code posted by 
//	Pete Isensee on his site: http://www.tantalon.com/pete.htm

#ifdef SPEEDTREE_MEMORY_STATS

#define DefineAllocator(NAME)								\
template<typename TYPE>										\
class NAME													\
{															\
public:														\
	typedef _SIZT size_type;								\
	typedef _PDFT difference_type;							\
	typedef TYPE _FARQ *pointer;							\
	typedef const TYPE _FARQ *const_pointer;				\
	typedef TYPE _FARQ& reference;							\
	typedef const TYPE _FARQ& const_reference;				\
	typedef TYPE value_type;								\
															\
	template<typename _Other>								\
	struct rebind											\
	{														\
		typedef NAME<_Other> other;							\
	};														\
															\
	pointer address(reference _Val) const					\
	{														\
		return (&_Val);										\
	}														\
															\
	const_pointer address(const_reference _Val) const		\
	{														\
		return (&_Val);										\
	}														\
															\
	NAME()													\
	{														\
	}														\
															\
	NAME(const NAME<TYPE>&)									\
	{														\
	}														\
															\
	template<typename _Other>								\
		NAME(const NAME<_Other>&)							\
	{														\
	}														\
															\
	template<typename _Other>								\
		NAME<TYPE>& operator=(const NAME<_Other>&)			\
	{														\
		return (*this);										\
	}														\
	bool operator==(NAME const&) const { return true; }		\
	bool operator!=(NAME const&) const { return false; }    \
	pointer allocate(size_type _Count, const void *)		\
	{														\
		void* pBlock = g_pAllocator ?						\
			g_pAllocator->Alloc(_Count * sizeof(TYPE)) :	\
			malloc(_Count * sizeof(TYPE));					\
		g_sHeapMemoryUsed += _Count * sizeof(TYPE);				\
		CSpeedTreeAllocator::TrackAlloc(#NAME,				\
			pBlock, _Count * sizeof(TYPE));					\
		return pointer(pBlock);								\
	}														\
	pointer allocate(size_type _Count)						\
	{														\
		void* pBlock = g_pAllocator ?						\
			g_pAllocator->Alloc(_Count * sizeof(TYPE)) :	\
			malloc(_Count * sizeof(TYPE));					\
		g_sHeapMemoryUsed += _Count * sizeof(TYPE);				\
		CSpeedTreeAllocator::TrackAlloc(#NAME,				\
			pBlock, _Count * sizeof(TYPE));					\
		return pointer(pBlock);								\
	}														\
	void deallocate(pointer _Ptr, size_type _Count)			\
	{														\
		if (_Ptr) {											\
			CSpeedTreeAllocator::TrackFree(#NAME,			\
				_Ptr, _Count * sizeof(TYPE));				\
			g_sHeapMemoryUsed -= _Count * sizeof(TYPE);			\
			if (g_pAllocator)								\
				g_pAllocator->Free(_Ptr);					\
			else											\
				free(_Ptr); }								\
	}														\
															\
	void construct(pointer _Ptr, const TYPE& _Val)			\
	{														\
		std::_Construct(_Ptr, _Val);						\
	}														\
															\
	void destroy(pointer _Ptr)								\
	{														\
		std::_Destroy(_Ptr);								\
	}														\
															\
	_SIZT max_size() const									\
	{														\
		_SIZT _Count = (_SIZT)(-1) / sizeof (TYPE);			\
		return (0 < _Count ? _Count : 1);					\
	}														\
};

#else // SPEEDTREE_MEMORY_STATS

#define DefineAllocator(NAME)								\
template<typename TYPE>										\
class NAME													\
{															\
public:														\
	typedef _SIZT size_type;								\
	typedef _PDFT difference_type;							\
	typedef TYPE _FARQ *pointer;							\
	typedef const TYPE _FARQ *const_pointer;				\
	typedef TYPE _FARQ& reference;							\
	typedef const TYPE _FARQ& const_reference;				\
	typedef TYPE value_type;								\
															\
	template<typename _Other>								\
	struct rebind											\
	{														\
		typedef NAME<_Other> other;							\
	};														\
															\
	pointer address(reference _Val) 						\
	{														\
		return (&_Val);										\
	}														\
															\
	const_pointer address(const_reference _Val)				\
	{														\
		return (&_Val);										\
	}														\
															\
	NAME()													\
	{														\
	}														\
															\
	NAME(const NAME<TYPE>&)									\
	{														\
	}														\
															\
	template<typename _Other>								\
		NAME(const NAME<_Other>&)							\
	{														\
	}														\
															\
	template<typename _Other>								\
		NAME<TYPE>& operator=(const NAME<_Other>&)			\
	{														\
		return (*this);										\
	}														\
	bool operator==(NAME const&) const { return true; }		\
	bool operator!=(NAME const&) const { return false; }    \
	pointer allocate(size_type _Count, const void *)		\
	{														\
		void* pBlock = g_pAllocator ?						\
			g_pAllocator->Alloc(_Count * sizeof(TYPE)) :	\
			malloc(_Count * sizeof(TYPE));					\
		g_sHeapMemoryUsed += _Count * sizeof(TYPE);				\
		return pointer(pBlock);								\
	}														\
															\
	pointer allocate(size_type _Count)						\
	{														\
		void* pBlock = g_pAllocator ?						\
			g_pAllocator->Alloc(_Count * sizeof(TYPE)) :	\
			malloc(_Count * sizeof(TYPE));					\
		g_sHeapMemoryUsed += _Count * sizeof(TYPE);				\
		return pointer(pBlock);								\
	}														\
															\
	void deallocate(pointer _Ptr, size_type _Count)			\
	{														\
		if (_Ptr) {											\
			g_sHeapMemoryUsed -= _Count * sizeof(TYPE);			\
			if (g_pAllocator)								\
				g_pAllocator->Free(_Ptr);					\
			else											\
				free(_Ptr); }								\
	}														\
															\
	void construct(pointer _Ptr, const TYPE& _Val)			\
	{														\
		std::_Construct(_Ptr, _Val);						\
	}														\
															\
	void destroy(pointer _Ptr)								\
	{														\
		std::_Destroy(_Ptr);								\
	}														\
															\
	_SIZT max_size() const									\
	{														\
		_SIZT _Count = (_SIZT)(-1) / sizeof (TYPE);			\
		return (0 < _Count ? _Count : 1);					\
	}														\
};
#endif // SPEEDTREE_MEMORY_STATS

#else // IDV_VISUALC_6

///////////////////////////////////////////////////////////////////////
//  STL allocator that uses our heap allocator

#ifdef SPEEDTREE_MEMORY_STATS
#define DefineAllocator(NAME) \
    template <typename TYPE> class NAME \
{ \
public: \
    typedef TYPE*          pointer; \
    typedef TYPE const*    const_pointer; \
    typedef TYPE&          reference; \
    typedef TYPE const&    const_reference; \
    typedef TYPE           value_type; \
    typedef size_t         size_type; \
    typedef ptrdiff_t      difference_type; \
    \
    template<typename U> struct rebind \
{ \
    typedef NAME<U> other; \
}; \
    \
    virtual TYPE* allocate(size_t nobjs, void* hint = 0) \
{ \
    hint = hint; \
    void* pBlock = g_pAllocator ? g_pAllocator->Alloc(nobjs * sizeof(TYPE)) : malloc(nobjs * sizeof(TYPE)); \
    CSpeedTreeAllocator::TrackAlloc(#NAME, pBlock, nobjs * sizeof(TYPE)); \
    return (TYPE*) pBlock; \
} \
    \
    void deallocate(TYPE* ptr, size_t count) \
{ \
    if (ptr) { \
		CSpeedTreeAllocator::TrackFree(#NAME, ptr, count * sizeof(TYPE)); \
		g_sHeapMemoryUsed -= count * sizeof(TYPE);			\
		if (g_pAllocator) \
            g_pAllocator->Free(ptr); \
        else \
            free(ptr); } \
} \
    \
    NAME( ) { } \
    virtual ~NAME( ) { } \
    \
    bool operator==(NAME const&) const { return true; } \
    bool operator!=(NAME const&) const { return false; } \
    \
    size_t      max_size(void) const { return static_cast<size_t>(-1); } \
    TYPE*       address(TYPE& t ) { return &t; } \
    TYPE const* address(TYPE const& t ) { return &t; } \
    void        construct(TYPE* p, TYPE const& t ) { new(p) TYPE(t); } \
    void        destroy(TYPE* p ) { p->~TYPE(); } \
}

#else // SPEEDTREE_MEMORY_STATS

#define DefineAllocator(NAME) \
    template <typename TYPE> class NAME \
{ \
public: \
    typedef TYPE*          pointer; \
    typedef TYPE const*    const_pointer; \
    typedef TYPE&          reference; \
    typedef TYPE const&    const_reference; \
    typedef TYPE           value_type; \
    typedef size_t         size_type; \
    typedef ptrdiff_t      difference_type; \
    \
    template<typename U> struct rebind \
{ \
    typedef NAME<U> other; \
}; \
    \
    virtual TYPE* allocate(size_t nobjs, void* hint = 0) \
{ \
    hint = hint; \
    void* pBlock = g_pAllocator ? g_pAllocator->Alloc(nobjs * sizeof(TYPE)) : malloc(nobjs * sizeof(TYPE)); \
    return (TYPE*) pBlock; \
} \
    \
    void deallocate(TYPE* ptr, size_t _Count) \
{ \
    if (ptr) { \
	g_sHeapMemoryUsed -= _Count * sizeof(TYPE); \
    if (g_pAllocator) \
        g_pAllocator->Free(ptr); \
    else \
        free(ptr); } \
} \
    \
    NAME( ) { } \
    virtual ~NAME( ) { } \
    \
    bool operator==(NAME const&) const { return true; } \
    bool operator!=(NAME const&) const { return false; } \
    \
    size_t      max_size(void) const { return static_cast<size_t>(-1); } \
    TYPE*       address(TYPE& t ) { return &t; } \
    TYPE const* address(TYPE const& t ) { return &t; } \
    void        construct(TYPE* p, TYPE const& t ) { new(p) TYPE(t); } \
    void        destroy(TYPE* p ) { p->~TYPE(); } \
}
#endif // SPEEDTREE_MEMORY_STATS

#endif // IDV_VISUALC_6


///////////////////////////////////////////////////////////////////////  
//  Commonly used STL types using SpeedTree allocator class

// forward references
class stVec;
class stVec3;
class CBillboardLeaf;
class CBranch;
struct SIdvBranchInfo;
struct SIdvBranch;
struct SFrondGuide;
struct SFrondTexture;
struct SFrondVertex;
struct SShape;
class CSpeedTreeRT;
struct SIdvLeafTexture;
struct SIdvBranchFlare;
struct SIdvBranchVertex;

// convenience typedefs & associated allocators
typedef unsigned char byte;
DefineAllocator(st_allocator); // default allocator

// define vector<float>
DefineAllocator(st_allocator_float);
typedef std::vector<float, st_allocator_float<float> > st_vector_float;

// define vector<const float*>
DefineAllocator(st_allocator_cfloat_p);
typedef std::vector<const float *, st_allocator_cfloat_p<const float*> > st_vector_cfloat_p;

// define vector<int>
DefineAllocator(st_allocator_int);
typedef std::vector<int, st_allocator_int<int> > st_vector_int;

// define vector<int*>
DefineAllocator(st_allocator_int_p);
typedef std::vector<int*, st_allocator_int<int*> > st_vector_int_p;

// define vector<bool>
DefineAllocator(st_allocator_bool);
typedef std::vector<bool, st_allocator_bool<bool> > st_vector_bool;

// define vector<byte>
DefineAllocator(st_allocator_byte);
typedef std::vector<byte, st_allocator_byte<byte> > st_vector_byte;

// define vector<char>
DefineAllocator(st_allocator_char);
typedef std::vector<char, st_allocator_char<char> > st_vector_char;

// define vector<const void*>
DefineAllocator(st_allocator_cvoid_p);
typedef std::vector<const void*, st_allocator_cvoid_p<const void*> > st_vector_cvoid_p;

// define vector<long>
DefineAllocator(st_allocator_long);
typedef std::vector<long, st_allocator_long<long> > st_vector_long;

// define vector<unsigned short>
DefineAllocator(st_allocator_ushort);
typedef std::vector<unsigned short, st_allocator_ushort<unsigned short> > st_vector_ushort;

// define vector<unsigned short*>
DefineAllocator(st_allocator_ushort_p);
typedef std::vector<unsigned short*, st_allocator_ushort<unsigned short*> > st_vector_ushort_p;

// define vector<unsigned int>
DefineAllocator(st_allocator_uint);
typedef std::vector<unsigned int, st_allocator_uint<unsigned int> > st_vector_uint;

// define vector<st_vector_ushort>
DefineAllocator(st_allocator_ushort_vector);
typedef std::vector<st_vector_ushort, st_allocator_ushort_vector<st_vector_ushort> > st_vector_ushort_vector;

// define vector<st_vector_ushort_p>
DefineAllocator(st_allocator_ushort_p_vector);
typedef std::vector<st_vector_ushort_p, st_allocator_ushort_p_vector<st_vector_ushort_p> > st_vector_ushort_p_vector;

// define vector<st_vector_float>
DefineAllocator(st_allocator_float_vector);
typedef std::vector<st_vector_float, st_allocator_float_vector<st_vector_float> > st_vector_float_vector;

// define vector<st_vector_int>
DefineAllocator(st_allocator_int_vector);
typedef std::vector<st_vector_int, st_allocator_int_vector<st_vector_int> > st_vector_int_vector;

// define vector<st_vector_int_p>
DefineAllocator(st_allocator_int_p_vector);
typedef std::vector<st_vector_int_p, st_allocator_int_p_vector<st_vector_int_p> > st_vector_int_p_vector;

// std::string heap use is different among STL libraries, change based on your library's behavior
#if defined(_XBOX) || defined(PS3)
DefineAllocator(st_allocator_string);
typedef std::basic_string<char, std::char_traits<char>, st_allocator_string<char> > st_string;
DefineAllocator(st_allocator_stringstream);
typedef std::basic_stringstream<char, std::char_traits<char>, st_allocator_stringstream<char> > st_stringstream;
#else // PC
typedef std::string st_string;
typedef std::stringstream st_stringstream;
#endif

// define vector<st_string>
DefineAllocator(st_allocator_string_vector);
typedef std::vector<st_string, st_allocator_string_vector<st_string> > st_vector_string;

// define vector<CSpeedTreeRT*>
DefineAllocator(st_allocator_speedtree_p_vector);
typedef std::vector<CSpeedTreeRT*, st_allocator_speedtree_p_vector<CSpeedTreeRT*> > st_vector_speedtree_p;


