// xmemory internal header (from <memory>)
#pragma once
#ifndef AALLOC_HPP
#define AALLOC_HPP
#include <cstdlib>
#include <new>
#include <xutility>
#include <malloc.h>

#pragma pack(push,8)
#pragma warning(push,3)

 #pragma warning(disable: 4100)

#ifndef _FARQ	/* specify standard memory model */
 #define _FARQ
 #define _PDFT	ptrdiff_t
 #define _SIZT	size_t
#endif

#ifndef _CPOINTER_X
 #define _CPOINTER_X(T, A)		\
	typename A::_TEMPLATE_MEMBER rebind<T>::other::const_pointer
 #define _CREFERENCE_X(T, A)	\
	typename A::_TEMPLATE_MEMBER rebind<T>::other::const_reference
 #define _POINTER_X(T, A)	\
	typename A::_TEMPLATE_MEMBER rebind<T>::other::pointer
 #define _REFERENCE_X(T, A)	\
	typename A::_TEMPLATE_MEMBER rebind<T>::other::reference
#endif

_STD_BEGIN
		// TEMPLATE FUNCTION _alignedAllocate
template<class _Ty> inline
	_Ty _FARQ *_alignedAllocate(_SIZT _Count, _Ty _FARQ *)
	{	// Allocate storage for _Count elements of type _Ty.
		// Calculate the max number of elements that can be allocated without
		// overflowing the "size_t" parameter of "_aligned_malloc".
		const static _SIZT s_maxCount = (~(size_t)0) / sizeof( _Ty );

		if (_Count > s_maxCount)
		{
			fprintf( stderr, "Error in _alignedAllocate: parameters exceed "
				"maximum allocation size (%lu elements of size %lu)\n",
				_Count, sizeof( _Ty ) );
			return NULL; // memory address precision overflow
		}
		return ((_Ty _FARQ *) _aligned_malloc( _Count * sizeof (_Ty), 16 ));
		//return ((_Ty _FARQ *)operator new(_Count * sizeof (_Ty)));
	}

		// TEMPLATE CLASS aallocator
template<class _Ty>
	class aallocator
	{	// generic allocator for objects of class _Ty
public:
	typedef _SIZT size_type;
	typedef _PDFT difference_type;
	typedef _Ty _FARQ *pointer;
	typedef const _Ty _FARQ *const_pointer;
	typedef _Ty _FARQ& reference;
	typedef const _Ty _FARQ& const_reference;
	typedef _Ty value_type;

	template<class _Other>
		struct rebind
		{	// convert an aallocator<_Ty> to an aallocator <_Other>
		typedef aallocator<_Other> other;
		};

	pointer address(reference _Val) const
		{	// return address of mutable _Val
		return (&_Val);
		}

	const_pointer address(const_reference _Val) const
		{	// return address of nonmutable _Val
		return (&_Val);
		}

	aallocator()
		{	// construct default allocator (do nothing)
		}

	aallocator(const aallocator<_Ty>&)
		{	// construct by copying (do nothing)
		}

	template<class _Other>
		aallocator(const aallocator<_Other>&)
		{	// construct from a related allocator (do nothing)
		}

	template<class _Other>
		aallocator<_Ty>& operator=(const aallocator<_Other>&)
		{	// assign from a related allocator (do nothing)
		return (*this);
		}

	pointer allocate(size_type _Count, const void *)
		{	// allocate array of _Count elements, ignore hint
		return (_alignedAllocate(_Count, (pointer)0));
		}

	pointer allocate(size_type _Count)
		{	// allocate array of _Count elements
		return (_alignedAllocate(_Count, (pointer)0));
		}

	void deallocate(pointer _Ptr, size_type)
		{	// deallocate object at _Ptr, ignore size
		//operator delete(_Ptr);
		_aligned_free(_Ptr);
		}

	void construct(pointer _Ptr, const _Ty& _Val)
		{	// construct object at _Ptr with value _Val
		_Construct(_Ptr, _Val);
		}

	void destroy(pointer _Ptr)
		{	// destroy object at _Ptr
		_Destroy(_Ptr);
		}

	_SIZT max_size() const
		{	// estimate maximum array size
		_SIZT _Count = (_SIZT)(-1) / sizeof (_Ty);
		return (0 < _Count ? _Count : 1);
		}
	};

		// allocator TEMPLATE OPERATORS
template<class _Ty,
	class _Other> inline
	bool operator==(const aallocator<_Ty>&, const aallocator<_Other>&)
	{	// test for allocator equality (always true)
	return (true);
	}

template<class _Ty,
	class _Other> inline
	bool operator!=(const aallocator<_Ty>&, const aallocator<_Other>&)
	{	// test for allocator inequality (always false)
	return (false);
	}

		// CLASS allocator<void>
template<> class _CRTIMP2 aallocator<void>
	{	// generic allocator for type void
public:
	typedef void _Ty;
	typedef _Ty _FARQ *pointer;
	typedef const _Ty _FARQ *const_pointer;
	typedef _Ty value_type;

	template<class _Other>
		struct rebind
		{	// convert an allocator<void> to an allocator <_Other>
		typedef aallocator<_Other> other;
		};

	aallocator()
		{	// construct default allocator (do nothing)
		}

	aallocator(const aallocator<_Ty>&)
		{	// construct by copying (do nothing)
		}

	template<class _Other>
		aallocator(const aallocator<_Other>&)
		{	// construct from related allocator (do nothing)
		}

	template<class _Other>
		aallocator<_Ty>& operator=(const aallocator<_Other>&)
		{	// assign from a related allocator (do nothing)
		return (*this);
		}
	};

_STD_END

  #pragma warning(default: 4100)
#pragma warning(pop)
#pragma pack(pop)

#endif /* AALLOC_HPP */

/*
 * Copyright (c) 1992-2001 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
 */

/*
 * This file is derived from software bearing the following
 * restrictions:
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this
 * software and its documentation for any purpose is hereby
 * granted without fee, provided that the above copyright notice
 * appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation.
 * Hewlett-Packard Company makes no representations about the
 * suitability of this software for any purpose. It is provided
 * "as is" without express or implied warranty.
 V3.10:0009 */
