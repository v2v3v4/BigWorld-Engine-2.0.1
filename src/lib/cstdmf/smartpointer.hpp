/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SMART_POINTER_HPP
#define SMART_POINTER_HPP

#include "stdmf.hpp"

#include "concurrency.hpp"

#ifdef _XBOX360
# include "Xtl.h"
#endif

#if defined( PLAYSTATION3 )
#include <cell/atomic.h>
#endif

//#define ENABLE_REFERENCE_COUNT_THREADING_DEBUG
#define REFERENCE_COUNT_THREADING_DEBUG_IGNORE_FIRST_THREAD

//forward delaration because of circular dependency
inline unsigned long OurThreadID();

/**
 *	This class is used to implement the behaviour required of an object
 *	referenced by a SmartPointer.
 *
 *	It stores a reference count with the object.
 */
class ReferenceCount
{
#ifdef ENABLE_REFERENCE_COUNT_THREADING_DEBUG
	mutable unsigned long threadID_;
#ifdef REFERENCE_COUNT_THREADING_DEBUG_IGNORE_FIRST_THREAD
	mutable bool firstThread_;
#endif//REFERENCE_COUNT_THREADING_DEBUG_IGNORE_FIRST_THREAD
	static const unsigned long INVALID_THREAD_ID = (unsigned long)-1;

	void check() const
	{
		if (threadID_ == INVALID_THREAD_ID)
		{
			threadID_ = OurThreadID();
#ifdef REFERENCE_COUNT_THREADING_DEBUG_IGNORE_FIRST_THREAD
			firstThread_ = true;
#endif//REFERENCE_COUNT_THREADING_DEBUG_IGNORE_FIRST_THREAD
		}
		else
		{
			if (threadID_ != OurThreadID())
			{
#ifdef REFERENCE_COUNT_THREADING_DEBUG_IGNORE_FIRST_THREAD
				if (firstThread_)
				{
					threadID_ = OurThreadID();
					firstThread_ = false;
				}
				else
#endif//REFERENCE_COUNT_THREADING_DEBUG_IGNORE_FIRST_THREAD
				{
					MF_ASSERT( !"ReferenceCounted object is accessed from different thread" );
				}
			}
		}
	}
	#define REFERENCE_COUNT_THREAD_CHECK check()
#else//ENABLE_REFERENCE_COUNT_THREADING_DEBUG
	#define REFERENCE_COUNT_THREAD_CHECK	(void)0
#endif//ENABLE_REFERENCE_COUNT_THREADING_DEBUG

public:
	/**
	 *	Constructor.
	 */
	ReferenceCount()
		: count_(0)
	{
#ifdef ENABLE_REFERENCE_COUNT_THREADING_DEBUG
		threadID_ = INVALID_THREAD_ID;
#endif//ENABLE_REFERENCE_COUNT_THREADING_DEBUG
		REFERENCE_COUNT_THREAD_CHECK;
	}

	/**
	 *	Copy constructor.
	 */
	ReferenceCount( const ReferenceCount & /*rOther*/ )
		: count_(0)
	{
#ifdef ENABLE_REFERENCE_COUNT_THREADING_DEBUG
		threadID_ = INVALID_THREAD_ID;
#endif//ENABLE_REFERENCE_COUNT_THREADING_DEBUG
		REFERENCE_COUNT_THREAD_CHECK;
	}

	/**
	 *	Destructor.
	 */
	virtual ~ReferenceCount() {}

	/**
	 *	This method increases the reference count by 1.
	 */
	inline void incRef() const
	{
		REFERENCE_COUNT_THREAD_CHECK;
		++count_;
	}

	/**
	 *	This method increases the reference count by count.
	 */
	inline void incRef( long count ) const
	{
		REFERENCE_COUNT_THREAD_CHECK;
		count_ += count;
	}

	/**
	 *	This method decreases the reference count by 1.
	 */
	inline void decRef() const
	{
		REFERENCE_COUNT_THREAD_CHECK;

		if (--count_ == 0)
			delete const_cast<ReferenceCount*>(this);
	}

	/**
	 *	This method returns the reference count of this object.
	 */
	inline long refCount() const
	{
		return count_;
	}

private:
	/**
	 *	Assignment operator, which makes no sense for objects
	 *	with a reference count.
	 */
	ReferenceCount & operator=( const ReferenceCount & rOther );

