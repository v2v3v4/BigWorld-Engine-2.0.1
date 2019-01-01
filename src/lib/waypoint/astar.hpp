/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef _ASTAR_HEADER
#define _ASTAR_HEADER

#include "cstdmf/debug.hpp"

#include <list>
#include <queue>
#include <set>
#include <vector>

class Buffer
{
	static const size_t MAX_SIZE = 1024 * 256;
	static const size_t ALIGNMENT = 32;
	char buffer_[MAX_SIZE];
	std::list<char*> buffers_;

	char* buffer()
	{
		if( buffers_.empty() )
			return buffer_;
		return *buffers_.begin();
	}
	size_t offset_;
public:
	Buffer() : offset_( 0 )
	{}
	~Buffer()
	{
		for( std::list<char*>::iterator iter = buffers_.begin();
			iter != buffers_.end(); ++iter )
		{
			delete[] *iter;
		}
	}
	void* get( size_t size )
	{
		size_t offset = offset_;
		offset_ += ( size + ALIGNMENT - 1 ) / ALIGNMENT * ALIGNMENT;
		if( offset_ > MAX_SIZE )
		{
			if( size + ALIGNMENT > MAX_SIZE )
			{
				char* result = new char[size];
				buffers_.insert( buffers_.begin(), result );

				char* buffer = new char[MAX_SIZE];
				buffers_.insert( buffers_.begin(), buffer );
				offset_ = 0;

				return result;
			}
			char* buffer = new char[MAX_SIZE];
			buffers_.insert( buffers_.begin(), buffer );
			offset_ = 0;
			return get( size );
		}
		return buffer() + offset;
	}
};

template <class T> class buffer_allocator
{
public:
	Buffer& buffer_;
	typedef T					value_type;
	typedef value_type*			pointer;
	typedef const value_type*	const_pointer;
	typedef value_type&			reference;
	typedef const value_type&	const_reference;
	typedef std::size_t			size_type;
	typedef std::ptrdiff_t		difference_type;

	template <class U> struct rebind
	{
		typedef buffer_allocator<U> other;
	};

	buffer_allocator( Buffer& buffer )
		: buffer_( buffer )
	{}

	buffer_allocator( const buffer_allocator& that ) : buffer_( that.buffer_ ) {}

	template <class U> buffer_allocator( const buffer_allocator<U>& that ) : buffer_( that.buffer_ ) {}

	pointer address( reference x ) const	{	return &x;	}

	const_pointer address( const_reference x ) const	{	return x;	}

	pointer allocate( size_type n, const_pointer = 0 )
	{
		return static_cast<pointer>( buffer_.get( n * sizeof(T) ) );
	}

	void deallocate( pointer, size_type ) {}

	size_type max_size() const
	{
		return size_type( -1 ) / sizeof( T );
	}

	void construct( pointer p, const_reference x )	{	new(p) value_type( x );	}

	void destroy( pointer p )	{	p->~value_type();	}
};

template <class T>
bool operator!=( const buffer_allocator<T>& A, const buffer_allocator<T>& B )
{
	return &A.buffer_ != &B.buffer_;
}

/**
 *	This class implements an A* search. It will search anything that
 *	can be described with the following interface:
 *
 *	typedef \<some suitable type\> adjacency_iterator;
 *
 *	int	compare(const State& other) const
 *	bool isGoal(const State& goal) const
 *	adjacency_iterator adjacenciesBegin() const
 *	adjacency_iterator adjacenciesEnd() const
 *	bool getAdjacency(
 *		adjacency_iterator i, State& adj, const State& goal) const
 *	float distanceFromParent() const
 *	float distanceToGoal(const State& goal) const
 */
template<class State, class GoalState = State> 
class AStar
{
public:
	typedef State TState;
	typedef GoalState TGoalState;

	AStar();
	~AStar();

	bool				search(const State& start, const GoalState& goal, float maxDistance );
	const State*		first();
	const State*		next();

