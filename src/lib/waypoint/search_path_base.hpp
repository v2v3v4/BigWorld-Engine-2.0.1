/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SEARCH_PATH_BASE_HPP
#define SEARCH_PATH_BASE_HPP

#include "astar.hpp"

#include <vector>


/**
 *	Base class for searched A* state paths. Each instance has access to current
 *	state, the next state, and the destination state. When the path is to be
 *	queried, clients call the matches() method with their current state and
 *	their intended destination state. 
 */
template< class SearchState >
class SearchPathBase
{
protected:
	/**
	 *	Constructor.
	 */
	SearchPathBase():
		reversePath_()
	{}

	virtual ~SearchPathBase()
	{}

public:

	/**
	 *	This method initialises this search path from the given A* search.
	 */
	virtual void init( AStar< SearchState > & astar ) = 0;


	/**
	 *	This method returns true if the two given search states are equivalent.
	 *	Subclasses must supply this method.
	 */
	virtual bool statesAreEquivalent( const SearchState & s1, 
			const SearchState & s2 ) const = 0;

	/**
	 *	Return true if the path is empty.
	 */
	bool empty() const
	{
		return reversePath_.empty();
	}


	/**
	 *	Return true if we have a valid path. This means that we have at least a
	 *	current state and the destination state.
	 */
	bool isValid() const
		{ return reversePath_.size() >= 2; }
	

	/**
	 *	Return the current search state, or NULL if the path is invalid.
	 */
	const SearchState * pCurrent() const
		{ return !this->empty() ? &reversePath_.back() : NULL; }


	/**
	 *	Return the next search state (which might be the destination), or NULL
	 *	if the path is invalid.
	 */
	const SearchState * pNext() const
	{ 
		return !this->empty() ? 
			&reversePath_[ reversePath_.size() - 2 ] : 
			NULL; 
	}

	
	/**
	 *	Return our destination search state.
	 */
	const SearchState * pDest() const
		{ return !this->empty() ? &reversePath_.front(): NULL; }


	/**
	 *	Clear the path.
	 */
	void clear()
		{ reversePath_.clear(); }


	/**
	 *	Proceed to the next search state. This means that the previous next
	 *	state is now our new current state.
	 */
	void pop()
	{
		if (this->isValid())
		{
			reversePath_.pop_back();
		}
	}

	bool matches( const SearchState & src, const SearchState & dst );


private:
	
	/**
	 *	Return whether the given destination state is equivalent to our stored
	 *	destination state.
	 */
	bool isDestinationState( const SearchState & dest ) const
	{ 
		return this->isValid() && 
			this->statesAreEquivalent( dest, *this->pDest() );
	}


	/**
	 *	Return whether the given current state is equivalent to our stored
	 *	current state.
	 */
	bool isCurrentState( const SearchState & current ) const
	{
		return this->isValid() &&
			this->statesAreEquivalent( current, *this->pCurrent() );
	}


	/**
	 *	Return whether the given current state is equivalent to the next state
	 *	in the path.
	 */
	bool isNextState( const SearchState & current ) const
	{
		return this->isValid() &&
			this->statesAreEquivalent( current, *this->pNext() );
	}

protected:

	// We use a reverse path (such that the destination is at the front, and
	// the current state is at the back) so that we can use std::vector
	// pop_back() for pop().
	std::vector< SearchState > reversePath_;

};


/**
 *	Return true if the given search states correspond to what is stored in this
 *	search path. The current node is advanced if the given source node is
 *	equivalent to the path's next node.
 */
template< class SearchState >
bool SearchPathBase< SearchState >::matches( const SearchState & src, 
		const SearchState & dst )
{
	// The criteria for this is that the destinations must be equivalent, and
	// the given source state must be equivalent to either the path's current
	// state or the path's next state.

	if (!this->isDestinationState( dst ))
	{
		// Destination has changed.
		return false;
	}

	// OK, this path leads to the right place.
	// Now make sure it starts from the right place too.
	if (this->isCurrentState( src ))
	{
		// Looks good.
		return true;
	}
	else if (this->isNextState( src ))
	{
		// Advance up the path.
		this->pop();
		return true;
	}

	// nope, no good. clear it now just for sanity
	return false;
}

// search_path_base.cpp

#endif // SEARCH_PATH_BASE_HPP