	volatile mutable long count_;

	friend class SafeReferenceCount;
};


#if defined( _WIN32 ) && !defined( _XBOX360 )
/**
 *	This class is a thread safe reference count
 *	@see ReferenceCount
 */
class SafeReferenceCount : public ReferenceCount
{
public:
	/**
	 *	This method increases the reference count by 1.
	 */
	inline void __fastcall incRef() const
	{
		__asm lock add [ecx]ReferenceCount.count_, 1;
	}

	/**
	 *	This method increases the reference count by count.
	 */
	inline void __fastcall incRef( long count ) const
	{
		__asm mov eax, count
		__asm lock add [ecx]ReferenceCount.count_, eax;
	}

	/**
	 *	This method decreases the reference count by 1.
	 */
	inline void __fastcall decRef() const
	{
		// if it was 1 before it was decremented, delete it now
		if (this->intDecRef() == 1)
			delete const_cast<SafeReferenceCount*>(this);
	}

	/**
	 *	This method tries to increment the reference count,
	 *	returns non-zero if the original value was non-zero.
	 *	returns zero if the original value was zero.
	 */
	inline int __fastcall incRefTry() const
	{
		irt_retry:
			// load cur ref count and return false if zero
			__asm mov eax, [ecx]ReferenceCount.count_
			__asm test eax,eax
			__asm jz irt_end

			// nonzero so increment in register
			__asm mov edx, eax
			__asm inc edx
			// store inc'd value if memory hasn't changed underneath
			__asm lock cmpxchg [ecx]ReferenceCount.count_, edx
			// if memory has changed then retry
			__asm jnz irt_retry
			// fall through to return old (non-zero) ref count
		irt_end:;
	}

private:
	/**
	 *	This private method decreases the reference count by 1.
	 */
#pragma warning (push)
#pragma warning (disable: 4035)
	inline int __fastcall intDecRef() const
	{
		__asm mov eax, -1
		__asm lock xadd [ecx]ReferenceCount.count_, eax;
	}
#pragma warning (pop)
};

#elif defined( _XBOX360 )

// TODO: This may not be the most efficient implementation.

/**
 *	This class is a thread safe reference count.
 *
 *	@see ReferenceCount
 */
class SafeReferenceCount : public ReferenceCount
{
public:
	/**
	 *	This method increases the reference count by 1.
	 */
	inline void incRef() const
	{
		InterlockedIncrement( &count_ );
	}

	/**
	 *	This method decreases the reference count by 1.
	 */
	inline void decRef() const
	{
		// if it was 1 before it was decremented, delete it now
		if (this->intDecRef() == 1)
			delete const_cast<SafeReferenceCount*>(this);
	}

	/**
	 *	This method tries to increment the reference count,
	 *	returns non-zero if the original value was non-zero.
	 *	returns zero if the original value was zero.
	 */
	//inline long incRefTry() const
	//{
	//	if (count_ == 0)
	//		return 0;

	//	return ++count_;
	//}

private:
	/**
	 *	This private method decreases the reference count by 1.
	 */
	inline int intDecRef() const
	{
		return InterlockedDecrement( &count_ ) + 1;
	}
};

#elif defined( PLAYSTATION3 )

/**
 *	This class is a thread safe reference count.
 *
 *	@see ReferenceCount
 */
class SafeReferenceCount : public ReferenceCount
{
public:
	/**
	 *	This method increases the reference count by 1.
	 */
	inline void incRef() const
	{
		cellAtomicIncr32( ( uint32* )&count_ );
	}

	/**
	 *	This method decreases the reference count by 1.
	 */
	inline void decRef() const
	{
		// if it was 1 before it was decremented, delete it now
		if (this->intDecRef() == 1)
			delete const_cast<SafeReferenceCount*>(this);
	}

	/**
	 *	This method tries to increment the reference count,
	 *	returns non-zero if the original value was non-zero.
	 *	returns zero if the original value was zero.
	 */
	//inline long incRefTry() const
	//{
	//	if (count_ == 0)
	//		return 0;

	//	return ++count_;
	//}

private:
	/**
	 *	This private method decreases the reference count by 1.
	 */
	inline int intDecRef() const
	{
		return cellAtomicDecr32( ( uint32* )&count_ ) + 1;
	}
};

