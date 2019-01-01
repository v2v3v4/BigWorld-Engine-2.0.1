/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BORROWED_LIGHT_COMBINER__HPP
#define BORROWED_LIGHT_COMBINER__HPP

#include "math/boundbox.hpp"
#include "moo/light_container.hpp"

#include "chunk_item.hpp"

/**
 *	Combines the current light container with the light	containers for the 
 *	given set of Borrowers, and set that as the active light container.
 *	Even though the logic here is transient, there will generally be one instance
 *	of this class stored on each ChunkItem type that potentially needs a combiner,
 *	to avoid spurious memory allocations.
 */
class BorrowedLightCombiner
{
public:
	BorrowedLightCombiner();
	~BorrowedLightCombiner();

	void begin( ChunkItemBase::Borrowers& borrowers );
	void end();

private:
	size_t numBorrowers_;
	Moo::LightContainerPtr combinedDiff_, combinedSpec_;
	Moo::LightContainerPtr origDiffuse_, origSpec_;
};

/**
 *	Helper class to automatically begin/end a combiner via scope.
 */
class BorrowedLightCombinerHolder
{
public:
	BorrowedLightCombinerHolder( BorrowedLightCombiner& combiner, ChunkItemBase::Borrowers& borrowers )
		: combiner_( combiner )
	{
		combiner_.begin( borrowers );
	}
	~BorrowedLightCombinerHolder()
	{
		combiner_.end();
	}

private:
	BorrowedLightCombiner& combiner_;
};

#endif
