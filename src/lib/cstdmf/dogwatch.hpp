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
 *	@internal
 *	@file
 */

#ifndef DOGWATCH_HPP
#define DOGWATCH_HPP


#include "cstdmf/config.hpp"
#include "cstdmf/stdmf.hpp"
#include "cstdmf/smartpointer.hpp"
#include "cstdmf/timestamp.hpp"
#include "cstdmf/profiler.hpp"
#include "cstdmf/debug.hpp"
#include <string>
#include <vector>
#include "cstdmf/static_array.hpp"


#if !ENABLE_DOG_WATCHERS
	#define NO_DOG_WATCHES
#endif

/**
 *	@internal
 *
 *	This class is used to time events during a frame. It keeps a short history
 *	of accumulated time per frame, and organises itself hierarchially based on
 *	what other DogWatchs are running.
 *
 *	DogWatches should be static or global, as they are intended to be used as
 *	timers for recurring events.
 */
class DogWatch
{
public:
	DogWatch( const char * title );

	void start();
	void stop();

	uint64 slice() const;
	const std::string & title() const;

	// Keep a slice around to initialise dog watch before starting, this way
	// we don't have to check for null in stop().
	static		uint64 s_nullSlice_;

private:
	uint64		started_;
	uint64		*pSlice_;
	int			id_;
	int			profilerId_;
	std::string title_;
};


/**
 *	@internal
 *
 *	Starts and stops a dogwatcher. This class is usefull
 *	when a method has more than one exit points. It will
 *	stop the dogWatcher when it goes out of scope.
 */
class ScopedDogWatch
{

public:
	inline ScopedDogWatch( DogWatch & dogWatch ) :
		dogWatch_( dogWatch )
	{
		this->dogWatch_.start();
	}

	inline ~ScopedDogWatch()
	{
		this->dogWatch_.stop();
	}

private:
	DogWatch & dogWatch_;
};


/**
 *	@internal
 *	This singleton class manages the dog watches and performs
 *	functions on all of them.
 */
class DogWatchManager
{
public:
	enum
	{
		NUM_SLICES		= 120,
		MAX_WATCHES		= 128,
	};

public:
	DogWatchManager();

	// move on to the next slice
	void tick();

	class iterator;

	iterator begin() const;
	iterator end() const;

	static DogWatchManager & instance();
	static void fini();

private:
	static DogWatchManager		* pInstance;

	void clearCache();

	int add( DogWatch & watch );

	uint64 & grabSlice( int id );
	void giveSlice();

	uint64 & grabSliceMissedCache( int id );

	friend class DogWatch;

	/// The slice index we're currently writing to
	uint32	slice_;

	/// The table elements currently in use
	struct TableElt;
	std::vector<TableElt *>	stack_;

	/// The manifestation cache
	struct Cache
	{
		TableElt	* stackNow;
		TableElt	* stackNew;
		uint64		* pSlice;
	} cache_[MAX_WATCHES];

	/// The manifestation of a watch
	class Stat : public StaticArray<uint64,NUM_SLICES>, public ReferenceCount
	{
	public:
		int			watchid;
		mutable int flags;
	};

	typedef SmartPointer < Stat > StatPtr;
	typedef std::vector< StatPtr >	Stats;
	Stats	stats_;

	StatPtr newManifestation( TableElt & te, int watchid );

	/// Tables organising the manifestations
	struct TableElt
	{
		int		statid;
		int		tableid;
	};
	class Table : public StaticArray<TableElt, MAX_WATCHES>, public ReferenceCount {};

	typedef SmartPointer < Table > TablePtr;
	typedef std::vector< TablePtr >	Tables;
	Tables	tables_;

	TablePtr newTable( TableElt & te );

	/// Root table elt
	TableElt	root_;

	/// Frame timer
	uint64		frameStart_;

	/// Vector of added DogWatches
	typedef std::vector<DogWatch*>	DogWatches;
	DogWatches	watches_;


public:
	/**
	 *	@internal
	 *	This class is used to iterate over the elements in a DogWatchManager.
	 */
	class iterator
	{
	public:
		iterator( const Table* pTable, int id,
			const TableElt * pTE = NULL );
		iterator( const Table* pTable );

		iterator & operator=( const iterator & iter );

		bool operator==( const iterator & iter );
		/// This method implements the not equals operator.
		bool operator!=( const iterator & iter ) { return !(*this == iter); }
		iterator & operator++();

		iterator begin() const;
		iterator end() const;

		const std::string & name() const;

		int flags() const;
		void flags( int flags );

		uint64 value( int frameAgo = 1 ) const;
		uint64 average( int firstAgo, int lastAgo ) const;

	private:
		const Table		* pTable_;
		int				id_;
		const TableElt	& te_;
	};

	friend class DogWatchManager::iterator;
};



// This class is always inlined, sorry.
#include "dogwatch.ipp"



#endif // DOGWATCH_HPP