#else

/**
 *	This class is a thread safe reference count
 *	@see ReferenceCount
 */
class SafeReferenceCount : public ReferenceCount
{
public:
	/**
	 *	This method increases the reference count by 1.
	 */
	inline void incRef() const
	{
		__asm__ volatile (
			"lock addl $1, %0"
			:						// no output
			: "m"	(this->count_) 	// input: this->count_
			: "memory" 				// clobbers memory
		);
	}

	/**
	 *	This method decreases the reference count by 1.
	 */
	inline void decRef() const
	{
		// if it was 1 before it was decremented, delete it now
		if (this->intDecRef() == 1)
			delete const_cast<SafeReferenceCount*>(this);
	}

	/**
	 *	This method tries to increment the reference count,
	 *	returns non-zero if the original value was non-zero.
	 *	returns zero if the original value was zero.
	 */
	inline int incRefTry() const
	{
		int ret, incced_val;
		__asm__ volatile (
			"1:\n\t\t"
				// load cur ref count and return false if zero
				"mov %2, %0\n\t\t"	// %0 == eax
				"test %%eax, %%eax\n\t\t"
				"jz 2f\n\t\t"

				// nonzero so increment in register
				"mov %%eax, %1\n\t\t"
				"inc %1\n\t\t"
				// store inc'd value if memory hasn't changed underneath
				"lock cmpxchg %1, %2\n\t\t"
				// if memory has changed then retry
				"jnz 1b\n"
				// fall through to return old (non-zero) ref count
			"2:"
			: 	"=&a"	(ret), 			// output-only and early clobber
				"=&q"	(incced_val) 	// output-only and early clobber
			: "m"		(this->count_)	// any kind of memory ref
			: "memory"					// ref'd memory is clobbered
		);
		return ret;
	}

private:
	/**
	 *	This private method decreases the reference count by 1.
	 */
	inline int intDecRef() const
	{
		int ret;
		__asm__ volatile (
			"mov $-1, %0  \n\t"
			"lock xadd %0, %1"
			: "=&a"	(ret)				// output only and early clobber
			: "m"	(this->count_)		// input (memory)
			: "memory"
		);
		return ret;
	}
};

#endif // _WIN32


/**
 *	This function increments the reference count of the input object.
 *	It can be specialised to increment the reference count of classes
 *	that do not have an 'incRef' method.
 */
template <class Ty>
inline void incrementReferenceCount( const Ty &Q )
{
	Q.incRef();
}


/**
 *	This function returns true if the reference count of the input object is
 *	equal to zero, or returns false otherwise.
 */
template <class Ty>
inline bool hasZeroReferenceCount( const Ty &Q )
{
	return (Q.refCount() == 0);
}


/**
 *	This function decrements the reference count of the input object.
 *	It can be specialised to increment the reference count of classes
 *	that do not have an 'decRef' method.
 *
 *	@note The decRef method in the object must delete itself when its
 *	reference count reaches zero - it is not done by the smart pointer
 *	itself (so it works with objects not allocated on the normal heap)
 */
template <class Ty>
inline void decrementReferenceCount( const Ty &Q )
{
	Q.decRef();
}

/**
 *	This function attempts to increment the reference count of the
 *	input object, and returns non-zero for success. It fails if the
 *	reference count is zero and returns zero. This is useful for
 *	incrementing a bare pointer in a population or census collection,
 *	where the object's destructor might have already been called.
 */
template <class Ty>
inline int incrementReferenceCountAttempt( const Ty &Q )
{
/*
	incrementReferenceCount( Q );
	return true;
}

template <>
inline bool incrementReferenceCountAttempt<SafeReferenceCount>(
	const SafeReferenceCount &Q )
{
*/
	return Q.incRefTry();
}


/**
 *	This function could be redefined to check that a pointer isn't null
 */
template <class Ty> inline
void null_pointer_check( const Ty * /*P*/ )
{
}



