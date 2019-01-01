/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
 *	This file contains the implementation of the AStar class.
 */

/**
 *	This is the constructor.
 */
template<class State, class GoalState>
AStar<State, GoalState>::AStar() :
	iter_(NULL),
	start_(NULL),
	open_( cmp_f(), AStarVector( buffer_ ) )
{}

/**
 *	This is the destructor.
 */
template<class State, class GoalState>
AStar<State, GoalState>::~AStar()
{
	set_.reset();
}

namespace
{
	/**
	 * keeps record of highest value passed in and prints out new high scores
	 *
	 */
	bool updateMaxCount( const int value )
	{
		static int highest = 0;
		static bool printed = 1;

		if (value == -1)
		{
			if (!printed)
			{
				TRACE_MSG( "AStar::search: "
					"New highest open_.size() = %d\n", highest );
				printed = true;
			}
			return true;
		}

		if (value > highest)
		{
			highest = value;
			printed = false;
		}

		if (value > 9999)
			return false;

		return true;
	}
}

/**
 *	This method performs an A* search.
 *
 *	@param start	This is the initial state of the search.
 *	@param goal		This is the goal state
 *	@param maxDistance		This is max distance to search
 *							(-1. means no limit)
 *
 *	@return	True if successful.
 */
template<class State, class GoalState>
bool AStar<State, GoalState>::search(const State& start, const GoalState& goal, float maxDistance )
{
	IntState* pCurrent;
	IntState* pAdjacency;
	IntState key;
	float g, f;
	bool checkMaxDist = maxDistance > 0.f;

	// Create an initial start node, and push it onto the open queue.

	pCurrent = IntState::alloc();
	pCurrent->ext = start;
	pCurrent->g = 0;
	pCurrent->h = start.distanceToGoal(goal);
	pCurrent->f = pCurrent->g + pCurrent->h;
	pCurrent->pParent = NULL;
	pCurrent->pChild = NULL;
	start_ = pCurrent;

	set_.insert(pCurrent);
	open_.push(pCurrent);

	this->infiniteLoopProblem = false;

	
	while(!open_.empty())
	{
		// Grab the top element from the queue, the one with the smallest
		// total cost.

		if (!updateMaxCount(open_.size()))
		{
			updateMaxCount(-1);
			ERROR_MSG( "AStar search has too many open states!\n" );
			this->infiniteLoopProblem = true;
		}

		pCurrent = open_.top();
		open_.pop();

		//DEBUG_MSG( "pop pCurrent->f %f open size %d\n", pCurrent->f, open_.size() );

		// If it satisfies the goal requirements, we are done. Walk
		// backwards and construct the path.

		if(pCurrent->ext.isGoal(goal))
		{
			int safetyCount = 0;
			while(pCurrent->pParent && (++safetyCount<10000) )
			{
				pCurrent->pParent->pChild = pCurrent;
				pCurrent = pCurrent->pParent;
			}
			updateMaxCount(-1);
			if ( safetyCount < 10000 )
			{
				return true;
			}
			else
			{
				ERROR_MSG( "AStar search entered infinite loop!\n" );
				this->infiniteLoopProblem = true;
				return false;
			}
		}

		// Now check all the adjacencies.

		typename State::adjacency_iterator adjIter =
			pCurrent->ext.adjacenciesBegin();

		for(; adjIter != pCurrent->ext.adjacenciesEnd(); ++adjIter)
		{
			// To make things easier for the State implementation,
			// not every adacency need be pursued. If getAdjacencyState
			// returns false, this is not a real adjacency, so ignore it.

			if(!pCurrent->ext.getAdjacency( adjIter, key.ext, goal ))
				continue;

			// Don't return to the start state. This would cause
			// a loop in the path.

			if(key.ext.compare(start_->ext) == 0)
				continue;

			if (checkMaxDist)
			{
				if (key.ext.distanceToGoal( start ) > maxDistance)
				{
					continue;
				}
			}

			g = pCurrent->g + key.ext.distanceFromParent();
			f = g + key.ext.distanceToGoal( goal );

			IntState* setIter = set_.find( &key );

			if( setIter != NULL )
			{
				// If we have already visited this state, it will be in
				// our set. If we previously visited it with a lower cost,
				// then forget about it this time.
				if(setIter->f <= f)
				{
					//DEBUG_MSG( "Found dup old g %g, new g %g, old f %g, new f %g\n",
					//	setIter->g, g, setIter->f, f );
					continue;
				}

				// Otherwise remove it from the set, we'll re-add it below
				// with a smaller cost.

				pAdjacency = setIter;
				set_.erase(&key);
			}
			else
			{
				pAdjacency = IntState::alloc();
			}
			
			// Sometimes they enter a loop in this case so we should break it
			IntState* p = pCurrent;

			while (p && p != pAdjacency)
			{
				p = p->pParent;
			}

			if (p)
			{
				continue;
			}

			pAdjacency->ext = key.ext;
			pAdjacency->g = g;

			pAdjacency->h = pAdjacency->ext.distanceToGoal(goal);

			pAdjacency->f = f; //pAdjacency->g + pAdjacency->h;
			pAdjacency->pParent = pCurrent;
			pAdjacency->pChild = NULL;

			// The set contains all states that we have encountered. The
			// priority queue contains all states that we have not yet
			// fully expanded. Add this new state to both.

			// DEBUG_MSG( "Adding new adjacency to open list: %f\n", pAdjacency->f );
			
			set_.insert(pAdjacency);
			open_.push(pAdjacency);
		}
	}

	updateMaxCount(-1);
	return false;
}

/**
 *	If the search was successful, this method is used to find the first
 *	state in the search result. It will always be the same as the start
 *	state that was passed in.
 *
 *	@return First state in the search.
 */
template<class State, class GoalState>
const State* AStar<State, GoalState>::first()
{
	iter_ = start_;
	return iter_ ? &iter_->ext : NULL;
}

/**
 *	This method should be called repeatedly to find subsequent states in
 *	the search result. If will return NULL if there are no more states.
 *
 *	@return Next state in the search, or NULL if complete.
 */
template<class State, class GoalState>
const State* AStar<State, GoalState>::next()
{
	if(iter_)
		iter_ = iter_->pChild;
	return iter_ ? &iter_->ext : NULL;
}
