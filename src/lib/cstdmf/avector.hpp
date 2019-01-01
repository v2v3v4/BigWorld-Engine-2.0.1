// avector standard header
#ifndef AVECTOR_HPP
#define AVECTOR_HPP

#if !defined( _WIN32 )

// WIN32PORT the Win32 server used the DirectX Matrix Libraries which requires Aligned Vectors
#define avector vector

#else
#pragma once
#include <memory>
#include <stdexcept>
#include <vector>

#include "aalloc.hpp"
#include "memory_counter.hpp"

#pragma pack(push,8)
#pragma warning(push,3)

 #pragma warning(disable: 4244)

_STD_BEGIN
template<class _Ty,
	class _Diff,
	class _Pointer,
	class _Reference>
	struct A_Ranit
		: public _Iterator_base

	{	// base type for container random-access iterator classes
	typedef random_access_iterator_tag iterator_category;
	typedef _Ty value_type;
	typedef _Diff difference_type;
	typedef _Diff distance_type;	// retained
	typedef _Pointer pointer;
	typedef _Reference reference;
	};

		// TEMPLATE CLASS _AVector_val
template<class _Ty, class _Alloc>
	class _AVector_val
	{	// base class for avector to hold allocator _Alval
protected:
	_AVector_val(_Alloc _Al = _Alloc())
		: _Alval(_Al)
		{	// construct allocator from _Al
		}

	typedef typename _Alloc::template
		rebind<_Ty>::other _Alty;

	_Alty _Alval;	// allocator object for values
	};

		// TEMPLATE CLASS avector