/**
 *	A ConstSmartPointer is a reference-counting smart const pointer to an
 *	Object. A SmartPointer is a ConstSmartPointer which allows non-const access
 *	to its Object.
 *
 *	Requirements for use are that for a given class C, the
 *	following expressions are valid:
 *
 *		EXPRESSION
 *		incrementReferenceCount(const C&)
 *		decrementReferenceCount(const C&)
 *
 *	Notes:
 *	- can be used as pointers, including the use of implicit casts. ie:
 *			class Base {};
 *			class Derived : public Base {};
 *			SmartPointer<Base> b;
 *			SmartPoiner<Derived> d;
 *			b = d;		 // ok, can create a base from a derived
 *			d = b;		 // compile-time error
 */
template<class Ty> class ConstSmartPointer
{
public:
	static const bool STEAL_REFERENCE = true;
	static const bool NEW_REFERENCE = false;
	enum TRY_tag { FALLIBLE };

	/// This type is what this pointer can point to.
	typedef Ty Object;
	/// A short-hand for this class.
	typedef ConstSmartPointer<Ty> This;

public:
	/**
	 *	This constructor initialises this pointer to refer to the input object.
	 */
	ConstSmartPointer( const Object *P = 0, bool alreadyIncremented = false )
	{
		object_ = P;
		if (object_)
		{
			if (!alreadyIncremented)
			{
				incrementReferenceCount( *object_ );
			}
			else
			{
				if (hasZeroReferenceCount( *object_ ))
				{
					MF_ASSERT_DEV( 0 &&
						"ConstSmartPointer::ConstSmartPointer - The reference "
						"count should not be zero\nfor objects that already "
						"have their reference incremented" );
				}
			}
		}
	}

	/**
	 *	This constructor initialises this pointer to refer to the input object,
	 *	as long as its reference count has not returned to zero. This
	 *	constructor is for thread safety and should be used iff creating a
	 *	smart pointer from a bare pointer in a population or census collection.
	 */
	ConstSmartPointer( const Object *P, TRY_tag ) :
		object_( incrementReferenceCountAttempt( *P ) ? P : 0 ) { }

	/**
	 *	The copy constructor.
	 */
	ConstSmartPointer( const This& P )
	{
		object_ = P.get();
		if (object_) incrementReferenceCount( *object_ );
	}

	/**
	 *	The assignment operator.
	 */
	This& operator=( const This& X )
	{
		if (object_ != X.get())
		{
			const Object* pOldObj = object_;
			object_ = X.get();
			if (object_) incrementReferenceCount( *object_ );
			if (pOldObj) decrementReferenceCount( *pOldObj );
		}
		return *this;
	}

	/**
	 *	Destructor.
	 */
	~ConstSmartPointer()
	{
		if (object_) decrementReferenceCount( *object_ );
		object_ = 0;
	}

	/**
	 *	This method returns the object that this pointer points to.
	 */
	const Object * get() const
	{
		return object_;
	}

	const Object * getObject() const
	{
		return this->get();
	}

	/**
	 *	This method returns whether or not this pointer points to anything.
	 */
	bool hasObject() const
	{
		return object_ != 0;
	}

	/**
	 *	This method returns whether or not this pointer points to anything.
	 */
	bool exists() const
	{
		return object_ != 0;
	}

	/**
	 *	This method implements the dereference operator. It helps allow this
	 *	object to be used as you would a normal pointer.
	 */
	const Object& operator*() const
	{
		null_pointer_check( object_ );
		return *object_;
	}

	/**
	 *	This method implements the dereference operator. It helps allow this
	 *	object to be used as you would a normal pointer.
	 */
	const Object* operator->() const
	{
		null_pointer_check( object_ );
		return object_;
	}

	/**
	 *	These functions return whether or not the input objects refer to the same
	 *	object.
	 */
	friend bool operator==( const ConstSmartPointer<Ty>& A,
		const ConstSmartPointer<Ty>& B )
	{
		return A.object_ == B.object_;
	}

	friend bool operator==( const ConstSmartPointer<Ty>& A,
		const Ty* B )
	{
		return A.object_ == B;
	}

	friend bool operator==( const Ty* A,
		const ConstSmartPointer<Ty>& B )
	{
		return A == B.object_;
	}

	/**
	 *	These functions return not or whether the input objects refer to the same
	 *	object.
	 */
	friend bool operator!=( const ConstSmartPointer<Ty>& A,
		const ConstSmartPointer<Ty>& B )
	{
		return A.object_ != B.object_;
	}

	friend bool operator!=( const ConstSmartPointer<Ty>& A,
		const Ty* B )
	{
		return A.object_ != B;
	}

	friend bool operator!=( const Ty* A,
		const ConstSmartPointer<Ty>& B )
	{
		return A != B.object_;
	}

	/**
	 *	These functions give an ordering on smart pointers so that they can be
	 *	placed in sorted containers.
	 */
	friend bool operator<( const ConstSmartPointer<Ty>& A,
		const ConstSmartPointer<Ty>& B )
	{
		return A.object_ < B.object_;
	}

	friend bool operator<( const ConstSmartPointer<Ty>& A,
		const Ty* B )
	{
		return A.object_ < B;
	}

	friend bool operator<( const Ty* A,
		const ConstSmartPointer<Ty>& B )
	{
		return A < B.object_;
	}

	/**
	 *	These functions give an ordering on smart pointers so that they can be
	 *	compared.
	 */
	friend bool operator>( const ConstSmartPointer<Ty>& A,
		const ConstSmartPointer<Ty>& B )
	{
		return A.object_ > B.object_;
	}

	friend bool operator>( const ConstSmartPointer<Ty>& A,
		const Ty* B )
	{
		return A.object_ > B;
	}

	friend bool operator>( const Ty* A,
		const ConstSmartPointer<Ty>& B )
	{
		return A > B.object_;
	}

	/**
	 *	This method returns whether or not this pointers points to anything.
	 */

	typedef const Ty * ConstSmartPointer<Ty>::*unspecified_bool_type;
	operator unspecified_bool_type() const
	{
		return object_ == 0? 0 : &ConstSmartPointer<Ty>::object_;
	}

protected:
	const Object *object_;		///< The object being pointed to.
};


