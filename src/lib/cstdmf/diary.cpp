/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"
#include "diary.hpp"

#ifndef CODE_INLINE
#include "diary.ipp"
#endif

#include "cstdmf/concurrency.hpp"
#include "config.hpp"
#include <algorithm>


static DiaryEntry s_dummyEntry( "dummy" );

// -----------------------------------------------------------------------------
// Section: Diary
// -----------------------------------------------------------------------------

/**
 *	Constructor.
 */
Diary::Diary() :
	pSaver_( NULL )
{
	SimpleMutexHolder smh( s_diariesMutex_ );
	s_diaries_.push_back( this );
	s_dummyEntry.incRef();

	levelColours_[0] = 0;
}


/**
 *	Destructor.
 */
Diary::~Diary()
{
	this->save();
	entries_.clear();
}


void Diary::save()
{
	entriesMutex_.grab();
	if (pSaver_ != NULL)
		pSaver_->save( entries_.begin(), entries_.end() );	
	entriesMutex_.give();

	entries_.clear();
}


/**
 *	Add an entry of the given desc
 */
DiaryEntryPtr Diary::add( const std::string & desc )
{
	if (!s_enabled_)
		return &s_dummyEntry;

	uint64 ts = timestamp();
	DiaryEntryPtr pDE = new DiaryEntry( desc );

	pDE->start_ = ts;
	pDE->stop_ = 0;

	entriesMutex_.grab();
	for (int i = int(levels_.size()) - 1; i >= 0; i--)
		if (levels_[i]->stop_ != 0) levels_.pop_back();

	pDE->level_ = levels_.size();
	MF_ASSERT( levels_.size() < 32 );
	pDE->colour_ = levelColours_[levels_.size()]++;

	levels_.push_back( pDE );
	levelColours_[levels_.size()] = 0;

	entries_.push_back( pDE );	
	int nEntries = entries_.size();
	entriesMutex_.give();

	//check must be ==, not >= so that the save diary entry
	//can be added without inifite recursion.
	if (nEntries == MAX_STORED_ENTRIES)
	{		
		DiaryEntryPtr des = this->add( "saveDiary" );

		entriesMutex_.grab();
		int nerase = (MAX_STORED_ENTRIES - MAX_READABLE_ENTRIES) / 2;
		if (pSaver_ != NULL)
			pSaver_->save( entries_.begin(), entries_.begin() + nerase );
		entries_.erase( entries_.begin(), entries_.begin() + nerase );		
		des->stop();
		entriesMutex_.give();
	}	

	return pDE;
}


/**
 *	Static method to look at all the diaries at this point in time
 */
void Diary::look( std::vector<Diary*> & diaries )
{
	SimpleMutexHolder smh( s_diariesMutex_ );
	diaries = s_diaries_;
}

/**
 *	Method to look at all the entries in this diary
 */
void Diary::look( DiaryEntries & entries )
{
	SimpleMutexHolder smh( entriesMutex_ );
	int nentries = entries_.size();
	int nshown = std::min( int(MAX_READABLE_ENTRIES), nentries );
	entries.assign( entries_.end() - nshown, entries_.end() );
}



/// Our funky TLS global
#ifndef MF_SINGLE_THREADED
static THREADLOCAL(Diary*) s_pMyDiary(NULL);
#else
Diary * s_pMyDiary = NULL;
#endif


/**
 *	Static instance method
 */
Diary & Diary::instance()
{
	if (s_pMyDiary == NULL) s_pMyDiary = new Diary();
	return *s_pMyDiary;
}


/**
 *	This method cleans up the Diaries.
 */
void Diary::fini()
{
	SimpleMutexHolder smh( s_diariesMutex_ );
	
	for
	(
		std::vector<Diary*>::iterator it = s_diaries_.begin();
		it != s_diaries_.end();
		++it
	)
	{
		delete *it;
	}
	s_diaries_.clear();

	s_enabled_ = false; // Access to diaries is no longer possible
}


/// Other statics
std::vector<Diary*>	Diary::s_diaries_;
SimpleMutex			Diary::s_diariesMutex_;
#if ENABLE_DIARIES
bool				Diary::s_enabled_ = true;
#else
bool				Diary::s_enabled_ = false;
#endif

// diary.cpp