template<class _Ty,
	class _Ax = aallocator<_Ty> >
	class avector
		: public _AVector_val<_Ty, _Ax>
	{	// varying size array of values
public:
	typedef avector<_Ty, _Ax> _Myt;
	typedef _AVector_val<_Ty, _Ax> _Mybase;
	typedef typename _Mybase::_Alty _Alloc;
	typedef _Alloc allocator_type;
	typedef typename _Alloc::size_type size_type;
	typedef typename _Alloc::difference_type difference_type;
	typedef typename _Alloc::difference_type _Dift;
	typedef typename _Alloc::pointer _Tptr;
	typedef typename _Alloc::const_pointer _Ctptr;
	typedef _Tptr pointer;
	typedef _Ctptr const_pointer;
	typedef typename _Alloc::reference _Reft;
	typedef typename _Alloc::reference reference;
	typedef typename _Alloc::const_reference const_reference;
	typedef typename _Alloc::value_type value_type;


#define _AVECTOR_ITER_BASE(it)	(it)._Myptr
		// CLASS const_iterator
	class const_iterator;
	friend class const_iterator;

	class const_iterator
		: public A_Ranit<_Ty, _Dift, _Ctptr, const_reference>
		{	// iterator for nonmutable vector
	public:
		typedef random_access_iterator_tag iterator_category;
		typedef _Ty value_type;
		typedef _Dift difference_type;
		typedef _Ctptr pointer;
		typedef const_reference reference;

		const_iterator()
			{	// construct with null pointer
			_Myptr = 0;
			}

		const_iterator(_Tptr _Ptr)
			{	// construct with pointer _Ptr
			_Myptr = _Ptr;
			}

		const_reference operator*() const
			{	// return designated object


			return (*_Myptr);
			}

		_Ctptr operator->() const
			{	// return pointer to class object
			return (&**this);
			}

		const_iterator& operator++()
			{	// preincrement
			++_Myptr;
			return (*this);
			}

		const_iterator operator++(int)
			{	// postincrement
			const_iterator _Tmp = *this;
			++*this;
			return (_Tmp);
			}

		const_iterator& operator--()
			{	// predecrement
			--_Myptr;
			return (*this);
			}

		const_iterator operator--(int)
			{	// postdecrement
			const_iterator _Tmp = *this;
			--*this;
			return (_Tmp);
			}

		const_iterator& operator+=(difference_type _Off)
			{	// increment by integer
			_Myptr += _Off;
			return (*this);
			}

		const_iterator operator+(difference_type _Off) const
			{	// return this + integer
			const_iterator _Tmp = *this;
			return (_Tmp += _Off);
			}

		const_iterator& operator-=(difference_type _Off)
			{	// decrement by integer
			return (*this += -_Off);
			}

		const_iterator operator-(difference_type _Off) const
			{	// return this - integer
			const_iterator _Tmp = *this;
			return (_Tmp -= _Off);
			}

		difference_type operator-(const const_iterator& _Right) const
			{	// return difference of iterators


			return (_Myptr - _Right._Myptr);
			}

		const_reference operator[](difference_type _Off) const
			{	// subscript
			return (*(*this + _Off));
			}

		bool operator==(const const_iterator& _Right) const
			{	// test for iterator equality


			return (_Myptr == _Right._Myptr);
			}

		bool operator!=(const const_iterator& _Right) const
			{	// test for iterator inequality
			return (!(*this == _Right));
			}

		bool operator<(const const_iterator& _Right) const
			{	// test if this < _Right


			return (_Myptr < _Right._Myptr);
			}

		bool operator>(const const_iterator& _Right) const
			{	// test if this > _Right
			return (_Right < *this);
			}

		bool operator<=(const const_iterator& _Right) const
			{	// test if this <= _Right
			return (!(_Right < *this));
			}

		bool operator>=(const const_iterator& _Right) const
			{	// test if this >= _Right
			return (!(*this < _Right));
			}

		friend const_iterator operator+(difference_type _Off,
			const const_iterator& _Right)
			{	// return iterator + integer
			return (_Right + _Off);
			}


		_Tptr _Myptr;	// offset of element in vector
		};

		// CLASS iterator
	class iterator;
	friend class iterator;

	class iterator
		: public const_iterator
		{	// iterator for mutable vector
	public:
		typedef random_access_iterator_tag iterator_category;
		typedef _Ty value_type;
		typedef _Dift difference_type;
		typedef _Tptr pointer;
		typedef _Reft reference;

		iterator()
			{	// construct with null vector pointer
			}

		iterator(pointer _Ptr)
			: const_iterator(_Ptr)
			{	// construct with pointer _Ptr
			}

		reference operator*() const
			{	// return designated object
			return ((reference)**(const_iterator *)this);
			}

		_Tptr operator->() const
			{	// return pointer to class object
			return (&**this);
			}

		iterator& operator++()
			{	// preincrement
			++this->_Myptr;
			return (*this);
			}

		iterator operator++(int)
			{	// postincrement
			iterator _Tmp = *this;
			++*this;
			return (_Tmp);
			}

		iterator& operator--()
			{	// predecrement
			--this->_Myptr;
			return (*this);
			}

		iterator operator--(int)
			{	// postdecrement
			iterator _Tmp = *this;
			--*this;
			return (_Tmp);
			}

		iterator& operator+=(difference_type _Off)
			{	// increment by integer
			this->_Myptr += _Off;
			return (*this);
			}

		iterator operator+(difference_type _Off) const
			{	// return this + integer
			iterator _Tmp = *this;
			return (_Tmp += _Off);
			}

		iterator& operator-=(difference_type _Off)
			{	// decrement by integer
			return (*this += -_Off);
			}

		iterator operator-(difference_type _Off) const
			{	// return this - integer
			iterator _Tmp = *this;
			return (_Tmp -= _Off);
			}

		difference_type operator-(const const_iterator& _Right) const
			{	// return difference of iterators
			return ((const_iterator)*this - _Right);
			}

		reference operator[](difference_type _Off) const
			{	// subscript
			return (*(*this + _Off));
			}

		friend iterator operator+(difference_type _Off,
			const iterator& _Right)
			{	// return iterator + integer
			return (_Right + _Off);
			}
		};


	typedef std::reverse_iterator<iterator>
		reverse_iterator;
	typedef std::reverse_iterator<const_iterator>
		const_reverse_iterator;

	avector()
		: _Mybase()
		{	// construct empty avector
		_Buy(0);
		}

	explicit avector(const _Alloc& _Al)
		: _Mybase(_Al)
		{	// construct empty avector with allocator
		_Buy(0);
		}

	explicit avector(size_type _Count)
		: _Mybase()
		{	// construct from _Count * _Ty()
		_Ty _Val = _Ty();	// may throw
		if (_Buy(_Count))
			_Construct_n(_Count, _Val);
		}

	avector(size_type _Count, const _Ty& _Val)
		: _Mybase()
		{	// construct from _Count * _Val
		if (_Buy(_Count))
			_Construct_n(_Count, _Val);
		}

	avector(size_type _Count, const _Ty& _Val, const _Alloc& _Al)
		: _Mybase(_Al)
		{	// construct from _Count * _Val, with allocator
		if (_Buy(_Count))
			_Construct_n(_Count, _Val);
		}

	avector(const _Myt& _Right)
		: _Mybase(_Right._Alval)
		{	// construct by copying _Right
		if (_Buy(_Right.size()))
			_TRY_BEGIN
			_Mylast = _Ucopy(_Right.begin(), _Right.end(), _Myfirst);
			_CATCH_ALL
			_Tidy();
			_RERAISE;
			_CATCH_END
		}

	template<class _Iter>
		avector(_Iter _First, _Iter _Last)
		: _Mybase()
		{	// construct from [_First, _Last)
		_Construct(_First, _Last, _Iter_cat(_First));
		}

	template<class _Iter>
		avector(_Iter _First, _Iter _Last, const _Alloc& _Al)
		: _Mybase(_Al)
		{	// construct from [_First, _Last), with allocator
		_Construct(_First, _Last, _Iter_cat(_First));
		}

	template<class _Iter>
		void _Construct(_Iter _Count, _Iter _Val, _Int_iterator_tag)
		{	// initialize with _Count * _Val
		size_type _Size = (size_type)_Count;
		if (_Buy(_Size))
			_Construct_n(_Size, _Val);
		}

	template<class _Iter>
		void _Construct(_Iter _First, _Iter _Last, input_iterator_tag)
		{	// initialize with [_First, _Last), input iterators
		_Buy(0);
		_TRY_BEGIN
		insert(begin(), _First, _Last);
		_CATCH_ALL
		_Tidy();
		_RERAISE;
		_CATCH_END
		}

	void _Construct_n(size_type _Count, const _Ty& _Val)
		{	// construct from _Count * _Val
		_TRY_BEGIN
		_Mylast = _Ufill(_Myfirst, _Count, _Val);
		_CATCH_ALL
		_Tidy();
		_RERAISE;
		_CATCH_END
		}

	~avector()
		{	// destroy the object
		_Tidy();
		}

	_Myt& operator=(const _Myt& _Right)
		{	// assign _Right
		if (this == &_Right)
			;	// nothing to do
		else if (_Right.size() == 0)
			clear();	// new sequence empty, free storage
		else if (_Right.size() <= size())
			{	// enough elements, copy new and destroy old
			pointer _Ptr = copy(_Right.begin(), _Right.end(), _Myfirst);
			_Destroy(_Ptr, _Mylast);
			_Mylast = _Myfirst + _Right.size();
			}
		else if (_Right.size() <= capacity())
			{	// enough room, copy and construct new
			const_iterator _Where = _Right.begin() + size();
			copy(_Right.begin(), _Where, _Myfirst);
			_Mylast = _Ucopy(_Where, _Right.end(), _Mylast);
			}
		else
			{	// not enough room, allocate new array and construct new
			_Destroy(_Myfirst, _Mylast);
			this->_Alval.deallocate(_Myfirst, _Myend - _Myfirst);
			if (_Buy(_Right.size()))
				_Mylast = _Ucopy(_Right.begin(), _Right.end(), _Myfirst);
			}
		return (*this);
		}

	void reserve(size_type _Count)
		{	// determine new minimum length of allocated storage
		if (max_size() < _Count)
			_Xlen();	// result too long
		else if (capacity() < _Count)
			{	// not enough room, reallocate
			pointer _Ptr = this->_Alval.allocate(_Count, (void *)0);

			_TRY_BEGIN
			_Ucopy(begin(), end(), _Ptr);
			_CATCH_ALL
			this->_Alval.deallocate(_Ptr, _Count);
			_RERAISE;
			_CATCH_END

			size_type _Size = size();
			if (_Myfirst != 0)
				{	// destroy old array
				_Destroy(_Myfirst, _Mylast);
				this->_Alval.deallocate(_Myfirst, _Myend - _Myfirst);
				}
			_Myend = _Ptr + _Count;
			_Mylast = _Ptr + _Size;
			_Myfirst = _Ptr;
			}
		}

	size_type capacity() const
		{	// return current length of allocated storage
		return (_Myfirst == 0 ? 0 : _Myend - _Myfirst);
		}

	iterator begin()
		{	// return iterator for beginning of mutable sequence
		return (iterator(_Myfirst));
		}

	const_iterator begin() const
		{	// return iterator for beginning of nonmutable sequence
		return (const_iterator(_Myfirst));
		}

	iterator end()
		{	// return iterator for end of mutable sequence
		return (iterator(_Mylast));
		}

	const_iterator end() const
		{	// return iterator for end of nonmutable sequence
		return (const_iterator(_Mylast));
		}

	reverse_iterator rbegin()
		{	// return iterator for beginning of reversed mutable sequence
		return (reverse_iterator(end()));
		}

	const_reverse_iterator rbegin() const
		{	// return iterator for beginning of reversed nonmutable sequence
		return (const_reverse_iterator(end()));
		}

	reverse_iterator rend()
		{	// return iterator for end of reversed mutable sequence
		return (reverse_iterator(begin()));
		}

	const_reverse_iterator rend() const
		{	// return iterator for end of reversed nonmutable sequence
		return (const_reverse_iterator(begin()));
		}

	void resize(size_type _Newsize)
		{	// determine new length, padding with _Ty() elements as needed
		resize(_Newsize, _Ty());
		}

	void resize(size_type _Newsize, const _Ty& _Val)
		{	// determine new length, padding with _Val elements as needed
		if (size() < _Newsize)
			_Insert_n(end(), _Newsize - size(), _Val);
		else if (_Newsize < size())
			erase(begin() + _Newsize, end());
		}

	size_type size() const
		{	// return length of sequence
		return (_Myfirst == 0 ? 0 : _Mylast - _Myfirst);
		}

	size_type max_size() const
		{	// return maximum possible length of sequence
		return (this->_Alval.max_size());
		}

	bool empty() const
		{	// test if sequence is empty
		return (size() == 0);
		}

	_Alloc get_allocator() const
		{	// return allocator object for values
		return (this->_Alval);
		}

	const_reference at(size_type _Off) const
		{	// subscript nonmutable sequence with checking
		if (size() <= _Off)
			_Xran();
		return (*(begin() + _Off));
		}

	reference at(size_type _Off)
		{	// subscript mutable sequence with checking
		if (size() <= _Off)
			_Xran();
		return (*(begin() + _Off));
		}

	reference operator[](size_type _Off)
		{	// subscript mutable sequence
		return (*(begin() + _Off));
		}

	const_reference operator[](size_type _Off) const
		{	// subscript nonmutable sequence
		return (*(begin() + _Off));
		}

	reference front()
		{	// return first element of mutable sequence
		return (*begin());
		}

	const_reference front() const
		{	// return first element of nonmutable sequence
		return (*begin());
		}

	reference back()
		{	// return last element of mutable sequence
		return (*(end() - 1));
		}

	const_reference back() const
		{	// return last element of nonmutable sequence
		return (*(end() - 1));
		}

	void push_back(const _Ty& _Val)
		{	// insert element at end
		if (size() < capacity())
			_Mylast = _Ufill(_Mylast, 1, _Val);
		else
			insert(end(), _Val);
		}

	void pop_back()
		{	// erase element at end
		if (!empty())
			{	// erase last element
			_Destroy(_Mylast - 1, _Mylast);
			--_Mylast;
			}
		}

	template<class _Iter>
		void assign(_Iter _First, _Iter _Last)
		{	// assign [_First, _Last)
		_Assign(_First, _Last, _Iter_cat(_First));
		}

	template<class _Iter>
		void _Assign(_Iter _Count, _Iter _Val, _Int_iterator_tag)
		{	// assign _Count * _Val
		_Assign_n((size_type)_Count, (_Ty)_Val);
		}

	template<class _Iter>
		void _Assign(_Iter _First, _Iter _Last, input_iterator_tag)
		{	// assign [_First, _Last), input iterators
		erase(begin(), end());
		insert(begin(), _First, _Last);
		}

	void assign(size_type _Count, const _Ty& _Val)
		{	// assign _Count * _Val
		_Assign_n(_Count, _Val);
		}

	iterator insert(iterator _Where, const _Ty& _Val)
		{	// insert _Val at _Where
		size_type _Off = size() == 0 ? 0 : _Where - begin();
		_Insert_n(_Where, (size_type)1, _Val);
		return (begin() + _Off);
		}

	void insert(iterator _Where, size_type _Count, const _Ty& _Val)
		{	// insert _Count * _Val at _Where
		_Insert_n(_Where, _Count, _Val);
		}

	template<class _Iter>
		void insert(iterator _Where, _Iter _First, _Iter _Last)
		{	// insert [_First, _Last) at _Where
		_Insert(_Where, _First, _Last, _Iter_cat(_First));
		}

	template<class _Iter>
		void _Insert(iterator _Where, _Iter _First, _Iter _Last,
			_Int_iterator_tag)
		{	// insert _Count * _Val at _Where
		_Insert_n(_Where, (size_type)_First, (_Ty)_Last);
		}

	template<class _Iter>
		void _Insert(iterator _Where, _Iter _First, _Iter _Last,
			input_iterator_tag)
		{	// insert [_First, _Last) at _Where, input iterators
		for (; _First != _Last; ++_First, ++_Where)
			_Where = insert(_Where, *_First);
		}

	template<class _Iter>
		void _Insert(iterator _Where, _Iter _First, _Iter _Last,
			forward_iterator_tag)
		{	// insert [_First, _Last) at _Where, forward iterators
		size_type _Count = 0;
		_Distance(_First, _Last, _Count);
		size_type _Capacity = capacity();

		if (_Count == 0)
			;
		else if (max_size() - size() < _Count)
			_Xlen();	// result too long
		else if (_Capacity < size() + _Count)
			{	// not enough room, reallocate
			_Capacity = max_size() - _Capacity / 2 < _Capacity
				? 0 : _Capacity + _Capacity / 2;	// try to grow by 50%
			if (_Capacity < size() + _Count)
				_Capacity = size() + _Count;
			pointer _Newvec = this->_Alval.allocate(_Capacity, (void *)0);
			pointer _Ptr = _Newvec;

			_TRY_BEGIN
			_Ptr = _Ucopy(begin(), _Where, _Newvec);	// copy prefix
			_Ptr = _Ucopy(_First, _Last, _Ptr);	// add new stuff
			_Ucopy(_Where, end(), _Ptr);	// copy suffix
			_CATCH_ALL
			_Destroy(_Newvec, _Ptr);
			this->_Alval.deallocate(_Newvec, _Capacity);
			_RERAISE;
			_CATCH_END

			_Count += size();
			if (_Myfirst != 0)
				{	// destroy and deallocate old array
				_Destroy(_Myfirst, _Mylast);
				this->_Alval.deallocate(_Myfirst, _Myend - _Myfirst);
				}
			_Myend = _Newvec + _Capacity;
			_Mylast = _Newvec + _Count;
			_Myfirst = _Newvec;
			}
		else if ((size_type)(end() - _Where) < _Count)
			{	// new stuff spills off end
			_Ucopy(_Where, end(), _AVECTOR_ITER_BASE(_Where) + _Count);	// copy suffix
			_Iter _Mid = _First;
			advance(_Mid, end() - _Where);

			_TRY_BEGIN
			_Ucopy(_Mid, _Last, _Mylast);	// insert new stuff off end
			_CATCH_ALL
			_Destroy(_AVECTOR_ITER_BASE(_Where) + _Count, _Mylast + _Count);
			_RERAISE;
			_CATCH_END

			_Mylast += _Count;
			#if defined(_MSC_VER) && _MSC_VER == 1600
				_Copy_impl(_First, _Mid, _Where);	// insert up to old end
			#elif defined(_MSC_VER) && _MSC_VER >= 1400
				_STDEXT unchecked_copy(_First, _Mid, _Where);	// insert up to old end
			#else
				copy(_First, _Mid, _Where);	// insert up to old end
			#endif
			}
		else
			{	// new stuff can all be assigned
			iterator _Oldend = end();
			_Mylast = _Ucopy(_Oldend - _Count, _Oldend,
				_Mylast);	// copy suffix
				
			#if defined(_MSC_VER) && _MSC_VER == 1600
				_Copy_backward(_Where, _Oldend - _Count, _Oldend);	// copy hole
				_Copy_impl(_First, _Last, _Where);	// insert into hole
			#elif defined(_MSC_VER) && _MSC_VER >= 1400
				_STDEXT unchecked_copy_backward(_Where, _Oldend - _Count, _Oldend);	// copy hole
				_STDEXT unchecked_copy(_First, _Last, _Where);	// insert into hole
			#else
				copy_backward(_Where, _Oldend - _Count, _Oldend);	// copy hole
				copy(_First, _Last, _Where);	// insert into hole
			#endif
			}
		}

	iterator erase(iterator _Where)
		{	// erase element at where
		#if defined(_MSC_VER) && _MSC_VER == 1600
			_Copy_impl(_Where + 1, end(), _Where);
		#else
			copy(_Where + 1, end(), _Where);
		#endif
		_Destroy(_Mylast - 1, _Mylast);
		--_Mylast;
		return (_Where);
		}

	iterator erase(iterator _First, iterator _Last)
		{	// erase [_First, _Last)
		if (_First != _Last)
			{	// worth doing, copy down over hole
			#if defined(_MSC_VER) && _MSC_VER == 1600
				pointer _Ptr = _Copy_impl(_Last, end(), _AVECTOR_ITER_BASE(_First));
			#elif defined(_MSC_VER) && _MSC_VER >= 1400
				pointer _Ptr = _STDEXT unchecked_copy(_Last, end(), _AVECTOR_ITER_BASE(_First));
			#else
				pointer _Ptr = copy(_Last, end(), _AVECTOR_ITER_BASE(_First));
			#endif
			_Destroy(_Ptr, _Mylast);
			_Mylast = _Ptr;
			}
		return (_First);
		}

	void clear()
		{	// erase all
		_Tidy();
		}

	bool _Eq(const _Myt& _Right) const
		{	// test for avector equality
		return (size() == _Right.size()
			&& equal(begin(), end(), _Right.begin()));
		}

	bool _Lt(const _Myt& _Right) const
		{	// test if this < _Right for avectors
		return (lexicographical_compare(begin(), end(),
			_Right.begin(), _Right.end()));
		}

	void swap(_Myt& _Right)
		{	// exchange contents with _Right
		if (this->_Alval == _Right._Alval)
			{	// same allocator, swap control information
			std::swap(_Myfirst, _Right._Myfirst);
			std::swap(_Mylast, _Right._Mylast);
			std::swap(_Myend, _Right._Myend);
			}
		else
			{	// different allocator, do multiple assigns
			_Myt _Ts = *this; *this = _Right, _Right = _Ts;
			}
		}

	friend void swap(_Myt& _Left, _Myt& _Right)
		{	// swap _Left and _Right avectors
		_Left.swap(_Right);
		}

protected:
	void _Assign_n(size_type _Count, const _Ty& _Val)
		{	// assign _Count * _Val
		_Ty _Tmp = _Val;	// in case _Val is in sequence
		erase(begin(), end());
		insert(begin(), _Count, _Tmp);
		}

	bool _Buy(size_type _Capacity)
		{	// allocate array with _Capacity elements
		_Myfirst = 0, _Mylast = 0, _Myend = 0;
		if (_Capacity == 0)
			return (false);
		else if (max_size() < _Capacity)
			_Xlen();	// result too long
		else
			{	// nonempty array, allocate storage
			_Myfirst = this->_Alval.allocate(_Capacity, (void *)0);
			_Mylast = _Myfirst;
			_Myend = _Myfirst + _Capacity;
			}
		return (true);
		}

	void _Destroy(pointer _First, pointer _Last)
		{	// destroy [_First, _Last) using allocator
		_Destroy_range(_First, _Last, this->_Alval);
		}

	void _Tidy()
		{	// free all storage
		if (_Myfirst != 0)
			{	// something to free, destroy and deallocate it
			_Destroy(_Myfirst, _Mylast);
			this->_Alval.deallocate(_Myfirst, _Myend - _Myfirst);
			}
		_Myfirst = 0, _Mylast = 0, _Myend = 0;
		}

	template<class _Iter>
		pointer _Ucopy(_Iter _First, _Iter _Last, pointer _Ptr)
		{	// copy initializing [_First, _Last), using allocator
			#if defined(_MSC_VER) && _MSC_VER == 1600
				return (_Uninitialized_copy0(_First, _Last, _Ptr));
			#elif defined(_MSC_VER) && _MSC_VER >= 1400
				return (_STDEXT unchecked_uninitialized_copy(_First, _Last,
					_Ptr, this->_Alval));
			#else
				return (_Uninitialized_copy(_First, _Last,
					_Ptr, this->_Alval));
			#endif
		}

	void _Insert_n(iterator _Where, size_type _Count, const _Ty& _Val)
		{	// insert _Count * _Val at _Where
		_Ty _Tmp = _Val;	// in case _Val is in sequence
		size_type _Capacity = capacity();

		if (_Count == 0)
			;
		else if (max_size() - size() < _Count)
			_Xlen();	// result too long
		else if (_Capacity < size() + _Count)
			{	// not enough room, reallocate
			_Capacity = max_size() - _Capacity / 2 < _Capacity
				? 0 : _Capacity + _Capacity / 2;	// try to grow by 50%
			if (_Capacity < size() + _Count)
				_Capacity = size() + _Count;
			pointer _Newvec = this->_Alval.allocate(_Capacity, (void *)0);
			pointer _Ptr = _Newvec;

			_TRY_BEGIN
			_Ptr = _Ucopy(begin(), _Where, _Newvec);	// copy prefix
			_Ptr = _Ufill(_Ptr, _Count, _Tmp);	// add new stuff
			_Ucopy(_Where, end(), _Ptr);	// copy suffix
			_CATCH_ALL
			_Destroy(_Newvec, _Ptr);
			this->_Alval.deallocate(_Newvec, _Capacity);
			_RERAISE;
			_CATCH_END

			_Count += size();
			if (_Myfirst != 0)
				{	// destroy and deallocate old array
				_Destroy(_Myfirst, _Mylast);
				this->_Alval.deallocate(_Myfirst, _Myend - _Myfirst);
				}
			_Myend = _Newvec + _Capacity;
			_Mylast = _Newvec + _Count;
			_Myfirst = _Newvec;
			}
		else if ((size_type)(end() - _Where) < _Count)
			{	// new stuff spills off end
			_Ucopy(_Where, end(), _AVECTOR_ITER_BASE(_Where) + _Count);	// copy suffix

			_TRY_BEGIN
			_Ufill(_Mylast, _Count - (end() - _Where),
				_Tmp);	// insert new stuff off end
			_CATCH_ALL
			_Destroy(_AVECTOR_ITER_BASE(_Where) + _Count, _Mylast + _Count);
			_RERAISE;
			_CATCH_END

			_Mylast += _Count;
			fill(_Where, end() - _Count, _Tmp);	// insert up to old end
			}
		else
			{	// new stuff can all be assigned
			iterator _Oldend = end();
			_Mylast = _Ucopy(_Oldend - _Count, _Oldend,
				_Mylast);	// copy suffix
			#if defined(_MSC_VER) && _MSC_VER == 1600
				_Copy_backward(_Where, _Oldend - _Count, _Oldend);
			#elif defined(_MSC_VER) && _MSC_VER >= 1400
				_STDEXT unchecked_copy_backward(_Where, _Oldend - _Count, _Oldend);	// copy hole
			#else
				copy_backward(_Where, _Oldend - _Count, _Oldend);	// copy hole
			#endif
			fill(_Where, _Where + _Count, _Tmp);	// insert into hole
			}
		}

	pointer _Ufill(pointer _Ptr, size_type _Count, const _Ty &_Val)
		{	// copy initializing _Count * _Val, using allocator
		#if defined(_MSC_VER) && _MSC_VER == 1600
			uninitialized_fill_n(_Ptr, _Count, _Val);
		#elif defined(_MSC_VER) && _MSC_VER >= 1400
			_STDEXT unchecked_uninitialized_fill_n(_Ptr, _Count, _Val, this->_Alval);
		#else
			_Uninitialized_fill_n(_Ptr, _Count, _Val, this->_Alval);
		#endif
		
		return (_Ptr + _Count);
		}

	void _Xlen() const
		{	// report a length_error
		_THROW(length_error, "avector<T> too long");
		}

	void _Xran() const
		{	// report an out_of_range error
		_THROW(out_of_range, "invalid avector<T> subscript");
		}

	pointer _Myfirst;	// pointer to beginning of array
	pointer _Mylast;	// pointer to current end of sequence
	pointer _Myend;	// pointer to end of array
	};

		// avector TEMPLATE FUNCTIONS