/**
 *	This class is a ConstSmartPointer which also allows non-const access to
 *	its object.
 *
 *	@see ConstSmartPointer
 */
template <class Ty>
class SmartPointer : public ConstSmartPointer<Ty>
{
public:
	/// This type is a short-hand for this type.
	typedef SmartPointer<Ty> This;
	/// This type is the base class of this class.
	typedef ConstSmartPointer<Ty> ConstProxy;
	/// This type is what this pointer can point to.
	typedef Ty Object;

public:
	/**
	 *	This constructor initialises this pointer to refer to the input object.
	 */
	SmartPointer( Object *P = 0, bool alreadyIncremented = false ) :
		ConstProxy( P, alreadyIncremented ) { }

	/**
	 *	This constructor is used for thread safety. See the corresponding
	 *	constructor in ConstSmartPointer.
	 */
	SmartPointer( Object *P, typename ConstProxy::TRY_tag tag ) :
		ConstProxy( P, tag ) { }

	/**
	 *	The copy constructor.
	 */
	SmartPointer( const This& P ) : ConstProxy( P ) { }

	/**
	 * Copy constructor from derived types.
	 */
	template<class DerivedType>
	SmartPointer( ConstSmartPointer<DerivedType>& dt ) :
		ConstProxy( dt.get() )
	{
	}

	/**
	 *	The assignment operator.
	 */
	This& operator=( const This& P )
	{
		ConstProxy::operator=(P);
		return *this;
	}

	/**
	 *  Assignment operator for derived types.
	 */
	template<class DerivedType>
	This& operator=( ConstSmartPointer<DerivedType>& dt )
	{
		ConstProxy::operator=(dt.get());
		return *this;
	}

	/**
	 *	This method returns the object that this pointer points to.
	 */
	Object * get() const
	{
		return const_cast<Object *>( this->object_ );
	}

	Object * getObject() const
	{
		// Deprecated version of 'get'.
		return this->get();
	}

	/**
	 *	This method implements the dereference operator. It helps allow this
	 *	object to be used as you would a normal pointer.
	 */
	Object& operator*() const
	{
		null_pointer_check( this->object_ );
		return *const_cast<Object *>( this->object_ );
	}

	/**
	 *	This method implements the dereference operator. It helps allow this
	 *	object to be used as you would a normal pointer.
	 */
	Object* operator->() const
	{
		null_pointer_check( this->object_ );
		return const_cast<Object *>( this->object_ );
	}
};



#endif // SMART_POINTER_HPP
