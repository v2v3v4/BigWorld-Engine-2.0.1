/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef DIARY_HPP
#define DIARY_HPP

#include "concurrency.hpp"
#include "smartpointer.hpp"
#include "timestamp.hpp"
#include "vectornodest.hpp"

#include <string>
#include <vector>

/**
 *	This struct is an event worth noting
 */
struct DiaryEntry : public SafeReferenceCount
{
	DiaryEntry( const std::string & desc ) : desc_( desc ) { }
	void stop()	{ stop_ = timestamp(); }

	uint64		start_;
	uint64		stop_;
	std::string desc_;
	int			level_;
	int			colour_;
};

typedef SmartPointer<DiaryEntry> DiaryEntryPtr;
typedef std::vector<DiaryEntryPtr>	DiaryEntries;


/**
 *	This class keeps a diary of the events in one thread.
 */
class Diary
{
public:
	static Diary & instance();
	static void fini();

	DiaryEntryPtr add( const std::string & type );

	static void look( std::vector<Diary*> & diaries );
	void look( DiaryEntries & entries );

	enum
	{
		MAX_READABLE_ENTRIES = 2048,
		MAX_STORED_ENTRIES = MAX_READABLE_ENTRIES * 2
	};

	/**
	 *	This class allows an app save out obsolete diary entries
	 */
	class Saver
	{
	public:
		virtual ~Saver() { }
		virtual void save( DiaryEntries::const_iterator beg,
			DiaryEntries::const_iterator end ) = 0;
	};

	Saver * saver() const				{ return pSaver_; }
	void saver( Saver * pSaver )		{ pSaver_ = pSaver; }

	void save();

private:
	Diary();
	~Diary();
	Diary( const Diary& );
	Diary& operator=( const Diary& );

	static std::vector<Diary*>	s_diaries_;
	static SimpleMutex			s_diariesMutex_;

	std::vector<DiaryEntryPtr>	levels_;
	int				levelColours_[32];

	DiaryEntries	entries_;
	SimpleMutex		entriesMutex_;

	Saver *			pSaver_;

public:
	static bool s_enabled_;
};


/**
 *	This class adds and holds a diary entry
 */
class DiaryScribe
{
public:
	DiaryScribe( Diary & diary, const std::string & desc ) :
		de_( diary.add( desc ) ) { }
	~DiaryScribe()
		{ de_->stop(); }

private:
	DiaryScribe( const DiaryScribe & other );

	DiaryEntryPtr de_;
};


#ifdef CODE_INLINE
#include "diary.ipp"
#endif

#endif // DIARY_HPP