template<class _Ty, class _Alloc> inline
	bool operator==(const avector<_Ty, _Alloc>& _Left,
		const avector<_Ty, _Alloc>& _Right)
	{	// test for avector equality
	return (_Left._Eq(_Right));
	}

template<class _Ty, class _Alloc> inline
	bool operator!=(const avector<_Ty, _Alloc>& _Left,
		const avector<_Ty, _Alloc>& _Right)
	{	// test for avector inequality
	return (!(_Left == _Right));
	}

template<class _Ty, class _Alloc> inline
	bool operator<(const avector<_Ty, _Alloc>& _Left,
		const avector<_Ty, _Alloc>& _Right)
	{	// test if _Left < _Right for avectors
	return (_Left._Lt(_Right));
	}

template<class _Ty, class _Alloc> inline
	bool operator>(const avector<_Ty, _Alloc>& _Left,
		const avector<_Ty, _Alloc>& _Right)
	{	// test if _Left > _Right for avectors
	return (_Right < _Left);
	}

template<class _Ty, class _Alloc> inline
	bool operator<=(const avector<_Ty, _Alloc>& _Left,
		const avector<_Ty, _Alloc>& _Right)
	{	// test if _Left <= _Right for avectors
	return (!(_Right < _Left));
	}

template<class _Ty, class _Alloc> inline
	bool operator>=(const avector<_Ty, _Alloc>& _Left,
		const avector<_Ty, _Alloc>& _Right)
	{	// test if _Left >= _Right for avectors
	return (!(_Left < _Right));
	}

_STD_END
#pragma warning(default: 4244)
#pragma warning(pop)
#pragma pack(pop)

template <class T, class A> uint32 memoryUse( const std::avector<T,A> & obj )
{ return (!obj.empty()) ? allocatedSize( ((uint8*)&obj[0])-16 ) : 0; }

#endif // no-op

#endif /* AVECTOR_HPP */

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