	// quick hack to detect this.
	bool infiniteLoopProblem;

private:
	Buffer buffer_;
	/**
	 *	This structure represents an internal state of the search.
	 */
	struct IntState
	{
		State		ext;		///< User defined state
		float		g;			///< Cost to get here from start
		float		h;			///< Estimated cost from here to goal
		float 		f;			///< g + h
		IntState*	pParent;	///< The state before us in the search
		IntState*	pChild;		///< The state after us in the search
		IntState*	hashNext;
		IntState*	freeNext;
		IntState()
			: hashNext( NULL ), freeNext( NULL )
		{}
		static IntState* alloc()
		{
			IntState* head = freeHead();
			if( head != NULL )
			{
				freeHead() = head->freeNext;
				new (head) IntState;
				return head;
			}
			return new IntState;
		}
		static void free( IntState* state )
		{
			state->~IntState();
			state->freeNext = freeHead();
			freeHead() = state;
		}
		static IntState*& freeHead()
		{
			static IntState* freeHead = NULL;
			return freeHead;
		}
	};

	class IntStateHash
	{
		static const int BIN_SIZE = 257;
		IntState* bin_[BIN_SIZE];
	public:
		IntStateHash()
		{
			for( int i = 0; i < BIN_SIZE; ++i )
				bin_[ i ] = 0;
		}
		unsigned int hash( IntState* state )
		{
			return (unsigned int)( state->ext.hash() ) % BIN_SIZE;
		}
		IntState* find( IntState* state )
		{
			IntState* head = bin_[ hash( state ) ];
			while( head != NULL )
			{
				if( head->ext.compare( state->ext ) == 0 )
					return head;
				head = head->hashNext;
			}
			return NULL;
		}
		void erase( IntState* state )
		{
			IntState* head = bin_[ hash( state ) ];
			MF_ASSERT( head );

			if( head->ext.compare( state->ext ) == 0 )
				bin_[ hash( state ) ] = head->hashNext;
			else
			{
				IntState* next = head->hashNext;
				while( next != NULL )
				{
					if( next->ext.compare( state->ext ) == 0 )
					{
						head->hashNext = next->hashNext;
						next->hashNext = NULL;
						break;
					}
					head = next;
					next = head->hashNext;
				}
			}
		}
		void insert( IntState* state )
		{
			if( find( state ) )
				return;
			state->hashNext = bin_[ hash( state ) ];
			bin_[ hash( state ) ] = state;
		}
		void reset()
		{
			for( int i = 0; i < BIN_SIZE; ++i )
			{
				IntState* item = bin_[ i ];
				while( item )
				{
					IntState* next = item->hashNext;
					IntState::free( item );
					item = next;
				}
				bin_[ i ] = NULL;
			}
		}
		int size() const
		{
			int size = 0;
			for( int i = 0; i < BIN_SIZE; ++i )
			{
				IntState* item = bin_[ i ];
				while( item )
				{
					++size;
					item = item->hashNext;
				}
			}
			return size;
		}
	};
	/**
	 *	This object is used to perform a comparison between internal
	 *	states on the priority queue. We want the first element on
	 *	the queue to be the state with the smallest value of f.
	 */
	struct cmp_f
	{
		bool operator()(const IntState* p1, const IntState* p2) const
		{
			return p1->f > p2->f;
		};
	};

	/**
	 *	This object is used to perform a comparison between internal
	 *	states in the set. It returns true if p1 is less than p2.
	 */
	struct cmp_ext
	{
		bool operator()(const IntState* p1, const IntState* p2) const
		{
			return p1->ext.compare(p2->ext) < 0;
		};
	};

	typedef IntStateHash AStarSet;
	typedef std::vector<IntState*, buffer_allocator<IntState*> > AStarVector;
	typedef std::priority_queue<IntState*, AStarVector, cmp_f> AStarQueue;


	IntState* 	iter_;
	IntState* 	start_;
	AStarSet 	set_;
	AStarQueue 	open_;
};

#include "astar.ipp"
#endif
