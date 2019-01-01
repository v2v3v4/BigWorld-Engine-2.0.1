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

#include "cstdmf/memory_tracker.hpp"

// Tests only run in Win32 Debug configuration, because thats all our memory
// tracking supports.
#ifdef _WIN32
#ifdef _DEBUG

TEST( MemoryTracker_testCounts )
{
	struct TestObject
	{
		uint32 a_;
		uint32 b_;
	};
	// We need this test object to be 8 bytes for later on
	CHECK_EQUAL( 8u, sizeof(TestObject) );

	MemTracker::AllocStats baseLine;

	// Get baseline
	MemTracker::instance().readStats( baseLine );

	// Perform some allocations
	char * string	= (char*)malloc( 128 ); // 128 bytes
	uint32 * array	= new uint32[16];		// 64  bytes
	TestObject* obj	= new TestObject;		// 8   bytes
											// 200 bytes total.
	// Get totals
	MemTracker::AllocStats totals;
	MemTracker::instance().readStats( totals );

	CHECK_EQUAL( baseLine.curBlocks_ + 3, totals.curBlocks_ );
	CHECK_EQUAL( baseLine.curBytes_ + 200, totals.curBytes_ );

	// Deallocate test allocations, we should be back to baseline.
	free( string );
	delete[] array;
	delete obj;

	MemTracker::instance().readStats( totals );

	CHECK_EQUAL( baseLine.curBlocks_, totals.curBlocks_ );
	CHECK_EQUAL( baseLine.curBytes_, totals.curBytes_ );
}

// This can't be tested automatically, as it will fire breaks.
/*
MEMTRACKER_DECLARE( testBreak, "testBreak", 0 );
MEMTRACKER_BREAK_ON_ALLOC( testBreak, 0 );
MEMTRACKER_BREAK_ON_ALLOC( testBreak, 1 );

TEST( MemoryTracker_testBreak )
{
	MEMTRACKER_SCOPED( testBreak );

	float* p1 = new float[10];
	float* p2 = new float[10];

	delete[] p1;
	delete[] p2;
}
*/

#endif // _DEBUG
#endif // _WIN32
