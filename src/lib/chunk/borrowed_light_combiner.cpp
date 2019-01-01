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

#include "borrowed_light_combiner.hpp"
#include "chunk_light.hpp"

PROFILER_DECLARE( BorrowedLightCombiner_profiler, "Borrowed Light Combiner" );

BorrowedLightCombiner::BorrowedLightCombiner() 
	: numBorrowers_(0)
{
}

BorrowedLightCombiner::~BorrowedLightCombiner()
{
}

void BorrowedLightCombiner::begin( ChunkItemBase::Borrowers& borrowers )
{
	MF_ASSERT( numBorrowers_ == 0 );
	
	numBorrowers_ = borrowers.size();
	if ( numBorrowers_ == 0 )
	{
		combinedDiff_ = NULL;
		combinedSpec_ = NULL;
		return;
	}

	if (!combinedDiff_)
	{
		combinedDiff_ = new Moo::LightContainer;
		combinedSpec_ = new Moo::LightContainer;
	}

	origDiffuse_ = Moo::rc().lightContainer();
	origSpec_ = Moo::rc().specularLightContainer();

	combinedDiff_->assign( origDiffuse_ );
	combinedSpec_->assign( origSpec_ );

	for (ChunkItemBase::Borrowers::iterator it = borrowers.begin();
		it != borrowers.end(); it++)
	{
		combinedDiff_->addToSelf(
				ChunkLightCache::instance( **it ).pAllLights(),
				true, true, false );

		combinedSpec_->addToSelf(
				ChunkLightCache::instance( **it ).pAllSpecularLights(),
				true, true, false );
	}
	
	Moo::rc().lightContainer( combinedDiff_ );
	Moo::rc().specularLightContainer( combinedSpec_ );
}

void BorrowedLightCombiner::end()
{
	if (numBorrowers_ > 0)
	{
		Moo::rc().lightContainer( origDiffuse_ );
		Moo::rc().specularLightContainer( origSpec_ );
	}

	numBorrowers_ = 0;
}
